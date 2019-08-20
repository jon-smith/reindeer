#include "PaceCurve.h"

#include "ContainerUtils.h"
#include "Optional.h"

using namespace reindeer;
using namespace obelisk;

std::vector<PaceCurvePoint> reindeer::calculatePaceCurve(
	const std::vector<GpxPoint> &gpxData,
	const double min_m,
	const double resolution_m,
	const double minElevationDiff_m)
{
	// Find total distance
	double totalDistance_m = 0.0;
	for (size_t i = 1; i < gpxData.size(); ++i)
	{
		totalDistance_m += DistTimeElev::fromGpx(gpxData[0], gpxData[1]).distanceTime.distance_m;
	}

	// Find the distances we want to find paces for
	std::vector<DistTimeElev> bestPacePerDistance;
	for (unsigned i = 0; ; ++i)
	{
		const auto distance_m = min_m + resolution_m * static_cast<double>(i);
		if (distance_m > totalDistance_m)
			break;

		// Only assign distance, we assign other values when we find a valid segment
		// when time_s == 0 we know it hasn't been set yet
		auto distTime = DistTimeElev::zero();
		distTime.distanceTime.distance_m = distance_m;
		bestPacePerDistance.push_back(distTime);
	}
	
	// Loop through every subsegment
	for (size_t i = 0; i < gpxData.size(); ++i)
	{
		auto cumulativeSubSegment = DistTimeElev::zero();
		for (size_t j = i + 1; j < gpxData.size(); ++j)
		{
			const auto distTime = DistTimeElev::fromGpx(gpxData[j - 1], gpxData[j]);
			cumulativeSubSegment = DistTimeElev::sum(cumulativeSubSegment, distTime);
			// If the elevation diff doesn't fit the criteria, ignore
			if (cumulativeSubSegment.elevation.elevationDiff_m < minElevationDiff_m)
				continue;

			// Go through best paces to see if we have improved on any
			for (size_t k = 0; k < bestPacePerDistance.size(); ++k)
			{
				const auto bestPaceIndex = bestPacePerDistance.size() - 1 - k;
				auto &bestPace = bestPacePerDistance[bestPaceIndex];
				// Is the distance within the current distance
				if (bestPace.distanceTime.distance_m <=
					cumulativeSubSegment.distanceTime.distance_m)
				{
					// If we have found a valid pace and it's better than the current sub segment
					// We can break the loop because no shorter distance will have an improvement
					if (bestPace.distanceTime.time_s != 0.0 &&
						bestPace.distanceTime.time_s <= cumulativeSubSegment.distanceTime.distance_m)
					{
						break;
					}

					// This is an improvement, so replace
					bestPace = cumulativeSubSegment;					
				}
			}
		}
	}

	// Filter out invalid paces
	std::vector<PaceCurvePoint> filtered;
	for (const auto &best : bestPacePerDistance)
	{
		if (best.distanceTime.time_s > 0.0)
		{
			PaceCurvePoint p(best.distanceTime);
			filtered.push_back(p);
		}
	}

	return filtered;
}

std::vector<PaceCurvePoint> reindeer::mergePaceCurves(const std::vector<std::vector<PaceCurvePoint>> &paceCurves)
{
	// Todo...implement
	return {};
}