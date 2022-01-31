#pragma once

namespace rage
{
	class fiFindData;
	class fiResourceInfo;

	class fiDevice
	{
	public:
		virtual ~fiDevice() = default;
		virtual HANDLE Open(const char* fileName, bool readOnly) = 0;
		virtual HANDLE OpenBulk(const char* fileName, uint64_t* ptr) = 0;
		virtual HANDLE OpenBulkWrap(const char* fileName, uint64_t* ptr, void* unk) = 0;
		virtual HANDLE CreateLocal(const char* fileName) = 0;
		virtual HANDLE Create(const char* fileName) = 0;
		virtual uint32_t ReadFile(HANDLE handle, void* buffer, uint32_t toRead) = 0;
		virtual uint32_t ReadBulk(HANDLE handle, uint64_t ptr, void* buffer, uint32_t toRead) = 0;
		virtual uint32_t WriteBulk(HANDLE handle, uint64_t offset, const void* buffer, uint32_t length) = 0;
		virtual uint32_t Write(HANDLE hande, const void* buffer, uint32_t length) = 0;
		virtual uint32_t Seek(HANDLE handle, int32_t distance, uint32_t method) = 0;
		virtual uint64_t Seek64(HANDLE handle, int64_t distance, uint32_t method) = 0;
		virtual uint32_t Close(HANDLE handle) = 0;
		virtual uint32_t CloseBulk(HANDLE handle) = 0;
		virtual uint32_t Size(HANDLE handle) = 0;
		virtual uint64_t Size64(HANDLE handle) = 0;
		virtual uint32_t Flush(HANDLE handle) = 0;
		virtual bool Delete(const char* fileName) = 0;
		virtual bool Rename(const char* from, const char* to) = 0;
		virtual bool MakeDirectory(const char* dir) = 0;
		virtual bool UnmakeDirectory(const char* dir) = 0;
		virtual void Sanitize() = 0;
		virtual uint64_t GetFileSize(const char* fileName) = 0;
		virtual uint64_t GetFileTime(const char* fileName) = 0;
		virtual bool SetFileTime(const char* file, uint64_t fileTime) = 0;
		virtual HANDLE FindFileBegin(const char* path, rage::fiFindData* findData) = 0;
		virtual bool FindFileNext(HANDLE handle, rage::fiFindData* findData) = 0;
		virtual int FindFileEnd(HANDLE handle) = 0;
		virtual rage::fiDevice* GetLowLevelDevice() = 0;
		virtual void* FixRelativeName(char*, int, const char*) = 0;
		virtual bool SetEndOfFile(HANDLE handle) = 0;
		virtual uint32_t GetAttributes(const char* path) = 0;
		virtual uint64_t GetRootDeviceId(const char*) = 0;
		virtual bool SetAttributes(const char* file, uint32_t attributes) = 0;
		virtual uint32_t IsMemoryMappedDevice() = 0;
		virtual bool SafeRead(HANDLE handle, void* buffer, uint32_t length) = 0;
		virtual bool SafeWrite(HANDLE handle, const void* buffer, uint32_t length) = 0;
		virtual uint32_t GetResourceInfo(const char* fileName, fiResourceInfo* flags) = 0;
		virtual int32_t IsValidHandle() = 0;
		virtual uint32_t GetBulkOffset(HANDLE handle) = 0;
		virtual uint32_t GetPhysicalSortKey(const char*) = 0;
		virtual bool IsRpf() = 0;
		virtual uint8_t GetRpfVersion() = 0;
		virtual fiDevice* GetRpfDevice() = 0;
		virtual bool IsCloud() = 0;
		virtual uint64_t GetPackfileIndex() = 0;
		virtual const char* GetDebugName() = 0;

		bool Mount(const char* mountPoint);
		void SetPath(const char* path, bool allowRoot, rage::fiDevice* parent);
	};

	class __declspec(novtable) fiDeviceRelative
	{
		void* VMT;
		char pad[0x108];
	public:
		fiDeviceRelative();

		bool Mount(const char* mountPoint);
		void SetPath(const char* path, bool allowRoot, rage::fiDevice* parent);
	};
}