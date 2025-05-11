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

	static std::unique_ptr<DescriptorSetLayout> createSet0DescLayout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createSet1DescLayout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createSet2DescLayout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createSet3DescLayout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createSet4DescLayout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createSet5DescLayout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createSet6DescLayout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createSet7DescLayout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createSet8DescLayout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createSet9DescLayout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createSet10DescLayout(VulkanContext* context);

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
	
	void initSet0DescLayout(VulkanContext* context);
	void initSet1DescLayout(VulkanContext* context);
	void initSet2DescLayout(VulkanContext* context);
	void initSet3DescLayout(VulkanContext* context);
	void initSet4DescLayout(VulkanContext* context);
	void initSet5DescLayout(VulkanContext* context);
	void initSet6DescLayout(VulkanContext* context);
	void initSet7DescLayout(VulkanContext* context);
	void initSet8DescLayout(VulkanContext* context);
	void initSet9DescLayout(VulkanContext* context);
	void initSet10DescLayout(VulkanContext* context);
	void cleanup();
};