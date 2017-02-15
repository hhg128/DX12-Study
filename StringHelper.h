#pragma once

namespace StringHelper
{
	void ConvertStringToWString(const std::string& input, std::wstring& output);
	void ConvertWStringToString(const std::wstring& input, std::string& output);
}