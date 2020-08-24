#pragma once

#define DO_NOT_PARALLELIZE

#ifndef DO_NOT_PARALLELIZE

#define PARALLEL_BEGIN(TOTAL) \
	int total = TOTAL;\
	std::thread* workers[4];\
	int batch = total / 4 + (total % 4 != 0); \
	for (int k = 0; k < 4; ++k){\
	int p_begin = batch * k;\
	int p_end = std::min(batch * (k + 1), total); \
	workers[k] = new std::thread([&]
#define PARALLEL_END() \
	);}\
	for (int k = 0; k < 4; ++k){\
		workers[k]->join();\
		delete workers[k];\
	}
#else
#define PARALLEL_BEGIN(TOTAL) \
	int p_begin = 0;\
	int p_end = TOTAL;
#define PARALLEL_END()
#endif
