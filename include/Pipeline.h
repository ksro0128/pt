#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "RenderPass.h"
#include "DescriptorSetLayout.h"
#include "VulkanUtil.h"

class Pipeline {
public:
	static std::unique_ptr<Pipeline> createGbufferPipeline(VulkanContext* context, RenderPass* renderPass, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	static std::unique_ptr<Pipeline> createLightPassPipeline(VulkanContext* context, RenderPass* renderPass, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	static std::unique_ptr<Pipeline> createShadowMapPipeline(VulkanContext* context, RenderPass* renderPass, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	~Pipeline();
	VkPipeline getPipeline() { return m_pipeline; }
	VkPipelineLayout getPipelineLayout() { return m_pipelineLayout; }

private:
	VulkanContext* context;
	VkPipeline m_pipeline;
	VkPipelineLayout m_pipelineLayout;


	void cleanup();
	VkShaderModule createShaderModule(VulkanContext* context, const std::vector<char>& code);
	void initGbuffer(VulkanContext* context, RenderPass* renderPass, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	void initLightPass(VulkanContext* context, RenderPass* renderPass, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	void initShadowMap(VulkanContext* context, RenderPass* renderPass, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
};