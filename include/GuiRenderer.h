#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "RenderPass.h"
#include "SwapChain.h"
#include "Texture.h"
#include "Scene.h"
#include "Model.h"
#include "VulkanUtil.h"

class GuiRenderer {
public:
    static std::unique_ptr<GuiRenderer> createGuiRenderer(VulkanContext* context, GLFWwindow* window, RenderPass* renderPass, SwapChain* swapChain);
    ~GuiRenderer();

    void newFrame();
    void render(uint32_t currentFrame, VkCommandBuffer cmd, Scene* scene, std::vector<Model>& modelList, float deltaTime);
    void createViewPortDescriptorSet(std::array<Texture*, 2> textures);
    ImVec2 getViewportSize() const { return m_viewportSize; }
	bool isBenchmarkRunning() const { return m_benchmarkRunning; }

private:
    VulkanContext* context;
    VkDescriptorPool m_descriptorPool;
    int32_t m_viewPortIndex = 0;
	int32_t m_benchmarkFrameCount = 0;
	float m_benchmarkTime = 0.0f;
	float m_benchmarkScore = 0.0f;

	bool m_benchmarkRunning = false;
	float m_fpsMax = 0.0f;
	float m_fpsMin = FLT_MAX;
	float m_fpsSum = 0.0f;

    std::vector<VkDescriptorSet> m_viewPortDescriptorSet;

    bool m_dockLayoutBuilt;
    ImVec2 m_viewportSize;

    void init(VulkanContext* context, GLFWwindow* window, RenderPass* renderPass, SwapChain* swapChain);
    void createDescriptorPool();
    void cleanup();
    void setDarkThemeColors();
    //void setupDockspace();
};
