#include "include/SyncObjects.h"

std::unique_ptr<SyncObjects> SyncObjects::createSyncObjects(VulkanContext* context) {
    std::unique_ptr<SyncObjects> syncObjects = std::make_unique<SyncObjects>();
    syncObjects->init(context);
    return syncObjects;
}

void SyncObjects::init(VulkanContext* context) {
    this->context = context;
    m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(context->getDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context->getDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context->getDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

SyncObjects::~SyncObjects() {
    cleanup();
}

void SyncObjects::cleanup() {
    std::cout << "SyncObjects::cleanup()" << std::endl;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(context->getDevice(), m_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(context->getDevice(), m_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(context->getDevice(), m_inFlightFences[i], nullptr);
    }
}
