#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "RenderPass.h"
#include "SwapChain.h"
#include "Texture.h"

class FrameBuffer {
public:
	static std::unique_ptr<FrameBuffer> createImGuiFrameBuffer(VulkanContext* context, RenderPass* renderPass, VkImageView& swapChainImageView, VkExtent2D extent);

	~FrameBuffer();

	VkFramebuffer getFrameBuffer() { return m_frameBuffer; }
private:
	VulkanContext* context;
	VkFramebuffer m_frameBuffer;

	void initImGui(VulkanContext* context, RenderPass* renderPass, VkImageView& swapChainImageView, VkExtent2D extent);
	void cleanup();
};