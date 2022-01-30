#include "main.h"

HANDLE OpenBulkHook(void* device, const char* path, __int64* a3)
{
	HANDLE FileW;
	WCHAR WideCharStr[256];

	*a3 = 0;

	MultiByteToWideChar(0xFDE9u, 0, path, -1, WideCharStr, 256);

	FileW = CreateFileW((L"mods/" + std::wstring(WideCharStr)).c_str(), 0x80000000, 1u, 0, 3, 0x80, 0);

	if(FileW == (HANDLE)-1)
		FileW = CreateFileW(WideCharStr, 0x80000000, 1u, 0, 3, 0x80, 0);
	else
	{
		logger::info("[%s] Found mods/%s", __FUNCTION__, path);
	}

	return FileW;
}

FILETIME GetFileTimeHook(void* device, const char* path)
{
	WCHAR WideCharStr[256];
	WIN32_FILE_ATTRIBUTE_DATA FileInformation;

	MultiByteToWideChar(0xFDE9u, 0, path, -1, WideCharStr, 256);

	if (GetFileAttributesExW((L"mods/" + std::wstring(WideCharStr)).c_str(), GetFileExInfoStandard, &FileInformation))
	{
		logger::info("[%s] Found mods/%s", __FUNCTION__, path);
		return FileInformation.ftLastWriteTime;
	}
	else if (GetFileAttributesExW(WideCharStr, GetFileExInfoStandard, &FileInformation))
		return FileInformation.ftLastWriteTime;
	else
		return (FILETIME)0;
}

uint64_t GetFileSizeHook(void* device, const char* path)
{
	WCHAR WideCharStr[256];
	WIN32_FILE_ATTRIBUTE_DATA FileInformation;

	MultiByteToWideChar(0xFDE9u, 0, path, -1, WideCharStr, 256);

	if (GetFileAttributesExW((L"mods/" + std::wstring(WideCharStr)).c_str(), GetFileExInfoStandard, &FileInformation))
	{
		logger::info("[%s] Found mods/%s", __FUNCTION__, path);
		return FileInformation.nFileSizeLow | (static_cast<size_t>(FileInformation.nFileSizeHigh) << 32);
	}
	else if (GetFileAttributesExW(WideCharStr, GetFileExInfoStandard, &FileInformation))
		return FileInformation.nFileSizeLow | (static_cast<size_t>(FileInformation.nFileSizeHigh) << 32);
	else
		return 0;
}

uint64_t GetAttributesHook(void* device, const char* path)
{
	WCHAR WideCharStr[256];
	DWORD FileAttributesW;

	MultiByteToWideChar(0xFDE9u, 0, path, -1, WideCharStr, 256);

	FileAttributesW = GetFileAttributesW((L"mods/" + std::wstring(WideCharStr)).c_str());

	if(FileAttributesW == -1)
		FileAttributesW = GetFileAttributesW(WideCharStr);
	else
	{
		logger::info("[%s] Found mods/%s", __FUNCTION__, path);
	}
	return FileAttributesW;
}

uint32_t currentEncryption;
bool FindEncryptionHook(uint32_t encryption)
{
	currentEncryption = encryption;
	return (encryption & 0xFF00000) == 0xFE00000;
}

void(*DecryptHeaderOrig)(uint32_t, char*, int);
void DecryptHeaderHook(uint32_t salt, char* entryTable, int size)
{
	if (currentEncryption == 0x4E45504F) //OPEN
	{
		logger::info("not encrypted RPF found");
		return;
	}
	DecryptHeaderOrig(salt, entryTable, size);
}

void(*DecryptHeader2Orig)(uint32_t, uint32_t, char*, int);
void DecryptHeader2Hook(uint32_t encryption, uint32_t salt, char* header, int nameTableLen)
{
	if (encryption == 0x4E45504F) //OPEN
	{
		logger::info("not encrypted RPF found");
		return;
	}
	DecryptHeader2Orig(encryption, salt, header, nameTableLen);
}

bool(*ParseHeaderOrig)(rage::fiPackfile*, const char*, bool, void*);
bool ParseHeaderHook(rage::fiPackfile* a1, const char* name, bool readHeader, void* customHeader)
{
	logger::info("Parsing header for %s", name);

	bool ret = ParseHeaderOrig(a1, name, readHeader, customHeader);
	if (ret)
	{
		for (int i = 0; i < a1->filesCount; ++i)
		{
			Entry* v21 = (Entry*)(a1->entryTable + 16 * i);
			if (v21->IsBinary() && v21->bin.nameOffset > 0 && v21->bin.isEncrypted)
			{
				if (currentEncryption == 0x4E45504F) //OPEN
					v21->bin.isEncrypted = 0xFEFFFFF;
			}
		}

		if (currentEncryption == 0x4E45504F) //OPEN
			a1->currentFileOffset = 0xFEFFFFF;
	}
	return ret;
}

static bool bInited = false;

void (*origGetSystemTimeAsFileTime)(LPFILETIME lpSystemTimeAsFileTime);
void HookGetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime)
{
	if (!bInited)
	{
		bInited = true;
		memory::init();

		//don't hide the console
		memory::scan("FF 15 ? ? ? ? E8 ? ? ? ? 65 48 8B 0C 25 ? ? ? ? 8B 05 ? ? ? ? 48 8B 04 C1 BA ? ? ? ? 83 24 02 00 E8").nop(6);

		//hooks for reading files
		memory::scan("40 53 48 81 EC ? ? ? ? 49 8B D8 4C 8B C2 48 8D 4C 24 ? BA ? ? ? ? E8 ? ? ? ? 48 83 64 24")
			.make_jmp_ret(OpenBulkHook);

		memory::scan("48 81 EC ? ? ? ? 4C 8B C2 48 8D 4C 24 ? BA ? ? ? ? E8 ? ? ? ? 4C 8D 44 24 ? 33 D2 48 8B C8 FF 15 ? ? ? ? 85 C0 75 04 33 C0 EB 0F 8B 44 24 38 8B 4C 24 34 48 C1 E0 20 48 0B C1 48 81 C4")
			.make_jmp_ret(GetFileTimeHook);

		memory::scan("48 81 EC ? ? ? ? 4C 8B C2 48 8D 4C 24 ? BA ? ? ? ? E8 ? ? ? ? 4C 8D 44 24 ? 33 D2 48 8B C8 FF 15 ? ? ? ? 85 C0 75 04 33 C0 EB 0F 8B 44 24 3C 8B 4C 24 40 48 C1 E0 20 48 0B C1 48 81 C4")
			.make_jmp_ret(GetFileSizeHook);

		memory::scan("48 89 5C 24 ? 57 48 81 EC ? ? ? ? 4C 8B C2 48 8D 4C 24 ? BA ? ? ? ? E8 ? ? ? ? 48 8B C8 FF 15 ? ? ? ? 83 CF FF 8B D8 3B C7 74 0F 48 8D 4C 24")
			.make_jmp_ret(GetAttributesHook);

		//allow unencrypted RPFs
		memory::scan("E8 ? ? ? ? 48 8B 53 20 44 8B C7 41 8B CE E8").set_call(FindEncryptionHook);

		auto mem = memory::scan("E8 ? ? ? ? 41 8B D4 44 39 63 28 76 3F 41 B9");
		DecryptHeaderOrig = mem.add(1).rip().as<decltype(DecryptHeaderOrig)>();
		mem.set_call(DecryptHeaderHook);

		mem = memory::scan("E8 ? ? ? ? 8B 55 F8 48 8B 43 10 48 03 D0 48 8B CB 48 89 53 18 66 44 89 22 33 D2 E8");
		DecryptHeader2Orig = mem.add(1).rip().as<decltype(DecryptHeader2Orig)>();
		mem.set_call(DecryptHeader2Hook);

		mem = memory::scan("44 88 BB ? ? ? ? 89 43 58 E8 ? ? ? ? 4C 8D 9C 24 ? ? ? ? 49 8B 5B 38 49 8B 73 40 49 8B 7B 48 49 8B E3 41 5F 41 5E 41 5D 41 5C 5D C3").add(10);
		ParseHeaderOrig = mem.add(1).rip().as<decltype(ParseHeaderOrig)>();
		mem.set_call(ParseHeaderHook);

		logger::info("ClosedIV Inited!");
	}
	GetSystemTimeAsFileTime(lpSystemTimeAsFileTime);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  dwReason, LPVOID lpReserved)
{
    if(dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		AllocConsole();

		FILE* unused = nullptr;
		freopen_s(&unused, "CONIN$", "r", stdin);
		freopen_s(&unused, "CONOUT$", "w", stdout);
		freopen_s(&unused, "CONOUT$", "w", stderr);

		//compatibility for any asi loader, as OpenIV supports only the one made by Alexander Blade
		if (!memory::HookIAT("kernel32.dll", "GetSystemTimeAsFileTime", (PVOID)HookGetSystemTimeAsFileTime, (PVOID*)&origGetSystemTimeAsFileTime)) {
			logger::info("[-] Hooking failed error (%ld)", GetLastError());
		}
    }
    return TRUE;
}