/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef SAMPLE_H
#define SAMPLE_H

#include <string>
#include <vector>


struct SampleConfig
{
	std::wstring			sampleName;
	std::vector<std::string> additionalResourcesDir;
};

int runSample(const SampleConfig& config);

#endif //SAMPLE_H