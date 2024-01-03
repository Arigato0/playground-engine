#include "vulkan_shader.hpp"
#include "../shader_interface.hpp"
#include "../../common_util/util.hpp"

namespace pge
{
	enum class ShaderType;
}

VkShaderModule create_shader_module(VkDevice device, std::string_view source)
{
  VkShaderModuleCreateInfo create_info
	  {
		  .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		  .codeSize = source.size(),
		  .pCode = (const uint32_t*)source.data()
	  };
  
  VkShaderModule module;
  
  auto result = vkCreateShaderModule(device, &create_info, nullptr, &module);
  
  return result != VK_SUCCESS ? nullptr : module;
}

inline constexpr VkShaderStageFlagBits get_vulkan_type(pge::ShaderType type)
{
  	using enum pge::ShaderType;
	  
  	switch (type)
	{
		case Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
		case Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
	}
}

std::vector<VkPipelineShaderStageCreateInfo> pge::create_shader_stages(VkDevice device,
	std::initializer_list<std::pair<const std::filesystem::path &,
													 ShaderType>> shaders
)
{
  	std::vector<VkPipelineShaderStageCreateInfo> output;
	  
	output.reserve(shaders.size());
	
	for (const auto &[shader_name, type] : shaders)
	{
	  	auto contents = util::read_file(shader_name);
		  
		auto module = create_shader_module(device, contents);
		
		output.emplace_back
		(
		  VkPipelineShaderStageCreateInfo
		  {
			 .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			 .stage = get_vulkan_type(type),
			 .module = module,
			 .pName = "main"
		});
	}
	
	return output;
}
