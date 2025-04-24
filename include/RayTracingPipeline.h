#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "DescriptorSetLayout.h"
#include "VulkanUtil.h"

class RayTracingPipeline {
public:
	static std::unique_ptr<RayTracingPipeline> createPtPipeline(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	~RayTracingPipeline();

	VkPipeline getPipeline() const { return m_pipeline; }
	VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; }
	VkStridedDeviceAddressRegionKHR getRaygenRegion() const { return m_raygenRegion; }
	VkStridedDeviceAddressRegionKHR getMissRegion() const { return m_missRegion; }
	VkStridedDeviceAddressRegionKHR getHitRegion() const { return m_hitRegion; }


private:
	VulkanContext* context;
	VkPipeline m_pipeline = VK_NULL_HANDLE;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

	VkBuffer m_sbtBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_sbtMemory = VK_NULL_HANDLE;

	VkStridedDeviceAddressRegionKHR m_raygenRegion{};
	VkStridedDeviceAddressRegionKHR m_missRegion{};
	VkStridedDeviceAddressRegionKHR m_hitRegion{};

	void cleanup();
	void initPt(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts);
	VkShaderModule createShaderModule(VulkanContext* context, const std::vector<char>& code);
};