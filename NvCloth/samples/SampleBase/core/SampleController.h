/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef SAMPLE_CONTROLLER_H
#define SAMPLE_CONTROLLER_H

#include "SampleManager.h"

class SampleController : public ISampleController
{
public:
	SampleController();
	virtual ~SampleController();

	virtual void onSampleStart();

private:
	SampleController& operator= (SampleController&);


	//////// used controllers ////////

	SceneController& getSceneController() const
	{
		return getManager()->getSceneController();
	}

	CommonUIController& getCommonUIController() const
	{
		return getManager()->getCommonUIController();
	}

};

#endif