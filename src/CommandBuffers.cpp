#include "include/CommandBuffers.h"


std::unique_ptr<CommandBuffers> CommandBuffers::createCommandBuffers(VulkanContext* context) {
	std::unique_ptr<CommandBuffers> commandBuffers = std::unique_ptr<CommandBuffers>(new CommandBuffers());
	commandBuffers->init(context);
	return commandBuffers;
}

void CommandBuffers::init(VulkanContext* context) {
	this->context = context;

	m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = context->getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();


	if (vkAllocateCommandBuffers(context->getDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

CommandBuffers::~CommandBuffers() {
	cleanup();
}

void CommandBuffers::cleanup() {
	vkFreeCommandBuffers(context->getDevice(), context->getCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
}
