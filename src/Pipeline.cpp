#include "include/Pipeline.h"

Pipeline::~Pipeline() {
	cleanup();
}

void Pipeline::cleanup() {
	std::cout << "Pipeline::cleanup" << std::endl;
	if (m_pipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(context->getDevice(), m_pipeline, nullptr);
		m_pipeline = VK_NULL_HANDLE;
	}
	if (m_pipelineLayout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(context->getDevice(), m_pipelineLayout, nullptr);
		m_pipelineLayout = VK_NULL_HANDLE;
	}
}

VkShaderModule Pipeline::createShaderModule(VulkanContext* context, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());


	VkShaderModule shaderModule;
	if (vkCreateShaderModule(context->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

