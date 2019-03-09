#pragma once

#include <algorithm>

namespace obelisk
{
	struct Box
	{
		Vector3f min;
		Vector3f max;

		constexpr Box(Vector3f min, Vector3f max) :
			min(min), max(max)
		{
		}

		Vector3f dimensions() const
		{
			return max - min;
		}

		Vector3f center() const
		{
			return (min + max) * 0.5f;
		}

		bool contains(const Vector3f &pos) const
		{
			return pos.x >= min.x && pos.x <= max.x &&
				pos.y >= min.y && pos.y <= max.y &&
				pos.z >= min.z && pos.z <= max.z;
		}

		bool contains(const Box &other) const
		{
			return min.x <= other.min.x && max.x >= other.max.x &&
				min.y <= other.min.y && max.y >= other.max.y &&
				min.z <= other.min.z && max.z >= other.max.z;
		}

		bool intersects(const Box &other) const
		{
			return max.x >= other.min.x && other.max.x >= min.x &&
				max.y >= other.min.y && other.max.y >= min.y &&
				max.z >= other.min.z && other.max.z >= min.z;
		}

		Vector3f nearestPointWithin(const Vector3f &pos) const
		{
			const auto x = (pos.x < min.x) ? min.x : (pos.x > max.x ? max.x : pos.x);
			const auto y = (pos.y < min.y) ? min.y : (pos.y > max.y ? max.y : pos.y);
			const auto z = (pos.z < min.z) ? min.z : (pos.z > max.z ? max.z : pos.z);
			return { x,y,z };
		}

		bool intersectsSphere(const Vector3f &center, float radius) const
		{
			return distanceToSphere(center, radius) <= 0.0f;
		}

		float distanceToSphere(const Vector3f &center, float radius) const
		{
			return (nearestPointWithin(center) - center).length() - radius;
		}
	};
}