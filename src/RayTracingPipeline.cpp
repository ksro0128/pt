#include "include/RayTracingPipeline.h"

VkShaderModule RayTracingPipeline::createShaderModule(VulkanContext* context, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());


	VkShaderModule shaderModule;
	if (vkCreateShaderModule(context->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

RayTracingPipeline::~RayTracingPipeline() {
	cleanup();
}

void RayTracingPipeline::cleanup() {
	std::cout << "RayTracingPipeline::cleanup" << std::endl;

	if (m_sbtBuffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(context->getDevice(), m_sbtBuffer, nullptr);
		m_sbtBuffer = VK_NULL_HANDLE;
	}
	if (m_sbtMemory != VK_NULL_HANDLE) {
		vkFreeMemory(context->getDevice(), m_sbtMemory, nullptr);
		m_sbtMemory = VK_NULL_HANDLE;
	}
	if (m_pipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(context->getDevice(), m_pipeline, nullptr);
		m_pipeline = VK_NULL_HANDLE;
	}
	if (m_pipelineLayout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(context->getDevice(), m_pipelineLayout, nullptr);
		m_pipelineLayout = VK_NULL_HANDLE;
	}
	
}

std::unique_ptr<RayTracingPipeline> RayTracingPipeline::createPtPipeline(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts) {
	std::unique_ptr<RayTracingPipeline> pipeline = std::unique_ptr<RayTracingPipeline>(new RayTracingPipeline());
	pipeline->initPt(context, descriptorSetLayouts);
	return pipeline;
}	

void RayTracingPipeline::initPt(VulkanContext* context, std::vector<DescriptorSetLayout*> descriptorSetLayouts) {
	this->context = context;

	auto rgenCode = VulkanUtil::readFile("spv/pt.rgen.spv");
	auto rmissCode = VulkanUtil::readFile("spv/pt.rmiss.spv");
	auto rchitCode = VulkanUtil::readFile("spv/pt.rchit.spv");
	auto shodowMissCode = VulkanUtil::readFile("spv/ptShadow.rmiss.spv");

	VkShaderModule  rgenModule = createShaderModule(context, rgenCode);
	VkShaderModule rmissModule = createShaderModule(context, rmissCode);
	VkShaderModule rchitModule = createShaderModule(context, rchitCode);
	VkShaderModule shadowMissModule = createShaderModule(context, shodowMissCode);

	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};

	VkRayTracingShaderGroupCreateInfoKHR raygenGroup{};
	raygenGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	raygenGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	raygenGroup.generalShader = 0;
	raygenGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
	raygenGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
	raygenGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
	shaderGroups.push_back(raygenGroup);

	VkRayTracingShaderGroupCreateInfoKHR missGroup{};
	missGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	missGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	missGroup.generalShader = 1;
	missGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
	missGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
	missGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
	shaderGroups.push_back(missGroup);

	VkRayTracingShaderGroupCreateInfoKHR shadowMissGroup{};
	shadowMissGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	shadowMissGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	shadowMissGroup.generalShader = 2;
	shadowMissGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
	shadowMissGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
	shadowMissGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
	shaderGroups.push_back(shadowMissGroup);

	VkRayTracingShaderGroupCreateInfoKHR hitGroup{};
	hitGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	hitGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
	hitGroup.generalShader = VK_SHADER_UNUSED_KHR;
	hitGroup.closestHitShader = 3; // 3
	hitGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
	hitGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
	shaderGroups.push_back(hitGroup);

	std::vector<VkDescriptorSetLayout> layouts;
	for (auto& dsl : descriptorSetLayouts) {
		layouts.push_back(dsl->getDescriptorSetLayout());
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
	pipelineLayoutInfo.pSetLayouts = layouts.data();

	if (vkCreatePipelineLayout(context->getDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create ray tracing pipeline layout!");
	}

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	VkPipelineShaderStageCreateInfo rgenStage{};
	rgenStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	rgenStage.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	rgenStage.module = rgenModule;
	rgenStage.pName = "main";
	shaderStages.push_back(rgenStage);

	VkPipelineShaderStageCreateInfo rmissStage{};
	rmissStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	rmissStage.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
	rmissStage.module = rmissModule;
	rmissStage.pName = "main";
	shaderStages.push_back(rmissStage);

	VkPipelineShaderStageCreateInfo shadowMissStage{};
	shadowMissStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shadowMissStage.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
	shadowMissStage.module = shadowMissModule;
	shadowMissStage.pName = "main";
	shaderStages.push_back(shadowMissStage);

	VkPipelineShaderStageCreateInfo rchitStage{};
	rchitStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	rchitStage.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	rchitStage.module = rchitModule;
	rchitStage.pName = "main";
	shaderStages.push_back(rchitStage);

	VkRayTracingPipelineCreateInfoKHR pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
	pipelineInfo.pGroups = shaderGroups.data();
	pipelineInfo.maxPipelineRayRecursionDepth = 4;
	pipelineInfo.layout = m_pipelineLayout;

	if (g_vkCreateRayTracingPipelinesKHR(context->getDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create ray tracing pipeline!");
	}

	vkDestroyShaderModule(context->getDevice(), rgenModule, nullptr);
	vkDestroyShaderModule(context->getDevice(), rmissModule, nullptr);
	vkDestroyShaderModule(context->getDevice(), rchitModule, nullptr);
	vkDestroyShaderModule(context->getDevice(), shadowMissModule, nullptr);

	const uint32_t groupCount = static_cast<uint32_t>(shaderGroups.size());

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtProps{};
	rtProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

	VkPhysicalDeviceProperties2 props2{};
	props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	props2.pNext = &rtProps;

	vkGetPhysicalDeviceProperties2(context->getPhysicalDevice(), &props2);

	const uint32_t handleSize = rtProps.shaderGroupHandleSize;
	const uint32_t baseAlignment = rtProps.shaderGroupBaseAlignment;
	const uint32_t handleSizeAligned = (handleSize + baseAlignment - 1) & ~(baseAlignment - 1);


	std::vector<uint8_t> handles(groupCount * handleSize);
	if (g_vkGetRayTracingShaderGroupHandlesKHR(context->getDevice(), m_pipeline, 0, groupCount, handles.size(), handles.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to get shader group handles!");
	}

	const uint32_t sbtSize = groupCount * handleSizeAligned;

	VulkanUtil::createBuffer(
		context,
		sbtSize,
		VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_sbtBuffer,
		m_sbtMemory
	);

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;
	VulkanUtil::createBuffer(
		context,
		sbtSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingMemory
	);

	void* mapped;
	vkMapMemory(context->getDevice(), stagingMemory, 0, sbtSize, 0, &mapped);
	for (uint32_t i = 0; i < groupCount; ++i) {
		memcpy(reinterpret_cast<uint8_t*>(mapped) + i * handleSizeAligned,
			handles.data() + i * handleSize,
			handleSize);
	}
	vkUnmapMemory(context->getDevice(), stagingMemory);

	VulkanUtil::copyBuffer(context, stagingBuffer, m_sbtBuffer, sbtSize);

	vkDestroyBuffer(context->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(context->getDevice(), stagingMemory, nullptr);

	VkDeviceAddress sbtAddress = VulkanUtil::getDeviceAddress(context, m_sbtBuffer);

	m_raygenRegion = {
		sbtAddress + 0 * handleSizeAligned,
		handleSizeAligned,
		handleSizeAligned
	};
	m_missRegion = {
		sbtAddress + 1 * handleSizeAligned,
		handleSizeAligned,
		handleSizeAligned * 2
	};
	m_hitRegion = {
		sbtAddress + 3 * handleSizeAligned,
		handleSizeAligned,
		handleSizeAligned
	};
}
