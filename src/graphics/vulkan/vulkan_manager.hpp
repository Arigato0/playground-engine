#pragma once

#include <vector>
#include <glm/vec4.hpp>

#include "../../application/glfw_window.hpp"
#include "../../application/log.hpp"
#include "../../application/error.hpp"
#include "../../common_util/util.hpp"
#include "queue_families.hpp"
#include "graphics_errors.hpp"

namespace pge
{
    class VulkanManager
    {
    public:

        VulkanManager() = default;

        ~VulkanManager() ;

        uint8_t init();

        VulkanErrorCode record_cmd(VkCommandBuffer cmd_buffer, size_t image_idx);

        inline void set_window(IWindow *window)
        {
            m_window = window;
        }

        inline void wait()
        {
            vkDeviceWaitIdle(m_device);
        }

        inline void set_clear_color(glm::vec4 value)
        {
            m_clear_color = { EXPAND_VEC4(value) };
        }

        VulkanErrorCode recreate_swap_chain();

        uint8_t draw_frame();

        std::vector<const char*> extensions();

        inline std::string_view error_message(uint8_t code)
        {
            return vulkan_error_message((VulkanErrorCode)code);
        }

    private:
        static constexpr auto MAX_IN_FLIGHT_FRAMES = 2;

        IWindow *m_window  = nullptr;
        VkInstance m_instance = VK_NULL_HANDLE;
        VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        VkQueue m_graphics_q;
        VkQueue m_presentation_q;
        VkSurfaceKHR m_surface;
        VkSwapchainKHR m_swap_chain;
        QueueFamilies m_queue_families;

        VkFormat m_format;
        VkExtent2D m_extent;

        VkRenderPass m_render_pass;
        VkPipelineLayout m_pipeline_layout;
        VkPipeline m_pipeline;

        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_image_views;
        std::vector<VkFramebuffer> m_frame_buffers;

        VkCommandPool m_command_pool;

        volatile uint32_t m_current_frame {};

        std::array<VkCommandBuffer, MAX_IN_FLIGHT_FRAMES> m_command_buffers;

        std::array<VkSemaphore, MAX_IN_FLIGHT_FRAMES> m_image_available_semaphores;
        std::array<VkSemaphore, MAX_IN_FLIGHT_FRAMES> m_render_finished_semaphores;
        std::array<VkFence,     MAX_IN_FLIGHT_FRAMES> m_in_flight_fences;

        VkClearValue m_clear_color;

        constexpr static const std::array<const char*, 1> m_required_extensions
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VkPhysicalDeviceProperties device_properties(VkPhysicalDevice device);

        bool is_device_suitable(VkPhysicalDevice device);

        bool verify_device_extensions(VkPhysicalDevice device);

        VkShaderModule create_shader_module(std::string_view source);

        VulkanErrorCode create_instance();

        VulkanErrorCode create_surface();

        VulkanErrorCode set_physical_device();

        VulkanErrorCode set_logical_device();

        VulkanErrorCode create_swap_chain();

        VulkanErrorCode create_image_views();

        VulkanErrorCode create_graphics_pipeline();

        VulkanErrorCode create_render_pass();

        VulkanErrorCode create_frame_buffers();

        VulkanErrorCode create_command_pool();

        VulkanErrorCode create_command_buffer();

        VulkanErrorCode create_sync_objects();

        void cleanup_swap_chains();
    };
}
