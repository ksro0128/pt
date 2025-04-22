#pragma once

#include "Common.h"
#include "VulkanContext.h"

class VulkanUtil {
public:
	static void createImage(VulkanContext* context, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, bool isCubeMap = false);
	static uint32_t findMemoryType(VulkanContext* context, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static VkImageView createImageView(VulkanContext* context, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, bool isCubeMap = false);
	static VkCommandBuffer beginSingleTimeCommands(VulkanContext* context);
	static void endSingleTimeCommands(VulkanContext* context, VkCommandBuffer commandBuffer);
	
	static void createBuffer(VulkanContext* context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	static void copyBuffer(VulkanContext* context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	static VkDeviceAddress getDeviceAddress(VulkanContext* context, VkBuffer buffer);


	static VkFormat findDepthFormat(VulkanContext* context);
	static VkFormat findSupportedFormat(VulkanContext* context, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	static std::vector<char> readFile(const std::string& filename);
};
