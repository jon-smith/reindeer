#include "PaceCurve.h"

#include "ContainerUtils.h"
#include "Optional.h"

#include <set>

using namespace reindeer;
using namespace obelisk;

namespace
{
	// Is candidate better than current?
	bool isImprovement(const DistanceTime &current, const DistanceTime &candidate)
	{
		// Compare on pace and then distance
		// Is paces are equal, then rank on distance
		const auto currentPaceDist = std::make_tuple(current.pace_ms(), current.distance_m);
		const auto candidatePaceDist = std::make_tuple(candidate.pace_ms(), candidate.distance_m);

		return candidatePaceDist > currentPaceDist;
	}

	bool isImprovement(const DistTimeElev &current, const DistTimeElev &candidate)
	{
		return isImprovement(current.distanceTime, candidate.distanceTime);
	}
}

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
	std::vector<PaceCurvePoint> bestPacePerDistance;
	for (unsigned i = 0; ; ++i)
	{
		const auto distance_m = min_m + resolution_m * static_cast<double>(i);
		if (distance_m > totalDistance_m)
			break;

		// Only assign distance, we assign other values when we find a valid segment
		// when time_s == 0 we know it hasn't been set yet
		bestPacePerDistance.push_back(PaceCurvePoint(distance_m, DistTimeElev::zero()));
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
				if (bestPace.distance_m <=
					cumulativeSubSegment.distanceTime.distance_m)
				{
					// If we have found a valid pace and it's better than the current sub segment
					// We can break the loop because no shorter distance will have an improvement
					if (bestPace.bestPaceSegment.distanceTime.time_s != 0.0 &&
						!isImprovement(bestPace.bestPaceSegment, cumulativeSubSegment))
					{
						break;
					}

					// This is an improvement, so replace
					bestPace.bestPaceSegment = cumulativeSubSegment;					
				}
			}
		}
	}

	// Filter out invalid paces
	std::vector<PaceCurvePoint> filtered;
	for (const auto &best : bestPacePerDistance)
	{
		if (best.bestPaceSegment.distanceTime.time_s > 0.0)
		{
			PaceCurvePoint p(best.distance_m, best.bestPaceSegment);
			filtered.push_back(p);
		}
	}

	return filtered;
}

std::vector<PaceCurvePoint> reindeer::mergePaceCurves(const std::vector<std::vector<PaceCurvePoint>> &paceCurves)
{
	// More each unique distance, find the best time
	std::map<double, PaceCurvePoint> distanceToBestPoint;
	for (const auto &paceCurve : paceCurves)
	{
		for (const auto &p : paceCurve)
		{
			const auto thisDistance = p.distance_m;
			auto existing = distanceToBestPoint.find(thisDistance);
			// If we don't have a point for this distance, or the pace is an approvement
			// Add/replace
			if (existing == distanceToBestPoint.end() ||
				isImprovement(existing->second.bestPaceSegment, p.bestPaceSegment))
			{
				distanceToBestPoint[thisDistance] = p;
			}
		}
	}

	// Get ordered values from map
	auto bestPoints = convertAll<PaceCurvePoint>(distanceToBestPoint,
		[](const std::pair<double, PaceCurvePoint> &kv){
		return kv.second;
	});

	// Cascade down distances (if we have a shorter distance with a worse pace, replace with the higher distance)
	for (size_t i = 1; i < bestPoints.size(); ++i)
	{
		const auto reverseIndex = bestPoints.size() - 1 - i;
		auto &currentPoint = bestPoints[reverseIndex];
		const auto &bestWithHigherDistance = bestPoints[reverseIndex + 1].bestPaceSegment;
		if (isImprovement(currentPoint.bestPaceSegment, bestWithHigherDistance))
		{
			currentPoint.bestPaceSegment = bestWithHigherDistance;
		}
	}

	return bestPoints;
}