#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "RenderPass.h"
#include "DescriptorSetLayout.h"
#include "VulkanUtil.h"

class Pipeline {
public:
	static std::unique_ptr<Pipeline> createCompositePipeline(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);

	~Pipeline();
	VkPipeline getPipeline() { return m_pipeline; }
	VkPipelineLayout getPipelineLayout() { return m_pipelineLayout; }

private:
	VulkanContext* context;
	VkPipeline m_pipeline;
	VkPipelineLayout m_pipelineLayout;


	void cleanup();
	VkShaderModule createShaderModule(VulkanContext* context, const std::vector<char>& code);
	
	void initComposite(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
};