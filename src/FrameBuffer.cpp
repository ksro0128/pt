#include "include/FrameBuffer.h"

FrameBuffer::~FrameBuffer() {
	cleanup();
}

void FrameBuffer::cleanup() {
	// std::cout << "FrameBuffer::cleanup" << std::endl;
	if (m_frameBuffer != VK_NULL_HANDLE) {
		vkDestroyFramebuffer(context->getDevice(), m_frameBuffer, nullptr);
		m_frameBuffer = VK_NULL_HANDLE;
	}
}


std::unique_ptr<FrameBuffer> FrameBuffer::createImGuiFrameBuffer(VulkanContext* context, RenderPass* renderPass, VkImageView& swapChainImageView, VkExtent2D extent) {
	std::unique_ptr<FrameBuffer> frameBuffer = std::unique_ptr<FrameBuffer>(new FrameBuffer());
	frameBuffer->initImGui(context, renderPass, swapChainImageView, extent);
	return frameBuffer;
}

void FrameBuffer::initImGui(VulkanContext* context, RenderPass* renderPass, VkImageView& swapChainImageView, VkExtent2D extent) {
	this->context = context;

	VkImageView attachments[] = { swapChainImageView };

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass->getRenderPass();
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(context->getDevice(), &framebufferInfo, nullptr, &m_frameBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create ImGui framebuffer!");
	}
}
