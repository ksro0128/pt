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
	VkAccelerationStructureKHR tlas, Texture* pingTexture, Texture* pongTexture) {
	std::unique_ptr<DescriptorSet> descriptorSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descriptorSet->initSet4DescSet(context, layout, tlas, pingTexture, pongTexture);
	return descriptorSet;
}

void DescriptorSet::initSet4DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	VkAccelerationStructureKHR tlas, Texture* pingTexture, Texture* pongTexture) {
		this->context = context;

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = context->getDescriptorPool();
		allocInfo.descriptorSetCount = 1;
		VkDescriptorSetLayout setLayout = layout->getDescriptorSetLayout();
		allocInfo.pSetLayouts = &setLayout;
	
		if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor set (Set3)!");
		}
	
		std::vector<VkWriteDescriptorSet> descriptorWrites;
	
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
	
		VkDescriptorImageInfo pingImageInfo{};
		pingImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		pingImageInfo.imageView = pingTexture->getImageView();
		pingImageInfo.sampler = VK_NULL_HANDLE;
	
		VkWriteDescriptorSet pingWrite{};
		pingWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		pingWrite.dstSet = m_descriptorSet;
		pingWrite.dstBinding = 1;
		pingWrite.dstArrayElement = 0;
		pingWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		pingWrite.descriptorCount = 1;
		pingWrite.pImageInfo = &pingImageInfo;
	
		descriptorWrites.push_back(pingWrite);
	
		VkDescriptorImageInfo pongImageInfo{};
		pongImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		pongImageInfo.imageView = pongTexture->getImageView();
		pongImageInfo.sampler = VK_NULL_HANDLE;
	
		VkWriteDescriptorSet pongWrite{};
		pongWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		pongWrite.dstSet = m_descriptorSet;
		pongWrite.dstBinding = 2;
		pongWrite.dstArrayElement = 0;
		pongWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		pongWrite.descriptorCount = 1;
		pongWrite.pImageInfo = &pongImageInfo;
	
		descriptorWrites.push_back(pongWrite);
	
		vkUpdateDescriptorSets(context->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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