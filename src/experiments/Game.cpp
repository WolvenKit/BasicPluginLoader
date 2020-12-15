#include "Game.h"
#include "Hooking.Patterns.h"

template<typename TFn, size_t Len>
inline void SetPattern(TFn& func, const char(&pattern)[Len], int offset = 0)
{
	func = hook::get_pattern<std::remove_pointer_t<TFn>>(pattern, offset);
}

AddLocalizationEntry_t AddLocalizationEntry;
GetArchiveName_t GetArchiveName;
Hash2String_t Hash2String;

void AddGameFunctions()
{
	SetPattern(AddLocalizationEntry, "48 85 D2 74 10 48 C7 C0 FF", -21);
	SetPattern(GetArchiveName, "81 79 14 00 00 00 40 48 8B C1");
	SetPattern(Hash2String, "48 83 EC ? 48 8B 11 48 8D 4C 24 ? E8 ? ? ? ? 48 8B 00 48 83 C4 ? C3");
}