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
		UniformBuffer* cameraBuffer, UniformBuffer* optionsBuffer, UniformBuffer* prevCameraBuffer, UniformBuffer* gbufferCameraBuffer);
	static std::unique_ptr<DescriptorSet> createSet1DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		StorageBuffer* instanceBuffer);
	static std::unique_ptr<DescriptorSet> createSet2DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		std::vector<StorageBuffer*> buffers);
	static std::unique_ptr<DescriptorSet> createSet3DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		std::vector<std::unique_ptr<Texture>>& textureList);
	static std::unique_ptr<DescriptorSet> createSet4DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		VkAccelerationStructureKHR tlas, Texture* directAccumHistoryTexture, Texture* directCurrentTexture, Texture* indirectAccumHistoryTexture, Texture* indirectCurrentTexture,
		Texture* directM1Texture, Texture* directM2Texture, Texture* indirectM1Texture, Texture* indirectM2Texture,
		Texture* directVarianceTexture, Texture* indirectVarianceTexture);
	static std::unique_ptr<DescriptorSet> createSet5DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		StorageBuffer* luminanceBuffer, Texture* texture);
	static std::unique_ptr<DescriptorSet> createSet6DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		Texture* hdrTexture, Texture* brightTexture, Texture* blurHTexture, Texture* blurVTexture, Texture* outputTexture);
	static std::unique_ptr<DescriptorSet> createSet7DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		Texture* inputTexture, Texture* outputTexture, Texture* historyTexture, Texture* varianceInputTexture, Texture* varianceOutputTexture);
	static std::unique_ptr<DescriptorSet> createSet8DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		Texture* normalTexture, Texture* depthTexture, Texture* albedoTexture, Texture* meshIDTexture, Texture* sampleCountTexture, Texture* motionVectorTexture,
		Texture* prevNormalTexture, Texture* prevDepthTexture, Texture* prevMeshIDTexture, Texture* jitterTexture, Texture* prevJitterTexture);
	static std::unique_ptr<DescriptorSet> createSet9DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		Texture* directFilterdTexture, Texture* indirectFilteredTexture, Texture* compositeTexture, Texture* prevCompositeTexture);
	static std::unique_ptr<DescriptorSet> createSet10DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		Texture* inputTexture, Texture* outputTexture);

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
		UniformBuffer* cameraBuffer, UniformBuffer* optionsBuffer, UniformBuffer* prevCameraBuffer, UniformBuffer* gbufferCameraBuffer);
	void initSet1DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		StorageBuffer* instanceBuffer);
	void initSet2DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		std::vector<StorageBuffer*> buffers);
	void initSet3DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		std::vector<std::unique_ptr<Texture>>& textureList);
	void initSet4DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		VkAccelerationStructureKHR tlas, Texture* directAccumHistoryTexture, Texture* directCurrentTexture, Texture* indirectAccumHistoryTexture, Texture* indirectCurrentTexture,
		Texture* directM1Texture, Texture* directM2Texture, Texture* indirectM1Texture, Texture* indirectM2Texture,
		Texture* directVarianceTexture, Texture* indirectVarianceTexture);
	void initSet5DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		StorageBuffer* exposureBuffer, Texture* texture);
	void initSet6DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		Texture* hdrTexture, Texture* brightTexture, Texture* blurHTexture, Texture* blurVTexture, Texture* outputTexture);
	void initSet7DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		Texture* inputTexture, Texture* outputTexture, Texture* historyTexture, Texture* varianceInputTexture, Texture* varianceOutputTexture);
	void initSet8DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		Texture* normalTexture, Texture* depthTexture, Texture* albedoTexture, Texture* meshIDTexture, Texture* sampleCountTexture, Texture* motionVectorTexture,
		Texture* prevNormalTexture, Texture* prevDepthTexture, Texture* prevMeshIDTexture, Texture* jitterTexture, Texture* prevJitterTexture);
	void initSet9DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		Texture* directFilterdTexture, Texture* indirectFilteredTexture, Texture* compositeTexture, Texture* prevCompositeTexture);
	void initSet10DescSet(VulkanContext* context, DescriptorSetLayout* layout,
		Texture* inputTexture, Texture* outputTexture);

	void cleanup();
};
