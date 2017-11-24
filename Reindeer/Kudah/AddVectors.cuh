#pragma once

#include "CudaDeviceArray.cuh"
#include "device_launch_parameters.h"

namespace kudah
{
	namespace impl
	{
		__global__ void addKernel(int *c, const int *a, const int *b)
		{
			int i = threadIdx.x;
			c[i] = a[i] + b[i];
		}
	}

	template <typename T>
	std::vector<T> addVectors(const std::vector<T> &a, const std::vector<T> &b)
	{
		if (a.size() != b.size())
			throw std::invalid_argument("addVectors called with inconsistent size vectors");

		// Allocate GPU buffers for three vectors (two input, one output)
		CudaDeviceArray<T> deviceA(a);
		CudaDeviceArray<T> deviceB(b);
		CudaDeviceArray<T> deviceResult(a.size());

		// Launch a kernel on the GPU with one thread for each element.
		impl::addKernel<<<1, static_cast<unsigned>(a.size()) >>>(deviceResult.data(), deviceA.data(), deviceB.data());

		// Check for any errors launching the kernel
		auto const postKernelStatus = cudaGetLastError();
		if (postKernelStatus != cudaSuccess)
		{
			throw std::exception(obelisk::formatString("Kernel launch failed: %s", cudaGetErrorString(postKernelStatus)).c_str());
		}

		// cudaDeviceSynchronize waits for the kernel to finish, and returns
		auto const deviceSyncStatus = cudaDeviceSynchronize();
		if (deviceSyncStatus != cudaSuccess)
		{
			throw std::exception(obelisk::formatString("cudaDeviceSynchronize returned error code %d after launching kernel", deviceSyncStatus).c_str());
		}

		// Copy output vector from GPU buffer to host memory.
		return deviceResult.getAsVector();
	}
}