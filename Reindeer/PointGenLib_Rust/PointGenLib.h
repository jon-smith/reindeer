#pragma once

#include <cstdint>

extern "C"
{
	void pointgen_set_cmwc_seed(uint32_t seed);
	uint32_t pointgen_random_cmwc();
	double pointgen_random_uniform_double();
}