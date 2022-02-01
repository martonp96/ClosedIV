#include "main.h"

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
		logger::write("mods", "not encrypted RPF found");
		return;
	}
	DecryptHeaderOrig(salt, entryTable, size);
}

void(*DecryptHeader2Orig)(uint32_t, uint32_t, char*, int);
void DecryptHeader2Hook(uint32_t encryption, uint32_t salt, char* header, int nameTableLen)
{
	if (encryption == 0x4E45504F) //OPEN
	{
		logger::write("mods", "not encrypted RPF found");
		return;
	}
	DecryptHeader2Orig(encryption, salt, header, nameTableLen);
}

bool(*ParseHeaderOrig)(rage::fiPackfile*, const char*, bool, void*);
bool ParseHeaderHook(rage::fiPackfile* a1, const char* name, bool readHeader, void* customHeader)
{
	logger::write("rpf", "Parsing header for %s", name);

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

static memory::InitFuncs PackfileEncryptionHooks([] {
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
});