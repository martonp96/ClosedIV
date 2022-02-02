#pragma once

namespace rage
{
	struct fiFindData
	{
		char fileName[256];
		uint64_t fileSize;
		FILETIME lastWriteTime;
		DWORD fileAttributes;
	};

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
		virtual uint32_t ReadBulk(HANDLE handle, uint64_t ptr, char* buffer, uint32_t toRead) = 0;
		virtual uint32_t WriteBulk(HANDLE handle, uint64_t offset, const void* buffer, uint32_t length) = 0;
		virtual uint32_t Write(HANDLE handle, const void* buffer, uint32_t length) = 0;
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

	class fiDeviceLocal : public fiDevice
	{
		char pad[0x120];
	public:
		fiDeviceLocal();
		~fiDeviceLocal();
		HANDLE Open(const char* fileName, bool readOnly) override;
		HANDLE OpenBulk(const char* fileName, uint64_t* ptr) override;
		HANDLE OpenBulkWrap(const char* fileName, uint64_t* ptr, void* unk) override;
		HANDLE CreateLocal(const char* fileName) override;
		HANDLE Create(const char* fileName) override;
		uint32_t ReadFile(HANDLE handle, void* buffer, uint32_t length) override;
		uint32_t ReadBulk(HANDLE handle, uint64_t offset, char* buffer, uint32_t length) override;
		uint32_t WriteBulk(HANDLE handle, uint64_t offset, const void* buffer, uint32_t length) override;
		uint32_t Write(HANDLE hande, const void* buffer, uint32_t length) override;
		uint32_t Seek(HANDLE handle, int32_t distance, uint32_t method) override;
		uint64_t Seek64(HANDLE handle, int64_t distance, uint32_t method) override;
		uint32_t Close(HANDLE handle) override;
		uint32_t CloseBulk(HANDLE handle) override;
		uint32_t Size(HANDLE handle) override;
		uint64_t Size64(HANDLE handle) override;
		uint32_t Flush(HANDLE handle) override;
		bool Delete(const char* fileName) override;
		bool Rename(const char* from, const char* to) override;
		bool MakeDirectory(const char* dir) override;
		bool UnmakeDirectory(const char* dir) override;
		void Sanitize() override;
		uint64_t GetFileSize(const char* fileName) override;
		uint64_t GetFileTime(const char* filfileNamee) override;
		bool SetFileTime(const char* fileName, uint64_t fileTime) override;
		HANDLE FindFileBegin(const char* path, rage::fiFindData* findData) override;
		bool FindFileNext(HANDLE handle, rage::fiFindData* findData) override;
		int FindFileEnd(HANDLE handle) override;
		rage::fiDevice* GetLowLevelDevice() override;
		void* FixRelativeName(char*, int, const char*) override;
		bool SetEndOfFile(HANDLE handle) override;
		uint32_t GetAttributes(const char* fileName) override;
		uint64_t GetRootDeviceId(const char*) override;
		bool SetAttributes(const char* fileName, uint32_t attributes) override;
		uint32_t IsMemoryMappedDevice() override;
		bool SafeRead(HANDLE handle, void* buffer, uint32_t length) override;
		bool SafeWrite(HANDLE handle, const void* buffer, uint32_t length) override;
		uint32_t GetResourceInfo(const char* fileName, rage::fiResourceInfo* flags) override;
		int32_t IsValidHandle() override;
		uint32_t GetBulkOffset(HANDLE handle) override;
		uint32_t GetPhysicalSortKey(const char*) override;
		bool IsRpf() override;
		uint8_t GetRpfVersion() override;
		rage::fiDevice* GetRpfDevice() override;
		bool IsCloud() override;
		uint64_t GetPackfileIndex() override;
		const char* GetDebugName() override;

		void ConvertFoundData(LPWIN32_FIND_DATAW foundDataWin, rage::fiFindData* foundDataRage);

		static std::string ToFullPath(const char* fileName)
		{
			std::filesystem::path cwd = std::filesystem::current_path() / "newmods/";

			std::string fName(fileName);

			auto pos = fName.find(":/");
			if (pos != std::string::npos)
				fName.erase(pos, 1);

			cwd /= fName;

			cwd.make_preferred();

			return cwd.string();
		}
	};
}