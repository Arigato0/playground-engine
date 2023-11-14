#pragma once

#include <cstdint>
#include <string_view>

namespace pge
{
    enum class VulkanErrorCode : uint8_t
    {
        Ok,
        NoPhysicalDevice,
        VulkanInstanceError,
        IncompleteQueueFamily,
        VulkanCreateError,
        VulkanSurfaceError,
        SwapChainCreationError,
        ImageViewCreationError,
        ShaderModuleCreationError,
        PipelineCreationError,
        RenderPassCreationError,
        FrameBufferCreationError,
        CommandPoolCreationError,
        CommandBufferCreationError,
        CommandBufferRecordingError,
        SemaphoreCreationError,
        QueueSubmitError,
        PresentationError,
        ImageAcquisitionError,
    };

    std::string_view vulkan_error_message(VulkanErrorCode code);
}
