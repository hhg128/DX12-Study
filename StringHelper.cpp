#include "stdafx.h"
#include "StringHelper.h"

namespace StringHelper
{
	void ConvertStringToWString(const std::string& input, std::wstring& output)
	{
		output.assign(input.begin(), input.end());
	}


	void ConvertWStringToString(const std::wstring& input, std::string& output)
	{
		output.assign(input.begin(), input.end());
	}

	void OutputDebugString(char* input)
	{
		OutputDebugString(std::string(input));
	}

	void OutputDebugString(std::string& input)
	{
		std::wstring output;
		ConvertStringToWString(input, output);

		::OutputDebugString(output.c_str());
	}

	void OutputDebugString(std::wstring& input)
	{
		::OutputDebugString(input.c_str());
	}
}