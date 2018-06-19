#include <stdio.h>
#include <vector>
#include <iostream>

#include "GeneralHelpers.h"
#include "StringFuncs.h"
#include "FormatString.hpp"
#include "ContainerMaker.hpp"

#include "..\KudahLib\AddVectorsWrapper.h"
#include "..\KudahLib\ScopedSetDevice.h"

using namespace kudah;

int main()
{
	const auto setDevice = ScopedCUDASetDevice(0);

	std::vector<int> const a = { 1, 2, 3, 4, 5, 6 };
	std::vector<int> const b = { 10, 20, 30, 40, 50, 60 };

	// Add vectors in parallel
	try
	{
		auto const result = addVectors(a, b);
		if (result.empty())
		{
			fprintf(stderr, "addWithCuda failed!");
			return 1;
		}
		if (result.size() != a.size())
		{
			fprintf(stderr, "addWithCuda returned incorrect size vector");
			return 1;
		}

		auto const aStrings = obelisk::generateVector<std::string>([&a](size_t i) {return std::to_string(a[i]); }, size_t{}, a.size());
		auto const bStrings = obelisk::generateVector<std::string>([&b](size_t i) {return std::to_string(b[i]); }, size_t{}, b.size());
		auto const resultStrings = obelisk::generateVector<std::string>([&result](size_t i) {return std::to_string(result[i]); }, size_t{}, result.size());

		auto const resultMessage = obelisk::formatString("{%s} + {%s} = {%s}", obelisk::stringJoin(aStrings, ",").c_str(),
			obelisk::stringJoin(bStrings, ",").c_str(),
			obelisk::stringJoin(resultStrings, ",").c_str());

		std::cout << resultMessage;
	}
	catch (const std::exception &e)
	{
		std::cout << "Exception: " << e.what() << "\n";
		return 1;
	}

	return 0;
}