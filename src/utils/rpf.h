#pragma once

struct DirectoryEntry
{
	uint32_t nameOffset;
	uint32_t entryType;
	uint32_t entryIndex;
	uint32_t entriesCnt;
};

struct BinaryEntry
{
	uint16_t nameOffset;
	uint8_t fileSize[3];
	uint8_t fileOffset[3];
	uint32_t realSize;
	uint32_t isEncrypted;

	uint32_t GetFileSize()
	{
		return fileSize[0] + (fileSize[1] << 8) + (fileSize[2] << 16);
	}

	uint32_t GetFileOffset()
	{
		return fileOffset[0] + (fileOffset[1] << 8) + (fileOffset[2] << 16);
	}

	bool IsCompressed()
	{
		return GetFileSize() != 0;
	}
};

struct ResourceEntry
{
	uint16_t nameOffset;
	uint8_t fileSize[3];
	uint8_t fileOffset[3];
	uint32_t systemFlags;
	uint32_t graphicsFlags;

	uint32_t GetFileSize()
	{
		return fileSize[0] + (fileSize[1] << 8) + (fileSize[2] << 16);
	}

	uint32_t GetFileOffset()
	{
		return (fileOffset[0] + (fileOffset[1] << 8) + (fileOffset[2] << 16)) & 0x7FFFFF;
	}
};

struct Entry
{
	union
	{
		DirectoryEntry dir;
		BinaryEntry bin;
		ResourceEntry res;
	};

	bool IsDirectory()
	{
		return dir.entryType == 0x7FFFFF00;
	}

	bool IsBinary()
	{
		return !IsDirectory() && (dir.entryType & 0x80000000) == 0L;
	}

	bool IsResource()
	{
		return !IsDirectory() && !IsBinary();
	}
};