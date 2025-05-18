#pragma once

#include "Common.h"
#include "VulkanContext.h"

class DescriptorSetLayout {
public:
	static std::unique_ptr<DescriptorSetLayout> createSet0Layout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createSet1Layout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createSet2Layout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createSet3Layout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createSet4Layout(VulkanContext* context);
	static std::unique_ptr<DescriptorSetLayout> createSet5Layout(VulkanContext* context);

	~DescriptorSetLayout();

	VkDescriptorSetLayout &getDescriptorSetLayout() { return m_layout; }

private:
	VulkanContext* context;
	VkDescriptorSetLayout m_layout;

	void cleanup();
	void initSet0Layout(VulkanContext* context);
	void initSet1Layout(VulkanContext* context);
	void initSet2Layout(VulkanContext* context);
	void initSet3Layout(VulkanContext* context);
	void initSet4Layout(VulkanContext* context);
	void initSet5Layout(VulkanContext* context);
};