#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "DescriptorSetLayout.h"
#include "Texture.h"
#include "Buffer.h"

class DescriptorSet {
public:
	VkDescriptorSet& getDescriptorSet() { return m_descriptorSet; }

	~DescriptorSet();

private:
	VulkanContext* context;
	VkDescriptorSet m_descriptorSet;

	void cleanup();
};
