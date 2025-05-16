#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "SwapChain.h"

class RenderPass {
public:
	static std::unique_ptr<RenderPass> createImGuiRenderPass(VulkanContext* context, SwapChain* swapChain);


	~RenderPass();

	VkRenderPass getRenderPass() { return m_renderPass; }
private:
	VulkanContext* context;
	VkRenderPass m_renderPass;

	void initImGui(VulkanContext* context, SwapChain* swapChain);

	void cleanup();
};