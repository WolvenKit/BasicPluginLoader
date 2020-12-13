#include <Windows.h>
#include <filesystem>
#include <DbgHelp.h>
#include "buildnumber.h"

#define LOGGER_NAME "Loader"
#include <logging.h>

void AttachConsole()
{
	// allocate console
	AllocConsole();
	AttachConsole(GetCurrentProcessId());

	// get console window
	HWND hConsole = GetConsoleWindow();
	RECT rect;
	GetWindowRect(hConsole, &rect);

	// if there's multiple monitors, put it on the secondary one
	if (GetSystemMetrics(SM_CMONITORS) > 1)
	{
		HWND hDesktop = GetDesktopWindow();
		RECT rDesktopRect;
		GetWindowRect(hDesktop, &rDesktopRect);

		SetWindowPos(hConsole, nullptr, rDesktopRect.left - rect.right - 50, 200, rect.right - rect.left, rect.bottom - rect.top, 0);
	}

	// redirect printf to console
	freopen("CON", "w", stdout);
	freopen("CONIN$", "r", stdin);
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

std::string get_relative_path(const std::string& path)
{
	static std::string relPath;

	if (relPath.empty())
	{
		char exeName[512];
		GetModuleFileNameA(GetModuleHandle(NULL), exeName, sizeof(exeName));

		char* exeBaseName = strrchr(exeName, '\\');
		exeBaseName[0] = L'\0';

		relPath = exeName;
		relPath += "\\";

		GetFullPathNameA(relPath.c_str(), sizeof(exeName), exeName, nullptr);

		relPath = exeName;
		relPath += "\\";
	}

	return relPath + path;
}

DWORD WINAPI OnAttach(LPVOID lpParameter)
{
	std::filesystem::path plugins_path(get_relative_path("plugins\\"));
	std::filesystem::directory_iterator it(plugins_path), end;

	while (it != end)
	{
		if (it->path().extension() == ".dll" ||
			it->path().extension() == ".asi")
		{
			void* plugin = LoadLibrary(it->path().c_str());

			if (!plugin)
			{
				logger::write(L"[*] Failed loading plugin %s\n", it->path().filename().c_str());
			}
			else
			{
				logger::write(L"[+] Loaded plugin %s\n", it->path().filename().c_str());
			}
		}

		it++;
	}

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		char fileName[MAX_PATH + 1];
		DWORD charsWritten = GetModuleFileNameA(NULL, fileName, MAX_PATH + 1);

		if (!ends_with(fileName, "REDEngineErrorReporter.exe"))
		{
			std::filesystem::create_directory("plugins\\");
			std::filesystem::create_directory("plugins\\errors\\");

			AttachConsole();
			logger::write("Version " BUILDNUMBER_STR " built by " BUILDHOST "\n");
			CreateThread(nullptr, 0, OnAttach, hModule, 0, nullptr);
		}
	}

	return true;
}