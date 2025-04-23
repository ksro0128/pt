#include "include/GuiRenderer.h"
#include <algorithm>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

GuiRenderer::~GuiRenderer() {
    cleanup();
}

void GuiRenderer::cleanup() {
    std::cout << "GuiRenderer::cleanup" << std::endl;
    ImGui_ImplVulkan_DestroyFontsTexture();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(context->getDevice(), m_descriptorPool, nullptr);
}

std::unique_ptr<GuiRenderer> GuiRenderer::createGuiRenderer(VulkanContext* context, GLFWwindow* window, RenderPass* renderPass, SwapChain* swapChain) {
    std::unique_ptr<GuiRenderer> guiRenderer = std::unique_ptr<GuiRenderer>(new GuiRenderer());
    guiRenderer->init(context, window, renderPass, swapChain);
    return guiRenderer;
}

void GuiRenderer::init(VulkanContext* context, GLFWwindow* window, RenderPass* renderPass, SwapChain* swapChain) {
    this->context = context;
    createDescriptorPool();
    IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    setDarkThemeColors();

	ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = context->getInstance();
	init_info.PhysicalDevice = context->getPhysicalDevice();
	init_info.Device = context->getDevice();
	init_info.Queue = context->getGraphicsQueue();
	init_info.QueueFamily = context->getQueueFamily();
	init_info.DescriptorPool = m_descriptorPool;
	init_info.RenderPass = renderPass->getRenderPass();
	init_info.MinImageCount = swapChain->getSwapChainImages().size();
	init_info.ImageCount = swapChain->getSwapChainImages().size();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	ImGui_ImplVulkan_Init(&init_info);

    ImGui_ImplVulkan_CreateFontsTexture();

    m_dockLayoutBuilt = false;
    m_viewportSize = ImVec2(1024, 1024);
}

void GuiRenderer::createDescriptorPool() {

    std::array<VkDescriptorPoolSize, 11> poolSizes = {
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000 * static_cast<uint32_t>(poolSizes.size());
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();

	if (vkCreateDescriptorPool(context->getDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create ImGui descriptor pool!");
	}
}

 void GuiRenderer::newFrame() {
 	ImGui_ImplVulkan_NewFrame();
 	ImGui_ImplGlfw_NewFrame();
 	ImGui::NewFrame();
 }

void GuiRenderer::render(uint32_t currentFrame, VkCommandBuffer cmd, Scene *scene, std::vector<Model>& modelList, float deltaTime) {
     static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

     ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
     const ImGuiViewport* viewport = ImGui::GetMainViewport();

     ImGui::SetNextWindowPos(viewport->WorkPos);
     ImGui::SetNextWindowSize(viewport->WorkSize);
     ImGui::SetNextWindowViewport(viewport->ID);
     window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | 
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
     window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

     ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
     ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

     ImGui::Begin("DockSpace Demo", nullptr, window_flags);
     ImGui::PopStyleVar(2);

     ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
     if (!m_dockLayoutBuilt) {
         std::cout << "setupDockspace" << std::endl;
         ImGui::DockBuilderRemoveNode(dockspace_id);
         ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
         ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

		 ImGuiID dock_main_id = dockspace_id;
		 ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);
		 ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);

		 // 왼쪽 패널을 위/아래로 나눔 → 위: Scene, 아래: Material Editor
		 ImGuiID dock_id_material = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.5f, nullptr, &dock_id_left);

		 // 각각의 도킹 위치에 창 할당
		 ImGui::DockBuilderDockWindow("Scene", dock_id_left);
		 ImGui::DockBuilderDockWindow("Viewport", dock_main_id);

         ImGui::DockBuilderFinish(dockspace_id);
         m_dockLayoutBuilt = true;
     }

	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	ImGui::End();


	 // Viewport
	ImGui::Begin("Viewport");
	ImGui::End();

	 // Scene
	ImGui::Begin("Scene");
	ImGui::End();


     ImGui::Render();
     ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
 }

void GuiRenderer::createViewPortDescriptorSet(std::array<Texture*, 2> textures) {
	if (m_viewPortDescriptorSet.size() == 2) {
		for (auto& descSet : m_viewPortDescriptorSet) {
			ImGui_ImplVulkan_RemoveTexture(descSet);
		}
	}

    m_viewPortDescriptorSet.resize(textures.size());
	m_viewPortDescriptorSet[0] = ImGui_ImplVulkan_AddTexture(textures[0]->getSampler(), textures[0]->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	m_viewPortDescriptorSet[1] = ImGui_ImplVulkan_AddTexture(textures[1]->getSampler(), textures[1]->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void GuiRenderer::createRayTracingDescriptorSet(std::array<Texture*, 2> textures) {
	if (m_rayTracingDescriptorSet.size() == 2) {
		for (auto& descSet : m_rayTracingDescriptorSet) {
			ImGui_ImplVulkan_RemoveTexture(descSet);
		}
	}
	m_rayTracingDescriptorSet.resize(textures.size());
	m_rayTracingDescriptorSet[0] = ImGui_ImplVulkan_AddTexture(textures[0]->getSampler(), textures[0]->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	m_rayTracingDescriptorSet[1] = ImGui_ImplVulkan_AddTexture(textures[1]->getSampler(), textures[1]->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void GuiRenderer::createAlbedoDescriptorSet(std::array<Texture*, 2> textures) {
	if (m_albedoDescriptorSet.size() == 2) {
		for (auto& descSet : m_albedoDescriptorSet) {
			ImGui_ImplVulkan_RemoveTexture(descSet);
		}
	}
	m_albedoDescriptorSet.resize(textures.size());
	m_albedoDescriptorSet[0] = ImGui_ImplVulkan_AddTexture(textures[0]->getSampler(), textures[0]->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	m_albedoDescriptorSet[1] = ImGui_ImplVulkan_AddTexture(textures[1]->getSampler(), textures[1]->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void GuiRenderer::createPositionDescriptorSet(std::array<Texture*, 2> textures) {
	if (m_positionDescriptorSet.size() == 2) {
		for (auto& descSet : m_positionDescriptorSet) {
			ImGui_ImplVulkan_RemoveTexture(descSet);
		}
	}
	m_positionDescriptorSet.resize(textures.size());
	m_positionDescriptorSet[0] = ImGui_ImplVulkan_AddTexture(textures[0]->getSampler(), textures[0]->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	m_positionDescriptorSet[1] = ImGui_ImplVulkan_AddTexture(textures[1]->getSampler(), textures[1]->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void GuiRenderer::createNormalDescriptorSet(std::array<Texture*, 2> textures) {
	if (m_normalDescriptorSet.size() == 2) {
		for (auto& descSet : m_normalDescriptorSet) {
			ImGui_ImplVulkan_RemoveTexture(descSet);
		}
	}
	m_normalDescriptorSet.resize(textures.size());
	m_normalDescriptorSet[0] = ImGui_ImplVulkan_AddTexture(textures[0]->getSampler(), textures[0]->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	m_normalDescriptorSet[1] = ImGui_ImplVulkan_AddTexture(textures[1]->getSampler(), textures[1]->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void GuiRenderer::createPbrDescriptorSet(std::array<Texture*, 2> textures) {
	if (m_pbrDescriptorSet.size() == 2) {
		for (auto& descSet : m_pbrDescriptorSet) {
			ImGui_ImplVulkan_RemoveTexture(descSet);
		}
	}
	m_pbrDescriptorSet.resize(textures.size());
	m_pbrDescriptorSet[0] = ImGui_ImplVulkan_AddTexture(textures[0]->getSampler(), textures[0]->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	m_pbrDescriptorSet[1] = ImGui_ImplVulkan_AddTexture(textures[1]->getSampler(), textures[1]->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void GuiRenderer::createEmissiveDescriptorSet(std::array<Texture*, 2> textures) {
	if (m_emissiveDescriptorSet.size() == 2) {
		for (auto& descSet : m_emissiveDescriptorSet) {
			ImGui_ImplVulkan_RemoveTexture(descSet);
		}
	}
	m_emissiveDescriptorSet.resize(textures.size());
	m_emissiveDescriptorSet[0] = ImGui_ImplVulkan_AddTexture(textures[0]->getSampler(), textures[0]->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	m_emissiveDescriptorSet[1] = ImGui_ImplVulkan_AddTexture(textures[1]->getSampler(), textures[1]->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}


void GuiRenderer::setDarkThemeColors()
{
	auto &colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

	// Headers
	colors[ImGuiCol_Header] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
	colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
	colors[ImGuiCol_HeaderActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

	// Buttons
	colors[ImGuiCol_Button] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
	colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
	colors[ImGuiCol_ButtonActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

	// Frame BG
	colors[ImGuiCol_FrameBg] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
	colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
	colors[ImGuiCol_FrameBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

	// Tabs
	colors[ImGuiCol_Tab] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
	colors[ImGuiCol_TabHovered] = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
	colors[ImGuiCol_TabActive] = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
	colors[ImGuiCol_TabUnfocused] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

	// Title
	colors[ImGuiCol_TitleBg] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
	colors[ImGuiCol_TitleBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
}


