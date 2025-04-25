#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "DescriptorSetLayout.h"
#include "Texture.h"
#include "Buffer.h"

struct MaterialDescriptorResources {
	StorageBuffer* areaLightBuffer;
	StorageBuffer* materialBuffer;
	StorageBuffer* uberBuffer;
	StorageBuffer* matteBuffer;
	StorageBuffer* metalBuffer;
	StorageBuffer* glassBuffer;
	StorageBuffer* mirrorBuffer;
	StorageBuffer* substrateBuffer;
	StorageBuffer* plasticBuffer;
};

class DescriptorSet {
public:
	static std::unique_ptr<DescriptorSet> createGlobalDescriptorSet(
		VulkanContext* context,
		DescriptorSetLayout* layout,
		UniformBuffer* cameraBuffer,
		StorageBuffer* lightBuffer, 
		UniformBuffer* renderOptionsBuffer
	);

	static std::unique_ptr<DescriptorSet> createObjectMaterialDescriptorSet(
		VulkanContext* context,
		DescriptorSetLayout* layout,
		StorageBuffer* objectInstanceBuffer
	);

	static std::unique_ptr<DescriptorSet> createBindlessDescriptorSet(
		VulkanContext* context,
		DescriptorSetLayout* layout,
		StorageBuffer* modelBuffer,
		StorageBuffer* materialBuffer,
		const std::vector<std::unique_ptr<Texture>>& textureList
	);

	static std::unique_ptr<DescriptorSet> createAttachmentDescriptorSet(
		VulkanContext* context,
		DescriptorSetLayout* layout,
		GbufferAttachment& gbufferAttachment
	);

	static std::unique_ptr<DescriptorSet> createShadowDescriptorSet(
		VulkanContext* context,
		DescriptorSetLayout* layout,
		UniformBuffer* lightMatrixBuffer,
		std::vector<Texture*>& shadowMapTextures,
		Texture* shadowCubeMapTexture
	);

	static std::unique_ptr<DescriptorSet> createRayTracingDescriptorSet(
		VulkanContext* context,
		DescriptorSetLayout* layout,
		Texture* rtReflectionTexture,
		VkAccelerationStructureKHR tlas
	);

	static std::unique_ptr<DescriptorSet> createSet0DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		UniformBuffer* cameraBuffer, UniformBuffer* optionsBuffer);
	static std::unique_ptr<DescriptorSet> createSet1DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		StorageBuffer* instanceBuffer);
	static std::unique_ptr<DescriptorSet> createSet2DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		std::vector<StorageBuffer*> buffers);
	static std::unique_ptr<DescriptorSet> createSet3DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		std::vector<std::unique_ptr<Texture>>& textureList);
	static std::unique_ptr<DescriptorSet> createSet4DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		VkAccelerationStructureKHR tlas, Texture* pingTexture, Texture* pongTexture);
		

	VkDescriptorSet& getDescriptorSet() { return m_descriptorSet; }

	void updateTLAS(VkAccelerationStructureKHR tlas);

	~DescriptorSet();

private:
	VulkanContext* context;
	VkDescriptorSet m_descriptorSet;

	void initGlobal(VulkanContext* context, DescriptorSetLayout* layout, 
		UniformBuffer* cameraBuffer, StorageBuffer* lightBuffer, UniformBuffer* renderOptionsBuffer);
	void initObjectMaterial(VulkanContext* context, DescriptorSetLayout* layout,
		StorageBuffer* objectInstanceBuffer);
	void initBindless(VulkanContext* context, DescriptorSetLayout* layout, StorageBuffer* modelBuffer, StorageBuffer* materialBuffer,
		const std::vector<std::unique_ptr<Texture>>& textureList);
	void initAttachment(VulkanContext* context, DescriptorSetLayout* layout, GbufferAttachment& gbufferAttachment);
	void initShadow(VulkanContext* context, DescriptorSetLayout* layout, UniformBuffer* lightMatrixBuffer, std::vector<Texture*>& shadowMapTextures, Texture* shadowCubeMapTexture);
	void initRayTracing(VulkanContext* context, DescriptorSetLayout* layout, Texture* rtReflectionTexture, VkAccelerationStructureKHR tlas);
	
	void initSet0DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		UniformBuffer* cameraBuffer, UniformBuffer* optionsBuffer);
	void initSet1DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		StorageBuffer* instanceBuffer);
	void initSet2DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		std::vector<StorageBuffer*> buffers);
	void initSet3DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		std::vector<std::unique_ptr<Texture>>& textureList);
	void initSet4DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		VkAccelerationStructureKHR tlas, Texture* pingTexture, Texture* pongTexture);
	void cleanup();
};
