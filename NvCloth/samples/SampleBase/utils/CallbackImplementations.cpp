/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "CallbackImplementations.h"
#include "JobManager.h"
#include <iostream>

//#if USE_DX11
#include <d3d11.h>
//#endif

NvClothEnvironment* NvClothEnvironment::sEnv = nullptr;


void ErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	const char* codeName = "???";
	switch(code)
	{
#define CASE(x) case physx::PxErrorCode::Enum::x : codeName = #x; break;
		CASE(eNO_ERROR)
		CASE(eDEBUG_INFO)
		CASE(eDEBUG_WARNING)
		CASE(eINVALID_PARAMETER)
		CASE(eINVALID_OPERATION)
		CASE(eOUT_OF_MEMORY)
		CASE(eINTERNAL_ERROR)
		CASE(eABORT)
		CASE(ePERF_WARNING)
		default:
			;
#undef CASE
	}

	std::cout << "Log " << codeName << " from file:" << file << ":" << line << "\n MSG:" << message << std::endl;
}

//#if USE_DX11
DxContextManagerCallbackImpl::DxContextManagerCallbackImpl(ID3D11Device* device, bool synchronizeResources)
	:
	mDevice(device),
	mSynchronizeResources(synchronizeResources)
{
	mDevice->AddRef();
	mDevice->GetImmediateContext(&mContext);
}
DxContextManagerCallbackImpl::~DxContextManagerCallbackImpl()
{
	mContext->Release();

#if _DEBUG
	ID3D11Debug* debugDevice;
	mDevice->QueryInterface(&debugDevice);
	if(debugDevice)
	{
		debugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		debugDevice->Release();
	}
#endif

	mDevice->Release();

}

void DxContextManagerCallbackImpl::acquireContext()
{

	mMutex.lock();
}
void DxContextManagerCallbackImpl::releaseContext()
{
	mMutex.unlock();
}
ID3D11Device* DxContextManagerCallbackImpl::getDevice() const
{
	return mDevice;
}
ID3D11DeviceContext* DxContextManagerCallbackImpl::getContext() const
{
	return mContext;
}
bool DxContextManagerCallbackImpl::synchronizeResources() const
{
	return mSynchronizeResources;
}
//#endif
