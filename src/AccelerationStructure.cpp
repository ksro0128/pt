#include "include/AccelerationStructure.h"

AccelerationStructure::~AccelerationStructure() {
	cleanup();
}

void AccelerationStructure::cleanup() {
	//std::cout << "AccelerationStructure::cleanup" << std::endl;

	if (m_as != VK_NULL_HANDLE) {
		g_vkDestroyAccelerationStructureKHR(context->getDevice(), m_as, nullptr);
		m_as = VK_NULL_HANDLE;
	}

	if (m_buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(context->getDevice(), m_buffer, nullptr);
		m_buffer = VK_NULL_HANDLE;
	}

	if (m_memory != VK_NULL_HANDLE) {
		vkFreeMemory(context->getDevice(), m_memory, nullptr);
		m_memory = VK_NULL_HANDLE;
	}

	m_deviceAddress = 0;
}

std::unique_ptr<BottomLevelAS> BottomLevelAS::createBottomLevelAS(VulkanContext* context, Mesh* mesh) {
	std::unique_ptr<BottomLevelAS> as = std::unique_ptr<BottomLevelAS>(new BottomLevelAS());
	as->initBLAS(context, mesh);
	return as;
}

void BottomLevelAS::initBLAS(VulkanContext* context, Mesh* mesh) {
	this->context = context;
	auto vertexBuffer = mesh->getVertexBuffer();
	auto indexBuffer = mesh->getIndexBuffer();

	VkDeviceAddress vertexAddress = vertexBuffer->getDeviceAddress();
	VkDeviceAddress indexAddress = indexBuffer->getDeviceAddress();
	uint32_t vertexCount = vertexBuffer->getVertexCount();
	uint32_t indexCount = indexBuffer->getIndexCount();

	VkAccelerationStructureGeometryKHR geometry{};
	geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	geometry.geometry.triangles.vertexData.deviceAddress = vertexAddress;
	geometry.geometry.triangles.vertexStride = sizeof(Vertex);
	geometry.geometry.triangles.maxVertex = vertexCount;
	geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
	geometry.geometry.triangles.indexData.deviceAddress = indexAddress;

	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &geometry;
	buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;

	uint32_t primitiveCount = indexCount / 3;

	VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
	sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

	g_vkGetAccelerationStructureBuildSizesKHR(
		context->getDevice(),
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&buildInfo,
		&primitiveCount,
		&sizeInfo
	);

	VulkanUtil::createBuffer(context, sizeInfo.accelerationStructureSize,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_buffer, m_memory);

	VkAccelerationStructureCreateInfoKHR asCreateInfo{};
	asCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	asCreateInfo.buffer = m_buffer;
	asCreateInfo.size = sizeInfo.accelerationStructureSize;
	asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

	if (g_vkCreateAccelerationStructureKHR(context->getDevice(), &asCreateInfo, nullptr, &m_as) != VK_SUCCESS) {
		throw std::runtime_error("failed to create BLAS!");
	}
	buildInfo.dstAccelerationStructure = m_as;

	VkBuffer scratchBuffer;
	VkDeviceMemory scratchMemory;
	VulkanUtil::createBuffer(context, sizeInfo.buildScratchSize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		scratchBuffer, scratchMemory);

	VkBufferDeviceAddressInfo scratchAddrInfo{};
	scratchAddrInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	scratchAddrInfo.buffer = scratchBuffer;

	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(context->getDevice(), &scratchAddrInfo);
	buildInfo.scratchData.deviceAddress = scratchAddress;

	VkAccelerationStructureBuildRangeInfoKHR rangeInfo{};
	rangeInfo.primitiveCount = primitiveCount;
	rangeInfo.primitiveOffset = 0;
	rangeInfo.firstVertex = 0;
	rangeInfo.transformOffset = 0;

	std::array<VkAccelerationStructureBuildRangeInfoKHR*, 1> rangeInfos = { &rangeInfo };

	auto cmd = VulkanUtil::beginSingleTimeCommands(context);
	g_vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildInfo, rangeInfos.data());
	VulkanUtil::endSingleTimeCommands(context, cmd);

	VkAccelerationStructureDeviceAddressInfoKHR addrInfo{};
	addrInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	addrInfo.accelerationStructure = m_as;
	m_deviceAddress = g_vkGetAccelerationStructureDeviceAddressKHR(context->getDevice(), &addrInfo);

	vkDestroyBuffer(context->getDevice(), scratchBuffer, nullptr);
	vkFreeMemory(context->getDevice(), scratchMemory, nullptr);
}

std::unique_ptr<TopLevelAS> TopLevelAS::createTopLevelAS(VulkanContext* context, std::vector<std::unique_ptr<BottomLevelAS>>& blasList,
	std::vector<ShapeGPU>& shapeList) {
	std::unique_ptr<TopLevelAS> as = std::unique_ptr<TopLevelAS>(new TopLevelAS());
	as->initTLAS(context, blasList, shapeList);
	return as;
}

void TopLevelAS::initTLAS(VulkanContext* context, std::vector<std::unique_ptr<BottomLevelAS>>& blasList,
	std::vector<ShapeGPU>& shapeList) {
	this->context = context;

	std::vector<VkAccelerationStructureInstanceKHR> instances;

	for (int i = 0; i < shapeList.size(); i++) {
		VkAccelerationStructureInstanceKHR tlasInstance{};
		// tlasInstance.transform = glmToVkTransform(shapeList[i].modelMatrix);
		tlasInstance.transform = glmToVkTransform(glm::mat4(1.0f));
		tlasInstance.instanceCustomIndex = i;
		tlasInstance.mask = 0xFF;
		tlasInstance.instanceShaderBindingTableRecordOffset = 0;
		tlasInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		tlasInstance.accelerationStructureReference = blasList[i]->getDeviceAddress();
		instances.push_back(tlasInstance);
	}



	VkDeviceSize instanceBufferSize = sizeof(VkAccelerationStructureInstanceKHR) * instances.size();

	VkBuffer instanceBuffer;
	VkDeviceMemory instanceMemory;

	VulkanUtil::createBuffer(
		context,
		instanceBufferSize,
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		instanceBuffer,
		instanceMemory
	);

	void* data;
	vkMapMemory(context->getDevice(), instanceMemory, 0, instanceBufferSize, 0, &data);
	memcpy(data, instances.data(), static_cast<size_t>(instanceBufferSize));
	vkUnmapMemory(context->getDevice(), instanceMemory);

	VkDeviceAddress instanceAddress = VulkanUtil::getDeviceAddress(context, instanceBuffer);

	VkAccelerationStructureGeometryKHR geometry{};
	geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	geometry.geometry.instances.arrayOfPointers = VK_FALSE;
	geometry.geometry.instances.data.deviceAddress = instanceAddress;

	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &geometry;
	buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;

	uint32_t primitiveCount = static_cast<uint32_t>(instances.size());

	VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
	sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

	g_vkGetAccelerationStructureBuildSizesKHR(
		context->getDevice(),
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&buildInfo,
		&primitiveCount,
		&sizeInfo
	);

	VulkanUtil::createBuffer(context,
		sizeInfo.accelerationStructureSize,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_buffer, m_memory);

	VkAccelerationStructureCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	createInfo.buffer = m_buffer;
	createInfo.size = sizeInfo.accelerationStructureSize;
	createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

	if (g_vkCreateAccelerationStructureKHR(context->getDevice(), &createInfo, nullptr, &m_as) != VK_SUCCESS) {
		throw std::runtime_error("failed to create TLAS!");
	}

	buildInfo.dstAccelerationStructure = m_as;

	VkBuffer scratchBuffer;
	VkDeviceMemory scratchMemory;
	VulkanUtil::createBuffer(context,
		sizeInfo.buildScratchSize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		scratchBuffer, scratchMemory);

	VkDeviceAddress scratchAddress = VulkanUtil::getDeviceAddress(context, scratchBuffer);
	buildInfo.scratchData.deviceAddress = scratchAddress;

	VkAccelerationStructureBuildRangeInfoKHR rangeInfo{};
	rangeInfo.primitiveCount = primitiveCount;
	rangeInfo.primitiveOffset = 0;
	rangeInfo.firstVertex = 0;
	rangeInfo.transformOffset = 0;

	const VkAccelerationStructureBuildRangeInfoKHR* rangeInfos[] = { &rangeInfo };

	VkCommandBuffer cmd = VulkanUtil::beginSingleTimeCommands(context);
	g_vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildInfo, rangeInfos);
	VulkanUtil::endSingleTimeCommands(context, cmd);

	VkAccelerationStructureDeviceAddressInfoKHR addrInfo{};
	addrInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	addrInfo.accelerationStructure = m_as;
	m_deviceAddress = g_vkGetAccelerationStructureDeviceAddressKHR(context->getDevice(), &addrInfo);

	vkDestroyBuffer(context->getDevice(), scratchBuffer, nullptr);
	vkFreeMemory(context->getDevice(), scratchMemory, nullptr);
	vkDestroyBuffer(context->getDevice(), instanceBuffer, nullptr);
	vkFreeMemory(context->getDevice(), instanceMemory, nullptr);
}

void TopLevelAS::rebuild(std::vector<std::unique_ptr<BottomLevelAS>>& blasList,
	std::vector<ShapeGPU>& shapeList) {

	cleanup();
	initTLAS(context, blasList, shapeList);
}

std::unique_ptr<TopLevelAS> TopLevelAS::createEmptyTopLevelAS(VulkanContext* context) {
	std::unique_ptr<TopLevelAS> as = std::unique_ptr<TopLevelAS>(new TopLevelAS());
	as->initEmptyTLAS(context);
	return as;
}

void TopLevelAS::initEmptyTLAS(VulkanContext* context) {
	this->context = context;

	// 빈 인스턴스 데이터 설정 (주소는 0)
	VkAccelerationStructureGeometryKHR geometry{};
	geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	geometry.geometry.instances.arrayOfPointers = VK_FALSE;
	geometry.geometry.instances.data.deviceAddress = 0;
	geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

	// 빌드 정보
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &geometry;
	buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;

	uint32_t primitiveCount = 0;

	VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
	sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

	g_vkGetAccelerationStructureBuildSizesKHR(
		context->getDevice(),
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&buildInfo,
		&primitiveCount,
		&sizeInfo
	);

	// 버퍼 생성
	VulkanUtil::createBuffer(
		context,
		sizeInfo.accelerationStructureSize,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_buffer,
		m_memory
	);

	VkAccelerationStructureCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	createInfo.buffer = m_buffer;
	createInfo.size = sizeInfo.accelerationStructureSize;
	createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

	if (g_vkCreateAccelerationStructureKHR(context->getDevice(), &createInfo, nullptr, &m_as) != VK_SUCCESS) {
		throw std::runtime_error("failed to create empty TLAS");
	}

	VkAccelerationStructureDeviceAddressInfoKHR addrInfo{};
	addrInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	addrInfo.accelerationStructure = m_as;

	m_deviceAddress = g_vkGetAccelerationStructureDeviceAddressKHR(context->getDevice(), &addrInfo);
}
