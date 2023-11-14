#include <array>
#include "graphics_errors.hpp"

constexpr std::array ERROR_MESSAGES
{
        "No error",
        "No suitable physical devices found",
        "Could not create vulkan instance",
        "Not all required queue families were available",
        "Could not create an instance of a requested vulkan resource",
        "Could not create vulkan surface",
        "Could not create swap chain",
        "Could not create image view",
        "Could not create shader module",
		"Could not create graphics pipeline",
		"Could not create render pass",
		"Could not create frame buffer",
		"Could not create command pool",
		"Could not create command buffer",
		"An error occurred when recording a command buffer",
		"Could not create semaphore",
		"Could not submit to the queue",
		"Could not present the current frame",
        "Could not acquire the next swap chain image",
};

std::string_view pge::vulkan_error_message(VulkanErrorCode code)
{
    auto idx = (int)code;

    if (idx >= ERROR_MESSAGES.size())
    {
        return {};
    }

    return ERROR_MESSAGES[idx];
}
