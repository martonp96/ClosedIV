#include "main.h"

bool rage::fiDevice::Mount(const char* mountPoint)
{
	logger::write("device", "[%s] %s", __FUNCTION__, mountPoint);
	static auto func = memory::scan("48 89 5C 24 ? 57 48 81 EC ? ? ? ? 44 8A 81 ? ? ? ? 48 8B DA 48 8B F9 48 8B D1 48 8B CB E8 ? ? ? ? 84 C0 74 64 48 8D 4C 24 ? 33 D2 41 B8")
		.as<bool(*)(void*, const char*)>();

	return func(this, mountPoint);
}

void rage::fiDevice::SetPath(const char* path, bool allowRoot, rage::fiDevice* parent)
{
	logger::write("device", "[%s] %s", __FUNCTION__, path);
	static auto func = memory::scan("48 89 5C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 83 64 24 ? ? 49 8B F9")
		.as<void(*)(void*, const char*, bool, rage::fiDevice*)>();

	func(this, path, allowRoot, parent);
}

rage::fiDeviceRelative::fiDeviceRelative()
{
	static auto relativeDeviceVMT = memory::scan("48 8D 05 ? ? ? ? 48 89 03 EB ? 33 DB 48 8D 15 ? ? ? ? 45 33 C9").add(3).rip().as<void*>();
	VMT = relativeDeviceVMT;
	pad[0xF8] = 0;
}

bool rage::fiDeviceRelative::Mount(const char* mountPoint)
{
	return reinterpret_cast<rage::fiDevice*>(this)->Mount(mountPoint);
}

void rage::fiDeviceRelative::SetPath(const char* path, bool allowRoot, rage::fiDevice* parent)
{
	reinterpret_cast<rage::fiDevice*>(this)->SetPath(path, allowRoot, parent);
}

rage::fiDeviceLocal::fiDeviceLocal() {
	pad[0xF8] = 0;
}
rage::fiDeviceLocal::~fiDeviceLocal(){}

std::unordered_map<HANDLE, std::string> handleNames;

HANDLE rage::fiDeviceLocal::Open(const char* fileName, bool readOnly)
{
	logger::write("device", "[%s] %s", __FUNCTION__, fileName);
	HANDLE handle = new std::ifstream(ToFullPath(fileName), std::ios::in | std::ios::binary);
	handleNames[handle] = fileName;
	return handle;
}

uint32_t rage::fiDeviceLocal::ReadFile(HANDLE handle, void* buffer, uint32_t length)
{
	logger::write("device", "[%s] %s %d", __FUNCTION__, handleNames[handle].c_str(), length);
	auto file = (std::ifstream*)handle;
	file->read((char*)buffer, length);
	return file->gcount();
}

uint32_t rage::fiDeviceLocal::ReadBulk(HANDLE handle, uint64_t offset, char* buffer, uint32_t length)
{
	logger::write("device", "[%s] %s %d", __FUNCTION__, handleNames[handle].c_str(), length);
	auto file = (std::ifstream*)handle;
	file->seekg(offset);
	return ReadFile(handle, buffer, length);
}

uint64_t rage::fiDeviceLocal::Seek64(HANDLE handle, int64_t distance, uint32_t method)
{
	logger::write("device", "[%s] %s %d", __FUNCTION__, handleNames[handle].c_str(), distance);
	auto file = (std::ifstream*)handle;
	if (method == SEEK_CUR)
	{
		file->seekg(distance, std::ios::cur);
	}
	else if (method == SEEK_SET)
	{
		file->seekg(distance, std::ios::beg);
	}
	else if (method == SEEK_END)
	{
		file->seekg(-1 * distance, std::ios::end);
	}

	return file->tellg();
}

uint32_t rage::fiDeviceLocal::Close(HANDLE handle)
{
	auto file = (std::ifstream*)handle;
	logger::write("device", "[%s] %s", __FUNCTION__, handleNames[handle].c_str());
	handleNames.erase(handle);
	return true;
}

uint64_t rage::fiDeviceLocal::Size64(HANDLE handle)
{
	uint64_t fileCurrentPos = Seek64(handle, 0, SEEK_CUR);
	uint64_t fileEndPos = Seek64(handle, 0, SEEK_END);
	Seek64(handle, fileCurrentPos, SEEK_SET);
	logger::write("device", "[%s] %s %d", __FUNCTION__, handleNames[handle].c_str(), fileEndPos);
	return fileEndPos;
}

uint64_t rage::fiDeviceLocal::GetFileSize(const char* fileName)
{
	uint64_t size = 0;
	WIN32_FILE_ATTRIBUTE_DATA fileData;
	if (GetFileAttributesExA(ToFullPath(fileName).c_str(), GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &fileData))
		size = fileData.nFileSizeLow | (static_cast<size_t>(fileData.nFileSizeHigh) << 32);

	logger::write("device", "[%s] %s %d", __FUNCTION__, fileName, size);
	return size;
}

uint64_t rage::fiDeviceLocal::GetFileTime(const char* fileName)
{
	WIN32_FILE_ATTRIBUTE_DATA fileData;
	uint64_t filetime = 0;
	if (GetFileAttributesExA(ToFullPath(fileName).c_str(), GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &fileData))
		filetime = *(uint64_t*)&fileData.ftLastWriteTime;

	logger::write("device", "[%s] %s %d", __FUNCTION__, fileName, filetime);
	return filetime;
}

uint32_t rage::fiDeviceLocal::GetAttributes(const char* fileName)
{
	uint32_t attributes = ::GetFileAttributesA(ToFullPath(fileName).c_str());
	if (attributes != INVALID_FILE_ATTRIBUTES)
		logger::write("device", "[%s] %s", __FUNCTION__, fileName);
	return attributes;
}

HANDLE rage::fiDeviceLocal::FindFileBegin(const char* path, rage::fiFindData* findData)
{
	logger::write("device", "[%s]", __FUNCTION__);
	WIN32_FIND_DATAW foundData;

	wchar_t fileName[MAX_PATH] = { 0 };
	auto pathLen = strlen(path);
	bool slashFound = false;
	if (pathLen)
	{
		if (path[pathLen - 1] == '/' || path[pathLen - 1] == '\\')
			slashFound = true;
	}

	wchar_t wcPath[256] = { 0 };
	MultiByteToWideChar(0xFDE9u, 0, ToFullPath(path).c_str(), -1, wcPath, 256);

	wprintf_s(fileName, 256, slashFound ? L"%s*" : L"%s\\*", wcPath);
	HANDLE firstFileHandle = FindFirstFileW(fileName, &foundData);
	if (firstFileHandle == INVALID_HANDLE_VALUE)
		return INVALID_HANDLE_VALUE;

	ConvertFoundData(&foundData, findData);
	return firstFileHandle;
}

bool rage::fiDeviceLocal::FindFileNext(HANDLE handle, rage::fiFindData* findData)
{
	logger::write("device", "[%s]", __FUNCTION__);
	WIN32_FIND_DATAW foundData;
	bool fileFound = FindNextFileW(handle, &foundData);
	if (fileFound)
		ConvertFoundData(&foundData, findData);
	return fileFound;
}

int rage::fiDeviceLocal::FindFileEnd(HANDLE handle)
{
	logger::write("device", "[%s] %s", __FUNCTION__, handleNames[handle].c_str());
	auto file = (std::ifstream*)handle;
	return (FindClose(handle) != 0) - 1;
}

HANDLE rage::fiDeviceLocal::OpenBulk(const char* fileName, uint64_t* ptr)
{
	return Open(fileName, true);
}

HANDLE rage::fiDeviceLocal::OpenBulkWrap(const char* fileName, uint64_t* ptr, void* unk)
{
	return OpenBulk(fileName, ptr);
}

uint32_t rage::fiDeviceLocal::Seek(HANDLE handle, int32_t distance, uint32_t method)
{
	return (uint32_t)Seek64(handle, (uint64_t)distance, method);
}

uint32_t rage::fiDeviceLocal::CloseBulk(HANDLE handle)
{
	return Close(handle);
}

uint32_t rage::fiDeviceLocal::Size(HANDLE handle)
{
	return (uint32_t)Size64(handle);
}

rage::fiDevice* rage::fiDeviceLocal::GetLowLevelDevice()
{
	logger::write("device", "[%s]", __FUNCTION__);
	return this;
}

void* rage::fiDeviceLocal::FixRelativeName(char* dest, int length, const char* source)
{
	logger::write("device", "[%s]", __FUNCTION__);
	memcpy(dest, source, length);
	return dest;
}

bool rage::fiDeviceLocal::SetEndOfFile(HANDLE handle)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return false;
}

bool rage::fiDeviceLocal::SafeRead(HANDLE handle, void* buffer, uint32_t length)
{
	logger::write("device", "[%s]", __FUNCTION__);
	uint32_t offset = 0;
	if (length <= 0)
		return true;
	for (;;)
	{
		uint32_t bytesRead = ReadFile(handle, (char*)buffer + offset, length - offset);
		if (bytesRead < 0)
			break;
		offset += bytesRead;
		if (offset >= length)
			return true;
	}
	return false;
}

bool rage::fiDeviceLocal::SafeWrite(HANDLE handle, const void* buffer, uint32_t length)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return false;
}

HANDLE rage::fiDeviceLocal::CreateLocal(const char* fileName)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 0;
}

HANDLE rage::fiDeviceLocal::Create(const char* fileName)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 0;
}

uint32_t rage::fiDeviceLocal::WriteBulk(HANDLE handle, uint64_t offset, const void* buffer, uint32_t length)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 0;
}

uint32_t rage::fiDeviceLocal::Write(HANDLE hande, const void* buffer, uint32_t length)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 0;
}

uint32_t rage::fiDeviceLocal::Flush(HANDLE handle)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 0;
}

bool rage::fiDeviceLocal::Delete(const char* fileName)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 0;
}

bool rage::fiDeviceLocal::Rename(const char* from, const char* to)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 0;
}

bool rage::fiDeviceLocal::MakeDirectory(const char* dir)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 0;
}

bool rage::fiDeviceLocal::UnmakeDirectory(const char* dir)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 0;
}

void rage::fiDeviceLocal::Sanitize()
{
	logger::write("device", "[%s]", __FUNCTION__);
}

bool rage::fiDeviceLocal::SetFileTime(const char* fileName, uint64_t fileTime)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 0;
}

uint64_t rage::fiDeviceLocal::GetRootDeviceId(const char*)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 1;
}

bool rage::fiDeviceLocal::SetAttributes(const char* fileName, uint32_t attributes)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return false;
}

uint32_t rage::fiDeviceLocal::IsMemoryMappedDevice()
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 2;
}

uint32_t rage::fiDeviceLocal::GetResourceInfo(const char* fileName, rage::fiResourceInfo* flags)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 0;
}

int32_t rage::fiDeviceLocal::IsValidHandle()
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 0;
}

uint32_t rage::fiDeviceLocal::GetBulkOffset(HANDLE handle)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 0;
}

uint32_t rage::fiDeviceLocal::GetPhysicalSortKey(const char*)
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 0x40000000;
}

bool rage::fiDeviceLocal::IsRpf()
{
	logger::write("device", "[%s]", __FUNCTION__);
	return false;
}

uint8_t rage::fiDeviceLocal::GetRpfVersion()
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 0;
}

rage::fiDevice* rage::fiDeviceLocal::GetRpfDevice()
{
	logger::write("device", "[%s]", __FUNCTION__);
	return this;
}

bool rage::fiDeviceLocal::IsCloud()
{
	logger::write("device", "[%s]", __FUNCTION__);
	return false;
}

uint64_t rage::fiDeviceLocal::GetPackfileIndex()
{
	logger::write("device", "[%s]", __FUNCTION__);
	return 0;
}

const char* rage::fiDeviceLocal::GetDebugName()
{
	logger::write("device", "[%s]", __FUNCTION__);
	return "rage::fiDeviceLocal";
}

void rage::fiDeviceLocal::ConvertFoundData(LPWIN32_FIND_DATAW foundDataWin, rage::fiFindData* foundDataRage)
{
	logger::write("device", "[%s]", __FUNCTION__);
	WideCharToMultiByte(0xFDE9u, 0, foundDataWin->cFileName, -1, foundDataRage->fileName, 256, 0i64, 0i64);
	for (uint8_t i = 0; i < strlen(foundDataRage->fileName); ++i)
		if (foundDataRage->fileName[i] == '/')
			foundDataRage->fileName[i] = '\\';
	foundDataRage->lastWriteTime = foundDataWin->ftLastWriteTime;
	foundDataRage->fileAttributes = foundDataWin->dwFileAttributes;
	foundDataRage->fileSize = *(uint64_t*)(&foundDataWin->nFileSizeHigh);
}