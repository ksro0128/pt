#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "RenderPass.h"
#include "DescriptorSetLayout.h"
#include "VulkanUtil.h"

class Pipeline {
public:
	~Pipeline();
	VkPipeline getPipeline() { return m_pipeline; }
	VkPipelineLayout getPipelineLayout() { return m_pipelineLayout; }

private:
	VulkanContext* context;
	VkPipeline m_pipeline;
	VkPipelineLayout m_pipelineLayout;


	void cleanup();
	VkShaderModule createShaderModule(VulkanContext* context, const std::vector<char>& code);
	
};