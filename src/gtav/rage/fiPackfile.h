#pragma once
#include "fiDevice.h"

namespace rage
{
	class fiPackfile : public fiDevice
	{
	public:
		virtual ~fiPackfile() = default;

		char pad_0x0008[0x8];
		char* headers;
		char* entriesDepth;
		char* entryTable;
		__int32 filesCount;
		char pad_0x002C[4];
		HANDLE openedFileHandle;
		char fileTime[8];
		__int64 currentBulkOffset;
		__int32 hashSalt;
		char pad_0x004C[4];
		void* relativeDevice;
		char pad_0x0058[0x28];
		char* packfileName;
		char pad_0x0088[0x8];
		char isBinaryEntry;
		char uint8_0x91;
		char pad_0x0092[2];
		__int32 physSortKey;
		char pad_0x0098[2];
		__int64 memoryPtr;
		char pad_0x00000[4];
		char packFileOpened;
		char pad_0x00A9[2];
		char uint8_0xAB;
		char nameTableStart[4];
		__int32 currentFileOffset;
		__int32 encryptionMagic;
		char uint8_0xB8;
		char pad_0x00B9[71];
	};
}