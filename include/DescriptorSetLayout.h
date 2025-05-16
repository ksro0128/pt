#pragma once

#include "Common.h"
#include "VulkanContext.h"

class DescriptorSetLayout {
public:


	~DescriptorSetLayout();

	VkDescriptorSetLayout &getDescriptorSetLayout() { return m_layout; }

private:
	VulkanContext* context;
	VkDescriptorSetLayout m_layout;

	void cleanup();
};