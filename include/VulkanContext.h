#pragma once

#include "Common.h"

class VulkanContext {
public:
	static std::unique_ptr<VulkanContext> createVulkanContext(GLFWwindow* window);
	~VulkanContext();
	void createSurface(GLFWwindow* window);

	VkInstance getInstance() { return m_instance; }
	VkDebugUtilsMessengerEXT getDebugMessenger() { return m_debugMessenger; }
	VkPhysicalDevice getPhysicalDevice() { return m_physicalDevice; }
	VkDevice getDevice() { return m_device; }
	VkQueue getGraphicsQueue() { return m_graphicsQueue; }
	VkQueue getPresentQueue() { return m_presentQueue; }
	VkCommandPool getCommandPool() { return m_commandPool; }
	VkSurfaceKHR getSurface() { return m_surface; }
	VkSampleCountFlagBits getMaxMsaaSamples() { return m_maxMsaaSamples; }
	VkDescriptorPool getDescriptorPool() { return m_descriptorPool; }
	uint32_t getQueueFamily() { return findQueueFamilies(m_physicalDevice).graphicsFamily.value(); }

private:
	VulkanContext() {}

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkSampleCountFlagBits m_maxMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkDevice m_device;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
	VkCommandPool m_commandPool;
	VkDescriptorPool m_descriptorPool;


	void init(GLFWwindow* window);
	void cleanup();
	void createInstance();
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	void setupDebugMessenger();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	VkSampleCountFlagBits getMaxUsableSampleCount();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	void createLogicalDevice();
	void createCommandPool();
	void createDescriptorPool();
	void loadRayTracingFunctions();


	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
};