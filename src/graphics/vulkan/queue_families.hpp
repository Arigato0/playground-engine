#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <set>

namespace pge
{
	struct QueueFamilies
	{
		std::optional<uint32_t> graphics;
		std::optional<uint32_t> presentation;
		uint32_t                count{};

		bool is_complete() const
		{
			return graphics.has_value() && presentation.has_value();
		}

		std::set<uint32_t> to_set() const
		{
			return {graphics.value(), presentation.value()};
		}

		void find_families(VkPhysicalDevice device, VkSurfaceKHR surface);
	};
}