#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "RenderPass.h"
#include "SwapChain.h"
#include "Texture.h"

class FrameBuffer {
public:
	static std::unique_ptr<FrameBuffer> createGbufferFrameBuffer(VulkanContext* context, RenderPass* renderPass, Texture* normal, Texture* linearDepth, Texture* meshID, Texture* motionVector, Texture* depthTexture, VkExtent2D extent);
	static std::unique_ptr<FrameBuffer> createImGuiFrameBuffer(VulkanContext* context, RenderPass* renderPass, VkImageView& swapChainImageView, VkExtent2D extent);
	static std::unique_ptr<FrameBuffer> createOutputFrameBuffer(VulkanContext* context, RenderPass* renderPass, Texture* texture, VkExtent2D extent);
	static std::unique_ptr<FrameBuffer> createShadowMapFrameBuffer(VulkanContext* context, RenderPass* renderPass, Texture* texture, VkExtent2D extent);
	static std::unique_ptr<FrameBuffer> createShadowCubeMapFrameBuffer(VulkanContext* context, RenderPass* renderPass, Texture* texture, VkExtent2D extent, uint32_t faceIndex);
	static std::unique_ptr<FrameBuffer> createToneMappingFrameBuffer(VulkanContext* context, RenderPass* renderPass, Texture* texture, VkExtent2D extent);
	~FrameBuffer();

	VkFramebuffer getFrameBuffer() { return m_frameBuffer; }
private:
	VulkanContext* context;
	VkFramebuffer m_frameBuffer;

	void initGbuffer(VulkanContext* context, RenderPass* renderPass, Texture* normal, Texture* linearDepth, Texture* meshID, Texture* motionVector, Texture* depthTexture, VkExtent2D extent);
	void initImGui(VulkanContext* context, RenderPass* renderPass, VkImageView& swapChainImageView, VkExtent2D extent);
	void initOutput(VulkanContext* context, RenderPass* renderPass, Texture* texture, VkExtent2D extent);
	void initShadowMap(VulkanContext* context, RenderPass* renderPass, Texture* texture, VkExtent2D extent);
	void initShadowCubeMap(VulkanContext* context, RenderPass* renderPass, Texture* texture, VkExtent2D extent, uint32_t faceIndex);
	void initToneMapping(VulkanContext* context, RenderPass* renderPass, Texture* texture, VkExtent2D extent);
	void cleanup();
};