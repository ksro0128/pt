#pragma once

#include "Common.h"
#include "VulkanContext.h"

class DescriptorSetLayout {
public:
	static std::unique_ptr<DescriptorSetLayout> createGlobalDescriptorSetLayout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createObjectMaterialDescriptorSetLayout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createBindlessDescriptorSetLayout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createAttachmentDescriptorSetLayout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createShadowDescriptorSetLayout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createRayTracingDescriptorSetLayout(VulkanContext* context);

	~DescriptorSetLayout();

	VkDescriptorSetLayout &getDescriptorSetLayout() { return m_layout; }

private:
	VulkanContext* context;
	VkDescriptorSetLayout m_layout;

	void initGlobal(VulkanContext* context);
	void initObjectMaterial(VulkanContext* context);
	void initBindless(VulkanContext* context);
	void initAttachment(VulkanContext* context);
	void initShadow(VulkanContext* context);
	void initRayTracing(VulkanContext* context);
	void cleanup();
};