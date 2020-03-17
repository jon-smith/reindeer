#pragma once

#include <tuple>

namespace obelisk
{
	template <typename T>
	struct Vector2
	{
		T x, y;

		constexpr Vector2() : x(T{}), y(T{})
		{
		}

		constexpr Vector2(T x, T y) : x(x), y(y)
		{
		}

		bool operator==(const Vector2 &o) const
		{
			return x == o.x && y == o.y;
		}

		bool operator!=(const Vector2 &o) const
		{
			return !(*this == o);
		}

		bool operator<(const Vector2 &o) const
		{
			return std::tie(x, y) < std::tie(o.x, o.y);
		}

		Vector2 operator+(const Vector2 &o) const
		{
			return Vector2(x + o.x, y + o.y);
		}

		Vector2 operator+(float a) const
		{
			return Vector2(x + a, y + a);
		}

		Vector2 &operator+=(const Vector2 &o)
		{
			x += o.x;
			y += o.y;
			return *this;
		}

		Vector2 &operator-=(const Vector2 &o)
		{
			x -= o.x;
			y -= o.y;
			return *this;
		}

		Vector2 operator-(const Vector2 &o) const
		{
			return Vector2(x - o.x, y - o.y);
		}

		Vector2 operator-(float a) const
		{
			return Vector2(x - a, y - a);
		}

		Vector2 operator-() const
		{
			return Vector2(-x, -y);
		}

		Vector2 operator*(const Vector2 &o) const
		{
			return Vector2(x * o.x, y * o.y);
		}

		Vector2 operator*(T a) const
		{
			return Vector2(x * a, y * a);
		}

		Vector2 &operator*=(const Vector2 &o)
		{
			x *= o.x;
			y *= o.y;
			return *this;
		}

		Vector2 &operator*=(T a)
		{
			x *= a;
			y *= a;
			return *this;
		}

		Vector2 &operator/=(T a)
		{
			x /= a;
			y /= a;
			return *this;
		}

		Vector2 operator/(T a) const
		{
			return Vector2(x / a, y / a);
		}

		T lengthSquared() const
		{
			return x * x + y * y;
		}

		T length() const
		{
			return sqrt(lengthSquared());
		}
	};

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

	using Vector2f = Vector2<float>;
	using Vector2d = Vector2<double>;
	using Vector2i = Vector2<int>;
	using Vector2u = Vector2<unsigned>;

	using Vector3f = Vector3<float>;
	using Vector3d = Vector3<double>;
	using Vector3i = Vector3<int>;
	using Vector3u = Vector3<unsigned>;

}
