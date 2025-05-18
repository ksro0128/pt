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

std::unique_ptr<DescriptorSet> DescriptorSet::createSet0DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	UniformBuffer* cameraBuffer, UniformBuffer* optionsBuffer) {
	std::unique_ptr<DescriptorSet> descSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descSet->initSet0DescSet(context, layout, cameraBuffer, optionsBuffer);
	return descSet;
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
	StorageBuffer* materialBuffer) {
	std::unique_ptr<DescriptorSet> descSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descSet->initSet1DescSet(context, layout, materialBuffer);
	return descSet;
}

void DescriptorSet::initSet1DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	StorageBuffer* materialBuffer) {
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
	bufferInfo.buffer = materialBuffer->getBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = materialBuffer->getCurrentSize();

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

std::unique_ptr<DescriptorSet> DescriptorSet::createSet2DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	std::vector<std::unique_ptr<Texture>>& textures) {
	std::unique_ptr<DescriptorSet> descSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descSet->initSet2DescSet(context, layout, textures);
	return descSet;
}

void DescriptorSet::initSet2DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	std::vector<std::unique_ptr<Texture>>& textures) {

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

	for (size_t i = 0; i < textures.size(); ++i) {
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textures[i]->getImageView();
		imageInfo.sampler = textures[i]->getSampler();
		imageInfos.push_back(imageInfo);
	}

	for (size_t i = textures.size(); i < MAX_TEXTURE_COUNT; ++i) {
		VkDescriptorImageInfo dummy{};
		dummy.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		dummy.imageView = textures[0]->getImageView();
		dummy.sampler = textures[0]->getSampler();
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

	vkUpdateDescriptorSets(context->getDevice(), 1, &textureWrite, 0, nullptr);
}

std::unique_ptr<DescriptorSet> DescriptorSet::createSet3DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	StorageBuffer* instanceBuffer, StorageBuffer* areaLightBuffer) {
	std::unique_ptr<DescriptorSet> descSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descSet->initSet3DescSet(context, layout, instanceBuffer, areaLightBuffer);
	return descSet;
}

void DescriptorSet::initSet3DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	StorageBuffer* instanceBuffer, StorageBuffer* areaLightBuffer) {
	this->context = context;

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = context->getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	VkDescriptorSetLayout setLayout = layout->getDescriptorSetLayout();
	allocInfo.pSetLayouts = &setLayout;

	if (vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor set (Set3 - instance buffer, area light buffer)!");
	}

	VkDescriptorBufferInfo instanceBufferInfo{};
	instanceBufferInfo.buffer = instanceBuffer->getBuffer();
	instanceBufferInfo.offset = 0;
	instanceBufferInfo.range = instanceBuffer->getCurrentSize();
	
	VkWriteDescriptorSet instanceBufferWrite{};
	instanceBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	instanceBufferWrite.dstSet = m_descriptorSet;
	instanceBufferWrite.dstBinding = 0;
	instanceBufferWrite.dstArrayElement = 0;
	instanceBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	instanceBufferWrite.descriptorCount = 1;
	instanceBufferWrite.pBufferInfo = &instanceBufferInfo;

	VkDescriptorBufferInfo areaLightBufferInfo{};
	areaLightBufferInfo.buffer = areaLightBuffer->getBuffer();
	areaLightBufferInfo.offset = 0;
	areaLightBufferInfo.range = areaLightBuffer->getCurrentSize();

	VkWriteDescriptorSet areaLightBufferWrite{};
	areaLightBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	areaLightBufferWrite.dstSet = m_descriptorSet;
	areaLightBufferWrite.dstBinding = 1;
	areaLightBufferWrite.dstArrayElement = 0;
	areaLightBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	areaLightBufferWrite.descriptorCount = 1;
	areaLightBufferWrite.pBufferInfo = &areaLightBufferInfo;

	std::array<VkWriteDescriptorSet, 2> writes{ instanceBufferWrite, areaLightBufferWrite };
	vkUpdateDescriptorSets(context->getDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

std::unique_ptr<DescriptorSet> DescriptorSet::createSet4DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	VkAccelerationStructureKHR tlas) {
	std::unique_ptr<DescriptorSet> descSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descSet->initSet4DescSet(context, layout, tlas);
	return descSet;
}

void DescriptorSet::initSet4DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	VkAccelerationStructureKHR tlas) {
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

	vkUpdateDescriptorSets(context->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

std::unique_ptr<DescriptorSet> DescriptorSet::createSet5DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	Texture* dirOutput, Texture* indirectOutput, Texture* dirAccum, Texture* indirectAccum, Texture* output) {
	std::unique_ptr<DescriptorSet> descSet = std::unique_ptr<DescriptorSet>(new DescriptorSet());
	descSet->initSet5DescSet(context, layout, dirOutput, indirectOutput, dirAccum, indirectAccum, output);
	return descSet;
}

void DescriptorSet::initSet5DescSet(VulkanContext* context, DescriptorSetLayout* layout,
	Texture* dirOutput, Texture* indirectOutput, Texture* dirAccum, Texture* indirectAccum, Texture* output) {
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

	VkDescriptorImageInfo dirOutputImageInfo{};
	dirOutputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	dirOutputImageInfo.imageView = dirOutput->getImageView();
	dirOutputImageInfo.sampler = VK_NULL_HANDLE;

	VkWriteDescriptorSet dirOutputWrite{};
	dirOutputWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	dirOutputWrite.dstSet = m_descriptorSet;
	dirOutputWrite.dstBinding = 0;
	dirOutputWrite.dstArrayElement = 0;
	dirOutputWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	dirOutputWrite.descriptorCount = 1;
	dirOutputWrite.pImageInfo = &dirOutputImageInfo;
	descriptorWrites.push_back(dirOutputWrite);

	VkDescriptorImageInfo indirectOutputImageInfo{};
	indirectOutputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	indirectOutputImageInfo.imageView = indirectOutput->getImageView();
	indirectOutputImageInfo.sampler = VK_NULL_HANDLE;

	VkWriteDescriptorSet indirectOutputWrite{};
	indirectOutputWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	indirectOutputWrite.dstSet = m_descriptorSet;
	indirectOutputWrite.dstBinding = 1;
	indirectOutputWrite.dstArrayElement = 0;
	indirectOutputWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	indirectOutputWrite.descriptorCount = 1;
	indirectOutputWrite.pImageInfo = &indirectOutputImageInfo;
	descriptorWrites.push_back(indirectOutputWrite);

	VkDescriptorImageInfo dirAccumImageInfo{};
	dirAccumImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	dirAccumImageInfo.imageView = dirAccum->getImageView();
	dirAccumImageInfo.sampler = VK_NULL_HANDLE;

	VkWriteDescriptorSet dirAccumWrite{};
	dirAccumWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	dirAccumWrite.dstSet = m_descriptorSet;
	dirAccumWrite.dstBinding = 2;
	dirAccumWrite.dstArrayElement = 0;
	dirAccumWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	dirAccumWrite.descriptorCount = 1;
	dirAccumWrite.pImageInfo = &dirAccumImageInfo;
	descriptorWrites.push_back(dirAccumWrite);

	VkDescriptorImageInfo indirectAccumImageInfo{};
	indirectAccumImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	indirectAccumImageInfo.imageView = indirectAccum->getImageView();
	indirectAccumImageInfo.sampler = VK_NULL_HANDLE;

	VkWriteDescriptorSet indirectAccumWrite{};
	indirectAccumWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	indirectAccumWrite.dstSet = m_descriptorSet;
	indirectAccumWrite.dstBinding = 3;
	indirectAccumWrite.dstArrayElement = 0;
	indirectAccumWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	indirectAccumWrite.descriptorCount = 1;
	indirectAccumWrite.pImageInfo = &indirectAccumImageInfo;
	descriptorWrites.push_back(indirectAccumWrite);

	VkDescriptorImageInfo outputImageInfo{};
	outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	outputImageInfo.imageView = output->getImageView();
	outputImageInfo.sampler = VK_NULL_HANDLE;

	VkWriteDescriptorSet outputWrite{};
	outputWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	outputWrite.dstSet = m_descriptorSet;
	outputWrite.dstBinding = 4;
	outputWrite.dstArrayElement = 0;
	outputWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	outputWrite.descriptorCount = 1;
	outputWrite.pImageInfo = &outputImageInfo;
	descriptorWrites.push_back(outputWrite);

	vkUpdateDescriptorSets(context->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}
