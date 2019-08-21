#pragma once

#include <vector>

#include "ActivityStructures.h"

namespace reindeer
{
	struct PaceCurvePoint
	{
		double distance_m = 0.0;
		DistTimeElev bestPaceSegment = DistTimeElev::zero();

		PaceCurvePoint() = default;

		PaceCurvePoint(double distance_m, DistTimeElev bestPaceSegment) :
			distance_m(distance_m), bestPaceSegment(bestPaceSegment)
		{
		}
	};

	std::vector<PaceCurvePoint> calculatePaceCurve(
		const std::vector<GpxPoint> &gpxData, 
		const double min_m, 
		const double resolution_m,
		const double minElevationDiff_m);

	std::vector<PaceCurvePoint> mergePaceCurves(const std::vector<std::vector<PaceCurvePoint>> &paceCurves);
}