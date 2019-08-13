#include "PaceCurve.h"

#include "ContainerUtils.h"

using namespace reindeer;
using namespace obelisk;

std::vector<PaceCurvePoint> reindeer::calculatePaceCurve(
	const std::vector<GpxPoint> &gpxData,
	const double min_m,
	const double resolution_m,
	const double maxElevationLoss_m)
{
	std::vector<DistTimeElev> converted;
	for (size_t i = 1; i < gpxData.size(); ++i)
	{
		converted.push_back(DistTimeElev::fromGpx(gpxData[0], gpxData[1]));
	}
	return calculatePaceCurve(converted, min_m, resolution_m, maxElevationLoss_m);
}

std::vector<PaceCurvePoint> reindeer::calculatePaceCurve(
	const std::vector<DistTimeElev> &motionData,
	const double min_m,
	const double resolution_m,
	const double maxElevationLoss_m)
{
	// Todo...implement
	return {};
}

std::vector<PaceCurvePoint> reindeer::mergePaceCurves(const std::vector<std::vector<PaceCurvePoint>> &paceCurves)
{
	// Todo...implement
	return {};
}