#pragma once
#include <cstdint>
#include <vector>
#include <functional>
#include "log.h"

class memory {
public:
	uintptr_t address;
	memory(memory& other) : address(other.address) {}
	memory(uintptr_t offset, bool useBase = false) : address(useBase ? base() + offset : offset) {}

	struct InitFuncs
	{
		std::function<void()> fn;
		static std::vector<InitFuncs>& funcs()
		{
			static std::vector<InitFuncs> _funcs;
			return _funcs;
		}

		static void run()
		{
			for (auto& cb : funcs())
			{
				cb.fn();
			}
		}

		InitFuncs(const std::function<void()>& fn) : fn(fn)
		{
			funcs().push_back(*this);
		}
	};

    inline static uintptr_t& base()
	{
		static uintptr_t _base;
		return _base;
	}

	inline static memory& virtual_mem()
	{
		static memory _virtualmem(0, false);
		return _virtualmem;
	}

	inline static memory get_virtual_mem(size_t size)
	{
		static memory current = virtual_mem();

		while (current.address % 16) {
			current.address += 1;
		}

		memory res(current);
		current.address += size;
		return res;
	}

	inline static void init()
	{
		memory::base() = (uintptr_t)GetModuleHandle(NULL);
		memory::virtual_mem() = memory((uintptr_t)VirtualAlloc((void*)(base() + 0x20000000), 1024, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE), false);
	}

	template <typename T>
	inline T as()
	{
		return (T)(address);
	}

	template <typename T>
	inline memory add(T offset)
	{
		return memory(address + offset);
	}

	inline memory rip()
	{
		return memory(address + *(int32_t*)address + 4);
	}

	inline void nop(size_t len)
	{
		scoped_unlock lock(address, len);
		memset((void*)address, 0x90, len);
	}

	inline void ret()
	{
		put<uint8_t>(0xC3);
	}

	inline void make_jmp(uintptr_t func)
	{
		memory(address, false).put<uint16_t>(0xB848);
		memory(address + 2, false).put<uintptr_t>(func);
		memory(address + 10, false).put<uint16_t>(0xE0FF);
	}

	inline void make_jmp_ret(void* func)
	{
		make_jmp_ret((uintptr_t)func);
	}

	inline void make_jmp_ret(uintptr_t func)
	{
		memory(address, false).put<uint16_t>(0xB848);
		memory(address + 2, false).put<uintptr_t>(func);
		memory(address + 10, false).put<uint16_t>(0xC350);
	}

	inline void make_call(uintptr_t func)
	{
		put<uint8_t>(0xE8);
		memory(address + 1, false).put(int32_t(func - address - 5));
	}

	inline void set_call(void* func, bool ret = false)
	{
		memory jmpMem = get_virtual_mem(12);

		if(ret)
			jmpMem.make_jmp_ret((uintptr_t)func);
		else
			jmpMem.make_jmp((uintptr_t)func);

		make_call(jmpMem.address);
	}

	template<typename T>
	inline void put(const T& value)
	{
		scoped_unlock lock(address, sizeof(T));
		memcpy((void*)address, &value, sizeof(T));
	}

	template <typename R = void*, typename ...Args>
	inline R call(Args... args)
	{
		return ((R(*)(Args...))address)(args...);
	}

	static BOOL HookIAT(const char* szModuleName, const char* szFuncName, PVOID pNewFunc, PVOID* pOldFunc)
	{//https://guidedhacking.com/threads/how-to-hook-import-address-table-iat-hooking.13555/
		#define PtrFromRva( base, rva ) ( ( ( PBYTE ) base ) + rva )

		PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)GetModuleHandle(NULL);
		PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)PtrFromRva(pDosHeader, pDosHeader->e_lfanew);

		if (pNtHeader->Signature != IMAGE_NT_SIGNATURE)
			return FALSE;

		PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)PtrFromRva(pDosHeader, pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		for (UINT uIndex = 0; pImportDescriptor[uIndex].Characteristics != 0; uIndex++)
		{
			char* szDllName = (char*)PtrFromRva(pDosHeader, pImportDescriptor[uIndex].Name);

			if (_strcmpi(szDllName, szModuleName) != 0)
				continue;

			if (!pImportDescriptor[uIndex].FirstThunk || !pImportDescriptor[uIndex].OriginalFirstThunk)
				return FALSE;

			PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)PtrFromRva(pDosHeader, pImportDescriptor[uIndex].FirstThunk);
			PIMAGE_THUNK_DATA pOrigThunk = (PIMAGE_THUNK_DATA)PtrFromRva(pDosHeader, pImportDescriptor[uIndex].OriginalFirstThunk);

			for (; pOrigThunk->u1.Function != NULL; pOrigThunk++, pThunk++)
			{
				if (pOrigThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
					continue;

				PIMAGE_IMPORT_BY_NAME import = (PIMAGE_IMPORT_BY_NAME)PtrFromRva(pDosHeader, pOrigThunk->u1.AddressOfData);

				if (_strcmpi(szFuncName, (char*)import->Name) != 0)
					continue;

				DWORD dwJunk = 0;
				MEMORY_BASIC_INFORMATION mbi;

				VirtualQuery(pThunk, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
				if (!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &mbi.Protect))
					return FALSE;

				*pOldFunc = (PVOID*)(DWORD_PTR)pThunk->u1.Function;

				pThunk->u1.Function = (ULONGLONG)(DWORD_PTR)pNewFunc;

				if (VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &dwJunk))
					return TRUE;
			}
		}
		return FALSE;
	}

	static memory scan(const char* signature)
	{
		static auto pattern_to_byte = [](const char* pattern) {
			auto bytes = std::vector<int>{};
			auto start = const_cast<char*>(pattern);
			auto end = const_cast<char*>(pattern) + strlen(pattern);

			for (auto current = start; current < end; ++current) {
				if (*current == '?') {
					++current;
					if (*current == '?')
						++current;
					bytes.push_back(-1);
				}
				else {
					bytes.push_back(strtoul(current, &current, 16));
				}
			}
			return bytes;
		};

		auto moduleBase = base();

		auto dosHeader = (PIMAGE_DOS_HEADER)moduleBase;
		auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)moduleBase + dosHeader->e_lfanew);

		constexpr auto offs = offsetof(IMAGE_DOS_HEADER, IMAGE_DOS_HEADER::e_lfanew);

		auto sizzz = sizeof(IMAGE_DOS_HEADER);

		auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
		auto patternBytes = pattern_to_byte(signature);
		auto scanBytes = reinterpret_cast<std::uint8_t*>(moduleBase);

		auto s = patternBytes.size();
		auto d = patternBytes.data();

		for (auto i = 0ul; i < sizeOfImage - s; ++i) {
			bool found = true;
			for (auto j = 0ul; j < s; ++j) {
				if (scanBytes[i + j] != d[j] && d[j] != -1) {
					found = false;
					break;
				}
			}
			if (found) {
				return memory((uintptr_t)&scanBytes[i], false);
			}
		}

		logger::write("info", "!! Pattern %s not found!", signature);
		return memory(0, false);
	}

private:
	struct scoped_unlock
	{
		DWORD rights;
		size_t len;
		void* addr;

		scoped_unlock(uint64_t _addr, size_t _len) :addr((void*)_addr), len(_len)
		{
			VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &rights);
		}

		~scoped_unlock()
		{
			VirtualProtect(addr, len, rights, NULL);
		}
	};
};