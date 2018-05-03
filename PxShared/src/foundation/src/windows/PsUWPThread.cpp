// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2014 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#include "foundation/PxPreprocessor.h"

#if PX_UWP

#include "foundation/PxAssert.h"

#include "windows/PsWindowsInclude.h"
#include "PsThread.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;

// an exception for setting the thread name in microsoft debuggers
#define NS_MS_VC_EXCEPTION 0x406D1388

namespace physx
{
namespace shdfnd
{

#define PX_TLS_MAX_SLOTS 64

namespace
{

// struct for naming a thread in the debugger
#pragma pack(push, 8)

typedef struct tagTHREADNAME_INFO
{
	DWORD dwType;     // Must be 0x1000.
	LPCSTR szName;    // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags;    // Reserved for future use, must be zero.
} THREADNAME_INFO;

#pragma pack(pop)

// Used to keep track of which TLS slots have been used
volatile DWORD gTlsSlots[PX_TLS_MAX_SLOTS] = { 0 };

// Actual TLS storage, unique per thread
__declspec(thread) void* tlsData[PX_TLS_MAX_SLOTS];

// Sets all the slot values to zero.  This method will be called every time a new Px thread is created
void PxClearTlsData()
{
	for(unsigned int i = 0; i < PX_TLS_MAX_SLOTS; i++)
	{
		tlsData[i] = NULL;
	}
}

class _ThreadImpl
{
  public:
	enum State
	{
		NotStarted,
		Started,
		Stopped
	};

	PX_FORCE_INLINE _ThreadImpl(ThreadImpl::ExecuteFn fn_, void* arg_);

	IAsyncAction ^ asyncAction;
	HANDLE signal;
	LONG quitNow; // Should be 32bit aligned on SMP systems.
	State state;
	DWORD threadID;

	ThreadImpl::ExecuteFn fn;
	void* arg;
};

PX_FORCE_INLINE _ThreadImpl::_ThreadImpl(ThreadImpl::ExecuteFn fn_, void* arg_)
: asyncAction(nullptr), quitNow(0), state(_ThreadImpl::NotStarted), threadID(0xffffFFFF), fn(fn_), arg(arg_)
{
	signal = CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
}

_ThreadImpl* getThread(ThreadImpl* impl)
{
	return reinterpret_cast<_ThreadImpl*>(impl);
}

DWORD WINAPI PxThreadStart(LPVOID arg)
{
	PxClearTlsData();
	_ThreadImpl* impl = getThread((ThreadImpl*)arg);

	// run either the passed in function or execute from the derived class (Runnable).
	if(impl->fn)
		(*impl->fn)(impl->arg);
	else if(impl->arg)
		((Runnable*)impl->arg)->execute();
	return 0;
}
}

static const uint32_t gSize = sizeof(_ThreadImpl);
uint32_t ThreadImpl::getSize()
{
	return gSize;
}

ThreadImpl::Id ThreadImpl::getId()
{
	return static_cast<Id>(GetCurrentThreadId());
}

ThreadImpl::ThreadImpl()
{
	PX_PLACEMENT_NEW(getThread(this), _ThreadImpl)(NULL, NULL);
}

ThreadImpl::ThreadImpl(ExecuteFn fn, void* arg, const char*)
{
	PX_PLACEMENT_NEW(getThread(this), _ThreadImpl)(fn, arg);
	start(0, NULL);
}

ThreadImpl::~ThreadImpl()
{
	if(getThread(this)->state == _ThreadImpl::Started)
		kill();
	if(getThread(this)->asyncAction)
		getThread(this)->asyncAction->Close();
	CloseHandle(getThread(this)->signal);
}

void ThreadImpl::start(uint32_t /*stackSize*/, Runnable* runnable)
{
	if(getThread(this)->state != _ThreadImpl::NotStarted)
		return;
	getThread(this)->state = _ThreadImpl::Started;

	if(runnable && !getThread(this)->arg && !getThread(this)->fn)
		getThread(this)->arg = runnable;

	// run lambda
	auto workItemDelegate = [this](IAsyncAction ^ workItem)
	{
		PxThreadStart((LPVOID) this); // function to run async
	};

	// onComplete lambda
	auto completionDelegate = [this](IAsyncAction ^ action, AsyncStatus /*status*/)
	{
		switch(action->Status)
		{
		case AsyncStatus::Started:
			break;
		case AsyncStatus::Completed:
		case AsyncStatus::Canceled:
		case AsyncStatus::Error:
			SetEvent(getThread(this)->signal);
			break;
		}
	};

	// thread pool work item, can run on any thread
	auto workItemHandler = ref new WorkItemHandler(workItemDelegate, CallbackContext::Any);
	// thread status handler (signal complete), can run on any thread
	auto completionHandler = ref new AsyncActionCompletedHandler(completionDelegate, Platform::CallbackContext::Any);

	// run with normal priority, time sliced
	getThread(this)->asyncAction =
	    ThreadPool::RunAsync(workItemHandler, WorkItemPriority::Normal, WorkItemOptions::TimeSliced);
	getThread(this)->asyncAction->Completed = completionHandler;
}

void ThreadImpl::signalQuit()
{
	InterlockedIncrement(&(getThread(this)->quitNow));
}

bool ThreadImpl::waitForQuit()
{
	if(getThread(this)->state == _ThreadImpl::NotStarted)
		return false;

	WaitForSingleObjectEx(getThread(this)->signal, INFINITE, false);

	return true;
}

bool ThreadImpl::quitIsSignalled()
{
	return InterlockedCompareExchange(&(getThread(this)->quitNow), 0, 0) != 0;
}

void ThreadImpl::quit()
{
	getThread(this)->state = _ThreadImpl::Stopped;
}

void ThreadImpl::kill()
{
	if(getThread(this)->state == _ThreadImpl::Started && getThread(this)->asyncAction)
	{
		getThread(this)->asyncAction->Cancel();
		InterlockedIncrement(&(getThread(this)->quitNow));
	}
	getThread(this)->state = _ThreadImpl::Stopped;
}

void ThreadImpl::sleep(uint32_t ms)
{
	// find something better than this:
	if(ms == 0)
		yield();
	else
	{
		HANDLE handle = CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
		WaitForSingleObjectEx(handle, ms, false);
		CloseHandle(handle);
	}
}

void ThreadImpl::yield()
{
	YieldProcessor();
}

uint32_t ThreadImpl::setAffinityMask(uint32_t /*mask*/)
{
	return 0;
}

void ThreadImpl::setName(const char* name)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = name;
	info.dwThreadID = getThread(this)->threadID;
	info.dwFlags = 0;

	// C++ Exceptions are disabled for this project, but SEH is not (and cannot be)
	// http://stackoverflow.com/questions/943087/what-exactly-will-happen-if-i-disable-c-exceptions-in-a-project
	__try
	{
		RaiseException(NS_MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// this runs if not attached to a debugger (thus not really naming the thread)
	}
}

void ThreadImpl::setPriority(ThreadPriority::Enum /*prio*/)
{
	// AFAIK this can only be done at creation in Metro Apps
}

ThreadPriority::Enum ThreadImpl::getPriority(Id /*threadId*/)
{
	// see setPriority
	return ThreadPriority::eNORMAL;
}

uint32_t TlsAlloc()
{
	for(unsigned int i = 0; i < PX_TLS_MAX_SLOTS; i++)
		if(InterlockedCompareExchange(&gTlsSlots[i], TRUE, FALSE) == FALSE)
			return i;
	PX_ASSERT(!"Cannot find a free tls slot, increase PX_TLS_MAX_SLOTS");
	return (uint32_t) - 1;
}

void TlsFree(uint32_t index)
{
	PX_ASSERT(index < PX_TLS_MAX_SLOTS);
	InterlockedExchange(&gTlsSlots[index], 0);
	// Clear the value for this slot.. This is a bit iffy as it clears only the value for the thread
	// that calls PxTlsFree()
	tlsData[index] = NULL;
}

void* TlsGet(uint32_t index)
{
	PX_ASSERT(index < PX_TLS_MAX_SLOTS);
	void* value = tlsData[index];
	return value;
}

uint32_t TlsSet(uint32_t index, void* value)
{
	PX_ASSERT(index < PX_TLS_MAX_SLOTS);
	tlsData[index] = value;
	return 1;
}

uint32_t ThreadImpl::getDefaultStackSize()
{
	return 1048576;
};

uint32_t ThreadImpl::getNbPhysicalCores()
{
	SYSTEM_INFO info;
	GetNativeSystemInfo(&info);
	return info.dwNumberOfProcessors;
}

} // namespace shdfnd
} // namespace physx

#endif // PX_UWP
