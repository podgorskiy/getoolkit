#pragma once
#include <stdint.h>
#include <spdlog/fmt/fmt.h>
#include "common.h"


namespace utils
{
	struct MemoryUsage
	{
		uint64_t memory_in_use_percents;
		uint64_t physical_memory_total;
		uint64_t physical_memory_free;
		uint64_t paging_file_total;
		uint64_t paging_file_free;
		uint64_t virtual_memory_total;
		uint64_t virtual_memory_free;
		uint64_t extended_memory_free;
	};

	MemoryUsage GetMemoryUsage();
}

template<>
struct fmt::formatter<utils::MemoryUsage>: fmt::formatter<string_view>
{
	template<typename FormatContext>
	auto format(const utils::MemoryUsage& v, FormatContext& ctx)
	{
		return fmt::format_to(ctx.out(),
				"Memory info:\n"
				"\t - {:30}{}%\n"
				"\t - {:30}{}B\n"
				"\t - {:30}{}B\n"
				"\t - {:30}{}B\n"
				"\t - {:30}{}B\n"
				"\t - {:30}{}B\n"
				"\t - {:30}{}B\n"
				"\t - {:30}{}B\n",
				"memory_in_use_percents: ", v.memory_in_use_percents,
				"physical_memory_total: ", utils::Humanize(v.physical_memory_total),
				"physical_memory_free: ", utils::Humanize(v.physical_memory_free),
				"paging_file_total: ", utils::Humanize(v.paging_file_total),
				"paging_file_free: ", utils::Humanize(v.paging_file_free),
				"virtual_memory_total: ", utils::Humanize(v.virtual_memory_total),
				"virtual_memory_free: ", utils::Humanize(v.virtual_memory_free),
				"extended_memory_free: ", utils::Humanize(v.extended_memory_free));
	}
};
