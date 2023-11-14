#pragma once

#include <vulkan/vulkan.h>
#include "../../application/glfw_window.hpp"

namespace pge
{
    struct SwapChainData
    {
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR format;
        VkPresentModeKHR present_mode;
    };

    SwapChainData get_swap_chain_data(VkPhysicalDevice device, VkSurfaceKHR surface);

    bool is_swap_chain_supported(VkPhysicalDevice device, VkSurfaceKHR surface);

    VkExtent2D get_extent(const VkSurfaceCapabilitiesKHR &capabilities, IWindow *window);
}
