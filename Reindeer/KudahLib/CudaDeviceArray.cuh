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

		// Initialise from ptr
		CudaDeviceArray(const T *sourcePtr, size_t n) :
			CudaDeviceArray(n)
		{
			if (length != 0)
			{
				// Copy contents
				auto const cpyStatus = cudaMemcpy(dataPtr, reinterpret_cast<const void*>(sourcePtr), length * sizeof(T), cudaMemcpyHostToDevice);

				if (cpyStatus != cudaSuccess)
				{
					auto const message = obelisk::formatString("Failed to copy host to device memory, status code: %d", cpyStatus);
					// Deallocate previously allocated memory before we throw
					dealloc();
					throw std::exception(message.c_str());
				}
			}
		}

		CudaDeviceArray(const std::vector<T> &vec) :
			CudaDeviceArray(vec.data(), vec.size())
		{
		}

		template <size_t ArraySize>
		CudaDeviceArray(const std::array<T, ArraySize> &array) :
			CudaDeviceArray(array.data(), ArraySize)
		{
		}

		// Copy device memory back to std::vector
		std::vector<T> getAsVector() const
		{
			std::vector<T> vec;
			copyToVector(vec);
			return vec;
		}

		void copyToVector(std::vector<T> &vec) const
		{
			vec.resize(length);
			if (length != 0)
			{
				auto const cpyStatus = cudaMemcpy(vec.data(), dataPtr, length * sizeof(T), cudaMemcpyDeviceToHost);
				if (cpyStatus != cudaSuccess)
				{
					auto const message = obelisk::formatString("Failed to copy from device to host, status code: %d", cpyStatus);
					throw std::exception(message.c_str());
				}
			}
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