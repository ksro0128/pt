#include "include/Buffer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
					  VkDeviceMemory &bufferMemory) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; 
	if (vkCreateBuffer(context->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(context->getDevice(), buffer, &memRequirements);

	VkMemoryAllocateFlagsInfo allocFlagsInfo{};
	allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = VulkanUtil::findMemoryType(context, memRequirements.memoryTypeBits, properties);
	allocInfo.pNext = &allocFlagsInfo;

	if (vkAllocateMemory(context->getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(context->getDevice(), buffer, bufferMemory, 0);
}

void Buffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkCommandBuffer commandBuffer = VulkanUtil::beginSingleTimeCommands(context);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	VulkanUtil::endSingleTimeCommands(context, commandBuffer);	
}

VkDeviceAddress Buffer::getDeviceAddress() {
	VkBufferDeviceAddressInfo addressInfo{};
	addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	addressInfo.buffer = m_buffer;
	return vkGetBufferDeviceAddress(context->getDevice(), &addressInfo);
}

// vertex buffer
std::unique_ptr<VertexBuffer> VertexBuffer::createVertexBuffer(VulkanContext* context, std::vector<Vertex> &vertices) {
	std::unique_ptr<VertexBuffer> buffer = std::unique_ptr<VertexBuffer>(new VertexBuffer());
	buffer->init(context, vertices);
	return buffer;
}

void VertexBuffer::init(VulkanContext* context, std::vector<Vertex> &vertices) {
	this->context = context;
	m_vertexCount = static_cast<uint32_t>(vertices.size());
	
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
				 stagingBufferMemory);

	void *data;
	vkMapMemory(context->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);

	vkUnmapMemory(context->getDevice(), stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_buffer, m_bufferMemory);
	copyBuffer(stagingBuffer, m_buffer, bufferSize);

	vkDestroyBuffer(context->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(context->getDevice(), stagingBufferMemory, nullptr);
}

void VertexBuffer::bind(VkCommandBuffer commandBuffer) {
	VkBuffer buffers[] = {m_buffer};
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

VertexBuffer::~VertexBuffer() {
	cleanup();
}

void VertexBuffer::cleanup() {
	vkDestroyBuffer(context->getDevice(), m_buffer, nullptr);
	vkFreeMemory(context->getDevice(), m_bufferMemory, nullptr);
}


std::unique_ptr<IndexBuffer> IndexBuffer::createIndexBuffer(VulkanContext* context, std::vector<uint32_t>& indices) {
	std::unique_ptr<IndexBuffer> indexBuffer = std::unique_ptr<IndexBuffer>(new IndexBuffer());
	indexBuffer->init(context, indices);
	return indexBuffer;
}

void IndexBuffer::init(VulkanContext* context, std::vector<uint32_t>& indices) {
	this->context = context;

	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
	m_indexCount = static_cast<uint32_t>(indices.size());

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(context->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(context->getDevice(), stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_buffer, m_bufferMemory);
	copyBuffer(stagingBuffer, m_buffer, bufferSize);

	vkDestroyBuffer(context->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(context->getDevice(), stagingBufferMemory, nullptr);
}

IndexBuffer::~IndexBuffer() {
	cleanup();
}

void IndexBuffer::cleanup() {
	vkDestroyBuffer(context->getDevice(), m_buffer, nullptr);
	vkFreeMemory(context->getDevice(), m_bufferMemory, nullptr);
}


std::unique_ptr<ImageBuffer> ImageBuffer::createImageBuffer(VulkanContext* context, std::string path, VkFormat format) {
	std::unique_ptr<ImageBuffer> imageBuffer = std::unique_ptr<ImageBuffer>(new ImageBuffer());
	if (!(imageBuffer->init(context, path, format)))
		return nullptr;
	return imageBuffer;
}

bool ImageBuffer::init(VulkanContext* context, std::string path, VkFormat format) {
	this->context = context;

	int texWidth, texHeight, texChannels;
	std::cout << "load: " << path << std::endl;
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	std::cout << "done" << std::endl;
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		std::cout << "ImageBuffer::init() fail to image load! path : " << path << std::endl;
		return false;
	}

	m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(context->getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(context->getDevice(), stagingBufferMemory);

	stbi_image_free(pixels);
	VulkanUtil::createImage(context, texWidth, texHeight, m_mipLevels,
		VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_textureImageMemory);

	transitionImageLayout(m_image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mipLevels);
	copyBufferToImage(stagingBuffer, m_image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	vkDestroyBuffer(context->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(context->getDevice(), stagingBufferMemory, nullptr);

	generateMipmaps(m_image, format, texWidth, texHeight, m_mipLevels);
	std::cout << "create image" << std::endl;
	return true;
}

void IndexBuffer::bind(VkCommandBuffer commandBuffer) {
	vkCmdBindIndexBuffer(commandBuffer, m_buffer, 0, VK_INDEX_TYPE_UINT32);
}

void ImageBuffer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
	VkCommandBuffer commandBuffer = VulkanUtil::beginSingleTimeCommands(context);

	// ������ ������ ���� ����ü
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;										// src �ܰ������ �̹��� ���̾ƿ�
	barrier.newLayout = newLayout;										// src �ܰ� ���� �����ų ���ο� �̹��� ���̾ƿ�
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;				// ������ ��ȯ ���� �� ���ҽ� �������� �Ѱ��� src ť �йи� (����� ���� ť �йи����� ����)
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;				// ���ҽ� �������� ���� dst ť �йи��� dst ť�йи����� ť ��ü�� ����ȭ�� ���� (����� ���� ť �йи����� ����)
	barrier.image = image;												// �踮�� ������ �̹��� ��ü
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// ��ȯ �۾��� ���� ����� color bit ���� ����
	barrier.subresourceRange.baseMipLevel = 0;							// ��ȯ �۾��� ������ miplevel
	barrier.subresourceRange.levelCount = mipLevels;					// ��ȯ �۾��� ������ miplevel�� ����
	barrier.subresourceRange.baseArrayLayer = 0;						// ��ȯ �۾��� ������ ���̾� �ε���
	barrier.subresourceRange.layerCount = 1;							// ��ȯ �۾��� ������ ���̾� ����

	VkPipelineStageFlags sourceStage;									// ������������ sourceStage �ܰ谡 ������ �踮�� ��ȯ ���� 
	VkPipelineStageFlags destinationStage;								// destinationStage �ܰ�� �踮�� ��ȯ�� ���������� ���

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		// �̹��� ���� ���� �̹��� ���̾ƿ�, ���� ���� ����
		barrier.srcAccessMask = 0;									// ���� ���� x
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;		// ���� ���� �ʿ�

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;			// Vulkan�� ���������ο��� ���� ��ܿ� ��ġ�� ù ��° �ܰ��, ��� �۾��� ������� ���� ����
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;			// ������ ���� �ܰ�
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		// �̹��� ���簡 �Ϸ�ǰ� �б⸦ �����ϱ� ���� Fragment shader �۾� ���
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;		// ���� ���� �ʿ�
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;			// �б� ���� �ʿ�

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;				// ������ ���� �ܰ�
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;	// Fragment shader �ܰ�
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	// ����� Ŀ�ǵ� ���ۿ� ���
	vkCmdPipelineBarrier(
		commandBuffer,						// ����� ����� Ŀ�ǵ� ����
		sourceStage, destinationStage,		// sourceStage �ܰ谡 ������ ������ �۾� ����, ������ �۾��� ������ ���� destinationStage�� ������ �ٸ� �۾��� ��� ���
		0,									// ������ �÷���
		0, nullptr,							// �޸� ������ (���� + ������ ������)
		0, nullptr,							// ���� ������   (���� + ������ ������)
		1, &barrier							// �̹��� ������ (���� + ������ ������)
	);

	VulkanUtil::endSingleTimeCommands(context, commandBuffer);
}

// Ŀ�ǵ� ���� ������ ���� ���� -> �̹��� ������ ���� 
void ImageBuffer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	// Ŀ�ǵ� ���� ���� �� ��� ����
	VkCommandBuffer commandBuffer = VulkanUtil::beginSingleTimeCommands(context);

	// ���� -> �̹��� ���縦 ���� ����
	VkBufferImageCopy region{};
	region.bufferOffset = 0;											// ������ ������ ���� ��ġ offset
	region.bufferRowLength = 0;											// ����� ������ row �� �ȼ� �� (0���� �ϸ� �̹��� �ʺ� �ڵ����� ��������.)
	region.bufferImageHeight = 0;										// ����� ������ col �� �ȼ� �� (0���� �ϸ� �̹��� ���̿� �ڵ����� ��������.)
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;		// �̹����� ������ Ÿ�� (����� �÷����� ����)
	region.imageSubresource.mipLevel = 0;								// �̹����� miplevel ����
	region.imageSubresource.baseArrayLayer = 0;							// �̹����� ���� layer ���� (cubemap�� ���� ��� ���� ���̾� ����)
	region.imageSubresource.layerCount = 1;								// �̹��� layer ����
	region.imageOffset = { 0, 0, 0 };										// �̹����� ������ ���� ��ġ
	region.imageExtent = {												// �̹����� ������ �ʺ�, ����, ����
		width,
		height,
		1
	};

	// Ŀ�ǵ� ���ۿ� ���� -> �̹����� ������ �����ϴ� ���� ���
	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	// Ŀ�ǵ� ���� ��� ���� �� ����
	VulkanUtil::endSingleTimeCommands(context, commandBuffer);
}

void ImageBuffer::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
	// �̹��� ������ ���� ���͸��� ����� Blit �۾��� �����ϴ��� Ȯ��
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(context->getPhysicalDevice(), imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	// Ŀ�ǵ� ���� ���� �� ��� ����
	VkCommandBuffer commandBuffer = VulkanUtil::beginSingleTimeCommands(context);

	// ������ ����
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	// miplevel 0 ~ mipLevels
	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;								// �ش� mipmap�� ���� barrier ����
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;					// ������ ���⿡ ������ ���̾ƿ�
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;					// ������ �б⿡ ������ ���̾ƿ�
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;						// ���� ���� on
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;						// �б� ���� on

		// ���������� ������ ���� (GPU Ư�� �۾����� ����ȭ ����)
		// ���� �ܰ��� mipmap ���簡 ������, ���� �ܰ� mipmap ���簡 ���۵ǰ� ������ ����
		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		// blit �۾��� �ҽ��� ����� ���� ������ �����ϴ� ����ü
		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };														// �ҽ��� ���� ����
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;																// �ҽ��� mipLevel ����
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };		// ����� ���� ����
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;																	// ����� mipLevel ����
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		// �ҽ� miplevel �� ��� miplevel�� ������ �°� ����
		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR); // ���������� ����

		// shader �ܰ迡�� ����ϱ� ���� Bllit �ܰ谡 �����⸦ ��ٸ�
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	// ������ �ܰ� miplevel ó��
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	VulkanUtil::endSingleTimeCommands(context, commandBuffer);
}

ImageBuffer::~ImageBuffer() {
	cleanup();
}

void ImageBuffer::cleanup() {
	// std::cout << "ImageBuffer::cleanup" << std::endl;
	vkDestroyImage(context->getDevice(), m_image, nullptr);
	vkFreeMemory(context->getDevice(), m_textureImageMemory, nullptr);
}

std::unique_ptr<ImageBuffer> ImageBuffer::createHDRImageBuffer(VulkanContext* context, std::string path)
{
	std::unique_ptr<ImageBuffer> imageBuffer = std::unique_ptr<ImageBuffer>(new ImageBuffer());
	if (!imageBuffer->initHDR(context, path))
		return nullptr;
	return imageBuffer;
}

bool ImageBuffer::initHDR(VulkanContext* context, std::string path) {
	this->context = context;

	int texWidth, texHeight, texChannels;
	float* pixels = stbi_loadf(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels)
		return false;
	VkDeviceSize imageSize = texWidth * texHeight * 4 * sizeof(float);

	m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
		stagingBufferMemory);

	void* data;
	vkMapMemory(context->getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(context->getDevice(), stagingBufferMemory);

	stbi_image_free(pixels);
	VulkanUtil::createImage(context,
		texWidth, texHeight, m_mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_textureImageMemory);

	transitionImageLayout(m_image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mipLevels);
	copyBufferToImage(stagingBuffer, m_image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	vkDestroyBuffer(context->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(context->getDevice(), stagingBufferMemory, nullptr);

	generateMipmaps(m_image, VK_FORMAT_R32G32B32A32_SFLOAT, texWidth, texHeight, m_mipLevels);
	return true;
}

std::unique_ptr<ImageBuffer> ImageBuffer::createDefaultImageBuffer(VulkanContext* context, glm::vec4 color)
{
	std::unique_ptr<ImageBuffer> imageBuffer = std::unique_ptr<ImageBuffer>(new ImageBuffer());
	imageBuffer->initDefault(context, color);
	return imageBuffer;
}


void ImageBuffer::initDefault(VulkanContext* context, glm::vec4 color)
{
	this->context = context;

	// 1. ������¡ ���� ���� �� ������ ����
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkDeviceSize bufferSize = 4; // RGBA 1�ȼ�

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
		stagingBufferMemory);

	void* data;
	vkMapMemory(context->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	uint8_t pixel[4] = { static_cast<uint8_t>(color.r * 255), static_cast<uint8_t>(color.g * 255),
						static_cast<uint8_t>(color.b * 255), static_cast<uint8_t>(color.a * 255) };
	memcpy(data, pixel, static_cast<size_t>(bufferSize));
	vkUnmapMemory(context->getDevice(), stagingBufferMemory);

	// 2. VulkanUtil�� ����Ͽ� Default Image ����
	m_mipLevels = 1; // Default Texture�� mipmap�� �ʿ� ����

	VulkanUtil::createImage(context, 1, 1, m_mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_textureImageMemory);

	transitionImageLayout(m_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mipLevels);

	copyBufferToImage(stagingBuffer, m_image, 1, 1);

	transitionImageLayout(m_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_mipLevels);

	vkDestroyBuffer(context->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(context->getDevice(), stagingBufferMemory, nullptr);
}

std::unique_ptr<ImageBuffer> ImageBuffer::createAttachmentImageBuffer(VulkanContext* context, uint32_t width,
	uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags) {
	std::unique_ptr<ImageBuffer> imageBuffer = std::unique_ptr<ImageBuffer>(new ImageBuffer());
	imageBuffer->initAttachment(context, width, height, format, usage, aspectFlags);
	return imageBuffer;
}

void ImageBuffer::initAttachment(VulkanContext* context, uint32_t width,
	uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags) {
	this->context = context;
	m_mipLevels = 1;

	VulkanUtil::createImage(context, width, height, m_mipLevels, VK_SAMPLE_COUNT_1_BIT,
		format, VK_IMAGE_TILING_OPTIMAL,
		usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_image, m_textureImageMemory);
}

std::unique_ptr<ImageBuffer> ImageBuffer::createCubeMapImageBuffer(VulkanContext* context, uint32_t width,
	uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags) {
	std::unique_ptr<ImageBuffer> imageBuffer = std::unique_ptr<ImageBuffer>(new ImageBuffer());
	imageBuffer->initCubeMap(context, width, height, format, usage, aspectFlags);
	return imageBuffer;
}

void ImageBuffer::initCubeMap(VulkanContext* context, uint32_t width,
	uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags) {
	this->context = context;
	m_mipLevels = 1;

	VulkanUtil::createImage(context, width, height, m_mipLevels, VK_SAMPLE_COUNT_1_BIT,
		format, VK_IMAGE_TILING_OPTIMAL,
		usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_image, m_textureImageMemory, true);
}

std::unique_ptr<ImageBuffer> ImageBuffer::createImageBufferFromMemory(VulkanContext* context, const aiTexture* aiTexture, VkFormat format) {
	std::unique_ptr<ImageBuffer> imageBuffer = std::unique_ptr<ImageBuffer>(new ImageBuffer());
	if (!(imageBuffer->initFromMemory(context, aiTexture, format)))
		return nullptr;
	return imageBuffer;
}

bool ImageBuffer::initFromMemory(VulkanContext* context, const aiTexture* texture, VkFormat format) {
	this->context = context;

	int texWidth, texHeight, texChannels;
	unsigned char* pixels = nullptr;

	if (texture->mHeight == 0)
	{
		// Compressed texture (e.g., JPEG/PNG)
		pixels =
			stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(texture->pcData), static_cast<int>(texture->mWidth),
				&texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			throw std::runtime_error("Failed to decode compressed texture!");
		}
	}
	else
	{
		// Uncompressed RAW texture
		texWidth = texture->mWidth;
		texHeight = texture->mHeight;
		texChannels = 4; // Assume RGBA for uncompressed textures
		pixels = reinterpret_cast<unsigned char*>(texture->pcData);
	}

	m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	VkDeviceSize imageSize = texWidth * texHeight * 4; // RGBA: 4 bytes per pixel
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
		stagingBufferMemory);

	void* data;
	vkMapMemory(context->getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(context->getDevice(), stagingBufferMemory);

	stbi_image_free(pixels);

	VulkanUtil::createImage(context,
		texWidth, texHeight, m_mipLevels, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_textureImageMemory);

	transitionImageLayout(m_image, format, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mipLevels);
	copyBufferToImage(stagingBuffer, m_image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	vkDestroyBuffer(context->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(context->getDevice(), stagingBufferMemory, nullptr);

	generateMipmaps(m_image, format, texWidth, texHeight, m_mipLevels);
}



std::unique_ptr<UniformBuffer> UniformBuffer::createUniformBuffer(VulkanContext* context, VkDeviceSize buffersize) {
	std::unique_ptr<UniformBuffer> uniformBuffer = std::unique_ptr<UniformBuffer>(new UniformBuffer());
	uniformBuffer->init(context, buffersize);
	return uniformBuffer;
}

void UniformBuffer::init(VulkanContext* context, VkDeviceSize buffersize) {
	this->context = context;

	createBuffer(buffersize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffer, m_bufferMemory);
	vkMapMemory(context->getDevice(), m_bufferMemory, 0, buffersize, 0, &m_mappedMemory);
}

UniformBuffer::~UniformBuffer() {
	cleanup();
}

void UniformBuffer::cleanup() {
	std::cout << "UniformBuffer::cleanup" << std::endl;
	if (m_mappedMemory)
	{
		vkUnmapMemory(context->getDevice(), m_bufferMemory);
		m_mappedMemory = nullptr;
	}
	if (m_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(context->getDevice(), m_buffer, nullptr);
		m_buffer = VK_NULL_HANDLE;
	}
	if (m_bufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(context->getDevice(), m_bufferMemory, nullptr);
		m_bufferMemory = VK_NULL_HANDLE;
	}
}

void UniformBuffer::updateUniformBuffer(void* data, VkDeviceSize size) {
	memcpy(m_mappedMemory, data, size);
}

std::unique_ptr<StorageBuffer> StorageBuffer::createStorageBuffer(VulkanContext* context, VkDeviceSize buffersize) {
	std::unique_ptr<StorageBuffer> storageBuffer = std::unique_ptr<StorageBuffer>(new StorageBuffer());
	storageBuffer->init(context, buffersize);
	return storageBuffer;
}

void StorageBuffer::init(VulkanContext* context, VkDeviceSize buffersize) {
	this->context = context;

	createBuffer(buffersize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffer, m_bufferMemory);
	vkMapMemory(context->getDevice(), m_bufferMemory, 0, buffersize, 0, &m_mappedMemory);
	m_currentSize = buffersize;
}

StorageBuffer::~StorageBuffer() {
	cleanup();
}

void StorageBuffer::cleanup() {
	std::cout << "StorageBuffer::cleanup" << std::endl;
	if (m_mappedMemory)
	{
		vkUnmapMemory(context->getDevice(), m_bufferMemory);
		m_mappedMemory = nullptr;
	}
	if (m_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(context->getDevice(), m_buffer, nullptr);
		m_buffer = VK_NULL_HANDLE;
	}
	if (m_bufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(context->getDevice(), m_bufferMemory, nullptr);
		m_bufferMemory = VK_NULL_HANDLE;
	}
	m_currentSize = 0;
}

void StorageBuffer::updateStorageBuffer(void* data, VkDeviceSize totalSize)
{
	if (m_mappedMemory == nullptr)
	{
		std::cerr << "StorageBuffer: Mapped memory is nullptr!" << std::endl;
		return;
	}
	if (totalSize > m_currentSize)
	{
		std::cerr << "StorageBuffer: Total size is greater than current size!" << std::endl;
		return;
	}
	memcpy(m_mappedMemory, data, totalSize);
}

void StorageBuffer::updateStorageBufferAt(uint32_t index, void* data, VkDeviceSize structSize)
{
	if (m_mappedMemory == nullptr)
	{
		std::cerr << "StorageBuffer: Mapped memory is nullptr!" << std::endl;
		return;
	}
	if ((index + 1) * structSize > m_currentSize)
	{
		std::cerr << "StorageBuffer: Index out of bounds update attempted!" << std::endl;
		return;
	}
	memcpy(static_cast<uint8_t*>(m_mappedMemory) + index * structSize, data, structSize);
}

void StorageBuffer::resizeStorageBuffer(VkDeviceSize size)
{
	if (size > m_currentSize)
	{
		cleanup();
		init(context, size);
	}
}