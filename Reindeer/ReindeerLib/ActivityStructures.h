#pragma once

#include <chrono>

namespace reindeer
{
	struct GpxPoint
	{
		double longitude;
		double latitude;
		double elevation_m;
		uint64_t dateTime_ms;

		GpxPoint(double longitude, double latitude, double elevation_m, uint64_t dateTime_ms) :
			longitude(longitude), latitude(latitude), elevation_m(elevation_m), dateTime_ms(dateTime_ms)
		{

		}

		GpxPoint() = delete;
	};

	struct DistanceTime
	{
		double distance_m;
		double time_s;

		DistanceTime(double distance_m, double time_s) :
			distance_m(distance_m), time_s(time_s)
		{

		}

		DistanceTime() = delete;

		double pace_ms() const
		{
			return distance_m / time_s;
		}

		double pace_minPerkm() const
		{
			return 60.0 / (1000 * pace_ms());
		}
	};

	struct ElevationInfo
	{
		// The elevation change between the start and end (end elev - start elev)
		double elevationDiff_m;
		// Sum of positive elevation changes between sub-datapoints
		double cumulativeElevation_m;

		ElevationInfo(double elevationDiff_m, double cumulativeElevation_m) :
			elevationDiff_m(elevationDiff_m), cumulativeElevation_m(cumulativeElevation_m)
		{

		}

		ElevationInfo() = delete;
	};

	struct DistTimeElev
	{
		DistanceTime distanceTime;
		ElevationInfo elevation;

		DistTimeElev(DistanceTime distanceTime, ElevationInfo elevation) :
			distanceTime(distanceTime), elevation(elevation)
		{

		}

		static DistTimeElev zero()
		{
			return DistTimeElev(
				DistanceTime(0.0, 0.0),
				ElevationInfo(0.0, 0.0));
		}

		static DistTimeElev sum(const DistTimeElev &a, const DistTimeElev &b)
		{
			return DistTimeElev(
				DistanceTime(a.distanceTime.distance_m + b.distanceTime.distance_m,
					a.distanceTime.time_s + b.distanceTime.time_s),
				ElevationInfo(a.elevation.elevationDiff_m + b.elevation.elevationDiff_m,
					a.elevation.cumulativeElevation_m + b.elevation.cumulativeElevation_m));
		}

		static DistTimeElev fromGpx(const GpxPoint &from, const GpxPoint &to)
		{
			// Calculate distance from long/lat change
			// This calculation is currently abitrary
			const auto longDiff = to.longitude - from.longitude;
			const auto latDiff = to.latitude - from.latitude;
			const auto distance_m = sqrt(longDiff*longDiff + latDiff*latDiff);

			const auto time_s = 1000.0 * static_cast<double>(to.dateTime_ms - from.dateTime_ms);

			const auto elevGain = to.elevation_m - from.elevation_m;

			return DistTimeElev(
				DistanceTime(distance_m, time_s),
				ElevationInfo(elevGain, std::abs(elevGain)));
		}
	};
}