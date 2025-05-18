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


std::unique_ptr<Pipeline> Pipeline::createCompositePipeline(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts) {
	std::unique_ptr<Pipeline> pipeline = std::unique_ptr<Pipeline>(new Pipeline());
	pipeline->initComposite(context, descriptorSetLayouts);
	return pipeline;
}

void Pipeline::initComposite(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts) {
	this->context = context;

	auto shaderCode = VulkanUtil::readFile("spv/composite.comp.spv");
	VkShaderModule computeShaderModule = createShaderModule(context, shaderCode);

	VkPipelineShaderStageCreateInfo shaderStageInfo{};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderStageInfo.module = computeShaderModule;
	shaderStageInfo.pName = "main";

	std::vector<VkDescriptorSetLayout> layouts;
	for (auto* setLayout : descriptorSetLayouts) {
		layouts.push_back(setLayout->getDescriptorSetLayout());
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
	pipelineLayoutInfo.pSetLayouts = layouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(context->getDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create Composite pipeline layout!");
	}

	VkComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.stage = shaderStageInfo;
	pipelineInfo.layout = m_pipelineLayout;

	if (vkCreateComputePipelines(context->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create Composite compute pipeline!");
	}

	vkDestroyShaderModule(context->getDevice(), computeShaderModule, nullptr);
}




