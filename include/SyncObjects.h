#pragma once

#include "Common.h"
#include "VulkanContext.h"

class SyncObjects {
public:
	static std::unique_ptr<SyncObjects> createSyncObjects(VulkanContext* context);
	~SyncObjects();

	std::vector<VkSemaphore>& getImageAvailableSemaphores() { return m_imageAvailableSemaphores; }
	std::vector<VkSemaphore>& getRenderFinishedSemaphores() { return m_renderFinishedSemaphores; }
	std::vector<VkFence>& getInFlightFences() { return m_inFlightFences; }

private:
	VulkanContext* context;
	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;

	void cleanup();
	void init(VulkanContext* context);
};
