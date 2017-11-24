#pragma once

#include <chrono>
#include <string>
#include <thread>

namespace obelisk
{
	namespace platform_utilities
	{
		bool beep(unsigned int frequencyHz, std::chrono::milliseconds time);

		// getULTickCount return the ms count from an inaccurate timer
		// The return type is important, as it allows it to wrap correctly, e.g.
		// auto time = getULTickCount(); .... auto timeTaken = getULTickCount() - time;
		unsigned long getULTickCount();

		// returns tick counter in seconds from accurate system timer (QueryPerformanceCounter)
		double getPreciseTime();

		std::wstring currentOSUsername();

		void outputDebugString(const std::wstring& str);
		void outputDebugString(const std::string& str);

		bool directoryExists(const std::wstring &path);
		bool createDirectory(const std::wstring &path);
		bool isValidFilename(const std::wstring filename);

		std::wstring getMyDocumentsPath();
		std::wstring getLocalAppDataPath(const std::wstring &appName);
		std::wstring getGlobalAppDataPath(const std::wstring &appName);
		std::wstring getExecutablePath();
		std::wstring getExecutableDirectory();

		enum class ThreadPriority{BACKGROUND, NORMAL, HIGH, VERY_HIGH};
		bool setThreadPriority(std::thread &thread, ThreadPriority priority);
		bool setCurrentThreadPriority(ThreadPriority priority);

		bool delayLoadDLL(std::wstring dllname);
	};
}; // end obelisk::
