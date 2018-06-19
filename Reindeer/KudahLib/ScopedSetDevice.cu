#include "ScopedSetDevice.h"

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include "FormatString.hpp"
#include "PlatformSpecific.h"

#include <exception>
#include <stdio.h>

using namespace kudah;

ScopedCUDASetDevice::ScopedCUDASetDevice(unsigned device)
{
	auto const setDeviceStatus = cudaSetDevice(static_cast<int>(device));
	if (setDeviceStatus != cudaSuccess)
	{
		const auto message = obelisk::formatString("cudaSetDevice(%d) failed with code %d", device, setDeviceStatus);
		obelisk::platform_utilities::outputDebugString(message);
		throw std::exception(message.c_str());
	}
}
ScopedCUDASetDevice::~ScopedCUDASetDevice()
{
	if (!doDeviceReset)
		return;

	// cudaDeviceReset must be called before exiting in order for profiling and
	// tracing tools such as Nsight and Visual Profiler to show complete traces.
	auto const cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess)
	{
		obelisk::platform_utilities::outputDebugString("cudaDeviceReset failed");
	}
}

ScopedCUDASetDevice::ScopedCUDASetDevice(ScopedCUDASetDevice &&o)
{
	// Just prevent the other class's destructor for doing the device reset
	o.doDeviceReset = false;
}
