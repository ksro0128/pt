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
	static std::unique_ptr<Pipeline> createComputeExposurePipeline(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	static std::unique_ptr<Pipeline> createToneMappingPipeline(VulkanContext* context, RenderPass* renderPass, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	static std::unique_ptr<Pipeline> createThresholdPipeline(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	static std::unique_ptr<Pipeline> createBlurHPipeline(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	static std::unique_ptr<Pipeline> createBlurVPipeline(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	static std::unique_ptr<Pipeline> createCompositeBloomPipeline(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	static std::unique_ptr<Pipeline> createATorusFilterPipeline(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	static std::unique_ptr<Pipeline> createCompositePipeline(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	static std::unique_ptr<Pipeline> createGaussianBlurPipeline(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
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
	void initComputeExposure(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	void initToneMapping(VulkanContext* context, RenderPass* renderPass, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	void initThreshold(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	void initBlurH(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	void initBlurV(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	void initCompositeBloom(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	void initATorusFilter(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	void initComposite(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	void initGaussianBlur(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
};