#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "VulkanUtil.h"

class SwapChain {
public:
	static std::unique_ptr<SwapChain> createSwapChain(GLFWwindow* window, VulkanContext* context);
	~SwapChain();
	void cleanup();

	void recreateSwapChain();

	VkSwapchainKHR getSwapChain() { return m_swapChain; }
	std::vector<VkImage>& getSwapChainImages() { return m_swapChainImages; }
	VkFormat getSwapChainImageFormat() { return m_swapChainImageFormat; }
	VkExtent2D getSwapChainExtent() { return m_swapChainExtent; }
	std::vector<VkImageView>& getSwapChainImageViews() { return m_swapChainImageViews; }
private:
	VulkanContext* context;
	GLFWwindow* window;

	VkSwapchainKHR m_swapChain;
	std::vector<VkImage> m_swapChainImages;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;
	std::vector<VkImageView> m_swapChainImageViews;

	void init(GLFWwindow* window, VulkanContext* context);
	void initSwapChain();
	void initImageViews();
	QueueFamilyIndices findQueueFamilies();
	SwapChainSupportDetails querySwapChainSupport();
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};