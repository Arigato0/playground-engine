#pragma once

#include <initializer_list>
#include <utility>
#include <filesystem>
#include <vector>

#include "vulkan/vulkan.h"

#include "../shaders.hpp"

namespace pge
{

	std::vector<VkPipelineShaderStageCreateInfo> create_shader_stages(
		VkDevice device, std::initializer_list<std::pair<const std::filesystem::path &,
														 ShaderType>> shaders
	);
}