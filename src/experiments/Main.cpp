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

#include "Hooking.Patterns.h"
#include <MinHook.h>
#include <injector/injector.hpp>
#include "Utils.h"

#define LOGGER_NAME "Experiments"
#include <logging.h>

static void(__cdecl* Orig_SetFeatureDefault)(void* vt, const char* overlayName, int enabled);
void __fastcall Hook_SetFeatureDefault(void* vt, const char* overlayName, int enabled)
{
	logger::write("Forcing feature: %s", overlayName);
	Orig_SetFeatureDefault(vt, overlayName, true);
}

static int(__cdecl* Orig_FeatureCtor)(__int64 a1, const char* category, const char* value, char a4, char a5);
int __fastcall Hook_FeatureCtor(__int64 a1, const char* category, const char* value, char a4, char a5)
{
	logger::write(L"ctor: %s -> %s (%d, %d)", utils::to_wide(category).c_str(), utils::to_wide(value).c_str(), a4, a5);
	return Orig_FeatureCtor(a1, category, value, a4, a5);
}

static uint16_t(__cdecl* Orig_FeatureCtor2)(__int64 a1, const char* category);
uint16_t __fastcall Hook_FeatureCtor2(__int64 a1, const char* category)
{
	logger::write(L"ctor: %s -> %s (%d, %d)", utils::to_wide(category).c_str());
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

DWORD WINAPI OnAttach(LPVOID lpParameter)
{
	logger::write("[+] OnAttach");

	hashes = fopen("hashes.txt", "w");

	hook::set_base((uintptr_t)GetModuleHandle(nullptr));

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("E8 ? ? ? ? 48 85 C0 74 ? 0F B7 10", -0x14), Hook_SetFeatureDefault, (void**)&Orig_SetFeatureDefault);
	MH_CreateHook(hook::get_pattern("4C 89 41 08 48 89 01 33 C0 48 89 41 18", -0x07), Hook_FeatureCtor, (void**)&Orig_FeatureCtor);
	MH_CreateHook(hook::pattern("48 8D ? ? ? ? ? 4C 89 41 08 48 89 01 33").count(2).get(1).get<void*>(), Hook_FeatureCtor2, (void**)&Orig_FeatureCtor2);
	MH_CreateHook(hook::get_pattern("48 83 EC 38 33 C0 48 89 54 24 20"), Hook_RegisterHashString, (void**)&Orig_RegisterHashString);

	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
	{
		logger::write("[*] OnAttach: failed hooking");
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

	logger::write("[-] OnAttach");

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		freopen("CON", "w", stdout);
		freopen("CONIN$", "r", stdin);

		CreateThread(nullptr, 0, OnAttach, hModule, 0, nullptr);
	}

	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		MH_DisableHook(MH_ALL_HOOKS);
	}

	return true;
}