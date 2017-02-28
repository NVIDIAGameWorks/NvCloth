/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "SampleManager.h"

#include "Utils.h"

#include "Renderer.h"
#include "CommonUIController.h"
#include "SceneController.h"
#include "SampleController.h"


SampleManager::SampleManager(const SampleConfig& config) 
: m_config(config)
{
}

int SampleManager::run()
{
	Application app(getConfig().sampleName);

	m_renderer = new Renderer();
	m_sceneController = new SceneController();
	m_sampleController = new SampleController();
	m_commonUIController = new CommonUIController();

	app.addControllerToFront(m_renderer);
	app.addControllerToFront(m_sceneController);
	app.addControllerToFront(m_sampleController);
	app.addControllerToFront(m_commonUIController);

	for (IApplicationController* c : app.getControllers())
	{
		(static_cast<ISampleController*>(c))->setManager(this);
	}

	int result = app.run();

	delete m_renderer;
	delete m_sceneController;
	delete m_sampleController;
	delete m_commonUIController;

	return result;
}


int runSample(const SampleConfig& config)
{
	SampleManager sampleManager(config);
	return sampleManager.run();
}