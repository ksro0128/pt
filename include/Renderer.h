#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "SwapChain.h"
#include "SyncObjects.h"
#include "Buffer.h"
#include "Mesh.h"
#include "Texture.h"
#include "DescriptorSetLayout.h"
#include "DescriptorSet.h"
#include "RenderPass.h"
#include "FrameBuffer.h"
#include "Pipeline.h"
#include "CommandBuffers.h"
#include "GuiRenderer.h"
#include "AccelerationStructure.h"
#include "RayTracingPipeline.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

class Renderer {
public:
	static std::unique_ptr<Renderer> createRenderer(GLFWwindow* window);
	~Renderer();

	void update(float deltaTime);
	void render(float deltaTime);
	bool isBenchmarkRunning() const { return m_guiRenderer->isBenchmarkRunning(); }
private:
	GLFWwindow* window;
	std::unique_ptr<VulkanContext> m_context;
	std::unique_ptr<SwapChain> m_swapChain;
	std::unique_ptr<SyncObjects> m_syncObjects;

	VkExtent2D m_extent;
	uint32_t currentFrame = 0;
	uint32_t m_frameCount = 0;

	// texture
	std::unique_ptr<Texture> m_outputPingTexture;
	std::unique_ptr<Texture> m_outputPongTexture;

	// command buffer
	std::unique_ptr<CommandBuffers> m_commandBuffers;

	// gui renderer
	std::unique_ptr<GuiRenderer> m_guiRenderer;
	std::vector<std::unique_ptr<FrameBuffer>> m_imguiFrameBuffers;
	std::unique_ptr<RenderPass> m_imguiRenderPass;

	void cleanup();
	void init(GLFWwindow* window);
	void recreateSwapChain();
	void recreateViewport(ImVec2 newExtent);
	void recordImGuiCommandBuffer(uint32_t imageIndex);
	void transferImageLayout(VkCommandBuffer cmd, Texture* texture, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, uint32_t layerCount = 1);

};