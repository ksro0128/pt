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
		 ImGui::DockBuilderDockWindow("Material Editor", dock_id_material);
		 ImGui::DockBuilderDockWindow("Viewport", dock_main_id);
		 ImGui::DockBuilderDockWindow("Profiler", dock_id_right);

         ImGui::DockBuilderFinish(dockspace_id);
         m_dockLayoutBuilt = true;
     }

     ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
     ImGui::End();


	 // Viewport
     ImGui::Begin("Viewport");
     m_viewportSize = ImGui::GetContentRegionAvail();

	 if (m_viewPortIndex == 0) {
		 ImGui::Image((ImTextureID)(uint64_t)m_viewPortDescriptorSet[currentFrame], m_viewportSize);
	 }
	 else if (m_viewPortIndex == 1) {
		 ImGui::Image((ImTextureID)(uint64_t)m_rayTracingDescriptorSet[currentFrame], m_viewportSize);
	 }
	 else if (m_viewPortIndex == 2) {
		 ImGui::Image((ImTextureID)(uint64_t)m_albedoDescriptorSet[currentFrame], m_viewportSize);
	 }
	 else if (m_viewPortIndex == 3) {
		 ImGui::Image((ImTextureID)(uint64_t)m_positionDescriptorSet[currentFrame], m_viewportSize);
	 }
	 else if (m_viewPortIndex == 4) {
		 ImGui::Image((ImTextureID)(uint64_t)m_normalDescriptorSet[currentFrame], m_viewportSize);
	 }
	 else if (m_viewPortIndex == 5) {
		 ImGui::Image((ImTextureID)(uint64_t)m_pbrDescriptorSet[currentFrame], m_viewportSize);
	 }
	 else if (m_viewPortIndex == 6) {
		 ImGui::Image((ImTextureID)(uint64_t)m_emissiveDescriptorSet[currentFrame], m_viewportSize);
	 }
	 
     ImGui::End();

	// Profiler 창
	ImGui::Begin("Profiler");
	static int currentRTMode = 0;

	bool disableUI = m_benchmarkRunning;
	if (disableUI) { ImGui::BeginDisabled(true); }

	ImGuiIO& io = ImGui::GetIO();
	ImGui::Text("Frame Duration: %.3f ms", deltaTime * 1000.0f);
	ImGui::Text("FPS: %.1f", io.Framerate);
	ImGui::Text("Viewport Size: %.0f x %.0f", m_viewportSize.x, m_viewportSize.y);


	const char* items[] = { "Viewport", "Ray Tracing", "Albedo", "Position", "Normal", "PBR", "Emissive"};
	int itemCount = IM_ARRAYSIZE(items);

	ImGui::Text("Select Viewport");

	ImGui::BeginChild("ViewportSelectorBox", ImVec2(0, itemCount * 22 + 10), true, ImGuiWindowFlags_NoScrollbar);
	for (int i = 0; i < itemCount; i++) {
		ImGui::PushID(i);

		bool is_selected = (m_viewPortIndex == i);
		bool is_rt_option = (i == 1);
		bool rt_enabled = currentRTMode == 1 || currentRTMode == 2;
		bool disabled = (is_rt_option && !rt_enabled);

		if (is_selected) {
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.6f, 1.0f, 0.4f));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.7f, 1.0f, 0.5f));
		}

		if (disabled)
			ImGui::BeginDisabled(true);

		if (ImGui::Selectable(items[i], is_selected)) {
			if (!disabled)
				m_viewPortIndex = i;
		}

		if (disabled)
			ImGui::EndDisabled();

		if (is_selected)
			ImGui::PopStyleColor(2);

		ImGui::PopID();
	}
	ImGui::EndChild();

	if (currentRTMode == 0 && m_viewPortIndex == 1) {
		m_viewPortIndex = 0;
	}


	const char* rtModes[] = { "off", "reflection", "path"};
	ImGui::Text("RT Reflection Mode");
	if (ImGui::Combo("##RTModeSelector", &currentRTMode, rtModes, IM_ARRAYSIZE(rtModes))) {
		if (setRTMode) setRTMode(currentRTMode);
	}


	static int reflectionSampleCount = 1;
	if (currentRTMode == 1) {
		ImGui::Text("Reflection Sample Count");
		ImGui::SliderInt("##ReflectionSampleCount", &reflectionSampleCount, 1, 128);
		if (setReflectionSampleCount) setReflectionSampleCount(reflectionSampleCount);
	}

	static int reflectionMaxBounce = 1;
	if (currentRTMode == 1) {
		ImGui::Text("Reflection Max Bounce");
		ImGui::SliderInt("##ReflectionMaxBounce", &reflectionMaxBounce, 1, 4);
		if (setReflectionMaxBounce) setReflectionMaxBounce(reflectionMaxBounce);
	}

	if (!m_benchmarkRunning && ImGui::Button("Start Benchmark")) {
		m_benchmarkRunning = true;
		m_benchmarkTime = 0.0f;
		m_benchmarkFrameCount = 0;
		m_fpsSum = 0.0f;
		m_fpsMax = 0.0f;
		m_fpsMin = FLT_MAX;
	}

	if (disableUI) { ImGui::EndDisabled(); }

	if (m_benchmarkRunning) {
		m_benchmarkTime += deltaTime;
		m_benchmarkFrameCount++;

		float fpsNow = 1.0f / deltaTime;
		m_fpsSum += fpsNow;

		if (fpsNow > m_fpsMax) m_fpsMax = fpsNow;
		if (fpsNow < m_fpsMin) m_fpsMin = fpsNow;

		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Benchmark Running...");
		ImGui::Text("Elapsed Time: %.2f / 10.00 sec", m_benchmarkTime);
		ImGui::Text("Current FPS: %.2f", fpsNow);
		ImGui::Text("Delta Time: %.4f", deltaTime);
		if (m_benchmarkTime >= 10.0f) {
			m_benchmarkRunning = false;
		}
	}

	if (!m_benchmarkRunning && m_benchmarkFrameCount > 0) {
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.15f, 0.25f, 0.8f));
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);

		ImGui::BeginChild("BenchmarkResult", ImVec2(0, 100), true);

		ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Benchmark Results");
		ImGui::Spacing();

		float avgFps = m_fpsSum / m_benchmarkFrameCount;

		ImGui::Text("Total Frames: %d", m_benchmarkFrameCount);
		ImGui::Text("Avg FPS:      %.2f", avgFps);
		ImGui::Text("Min FPS:      %.2f", m_fpsMin);
		ImGui::Text("Max FPS:      %.2f", m_fpsMax);

		ImGui::EndChild();

		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}



	ImGui::End();
	 // Scene
	ImGui::Begin("Scene");
	if (disableUI) { ImGui::BeginDisabled(true); }

	if (ImGui::Button("Add Light")) {
		 Light newLight{};
		 newLight.type = 0; // Directional
		 newLight.direction = glm::vec3(0.0f, -1.0f, 0.0f);
		 newLight.color = glm::vec3(1.0f);
		 newLight.intensity = 1.0f;
		 newLight.range = 10.0f;
		 newLight.position = glm::vec3(0.0f);
		 newLight.spotInnerAngle = 30.0f;
		 newLight.spotOuterAngle = 45.0f;
		 scene->getLights().push_back(newLight);
	 }
	 ImGui::SameLine();
	 if (ImGui::Button("Add Object")) {
		 Object newObj;
		 newObj.modelIndex = 0;
		 newObj.position = glm::vec3(0.0f);
		 newObj.rotation = glm::vec3(0.0f);
		 newObj.scale = glm::vec3(1.0f);

		 if (scene->getMaxMaterialIndex() > 0)
			 newObj.overrideMaterialIndex.push_back(0);

		 scene->getObjects().push_back(newObj);
	 }
	 

	 auto& lights = scene->getLights();
	 ImGui::ColorEdit3("Ambient Color", glm::value_ptr(scene->getAmbientColor()));
	 int id = 0;
	 for (int i = 0; i < lights.size(); i++) {
		 ImGui::PushID(id++);
		 if (ImGui::TreeNode(("Light " + std::to_string(i)).c_str())) {
			 ImGui::Combo("Type", &lights[i].type, "Directional\0Point\0Spot\0");
			 ImGui::DragFloat3("Position", glm::value_ptr(lights[i].position), 0.1f);
			 ImGui::DragFloat3("Direction", glm::value_ptr(lights[i].direction), 0.1f);
			 ImGui::ColorEdit3("Color", glm::value_ptr(lights[i].color));
			 ImGui::DragFloat("Intensity", &lights[i].intensity, 0.1f, 0.0f, 10.0f);
			 ImGui::DragFloat("Range", &lights[i].range, 0.1f, 0.0f, 100.0f);
			 ImGui::DragFloat("Spot Inner", &lights[i].spotInnerAngle, 1.0f, 0.0f, 90.0f);
			 ImGui::DragFloat("Spot Outer", &lights[i].spotOuterAngle, 1.0f, 0.0f, 90.0f);
			 bool castsShadow = lights[i].castsShadow != 0;
			 if (ImGui::Checkbox("Casts Shadow", &castsShadow)) {
				 lights[i].castsShadow = castsShadow ? 1 : 0;
			 }
			 ImGui::TreePop();

			 if (ImGui::Button("Remove Light")) {
				 lights.erase(lights.begin() + i);
				 ImGui::PopID();
				 break;
			 }
		 }
		 ImGui::PopID();
	 }

	auto& objects = scene->getObjects();
	uint32_t maxModelIndex = scene->getMaxModelIndex();
	uint32_t maxMaterialIndex = scene->getMaxMaterialIndex();

	for (int i = 0; i < objects.size(); i++) {
		ImGui::PushID(id++);
		if (ImGui::TreeNode(("Object " + std::to_string(i)).c_str())) {
			ImGui::DragFloat3("Position", glm::value_ptr(objects[i].position), 0.1f);
			ImGui::DragFloat3("Rotation", glm::value_ptr(objects[i].rotation), 1.0f);
			ImGui::DragFloat3("Scale", glm::value_ptr(objects[i].scale), 0.1f);

			std::vector<std::string> modelItems;
			for (uint32_t j = 0; j < maxModelIndex; j++)
				modelItems.push_back(std::to_string(j));
			std::vector<const char*> modelCStrs;
			for (auto& s : modelItems)
				modelCStrs.push_back(s.c_str());
			int oldModelIndex = objects[i].modelIndex;
			ImGui::Combo("Model Index", &objects[i].modelIndex, modelCStrs.data(), modelCStrs.size());
			if (oldModelIndex != objects[i].modelIndex) {
				objects[i].overrideMaterialIndex.clear();
			}

			if (objects[i].modelIndex >= 0 && objects[i].modelIndex < modelList.size()) {
				const auto& model = modelList[objects[i].modelIndex];
				int meshCount = (int)model.mesh.size();

				if (objects[i].overrideMaterialIndex.empty()) {
					ImGui::Text("Using default materials:");
				}

				for (int j = 0; j < meshCount; j++) {
					std::string label = "Material[" + std::to_string(j) + "]";

					if (objects[i].overrideMaterialIndex.empty()) {
						int defaultMat = model.material[j];
						ImGui::Text("%s: %d (default)", label.c_str(), defaultMat);
					} else {
						int& matIndex = objects[i].overrideMaterialIndex[j];
						matIndex = std::clamp(matIndex, 0, (int)maxMaterialIndex - 1);
						ImGui::SliderInt(label.c_str(), &matIndex, 0, maxMaterialIndex - 1);
					}
				}

				if (!objects[i].overrideMaterialIndex.empty()) {
					if (ImGui::Button("Clear All Overrides")) {
						objects[i].overrideMaterialIndex.clear();
					}
				}
				else {
					if (ImGui::Button("Override All Materials")) {
						objects[i].overrideMaterialIndex.resize(meshCount, -1);
						for (int j = 0; j < meshCount; j++) {
							objects[i].overrideMaterialIndex[j] = model.material[j];
						}
					}
				}
			}

			ImGui::TreePop();
			if (ImGui::Button("Remove Object")) {
				objects.erase(objects.begin() + i);
				ImGui::PopID();
				break;
			}
		}
		ImGui::PopID();
	}

	if (disableUI) { ImGui::EndDisabled(); }

     ImGui::End();

	 ImGui::Begin("Material Editor");

	 static Material newMaterial;
	 static bool showCreate = true;

	 if (showCreate) {
		 ImGui::Text("Create New Material");
		 ImGui::ColorEdit4("Base Color", glm::value_ptr(newMaterial.baseColor));
		 ImGui::SliderFloat("Roughness", &newMaterial.roughness, 0.0f, 1.0f);
		 ImGui::SliderFloat("Metallic", &newMaterial.metallic, 0.0f, 1.0f);
		 ImGui::ColorEdit3("Emissive Color", glm::value_ptr(newMaterial.emissiveFactor));
		 if (ImGui::Button("Create Material")) {
			 newMaterial.ao = 1.0f;
			 newMaterial.albedoTexIndex = -1;
			 newMaterial.normalTexIndex = -1;
			 newMaterial.metallicTexIndex = -1;
			 newMaterial.roughnessTexIndex = -1;
			 newMaterial.aoTexIndex = -1;
			 newMaterial.emissiveTexIndex = -1;

			 scene->getMaxMaterialIndex()++;

			 if (addMaterial) {
				 addMaterial(newMaterial);
			 }
		 }
	 }
	 static int selectedMaterialIndex = 0;
	 int materialCount = scene->getMaxMaterialIndex();

	 ImGui::Separator();
	 ImGui::Text("Edit Existing Material");

	 ImGui::SliderInt("Material Index", &selectedMaterialIndex, 0, materialCount - 1);

	 if (getMaterial) {
		 Material& mat = getMaterial(selectedMaterialIndex);
		 ImGui::ColorEdit4("Base Color##edit", glm::value_ptr(mat.baseColor));
		 ImGui::SliderFloat("Roughness##edit", &mat.roughness, 0.0f, 1.0f);
		 ImGui::SliderFloat("Metallic##edit", &mat.metallic, 0.0f, 1.0f);
		 ImGui::ColorEdit3("Emissive Color##edit", glm::value_ptr(mat.emissiveFactor));
	 }
	 

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


