#include "Game.h"
#include "Hooking.Patterns.h"

template<typename TFn, size_t Len>
inline void SetPattern(TFn& func, const char(&pattern)[Len], int offset = 0)
{
	func = hook::get_pattern<std::remove_pointer_t<TFn>>(pattern, offset);
}

AddLocalizationEntry_t AddLocalizationEntry;

void AddGameFunctions()
{
	SetPattern(AddLocalizationEntry, "48 85 D2 74 10 48 C7 C0 FF", -21);
	//SetPattern(Name2Hash, "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 4C 8B F1 48 8B F2 48 8B CA E8 ? ? ? ? 48");
	//SetPattern(Hash2Name, "48 83 EC ? 33 C0 48 89 54 24 ? 48 85 D2 74 ? 38 02 74 ? 48");
}