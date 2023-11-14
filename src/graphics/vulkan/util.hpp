#pragma once

#include <vector>
#include <cstdint>
#include <vulkan/vulkan.h>
#include "graphics_errors.hpp"
#include "../../common_util/util.hpp"

namespace util
{
    template<class T, class FN, class ...A>
    static std::vector<T> vk_enumerate(FN fn, A ...a)
    {
        uint32_t count{};

        fn(a..., &count, nullptr);

        if (count == 0)
        {
            return {};
        }

        std::vector <T> output(count);

        fn(a..., &count, output.data());

        return output;
    }

	template<class FN, IsIterable T>
	static void destroy_all(VkDevice device, FN fn, T &objects, VkAllocationCallbacks *allocator = nullptr)
	{
		for (auto &obj : objects)
		{
			fn(device, obj, allocator);
		}
	}

	template<class FN, class CI, IsIterable T>
	static pge::VulkanErrorCode create_objects(FN fn, const CI &ci, VkDevice device, T &iterable)
	{
		for (auto &obj : iterable)
		{
			auto result = fn(device, &ci, nullptr, &obj);

			if (result != VK_SUCCESS)
			{
				return pge::VulkanErrorCode::SemaphoreCreationError;
			}
		}

		return pge::VulkanErrorCode::Ok;
	}

	template<IsIterable T>
	static pge::VulkanErrorCode create_semaphores(VkDevice device, T &iterable)
	{
		VkSemaphoreCreateInfo semaphore_ci
		{
		  	.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};

		return create_objects(vkCreateSemaphore, semaphore_ci, device, iterable);
	}

	template<IsIterable T>
	static pge::VulkanErrorCode create_fences(VkDevice device, T &iterable)
	{
		VkFenceCreateInfo fence_ci
		{
			  .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			  .flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};

		return create_objects(vkCreateFence, fence_ci, device, iterable);
	}
}
