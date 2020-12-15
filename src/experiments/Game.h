typedef void(__cdecl* AddLocalizationEntry_t)(const char* text, const char* locKey);
extern AddLocalizationEntry_t AddLocalizationEntry;

typedef const char* (__cdecl* GetArchiveName_t)(void* archive);
extern GetArchiveName_t GetArchiveName;

typedef const char* (__cdecl* Hash2String_t)(__int64* hash);
extern Hash2String_t Hash2String;