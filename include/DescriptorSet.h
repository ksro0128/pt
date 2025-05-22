#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "DescriptorSetLayout.h"
#include "Texture.h"
#include "Buffer.h"

class DescriptorSet {
public:
	static std::unique_ptr<DescriptorSet> createSet0DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		UniformBuffer* cameraBuffer, UniformBuffer* optionsBuffer);
	static std::unique_ptr<DescriptorSet> createSet1DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		StorageBuffer* materialBuffer);
	static std::unique_ptr<DescriptorSet> createSet2DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		std::vector<std::unique_ptr<Texture>>& textures);
	static std::unique_ptr<DescriptorSet> createSet3DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		StorageBuffer* instanceBuffer, StorageBuffer* areaLightBuffer);
	VkDescriptorSet& getDescriptorSet() { return m_descriptorSet; }
	static std::unique_ptr<DescriptorSet> createSet4DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		VkAccelerationStructureKHR tlas);
	static std::unique_ptr<DescriptorSet> createSet5DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		Texture* output, Texture* accumPrev, Texture* accumCur);
	~DescriptorSet();

private:
	VulkanContext* context;
	VkDescriptorSet m_descriptorSet;

	void cleanup();
	void initSet0DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		UniformBuffer* cameraBuffer, UniformBuffer* optionsBuffer);
	void initSet1DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		StorageBuffer* materialBuffer);
	void initSet2DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		std::vector<std::unique_ptr<Texture>>& textures);
	void initSet3DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		StorageBuffer* instanceBuffer, StorageBuffer* areaLightBuffer);
	void initSet4DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		VkAccelerationStructureKHR tlas);
	void initSet5DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		Texture* output, Texture* accumPrev, Texture* accumCur);
};
