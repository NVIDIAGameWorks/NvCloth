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
#include <vector>
#include <string>
#include <assert.h>

class DataStream
{
public:
	DataStream();
	DataStream(void* data, int sizeInBytes);
	DataStream(const char* filename);
	static bool fileExists(const char* filename);

	void saveToFile(const char* filename);

	template<typename T>
	void write(T& value)
	{
		if(mBuffer.size() < mWritePos + sizeof(T))
			mBuffer.resize(mWritePos + sizeof(T));
		memcpy(&mBuffer[mWritePos], &value, sizeof(T));
		mWritePos += sizeof(T);
	}
	template<>
	void write<std::string>(std::string& value)
	{
		int len = (int)value.length();
		if(mBuffer.size() < mWritePos + len + sizeof(int))
			mBuffer.resize(mWritePos + len + sizeof(int));
		memcpy(&mBuffer[mWritePos], &len, sizeof(int));
		mWritePos += sizeof(int);
		memcpy(&mBuffer[mWritePos], value.c_str(), len);
		mWritePos += len;
	}

	void write(void* data, int sizeInBytes);

	template<typename T>
	T read()
	{
		T value;
		assert(mReadPos + sizeof(T) <= mBuffer.size());
		memcpy(&value, &mBuffer[mReadPos], sizeof(T));
		mReadPos += sizeof(T);
		return value;
	}
	template<>
	std::string read<std::string>()
	{
		int len = read<int>();
		std::string value;
		value.resize(len);
		assert(mReadPos + len < (int)mBuffer.size());
		memcpy(&value[0], &mBuffer[mReadPos], len);
		mReadPos += len;
		return value;
	}
private:
	std::vector<unsigned char> mBuffer;
	int mReadPos;
	int mWritePos;
};