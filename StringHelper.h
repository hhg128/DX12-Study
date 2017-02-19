#pragma once

namespace StringHelper
{
	void ConvertStringToWString(const std::string& input, std::wstring& output);
	void ConvertWStringToString(const std::wstring& input, std::string& output);

	void OutputDebugString(char* input);
	void OutputDebugString(std::string& input);
	void OutputDebugString(std::wstring& input);
}