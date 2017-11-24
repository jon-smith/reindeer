#pragma once

#include "cuda_runtime.h"
#include <vector>

#include "PlatformSpecific.h"
#include "FormatString.hpp"

namespace kudah
{
	// RAII CUDA device array class
	template <typename T>
	class CudaDeviceArray
	{
	public:
		// Initialise empty array of size 'length'
		CudaDeviceArray(size_t length) :
			length(length)
		{
			if (length != 0)
			{
				auto const cudaStatus = cudaMalloc((void**)&dataPtr, length * sizeof(T));
				if (cudaStatus != cudaSuccess)
				{
					auto const message = obelisk::formatString("Failed to allocate device memory, status code: %d", cudaStatus);
					throw std::exception(message.c_str());
				}
			}
		}

		// Initialise from vector
		CudaDeviceArray(const std::vector<T> &vec) :
			length(vec.size())
		{
			if (length != 0)
			{
				// Allocate empty memory
				auto const mallocStatus = cudaMalloc((void**)&dataPtr, length * sizeof(T));
				if (mallocStatus != cudaSuccess)
				{
					auto const message = obelisk::formatString("Failed to allocate device memory, status code: %d", mallocStatus);
					throw std::exception(message.c_str());
				}
				// Copy contents
				auto const cpyStatus = cudaMemcpy(dataPtr, reinterpret_cast<const void*>(vec.data()), length * sizeof(T), cudaMemcpyHostToDevice);

				if (cpyStatus != cudaSuccess)
				{
					auto const message = obelisk::formatString("Failed to copy host to device memory, status code: %d", cpyStatus);
					// Deallocate previously allocated memory before we throw
					dealloc();
					throw std::exception(message.c_str());
				}
			}
		}

		// Copy device memory back to std::vector
		std::vector<T> getAsVector() const
		{
			std::vector<T> vec(length);
			if (length != 0)
			{
				auto const cpyStatus = cudaMemcpy(vec.data(), dataPtr, length * sizeof(T), cudaMemcpyDeviceToHost);
				if (cpyStatus != cudaSuccess)
				{
					auto const message = obelisk::formatString("Failed to copy from device to host, status code: %d", cpyStatus);
					throw std::exception(message.c_str());
				}
			}
			return vec;
		}

		~CudaDeviceArray()
		{
			dealloc();
		}

		size_t size() const
		{
			return length;
		}

		T *data() const
		{
			return dataPtr;
		}

	private:

		void dealloc()
		{
			if (dataPtr)
			{
				auto const status = cudaFree(dataPtr);
				if (status != cudaSuccess)
				{
					auto const message = obelisk::formatString("Failed to deallocate device memory, status code: %d", status);
					obelisk::platform_utilities::outputDebugString(message);
				}
				dataPtr = nullptr;
			}
		}

		T *dataPtr = nullptr;
		size_t length = 0;
	};
}