#include "include/Renderer.h"

std::unique_ptr<Renderer> Renderer::createRenderer(GLFWwindow* window) {
	std::unique_ptr<Renderer> renderer = std::unique_ptr<Renderer>(new Renderer());
	renderer->init(window);
	return renderer;
}

Renderer::~Renderer() {
	cleanup();
}

void Renderer::cleanup() {
	std::cout << "Renderer::cleanup" << std::endl;
	vkDeviceWaitIdle(m_context->getDevice());
}

void Renderer::init(GLFWwindow* window) {
	std::cout << "Renderer::init" << std::endl;
	this->window = window;
	m_context = VulkanContext::createVulkanContext(window);
	m_swapChain = SwapChain::createSwapChain(window, m_context.get());
	m_syncObjects = SyncObjects::createSyncObjects(m_context.get());
	m_commandBuffers = CommandBuffers::createCommandBuffers(m_context.get());
	m_extent = {1024, 1024};
	createDefaultModels();

	// descriptorset layout
	m_globalLayout = DescriptorSetLayout::createGlobalDescriptorSetLayout(m_context.get());
	m_objectMaterialLayout = DescriptorSetLayout::createObjectMaterialDescriptorSetLayout(m_context.get());
	m_bindlessLayout = DescriptorSetLayout::createBindlessDescriptorSetLayout(m_context.get());
	m_attachmentLayout = DescriptorSetLayout::createAttachmentDescriptorSetLayout(m_context.get());
	m_shadowLayout = DescriptorSetLayout::createShadowDescriptorSetLayout(m_context.get());
	m_rayTracingLayout = DescriptorSetLayout::createRayTracingDescriptorSetLayout(m_context.get());

	// buffers
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_cameraBuffers[i] = UniformBuffer::createUniformBuffer(m_context.get(), sizeof(CameraBuffer));
		m_lightBuffers[i] = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(LightBuffer));
		m_objectInstanceBuffers[i] = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(ObjectInstance) * MAX_OBJECT_COUNT * 2);
		m_modelBuffers[i] = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(ModelBuffer) * MAX_OBJECT_COUNT);
		m_materialBuffers[i] = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(Material) * MAX_MATERIAL_COUNT * 2);
		m_lightMatrixBuffers[i] = UniformBuffer::createUniformBuffer(m_context.get(), sizeof(LightMatrix) * 13);
		m_renderOptionsBuffers[i] = UniformBuffer::createUniformBuffer(m_context.get(), sizeof(RenderOptions));
	}

	// renderpass
	m_gbufferRenderPass = RenderPass::createGbufferRenderPass(m_context.get());
	m_imguiRenderPass = RenderPass::createImGuiRenderPass(m_context.get(), m_swapChain.get());
	m_lightPassRenderPass = RenderPass::createLightPassRenderPass(m_context.get());
	m_shadowMapRenderPass = RenderPass::createShadowMapRenderPass(m_context.get());

	// attachment
	m_gbufferAttachments.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_gbufferAttachments[i].albedo = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_gbufferAttachments[i].normal = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_gbufferAttachments[i].position = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_gbufferAttachments[i].pbr = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_gbufferAttachments[i].emissive = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_gbufferAttachments[i].depth = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	m_outputTextures.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_outputTextures[i] = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	m_shadowMapTextures.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_shadowMapTextures[i].resize(7); // directional light : 3 (csm), spot light : 4
		for (int j = 0; j < 7; j++) {
			m_shadowMapTextures[i][j] = Texture::createAttachmentTexture(m_context.get(), 2048, 2048, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
		}
	}

	m_shadowCubeMapTextures.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_shadowCubeMapTextures[i] = Texture::createCubeMapTexture(m_context.get(), 2048, 2048, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	m_rtReflectionTextures.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_rtReflectionTextures[i] = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	// framebuffer
	m_gbufferFrameBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	m_outputFrameBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_gbufferFrameBuffers[i] = FrameBuffer::createGbufferFrameBuffer(m_context.get(), m_gbufferRenderPass.get(), m_gbufferAttachments[i], m_extent);
		m_outputFrameBuffers[i] = FrameBuffer::createOutputFrameBuffer(m_context.get(), m_lightPassRenderPass.get(), m_outputTextures[i].get(), m_extent);
	}

	m_imguiFrameBuffers.resize(m_swapChain->getSwapChainImages().size());
	for (int i = 0; i < m_swapChain->getSwapChainImages().size(); i++) {
		m_imguiFrameBuffers[i] = FrameBuffer::createImGuiFrameBuffer(m_context.get(), m_imguiRenderPass.get(), m_swapChain->getSwapChainImageViews()[i], m_swapChain->getSwapChainExtent());
	}

	m_shadowMapFrameBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_shadowMapFrameBuffers[i].resize(7);
		for (int j = 0; j < 7; j++) {
			m_shadowMapFrameBuffers[i][j] = FrameBuffer::createShadowMapFrameBuffer(m_context.get(), m_shadowMapRenderPass.get(), m_shadowMapTextures[i][j].get(), {2048, 2048});
		}
	}

	m_shadowCubeMapFrameBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_shadowCubeMapFrameBuffers[i].resize(6);
		for (int j = 0; j < 6; j++) {
			m_shadowCubeMapFrameBuffers[i][j] = FrameBuffer::createShadowCubeMapFrameBuffer(m_context.get(), m_shadowMapRenderPass.get(), m_shadowCubeMapTextures[i].get(), {2048, 2048}, j);
		}
	}


	// descriptorset
	m_attachmentDescSets.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_globlaDescSets[i] = DescriptorSet::createGlobalDescriptorSet(m_context.get(), m_globalLayout.get(), m_cameraBuffers[i].get(), m_lightBuffers[i].get(), m_renderOptionsBuffers[i].get());
		m_objectMaterialDescSets[i] = DescriptorSet::createObjectMaterialDescriptorSet(m_context.get(), m_objectMaterialLayout.get(), m_objectInstanceBuffers[i].get());
		m_bindlessDescSets[i] = DescriptorSet::createBindlessDescriptorSet(m_context.get(), m_bindlessLayout.get(), m_modelBuffers[i].get(), m_materialBuffers[i].get(), m_textureList);
		m_attachmentDescSets[i] = DescriptorSet::createAttachmentDescriptorSet(m_context.get(), m_attachmentLayout.get(), m_gbufferAttachments[i]);
		std::vector<Texture*> shadowTextures;
		for (int j = 0; j < m_shadowMapTextures[i].size(); j++) {
			shadowTextures.push_back(m_shadowMapTextures[i][j].get());
		}
		m_shadowDescSets[i] = DescriptorSet::createShadowDescriptorSet(m_context.get(), m_shadowLayout.get(), m_lightMatrixBuffers[i].get(), shadowTextures, m_shadowCubeMapTextures[i].get());
	}

	// pipeline
	m_gbufferPipeline = Pipeline::createGbufferPipeline(m_context.get(), m_gbufferRenderPass.get(), {m_globalLayout.get(), m_objectMaterialLayout.get(), m_bindlessLayout.get()});
	m_lightPassPipeline = Pipeline::createLightPassPipeline(m_context.get(), m_lightPassRenderPass.get(), { m_globalLayout.get(), m_attachmentLayout.get(), m_shadowLayout.get(), m_rayTracingLayout.get()});
	m_shadowMapPipeline = Pipeline::createShadowMapPipeline(m_context.get(), m_shadowMapRenderPass.get(), { m_objectMaterialLayout.get(), m_bindlessLayout.get() });
	m_reflectionPipeline = RayTracingPipeline::createReflectionPipeline(m_context.get(), { m_globalLayout.get(), m_rayTracingLayout.get(), m_objectMaterialLayout.get(), m_bindlessLayout.get(), m_attachmentLayout.get() });
	m_giPipeline = RayTracingPipeline::createGIPipeline(m_context.get(), { m_globalLayout.get(), m_rayTracingLayout.get(), m_objectMaterialLayout.get(), m_bindlessLayout.get(), m_attachmentLayout.get() });

	printAllResources();


	// gui renderer
	m_guiRenderer = GuiRenderer::createGuiRenderer(m_context.get(), window, m_imguiRenderPass.get(), m_swapChain.get());
	m_guiRenderer->setRTEnabled = [this](bool enabled) { m_rtEnabled = enabled; };
	m_guiRenderer->getRTEnabled = [this]() { return m_rtEnabled; };
	m_guiRenderer->addMaterial = [this](const Material& material) {
		m_materialList.push_back(material);
		m_materialBuffers[0]->updateStorageBuffer(&m_materialList[0], sizeof(Material) * m_materialList.size());
		m_materialBuffers[1]->updateStorageBuffer(&m_materialList[0], sizeof(Material) * m_materialList.size());
		};
	m_guiRenderer->setRTMode = [this](int32_t mode) {
		m_rtMode = mode;
		};
	m_guiRenderer->getMaterial = [this](int32_t index) -> Material& {
		m_materialBuffers[0]->updateStorageBuffer(&m_materialList[0], sizeof(Material)* m_materialList.size());
		m_materialBuffers[1]->updateStorageBuffer(&m_materialList[0], sizeof(Material)* m_materialList.size());
		if (index < 0 || index >= m_materialList.size()) {
			std::cerr << "Material index out of range!" << std::endl;
			return m_materialList[0];
		}
		return m_materialList[index];
		};
	m_guiRenderer->setReflectionSampleCount = [this](int32_t count) {
		m_reflectionSampleCount = count;
	};
	m_guiRenderer->setReflectionMaxBounce = [this](int32_t bounce) {
		m_reflectionMaxBounce = bounce;
	};

	m_scene = Scene::createScene(m_modelList.size(), m_materialList.size(), m_textureList.size());

	//descriptor set update
	std::vector<Material> materials;
	for (int32_t i = 0; i < m_materialList.size(); i++) {
		materials.push_back(m_materialList[i]);
	}
	m_materialBuffers[0]->updateStorageBuffer(&materials[0], sizeof(Material) * materials.size());
	m_materialBuffers[1]->updateStorageBuffer(&materials[0], sizeof(Material) * materials.size());


	auto cmd = VulkanUtil::beginSingleTimeCommands(m_context.get());
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		for (int j = 0; j < 7; j++) {
			transferImageLayout(cmd, m_shadowMapTextures[i][j].get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
		}
		transferImageLayout(cmd, m_shadowCubeMapTextures[i].get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 6);

		transferImageLayout(cmd,
			m_rtReflectionTextures[i].get(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			0, VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	}
	VulkanUtil::endSingleTimeCommands(m_context.get(), cmd);

	// create bottom level acceleration structure
	for (auto& mesh : m_meshList) {
		std::cout << "create blas" << std::endl;
		auto blas = BottomLevelAS::createBottomLevelAS(m_context.get(), mesh.get());
		m_blasList.push_back(std::move(blas));
	}

	// create top level acceleration structure

	std::vector<ObjectInstance> objDescs;
	std::vector<ModelBuffer> modelBuffers;

	createObjDesc(objDescs, modelBuffers);

	m_tlas.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_tlas[i] = TopLevelAS::createTopLevelAS(m_context.get(), m_blasList, modelBuffers, objDescs);

	}

	m_emptyTLAS = TopLevelAS::createEmptyTopLevelAS(m_context.get());

	m_rtDescSets.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_rtDescSets[i] = DescriptorSet::createRayTracingDescriptorSet(m_context.get(), m_rayTracingLayout.get(), m_rtReflectionTextures[i].get(), m_tlas[i]->getHandle());
	}

	m_guiRenderer->createViewPortDescriptorSet({m_outputTextures[0].get(), m_outputTextures[1].get()});
	m_guiRenderer->createRayTracingDescriptorSet({ m_rtReflectionTextures[0].get(), m_rtReflectionTextures[1].get() });
	m_guiRenderer->createAlbedoDescriptorSet({m_gbufferAttachments[0].albedo.get(), m_gbufferAttachments[1].albedo.get()});
	m_guiRenderer->createPositionDescriptorSet({m_gbufferAttachments[0].position.get(), m_gbufferAttachments[1].position.get()});
	m_guiRenderer->createNormalDescriptorSet({m_gbufferAttachments[0].normal.get(), m_gbufferAttachments[1].normal.get()});
	m_guiRenderer->createPbrDescriptorSet({m_gbufferAttachments[0].pbr.get(), m_gbufferAttachments[1].pbr.get()});
	m_guiRenderer->createEmissiveDescriptorSet({ m_gbufferAttachments[0].emissive.get(), m_gbufferAttachments[1].emissive.get() });
}

void Renderer::update(float deltaTime) {
	updateCamera(deltaTime);
}

void Renderer::render(float deltaTime) {
	vkWaitForFences(m_context->getDevice(), 1, &m_syncObjects->getInFlightFences()[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_context->getDevice(), m_swapChain->getSwapChain(), UINT64_MAX, 
		m_syncObjects->getImageAvailableSemaphores()[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		std::cout << "Swapchain out of date!" << std::endl;
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	ImVec2 newExtent = m_guiRenderer->getViewportSize();
	if (abs((int)newExtent.x - (int)m_extent.width) >= 1 ||
		abs((int)newExtent.y - (int)m_extent.height) >= 1) {
		recreateViewport(newExtent);
	}

	vkResetFences(m_context->getDevice(), 1, &m_syncObjects->getInFlightFences()[currentFrame]);

	vkResetCommandBuffer(m_commandBuffers->getCommandBuffers()[currentFrame], 0);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(m_commandBuffers->getCommandBuffers()[currentFrame], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}


	std::vector<ObjectInstance> objDescs;
	std::vector<ModelBuffer> modelBuffers;

	createObjDesc(objDescs, modelBuffers);

	updateTLAS(objDescs, modelBuffers);

	//printObjectInstances(objDescs);



	recordShadowMapCommandBuffer(objDescs);

	for (int i = 0; i < 7; i++) {
		transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame], m_shadowMapTextures[currentFrame][i].get(),
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	}

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame], m_shadowCubeMapTextures[currentFrame].get(),
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 6);


	// record command buffer
	recordGbufferCommandBuffer(objDescs);


	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame], m_gbufferAttachments[currentFrame].albedo.get(), 
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, 
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame], m_gbufferAttachments[currentFrame].normal.get()
		, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame], m_gbufferAttachments[currentFrame].position.get()
		, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame], m_gbufferAttachments[currentFrame].pbr.get()
		, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame], m_gbufferAttachments[currentFrame].emissive.get()
		, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_rtReflectionTextures[currentFrame].get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);


	if (m_rtMode == 0) {

	}
	else if (m_rtMode == 1) {
		recordReflectionCommandBuffer();
	}
	else if (m_rtMode == 2) {
		recordGICmdBuffer();
	}
	RenderOptions renderOptions;
	renderOptions.useRTReflection = 0;
	renderOptions.rtMode = m_rtMode;
	renderOptions.sampleCount = m_reflectionSampleCount;
	renderOptions.maxBounce = m_reflectionMaxBounce;
	m_renderOptionsBuffers[currentFrame]->updateUniformBuffer(&renderOptions, sizeof(RenderOptions));

	
	recordLightPassCommandBuffer();


	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame], m_outputTextures[currentFrame].get(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	for (int i = 0; i < 7; i++) {
		transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame], m_shadowMapTextures[currentFrame][i].get(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
	}

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame], m_shadowCubeMapTextures[currentFrame].get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 6);
	
	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_rtReflectionTextures[currentFrame].get(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	recordImGuiCommandBuffer(imageIndex, deltaTime);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame], m_gbufferAttachments[currentFrame].albedo.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame], m_gbufferAttachments[currentFrame].normal.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame], m_gbufferAttachments[currentFrame].position.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame], m_gbufferAttachments[currentFrame].pbr.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame], m_gbufferAttachments[currentFrame].emissive.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame], m_outputTextures[currentFrame].get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);



	// end

	if (vkEndCommandBuffer(m_commandBuffers->getCommandBuffers()[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_syncObjects->getImageAvailableSemaphores()[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers->getCommandBuffers()[currentFrame];

	VkSemaphore signalSemaphores[] = { m_syncObjects->getRenderFinishedSemaphores()[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_context->getGraphicsQueue(), 1, &submitInfo, m_syncObjects->getInFlightFences()[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_swapChain->getSwapChain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(m_context->getPresentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		recreateSwapChain();
		std::cout << "Swapchain out of date!" << std::endl;
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::recreateSwapChain() {
	vkDeviceWaitIdle(m_context->getDevice());

	int32_t width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	m_imguiFrameBuffers.clear();
	m_swapChain.reset();
	m_swapChain = SwapChain::createSwapChain(window, m_context.get());


	m_imguiFrameBuffers.resize(m_swapChain->getSwapChainImages().size());
	for (int i = 0; i < m_swapChain->getSwapChainImages().size(); i++) {
		m_imguiFrameBuffers[i] = FrameBuffer::createImGuiFrameBuffer(m_context.get(), m_imguiRenderPass.get(), m_swapChain->getSwapChainImageViews()[i], m_swapChain->getSwapChainExtent());

	}

}

void Renderer::recreateViewport(ImVec2 newExtent) {
	vkDeviceWaitIdle(m_context->getDevice());

	std::cout << "newExtent: " << newExtent.x << " " << newExtent.y << std::endl;
	std::cout << "m_extent: " << m_extent.width << " " << m_extent.height << std::endl;

	if (newExtent.x <= 0 || newExtent.y <= 0) {
		return;
	}

	m_extent.width = static_cast<uint32_t>(newExtent.x);
	m_extent.height = static_cast<uint32_t>(newExtent.y);
	
	m_outputFrameBuffers.clear();
	m_outputTextures.clear();
	m_attachmentDescSets.clear();
	m_gbufferFrameBuffers.clear();
	m_gbufferAttachments.clear();

	m_rtDescSets.clear();
	m_rtReflectionTextures.clear();

	m_gbufferAttachments.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_gbufferAttachments[i].albedo = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_gbufferAttachments[i].normal = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_gbufferAttachments[i].position = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_gbufferAttachments[i].pbr = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_gbufferAttachments[i].emissive = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_gbufferAttachments[i].depth = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
	}
	
	m_gbufferFrameBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_gbufferFrameBuffers[i] = FrameBuffer::createGbufferFrameBuffer(m_context.get(), m_gbufferRenderPass.get(), m_gbufferAttachments[i], m_extent);
	}

	m_attachmentDescSets.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_attachmentDescSets[i] = DescriptorSet::createAttachmentDescriptorSet(m_context.get(), m_attachmentLayout.get(), m_gbufferAttachments[i]);
	}

	m_outputTextures.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_outputTextures[i] = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	m_outputFrameBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_outputFrameBuffers[i] = FrameBuffer::createOutputFrameBuffer(m_context.get(), m_lightPassRenderPass.get(), m_outputTextures[i].get(), m_extent);
	}

	m_rtReflectionTextures.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_rtReflectionTextures[i] = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	m_rtDescSets.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_rtDescSets[i] = DescriptorSet::createRayTracingDescriptorSet(m_context.get(), m_rayTracingLayout.get(), m_rtReflectionTextures[i].get(), m_tlas[i]->getHandle());
	}
	

	auto cmd = VulkanUtil::beginSingleTimeCommands(m_context.get());
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		transferImageLayout(cmd,
			m_rtReflectionTextures[i].get(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			0, VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	}
	VulkanUtil::endSingleTimeCommands(m_context.get(), cmd);

	m_guiRenderer->createViewPortDescriptorSet({ m_outputTextures[0].get(), m_outputTextures[1].get() });
	m_guiRenderer->createRayTracingDescriptorSet({ m_rtReflectionTextures[0].get(), m_rtReflectionTextures[1].get() });
	m_guiRenderer->createAlbedoDescriptorSet({m_gbufferAttachments[0].albedo.get(), m_gbufferAttachments[1].albedo.get()});
	m_guiRenderer->createPositionDescriptorSet({m_gbufferAttachments[0].position.get(), m_gbufferAttachments[1].position.get()});
	m_guiRenderer->createNormalDescriptorSet({m_gbufferAttachments[0].normal.get(), m_gbufferAttachments[1].normal.get()});
	m_guiRenderer->createPbrDescriptorSet({m_gbufferAttachments[0].pbr.get(), m_gbufferAttachments[1].pbr.get()});
	m_guiRenderer->createEmissiveDescriptorSet({ m_gbufferAttachments[0].emissive.get(), m_gbufferAttachments[1].emissive.get() });


}

void Renderer::createDefaultModels()
{
	glm::vec4 defaultColor = glm::vec4(1.0f);

	auto defaultTex = Texture::createDefaultTexture(m_context.get(), defaultColor);
	int32_t texIndex = static_cast<int32_t>(m_textureList.size());
	m_textureList.push_back(std::move(defaultTex));

	Material defaultMat{};
	defaultMat.albedoTexIndex = texIndex;
	defaultMat.baseColor = defaultColor;
	int32_t matIndex = static_cast<int32_t>(m_materialList.size());
	m_materialList.push_back(defaultMat);

	Material redMat{};
	redMat.albedoTexIndex = -1;
	redMat.baseColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	m_materialList.push_back(redMat);

	Material greenMat{};
	greenMat.albedoTexIndex = -1;
	greenMat.baseColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	m_materialList.push_back(greenMat);

	Material blueMat{};
	blueMat.albedoTexIndex = -1;
	blueMat.baseColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	m_materialList.push_back(blueMat);

	Material iron1{};
	iron1.baseColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	iron1.metallic = 1.0f;
	iron1.roughness = 0.1f;
	m_materialList.push_back(iron1);

	Material iron2{};
	iron2.baseColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	iron2.metallic = 1.0f;
	iron2.roughness = 0.2f;
	m_materialList.push_back(iron2);

	Material iron3{};
	iron3.baseColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	iron3.metallic = 1.0f;
	iron3.roughness = 0.3f;
	m_materialList.push_back(iron3);

	auto createAndRegisterMesh = [&](std::unique_ptr<Mesh> mesh) -> int32_t {
		int32_t meshIndex = static_cast<int32_t>(m_meshList.size());
		m_meshList.push_back(std::move(mesh));
		return meshIndex;
	};

	// Plane
	{
		Model model;
		model.mesh.push_back(createAndRegisterMesh(Mesh::createPlaneMesh(m_context.get())));
		model.material.push_back(matIndex);
		m_modelList.push_back(model);
	}

	// Box
	{
		Model model;
		model.mesh.push_back(createAndRegisterMesh(Mesh::createBoxMesh(m_context.get())));
		model.material.push_back(matIndex);
		m_modelList.push_back(model);
	}

	// Sphere
	{
		Model model;
		model.mesh.push_back(createAndRegisterMesh(Mesh::createSphereMesh(m_context.get())));
		model.material.push_back(matIndex);
		m_modelList.push_back(model);
	}

	std::cout << "[Renderer] Default Models Loaded: Plane, Box, Sphere" << std::endl;
}

void Renderer::loadModel(const std::string& modelPath) {
	Model model = ModelLoader::loadGLTFModel(modelPath, m_context.get(), m_meshList, m_textureList, m_materialList, m_texturePathMap);
	m_modelList.push_back(model);
}


void Renderer::printAllResources() {
	std::cout << "======== [Renderer Resources] ========" << std::endl;

	std::cout << "\n[Meshes] Count: " << m_meshList.size() << std::endl;
	for (size_t i = 0; i < m_meshList.size(); ++i)
		std::cout << " * Mesh[" << i << "] pointer: " << m_meshList[i].get() << std::endl;

	std::cout << "\n[Textures] Count: " << m_textureList.size() << std::endl;
	for (size_t i = 0; i < m_textureList.size(); ++i)
		std::cout << " * Texture[" << i << "] pointer: " << m_textureList[i].get() << std::endl;

	std::cout << "\n[Materials] Count: " << m_materialList.size() << std::endl;
	for (size_t i = 0; i < m_materialList.size(); ++i) {
		std::cout << " * Material[" << i << "]" << std::endl;
		printMaterial(m_materialList[i]);
	}

	std::cout << "\n[Models] Count: " << m_modelList.size() << std::endl;
	for (size_t i = 0; i < m_modelList.size(); ++i) {
		std::cout << " * Model[" << i << "]" << std::endl;
		printModel(m_modelList[i]);
	}

	std::cout << "=======================================" << std::endl;
}


void Renderer::createObjDesc(std::vector<ObjectInstance>& ObjDescs, std::vector<ModelBuffer>& modelBuffers) {
	auto& objects = m_scene->getObjects();
	std::unordered_map<int32_t, std::vector<int32_t>> objectMap;
	modelBuffers.resize(objects.size());
	for (int32_t i = 0; i < objects.size(); i++) {
		objectMap[objects[i].modelIndex].push_back(i);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, objects[i].position);
		model = glm::rotate(model, glm::radians(objects[i].rotation.x), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(objects[i].rotation.y), glm::vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(objects[i].rotation.z), glm::vec3(0, 0, 1));
		model = glm::scale(model, objects[i].scale);
		modelBuffers[i].model = model;
	}
	if (!modelBuffers.empty()) {
		m_modelBuffers[currentFrame]->updateStorageBuffer(&modelBuffers[0], sizeof(ModelBuffer) * modelBuffers.size());
	}

	ObjDescs.reserve(objects.size() * 4);
	for (const auto& [key, value] : objectMap) {
		for (int32_t i = 0; i < m_modelList[key].mesh.size(); i++) {
			for (int32_t j = 0; j < value.size(); j++) {
				int32_t modelMatrixIdx = value[j];
				int32_t materialIndex;
				if (objects[modelMatrixIdx].overrideMaterialIndex.size() > i) {
					materialIndex = objects[modelMatrixIdx].overrideMaterialIndex[i];
				}
				else {
					materialIndex = m_modelList[key].material[i];
				}
				int32_t meshIdx = m_modelList[key].mesh[i];
				uint64_t vertexAddress = m_meshList[meshIdx]->getVertexBuffer()->getDeviceAddress();
				uint64_t indexAddress = m_meshList[meshIdx]->getIndexBuffer()->getDeviceAddress();
				ObjDescs.push_back({ vertexAddress, indexAddress, modelMatrixIdx, materialIndex, meshIdx, 0 });
			}
		}
	}
	if (!ObjDescs.empty()) {
		m_objectInstanceBuffers[currentFrame]->updateStorageBuffer(&ObjDescs[0], sizeof(ObjectInstance) * ObjDescs.size());
	}
}

void Renderer::recordGbufferCommandBuffer(std::vector<ObjectInstance>& objDescs) {
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_gbufferRenderPass->getRenderPass();
	renderPassInfo.framebuffer = m_gbufferFrameBuffers[currentFrame]->getFrameBuffer();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_extent;

	std::array<VkClearValue, 6> clearValues{};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[2].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[3].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[4].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[5].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(m_commandBuffers->getCommandBuffers()[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(m_commandBuffers->getCommandBuffers()[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_gbufferPipeline->getPipeline());

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_extent.width);
	viewport.height = static_cast<float>(m_extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_commandBuffers->getCommandBuffers()[currentFrame], 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_extent;
	vkCmdSetScissor(m_commandBuffers->getCommandBuffers()[currentFrame], 0, 1, &scissor);

	// camera buffer update
	CameraBuffer cameraBuffer{};
	cameraBuffer.view = glm::lookAt(m_camera.position, m_camera.position + m_camera.front, m_camera.up);
	cameraBuffer.proj = glm::perspective(glm::radians(45.0f), (float)m_extent.width / (float)m_extent.height, 0.1f, 100.0f);
	cameraBuffer.proj[1][1] *= -1;
	cameraBuffer.camPos = m_camera.position;
	cameraBuffer.frameCount = m_frameCount++;

	m_cameraBuffers[currentFrame]->updateUniformBuffer(&cameraBuffer, sizeof(CameraBuffer));

	// bind descriptor sets
	vkCmdBindDescriptorSets(m_commandBuffers->getCommandBuffers()[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_gbufferPipeline->getPipelineLayout(), 0, 1, &m_globlaDescSets[currentFrame]->getDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_commandBuffers->getCommandBuffers()[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_gbufferPipeline->getPipelineLayout(), 1, 1, &m_objectMaterialDescSets[currentFrame]->getDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_commandBuffers->getCommandBuffers()[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_gbufferPipeline->getPipelineLayout(), 2, 1, &m_bindlessDescSets[currentFrame]->getDescriptorSet(), 0, nullptr);

	/*int32_t index = 0;
	for (const auto& [key, value] : modelToMatrixIndices) {
		for (int32_t i = 0; i < m_modelList[key].mesh.size(); i++) {
			int32_t startIndex = index;
			m_meshList[m_modelList[key].mesh[i]]->drawInstance(m_commandBuffers->getCommandBuffers()[currentFrame], value.size(), startIndex);
			index += value.size();
		}
	}*/
	int32_t currentMeshIdx = -1;
	uint32_t firstInstance = 0;
	uint32_t instanceCount = 0;

	for (uint32_t i = 0; i < objDescs.size(); i++) {
		int32_t meshIdx = objDescs[i].meshIndex;
		if (meshIdx != currentMeshIdx) {
			if (instanceCount > 0 && currentMeshIdx >= 0) {
				m_meshList[currentMeshIdx]->drawInstance(m_commandBuffers->getCommandBuffers()[currentFrame], instanceCount, firstInstance);
			}
			currentMeshIdx = meshIdx;
			firstInstance = i;
			instanceCount = 1;
		}
		else {
			instanceCount++;
		}
	}
	if (instanceCount > 0 && currentMeshIdx >= 0) {
		m_meshList[currentMeshIdx]->drawInstance(m_commandBuffers->getCommandBuffers()[currentFrame], instanceCount, firstInstance);
	}
	vkCmdEndRenderPass(m_commandBuffers->getCommandBuffers()[currentFrame]);
}

void Renderer::recordLightPassCommandBuffer() {
	VkClearValue clearValue{};
	clearValue.color = { {0.0f, 1.0f, 0.0f, 1.0f} };

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_lightPassRenderPass->getRenderPass();
	renderPassInfo.framebuffer = m_outputFrameBuffers[currentFrame]->getFrameBuffer();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_extent;
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(m_commandBuffers->getCommandBuffers()[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_extent.width);
	viewport.height = static_cast<float>(m_extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_commandBuffers->getCommandBuffers()[currentFrame], 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_extent;
	vkCmdSetScissor(m_commandBuffers->getCommandBuffers()[currentFrame], 0, 1, &scissor);

	vkCmdBindPipeline(m_commandBuffers->getCommandBuffers()[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_lightPassPipeline->getPipeline());

	VkDescriptorSet sets[] = {
		m_globlaDescSets[currentFrame]->getDescriptorSet(),
		m_attachmentDescSets[currentFrame]->getDescriptorSet(),
		m_shadowDescSets[currentFrame]->getDescriptorSet(),
		m_rtDescSets[currentFrame]->getDescriptorSet()
	};
	vkCmdBindDescriptorSets(m_commandBuffers->getCommandBuffers()[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_lightPassPipeline->getPipelineLayout(), 0, 4, sets, 0, nullptr);

	LightBuffer lightBuffer;
	memset(&lightBuffer, 0, sizeof(LightBuffer));
	auto& lights = m_scene->getLights();
	for (uint32_t i = 0; i < lights.size(); i++) {
		lightBuffer.lights[i] = lights[i];
	}
	lightBuffer.ambientColor = m_scene->getAmbientColor();
	lightBuffer.lightCount = static_cast<uint32_t>(lights.size());

	m_lightBuffers[currentFrame]->updateStorageBuffer(&lightBuffer, sizeof(LightBuffer));


	vkCmdDraw(m_commandBuffers->getCommandBuffers()[currentFrame], 6, 1, 0, 0);

	vkCmdEndRenderPass(m_commandBuffers->getCommandBuffers()[currentFrame]);
}

void Renderer::recordShadowMapCommandBuffer(std::vector<ObjectInstance>& objDescs) {
	auto& lights = m_scene->getLights();

	int32_t directionalCount = 0;
	int32_t spotCount = 0;
	int32_t pointCount = 0;

	std::array<LightMatrix, 13> lightMatrices;
	memset(&lightMatrices, 0, sizeof(lightMatrices));
	for (int i = 0; i < lights.size(); ++i) {
		Light& light = lights[i];
		light.shadowMapIndex = -1;

		if (!light.castsShadow)
			continue;

		int type = light.type;
		int shadowMapIndex = 0;

		if (type == LIGHT_TYPE_POINT && pointCount > 0) // point light 1개만 처리
			continue;

		if (type == LIGHT_TYPE_DIRECTIONAL && directionalCount > 0)
			continue;

		if (type == LIGHT_TYPE_SPOT && spotCount > 3)
			continue;

		if (type == LIGHT_TYPE_DIRECTIONAL) {
			directionalCount++;
		}

		if (type == LIGHT_TYPE_SPOT) {
			shadowMapIndex = 3 + spotCount;
			spotCount++;
		}

		if (type == LIGHT_TYPE_POINT) {
			pointCount++;
			shadowMapIndex = 7;
		}

		VkClearValue clearValue{};
		clearValue.depthStencil = { 1.0f, 0 };


		if (type == LIGHT_TYPE_POINT) {
			for (int j = 0; j < 6; j++) {
				glm::mat4 lightViewProj = computePointLightMatrix(light, j);

				VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = m_shadowMapRenderPass->getRenderPass();
				renderPassInfo.framebuffer = m_shadowCubeMapFrameBuffers[currentFrame][j]->getFrameBuffer();
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = { 2048, 2048 };
				renderPassInfo.clearValueCount = 1;
				renderPassInfo.pClearValues = &clearValue;

				vkCmdBeginRenderPass(m_commandBuffers->getCommandBuffers()[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
				vkCmdBindPipeline(m_commandBuffers->getCommandBuffers()[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_shadowMapPipeline->getPipeline());
				vkCmdBindDescriptorSets(m_commandBuffers->getCommandBuffers()[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_shadowMapPipeline->getPipelineLayout(), 0, 1, &m_objectMaterialDescSets[currentFrame]->getDescriptorSet(), 0, nullptr);
				vkCmdBindDescriptorSets(m_commandBuffers->getCommandBuffers()[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_shadowMapPipeline->getPipelineLayout(), 1, 1, &m_bindlessDescSets[currentFrame]->getDescriptorSet(), 0, nullptr);
				vkCmdPushConstants(
					m_commandBuffers->getCommandBuffers()[currentFrame],
					m_shadowMapPipeline->getPipelineLayout(),
					VK_SHADER_STAGE_VERTEX_BIT,
					0, sizeof(glm::mat4),
					&lightViewProj
				);
				/*
				int32_t index = 0;
				for (const auto& [key, value] : modelToMatrixIndices) {
					for (int32_t i = 0; i < m_modelList[key].mesh.size(); i++) {
						int32_t startIndex = index;
						m_meshList[m_modelList[key].mesh[i]]->drawInstance(m_commandBuffers->getCommandBuffers()[currentFrame], value.size(), startIndex);
						index += value.size();
					}
				}
				*/

				int32_t currentMeshIdx = -1;
				uint32_t firstInstance = 0;
				uint32_t instanceCount = 0;

				for (uint32_t i = 0; i < objDescs.size(); i++) {
					int32_t meshIdx = objDescs[i].meshIndex;
					if (meshIdx != currentMeshIdx) {
						if (instanceCount > 0 && currentMeshIdx >= 0) {
							m_meshList[currentMeshIdx]->drawInstance(m_commandBuffers->getCommandBuffers()[currentFrame], instanceCount, firstInstance);
						}
						currentMeshIdx = meshIdx;
						firstInstance = i;
						instanceCount = 1;
					}
					else {
						instanceCount++;
					}
				}
				if (instanceCount > 0 && currentMeshIdx >= 0) {
					m_meshList[currentMeshIdx]->drawInstance(m_commandBuffers->getCommandBuffers()[currentFrame], instanceCount, firstInstance);
				}
				light.shadowMapIndex = shadowMapIndex;
				lightMatrices[shadowMapIndex + j].mat = lightViewProj;
				vkCmdEndRenderPass(m_commandBuffers->getCommandBuffers()[currentFrame]);
			}
		}
		else {

			glm::mat4 lightViewProj = computeLightMatrix(light);

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_shadowMapRenderPass->getRenderPass();
			renderPassInfo.framebuffer = m_shadowMapFrameBuffers[currentFrame][shadowMapIndex]->getFrameBuffer();
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = { 2048, 2048 };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearValue;

			vkCmdBeginRenderPass(m_commandBuffers->getCommandBuffers()[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(m_commandBuffers->getCommandBuffers()[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_shadowMapPipeline->getPipeline());
			vkCmdBindDescriptorSets(m_commandBuffers->getCommandBuffers()[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_shadowMapPipeline->getPipelineLayout(), 0, 1, &m_objectMaterialDescSets[currentFrame]->getDescriptorSet(), 0, nullptr);
			vkCmdBindDescriptorSets(m_commandBuffers->getCommandBuffers()[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_shadowMapPipeline->getPipelineLayout(), 1, 1, &m_bindlessDescSets[currentFrame]->getDescriptorSet(), 0, nullptr);

			vkCmdPushConstants(
				m_commandBuffers->getCommandBuffers()[currentFrame],
				m_shadowMapPipeline->getPipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT,
				0, sizeof(glm::mat4),
				&lightViewProj
			);
			/*
			int32_t index = 0;
			for (const auto& [key, value] : modelToMatrixIndices) {
				for (int32_t i = 0; i < m_modelList[key].mesh.size(); i++) {
					int32_t startIndex = index;
					m_meshList[m_modelList[key].mesh[i]]->drawInstance(m_commandBuffers->getCommandBuffers()[currentFrame], value.size(), startIndex);
					index += value.size();
				}
			}
			*/

			int32_t currentMeshIdx = -1;
			uint32_t firstInstance = 0;
			uint32_t instanceCount = 0;

			for (uint32_t i = 0; i < objDescs.size(); i++) {
				int32_t meshIdx = objDescs[i].meshIndex;
				if (meshIdx != currentMeshIdx) {
					if (instanceCount > 0 && currentMeshIdx >= 0) {
						m_meshList[currentMeshIdx]->drawInstance(m_commandBuffers->getCommandBuffers()[currentFrame], instanceCount, firstInstance);
					}
					currentMeshIdx = meshIdx;
					firstInstance = i;
					instanceCount = 1;
				}
				else {
					instanceCount++;
				}
			}
			if (instanceCount > 0 && currentMeshIdx >= 0) {
				m_meshList[currentMeshIdx]->drawInstance(m_commandBuffers->getCommandBuffers()[currentFrame], instanceCount, firstInstance);
			}



			light.shadowMapIndex = shadowMapIndex;
			lightMatrices[shadowMapIndex].mat = lightViewProj;
			vkCmdEndRenderPass(m_commandBuffers->getCommandBuffers()[currentFrame]);
		}

	}
	m_lightMatrixBuffers[currentFrame]->updateUniformBuffer(&lightMatrices[0], sizeof(LightMatrix) * lightMatrices.size());
}

void Renderer::recordImGuiCommandBuffer(uint32_t imageIndex, float deltaTime) {
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_imguiRenderPass->getRenderPass();
	renderPassInfo.framebuffer = m_imguiFrameBuffers[imageIndex]->getFrameBuffer();
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();

	std::array<VkClearValue, 1> clearValues{};
	clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(m_commandBuffers->getCommandBuffers()[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_guiRenderer->newFrame();
	m_guiRenderer->render(currentFrame, m_commandBuffers->getCommandBuffers()[currentFrame], m_scene.get(), m_modelList, deltaTime);
	vkCmdEndRenderPass(m_commandBuffers->getCommandBuffers()[currentFrame]);
}

void Renderer::transferImageLayout( VkCommandBuffer cmd, Texture* texture, VkImageLayout oldLayout, VkImageLayout newLayout,
	VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, uint32_t layerCount) {
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;
	barrier.image = texture->getImageBuffer()->getImage();
	if (texture->getFormat() == VK_FORMAT_D32_SFLOAT) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layerCount;

	vkCmdPipelineBarrier(
		cmd,
		srcStage,
		dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}


void Renderer::recordReflectionCommandBuffer() {
	VkCommandBuffer cmd = m_commandBuffers->getCommandBuffers()[currentFrame];

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_reflectionPipeline->getPipeline());

	VkDescriptorSet sets[] = {
		m_globlaDescSets[currentFrame]->getDescriptorSet(),  // set=0 (camera)
		m_rtDescSets[currentFrame]->getDescriptorSet(),       // set=1 (outputImage + TLAS)
		m_objectMaterialDescSets[currentFrame]->getDescriptorSet(), // set=2 (object material)
		m_bindlessDescSets[currentFrame]->getDescriptorSet(), // set=3 (bindless)
		m_attachmentDescSets[currentFrame]->getDescriptorSet() // set=4 (gbuffer)
	};
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		m_reflectionPipeline->getPipelineLayout(), 0, 5, sets, 0, nullptr);

	VkStridedDeviceAddressRegionKHR emptyRegion{};
	g_vkCmdTraceRaysKHR(
		cmd,
		&m_reflectionPipeline->getRaygenRegion(),
		&m_reflectionPipeline->getMissRegion(),
		&m_reflectionPipeline->getHitRegion(),
		&emptyRegion,
		m_extent.width,
		m_extent.height,
		1);
}

void Renderer::recordGICmdBuffer() {
	VkCommandBuffer cmd = m_commandBuffers->getCommandBuffers()[currentFrame];
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_giPipeline->getPipeline());

	VkDescriptorSet sets[] = {
		m_globlaDescSets[currentFrame]->getDescriptorSet(),  // set=0 (camera)
		m_rtDescSets[currentFrame]->getDescriptorSet(),       // set=1 (outputImage + TLAS)
		m_objectMaterialDescSets[currentFrame]->getDescriptorSet(), // set=2 (object material)
		m_bindlessDescSets[currentFrame]->getDescriptorSet(), // set=3 (bindless)
		m_attachmentDescSets[currentFrame]->getDescriptorSet() // set=4 (gbuffer)
	};
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		m_giPipeline->getPipelineLayout(), 0, 5, sets, 0, nullptr);

	VkStridedDeviceAddressRegionKHR emptyRegion{};
	g_vkCmdTraceRaysKHR(
		cmd,
		&m_giPipeline->getRaygenRegion(),
		&m_giPipeline->getMissRegion(),
		&m_giPipeline->getHitRegion(),
		&emptyRegion,
		m_extent.width,
		m_extent.height,
		1);
}


void Renderer::updateCamera(float deltaTime) {

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		if (!m_mousePressed) {
			m_mousePressed = true;
			m_lastMouseX = xpos;
			m_lastMouseY = ypos;
		}

		float xoffset = static_cast<float>(xpos - m_lastMouseX);
		float yoffset = static_cast<float>(m_lastMouseY - ypos);

		m_lastMouseX = xpos;
		m_lastMouseY = ypos;

		xoffset *= m_mouseSensitivity;
		yoffset *= m_mouseSensitivity;

		m_yaw += xoffset;
		m_pitch += yoffset;

		m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

		glm::vec3 direction;
		direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		direction.y = sin(glm::radians(m_pitch));
		direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		m_camera.front = glm::normalize(direction);
	}
	else {
		m_mousePressed = false;
	}

	glm::vec3 right = glm::normalize(glm::cross(m_camera.front, m_camera.up));
	glm::vec3 move = glm::vec3(0.0f);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += m_camera.front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= m_camera.front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= right;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += right;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) move -= m_camera.up;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) move += m_camera.up;

	if (glm::length(move) > 0.0f) {
		move = glm::normalize(move);
		m_camera.position += move * m_moveSpeed * deltaTime;
	}
}

glm::mat4 Renderer::computeLightMatrix(Light& light) {
	glm::mat4 lightView;
	glm::mat4 lightProj;
	glm::vec3 lightDir = glm::normalize(light.direction);
	glm::vec3 up = (glm::abs(lightDir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

	if (light.type == LIGHT_TYPE_DIRECTIONAL) {
		/*glm::vec3 eye = glm::vec3(0.0f) - glm::normalize(lightDir) * 120.0f;
		glm::vec3 center = glm::vec3(0.0f);*/
		/*lightView = glm::lookAt(eye, center, up);
		lightProj = glm::ortho(-80.f, 80.f, -80.f, 80.f, -50.0f, 250.f);
		lightProj[1][1] *= -1.0f;*/
		glm::vec3 eye = glm::vec3(0.0f) - glm::normalize(lightDir) * 10.0f;
		glm::vec3 center = glm::vec3(0.0f);
		lightView = glm::lookAt(eye, center, up);
		lightProj = glm::ortho(-10.f, 10.f, -10.f, 10.f, -10.0f, 20.f);
		lightProj[1][1] *= -1.0f;
	}
	else if (light.type == LIGHT_TYPE_SPOT) {
		glm::vec3 eye = light.position;
		glm::vec3 target = light.position + glm::normalize(lightDir);

		lightView = glm::lookAt(eye, target, up);
		lightProj = glm::perspective(glm::radians(light.spotOuterAngle * 2.0f), 1.0f, 0.1f, 100.0f);
		lightProj[1][1] *= -1.0f;
	}

	return lightProj * lightView;
}

glm::mat4 Renderer::computePointLightMatrix(Light& light, uint32_t faceIndex) {
	static const glm::vec3 targets[6] = {
		glm::vec3(1,  0,  0), // +X
		glm::vec3(-1,  0,  0), // -X
		glm::vec3(0,  1,  0), // +Y
		glm::vec3(0, -1,  0), // -Y
		glm::vec3(0,  0,  1), // +Z
		glm::vec3(0,  0, -1)  // -Z
	};
	static const glm::vec3 ups[6] = {
		glm::vec3(0, -1,  0), // +X
		glm::vec3(0, -1,  0), // -X
		glm::vec3(0,  0,  1), // +Y
		glm::vec3(0,  0, -1), // -Y
		glm::vec3(0, -1,  0), // +Z
		glm::vec3(0, -1,  0)  // -Z
	};

	glm::vec3 lightPos = light.position;
	glm::mat4 view = glm::lookAt(lightPos, lightPos + targets[faceIndex], ups[faceIndex]);
	glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 50.0f);
	//proj[1][1] *= -1.0f;

	return proj * view;
}

//void Renderer::updateTLAS(std::unordered_map<int32_t, std::vector<int32_t>>& modelToMatrixIndices, std::vector<ModelBuffer>& modelBuffers) {
void Renderer::updateTLAS(std::vector<ObjectInstance>& objDescs, std::vector<ModelBuffer>& modelBuffers) {

	/*if (modelToMatrixIndices.size() > 0) {
		m_tlas[currentFrame]->rebuild(m_blasList, m_modelList, modelToMatrixIndices, modelBuffers, m_scene->getObjects());
		m_rtDescSets[currentFrame]->updateTLAS(m_tlas[currentFrame]->getHandle());
	}
	else {
		m_rtDescSets[currentFrame]->updateTLAS(m_emptyTLAS->getHandle());
	}*/

	if (objDescs.size() > 0) {
		m_tlas[currentFrame]->rebuild(m_blasList, modelBuffers, objDescs);
		m_rtDescSets[currentFrame]->updateTLAS(m_tlas[currentFrame]->getHandle());
	}
	else {
		m_rtDescSets[currentFrame]->updateTLAS(m_emptyTLAS->getHandle());
	}
}


void Renderer::printObjectInstances(const std::vector<ObjectInstance>& instances) {
	std::cout << "========== ObjectInstance List ==========" << std::endl;
	for (size_t i = 0; i < instances.size(); ++i) {
		const auto& inst = instances[i];
		std::cout << "[" << std::setw(3) << i << "] "
			<< "vertexAddr: 0x" << std::hex << inst.vertexAddress << std::dec << ", "
			<< "indexAddr:  0x" << std::hex << inst.indexAddress << std::dec << ", "
			<< "modelMatIdx: " << inst.modelMatrixIndex << ", "
			<< "materialIdx: " << inst.materialIndex << ", "
			<< "meshIdx: " << inst.meshIndex
			<< std::endl;
	}
	std::cout << "==========================================" << std::endl;
}