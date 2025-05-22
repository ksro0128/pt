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


void Renderer::updateAssets() {
	// default texture
	m_textures.push_back(Texture::createDefaultTexture(m_context.get(), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
	m_materials.push_back(MaterialGPU());


	{ // plane - default material
		std::unique_ptr<Mesh> mesh = Mesh::createPlaneMesh(m_context.get());
		m_meshes.push_back(std::move(mesh));
		Model model;
		model.name = "default plane";
		model.mesh.push_back(m_meshes.size() - 1);
		model.material.push_back(0);
		m_models.push_back(model);
	}
	
	{ // cube - default material
		std::unique_ptr<Mesh> mesh = Mesh::createBoxMesh(m_context.get());
		m_meshes.push_back(std::move(mesh));
		Model model;
		model.name = "default box";
		model.mesh.push_back(m_meshes.size() - 1);
		model.material.push_back(0);
		m_models.push_back(model);
	}

	{ // sphere - default material
		std::unique_ptr<Mesh> mesh = Mesh::createSphereMesh(m_context.get());
		m_meshes.push_back(std::move(mesh));
		Model model;
		model.name = "default sphere";
		model.mesh.push_back(m_meshes.size() - 1);
		model.material.push_back(0);
		m_models.push_back(model);
	}

	// 3
	loadTinyGLTFModel("assets/boombox_1k/boombox_1k.gltf");
	// 4
	loadTinyGLTFModel("assets/lightbulb_01_1k/lightbulb_01_1k.gltf");
	// 5
	loadTinyGLTFModel("assets/Ukulele_01_1k/Ukulele_01_1k.gltf");
	// 6
	loadTinyGLTFModel("assets/rocky_terrain_03_1k/rocky_terrain_03_1k.gltf");
	// 7
	loadTinyGLTFModel("assets/tree_small_02_1k/tree_small_02_1k.gltf");
	// 8
	loadTinyGLTFModel("assets/ornate_mirror_01_1k/ornate_mirror_01_1k.gltf");
	// 9
	loadTinyGLTFModel("assets/glass/glass.gltf");



}

void Renderer::createScene() {

	// 카메라 방향 정리
    glm::vec3 dir = glm::normalize(m_camera.camDir);
    m_pitch = glm::degrees(asin(dir.y));
    m_yaw = glm::degrees(atan2(dir.z, dir.x));
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    m_camera.camRight = glm::normalize(glm::cross(m_camera.camDir, worldUp));
    m_camera.camUp = glm::normalize(glm::cross(m_camera.camRight, m_camera.camDir));
	m_camera.camPos = glm::vec3(0.0f, 0.0f, 5.0f);


	// 바닥
	{
		Object object;
		object.modelIndex = 0; // plane
		object.position = glm::vec3(0.0f, -1.0f, 0.0f);
		object.rotation = glm::vec3(-90.0f, 0.0f, 0.0f); // XY → XZ (법선 +Y → +Z)
		object.scale = glm::vec3(4.0f);
		m_scene.objects.push_back(object);
	}

	// 천장
	{
		Object object;
		object.modelIndex = 0;
		object.position = glm::vec3(0.0f, 3.0f, 0.0f);
		object.rotation = glm::vec3(90.0f, 0.0f, 0.0f); // 바닥 반대
		object.scale = glm::vec3(4.0f);
		m_scene.objects.push_back(object);
	}

	// 뒷벽 (정면, 회전 없음)
	{
		Object object;
		object.modelIndex = 0;
		object.position = glm::vec3(0.0f, 1.0f, -2.0f);
		object.rotation = glm::vec3(0.0f); // 그대로 사용
		object.scale = glm::vec3(4.0f);
		m_scene.objects.push_back(object);
	}

	// 왼쪽 벽
	{
		Object object;
		object.modelIndex = 0;
		object.position = glm::vec3(-2.0f, 1.0f, 0.0f);
		object.rotation = glm::vec3(0.0f, 90.0f, 0.0f); // Y축 회전해서 +X 벽
		object.scale = glm::vec3(4.0f);
		m_scene.objects.push_back(object);
	}

	// 오른쪽 벽
	{
		Object object;
		object.modelIndex = 0;
		object.position = glm::vec3(2.0f, 1.0f, 0.0f);
		object.rotation = glm::vec3(0.0f, -90.0f, 0.0f); // -X 벽
		object.scale = glm::vec3(4.0f);
		m_scene.objects.push_back(object);
	}

	// // 천장 조명
	// {
	// 	AreaLight areaLight;
	// 	areaLight.color = glm::vec3(1.0f);
	// 	areaLight.intensity = 30.0f;
	// 	areaLight.position = glm::vec3(0.0f, 2.9f, 0.0f);
	// 	areaLight.rotation = glm::vec3(90.0f, 0.0f, 0.0f); // 아래 방향 (법선 -Y → +Z)
	// 	areaLight.scale = glm::vec3(0.5f);
	// 	areaLight.useTemperature = false;
	// 	areaLight.temperature = 0.0f;
	// 	m_scene.areaLights.push_back(areaLight);
	// }

	// 바닥 조명
	{
		AreaLight areaLight;
		areaLight.color = glm::vec3(1.0f, 0.85f, 0.7f);;
		areaLight.intensity = 10.0f;
		areaLight.position = glm::vec3(0.0f, -0.9f, 0.0f);
		areaLight.rotation = glm::vec3(-90.0f, 0.0f, 0.0f); // 아래 방향 (법선 -Y → +Z)
		areaLight.scale = glm::vec3(1.0f);
		areaLight.useTemperature = false;
		areaLight.temperature = 0.0f;
		m_scene.areaLights.push_back(areaLight);
	}

	// {
	// 	Object object;
	// 	object.modelIndex = 7;
	// 	object.overrideMaterialIndex = -1;
	// 	object.position = glm::vec3(0.0f, -1.0f, 0.0f);
	// 	object.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	// 	object.scale = glm::vec3(0.5f, 0.5f, 0.5f);
	// 	m_scene.objects.push_back(object);
	// }

	{
		Object object;
		object.modelIndex = 4;
		object.overrideMaterialIndex = -1;
		object.position = glm::vec3(0.0f, 0.0f, 0.0f);
		object.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		object.scale = glm::vec3(3.0f, 3.0f, 3.0f);
		m_scene.objects.push_back(object);
	}

	// {
	// 	Object object;
	// 	object.modelIndex = 6;
	// 	object.overrideMaterialIndex = -1;
	// 	object.position = glm::vec3(1.0f, 0.0f, 0.0f);
	// 	object.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	// 	object.scale = glm::vec3(0.01f, 0.01f, 0.01f);
	// 	// object.scale = glm::vec3(1.0f, 1.0f, 1.0f);
	// 	m_scene.objects.push_back(object);
	// }

	// {
	// 	Object object;
	// 	object.modelIndex = 3;
	// 	object.overrideMaterialIndex = -1;
	// 	object.position = glm::vec3(-1.0f, 0.0f, 0.0f);
	// 	object.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	// 	object.scale = glm::vec3(1.0f, 1.0f, 1.0f);
	// 	m_scene.objects.push_back(object);
	// }


}

void Renderer::init(GLFWwindow* window) {
	std::cout << "Renderer::init" << std::endl;
	this->window = window;
	m_context = VulkanContext::createVulkanContext(window);
	m_swapChain = SwapChain::createSwapChain(window, m_context.get());
	m_syncObjects = SyncObjects::createSyncObjects(m_context.get());
	m_commandBuffers = CommandBuffers::createCommandBuffers(m_context.get());
	m_extent = {1280, 720};

	updateAssets();
	createScene();
	uploadSceneToGPU();

	// printAllModelInfo();
	// printAllInstanceInfo();
	// printAllAreaLightInfo();

	//output texture
	m_outputTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_accum0Texture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_accum1Texture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);


	// descriptorset layout
	m_set0Layout = DescriptorSetLayout::createSet0Layout(m_context.get()); // camera, options
	m_set1Layout = DescriptorSetLayout::createSet1Layout(m_context.get()); // material
	m_set2Layout = DescriptorSetLayout::createSet2Layout(m_context.get()); // texture
	m_set3Layout = DescriptorSetLayout::createSet3Layout(m_context.get()); // instance, arealight
	m_set4Layout = DescriptorSetLayout::createSet4Layout(m_context.get()); // tlas
	m_set5Layout = DescriptorSetLayout::createSet5Layout(m_context.get()); // 

	// buffers
	m_cameraBuffer = UniformBuffer::createUniformBuffer(m_context.get(), sizeof(CameraGPU));
	m_optionsBuffer = UniformBuffer::createUniformBuffer(m_context.get(), sizeof(OptionsGPU));
	m_materialBuffer = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(MaterialGPU), MAX_MATERIAL_COUNT);
	m_instanceBuffer = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(InstanceGPU), MAX_OBJECT_COUNT);
	m_areaLightBuffer = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(AreaLightGPU), MAX_LIGHT_COUNT);

	// acceleration structure
	m_blas.resize(m_meshes.size());
	for (int i = 0; i < m_meshes.size(); i++) {
		m_blas[i] = BottomLevelAS::createBottomLevelAS(m_context.get(), m_meshes[i].get());
	}

	m_tlas = TopLevelAS::createTopLevelAS(m_context.get(), m_blas, m_instanceGPU);

	// pipeline
	m_ptPipeline = RayTracingPipeline::createPtPipeline(m_context.get(), {m_set0Layout.get(), m_set1Layout.get(), m_set2Layout.get(), m_set3Layout.get(), m_set4Layout.get(), m_set5Layout.get()});


	// descriptor set
	m_set0DescSet = DescriptorSet::createSet0DescSet(m_context.get(), m_set0Layout.get(), m_cameraBuffer.get(), m_optionsBuffer.get());
	m_set1DescSet = DescriptorSet::createSet1DescSet(m_context.get(), m_set1Layout.get(), m_materialBuffer.get());
	m_set2DescSet = DescriptorSet::createSet2DescSet(m_context.get(), m_set2Layout.get(), m_textures);
	m_set3DescSet = DescriptorSet::createSet3DescSet(m_context.get(), m_set3Layout.get(), m_instanceBuffer.get(), m_areaLightBuffer.get());
	m_set4DescSet = DescriptorSet::createSet4DescSet(m_context.get(), m_set4Layout.get(), m_tlas->getHandle());
	m_set5DescSets[0] = DescriptorSet::createSet5DescSet(m_context.get(), m_set5Layout.get(), m_outputTexture.get(), m_accum0Texture.get(), m_accum1Texture.get());
	m_set5DescSets[1] = DescriptorSet::createSet5DescSet(m_context.get(), m_set5Layout.get(), m_outputTexture.get(), m_accum1Texture.get(), m_accum0Texture.get());

	// update buffers
	m_cameraBuffer->updateUniformBuffer(&m_camera, sizeof(CameraGPU));
	m_optionsBuffer->updateUniformBuffer(&m_options, sizeof(OptionsGPU));
	m_materialBuffer->updateStorageBuffer(&m_materials[0], sizeof(MaterialGPU) * m_materials.size());
	m_instanceBuffer->updateStorageBuffer(&m_instanceGPU[0], sizeof(InstanceGPU) * m_instanceGPU.size());
	m_areaLightBuffer->updateStorageBuffer(&m_areaLightGPU[0], sizeof(AreaLightGPU) * m_areaLightGPU.size());

	// gui
	m_imguiRenderPass = RenderPass::createImGuiRenderPass(m_context.get(), m_swapChain.get());
	m_imguiFrameBuffers.resize(m_swapChain->getSwapChainImages().size());
	for (int i = 0; i < m_swapChain->getSwapChainImages().size(); i++) {
		m_imguiFrameBuffers[i] = FrameBuffer::createImGuiFrameBuffer(m_context.get(), m_imguiRenderPass.get(), m_swapChain->getSwapChainImageViews()[i], m_swapChain->getSwapChainExtent());
	}
	m_guiRenderer = GuiRenderer::createGuiRenderer(m_context.get(), window, m_imguiRenderPass.get(), m_swapChain.get());
	m_guiRenderer->createViewPortDescriptorSet({m_outputTexture.get(), m_outputTexture.get()});
	m_guiRenderer->updateModel(m_models);


	// transfer image layout
	auto cmd = VulkanUtil::beginSingleTimeCommands(m_context.get());

	transferImageLayout(cmd, m_outputTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_accum0Texture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_accum1Texture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

	VulkanUtil::endSingleTimeCommands(m_context.get(), cmd);


}

void Renderer::update(float deltaTime) {
    bool rightPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

	CameraGPU prevCamera = m_camera;

    if (rightPressed && !m_mousePressed) {
        // 처음 눌렸을 때 마우스 위치 저장
        glfwGetCursorPos(window, &m_lastMouseX, &m_lastMouseY);
    }

    if (rightPressed) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

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
        m_camera.camDir = glm::normalize(direction);

        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        m_camera.camRight = glm::normalize(glm::cross(m_camera.camDir, worldUp));
        m_camera.camUp = glm::normalize(glm::cross(m_camera.camRight, m_camera.camDir));
    }

    m_mousePressed = rightPressed;

    glm::vec3 move = glm::vec3(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += m_camera.camDir;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= m_camera.camDir;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += m_camera.camRight;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= m_camera.camRight;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) move -= m_camera.camUp;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) move += m_camera.camUp;

    if (glm::length(move) > 0.0f) {
        move = glm::normalize(move);
        m_camera.camPos += move * m_moveSpeed * deltaTime;
    }

	if (prevCamera.camPos != m_camera.camPos || prevCamera.camDir != m_camera.camDir) {
		m_options.currentSpp = -1;
	}
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

	VkCommandBuffer cmd = m_commandBuffers->getCommandBuffers()[currentFrame];
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	// start record

	if (m_scene.isDirty)  {
		std::cout << "scene is dirty!" << std::endl;
		vkDeviceWaitIdle(m_context->getDevice());
		uploadSceneToGPU();
		m_instanceBuffer->updateStorageBuffer(&m_instanceGPU[0], sizeof(InstanceGPU) * m_instanceGPU.size());

		m_tlas->recreate(m_blas, m_instanceGPU);
		m_set4DescSet.reset();
		m_set4DescSet = DescriptorSet::createSet4DescSet(m_context.get(), m_set4Layout.get(), m_tlas->getHandle());
		m_scene.isDirty = false;
		m_options.currentSpp = -1;
	}

	m_cameraBuffer->updateUniformBuffer(&m_camera, sizeof(CameraGPU));
	
	m_options.frameCount++;
	m_options.currentSpp++;

	if (m_options.currentSpp >= m_options.maxSpp) {
		m_options.currentSpp = m_options.maxSpp;
	}
	m_optionsBuffer->updateUniformBuffer(&m_options, sizeof(OptionsGPU));
	m_areaLightBuffer->updateStorageBuffer(&m_areaLightGPU[0], sizeof(AreaLightGPU) * m_areaLightGPU.size());


	recordPathTracingCommandBuffer();
	transferImageLayout(cmd, m_outputTexture.get(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	recordImGuiCommandBuffer(imageIndex, deltaTime);

	transferImageLayout(cmd, m_outputTexture.get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);


	if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
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
	submitInfo.pCommandBuffers = &cmd;

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

	m_options.currentSpp = -1;
	if (newExtent.x <= 0 || newExtent.y <= 0) {
		return;
	}

	m_extent.width = static_cast<uint32_t>(newExtent.x);
	m_extent.height = static_cast<uint32_t>(newExtent.y);

	// clear textures
	m_outputTexture.reset();
	m_accum0Texture.reset();
	m_accum1Texture.reset();

	// clear descriptor set
	m_set5DescSets[0].reset();
	m_set5DescSets[1].reset();

	// create textures
	m_outputTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_accum0Texture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_accum1Texture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	// create descriptor set
	m_set5DescSets[0] = DescriptorSet::createSet5DescSet(m_context.get(), m_set5Layout.get(), m_outputTexture.get(), m_accum0Texture.get(), m_accum1Texture.get());
	m_set5DescSets[1] = DescriptorSet::createSet5DescSet(m_context.get(), m_set5Layout.get(), m_outputTexture.get(), m_accum1Texture.get(), m_accum0Texture.get());

	// gui
	m_guiRenderer->createViewPortDescriptorSet({m_outputTexture.get(), m_outputTexture.get()});

	auto cmd = VulkanUtil::beginSingleTimeCommands(m_context.get());

	transferImageLayout(cmd, m_outputTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_accum0Texture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_accum1Texture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

	VulkanUtil::endSingleTimeCommands(m_context.get(), cmd);
}

void Renderer::recordPathTracingCommandBuffer() {
	VkCommandBuffer cmd = m_commandBuffers->getCommandBuffers()[currentFrame];

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_ptPipeline->getPipeline());

	VkDescriptorSet sets[] = {
		m_set0DescSet->getDescriptorSet(),
		m_set1DescSet->getDescriptorSet(),
		m_set2DescSet->getDescriptorSet(),
		m_set3DescSet->getDescriptorSet(),
		m_set4DescSet->getDescriptorSet(),
		m_set5DescSets[m_options.frameCount % 2]->getDescriptorSet()
	};

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		m_ptPipeline->getPipelineLayout(), 0, 6, sets, 0, nullptr);

	VkStridedDeviceAddressRegionKHR emptyRegion{};
	g_vkCmdTraceRaysKHR(
		cmd,
		&m_ptPipeline->getRaygenRegion(),
		&m_ptPipeline->getMissRegion(),
		&m_ptPipeline->getHitRegion(),
		&emptyRegion,
		m_extent.width,
		m_extent.height,
		1);
}


void Renderer::recordImGuiCommandBuffer(uint32_t imageIndex, float deltaTime) {
	VkCommandBuffer cmd = m_commandBuffers->getCommandBuffers()[currentFrame];
	
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

	vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_guiRenderer->newFrame();
	m_guiRenderer->render(cmd, m_options, m_scene, deltaTime);
	vkCmdEndRenderPass(cmd);
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



void Renderer::uploadSceneToGPU() {
	m_instanceGPU.clear();
	m_areaLightGPU.clear();

	for (auto& object : m_scene.objects) {
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, object.position);
		transform = glm::rotate(transform, glm::radians(object.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		transform = glm::rotate(transform, glm::radians(object.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		transform = glm::rotate(transform, glm::radians(object.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::scale(transform, object.scale);

		for (int i = 0; i < m_models[object.modelIndex].mesh.size(); i++) {
			InstanceGPU instance;
			instance.transform = transform;
			instance.meshIndex = m_models[object.modelIndex].mesh[i];
			instance.vertexAddress = m_meshes[instance.meshIndex]->getVertexBuffer()->getDeviceAddress();
			instance.indexAddress = m_meshes[instance.meshIndex]->getIndexBuffer()->getDeviceAddress();
			if (object.overrideMaterialIndex != -1) {
				instance.materialIndex = m_models[object.overrideMaterialIndex].material[0];
			}
			else {
				instance.materialIndex = m_models[object.modelIndex].material[i];
			}
			m_instanceGPU.push_back(instance);
		}
	}

	for (auto& areaLight : m_scene.areaLights) {
		AreaLightGPU areaLightGPU;
		areaLightGPU.color = areaLight.color;
		areaLightGPU.intensity = areaLight.intensity;

		areaLightGPU.area = areaLight.scale.x * areaLight.scale.y;

		InstanceGPU instance;
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, areaLight.position);
		transform = glm::rotate(transform, glm::radians(areaLight.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		transform = glm::rotate(transform, glm::radians(areaLight.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		transform = glm::rotate(transform, glm::radians(areaLight.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::scale(transform, areaLight.scale);

		auto toWorld = [&](const glm::vec3& p) {
			return glm::vec3(transform * glm::vec4(p, 1.0f));
		};

		areaLightGPU.p0 = toWorld(glm::vec3(-0.5f, -0.5f, 0.0f));
		areaLightGPU.p1 = toWorld(glm::vec3( 0.5f, -0.5f, 0.0f));
		areaLightGPU.p2 = toWorld(glm::vec3( 0.5f,  0.5f, 0.0f));
		areaLightGPU.p3 = toWorld(glm::vec3(-0.5f,  0.5f, 0.0f));
		
		instance.transform = transform;

		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
		areaLightGPU.normal = glm::normalize(normalMatrix * glm::vec3(0.0f, 0.0f, 1.0f));


		instance.meshIndex = m_models[0].mesh[0];
		instance.vertexAddress = m_meshes[instance.meshIndex]->getVertexBuffer()->getDeviceAddress();
		instance.indexAddress = m_meshes[instance.meshIndex]->getIndexBuffer()->getDeviceAddress();
		instance.materialIndex = -1;
		m_areaLightGPU.push_back(areaLightGPU);
		instance.lightIndex = m_areaLightGPU.size() - 1;


		m_instanceGPU.push_back(instance);
	}

	m_options.lightCount = m_scene.areaLights.size();
}
