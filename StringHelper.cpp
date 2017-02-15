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
}