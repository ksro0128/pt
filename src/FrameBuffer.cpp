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

std::unique_ptr<FrameBuffer> FrameBuffer::createGbufferFrameBuffer(VulkanContext* context,
    RenderPass* renderPass, Texture* normal, Texture* linearDepth,
    Texture* meshID, Texture* motionVector, Texture* depthTexture, VkExtent2D extent) {
	std::unique_ptr<FrameBuffer> frameBuffer = std::unique_ptr<FrameBuffer>(new FrameBuffer());
	frameBuffer->initGbuffer(context, renderPass, normal, linearDepth, meshID, motionVector, depthTexture, extent);
	return frameBuffer;
}

void FrameBuffer::initGbuffer(VulkanContext* context, 
	RenderPass* renderPass, Texture* normal, Texture* linearDepth, 
	Texture* meshID, Texture* motionVector, Texture* depthTexture, VkExtent2D extent) {
    this->context = context;

    std::array<VkImageView, 5> attachments = {
        normal->getImageView(),         // layout(location = 0)
        linearDepth->getImageView(),    // layout(location = 1)
        meshID->getImageView(),         // layout(location = 2)
        motionVector->getImageView(),   // layout(location = 3)
        depthTexture->getImageView()    // depth attachment (attachment = 4)
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass->getRenderPass();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(context->getDevice(), &framebufferInfo, nullptr, &m_frameBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create G-buffer framebuffer!");
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


std::unique_ptr<FrameBuffer> FrameBuffer::createOutputFrameBuffer(VulkanContext* context, RenderPass* renderPass, Texture* texture, VkExtent2D extent) {
	std::unique_ptr<FrameBuffer> frameBuffer = std::unique_ptr<FrameBuffer>(new FrameBuffer());
	frameBuffer->initOutput(context, renderPass, texture, extent);
	return frameBuffer;
}

void FrameBuffer::initOutput(VulkanContext* context, RenderPass* renderPass, Texture* texture, VkExtent2D extent) {
	this->context = context;

	VkImageView attachments[] = { texture->getImageView() };

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass->getRenderPass();
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.layers = 1;
	if (vkCreateFramebuffer(context->getDevice(), &framebufferInfo, nullptr, &m_frameBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create output framebuffer!");
	}
}

std::unique_ptr<FrameBuffer> FrameBuffer::createShadowMapFrameBuffer(VulkanContext* context, RenderPass* renderPass, Texture* texture, VkExtent2D extent) {
	std::unique_ptr<FrameBuffer> frameBuffer = std::unique_ptr<FrameBuffer>(new FrameBuffer());
	frameBuffer->initShadowMap(context, renderPass, texture, extent);
	return frameBuffer;
}

void FrameBuffer::initShadowMap(VulkanContext* context, RenderPass* renderPass, Texture* texture, VkExtent2D extent) {
	this->context = context;
	
	VkImageView attachments[] = { texture->getImageView() };

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass->getRenderPass();
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.layers = 1;
	if (vkCreateFramebuffer(context->getDevice(), &framebufferInfo, nullptr, &m_frameBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create output framebuffer!");
	}
}

std::unique_ptr<FrameBuffer> FrameBuffer::createShadowCubeMapFrameBuffer(VulkanContext* context, RenderPass* renderPass, Texture* texture, VkExtent2D extent, uint32_t faceIndex) {
	std::unique_ptr<FrameBuffer> frameBuffer = std::unique_ptr<FrameBuffer>(new FrameBuffer());
	frameBuffer->initShadowCubeMap(context, renderPass, texture, extent, faceIndex);
	return frameBuffer;
}

void FrameBuffer::initShadowCubeMap(VulkanContext* context, RenderPass* renderPass, Texture* texture, VkExtent2D extent, uint32_t faceIndex) {
	this->context = context;

	VkImageView attachments[] = { texture->getCubeMapImageViews()[faceIndex] };

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass->getRenderPass();
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.layers = 1;
	if (vkCreateFramebuffer(context->getDevice(), &framebufferInfo, nullptr, &m_frameBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shadow cube map framebuffer!");
	}
}


std::unique_ptr<FrameBuffer> FrameBuffer::createToneMappingFrameBuffer(VulkanContext* context, RenderPass* renderPass, Texture* texture, VkExtent2D extent) {
	std::unique_ptr<FrameBuffer> frameBuffer = std::unique_ptr<FrameBuffer>(new FrameBuffer());
	frameBuffer->initToneMapping(context, renderPass, texture, extent);
	return frameBuffer;
}

void FrameBuffer::initToneMapping(VulkanContext* context, RenderPass* renderPass, Texture* texture, VkExtent2D extent) {
	this->context = context;

	VkImageView attachments[] = { texture->getImageView() };

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass->getRenderPass();
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.layers = 1;
	if (vkCreateFramebuffer(context->getDevice(), &framebufferInfo, nullptr, &m_frameBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create tone mapping framebuffer!");
	}
}