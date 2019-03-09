#include "PlatformSpecific.h"

#include <type_traits>
#include <set>
#include <filesystem>

#define NOMINMAX
#include <windows.h>
#include <debugapi.h>
#include <Lmcons.h>
#include <Shlobj.h>
#include <tlhelp32.h>

#include "StringFuncs.h"

namespace obelisk
{
	namespace platform_utilities
	{
		bool beep(unsigned int frequencyHz, std::chrono::milliseconds time)
		{
			return Beep(static_cast<DWORD>(frequencyHz), static_cast<DWORD>(time.count())) == TRUE;
		}

		unsigned long getULTickCount()
		{
			static_assert(std::is_same<std::result_of<decltype(&timeGetTime)()>::type, unsigned long>::value, "getULTickCount not implemented with expected return value");
			return GetTickCount();
		}

		double getPreciseTime()
		{
			int64_t freq, time;

			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time));
			QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&freq));

			return double(time) / double(freq);
		}

		std::wstring currentOSUsername()
		{
			wchar_t username[UNLEN + 1];
			DWORD username_len = UNLEN + 1;
			GetUserNameW(username, &username_len);
			return username;
		}

		void outputDebugString(const std::wstring& str)
		{
			OutputDebugStringW(std::wstring(str + L"\n").c_str());
		}

		void outputDebugString(const std::string& str)
		{
			OutputDebugStringA(std::string(str + "\n").c_str());
		}

		bool directoryExists(const std::wstring &path)
		{
			DWORD dwAttrib = GetFileAttributesW(path.c_str());

			return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
				(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
		}

		bool createDirectory(const std::wstring &path)
		{
			return SHCreateDirectoryExW(nullptr, path.c_str(), nullptr) == ERROR_SUCCESS;
		}

		bool isValidFilename(const std::wstring filename)
		{
			// Make sure string and wstring are the same
			if (filename != strToWStr(wStrToStr(filename)))
				return false;

			if (filename.empty())
				return false;

			// Contains invalid characters
			if (filename.find_first_of(L"<>:\"/\\|?*") != std::wstring::npos)
				return false;

			// Ends with a fullstop
			if (*filename.crbegin() == L'.')
				return false;

			// Reserved filenames
			static const std::set<std::wstring> reservedFilenames =
			{ L"CON",L"PRN", L"AUX", L"NUL", L"COM1", L"COM2", L"COM3", L"COM4", L"COM5", L"COM6", L"COM7", L"COM8", L"COM9",
				L"LPT1", L"LPT2", L"LPT3", L"LPT4", L"LPT5", L"LPT6", L"LPT7", L"LPT8", L"LPT9" };

			auto const path = std::experimental::filesystem::path(filename);
			if (reservedFilenames.find(path.stem()) != reservedFilenames.end())
				return false;

			return true;
		}

		std::wstring getMyDocumentsPath()
		{
			wchar_t myDocuments[MAX_PATH];
			HRESULT result = SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, myDocuments);

			if (result != S_OK)
			{
				return{};
			}

			return myDocuments;
		}

		std::wstring getLocalAppDataPath(const std::wstring &appName)
		{
			wchar_t appData[MAX_PATH];
			HRESULT result = SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, appData);

			if (result != S_OK)
			{
				return{};
			}

			// Attempt to create an app folder if not present
			auto const fullPath = std::wstring(appData) + L"/" + appName + L"/";
			if (!directoryExists(fullPath))
				if (!createDirectory(fullPath))
				{
					return{};
				}

			return fullPath;
		}

		std::wstring getGlobalAppDataPath(const std::wstring &appName)
		{
			wchar_t appData[MAX_PATH];
			HRESULT result = SHGetFolderPathW(nullptr, CSIDL_COMMON_APPDATA, nullptr, SHGFP_TYPE_CURRENT, appData);

			if (result != S_OK)
			{
				return{};
			}

			// Attempt to create an app folder if not present
			auto const fullPath = std::wstring(appData) + L"/" + appName + L"/";
			if (!directoryExists(fullPath))
				if (!createDirectory(fullPath))
				{
					return{};
				}

			return fullPath;
		}

		std::wstring getExecutablePath()
		{
			wchar_t path[MAX_PATH];
			GetModuleFileNameW(nullptr, path, MAX_PATH);
			return path;
		}

		std::wstring getExecutableDirectory()
		{
			return std::experimental::filesystem::path(getExecutablePath()).parent_path();
		}

		bool setThreadPriority(std::thread &thread, ThreadPriority priority)
		{
			auto nativeHandle = thread.native_handle();
			switch (priority)
			{
			case ThreadPriority::BACKGROUND:
				return SetThreadPriority(nativeHandle, THREAD_PRIORITY_LOWEST) != FALSE;
			case ThreadPriority::NORMAL:
				return SetThreadPriority(nativeHandle, THREAD_PRIORITY_NORMAL) != FALSE;
			case ThreadPriority::HIGH:
				return SetThreadPriority(nativeHandle, THREAD_PRIORITY_HIGHEST) != FALSE;
			case ThreadPriority::VERY_HIGH:
				return SetThreadPriority(nativeHandle, THREAD_PRIORITY_TIME_CRITICAL) != FALSE;
			default:
				return false;
			}
		}

		bool setCurrentThreadPriority(ThreadPriority priority)
		{
			switch (priority)
			{
			case ThreadPriority::BACKGROUND:
				return SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST) != FALSE;
			case ThreadPriority::NORMAL:
				return SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL) != FALSE;
			case ThreadPriority::HIGH:
				return SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST) != FALSE;
			case ThreadPriority::VERY_HIGH:
				return SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) != FALSE;
			default:
				return false;
			}
		}

		// From https://msdn.microsoft.com/en-us/library/ms686701(v=VS.85).aspx
		// https://stackoverflow.com/questions/3749668/how-to-query-the-thread-count-of-a-process-using-the-regular-windows-c-c-apis
		unsigned getCurrentProcessThreadCount()
		{
			// first determine the id of the current process
			DWORD const  id = GetCurrentProcessId();

			// then get a process list snapshot.
			HANDLE const  snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);

			// initialize the process entry structure.
			PROCESSENTRY32 entry = { 0 };
			entry.dwSize = sizeof(entry);

			// get the first process info.
			BOOL ret = Process32First(snapshot, &entry);
			while (ret && entry.th32ProcessID != id) {
				ret = Process32Next(snapshot, &entry);
			}
			CloseHandle(snapshot);

			return ret ? entry.cntThreads : 0;
		}

		bool delayLoadDLL(std::wstring dllname)
		{
			return ::LoadLibraryW(dllname.c_str()) != nullptr;
		}
	}
}
