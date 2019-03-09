#pragma once

#include "VectorT.h"
#include "Box.h"

#include <memory>
#include <array>
#include <algorithm>
#include <vector>

namespace obelisk
{
	template <typename T>
	struct ImplicitGetPos
	{
		static const Vector3f &getPos(const T &item)
		{
			return item;
		}
	};

	template <typename ItemT, size_t MaxItemsPerNode = 256, typename GetPosT = ImplicitGetPos<ItemT>>
	class Octree
	{
	private:
		// total items in this and all child nodes
		size_t nTotalItems = {};

		const Vector3f center;
		const Vector3f halfsize;

		std::vector<ItemT> items;
		std::array<std::unique_ptr<Octree>, 8> nodes;

	public:

		Octree(Vector3f center, Vector3f size) :
			center(center),
			halfsize(size * 0.5f)
		{
		}

		Octree(Box extents) :
			Octree(extents.center(), extents.dimensions())
		{
		}

		Octree(Octree &&o) :
			center(o.center),
			halfsize(o.halfsize),
			nTotalItems(o.nTotalItems),
			items(std::move(o.items)),
			nodes(std::move(o.nodes))
		{
		}

		Octree::~Octree()
		{
		}

		bool hasChildren() const
		{
			return nodes[0] != nullptr;
		}

		unsigned getChildIndex(const Vector3f &pos) const
		{
			unsigned index = 0;
			if (pos.x > center.x)
				index += 1;
			if (pos.y > center.y)
				index += 2;
			if (pos.z > center.z)
				index += 4;
			return index;
		}

		template <typename InterableT>
		void insertRange(InterableT &&items)
		{
			for (auto &&item : items)
				insert(item);
		}

		void insert(const ItemT &item)
		{
			const auto &pos = GetPosT::getPos(item);

			if (hasChildren())
			{
				nodes[getChildIndex(pos)]->insert(item);
				++nTotalItems;
			}
			else if (items.size() == MaxItemsPerNode)
			{
				for (size_t i = 0; i < nodes.size(); ++i)
				{
					auto childCentre = center - halfsize * 0.5f;
					if (i & 1)
						childCentre.x += halfsize.x;
					if (i & 2)
						childCentre.y += halfsize.y;
					if (i & 4)
						childCentre.z += halfsize.z;
					nodes[i] = std::make_unique<Octree>(childCentre, halfsize);
				}

				insertRange(items);
				insert(item);
				nTotalItems -= items.size();
				items.clear();
			}
			else
			{
				items.push_back(item);
				++nTotalItems;
			}
		}

		const Octree &findNode(const Vector3f &pt) const
		{
			if (hasChildren())
			{
				const auto childIndex = getChildIndex(pt);
				return nodes[childIndex]->findNode(pt);
			}
			return *this;
		}

		template <typename TFunc>
		void forEachItemInSphere(const Vector3f &center, float radius, TFunc f) const
		{
			if (hasChildren())
			{
				for (const auto &node : nodes)
				{
					if (node->boundingBox().intersectsSphere(center, radius))
						node->forEachItemInSphere(center, radius, f);
				}
			}
			else
			{
				const auto rSquared = radius * radius;
				for (const auto &item : items)
				{
					if ((center - GetPosT::getPos(item)).lengthSquared() < rSquared)
						f(item);
				}
			}
		}

		const ItemT *findNearestNeighbour(Vector3f pos, float searchRadius) const
		{
			auto currentMinDistanceSq = std::numeric_limits<float>::max();
			const ItemT *closestItem = nullptr;

			forEachItemInSphere(pos, searchRadius,
				[&](const ItemT &item)
			{
				const auto &itemPos = GetPosT::getPos(item);
				const auto distanceSq = (pos - itemPos).lengthSquared();
				if (distanceSq < currentMinDistanceSq)
				{
					currentMinDistanceSq = distanceSq;
					closestItem = &item;
				}
			});

			return closestItem;
		}

		Vector3f minExtents() const
		{
			return center - halfsize;
		}

		Vector3f maxExtents() const
		{
			return center + halfsize;
		}

		Box boundingBox() const
		{
			return Box(minExtents(), maxExtents());
		}

		size_t getTotalItemCount() const
		{
			return nTotalItems;
		}

		template <typename TFunc>
		void forEachItem(TFunc f) const
		{
			if (hasChildren())
			{
				for (const auto &node : nodes)
					node->forEachItem(f);
			}
			else
			{
				for (const auto &item : items)
					f(item);
			}
		}

		template <typename TFunc>
		void forEachItemInBox(const Box &box, TFunc f) const
		{
			if (hasChildren())
			{
				for (const auto &node : nodes)
				{
					if (node->boundingBox().intersects(box))
						node->forEachItemInBox(box, f);
				}
			}
			else
			{
				for (const auto &item : items)
				{
					if (box.contains(GetPosT::getPos(item)))
						f(item);
				}
			}
		}

		size_t getMaxDepth() const
		{
			size_t depth = 0;
			for (auto &node : nodes)
			{
				if (node)
				{
					depth = std::max(depth, node->getMaxDepth() + 1);
				}
			}
			return depth;
		}
	};
}
