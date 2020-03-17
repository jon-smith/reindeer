#include "StringFuncs.h"

#include <locale>
#include <codecvt>
#include <stdarg.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <cassert>

#include "FormatString.hpp"

// Disable warnings/errors about deprecated conversion
#pragma warning(disable: 4996)

namespace obelisk
{
	std::string wStrToStr(const std::wstring& wStr)
	{
		// Attempt both utf8 and utf16 conversion	
		try {
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> utf8Converter;
			return utf8Converter.to_bytes(wStr);
		}
		catch (...)
		{
			try {
				std::wstring_convert<std::codecvt_utf16<wchar_t>, wchar_t> utf16Converter;
				return utf16Converter.to_bytes(wStr);
			}
			catch (...)
			{
				assert(false);
				return "[wstrToStr conversion failed]";
			}
		}
	}

	std::wstring strToWStr(const std::string& str)
	{
		// Attempt both utf8 and utf16 conversion	
		try {
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> utf8Converter;
			return utf8Converter.from_bytes(str);
		}
		catch (...)
		{
			try {
				std::wstring_convert<std::codecvt_utf16<wchar_t>, wchar_t> utf16Converter;
				return utf16Converter.from_bytes(str);
			}
			catch (...)
			{
				return std::wstring(str.begin(), str.end());
			}
		}
	}

	std::tm localTimeNow()
	{
		std::time_t t = std::time(nullptr);
		std::tm now;
		localtime_s(&now, &t);
		return now;
	}

	std::string getLocalDateTimeString()
	{
		auto now = localTimeNow();

		std::stringstream ss;
		// Format is ISO date, ISO time, time zone
		ss << std::put_time(&now, "%Y-%m-%d %H:%M:%S %Z");
		return ss.str();
	}

	std::string getLocalDateString()
	{
		auto now = localTimeNow();

		std::stringstream ss;
		// Format is ISO date
		ss << std::put_time(&now, "%Y-%m-%d");
		return ss.str();
	}

	std::string getLocalTimeString()
	{
		auto now = localTimeNow();

		std::stringstream ss;
		// Format is ISO date
		ss << std::put_time(&now, "%H:%M:%S");
		return ss.str();
	}

	std::string getLocalTimeStringSuitableForFilename()
	{
		auto now = localTimeNow();

		std::stringstream ss;
		// Format is ISO date, ISO time (no colons)
		ss << std::put_time(&now, "%Y-%m-%d_%Hh%Mm%Ss");
		return ss.str();
	}

	std::string formatTime(std::chrono::duration<double> time, bool includeMillisecond)
	{
		auto const days = static_cast<uint32_t>(time.count() / 60 / 60 / 24);
		auto const remainderAfterDays = time - std::chrono::duration<double>(days * 60 * 60 * 24);
		auto const hours = static_cast<uint32_t>(remainderAfterDays.count() / 60 / 60);
		auto const remainderAfterHours = remainderAfterDays - std::chrono::duration<double>(hours * 60 * 60);
		auto const minutes = static_cast<uint32_t>(remainderAfterHours.count() / 60);
		auto const remainderAfterMinutes = remainderAfterHours - std::chrono::duration<double>(minutes * 60);
		auto const seconds = static_cast<uint32_t>(remainderAfterMinutes.count());
		auto const remainderAfterSeconds = remainderAfterMinutes - std::chrono::duration<double>(seconds);
		auto const milliseconds = static_cast<uint32_t>(round(remainderAfterSeconds.count() * 1000));

		if (days != 0)
		{
			if (includeMillisecond)
				return formatString("%dd %dh %dm %ds %dms", days, hours, minutes, seconds, milliseconds);

			return formatString("%dd %dh %dm %ds", days, hours, minutes, seconds);
		}

		if (hours != 0)
		{
			if (includeMillisecond)
				return formatString("%dh %dm %ds %dms", hours, minutes, seconds, milliseconds);

			return formatString("%dh %dm %ds", hours, minutes, seconds);
		}

		if (minutes != 0)
		{
			if (includeMillisecond)
				return formatString("%dm %ds %dms", minutes, seconds, milliseconds);

			return formatString("%dm %ds", minutes, seconds);
		}

		if (seconds != 0)
		{
			if (includeMillisecond)
				return formatString("%ds %dms", seconds, milliseconds);

			return formatString("%ds", seconds);
		}

		if (includeMillisecond)
			return formatString("%dms", milliseconds);

		return "0s";
	}

	std::vector<std::string> stringSplit(const std::string& str, char delim)
	{
		std::vector<std::string> r;

		std::stringstream ss;
		ss.str(str);
		std::string item;
		while (getline(ss, item, delim))
		{
			r.push_back(item);
		}

		return r;
	}

	std::string stringJoin(const std::vector<std::string>& strings, std::string separator)
	{
		std::string joined;
		for (size_t i = 0; i < strings.size(); ++i)
		{
			joined += strings[i];
			if (i != strings.size() - 1)
				joined += separator;
		}
		return joined;
	}

	std::wstring stringJoin(const std::vector<std::wstring>& strings, std::wstring separator)
	{
		std::wstring joined;
		for (size_t i = 0; i < strings.size(); ++i)
		{
			joined += strings[i];
			if (i != strings.size() - 1)
				joined += separator;
		}
		return joined;
	}

	// Functions below from http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring?rq=1
	std::string trimLeft(std::string str)
	{
		str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
			return !std::isspace(ch);
		}));

		return str;
	}

	std::string trimRight(std::string str)
	{
		str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) {
			return !std::isspace(ch);
		}).base(), str.end());

		return str;
	}

	std::string trim(std::string str)
	{
		return trimLeft(trimRight(str));
	}

	std::string toLower(const std::string &str)
	{
		std::string lower;
		std::transform(str.begin(), str.end(), std::back_inserter(lower), [](auto c) {return tolower(c); });
		return lower;
	}

	std::string toUpper(const std::string &str)
	{
		std::string upper;
		std::transform(str.begin(), str.end(), std::back_inserter(upper), [](auto c) {return toupper(c); });
		return upper;
	}

	bool startsWith(const std::string& str, const std::string& start)
	{
		return str.length() >= start.length() && str.substr(0, start.length()) == start;
	}
}