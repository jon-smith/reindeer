#pragma once

#include <vector>
#include "DistributionFunctions.hpp"

namespace obelisk
{
	namespace Impl
	{
		template <typename T>
		struct FloatIfFloatOtherwiseDouble
		{
			using Type = double;
		};

		template <>
		struct FloatIfFloatOtherwiseDouble<float>
		{
			using Type = float;
		};
	}

	/// 3x3 box blur implementation (Naive)
	template <typename T>
	void boxBlur3x3(std::vector<T> &inputOutput, size_t width)
	{
		using CastType = typename Impl::FloatIfFloatOtherwiseDouble<T>::Type;

		if (inputOutput.empty())
			return;

		if (width == 0)
			throw std::invalid_argument("box blur input width cannot be zero");

		if (inputOutput.size() % width != 0)
			throw std::invalid_argument("box blur input size is incompatible with width");

		auto const height = inputOutput.size() / width;

		auto tmp = inputOutput;

		constexpr auto half = static_cast<CastType>(0.5);
		constexpr auto reciprocalThree = static_cast<CastType>(1.0 / 3.0);

		// Blur horizontally (inputOutput into tmp)
		// First and last columns use average of two
		for (size_t r = 0; r < height; ++r)
		{
			auto const firstColumnIndex = r*width;
			tmp[firstColumnIndex] = static_cast<T>(half*
				static_cast<CastType>
				(inputOutput[firstColumnIndex] +
					inputOutput[firstColumnIndex + 1]));

			auto const lastColumnIndex = r*width + height-1;
			tmp[lastColumnIndex] = static_cast<T>(half*
				static_cast<CastType>
				(inputOutput[lastColumnIndex] +
					inputOutput[lastColumnIndex - 1]));
		}
		// All other columns use all three
		for (size_t r = 0; r < height; ++r)
			for (size_t c = 1; c < width-1; ++c)
			{
				auto const index = r*width + c;
				tmp[index] = static_cast<T>(reciprocalThree*
					static_cast<CastType>
					(inputOutput[index - 1] +
					inputOutput[index] +
					inputOutput[index + 1]));
			}

		// Blur vertically (tmp into inputOutput)
		// First + last row
		for (size_t c = 0; c < width; ++c)
		{
			auto const firstRowIndex = c;
			inputOutput[firstRowIndex] = static_cast<T>(half*
				static_cast<CastType>
				(tmp[firstRowIndex] +
					tmp[firstRowIndex + width]));

			auto const lastRowIndex = (width-1)*width + c;
			inputOutput[lastRowIndex] = static_cast<T>(half*
				static_cast<CastType>
				(tmp[lastRowIndex] +
					tmp[lastRowIndex - width]));
		}
		// Middle rows
		for (size_t r = 1; r < height - 1; ++r)
			for (size_t c = 0; c < width; ++c)
			{
				auto const index = r*width + c;
				inputOutput[index] = static_cast<T>(reciprocalThree*
					static_cast<CastType>
						(tmp[index - width] +
						tmp[index] +
						tmp[index + width]));
			}
	}

	/// function to create a gaussian kernel of filterWidth*filterWidth with the specified sigma
	template <typename T, bool normalise>
	std::vector<T> create2DGaussianKernel(size_t filterWidth, T sigma)
	{
		static_assert(std::is_floating_point<T>::value, "Only supports floating point");

		// Abitrarily chosen
		constexpr size_t halfPointsForSimpson = 8;

		std::vector<T> gaussianKernel(filterWidth*filterWidth, 0.0);

		for (size_t x = 0; x < filterWidth; ++x)
		{
			for (size_t y = x; y < filterWidth; ++y)
			{
				auto const x0 = -static_cast<T>(filterWidth) / T(2.0) + static_cast<T>(x);
				auto const y0 = -static_cast<T>(filterWidth) / T(2.0) + static_cast<T>(y);
				auto const x1 = x0 + T(1.0);
				auto const y1 = y0 + T(1.0);

				auto const gauss = gaussianBivariateIntegrate<T>(T{}, sigma, T{}, sigma, x0, y0, x1, y1, halfPointsForSimpson);

				// If we're not on the diagonal, we need to do either side of the kernal
				if (y != x)
				{
					gaussianKernel[y * filterWidth + x] = gauss;
					gaussianKernel[x * filterWidth + y] = gauss;
				}
				else
				{
					gaussianKernel[x * filterWidth + y] = gauss;
				}
			}
		}

		if (normalise)
		{
			T sum = {};
			for (auto const &k : gaussianKernel)
				sum += k;

			if (sum != 0)
			{
				for (auto &k : gaussianKernel)
					k /= sum;
			}
		}

		return gaussianKernel;
	}
}