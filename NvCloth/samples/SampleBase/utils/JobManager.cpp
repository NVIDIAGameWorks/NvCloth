/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "JobManager.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <NvCloth/Solver.h>

void Job::Initialize(JobManager* parent, std::function<void(Job*)> function, int refcount)
{
	mFunction = function;
	mParent = parent;
	Reset(refcount);
}

Job::Job(const Job& job)
{
	mFunction = job.mFunction;
	mParent = job.mParent;
	mRefCount.store(job.mRefCount);
	mFinished = job.mFinished;
}

void Job::Reset(int refcount)
{
	mRefCount = refcount;
	mFinished = false;
}

void Job::Execute()
{
	if (mFunction)
		mFunction(this);
	else
		ExecuteInternal();

	{
		std::lock_guard<std::mutex> lock(mFinishedLock);
		mFinished = true;
	}
	mFinishedEvent.notify_one();
}

void Job::AddReference()
{
	mRefCount++;
}
void Job::RemoveReference()
{
	int refCount = --mRefCount;
	if (0 == refCount)
	{
		mParent->Submit(this);
	}
	assert(refCount >= 0);
}

void Job::Wait()
{
	std::unique_lock<std::mutex> lock(mFinishedLock);
	mFinishedEvent.wait(lock, [this](){return mFinished;} );
	lock.unlock();
	return;
}

void JobManager::WorkerEntryPoint(JobManager* parrent)
{
	while (true)
	{
		Job* job;
		{
			std::unique_lock<std::mutex> lock(parrent->mJobQueueLock);
			while (parrent->mJobQueue.size() == 0 && !parrent->mQuit)
				parrent->mJobQueueEvent.wait(lock);

			if (parrent->mQuit)
				return;

			job = parrent->mJobQueue.front();
			parrent->mJobQueue.pop();
		}
		job->Execute();
	}
}

void JobManager::Submit(Job* job)
{
	mJobQueueLock.lock();
	mJobQueue.push(job);
	mJobQueueLock.unlock();
	mJobQueueEvent.notify_one();
}

void MultithreadedSolverHelper::Initialize(nv::cloth::Solver* solver, JobManager* jobManager)
{
	mSolver = solver;
	mJobManager = jobManager;
	mEndSimulationJob.Initialize(mJobManager, [this](Job*) {
		mSolver->endSimulation();
	});

	mStartSimulationJob.Initialize(mJobManager, [this](Job*) {
		mSolver->beginSimulation(mDt);
		for(int j = 0; j < mSolver->getSimulationChunkCount(); j++)
			mSimulationChunkJobs[j].RemoveReference();
	});
}

void MultithreadedSolverHelper::StartSimulation(float dt)
{
	mDt = dt;

	if (mSolver->getSimulationChunkCount() != mSimulationChunkJobs.size())
	{
		mSimulationChunkJobs.resize(mSolver->getSimulationChunkCount(), JobDependency());
		for (int j = 0; j < mSolver->getSimulationChunkCount(); j++)
		{
			mSimulationChunkJobs[j].Initialize(mJobManager, [this, j](Job*) {mSolver->simulateChunk(j); });
			mSimulationChunkJobs[j].SetDependentJob(&mEndSimulationJob);
		}
	}
	else
	{
		for (int j = 0; j < mSolver->getSimulationChunkCount(); j++)
			mSimulationChunkJobs[j].Reset();
	}

	mStartSimulationJob.Reset();
	mEndSimulationJob.Reset(mSolver->getSimulationChunkCount());
	mStartSimulationJob.RemoveReference();
}

void MultithreadedSolverHelper::WaitForSimulation()
{
	if (mSolver->getSimulationChunkCount() == 0)
		return;

	mEndSimulationJob.Wait();
}