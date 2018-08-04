#include "DiffusionSimulator.h"

#include "PointGenLib_Rust\PointGenLib.h"

using namespace reindeer;

DiffusionSimulator::DiffusionSimulator() = default;
DiffusionSimulator::~DiffusionSimulator() = default;

void DiffusionSimulator::initialise(size_t totalPoints, float width, float height)
{
	auto const midPointX = width*0.5;
	auto const midPointY = height*0.5;
	data.lockedModify([this, totalPoints, midPointX, midPointY](DataT &data)
	{
		// Allocate arrays
		// Distribute the points across the chunks - if not exactly divisible, the last chunk will have less chunk
		auto const pointsPerChunk = (totalPoints - 1 / nChunks) + 1UL;
		auto pointsSoFar = decltype(pointsPerChunk){};
		for (size_t i = 0; i<data.size(); ++i)
		{
			auto const pointsThisChunk = std::min(pointsPerChunk, totalPoints - pointsSoFar);
			data[i].positions.assign(pointsThisChunk, { 0.f,0.f,0.f });
			data[i].colours.assign(3 * pointsThisChunk, 0);
			pointsSoFar += pointsThisChunk;
		}

		auto const randomXGen = [max = midPointX*2.0](){
			return static_cast<float>(pointgen_random_uniform_double()*max);
		};

		auto const randomYGen = [max = midPointY*2.0](){
			return static_cast<float>(pointgen_random_uniform_double()*max);
		};

		// Random initial positions
		for (auto &d : data)
		{
			for (auto &p : d.positions)
			{
				p.x = randomXGen();
				p.y = randomYGen();
				p.z = 0.f;
			}
		}
	});
}

UpdateTimings DiffusionSimulator::update()
{
	return data.lockedModify<UpdateTimings>([this](DataT &data)
	{
		UpdateTimings timings;
		timings.updatePositionTime = updatePositions(data);
		timings.updateColourTime == updateColours(data);
		return timings;
	});
}

std::chrono::nanoseconds DiffusionSimulator::updatePositions(DataT &data)
{
	auto const beforeTime = std::chrono::high_resolution_clock::now();
	for (auto &d : data)
	{
		for (auto &p : d.positions)
		{
			p.x += randomMovement(randomEng);
			p.y += randomMovement(randomEng);
			p.z += randomMovement(randomEng);
		}
	}
	return std::chrono::high_resolution_clock::now() - beforeTime;
}

std::chrono::nanoseconds DiffusionSimulator::updateColours(DataT &data)
{
	auto const beforeTime = std::chrono::high_resolution_clock::now();
	for (auto &d : data)
	{
		assert(d.colours.size() == d.positions.size() * 3);

		for (size_t i = 0; i < d.positions.size(); ++i)
		{
			// Base colour on Z position
			d.colours[3 * i] = 255;
			d.colours[3 * i + 1] = 0;
			d.colours[3 * i + 2] = static_cast<unsigned char>(std::min(255.f, std::max(0.f, 5.f*d.positions[i].z + (255.f*0.5f))));
		}
	}
	return std::chrono::high_resolution_clock::now() - beforeTime;
}