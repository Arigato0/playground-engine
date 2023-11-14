#include <vector>
#include <algorithm>

#include "swap_chain.hpp"
#include "util.hpp"

bool pge::is_swap_chain_supported(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    uint32_t format_count {};

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

    uint32_t present_modes_count {};

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_modes_count, nullptr);

    return format_count != 0 && present_modes_count != 0;
}

struct SwapChainSupportData
{
    VkSurfaceCapabilitiesKHR capabilities {};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;

    inline bool ok() const
    {
        return !formats.empty() || !present_modes.empty();
    }
};

SwapChainSupportData get_support_data(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportData data;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &data.capabilities);

    data.formats = util::vk_enumerate<VkSurfaceFormatKHR>(vkGetPhysicalDeviceSurfaceFormatsKHR,
                                                          device, surface);
    data.present_modes = util::vk_enumerate<VkPresentModeKHR>(vkGetPhysicalDeviceSurfacePresentModesKHR,
                                                              device, surface);

    return data;
}

inline VkSurfaceFormatKHR find_format(const std::vector<VkSurfaceFormatKHR> &formats)
{
    for (const auto format : formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB
            && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    return formats.front();
}

inline VkPresentModeKHR find_present_mode(const std::vector<VkPresentModeKHR> &present_modes)
{
    for (const auto mode : present_modes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

pge::SwapChainData pge::get_swap_chain_data(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    auto support_data = get_support_data(device, surface);

    return SwapChainData
    {
        support_data.capabilities,
        find_format(support_data.formats),
        find_present_mode(support_data.present_modes)
    };
}

VkExtent2D pge::get_extent(const VkSurfaceCapabilitiesKHR &capabilities, IWindow *window)
{
    auto [width, height] = window->framebuffer_size();

    VkExtent2D extent
    {
        width,
        height
    };

    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return extent;
}
