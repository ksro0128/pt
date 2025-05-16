#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "Buffer.h"
#include "Mesh.h"
#include "VulkanUtil.h"


class AccelerationStructure {
public:
	virtual ~AccelerationStructure();

	VkAccelerationStructureKHR getHandle() const { return m_as; }
	VkDeviceAddress getDeviceAddress() const { return m_deviceAddress; }
	VkBuffer getBuffer() const { return m_buffer; }

protected:
	VulkanContext* context;

	VkAccelerationStructureKHR m_as = VK_NULL_HANDLE;
	VkBuffer m_buffer = VK_NULL_HANDLE;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;
	VkDeviceAddress m_deviceAddress = 0;
	void cleanup();
};

class BottomLevelAS : public AccelerationStructure {
public:
	static std::unique_ptr<BottomLevelAS> createBottomLevelAS(VulkanContext* context, Mesh* mesh);
private:
	void initBLAS(VulkanContext* context, Mesh* mesh);
};

class TopLevelAS : public AccelerationStructure {
public:
	// static std::unique_ptr<TopLevelAS> createTopLevelAS(VulkanContext* context, std::vector<std::unique_ptr<BottomLevelAS>>& blasList,
	// 	std::vector<ShapeGPU>& shapeList);
	// static std::unique_ptr<TopLevelAS> createEmptyTopLevelAS(VulkanContext* context);
	// void rebuild(std::vector<std::unique_ptr<BottomLevelAS>>& blasList,
	// 	std::vector<ShapeGPU>& shapeList);
private:
	// void initTLAS(VulkanContext* context, std::vector<std::unique_ptr<BottomLevelAS>>& blasList,
	// 	std::vector<ShapeGPU>& shapeList);
	// void initEmptyTLAS(VulkanContext* context);
};