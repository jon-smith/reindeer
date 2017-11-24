#pragma once

#include <array>
#include <random>
#include <cassert>
#include <atomic>

#include "MutexedObject.h"
#include "XYZ.hpp"

namespace reindeer
{
	// Contains position data of points
	// and vertex and colour arrays for OpenGL
	// In use, we should maintain the following:
	// positions.size() * 3 == colours.size()
	struct PointDataArrays
	{
		std::vector<XYZ<float>> positions;
		std::vector<unsigned char> colours;
	};

	struct UpdateTimings
	{
		std::chrono::nanoseconds updatePositionTime = {};
		std::chrono::nanoseconds updateColourTime = {};
	};

	class DiffusionSimulator
	{
	private:
		// Split the number of points into 'chunks' to avoid giant vectors
		static constexpr size_t nChunks = 10;

	public:

		DiffusionSimulator();
		~DiffusionSimulator();

		// Data arrays
		using DataT = std::array<PointDataArrays, nChunks>;
		obelisk::MutexedObject<DataT> data;

		void initialise(size_t totalPoints, float width, float height);
		UpdateTimings update();
		
	private:

		std::chrono::nanoseconds updatePositions(DataT &data);
		std::chrono::nanoseconds updateColours(DataT &data);

		// Random number generators
		std::mt19937 randomEng = std::mt19937(std::random_device()());
		std::uniform_int<uint8_t> randomColour = std::uniform_int<uint8_t>(0, 255);
		std::normal_distribution<float> randomMovement = std::normal_distribution<float>(0.f, 2.f);
	};
}
