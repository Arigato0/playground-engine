#include <span>
#include <algorithm>

#include "util.hpp"
#include "vulkan_manager.hpp"
#include "queue_families.hpp"
#include "swap_chain.hpp"
#include "../shader_interface.hpp"

pge::VulkanManager::~VulkanManager()
{
	util::destroy_all(m_device, vkDestroySemaphore, m_image_available_semaphores);
	util::destroy_all(m_device, vkDestroySemaphore, m_render_finished_semaphores);
	util::destroy_all(m_device, vkDestroyFence, m_in_flight_fences);

	vkDestroyCommandPool(m_device, m_command_pool, nullptr);
    cleanup_swap_chains();
  	vkDestroyPipeline(m_device, m_pipeline, nullptr);
  	vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
  	vkDestroyRenderPass(m_device, m_render_pass, nullptr);
    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

pge::VulkanErrorCode pge::VulkanManager::create_instance()
{
    VkApplicationInfo app_info
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "playground",
        .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .pEngineName = "playground engine",
        .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    VkInstanceCreateInfo create_info
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
    };

    uint32_t glfw_extensions_count {};

    auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

    create_info.enabledExtensionCount = glfw_extensions_count;
    create_info.ppEnabledExtensionNames = glfw_extensions;

    create_info.enabledLayerCount = 0;

    auto result = vkCreateInstance(&create_info, nullptr, &m_instance);

    return result == VK_SUCCESS ? VulkanErrorCode::Ok : VulkanErrorCode::VulkanInstanceError;
}

pge::VulkanErrorCode pge::VulkanManager::set_physical_device()
{
    auto devices = util::vk_enumerate<VkPhysicalDevice>(vkEnumeratePhysicalDevices, m_instance);

    if (devices.empty())
    {
        return VulkanErrorCode::NoPhysicalDevice;
    }

    for (auto &candidate : devices)
    {
        if (is_device_suitable(candidate))
        {
            m_physical_device = candidate;
            break;
        }
    }

    if (m_physical_device == nullptr)
    {
        return VulkanErrorCode::NoPhysicalDevice;
    }

    return VulkanErrorCode::Ok;
}

pge::VulkanErrorCode pge::VulkanManager::set_logical_device()
{
    VkPhysicalDeviceFeatures features {};

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

    auto indexes = m_queue_families.to_set();

    queue_create_infos.reserve(indexes.size());

    auto queue_priority = 1.0f;

    for (auto idx : indexes)
    {
        queue_create_infos.emplace_back(VkDeviceQueueCreateInfo
        {
             .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
             .queueFamilyIndex = idx,
             .queueCount = 1,
             .pQueuePriorities = &queue_priority,
         });
    }

   VkDeviceCreateInfo device_create_info
   {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = (uint32_t)queue_create_infos.size(),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledExtensionCount = m_required_extensions.size(),
        .ppEnabledExtensionNames = m_required_extensions.data(),
        .pEnabledFeatures = &features,
   };

    auto result = vkCreateDevice(m_physical_device, &device_create_info, nullptr, &m_device);

    vkGetDeviceQueue(m_device, m_queue_families.graphics.value(), 0, &m_graphics_q);
    vkGetDeviceQueue(m_device, m_queue_families.presentation.value(), 0, &m_presentation_q);

   return result == VK_SUCCESS ? VulkanErrorCode::Ok : VulkanErrorCode::VulkanCreateError;
}


uint8_t pge::VulkanManager::init()
{
    VALIDATE_ERR_AS(create_instance(), uint8_t);
    VALIDATE_ERR_AS(create_surface(), uint8_t);
    VALIDATE_ERR_AS(set_physical_device(), uint8_t);

	m_queue_families.find_families(m_physical_device, m_surface);

	if (!m_queue_families.is_complete())
	{
		return (uint8_t)VulkanErrorCode::IncompleteQueueFamily;
	}

    VALIDATE_ERR_AS(set_logical_device(), uint8_t);
    VALIDATE_ERR_AS(create_swap_chain(), uint8_t);
    VALIDATE_ERR_AS(create_image_views(), uint8_t);
	VALIDATE_ERR_AS(create_render_pass(), uint8_t);
    VALIDATE_ERR_AS(create_graphics_pipeline(), uint8_t);
	VALIDATE_ERR_AS(create_frame_buffers(), uint8_t);
	VALIDATE_ERR_AS(create_command_pool(), uint8_t);
	VALIDATE_ERR_AS(create_command_buffer(), uint8_t);
	VALIDATE_ERR_AS(create_sync_objects(), uint8_t);

	return (uint8_t)VulkanErrorCode::Ok;
}

bool pge::VulkanManager::is_device_suitable(VkPhysicalDevice device)
{
    auto properties = device_properties(device);

    return
    properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
    && verify_device_extensions(device)
    && is_swap_chain_supported(device, m_surface);
}

bool pge::VulkanManager::verify_device_extensions(VkPhysicalDevice device)
{
    auto available_extensions = util::vk_enumerate<VkExtensionProperties>(vkEnumerateDeviceExtensionProperties,
                                                                          device, nullptr);

    for (auto extension : available_extensions)
    {
        auto found = std::find(m_required_extensions.begin(), m_required_extensions.end(), extension.extensionName);

        if (!found)
        {
            return false;
        }
    }

    return true;
}

VkPhysicalDeviceProperties pge::VulkanManager::device_properties(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties properties;

    vkGetPhysicalDeviceProperties(device, &properties);

    return properties;
}

pge::VulkanErrorCode pge::VulkanManager::create_surface()
{
    auto result = glfwCreateWindowSurface(m_instance, (GLFWwindow*)m_window->handle(), nullptr, &m_surface);

    return result == VK_SUCCESS ? VulkanErrorCode::Ok : VulkanErrorCode::VulkanSurfaceError;
}

std::vector<const char *> pge::VulkanManager::extensions()
{
    uint32_t glfw_extensions_count {};

    auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

    std::vector<const char *> output;

    auto size = glfw_extensions_count + m_required_extensions.size();

    output.reserve(size);

    util::concat(output, glfw_extensions, glfw_extensions_count);
    util::concat(output, m_required_extensions);

    return output;
}

pge::VulkanErrorCode pge::VulkanManager::create_swap_chain()
{
    auto swap_data = get_swap_chain_data(m_physical_device, m_surface);
    auto extent = get_extent(swap_data.capabilities, m_window);

    auto image_count = std::min(swap_data.capabilities.minImageCount + 1, swap_data.capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR create_info
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_surface,
        .minImageCount = image_count,
        .imageFormat = swap_data.format.format,
        .imageColorSpace = swap_data.format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    };

    if (m_queue_families.graphics != m_queue_families.presentation)
    {
        std::array indices
        {
				m_queue_families.graphics.value(),
				m_queue_families.presentation.value()
        };

        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = indices.size();
        create_info.pQueueFamilyIndices = indices.data();
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }

    create_info.preTransform = swap_data.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;;
    create_info.presentMode = swap_data.present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    auto result = vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swap_chain);

    if (result != VK_SUCCESS)
    {
        return VulkanErrorCode::SwapChainCreationError;
    }

    m_images = util::vk_enumerate<VkImage>(vkGetSwapchainImagesKHR, m_device, m_swap_chain);
    m_format = swap_data.format.format;
    m_extent = extent;

    return VulkanErrorCode::Ok;
}

pge::VulkanErrorCode pge::VulkanManager::create_image_views()
{
    m_image_views.resize(m_images.size());

    for (int i = 0; i < m_images.size(); i++)
    {
        VkImageViewCreateInfo create_info
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_format,
            .components =
                {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
            .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }
        };

        auto result = vkCreateImageView(m_device, &create_info, nullptr, &m_image_views[i]);

        if (result != VK_SUCCESS)
        {
            return VulkanErrorCode::ImageViewCreationError;
        }
    }

    return VulkanErrorCode::Ok;
}

pge::VulkanErrorCode pge::VulkanManager::create_graphics_pipeline()
{
    auto vert_shader_src = util::read_file(PGE_FIND_SHADER("lighting.vert"));
    auto frag_shader_src = util::read_file(PGE_FIND_SHADER("shader.frag"));

    auto vert_shader_module = create_shader_module(vert_shader_src);
    auto frag_shader_module = create_shader_module(frag_shader_src);

    if (!util::any_true(vert_shader_module, frag_shader_module))
    {
        return VulkanErrorCode::ShaderModuleCreationError;
    }

    // TODO: can cause possible leak if the function exits early because only one shader module was not created
    DEFER([&]()
    {
        vkDestroyShaderModule(m_device, vert_shader_module, nullptr);
        vkDestroyShaderModule(m_device, frag_shader_module, nullptr);
    });

    VkPipelineShaderStageCreateInfo vert_stage_create_info
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vert_shader_module,
        .pName = "main"
    };

    VkPipelineShaderStageCreateInfo frag_stage_create_info
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = frag_shader_module,
        .pName = "main"
    };
	
	std::array shader_stages
	{
		frag_stage_create_info, vert_stage_create_info
	};

    std::vector dynamic_states
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state_ci
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = (uint32_t)dynamic_states.size(),
        .pDynamicStates = dynamic_states.data()
    };

    VkPipelineVertexInputStateCreateInfo vertext_input_state_ci
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 0,
		.pVertexBindingDescriptions = nullptr,
		.vertexAttributeDescriptionCount = 0,
		.pVertexAttributeDescriptions = nullptr,
	};

	VkPipelineInputAssemblyStateCreateInfo input_assembly_ci
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkPipelineViewportStateCreateInfo viewport_state_ci
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1,
	};

	VkPipelineRasterizationStateCreateInfo rasterization_state_ci
	{
  		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.f,
		.depthBiasClamp = 0.f,
		.depthBiasSlopeFactor = 0.f,
		.lineWidth = 1.f,
	};

	VkPipelineColorBlendAttachmentState color_blend_attachment
	{
		.blendEnable = VK_FALSE,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo color_blend_ci
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &color_blend_attachment,
	  	.blendConstants = {0.f, 0.f, 0.f, 0.f}
	};

	VkPipelineLayoutCreateInfo layout_ci
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 0,
		.pSetLayouts = nullptr,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = nullptr,
	};
 
	VkPipelineMultisampleStateCreateInfo multisampling_state_ci
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.f,
		.pSampleMask = nullptr,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE,
	};

	auto result = vkCreatePipelineLayout(m_device, &layout_ci, nullptr, &m_pipeline_layout);
	
	if (result != VK_SUCCESS)
	{
	  	return VulkanErrorCode::PipelineCreationError;
	}
	
	VkGraphicsPipelineCreateInfo graphics_pipeline_ci
	{
  		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = shader_stages.size(),
		.pStages = shader_stages.data(),
		.pVertexInputState = &vertext_input_state_ci,
		.pInputAssemblyState = &input_assembly_ci,
		.pViewportState = &viewport_state_ci,
		.pRasterizationState = &rasterization_state_ci,
		.pMultisampleState = &multisampling_state_ci,
		.pDepthStencilState = nullptr,
		.pColorBlendState = &color_blend_ci,
		.pDynamicState = &dynamic_state_ci,
		.layout = m_pipeline_layout,
		.renderPass = m_render_pass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1,
	};
	
	result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &graphics_pipeline_ci, nullptr, &m_pipeline);
	
	return result == VK_SUCCESS ? VulkanErrorCode::Ok : VulkanErrorCode::PipelineCreationError;
}

VkShaderModule pge::VulkanManager::create_shader_module(std::string_view source)
{
    VkShaderModuleCreateInfo create_info
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = source.size(),
        .pCode = (const uint32_t*)source.data()
    };

    VkShaderModule module;

    auto result = vkCreateShaderModule(m_device, &create_info, nullptr, &module);

    return result != VK_SUCCESS ? nullptr : module;
}

pge::VulkanErrorCode pge::VulkanManager::create_render_pass()
{
  	VkAttachmentDescription attachment_description
	{
		.format = m_format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentReference attachment_reference
	{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass_description
	{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &attachment_reference,
	};

	VkSubpassDependency dependency
	{
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	};

	VkRenderPassCreateInfo render_pass_ci
	{
  		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &attachment_description,
		.subpassCount = 1,
		.pSubpasses = &subpass_description,
		.dependencyCount = 1,
		.pDependencies = &dependency
	};

	auto result = vkCreateRenderPass(m_device, &render_pass_ci, nullptr, &m_render_pass);

	return result == VK_SUCCESS ? VulkanErrorCode::Ok : VulkanErrorCode::RenderPassCreationError;
}

pge::VulkanErrorCode pge::VulkanManager::create_frame_buffers()
{
	m_frame_buffers.resize(m_image_views.size());

	for (int i = 0; i < m_frame_buffers.size(); i++)
	{
		VkImageView image_View { m_image_views[i] };

		VkFramebufferCreateInfo frame_buffer_ci
		{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = m_render_pass,
			.attachmentCount = 1,
			.pAttachments = &image_View,
			.width = m_extent.width,
			.height = m_extent.height,
			.layers = 1,
		};

		auto result = vkCreateFramebuffer(m_device, &frame_buffer_ci, nullptr, &m_frame_buffers[i]);

		if (result != VK_SUCCESS)
		{
			return VulkanErrorCode::FrameBufferCreationError;
		}
	}

    return VulkanErrorCode::Ok;
}

pge::VulkanErrorCode pge::VulkanManager::create_command_pool()
{
	VkCommandPoolCreateInfo command_pool_ci
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = m_queue_families.graphics.value()
	};

	auto result = vkCreateCommandPool(m_device, &command_pool_ci, nullptr, &m_command_pool);

	return result == VK_SUCCESS ? VulkanErrorCode::Ok : VulkanErrorCode::CommandPoolCreationError;
}

pge::VulkanErrorCode pge::VulkanManager::create_command_buffer()
{
	VkCommandBufferAllocateInfo command_buffer_ai
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = (uint32_t)m_command_buffers.size(),
	};

	auto result = vkAllocateCommandBuffers(m_device, &command_buffer_ai, m_command_buffers.data());

	return result == VK_SUCCESS ? VulkanErrorCode::Ok : VulkanErrorCode::CommandBufferCreationError;
}

pge::VulkanErrorCode pge::VulkanManager::record_cmd(VkCommandBuffer cmd_buffer, size_t image_idx)
{
	VkCommandBufferBeginInfo command_buffer_bi
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	auto result = vkBeginCommandBuffer(cmd_buffer, &command_buffer_bi);

	if (result != VK_SUCCESS)
	{
		return VulkanErrorCode::CommandBufferRecordingError;
	}

	VkRenderPassBeginInfo render_pass_bi
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = m_render_pass,
		.framebuffer = m_frame_buffers[image_idx],
		.renderArea =
		{
			.offset = VkOffset2D{},
			.extent = m_extent,
		},
		.clearValueCount = 1,
		.pClearValues = &m_clear_color
	};

	vkCmdBeginRenderPass(cmd_buffer, &render_pass_bi, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

	VkViewport viewport
	{
	   .x = 0.f,
	   .y = 0.f,
	   .width = (float)m_extent.width,
	   .height = (float)m_extent.height,
	   .minDepth = 0.f,
	   .maxDepth = 1.f
	};

	VkRect2D scissor
	{
		 .offset = VkOffset2D{},
		 .extent = m_extent,
	};

	vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
	vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);
	vkCmdDraw(cmd_buffer, 3, 1, 0, 0);
	vkCmdEndRenderPass(cmd_buffer);


	result = vkEndCommandBuffer(cmd_buffer);

	return result == VK_SUCCESS ? VulkanErrorCode::Ok : VulkanErrorCode::CommandBufferRecordingError;
}

pge::VulkanErrorCode pge::VulkanManager::create_sync_objects()
{
	VALIDATE_ERR(util::create_semaphores(m_device, m_image_available_semaphores));
	VALIDATE_ERR(util::create_semaphores(m_device, m_render_finished_semaphores));
	VALIDATE_ERR(util::create_fences(m_device, m_in_flight_fences));

	return VulkanErrorCode::Ok;
}

uint8_t pge::VulkanManager::draw_frame()
{

	vkWaitForFences(m_device, 1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX);

	uint32_t image_idx;

	auto result = vkAcquireNextImageKHR(m_device, m_swap_chain, UINT64_MAX, m_image_available_semaphores[m_current_frame],
						  VK_NULL_HANDLE, &image_idx);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        recreate_swap_chain();
        return (uint8_t)VulkanErrorCode::Ok;
    }
    else if (result != VK_SUCCESS)
    {
        return (uint8_t)VulkanErrorCode::ImageAcquisitionError;
    }

    vkResetFences(m_device, 1, &m_in_flight_fences[m_current_frame]);

	result = vkResetCommandBuffer(m_command_buffers[m_current_frame], 0);

    if (result != VK_SUCCESS)
    {
        return (uint8_t)VulkanErrorCode::CommandBufferRecordingError;
    }

	VALIDATE_ERR_AS(record_cmd(m_command_buffers[m_current_frame], image_idx), uint8_t);

	VkPipelineStageFlags wait_stages[]
	{
	 	VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	VkSubmitInfo submit_info
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount =1,
		.pWaitSemaphores = &m_image_available_semaphores[m_current_frame],
		.pWaitDstStageMask = wait_stages,
		.commandBufferCount = 1,
		.pCommandBuffers = &m_command_buffers[m_current_frame],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &m_render_finished_semaphores[m_current_frame],
	};

	result = vkQueueSubmit(m_graphics_q, 1, &submit_info, m_in_flight_fences[m_current_frame]);

	if (result != VK_SUCCESS)
	{
		return (uint8_t)VulkanErrorCode::QueueSubmitError;
	}

	VkPresentInfoKHR present_info
	{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_render_finished_semaphores[m_current_frame],
		.swapchainCount = 1,
		.pSwapchains = &m_swap_chain,
		.pImageIndices = &image_idx,
	};

	result = vkQueuePresentKHR(m_presentation_q, &present_info);

	if (result != VK_SUCCESS)
	{
		return (uint8_t)VulkanErrorCode::PresentationError;
	}

	m_current_frame = (m_current_frame + 1) % MAX_IN_FLIGHT_FRAMES;


	return (uint8_t)VulkanErrorCode::Ok;
}

pge::VulkanErrorCode pge::VulkanManager::recreate_swap_chain()
{
    vkDeviceWaitIdle(m_device);

    cleanup_swap_chains();

    VALIDATE_ERR(create_swap_chain());
    VALIDATE_ERR(create_image_views());
    VALIDATE_ERR(create_frame_buffers());

    return VulkanErrorCode::Ok;
}

void pge::VulkanManager::cleanup_swap_chains()
{
    util::destroy_all(m_device, vkDestroyFramebuffer, m_frame_buffers);
	util::destroy_all(m_device, vkDestroyImageView, m_image_views);
	vkDestroySwapchainKHR(m_device, m_swap_chain, nullptr);
}