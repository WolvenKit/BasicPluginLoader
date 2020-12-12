#include "Utils.h"

#include <Windows.h>
#include <string>
#include <vector>
#include <codecvt>
#include <filesystem>

#define BUFFER_COUNT 8
#define BUFFER_LENGTH 32768

namespace utils
{
	const char* va(const char* string, ...)
	{
		static thread_local int currentBuffer;
		static thread_local std::vector<char> buffer;

		if (!buffer.size())
		{
			buffer.resize(BUFFER_COUNT * BUFFER_LENGTH);
		}

		int thisBuffer = currentBuffer;

		va_list ap;

		va_start(ap, string);
		int length = vsnprintf(&buffer[thisBuffer * BUFFER_LENGTH], BUFFER_LENGTH, string, ap);
		va_end(ap);

		if (length >= BUFFER_LENGTH)
		{
			return "";
		}

		buffer[(thisBuffer * BUFFER_LENGTH) + BUFFER_LENGTH - 1] = '\0';

		currentBuffer = (currentBuffer + 1) % BUFFER_COUNT;

		return &buffer[thisBuffer * BUFFER_LENGTH];
	}

	const wchar_t* va(const wchar_t* string, ...)
	{
		static thread_local int currentBuffer;
		static thread_local std::vector<wchar_t> buffer;

		if (!buffer.size())
		{
			buffer.resize(BUFFER_COUNT * BUFFER_LENGTH);
		}

		int thisBuffer = currentBuffer;

		va_list ap;

		va_start(ap, string);
		int length = vswprintf(&buffer[thisBuffer * BUFFER_LENGTH], BUFFER_LENGTH, string, ap);
		va_end(ap);

		if (length >= BUFFER_LENGTH)
		{
			return L"";
		}

		buffer[(thisBuffer * BUFFER_LENGTH) + BUFFER_LENGTH - 1] = '\0';

		currentBuffer = (currentBuffer + 1) % BUFFER_COUNT;

		return &buffer[thisBuffer * BUFFER_LENGTH];
	}

	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> m_converter;

	std::string to_narrow(const std::wstring& wide)
	{
		return m_converter.to_bytes(wide);
	}

	std::wstring to_wide(const std::string& narrow)
	{
		return m_converter.from_bytes(narrow);
	}

	template<typename Out>
	void split(const std::string& s, char delim, Out result)
	{
		std::stringstream ss(s);
		std::string item;

		while (std::getline(ss, item, delim))
		{
			*(result++) = item;
		}
	}

	std::vector<std::string> split(const std::string& s, char delim)
	{
		std::vector<std::string> elems;

		split(s, delim, std::back_inserter(elems));

		return elems;
	}

	std::vector<std::string> get_directories(const std::string& startDirectory)
	{
		std::vector<std::string> ret;

		for (auto& path : std::filesystem::recursive_directory_iterator(startDirectory))
		{
			if (path.status().type() == std::filesystem::file_type::directory)
			{
				ret.push_back(path.path().string());
			}
		}

		return ret;
	}

	bool ends_with(std::string const& fullString, std::string const& ending)
	{
		if (fullString.length() >= ending.length())
		{
			return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
		}
		else
		{
			return false;
		}
	}

	void log(const char* message, ...)
	{
		va_list args;
		char buffer[1024];
		char buffer2[1024];

		va_start(args, message);
		_vsnprintf(buffer, sizeof(buffer), message, args);
		va_end(args);

		_snprintf(buffer2, sizeof(buffer2), "[CP77Fun] %s\n", buffer);

		printf(buffer2);
		OutputDebugStringA(buffer2);
	}

	void log(const wchar_t* message, ...)
	{
		va_list args;
		wchar_t buffer[2048];
		wchar_t buffer2[2048];

		va_start(args, message);
		_vsnwprintf(buffer, sizeof(buffer), message, args);
		va_end(args);

		_snwprintf(buffer2, sizeof(buffer2), L"[CP77Fun] %s\n", buffer);

		wprintf(buffer2);
		OutputDebugStringW(buffer2);
	}
}