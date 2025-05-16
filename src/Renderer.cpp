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
	m_extent = {1280, 720};


	//output texture
	m_outputPingTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_outputPongTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	m_imguiRenderPass = RenderPass::createImGuiRenderPass(m_context.get(), m_swapChain.get());
	m_imguiFrameBuffers.resize(m_swapChain->getSwapChainImages().size());
	for (int i = 0; i < m_swapChain->getSwapChainImages().size(); i++) {
		m_imguiFrameBuffers[i] = FrameBuffer::createImGuiFrameBuffer(m_context.get(), m_imguiRenderPass.get(), m_swapChain->getSwapChainImageViews()[i], m_swapChain->getSwapChainExtent());
	}
	m_guiRenderer = GuiRenderer::createGuiRenderer(m_context.get(), window, m_imguiRenderPass.get(), m_swapChain.get());
	m_guiRenderer->createViewPortDescriptorSet({m_outputPingTexture.get(), m_outputPongTexture.get()});



	auto cmd = VulkanUtil::beginSingleTimeCommands(m_context.get());

	transferImageLayout(cmd, m_outputPingTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_outputPongTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	VulkanUtil::endSingleTimeCommands(m_context.get(), cmd);


}

void Renderer::update(float deltaTime) {

}

void Renderer::render(float deltaTime) {
	m_frameCount++;

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

	std::cout << "frame count: " << m_frameCount << std::endl;

	transferImageLayout(cmd, m_outputPingTexture.get(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	transferImageLayout(cmd, m_outputPongTexture.get(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	recordImGuiCommandBuffer(imageIndex);

	transferImageLayout(cmd, m_outputPingTexture.get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_outputPongTexture.get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);



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

	if (newExtent.x <= 0 || newExtent.y <= 0) {
		return;
	}

	m_extent.width = static_cast<uint32_t>(newExtent.x);
	m_extent.height = static_cast<uint32_t>(newExtent.y);

	m_outputPingTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_outputPongTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);


	m_guiRenderer->createViewPortDescriptorSet({m_outputPingTexture.get(), m_outputPongTexture.get()});

	auto cmd = VulkanUtil::beginSingleTimeCommands(m_context.get());

	transferImageLayout(cmd, m_outputPingTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_outputPongTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	VulkanUtil::endSingleTimeCommands(m_context.get(), cmd);
}


void Renderer::recordImGuiCommandBuffer(uint32_t imageIndex) {
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
	m_guiRenderer->render(cmd, m_frameCount);
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
