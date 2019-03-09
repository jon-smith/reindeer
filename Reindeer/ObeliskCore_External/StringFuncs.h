#pragma once

#include <string>
#include <vector>
#include <chrono>

namespace obelisk
{
	std::string wStrToStr(const std::wstring& wstr);
	std::wstring strToWStr(const std::string& str);

	// Local time in ISO format
	std::string getLocalDateTimeString();
	std::string getLocalDateString();
	std::string getLocalTimeString();
	std::string getLocalTimeStringSuitableForFilename();

	// Format time in #...#d ##h ##m ##s ###ms
	std::string formatTime(std::chrono::duration<double> time, bool includeMillisecond);

	std::vector<std::string> stringSplit(const std::string& str, char delim);
	std::string stringJoin(const std::vector<std::string>& strings, std::string separator = " ");
	std::wstring stringJoin(const std::vector<std::wstring>& strings, std::wstring separator = L" ");

	std::string trimLeft(std::string str);
	std::string trimRight(std::string str);
	std::string trim(std::string str);

	std::string toLower(const std::string &str);
	std::string toUpper(const std::string &str);

	bool startsWith(const std::string& str, const std::string& start);
}
