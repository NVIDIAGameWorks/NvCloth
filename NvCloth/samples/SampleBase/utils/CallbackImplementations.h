/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#pragma once
#include <NvCloth/Callbacks.h>
#include <foundation/PxAllocatorCallback.h>
#include <foundation/PxErrorCallback.h>
#include <foundation/PxAssert.h>
#include <foundation/PxProfiler.h>

#include <vector>
#include <map>

#if USE_CUDA
#include <cuda.h>
#endif

#include <string>
#include <sstream>
#include <assert.h>
#include <mutex>

#include <NvCloth/DxContextManagerCallback.h>

#ifdef _MSC_VER 
#include <Windows.h>
#endif

class Allocator : public physx::PxAllocatorCallback
{
	public:
		Allocator()
		{
			mEnableLeakDetection = false;
		}
		virtual void* allocate(size_t size, const char* typeName, const char* filename, int line)
		{
			#ifdef _MSC_VER 
				void* ptr = _aligned_malloc(size, 16);
			#else 
				void* ptr;
				if(posix_memalign(&ptr, 16, size)) ptr = 0;
			#endif
			if (mEnableLeakDetection)
			{
				std::lock_guard<std::mutex> lock(mAllocationsMapLock);
				mAllocations[ptr] = Allocation(size, typeName, filename, line);
			}
			return ptr;
		}
		virtual void deallocate(void* ptr)
		{
			if (mEnableLeakDetection && ptr)
			{
				std::lock_guard<std::mutex> lock(mAllocationsMapLock);
				auto i = mAllocations.find(ptr);
				if (i == mAllocations.end())
				{
					printf("Tried to deallocate %p which was not allocated with this allocator callback.",ptr);
				}
				else
				{
					mAllocations.erase(i);
				}
			}
		#ifdef _MSC_VER 
			_aligned_free(ptr);
		#else 
			free(ptr);
		#endif
		}

		void StartTrackingLeaks()
		{
			std::lock_guard<std::mutex> lock(mAllocationsMapLock);
			mAllocations.clear();
			mEnableLeakDetection = true;
		}

		void StopTrackingLeaksAndReport()
		{
			std::lock_guard<std::mutex> lock(mAllocationsMapLock);
			mEnableLeakDetection = false;
	
			size_t totalBytes = 0;
			std::stringstream message;
			message << "Memory leaks detected:\n";
			for (auto it = mAllocations.begin(); it != mAllocations.end(); ++it)
			{
				const Allocation& alloc = it->second;
				message << "* Allocated ptr " << it->first << " of " << alloc.mSize << "bytes (type=" << alloc.mTypeName << ") at " << alloc.mFileName << ":" << alloc.mLine<<"\n";
				totalBytes += alloc.mSize;
			}
			if (mAllocations.size()>0)
			{
				message << "=====Total of " << totalBytes << " bytes in " << mAllocations.size() << " allocations leaked=====";
				const std::string& tmp = message.str(); 
#ifdef _MSC_VER 
//				OutputDebugString(tmp.c_str()); //Write to visual studio output so we can see it after the application closes
#endif
				printf("%s\n", tmp.c_str());
			}
	
			mAllocations.clear();
		}
	private:
		bool mEnableLeakDetection;
		struct Allocation
		{
			Allocation(){}
			Allocation(size_t size, const char* typeName, const char* filename, int line)
				: mSize(size), mTypeName(typeName), mFileName(filename), mLine(line)
			{

			}
			size_t mSize;
			std::string mTypeName;
			std::string mFileName;
			int mLine;
		};
		std::map<void*,Allocation> mAllocations;
		std::mutex mAllocationsMapLock;
};

class ErrorCallback : public physx::PxErrorCallback
{
	public:
		ErrorCallback(){}
		virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line);
};


class AssertHandler : public physx::PxAssertHandler
{
	public:
		virtual void operator()(const char* exp, const char* file, int line, bool& ignore)
		{
            PX_UNUSED(ignore);
			printf("NV_CLOTH_ASSERT(%s) from file:%s:%d Failed\n", exp, file, line);
			assert(("Assertion failed, see log/console for more info.",0));
		}
};


class NvClothEnvironment
{
		NvClothEnvironment()
		{
			SetUp();
		}
		virtual ~NvClothEnvironment() 
		{
			TearDown();
		}

		static NvClothEnvironment* sEnv;

	public:
		static void AllocateEnv()
		{ 
			sEnv = new NvClothEnvironment; 
		}
		static void FreeEnv(){ delete sEnv; sEnv = nullptr; }
		static void ReportEnvFreed(){ sEnv = nullptr; } //google test will free it for us, so we just reset the value
		static NvClothEnvironment* GetEnv(){ return sEnv; }

		virtual void SetUp()
		{
			mAllocator = new Allocator;
			mAllocator->StartTrackingLeaks();
			mFoundationAllocator = new Allocator;
			mFoundationAllocator->StartTrackingLeaks();
			mErrorCallback = new ErrorCallback;
			mAssertHandler = new AssertHandler;
			nv::cloth::InitializeNvCloth(mAllocator, mErrorCallback, mAssertHandler, nullptr);
#if USE_CUDA
			cuInit(0);
#endif
        }
		virtual void TearDown()
		{
			mAllocator->StopTrackingLeaksAndReport();
			mFoundationAllocator->StopTrackingLeaksAndReport();
			delete mErrorCallback;
			delete mFoundationAllocator;
			delete mAllocator;
			delete mAssertHandler;
		}

		Allocator* GetAllocator(){ return mAllocator; }
		Allocator* GetFoundationAllocator(){ return mFoundationAllocator; }
		ErrorCallback* GetErrorCallback(){ return mErrorCallback; }
		AssertHandler* GetAssertHandler(){ return mAssertHandler; }

	private:
		Allocator* mAllocator;
		Allocator* mFoundationAllocator;
		ErrorCallback* mErrorCallback;
		AssertHandler* mAssertHandler;
};

//#if USE_DX11
class DxContextManagerCallbackImpl : public nv::cloth::DxContextManagerCallback
{
public:
	DxContextManagerCallbackImpl(ID3D11Device* device, bool synchronizeResources = false);
	~DxContextManagerCallbackImpl();
	virtual void acquireContext() override;
	virtual void releaseContext() override;
	virtual ID3D11Device* getDevice() const override;
	virtual ID3D11DeviceContext* getContext() const override;
	virtual bool synchronizeResources() const override;

private:
	std::recursive_mutex mMutex;
	ID3D11Device* mDevice;
	ID3D11DeviceContext* mContext;
	bool mSynchronizeResources;
#ifdef _DEBUG
	uint32_t mLockCountTls;
#endif
};
//#endif
