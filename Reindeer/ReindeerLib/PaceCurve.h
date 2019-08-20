#pragma once

#include <vector>

#include "ActivityStructures.h"

namespace reindeer
{
	struct PaceCurvePoint
	{
		DistanceTime distanceTime;

		PaceCurvePoint(DistanceTime distanceTime) :
			distanceTime(distanceTime)
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