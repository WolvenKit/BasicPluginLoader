#define _CRT_SECURE_NO_WARNINGS 1

#include <Windows.h>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <strsafe.h>
#include <filesystem>
#include <DbgHelp.h>

#include "Hooking.Patterns.h"
#include "deps/include/MinHook.h"
#include "deps/include/injector/injector.hpp"
#include "Utils.h"

#pragma comment(lib, "deps/lib/MinHook-x64.lib")
#pragma comment(lib, "Dbghelp.lib")

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

LONG WINAPI CustomUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
	// step 1: write minidump
	static LPEXCEPTION_POINTERS exceptionData;

	exceptionData = ExceptionInfo;

	wchar_t error[1024] = { 0 };
	wchar_t filename[MAX_PATH];
	__time64_t time;
	tm* ltime;

	_time64(&time);
	ltime = _localtime64(&time);
	wcsftime(filename, std::size(filename) - 1, L"cp77f-%Y%m%d%H%M%S.dmp", ltime);
	_snwprintf(error, std::size(error) - 1, L"A minidump has been written to %s.", filename);

	HANDLE hFile = CreateFileW(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION ex;
		memset(&ex, 0, sizeof(ex));
		ex.ThreadId = GetCurrentThreadId();
		ex.ExceptionPointers = exceptionData;
		ex.ClientPointers = FALSE;

#if defined(DEBUG)
		MINIDUMP_TYPE mdType = (MINIDUMP_TYPE)(MiniDumpWithProcessThreadData | MiniDumpWithUnloadedModules | MiniDumpWithThreadInfo | MiniDumpWithFullMemory);
#else
		MINIDUMP_TYPE mdType = MiniDumpNormal;
#endif

		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, mdType, &ex, NULL, NULL);
		utils::log(error);

		CloseHandle(hFile);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

bool ret1()
{
	return true;
}

bool ret0()
{
	return false;
}

static void(__cdecl* Orig_SetFeatureDefault)(void* vt, const char* overlayName, int enabled);
void __fastcall Hook_SetFeatureDefault(void* vt, const char* overlayName, int enabled)
{
	utils::log("Forcing feature: %s", overlayName);
	Orig_SetFeatureDefault(vt, overlayName, true);
}

static int(__cdecl* Orig_FeatureCtor)(__int64 a1, const char* category, const char* value, char a4, char a5);
int __fastcall Hook_FeatureCtor(__int64 a1, const char* category, const char* value, char a4, char a5)
{
	utils::log(L"ctor: %s -> %s (%d, %d)", utils::to_wide(category).c_str(), utils::to_wide(value).c_str(), a4, a5);
	Orig_FeatureCtor(a1, category, value, a4, a5);
}

static uint16_t(__cdecl* Orig_FeatureCtor2)(__int64 a1, const char* category);
uint16_t __fastcall Hook_FeatureCtor2(__int64 a1, const char* category)
{
	utils::log(L"ctor: %s -> %s (%d, %d)", utils::to_wide(category).c_str());
	return Orig_FeatureCtor2(a1, category);
}

FILE* hashes;

static __int64(__cdecl* Orig_RegisterHashString)(__int64 a1, const char* str);
__int64 __fastcall Hook_RegisterHashString(__int64 a1, const char* str)
{
	fprintf(hashes, "{ \"%s\", 0x%llx },\n", str, a1);
	fflush(hashes);
	return Orig_RegisterHashString(a1, str);
}

DWORD WINAPI OnAttachImpl(LPVOID lpParameter)
{
	utils::log("[+] OnAttachImpl");

	hook::set_base((uintptr_t)GetModuleHandle(nullptr));

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("E8 ? ? ? ? 48 85 C0 74 ? 0F B7 10", -0x14), Hook_SetFeatureDefault, (void**)&Orig_SetFeatureDefault);
	MH_CreateHook(hook::get_pattern("4C 89 41 08 48 89 01 33 C0 48 89 41 18", -0x07), Hook_FeatureCtor, (void**)&Orig_FeatureCtor);
	MH_CreateHook(hook::pattern("48 8D ? ? ? ? ? 4C 89 41 08 48 89 01 33").count(2).get(1).get<void*>(), Hook_FeatureCtor2, (void**)&Orig_FeatureCtor2);
	MH_CreateHook(hook::get_pattern("48 83 EC 38 33 C0 48 89 54 24 20"), Hook_RegisterHashString, (void**)&Orig_RegisterHashString);

	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
	{
		utils::log("[*] OnAttachImpl: failed hooking");
	}

	// old gog 1.03 offsets
	/*injector::MakeNOP(hook::get_adjusted(0x140A98A6B), 6); // overlay
	injector::MakeNOP(hook::get_adjusted(0x140A98B47), 2); // versionstr
	injector::MakeNOP(hook::get_adjusted(0x140A98C13), 2); // dbg #1
	injector::MakeNOP(hook::get_adjusted(0x140A98C21), 2); // dbg #2
	injector::MakeNOP(hook::get_adjusted(0x140AC8657), 6); // 
	injector::MakeNOP(hook::get_adjusted(0x140AD4246), 6); // imgui?

	// SetDebugOverlayVisibility
	injector::WriteObject<uint64_t>(hook::get_adjusted(0x143812248), 3, true);*/

	utils::log("[-] OnAttachImpl");

	return 0;
}

DWORD WINAPI OnAttach(LPVOID lpParameter)
{
	__try
	{
		return OnAttachImpl(lpParameter);
	}
	__except (CustomUnhandledExceptionFilter(GetExceptionInformation()))
	{
		utils::log("MODULE CRASHED! UNLOADING.");
		FreeLibraryAndExitThread((HMODULE)lpParameter, 0xDECEA5ED);
	}

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		TCHAR fileName[MAX_PATH + 1];
		DWORD charsWritten = GetModuleFileName(NULL, fileName, MAX_PATH + 1);

		if (utils::ends_with(fileName, "REDEngineErrorReporter.exe"))
		{
			utils::log("omitting error reporter");
		}
		else
		{
			AttachConsole();

			hashes = fopen("hashlist.txt", "w");

			// IsDebuggerPresent int3 traps
			{
				auto locations = hook::pattern("FF ? ? ? ? ? 85 C0 74 01 CC");
				if (locations.size() > 0)
				{
					for (int i = 0; i < locations.size(); i++)
					{
						char* loc = locations.get(i).get<char>(10);
						injector::MakeNOP(loc, 1);
					}
				}
			}

			CreateThread(nullptr, 0, OnAttach, hModule, 0, nullptr);
		}
	}

	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		MH_DisableHook(MH_ALL_HOOKS);
	}

	return true;
}