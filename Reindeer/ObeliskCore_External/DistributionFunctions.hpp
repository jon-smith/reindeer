#pragma once

#include <functional>
#include <vector>

#include "ContainerMaker.hpp"

namespace obelisk
{
	constexpr auto pi = 3.14159265358979323846;

	template <typename T>
	T gaussianPDF(T mu, T sigma, T x)
	{
		return (T(1.0) / (sqrt(T(2.0) *sigma*sigma*T(pi)))*exp(-((x - mu)*(x - mu)) / (T(2.0) *(sigma*sigma))));
	}

	template <typename T>
	T gaussianCDF(T mu, T sigma, T x)
	{
		return T(0.5) *(T(1.0) +erf((x - mu) / (sigma*sqrt(T(2.0)))));
	}

	template <typename T>
	T gaussianBivariatePDF(T muX, T sigmaX, T muY, T sigmaY, T x, T y)
	{
		// Assumes 0 correlation
		auto const a = T(1.0) / (T(2.0) *T(pi)*sigmaX*sigmaY);
		auto const b = T(-0.5);
		auto const c = (x - muX)*(x - muX) / (sigmaX*sigmaX);
		auto const d = (y - muY)*(y - muY) / (sigmaY*sigmaY);
		return a*exp(b*(c + d));
	}

	template <typename T>
	T simpsons2D(std::function<T(T,T)> f, T x0, T y0, T x1, T y1, size_t halfPoints)
	{
		auto const h = (x1 - x0) / static_cast<T>(2 * halfPoints);
		auto const k = (y1 - y0) / static_cast<T>(2 * halfPoints);

		auto const subX = generateVector<T>([x0,h](size_t i)
		{
			return x0 + static_cast<T>(i)*h;
		}, 0, halfPoints * 2);

		auto const subY = generateVector<T>([y0,k](size_t i)
		{
			return y0 + static_cast<T>(i)*k;
		}, 0, halfPoints * 2);

		auto const v1 = f(x0, y0) + f(x0, y1) + f(x1, y0) + f(x1, y1);

		// Fixed x, range y
		auto v2 = T{};
		for (size_t i = 1; i < halfPoints+1; ++i)
			v2 += f(x0, subY[2 * i - 1]);
		v2 *= T(4.0);

		auto v3 = T{};
		for (size_t i = 1; i < halfPoints; ++i)
			v3 += f(x0, subY[2 * i]);
		v3 *= T(2.0);

		auto v4 = T{};
		for (size_t i = 1; i < halfPoints+1; ++i)
			v4 += f(x1, subY[2 * i-1]);
		v4 *= T(4.0);

		auto v5 = T{};
		for (size_t i = 1; i < halfPoints; ++i)
			v5 += f(x1, subY[2 * i]);
		v5 *= T(2.0);

		// Fixed y, range x
		auto v6 = T{};
		for (size_t i = 1; i < halfPoints+1; ++i)
			v6 += f(subX[2 * i - 1], y0);
		v6 *= T(4.0);

		auto v7 = T{};
		for (size_t i = 1; i < halfPoints; ++i)
			v7 += f(subX[2 * i], y0);
		v7 *= T(2.0);

		auto v8 = T{};
		for (size_t i = 1; i < halfPoints+1; ++i)
			v8 += f(subX[2 * i - 1], y1);
		v8 *= T(4.0);

		auto v9 = T{};
		for (size_t i = 1; i < halfPoints; ++i)
			v9 += f(subX[2 * i], y1);
		v9 *= T(2.0);

		// Range x and y
		auto v10 = T{};
		for (size_t j = 1; j < halfPoints+1; ++j)
			for (size_t i = 1; i < halfPoints+1; ++i)
				v10 += f(subX[2 * i-1], subY[2*j-1]);
		v10 *= T(16.0);

		auto v11 = T{};
		for (size_t j = 1; j < halfPoints; ++j)
			for (size_t i = 1; i < halfPoints+1; ++i)
				v11 += f(subX[2 * i - 1], subY[2 * j]);
		v11 *= T(8.0);

		auto v12 = T{};
		for (size_t j = 1; j < halfPoints+1; ++j)
			for (size_t i = 1; i < halfPoints; ++i)
				v12 += f(subX[2 * i], subY[2 * j-1]);
		v12 *= T(8.0);

		auto v13 = T{};
		for (size_t j = 1; j < halfPoints; ++j)
			for (size_t i = 1; i < halfPoints; ++i)
				v13 += f(subX[2 * i], subY[2 * j]);
		v13 *= T(4.0);
		
		return T(1.0 / 9.0) * h * k * (v1 + v2 +v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13);
	}

	template <typename T>
	T gaussianBivariateIntegrate(T muX, T sigmaX, T muY, T sigmaY, T x0, T y0, T x1, T y1, unsigned halfPoints)
	{
		auto const fn = [=](T x, T y)
		{
			return gaussianBivariatePDF<T>(muX, sigmaX, muY, sigmaY, x, y);
		};
		return simpsons2D<T>(fn, x0, y0, x1, y1, halfPoints);
	}
}
