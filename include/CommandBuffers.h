#pragma once

#include "Common.h"
#include "VulkanContext.h"

class CommandBuffers {
public:
	static std::unique_ptr<CommandBuffers> createCommandBuffers(VulkanContext* context);
	~CommandBuffers();

	std::vector<VkCommandBuffer>& getCommandBuffers() { return m_commandBuffers; }

private:
	VulkanContext* context;
	std::vector<VkCommandBuffer> m_commandBuffers;


	void init(VulkanContext* context);
	void cleanup();
};