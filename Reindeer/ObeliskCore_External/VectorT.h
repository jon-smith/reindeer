#pragma once

#include <tuple>

namespace obelisk
{
	template <typename T>
	struct Vector3
	{
		T x, y, z;

		constexpr Vector3() : x(T{}), y(T{}), z(T{})
		{
		}

		constexpr Vector3(T x, T y, T z) :x(x), y(y), z(z)
		{
		}

		bool operator==(const Vector3 &o) const
		{
			return x == o.x && y == o.y && z == o.z;
		}

		bool operator!=(const Vector3 &o) const
		{
			return !(*this == o);
		}

		bool operator<(const Vector3 &o) const
		{
			return std::tie(x, y, z) < std::tie(o.x, o.y, o.z);
		}

		Vector3 operator+(const Vector3 &o) const
		{
			return Vector3(x + o.x, y + o.y, z + o.z);
		}

		Vector3 operator+(float a) const
		{
			return Vector3(x + a, y + a, z + a);
		}

		Vector3 &operator+=(const Vector3 &o)
		{
			x += o.x;
			y += o.y;
			z += o.z;
			return *this;
		}

		Vector3 &operator-=(const Vector3 &o)
		{
			x -= o.x;
			y -= o.y;
			z -= o.z;
			return *this;
		}

		Vector3 operator-(const Vector3 &o) const
		{
			return Vector3(x - o.x, y - o.y, z - o.z);
		}

		Vector3 operator-(float a) const
		{
			return Vector3(x - a, y - a, z - a);
		}

		Vector3 operator-() const
		{
			return Vector3(-x, -y, -z);
		}

		Vector3 operator*(const Vector3 &o) const
		{
			return Vector3(x * o.x, y * o.y, z * o.z);
		}

		Vector3 operator*(T a) const
		{
			return Vector3(x * a, y * a, z * a);
		}

		Vector3 &operator*=(const Vector3 &o)
		{
			x *= o.x;
			y *= o.y;
			z *= o.z;
			return *this;
		}

		Vector3 &operator*=(T a)
		{
			x *= a;
			y *= a;
			z *= a;
			return *this;
		}		

		Vector3 &operator/=(T a)
		{
			x /= a;
			y /= a;
			z /= a;
			return *this;
		}

		Vector3 operator/(T a) const
		{
			return Vector3(x / a, y / a, z / a);
		}

		T lengthSquared() const
		{
			return x * x + y * y + z * z;
		}

		T length() const
		{
			return sqrt(lengthSquared());
		}
	};

	using Vector3f = Vector3<float>;
	using Vector3d = Vector3<double>;
	using Vector3i = Vector3<int>;
	using Vector3u = Vector3<unsigned>;

}
