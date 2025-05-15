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


std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createSet0DescLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initSet0DescLayout(context);
	return layout;
}

void DescriptorSetLayout::initSet0DescLayout(VulkanContext* context) {
	this->context = context;

	std::vector<VkDescriptorSetLayoutBinding> bindings(3);

	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[0].pImmutableSamplers = nullptr;

	bindings[1].binding = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[1].descriptorCount = 1;
	bindings[1].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[1].pImmutableSamplers = nullptr;

	bindings[2].binding = 2;
	bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[2].descriptorCount = 1;
	bindings[2].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[2].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create Set0 descriptor set layout!");
	}
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createSet1DescLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initSet1DescLayout(context);
	return layout;
}

void DescriptorSetLayout::initSet1DescLayout(VulkanContext* context) {
	this->context = context;

	std::vector<VkDescriptorSetLayoutBinding> bindings(1);

	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	bindings[0].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create Set1 descriptor set layout!");
	}
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createSet2DescLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initSet2DescLayout(context);
	return layout;
}

void DescriptorSetLayout::initSet2DescLayout(VulkanContext* context) {
	this->context = context;

	const int bindingCount = 10;
	std::vector<VkDescriptorSetLayoutBinding> bindings(bindingCount);

	for (int i = 0; i < bindingCount; i++) {
		bindings[i].binding = i;
		bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bindings[i].descriptorCount = 1;
		bindings[i].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		bindings[i].pImmutableSamplers = nullptr;
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();
	layoutInfo.pNext = nullptr;
	layoutInfo.flags = 0;

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create Set2 descriptor set layout!");
	}
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createSet3DescLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initSet3DescLayout(context);
	return layout;
}

void DescriptorSetLayout::initSet3DescLayout(VulkanContext* context) {
	this->context = context;

	VkDescriptorSetLayoutBinding binding{};
	binding.binding = 0;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding.descriptorCount = MAX_TEXTURE_COUNT;
	binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	binding.pImmutableSamplers = nullptr;

	VkDescriptorBindingFlags bindingFlag =
		VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
		VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

	VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{};
	extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	extendedInfo.bindingCount = 1;
	extendedInfo.pBindingFlags = &bindingFlag;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &binding;
	layoutInfo.pNext = &extendedInfo;
	layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create Set3 descriptor set layout!");
	}
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createSet4DescLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initSet4DescLayout(context);
	return layout;
}

void DescriptorSetLayout::initSet4DescLayout(VulkanContext* context) {
	this->context = context;
	std::vector<VkDescriptorSetLayoutBinding> bindings(11);

	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	bindings[0].pImmutableSamplers = nullptr;

	// Direct history
	bindings[1].binding = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[1].descriptorCount = 1;
	bindings[1].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[1].pImmutableSamplers = nullptr;

	// Direct current
	bindings[2].binding = 2;
	bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[2].descriptorCount = 1;
	bindings[2].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[2].pImmutableSamplers = nullptr;

	// Indirect history
	bindings[3].binding = 3;
	bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[3].descriptorCount = 1;
	bindings[3].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[3].pImmutableSamplers = nullptr;

	// Indirect current
	bindings[4].binding = 4;
	bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[4].descriptorCount = 1;
	bindings[4].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[4].pImmutableSamplers = nullptr;

	// direct m1
	bindings[5].binding = 5;
	bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[5].descriptorCount = 1;
	bindings[5].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	bindings[5].pImmutableSamplers = nullptr;

	// direct m2
	bindings[6].binding = 6;
	bindings[6].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[6].descriptorCount = 1;
	bindings[6].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	bindings[6].pImmutableSamplers = nullptr;

	// indirect m1
	bindings[7].binding = 7;
	bindings[7].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[7].descriptorCount = 1;
	bindings[7].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	bindings[7].pImmutableSamplers = nullptr;

	// indirect m2
	bindings[8].binding = 8;
	bindings[8].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[8].descriptorCount = 1;
	bindings[8].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	bindings[8].pImmutableSamplers = nullptr;

	// direct variance
	bindings[9].binding = 9;
	bindings[9].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[9].descriptorCount = 1;
	bindings[9].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	bindings[9].pImmutableSamplers = nullptr;

	// indirect variance
	bindings[10].binding = 10;
	bindings[10].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[10].descriptorCount = 1;
	bindings[10].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	bindings[10].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create Set4 descriptor set layout!");
	}
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createSet5DescLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initSet5DescLayout(context);
	return layout;
}

void DescriptorSetLayout::initSet5DescLayout(VulkanContext* context) {
	this->context = context;

	VkDescriptorSetLayoutBinding bufferBinding{};
	bufferBinding.binding = 0;
	bufferBinding.descriptorCount = 1;
	bufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	bufferBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 1;
    samplerBinding.descriptorCount = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerBinding.pImmutableSamplers = nullptr;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { bufferBinding, samplerBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create Set5 descriptor set layout!");
	}
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createSet6DescLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initSet6DescLayout(context);
	return layout;
}

void DescriptorSetLayout::initSet6DescLayout(VulkanContext* context) {
	this->context = context;

	VkDescriptorSetLayoutBinding hdrBinding{};
	hdrBinding.binding = 0;
	hdrBinding.descriptorCount = 1;
	hdrBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	hdrBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	hdrBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding brightBinding{};	
	brightBinding.binding = 1;
	brightBinding.descriptorCount = 1;
	brightBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	brightBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	brightBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding blurHBinding{};
	blurHBinding.binding = 2;
	blurHBinding.descriptorCount = 1;
	blurHBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	blurHBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	blurHBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding blurVBinding{};
	blurVBinding.binding = 3;
	blurVBinding.descriptorCount = 1;
	blurVBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	blurVBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	blurVBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding outputBinding{};
	outputBinding.binding = 4;
	outputBinding.descriptorCount = 1;
	outputBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	outputBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	outputBinding.pImmutableSamplers = nullptr;

	std::array<VkDescriptorSetLayoutBinding, 5> bindings = {
		hdrBinding,
		brightBinding,
		blurHBinding,
		blurVBinding,
		outputBinding
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create Set6 descriptor set layout!");
	}
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createSet7DescLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initSet7DescLayout(context);
	return layout;
}

void DescriptorSetLayout::initSet7DescLayout(VulkanContext* context) {
	this->context = context;

	VkDescriptorSetLayoutBinding inputBinding{};
	inputBinding.binding = 0;
	inputBinding.descriptorCount = 1;
	inputBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	inputBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	inputBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding outputBinding{};
	outputBinding.binding = 1;
	outputBinding.descriptorCount = 1;
	outputBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	outputBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	outputBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding historyBinding{};
	historyBinding.binding = 2;
	historyBinding.descriptorCount = 1;
	historyBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	historyBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	historyBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding varianceInputBinding{};
	varianceInputBinding.binding = 3;
	varianceInputBinding.descriptorCount = 1;
	varianceInputBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	varianceInputBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	varianceInputBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding varianceOutputBinding{};	
	varianceOutputBinding.binding = 4;
	varianceOutputBinding.descriptorCount = 1;
	varianceOutputBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	varianceOutputBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	varianceOutputBinding.pImmutableSamplers = nullptr;

	std::array<VkDescriptorSetLayoutBinding, 5> bindings = {
		inputBinding,
		outputBinding,
		historyBinding,
		varianceInputBinding,
		varianceOutputBinding
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create Set7 descriptor set layout!");
	}
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createSet8DescLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initSet8DescLayout(context);
	return layout;
}

void DescriptorSetLayout::initSet8DescLayout(VulkanContext* context) {
	this->context = context;


	std::array<VkDescriptorSetLayoutBinding, 11> bindings{};

	// 0: Normal image
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[0].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT;

	// 1: Depth image
	bindings[1].binding = 1;
	bindings[1].descriptorCount = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[1].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT;

	// 2: albedo image
	bindings[2].binding = 2;
	bindings[2].descriptorCount = 1;
	bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[2].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT;

	// 3: mesh id image
	bindings[3].binding = 3;
	bindings[3].descriptorCount = 1;
	bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[3].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT;

	// 4: sample count image
	bindings[4].binding = 4;
	bindings[4].descriptorCount = 1;
	bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[4].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT;

	// 5: motion vector image
	bindings[5].binding = 5;
	bindings[5].descriptorCount = 1;
	bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[5].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	// 6: prev normal image
	bindings[6].binding = 6;
	bindings[6].descriptorCount = 1;
	bindings[6].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[6].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT;
	
	// 7: prev depth image
	bindings[7].binding = 7;
	bindings[7].descriptorCount = 1;
	bindings[7].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[7].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT;
	
	// 8: prev mesh id image
	bindings[8].binding = 8;
	bindings[8].descriptorCount = 1;
	bindings[8].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[8].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT;

	// 9: current jitter image
	bindings[9].binding = 9;
	bindings[9].descriptorCount = 1;
	bindings[9].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[9].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT;

	// 10: prev jitter image
	bindings[10].binding = 10;
	bindings[10].descriptorCount = 1;
	bindings[10].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[10].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT;

	


	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create Set8 descriptor set layout!");
	}
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createSet9DescLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initSet9DescLayout(context);
	return layout;
}

void DescriptorSetLayout::initSet9DescLayout(VulkanContext* context) {
	this->context = context;


	std::array<VkDescriptorSetLayoutBinding, 4> bindings{};

	// 0: direct filtered image
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	// 1: indirect filtered image
	bindings[1].binding = 1;
	bindings[1].descriptorCount = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	// 2: composite image
	bindings[2].binding = 2;
	bindings[2].descriptorCount = 1;
	bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	// 3: prevComposite image
	bindings[3].binding = 3;
	bindings[3].descriptorCount = 1;
	bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create Set9 descriptor set layout!");
	}
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createSet10DescLayout(VulkanContext* context) {
	std::unique_ptr<DescriptorSetLayout> layout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
	layout->initSet10DescLayout(context);
	return layout;
}


void DescriptorSetLayout::initSet10DescLayout(VulkanContext* context) {
	this->context = context;

	VkDescriptorSetLayoutBinding inputBinding{};
	inputBinding.binding = 0;
	inputBinding.descriptorCount = 1;
	inputBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	inputBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	inputBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding outputBinding{};
	outputBinding.binding = 1;
	outputBinding.descriptorCount = 1;
	outputBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	outputBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	outputBinding.pImmutableSamplers = nullptr;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { inputBinding, outputBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(context->getDevice(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {	
		throw std::runtime_error("failed to create Set10 descriptor set layout!");
	}
}
