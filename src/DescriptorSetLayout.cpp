#include "include/DescriptorSetLayout.h"

DescriptorSetLayout::~DescriptorSetLayout() {
	cleanup();
}

void DescriptorSetLayout::cleanup() {
	std::cout << "DescriptorSetLayout::cleanup" << std::endl;
	if (m_layout != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(context->getDevice(), m_layout, nullptr);
		m_layout = VK_NULL_HANDLE;
	}
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createGlobalDescriptorSetLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initGlobal(context);
	return layout;
}

void DescriptorSetLayout::initGlobal(VulkanContext* context) {
	this->context = context;

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	VkDescriptorSetLayoutBinding cameraBinding{};
	cameraBinding.binding = 0;
	cameraBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraBinding.descriptorCount = 1;
	cameraBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	cameraBinding.pImmutableSamplers = nullptr;
	bindings.push_back(cameraBinding);

	VkDescriptorSetLayoutBinding lightBinding{};
	lightBinding.binding = 1;
	lightBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	lightBinding.descriptorCount = 1;
	lightBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	lightBinding.pImmutableSamplers = nullptr;
	bindings.push_back(lightBinding);

	VkDescriptorSetLayoutBinding optionsBinding{};
	optionsBinding.binding = 2;
	optionsBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	optionsBinding.descriptorCount = 1;
	optionsBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	optionsBinding.pImmutableSamplers = nullptr;
	bindings.push_back(optionsBinding);

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createObjectMaterialDescriptorSetLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initObjectMaterial(context);
	return layout;
}

void DescriptorSetLayout::initObjectMaterial(VulkanContext* context) {
	this->context = context;

	VkDescriptorSetLayoutBinding instanceBinding{};
	instanceBinding.binding = 0;
	instanceBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	instanceBinding.descriptorCount = 1;
	instanceBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | 
		VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	instanceBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &instanceBinding;

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create Set1 descriptor set layout!");
	}
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createBindlessDescriptorSetLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initBindless(context);
	return layout;
}

void DescriptorSetLayout::initBindless(VulkanContext* context) {
	this->context = context;

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	VkDescriptorSetLayoutBinding modelBinding{};
	modelBinding.binding = 0;
	modelBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	modelBinding.descriptorCount = 1;
	modelBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	modelBinding.pImmutableSamplers = nullptr;
	bindings.push_back(modelBinding);

	VkDescriptorSetLayoutBinding materialBinding{};
	materialBinding.binding = 1;
	materialBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	materialBinding.descriptorCount = 1;
	materialBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	materialBinding.pImmutableSamplers = nullptr;
	bindings.push_back(materialBinding);

	VkDescriptorSetLayoutBinding textureBinding{};
	textureBinding.binding = 2;
	textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureBinding.descriptorCount = MAX_TEXTURE_COUNT;
	textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	textureBinding.pImmutableSamplers = nullptr;
	bindings.push_back(textureBinding);

	std::vector<VkDescriptorBindingFlags> bindingFlags = {
		0,
		0,
		VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
	};

	VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
	bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
	bindingFlagsInfo.pBindingFlags = bindingFlags.data();

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();
	layoutInfo.pNext = &bindingFlagsInfo;
	layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create bindless descriptor set layout!");
	}
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createAttachmentDescriptorSetLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initAttachment(context);
	return layout;
}

void DescriptorSetLayout::initAttachment(VulkanContext* context) {
	this->context = context;
	
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	for (uint32_t i = 0; i < 5; i++) {
		VkDescriptorSetLayoutBinding binding{};
		binding.binding = i;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		binding.pImmutableSamplers = nullptr;
		bindings.push_back(binding);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create attachment descriptor set layout!");
	}
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createShadowDescriptorSetLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initShadow(context);
	return layout;
}

void DescriptorSetLayout::initShadow(VulkanContext* context) {
	this->context = context;
	
	std::vector<VkDescriptorSetLayoutBinding> bindings(3);

	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[0].pImmutableSamplers = nullptr;

	bindings[1].binding = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[1].descriptorCount = 7;
	bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[1].pImmutableSamplers = nullptr;

	bindings[2].binding = 2;
	bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[2].descriptorCount = 1;
	bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[2].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shadow descriptor set layout!");
	}
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createRayTracingDescriptorSetLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initRayTracing(context);
	return layout;
}

void DescriptorSetLayout::initRayTracing(VulkanContext* context) {
	this->context = context;
	std::vector<VkDescriptorSetLayoutBinding> bindings(2);

	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[0].pImmutableSamplers = nullptr;

	bindings[1].binding = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	bindings[1].descriptorCount = 1;
	bindings[1].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	bindings[1].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create ray tracing descriptor set layout!");
	}
}