#include "AddVectorsWrapper.h"
#include "AddVectors.cuh"

using namespace kudah;

std::vector<double> kudah::addVectors(const std::vector<double> &a, const std::vector<double> &b)
{
	return impl::addVectors(a, b);
}

std::vector<int> kudah::addVectors(const std::vector<int> &a, const std::vector<int> &b)
{
	return impl::addVectors(a, b);
}

std::vector<float> kudah::addVectors(const std::vector<float> &a, const std::vector<float> &b)
{
	return impl::addVectors(a, b);
}