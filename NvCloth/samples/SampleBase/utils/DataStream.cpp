/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "DataStream.h"
#include <fstream>

DataStream::DataStream()
	:
	mReadPos(0),
	mWritePos(0)
{
	
}
DataStream::DataStream(void* data, int sizeInBytes)
	:
	mReadPos(0),
	mWritePos(0)
{
	write(data, sizeInBytes);
}
DataStream::DataStream(const char* filename)
	:
	mReadPos(0),
	mWritePos(0)
{
	std::ifstream file;
	file.open(filename,std::ifstream::binary | std::ios::ate);
	assert(file.good());

	std::streamsize size = file.tellg();
	mBuffer.resize(size);
	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char*>(mBuffer.data()), size);

	mWritePos = size;
	file.close();
}

bool DataStream::fileExists(const char* filename)
{
	std::ifstream file;
	file.open(filename, std::ifstream::binary | std::ios::ate);
	return file.good();
}

void DataStream::saveToFile(const char* filename)
{
	std::ofstream file(filename, std::fstream::binary | std::fstream::trunc);
	file.write(reinterpret_cast<char*>(mBuffer.data()), mBuffer.size());
	file.flush();
	file.close();
}

void DataStream::write(void* data, int sizeInBytes)
{
	if((int)mBuffer.size() < mWritePos + sizeInBytes)
		mBuffer.resize(mWritePos + sizeInBytes);
	memcpy(&mBuffer[mWritePos], data, sizeInBytes);
	mWritePos += sizeInBytes;
}