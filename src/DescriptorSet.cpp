#include "include/DescriptorSet.h"

DescriptorSet::~DescriptorSet() {
	cleanup();
}

void DescriptorSet::cleanup() {
	std::cout << "DescriptorSet::cleanup" << std::endl;

	if (m_descriptorSet != VK_NULL_HANDLE) {
		vkFreeDescriptorSets(context->getDevice(), context->getDescriptorPool(), 1, &m_descriptorSet);
		m_descriptorSet = VK_NULL_HANDLE;
	}
}

std::unique_ptr<DescriptorSet> DescriptorSet::createGlobalDescriptorSet(VulkanContext* context, DescriptorSetLayout* layout, 
	UniformBuffer* cameraBuffer, StorageBuffer* lightBuffer, UniformBuffer* renderOptionsBuffer) {
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descriptorSet->initGlobal(context, layout, cameraBuffer, lightBuffer, renderOptionsBuffer);
	return descriptorSet;
}

void DescriptorSet::initGlobal(VulkanContext* context, DescriptorSetLayout* layout,
	UniformBuffer* cameraBuffer, StorageBuffer* lightBuffer, UniformBuffer* renderOptionsBuffer)
{
	this->context = context;

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = context->getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	VkDescriptorSetLayout& vkLayout = layout->getDescriptorSetLayout();
	allocInfo.pSetLayouts = &vkLayout;

	if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate global descriptor set!");
	}

	std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

	// Binding 0: Camera (UBO)
	VkDescriptorBufferInfo cameraBufferInfo{};
	cameraBufferInfo.buffer = cameraBuffer->getBuffer();
	cameraBufferInfo.offset = 0;
	cameraBufferInfo.range = sizeof(CameraBuffer);

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = m_descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &cameraBufferInfo;

	// Binding 1: Light (SSBO)
	VkDescriptorBufferInfo lightBufferInfo{};
	lightBufferInfo.buffer = lightBuffer->getBuffer();
	lightBufferInfo.offset = 0;
	lightBufferInfo.range = lightBuffer->getCurrentSize();

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = m_descriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pBufferInfo = &lightBufferInfo;

	// Binding 2: RenderOptions (UBO)
	VkDescriptorBufferInfo optionsBufferInfo{};
	optionsBufferInfo.buffer = renderOptionsBuffer->getBuffer();
	optionsBufferInfo.offset = 0;
	optionsBufferInfo.range = sizeof(RenderOptions);

	descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[2].dstSet = m_descriptorSet;
	descriptorWrites[2].dstBinding = 2;
	descriptorWrites[2].dstArrayElement = 0;
	descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[2].descriptorCount = 1;
	descriptorWrites[2].pBufferInfo = &optionsBufferInfo;

	vkUpdateDescriptorSets(context->getDevice(),
		static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

std::unique_ptr<DescriptorSet> DescriptorSet::createObjectMaterialDescriptorSet(VulkanContext* context, DescriptorSetLayout* layout,
	StorageBuffer* objectInstanceBuffer)
{
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descriptorSet->initObjectMaterial(context, layout, objectInstanceBuffer);
	return descriptorSet;
}

void DescriptorSet::initObjectMaterial(VulkanContext* context, DescriptorSetLayout* layout,
	StorageBuffer* objectInstanceBuffer)
{
	this->context = context;

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = context->getDescriptorPool();
	allocInfo.descriptorSetCount = 1;

	VkDescriptorSetLayout vkLayout = layout->getDescriptorSetLayout();
	allocInfo.pSetLayouts = &vkLayout;

	if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate object descriptor set!");
	}

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = objectInstanceBuffer->getBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = objectInstanceBuffer->getCurrentSize();

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = m_descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(context->getDevice(), 1, &descriptorWrite, 0, nullptr);
}

std::unique_ptr<DescriptorSet> DescriptorSet::createBindlessDescriptorSet(
	VulkanContext* context, DescriptorSetLayout* layout, StorageBuffer* modelBuffer, StorageBuffer* materialBuffer,
	const std::vector<std::unique_ptr<Texture>>& textureList)
{
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descriptorSet->initBindless(context, layout, modelBuffer, materialBuffer, textureList);
	return descriptorSet;
}

void DescriptorSet::initBindless(VulkanContext* context, DescriptorSetLayout* layout, StorageBuffer* modelBuffer, StorageBuffer* materialBuffer,
	const std::vector<std::unique_ptr<Texture>>& textureList)
{
	this->context = context;

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = context->getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	VkDescriptorSetLayout vkLayout = layout->getDescriptorSetLayout();
	allocInfo.pSetLayouts = &vkLayout;

	if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate bindless descriptor set!");
	}

	VkDescriptorBufferInfo modelBufferInfo{};
	modelBufferInfo.buffer = modelBuffer->getBuffer();
	modelBufferInfo.offset = 0;
	modelBufferInfo.range = modelBuffer->getCurrentSize();

	VkWriteDescriptorSet modelWrite{};
	modelWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	modelWrite.dstSet = m_descriptorSet;
	modelWrite.dstBinding = 0;
	modelWrite.dstArrayElement = 0;
	modelWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	modelWrite.descriptorCount = 1;
	modelWrite.pBufferInfo = &modelBufferInfo;

	VkDescriptorBufferInfo materialBufferInfo{};
	materialBufferInfo.buffer = materialBuffer->getBuffer();
	materialBufferInfo.offset = 0;
	materialBufferInfo.range = materialBuffer->getCurrentSize();

	VkWriteDescriptorSet materialWrite{};
	materialWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	materialWrite.dstSet = m_descriptorSet;
	materialWrite.dstBinding = 1;
	materialWrite.dstArrayElement = 0;
	materialWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	materialWrite.descriptorCount = 1;
	materialWrite.pBufferInfo = &materialBufferInfo;

	std::vector<VkDescriptorImageInfo> imageInfos;
	imageInfos.reserve(MAX_TEXTURE_COUNT);

	for (size_t i = 0; i < textureList.size(); ++i) {
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureList[i]->getImageView();
		imageInfo.sampler = textureList[i]->getSampler();

		if (imageInfo.imageView == VK_NULL_HANDLE || imageInfo.sampler == VK_NULL_HANDLE) {
			std::cerr << "[BindlessDescriptor] Texture[" << i << "] has null handle! Skipping.\n";
			return;
		}
		imageInfos.push_back(imageInfo);
	}

	while (imageInfos.size() < MAX_TEXTURE_COUNT) {
		VkDescriptorImageInfo dummy{};
		dummy.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		dummy.imageView = textureList[0]->getImageView();
		dummy.sampler = textureList[0]->getSampler();
		imageInfos.push_back(dummy);
	}

	VkWriteDescriptorSet textureWrite{};
	textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	textureWrite.dstSet = m_descriptorSet;
	textureWrite.dstBinding = 2;
	textureWrite.dstArrayElement = 0;
	textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureWrite.descriptorCount = static_cast<uint32_t>(imageInfos.size());
	textureWrite.pImageInfo = imageInfos.data();

	std::array<VkWriteDescriptorSet, 3> writes = { modelWrite, materialWrite, textureWrite };

	vkUpdateDescriptorSets(context->getDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}


std::unique_ptr<DescriptorSet> DescriptorSet::createAttachmentDescriptorSet(VulkanContext* context, DescriptorSetLayout* layout, GbufferAttachment& gbufferAttachment) {
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descriptorSet->initAttachment(context, layout, gbufferAttachment);
	return descriptorSet;
}

void DescriptorSet::initAttachment(VulkanContext* context, DescriptorSetLayout* layout, GbufferAttachment& gbufferAttachment) {
	this->context = context;

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = context->getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	VkDescriptorSetLayout vkLayout = layout->getDescriptorSetLayout();
	allocInfo.pSetLayouts = &vkLayout;

	if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate attachment descriptor set!");
	}

	std::array<VkDescriptorImageInfo, 5> imageInfos{};

	imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfos[0].imageView = gbufferAttachment.position->getImageView();
	imageInfos[0].sampler = gbufferAttachment.position->getSampler();

	imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfos[1].imageView = gbufferAttachment.normal->getImageView();
	imageInfos[1].sampler = gbufferAttachment.normal->getSampler();

	imageInfos[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfos[2].imageView = gbufferAttachment.albedo->getImageView();
	imageInfos[2].sampler = gbufferAttachment.albedo->getSampler();

	imageInfos[3].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfos[3].imageView = gbufferAttachment.pbr->getImageView();
	imageInfos[3].sampler = gbufferAttachment.pbr->getSampler();

	imageInfos[4].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfos[4].imageView = gbufferAttachment.emissive->getImageView();
	imageInfos[4].sampler = gbufferAttachment.emissive->getSampler();

	std::array<VkWriteDescriptorSet, 5> descriptorWrites{};
	for (uint32_t i = 0; i < 5; ++i) {
		descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[i].dstSet = m_descriptorSet;
		descriptorWrites[i].dstBinding = i;
		descriptorWrites[i].dstArrayElement = 0;
		descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[i].descriptorCount = 1;
		descriptorWrites[i].pImageInfo = &imageInfos[i];
	}

	vkUpdateDescriptorSets(context->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	
}

std::unique_ptr<DescriptorSet> DescriptorSet::createShadowDescriptorSet(VulkanContext* context, DescriptorSetLayout* layout,
	UniformBuffer* lightMatrixBuffer, std::vector<Texture*>& shadowMapTextures, Texture* shadowCubeMapTexture) {
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descriptorSet->initShadow(context, layout, lightMatrixBuffer, shadowMapTextures, shadowCubeMapTexture);
	return descriptorSet;
}

void DescriptorSet::initShadow(VulkanContext* context, DescriptorSetLayout* layout,
	UniformBuffer* lightMatrixBuffer, std::vector<Texture*>& shadowMapTextures, Texture* shadowCubeMapTexture) {
	this->context = context;

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = context->getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout->getDescriptorSetLayout();

	if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate shadow descriptor set!");
	}

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = lightMatrixBuffer->getBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(LightMatrix) * 13;

	VkWriteDescriptorSet uboWrite{};
	uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uboWrite.dstSet = m_descriptorSet;
	uboWrite.dstBinding = 0;
	uboWrite.dstArrayElement = 0;
	uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboWrite.descriptorCount = 1;
	uboWrite.pBufferInfo = &bufferInfo;

	std::vector<VkDescriptorImageInfo> imageInfos(shadowMapTextures.size());

	for (size_t i = 0; i < shadowMapTextures.size(); ++i) {
		imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfos[i].imageView = shadowMapTextures[i]->getImageView();
		imageInfos[i].sampler = shadowMapTextures[i]->getSampler();
	}

	VkWriteDescriptorSet imageWrite{};
	imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	imageWrite.dstSet = m_descriptorSet;
	imageWrite.dstBinding = 1;
	imageWrite.dstArrayElement = 0;
	imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	imageWrite.descriptorCount = static_cast<uint32_t>(imageInfos.size());
	imageWrite.pImageInfo = imageInfos.data();

	VkDescriptorImageInfo cubeMapInfo{};
	cubeMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	cubeMapInfo.imageView = shadowCubeMapTexture->getImageView();
	cubeMapInfo.sampler = shadowCubeMapTexture->getSampler();

	VkWriteDescriptorSet cubeMapWrite{};
	cubeMapWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	cubeMapWrite.dstSet = m_descriptorSet;
	cubeMapWrite.dstBinding = 2;
	cubeMapWrite.dstArrayElement = 0;
	cubeMapWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	cubeMapWrite.descriptorCount = 1;
	cubeMapWrite.pImageInfo = &cubeMapInfo;


	std::array<VkWriteDescriptorSet, 3> writes = { uboWrite, imageWrite, cubeMapWrite };
	vkUpdateDescriptorSets(context->getDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

std::unique_ptr<DescriptorSet> DescriptorSet::createRayTracingDescriptorSet(VulkanContext* context, DescriptorSetLayout* layout,
	Texture* rtReflectionTexture, VkAccelerationStructureKHR tlas) {
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descriptorSet->initRayTracing(context, layout, rtReflectionTexture, tlas);
	return descriptorSet;
}	

void DescriptorSet::initRayTracing(VulkanContext* context, DescriptorSetLayout* layout,
	Texture* rtReflectionTexture, VkAccelerationStructureKHR tlas) {
	this->context = context;

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = context->getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	VkDescriptorSetLayout vkLayout = layout->getDescriptorSetLayout();
	allocInfo.pSetLayouts = &vkLayout;

	if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate ray tracing descriptor set!");
	}

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageInfo.imageView = rtReflectionTexture->getImageView();
	imageInfo.sampler = VK_NULL_HANDLE;

	VkWriteDescriptorSet imageWrite{};
	imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	imageWrite.dstSet = m_descriptorSet;
	imageWrite.dstBinding = 0;
	imageWrite.dstArrayElement = 0;
	imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	imageWrite.descriptorCount = 1;
	imageWrite.pImageInfo = &imageInfo;

	VkWriteDescriptorSetAccelerationStructureKHR accelWriteInfo{};
	accelWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	accelWriteInfo.accelerationStructureCount = 1;
	accelWriteInfo.pAccelerationStructures = &tlas;

	VkWriteDescriptorSet accelWrite{};
	accelWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	accelWrite.dstSet = m_descriptorSet;
	accelWrite.dstBinding = 1;
	accelWrite.dstArrayElement = 0;
	accelWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	accelWrite.descriptorCount = 1;
	accelWrite.pNext = &accelWriteInfo;

	std::array<VkWriteDescriptorSet, 2> descriptorWrites = { imageWrite, accelWrite };

	vkUpdateDescriptorSets(context->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void DescriptorSet::updateTLAS(VkAccelerationStructureKHR tlas) {
	VkWriteDescriptorSetAccelerationStructureKHR accelWriteInfo{};
	accelWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	accelWriteInfo.accelerationStructureCount = 1;
	accelWriteInfo.pAccelerationStructures = &tlas;
	VkWriteDescriptorSet accelWrite{};

	accelWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	accelWrite.dstSet = m_descriptorSet;
	accelWrite.dstBinding = 1;
	accelWrite.dstArrayElement = 0;
	accelWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	accelWrite.descriptorCount = 1;
	accelWrite.pNext = &accelWriteInfo;
	vkUpdateDescriptorSets(context->getDevice(), 1, &accelWrite, 0, nullptr);
}

std::unique_ptr<DescriptorSet> DescriptorSet::createSet0DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	UniformBuffer* cameraBuffer, UniformBuffer* optionsBuffer) {
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descriptorSet->initSet0DescSet(context, layout, cameraBuffer, optionsBuffer);
	return descriptorSet;
}

void DescriptorSet::initSet0DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	UniformBuffer* cameraBuffer, UniformBuffer* optionsBuffer) {
	this->context = context;

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = context->getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	VkDescriptorSetLayout vkLayout = layout->getDescriptorSetLayout();
	allocInfo.pSetLayouts = &vkLayout;

	if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate set0 descriptor set!");
	}

	VkDescriptorBufferInfo cameraBufferInfo{};
	cameraBufferInfo.buffer = cameraBuffer->getBuffer();
	cameraBufferInfo.offset = 0;
	cameraBufferInfo.range = sizeof(CameraGPU);

	VkWriteDescriptorSet cameraWrite{};
	cameraWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	cameraWrite.dstSet = m_descriptorSet;
	cameraWrite.dstBinding = 0;
	cameraWrite.dstArrayElement = 0;
	cameraWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraWrite.descriptorCount = 1;
	cameraWrite.pBufferInfo = &cameraBufferInfo;

	VkDescriptorBufferInfo optionsBufferInfo{};
	optionsBufferInfo.buffer = optionsBuffer->getBuffer();
	optionsBufferInfo.offset = 0;
	optionsBufferInfo.range = sizeof(OptionsGPU);

	VkWriteDescriptorSet optionsWrite{};
	optionsWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	optionsWrite.dstSet = m_descriptorSet;
	optionsWrite.dstBinding = 1;
	optionsWrite.dstArrayElement = 0;
	optionsWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	optionsWrite.descriptorCount = 1;
	optionsWrite.pBufferInfo = &optionsBufferInfo;

	std::array<VkWriteDescriptorSet, 2> writes{ cameraWrite, optionsWrite };
	vkUpdateDescriptorSets(context->getDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

std::unique_ptr<DescriptorSet> DescriptorSet::createSet1DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	StorageBuffer* instanceBuffer) {
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descriptorSet->initSet1DescSet(context, layout, instanceBuffer);
	return descriptorSet;
}

void DescriptorSet::initSet1DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	StorageBuffer* instanceBuffer) {
	this->context = context;

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = context->getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	VkDescriptorSetLayout vkLayout = layout->getDescriptorSetLayout();
	allocInfo.pSetLayouts = &vkLayout;

	if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate set1 descriptor set!");
	}

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = instanceBuffer->getBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = instanceBuffer->getCurrentSize();

	VkWriteDescriptorSet bufferWrite{};
	bufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	bufferWrite.dstSet = m_descriptorSet;
	bufferWrite.dstBinding = 0;
	bufferWrite.dstArrayElement = 0;
	bufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bufferWrite.descriptorCount = 1;
	bufferWrite.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(context->getDevice(), 1, &bufferWrite, 0, nullptr);
}

std::unique_ptr<DescriptorSet> DescriptorSet::createSet3DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	std::vector<std::unique_ptr<Texture>>& textureList) {
	
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	
	descriptorSet->initSet3DescSet(context, layout, textureList);
	
	return descriptorSet;
}

void DescriptorSet::initSet3DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	std::vector<std::unique_ptr<Texture>>& textureList) {
	
		this->context = context;

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = context->getDescriptorPool();
		allocInfo.descriptorSetCount = 1;
		VkDescriptorSetLayout setLayout = layout->getDescriptorSetLayout();
		allocInfo.pSetLayouts = &setLayout;
	
		if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor set (Set2 - textures)!");
		}
	
		std::vector<VkDescriptorImageInfo> imageInfos;
	
		for (size_t i = 0; i < textureList.size(); ++i) {
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = textureList[i]->getImageView();
			imageInfo.sampler = textureList[i]->getSampler();
			imageInfos.push_back(imageInfo);
		}
	\
		for (size_t i = textureList.size(); i < MAX_TEXTURE_COUNT; ++i) {
			VkDescriptorImageInfo dummy{};
			dummy.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			dummy.imageView = textureList[0]->getImageView();
			dummy.sampler = textureList[0]->getSampler();
			imageInfos.push_back(dummy);
		}
	
		VkWriteDescriptorSet textureWrite{};
		textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		textureWrite.dstSet = m_descriptorSet;
		textureWrite.dstBinding = 0;
		textureWrite.dstArrayElement = 0;
		textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureWrite.descriptorCount = static_cast<uint32_t>(imageInfos.size());
		textureWrite.pImageInfo = imageInfos.data();
	
		vkUpdateDescriptorSets(
			context->getDevice(),
			1, &textureWrite,
			0, nullptr
		);
}

std::unique_ptr<DescriptorSet> DescriptorSet::createSet2DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	std::vector<StorageBuffer*> buffers) {
	
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	
	descriptorSet->initSet2DescSet(context, layout, buffers);
	
	return descriptorSet;
}

void DescriptorSet::initSet2DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	std::vector<StorageBuffer*> buffers) {
	
		this->context = context;

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = context->getDescriptorPool();
		allocInfo.descriptorSetCount = 1;
		VkDescriptorSetLayout setLayout = layout->getDescriptorSetLayout();
		allocInfo.pSetLayouts = &setLayout;
	
		if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor set (Set3 - materials)!");
		}
	
		std::vector<VkDescriptorBufferInfo> bufferInfos(buffers.size());
		std::vector<VkWriteDescriptorSet>   descriptorWrites(buffers.size());

    for (size_t i = 0; i < buffers.size(); ++i) {
        bufferInfos[i].buffer = buffers[i]->getBuffer();
        bufferInfos[i].offset = 0;
        bufferInfos[i].range  = buffers[i]->getCurrentSize();

        descriptorWrites[i].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[i].dstSet          = m_descriptorSet;
        descriptorWrites[i].dstBinding      = static_cast<uint32_t>(i);
        descriptorWrites[i].dstArrayElement = 0;
        descriptorWrites[i].descriptorCount = 1;
        descriptorWrites[i].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[i].pBufferInfo     = &bufferInfos[i];
    }

    vkUpdateDescriptorSets(context->getDevice(),
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(),
                           0, nullptr);
}



std::unique_ptr<DescriptorSet> DescriptorSet::createSet4DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	VkAccelerationStructureKHR tlas, Texture* directAccumHistoryTexture, Texture* directCurrentTexture, Texture* indirectAccumHistoryTexture, Texture* indirectCurrentTexture,
	Texture* directM1Texture, Texture* directM2Texture, Texture* indirectM1Texture, Texture* indirectM2Texture,
	Texture* directVarianceTexture, Texture* indirectVarianceTexture) {
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descriptorSet->initSet4DescSet(context, layout, tlas, directAccumHistoryTexture, directCurrentTexture, indirectAccumHistoryTexture, indirectCurrentTexture,
		directM1Texture, directM2Texture, indirectM1Texture, indirectM2Texture,
		directVarianceTexture, indirectVarianceTexture);
	return descriptorSet;
}

void DescriptorSet::initSet4DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	VkAccelerationStructureKHR tlas, Texture* directAccumHistoryTexture, Texture* directCurrentTexture, Texture* indirectAccumHistoryTexture, Texture* indirectCurrentTexture,
	Texture* directM1Texture, Texture* directM2Texture, Texture* indirectM1Texture, Texture* indirectM2Texture,
	Texture* directVarianceTexture, Texture* indirectVarianceTexture) {
		this->context = context;

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = context->getDescriptorPool();
		allocInfo.descriptorSetCount = 1;
		VkDescriptorSetLayout setLayout = layout->getDescriptorSetLayout();
		allocInfo.pSetLayouts = &setLayout;
	
		if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor set (Set4)!");
		}
	
		
		std::vector<VkWriteDescriptorSet> descriptorWrites;

		// Binding 0: TLAS
		VkWriteDescriptorSetAccelerationStructureKHR asInfo{};
		asInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		asInfo.accelerationStructureCount = 1;
		asInfo.pAccelerationStructures = &tlas;
	
		VkWriteDescriptorSet asWrite{};
		asWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		asWrite.pNext = &asInfo;
		asWrite.dstSet = m_descriptorSet;
		asWrite.dstBinding = 0;
		asWrite.dstArrayElement = 0;
		asWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		asWrite.descriptorCount = 1;
	
		descriptorWrites.push_back(asWrite);
	
		// Binding 1: directAccumTexture
		VkDescriptorImageInfo directAccumInfo{};
		directAccumInfo.sampler = VK_NULL_HANDLE;
		directAccumInfo.imageView = directAccumHistoryTexture->getImageView();
		directAccumInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	
		VkWriteDescriptorSet directAccumWrite{};
		directAccumWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		directAccumWrite.dstSet = m_descriptorSet;
		directAccumWrite.dstBinding = 1;
		directAccumWrite.dstArrayElement = 0;
		directAccumWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		directAccumWrite.descriptorCount = 1;
		directAccumWrite.pImageInfo = &directAccumInfo;
	
		descriptorWrites.push_back(directAccumWrite);
	
		// Binding 2: directOutputTexture
		VkDescriptorImageInfo directOutputInfo{};
		directOutputInfo.sampler = VK_NULL_HANDLE;
		directOutputInfo.imageView = directCurrentTexture->getImageView();
		directOutputInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	
		VkWriteDescriptorSet directOutputWrite{};
		directOutputWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		directOutputWrite.dstSet = m_descriptorSet;
		directOutputWrite.dstBinding = 2;
		directOutputWrite.dstArrayElement = 0;
		directOutputWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		directOutputWrite.descriptorCount = 1;
		directOutputWrite.pImageInfo = &directOutputInfo;
	
		descriptorWrites.push_back(directOutputWrite);
	
		// Binding 3: indirectAccumTexture
		VkDescriptorImageInfo indirectAccumInfo{};
		indirectAccumInfo.sampler = VK_NULL_HANDLE;
		indirectAccumInfo.imageView = indirectAccumHistoryTexture->getImageView();
		indirectAccumInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	
		VkWriteDescriptorSet indirectAccumWrite{};
		indirectAccumWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		indirectAccumWrite.dstSet = m_descriptorSet;
		indirectAccumWrite.dstBinding = 3;
		indirectAccumWrite.dstArrayElement = 0;
		indirectAccumWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		indirectAccumWrite.descriptorCount = 1;
		indirectAccumWrite.pImageInfo = &indirectAccumInfo;
	
		descriptorWrites.push_back(indirectAccumWrite);
	
		// Binding 4: indirectOutputTexture
		VkDescriptorImageInfo indirectOutputInfo{};
		indirectOutputInfo.sampler = VK_NULL_HANDLE;
		indirectOutputInfo.imageView = indirectCurrentTexture->getImageView();
		indirectOutputInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	
		VkWriteDescriptorSet indirectOutputWrite{};
		indirectOutputWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		indirectOutputWrite.dstSet = m_descriptorSet;
		indirectOutputWrite.dstBinding = 4;
		indirectOutputWrite.dstArrayElement = 0;
		indirectOutputWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		indirectOutputWrite.descriptorCount = 1;
		indirectOutputWrite.pImageInfo = &indirectOutputInfo;
	
		descriptorWrites.push_back(indirectOutputWrite);

		// Binding 5: directM1Texture
		VkDescriptorImageInfo directM1Info{};
		directM1Info.sampler = VK_NULL_HANDLE;
		directM1Info.imageView = directM1Texture->getImageView();
		directM1Info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkWriteDescriptorSet directM1Write{};
		directM1Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		directM1Write.dstSet = m_descriptorSet;
		directM1Write.dstBinding = 5;
		directM1Write.dstArrayElement = 0;
		directM1Write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		directM1Write.descriptorCount = 1;
		directM1Write.pImageInfo = &directM1Info;
	
		descriptorWrites.push_back(directM1Write);

		// Binding 6: directM2Texture
		VkDescriptorImageInfo directM2Info{};
		directM2Info.sampler = VK_NULL_HANDLE;
		directM2Info.imageView = directM2Texture->getImageView();
		directM2Info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		
		VkWriteDescriptorSet directM2Write{};
		directM2Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		directM2Write.dstSet = m_descriptorSet;
		directM2Write.dstBinding = 6;
		directM2Write.dstArrayElement = 0;
		directM2Write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		directM2Write.descriptorCount = 1;
		directM2Write.pImageInfo = &directM2Info;

		descriptorWrites.push_back(directM2Write);

		// Binding 7: indirectM1Texture
		VkDescriptorImageInfo indirectM1Info{};
		indirectM1Info.sampler = VK_NULL_HANDLE;
		indirectM1Info.imageView = indirectM1Texture->getImageView();
		indirectM1Info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkWriteDescriptorSet indirectM1Write{};
		indirectM1Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		indirectM1Write.dstSet = m_descriptorSet;
		indirectM1Write.dstBinding = 7;
		indirectM1Write.dstArrayElement = 0;
		indirectM1Write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		indirectM1Write.descriptorCount = 1;
		indirectM1Write.pImageInfo = &indirectM1Info;

		descriptorWrites.push_back(indirectM1Write);

		// Binding 8: indirectM2Texture
		VkDescriptorImageInfo indirectM2Info{};
		indirectM2Info.sampler = VK_NULL_HANDLE;
		indirectM2Info.imageView = indirectM2Texture->getImageView();
		indirectM2Info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkWriteDescriptorSet indirectM2Write{};
		indirectM2Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		indirectM2Write.dstSet = m_descriptorSet;
		indirectM2Write.dstBinding = 8;
		indirectM2Write.dstArrayElement = 0;
		indirectM2Write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		indirectM2Write.descriptorCount = 1;
		indirectM2Write.pImageInfo = &indirectM2Info;

		descriptorWrites.push_back(indirectM2Write);

		// Binding 9: directVarianceTexture
		VkDescriptorImageInfo directVarianceInfo{};
		directVarianceInfo.sampler = VK_NULL_HANDLE;
		directVarianceInfo.imageView = directVarianceTexture->getImageView();
		directVarianceInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkWriteDescriptorSet directVarianceWrite{};
		directVarianceWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		directVarianceWrite.dstSet = m_descriptorSet;
		directVarianceWrite.dstBinding = 9;
		directVarianceWrite.dstArrayElement = 0;
		directVarianceWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		directVarianceWrite.descriptorCount = 1;
		directVarianceWrite.pImageInfo = &directVarianceInfo;

		descriptorWrites.push_back(directVarianceWrite);

		// Binding 10: indirectVarianceTexture
		VkDescriptorImageInfo indirectVarianceInfo{};
		indirectVarianceInfo.sampler = VK_NULL_HANDLE;
		indirectVarianceInfo.imageView = indirectVarianceTexture->getImageView();
		indirectVarianceInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkWriteDescriptorSet indirectVarianceWrite{};
		indirectVarianceWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		indirectVarianceWrite.dstSet = m_descriptorSet;
		indirectVarianceWrite.dstBinding = 10;
		indirectVarianceWrite.dstArrayElement = 0;
		indirectVarianceWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		indirectVarianceWrite.descriptorCount = 1;
		indirectVarianceWrite.pImageInfo = &indirectVarianceInfo;

		descriptorWrites.push_back(indirectVarianceWrite);
	
		// Update all
		vkUpdateDescriptorSets(context->getDevice(),
			static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
			0, nullptr);
}

std::unique_ptr<DescriptorSet> DescriptorSet::createSet5DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	StorageBuffer* exposureBuffer, Texture* texture) {
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descriptorSet->initSet5DescSet(context, layout, exposureBuffer, texture);
	return descriptorSet;
}

void DescriptorSet::initSet5DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	StorageBuffer* exposureBuffer, Texture* texture) {
	this->context = context;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = context->getDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    VkDescriptorSetLayout setLayout = layout->getDescriptorSetLayout();
    allocInfo.pSetLayouts = &setLayout;

    if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set (Set5)!");
    }

    std::vector<VkWriteDescriptorSet> descriptorWrites;

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = exposureBuffer->getBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(float);

    VkWriteDescriptorSet bufferWrite{};
    bufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    bufferWrite.dstSet = m_descriptorSet;
    bufferWrite.dstBinding = 0;
    bufferWrite.dstArrayElement = 0;
    bufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bufferWrite.descriptorCount = 1;
    bufferWrite.pBufferInfo = &bufferInfo;

    descriptorWrites.push_back(bufferWrite);

	VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = texture->getSampler();
    imageInfo.imageView = texture->getImageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet imageWrite{};
    imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    imageWrite.dstSet = m_descriptorSet;
    imageWrite.dstBinding = 1;
    imageWrite.dstArrayElement = 0;
    imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    imageWrite.descriptorCount = 1;
    imageWrite.pImageInfo = &imageInfo;

    descriptorWrites.push_back(imageWrite);

    vkUpdateDescriptorSets(context->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

std::unique_ptr<DescriptorSet> DescriptorSet::createSet6DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	Texture* hdrTexture, Texture* brightTexture, Texture* blurHTexture, Texture* blurVTexture, Texture* outputTexture) {
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descriptorSet->initSet6DescSet(context, layout, hdrTexture, brightTexture, blurHTexture, blurVTexture, outputTexture);
	return descriptorSet;
}

void DescriptorSet::initSet6DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	Texture* hdrTexture, Texture* brightTexture, Texture* blurHTexture, Texture* blurVTexture, Texture* outputTexture) {
	this->context = context;

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = context->getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	VkDescriptorSetLayout setLayout = layout->getDescriptorSetLayout();
	allocInfo.pSetLayouts = &setLayout;

	if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor set (Set6)!");
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	std::vector<VkDescriptorImageInfo> imageInfos(5);

	// Binding 0: HDR (sampler2D)
	imageInfos[0].sampler = nullptr;
	imageInfos[0].imageView = hdrTexture->getImageView();
	imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet hdrWrite{};
	hdrWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	hdrWrite.dstSet = m_descriptorSet;
	hdrWrite.dstBinding = 0;
	hdrWrite.dstArrayElement = 0;
	hdrWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	hdrWrite.descriptorCount = 1;
	hdrWrite.pImageInfo = &imageInfos[0];
	descriptorWrites.push_back(hdrWrite);

	// Binding 1: Bright (image2D)
	imageInfos[1].sampler = nullptr;
	imageInfos[1].imageView = brightTexture->getImageView();
	imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet brightWrite{};
	brightWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	brightWrite.dstSet = m_descriptorSet;
	brightWrite.dstBinding = 1;
	brightWrite.dstArrayElement = 0;
	brightWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	brightWrite.descriptorCount = 1;
	brightWrite.pImageInfo = &imageInfos[1];
	descriptorWrites.push_back(brightWrite);

	// Binding 2: BlurH (image2D)
	imageInfos[2].sampler = nullptr;
	imageInfos[2].imageView = blurHTexture->getImageView();
	imageInfos[2].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet blurHWrite{};
	blurHWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	blurHWrite.dstSet = m_descriptorSet;
	blurHWrite.dstBinding = 2;
	blurHWrite.dstArrayElement = 0;
	blurHWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	blurHWrite.descriptorCount = 1;
	blurHWrite.pImageInfo = &imageInfos[2];
	descriptorWrites.push_back(blurHWrite);

	// Binding 3: BlurV (image2D)
	imageInfos[3].sampler = nullptr;
	imageInfos[3].imageView = blurVTexture->getImageView();
	imageInfos[3].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet blurVWrite{};
	blurVWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	blurVWrite.dstSet = m_descriptorSet;
	blurVWrite.dstBinding = 3;
	blurVWrite.dstArrayElement = 0;
	blurVWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	blurVWrite.descriptorCount = 1;
	blurVWrite.pImageInfo = &imageInfos[3];
	descriptorWrites.push_back(blurVWrite);

	// Binding 4: Output (image2D)
	imageInfos[4].sampler = nullptr;
	imageInfos[4].imageView = outputTexture->getImageView();
	imageInfos[4].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet outputWrite{};
	outputWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	outputWrite.dstSet = m_descriptorSet;
	outputWrite.dstBinding = 4;
	outputWrite.dstArrayElement = 0;
	outputWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	outputWrite.descriptorCount = 1;
	outputWrite.pImageInfo = &imageInfos[4];
	descriptorWrites.push_back(outputWrite);

	vkUpdateDescriptorSets(context->getDevice(),
		static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
		0, nullptr);
}


std::unique_ptr<DescriptorSet> DescriptorSet::createSet7DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	Texture* inputTexture, Texture* outputTexture, Texture* historyTexture, Texture* varianceInputTexture, Texture* varianceOutputTexture) {
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descriptorSet->initSet7DescSet(context, layout, inputTexture, outputTexture, historyTexture, varianceInputTexture, varianceOutputTexture);
	return descriptorSet;
}

void DescriptorSet::initSet7DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	Texture* inputTexture, Texture* outputTexture, Texture* historyTexture, Texture* varianceInputTexture, Texture* varianceOutputTexture) {

	this->context = context;

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = context->getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	VkDescriptorSetLayout setLayout = layout->getDescriptorSetLayout();
	allocInfo.pSetLayouts = &setLayout;

	if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor set (Set7)!");
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	std::vector<VkDescriptorImageInfo> imageInfos(5);

	imageInfos[0].sampler = nullptr;
	imageInfos[0].imageView = inputTexture->getImageView();
	imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet inputWrite{};
	inputWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	inputWrite.dstSet = m_descriptorSet;
	inputWrite.dstBinding = 0;
	inputWrite.dstArrayElement = 0;
	inputWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	inputWrite.descriptorCount = 1;
	inputWrite.pImageInfo = &imageInfos[0];
	descriptorWrites.push_back(inputWrite);

	imageInfos[1].sampler = nullptr;
	imageInfos[1].imageView = outputTexture->getImageView();
	imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet outputWrite{};
	outputWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	outputWrite.dstSet = m_descriptorSet;
	outputWrite.dstBinding = 1;
	outputWrite.dstArrayElement = 0;
	outputWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	outputWrite.descriptorCount = 1;
	outputWrite.pImageInfo = &imageInfos[1];
	descriptorWrites.push_back(outputWrite);

	imageInfos[2].sampler = nullptr;
	imageInfos[2].imageView = historyTexture->getImageView();
	imageInfos[2].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet historyWrite{};
	historyWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	historyWrite.dstSet = m_descriptorSet;
	historyWrite.dstBinding = 2;
	historyWrite.dstArrayElement = 0;
	historyWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	historyWrite.descriptorCount = 1;
	historyWrite.pImageInfo = &imageInfos[2];
	descriptorWrites.push_back(historyWrite);

	imageInfos[3].sampler = nullptr;
	imageInfos[3].imageView = varianceInputTexture->getImageView();
	imageInfos[3].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet varianceInputWrite{};
	varianceInputWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	varianceInputWrite.dstSet = m_descriptorSet;
	varianceInputWrite.dstBinding = 3;
	varianceInputWrite.dstArrayElement = 0;
	varianceInputWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	varianceInputWrite.descriptorCount = 1;
	varianceInputWrite.pImageInfo = &imageInfos[3];
	descriptorWrites.push_back(varianceInputWrite);

	imageInfos[4].sampler = nullptr;
	imageInfos[4].imageView = varianceOutputTexture->getImageView();
	imageInfos[4].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet varianceOutputWrite{};
	varianceOutputWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	varianceOutputWrite.dstSet = m_descriptorSet;
	varianceOutputWrite.dstBinding = 4;
	varianceOutputWrite.dstArrayElement = 0;
	varianceOutputWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	varianceOutputWrite.descriptorCount = 1;
	varianceOutputWrite.pImageInfo = &imageInfos[4];
	descriptorWrites.push_back(varianceOutputWrite);


	vkUpdateDescriptorSets(context->getDevice(),
		static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
		0, nullptr);
}

std::unique_ptr<DescriptorSet> DescriptorSet::createSet8DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	Texture* normalTexture, Texture* depthTexture, Texture* albedoTexture) {
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descriptorSet->initSet8DescSet(context, layout, normalTexture, depthTexture, albedoTexture);
	return descriptorSet;
}

void DescriptorSet::initSet8DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	Texture* normalTexture, Texture* depthTexture, Texture* albedoTexture) {
	this->context = context;
	
	
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = context->getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	VkDescriptorSetLayout setLayout = layout->getDescriptorSetLayout();
	allocInfo.pSetLayouts = &setLayout;

	if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor set (Set8)!");
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	std::vector<VkDescriptorImageInfo> imageInfos(3);

	imageInfos[0].sampler = nullptr;
	imageInfos[0].imageView = normalTexture->getImageView();
	imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet normalWrite{};
	normalWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	normalWrite.dstSet = m_descriptorSet;
	normalWrite.dstBinding = 0;
	normalWrite.dstArrayElement = 0;
	normalWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	normalWrite.descriptorCount = 1;
	normalWrite.pImageInfo = &imageInfos[0];
	descriptorWrites.push_back(normalWrite);

	imageInfos[1].sampler = nullptr;
	imageInfos[1].imageView = depthTexture->getImageView();
	imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet depthWrite{};
	depthWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	depthWrite.dstSet = m_descriptorSet;
	depthWrite.dstBinding = 1;
	depthWrite.dstArrayElement = 0;
	depthWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	depthWrite.descriptorCount = 1;
	depthWrite.pImageInfo = &imageInfos[1];
	descriptorWrites.push_back(depthWrite);
	
	imageInfos[2].imageView = albedoTexture->getImageView();
	imageInfos[2].sampler = nullptr;
	imageInfos[2].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet albedoWrite{};
	albedoWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	albedoWrite.dstSet = m_descriptorSet;
	albedoWrite.dstBinding = 2;
	albedoWrite.dstArrayElement = 0;
	albedoWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	albedoWrite.descriptorCount = 1;
	albedoWrite.pImageInfo = &imageInfos[2];
	descriptorWrites.push_back(albedoWrite);

	vkUpdateDescriptorSets(context->getDevice(),
		static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
		0, nullptr);
}


std::unique_ptr<DescriptorSet> DescriptorSet::createSet9DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	Texture* directFilteredTexture, Texture* indirectFilteredTexture, Texture* compositeTexture) {
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descriptorSet->initSet9DescSet(context, layout, directFilteredTexture, indirectFilteredTexture, compositeTexture);
	return descriptorSet;
}

void DescriptorSet::initSet9DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	Texture* directFilteredTexture, Texture* indirectFilteredTexture, Texture* compositeTexture) {
	this->context = context;
	
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = context->getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	VkDescriptorSetLayout setLayout = layout->getDescriptorSetLayout();
	allocInfo.pSetLayouts = &setLayout;

	if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor set (Set9)!");
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	std::vector<VkDescriptorImageInfo> imageInfos(3);

	imageInfos[0].sampler = nullptr;
	imageInfos[0].imageView = directFilteredTexture->getImageView();
	imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet directWrite{};
	directWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	directWrite.dstSet = m_descriptorSet;
	directWrite.dstBinding = 0;
	directWrite.dstArrayElement = 0;
	directWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	directWrite.descriptorCount = 1;
	directWrite.pImageInfo = &imageInfos[0];
	descriptorWrites.push_back(directWrite);

	imageInfos[1].sampler = nullptr;
	imageInfos[1].imageView = indirectFilteredTexture->getImageView();
	imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet indirectWrite{};
	indirectWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	indirectWrite.dstSet = m_descriptorSet;
	indirectWrite.dstBinding = 1;
	indirectWrite.dstArrayElement = 0;
	indirectWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	indirectWrite.descriptorCount = 1;
	indirectWrite.pImageInfo = &imageInfos[1];
	descriptorWrites.push_back(indirectWrite);

	imageInfos[2].sampler = nullptr;
	imageInfos[2].imageView = compositeTexture->getImageView();
	imageInfos[2].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet compositeWrite{};
	compositeWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	compositeWrite.dstSet = m_descriptorSet;
	compositeWrite.dstBinding = 2;
	compositeWrite.dstArrayElement = 0;
	compositeWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	compositeWrite.descriptorCount = 1;
	compositeWrite.pImageInfo = &imageInfos[2];
	descriptorWrites.push_back(compositeWrite);

	vkUpdateDescriptorSets(context->getDevice(),
		static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
		0, nullptr);
}

std::unique_ptr<DescriptorSet> DescriptorSet::createSet10DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	Texture* inputTexture, Texture* outputTexture) {
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descriptorSet->initSet10DescSet(context, layout, inputTexture, outputTexture);
	return descriptorSet;
}


void DescriptorSet::initSet10DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	Texture* inputTexture, Texture* outputTexture) {
	this->context = context;

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = context->getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	VkDescriptorSetLayout setLayout = layout->getDescriptorSetLayout();
	allocInfo.pSetLayouts = &setLayout;

	if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor set (Set10)!");
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	std::vector<VkDescriptorImageInfo> imageInfos(2);

	imageInfos[0].sampler = nullptr;
	imageInfos[0].imageView = inputTexture->getImageView();
	imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet inputWrite{};
	inputWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	inputWrite.dstSet = m_descriptorSet;
	inputWrite.dstBinding = 0;
	inputWrite.dstArrayElement = 0;
	inputWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	inputWrite.descriptorCount = 1;
	inputWrite.pImageInfo = &imageInfos[0];
	descriptorWrites.push_back(inputWrite);

	imageInfos[1].sampler = nullptr;
	imageInfos[1].imageView = outputTexture->getImageView();
	imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet outputWrite{};
	outputWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	outputWrite.dstSet = m_descriptorSet;
	outputWrite.dstBinding = 1;
	outputWrite.dstArrayElement = 0;
	outputWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	outputWrite.descriptorCount = 1;
	outputWrite.pImageInfo = &imageInfos[1];
	descriptorWrites.push_back(outputWrite);
	
	vkUpdateDescriptorSets(context->getDevice(),
		static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
		0, nullptr);
}

