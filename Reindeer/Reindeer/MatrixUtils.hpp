#pragma once

namespace reindeer
{
	template <size_t Size, typename T>
	std::array<T, Size*Size> squareMatrixMultiply(const std::array<T, Size*Size> &a, const std::array<T, Size*Size> &b)
	{
		std::array<T, Size*Size> res = {};

		for (auto i = 0; i < Size; i++)
		{
			for (auto j = 0; j < Size; j++)
			{
				for (auto k = 0; k < Size; k++)
				{
					res[i * Size + j] += a[i * Size + k] * b[k * Size + j];
				}
			}
		}

		return res;
	}
}