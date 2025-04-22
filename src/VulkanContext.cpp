#include "include/VulkanContext.h"

std::unique_ptr<VulkanContext> VulkanContext::createVulkanContext(GLFWwindow* window) {
	std::unique_ptr<VulkanContext> context = std::unique_ptr<VulkanContext>(new VulkanContext());
	context->init(window);
	return context;
}

VulkanContext::~VulkanContext() {
	cleanup();
}

void VulkanContext::init(GLFWwindow* window) {
	std::cout << "VulkanContext::init" << std::endl;
	createInstance();
	setupDebugMessenger();
	createSurface(window);
	pickPhysicalDevice();
	createLogicalDevice();
	loadRayTracingFunctions();
	createCommandPool();
	createDescriptorPool();
}


void VulkanContext::cleanup() {
	std::cout << "VulkanContext::cleanup" << std::endl;
	vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    vkDestroyDevice(m_device, nullptr);
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

void VulkanContext::createInstance() {
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	std::vector<const char*> extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

bool  VulkanContext::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false; 
		}
	}
	return true; 
}

void VulkanContext::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

std::vector<const char*> VulkanContext::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void VulkanContext::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}


void VulkanContext::createSurface(GLFWwindow* window) {
	if (glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}



void VulkanContext::pickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			m_physicalDevice = device;
			m_maxMsaaSamples = getMaxUsableSampleCount();
			break;
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

bool VulkanContext::isDeviceSuitable(VkPhysicalDevice device) {
	QueueFamilyIndices indices = findQueueFamilies(device);
	bool extensionsSupported = checkDeviceExtensionSupport(device);
	bool swapChainAdequate = false;

	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}
	VkPhysicalDeviceVulkan12Features features12{};
	features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

	VkPhysicalDeviceFeatures2 features2{};
	features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features2.pNext = &features12;

	vkGetPhysicalDeviceFeatures2(device, &features2);

	std::cout << "Checking device suitability..." << std::endl;
	std::cout << "  - Queue Complete : " << indices.isComplete() << std::endl;
	std::cout << "  - Extensions     : " << extensionsSupported << std::endl;
	std::cout << "  - SwapChain OK   : " << swapChainAdequate << std::endl;
	std::cout << "  - Anisotropy     : " << features2.features.samplerAnisotropy << std::endl;
	std::cout << "  - Vulkan12 Feat  : UpdateAfterBind(" << features12.descriptorBindingSampledImageUpdateAfterBind
		<< "), PartiallyBound(" << features12.descriptorBindingPartiallyBound << ")" << std::endl;


	return indices.isComplete() && extensionsSupported && swapChainAdequate && features2.features.samplerAnisotropy;
}


VkSampleCountFlagBits VulkanContext::getMaxUsableSampleCount() {
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(m_physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}


QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}
	return indices;
}

bool VulkanContext::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	if (!requiredExtensions.empty()) {
		std::cout << "Missing extensions for this device:" << std::endl;
		for (const auto& ext : requiredExtensions) {
			std::cout << "  - " << ext << std::endl;
		}
	}

	return requiredExtensions.empty();
}

SwapChainSupportDetails VulkanContext::querySwapChainSupport(VkPhysicalDevice device) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

void VulkanContext::createLogicalDevice() {
	QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE;
	deviceFeatures.shaderInt64 = VK_TRUE;

	// Ray Tracing feature chain
	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
	accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	accelerationStructureFeatures.accelerationStructure = VK_TRUE;

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
	rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	rayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;

	VkPhysicalDeviceVulkan12Features features12{};
	features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	features12.descriptorBindingPartiallyBound = VK_TRUE;
	features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
	features12.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
	features12.descriptorIndexing = VK_TRUE;
	features12.runtimeDescriptorArray = VK_TRUE;
	features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	features12.bufferDeviceAddress = VK_TRUE;


	accelerationStructureFeatures.pNext = nullptr;
	rayTracingPipelineFeatures.pNext = &accelerationStructureFeatures;
	features12.pNext = &rayTracingPipelineFeatures;

	VkPhysicalDeviceFeatures2 features2{};
	features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features2.features = deviceFeatures;
	features2.pNext = &features12;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pNext = &features2;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
}


void VulkanContext::createCommandPool() {
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}


void VulkanContext::createDescriptorPool() {
	std::array<VkDescriptorPoolSize, 4> poolSizes{};

	// Set0 - Global
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT * 1000;

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT * 1000;

	// Set1 - Object, Material
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[2].descriptorCount = MAX_OBJECT_COUNT;

	// Set2 - Texture
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[3].descriptorCount = MAX_TEXTURE_COUNT * 10;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT * 256;

	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

PFN_vkCreateAccelerationStructureKHR g_vkCreateAccelerationStructureKHR = nullptr;
PFN_vkDestroyAccelerationStructureKHR g_vkDestroyAccelerationStructureKHR = nullptr;
PFN_vkGetAccelerationStructureBuildSizesKHR g_vkGetAccelerationStructureBuildSizesKHR = nullptr;
PFN_vkCmdBuildAccelerationStructuresKHR g_vkCmdBuildAccelerationStructuresKHR = nullptr;
PFN_vkGetAccelerationStructureDeviceAddressKHR g_vkGetAccelerationStructureDeviceAddressKHR = nullptr;

PFN_vkCreateRayTracingPipelinesKHR g_vkCreateRayTracingPipelinesKHR = nullptr;
PFN_vkGetRayTracingShaderGroupHandlesKHR g_vkGetRayTracingShaderGroupHandlesKHR = nullptr;
PFN_vkCmdTraceRaysKHR g_vkCmdTraceRaysKHR = nullptr;

void VulkanContext::loadRayTracingFunctions() {
	// Acceleration Structure
	g_vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
		vkGetDeviceProcAddr(m_device, "vkCreateAccelerationStructureKHR"));

	g_vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
		vkGetDeviceProcAddr(m_device, "vkDestroyAccelerationStructureKHR"));

	g_vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
		vkGetDeviceProcAddr(m_device, "vkGetAccelerationStructureBuildSizesKHR"));

	g_vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
		vkGetDeviceProcAddr(m_device, "vkCmdBuildAccelerationStructuresKHR"));

	g_vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(
		vkGetDeviceProcAddr(m_device, "vkGetAccelerationStructureDeviceAddressKHR"));

	// Ray Tracing Pipeline
	g_vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(
		vkGetDeviceProcAddr(m_device, "vkCreateRayTracingPipelinesKHR"));

	g_vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(
		vkGetDeviceProcAddr(m_device, "vkGetRayTracingShaderGroupHandlesKHR"));

	g_vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(
		vkGetDeviceProcAddr(m_device, "vkCmdTraceRaysKHR"));
}





VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}