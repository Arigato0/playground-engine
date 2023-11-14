#include "queue_families.hpp"

#include "../../common_util/util.hpp"
#include "util.hpp"
#include "../../application/fmt.hpp"

void pge::QueueFamilies::find_families(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    auto family_properties = util::vk_enumerate<VkQueueFamilyProperties>(vkGetPhysicalDeviceQueueFamilyProperties,
                                                                         device);
    if (family_properties.empty())
    {
        return;
    }

    int idx = 0;

    for (auto &family: family_properties)
    {
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphics = idx;
        }

        auto supported = VK_FALSE;

        vkGetPhysicalDeviceSurfaceSupportKHR(device, idx, surface, &supported);

        if (supported)
        {
            presentation = idx;
        }

        if (is_complete())
        {
            break;
        }

        idx++;
    }

    count = idx;
}
