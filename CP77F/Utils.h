#pragma once

#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS 1

#include <string>
#include <vector>
#include <sstream>

namespace utils
{

	const char* va(const char* string, ...);
	const wchar_t* va(const wchar_t* string, ...);

	std::string to_narrow(const std::wstring& wide);
	std::wstring to_wide(const std::string& narrow);

	template<typename Out>
	void split(const std::string& s, char delim, Out result);

	std::vector<std::string> split(const std::string& s, char delim);
	std::vector<std::string> get_directories(const std::string& startDirectory);
	bool ends_with(std::string const& fullString, std::string const& ending);

	void log(const char* message, ...);
	void log(const wchar_t* message, ...);
}