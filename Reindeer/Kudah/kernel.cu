#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <vector>
#include <iostream>

#include "GeneralHelpers.h"
#include "StringFuncs.h"
#include "FormatString.hpp"
#include "ContainerMaker.hpp"

#include "..\KudahLib\AddVectorsWrapper.h"

using namespace kudah;

int main()
{
	// Choose which GPU to run on, change this on a multi-GPU system.
	auto const setDeviceStatus = cudaSetDevice(0);
	if (setDeviceStatus != cudaSuccess)
	{
		fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
		return {};
	}
	
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
	catch (...)
	{
		fprintf(stderr, "addWithCuda threw an exception");
		return 1;
	}

	// cudaDeviceReset must be called before exiting in order for profiling and
	// tracing tools such as Nsight and Visual Profiler to show complete traces.
	auto const cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess)
	{
		fprintf(stderr, "cudaDeviceReset failed!");
		return 1;
	}

	return 0;
}