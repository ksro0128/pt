#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "SwapChain.h"

class RenderPass {
public:
	static std::unique_ptr<RenderPass> createGbufferRenderPass(VulkanContext* context);
	static std::unique_ptr<RenderPass> createImGuiRenderPass(VulkanContext* context, SwapChain* swapChain);
	static std::unique_ptr<RenderPass> createLightPassRenderPass(VulkanContext* context);
	static std::unique_ptr<RenderPass> createShadowMapRenderPass(VulkanContext* context);
	static std::unique_ptr<RenderPass> createToneMappingRenderPass(VulkanContext* context);

	~RenderPass();

	VkRenderPass getRenderPass() { return m_renderPass; }
private:
	VulkanContext* context;
	VkRenderPass m_renderPass;

	void initGbuffer(VulkanContext* context);
	void initImGui(VulkanContext* context, SwapChain* swapChain);
	void initLightPass(VulkanContext* context);
	void initShadowMap(VulkanContext* context);
	void initToneMapping(VulkanContext* context);
	void cleanup();
};