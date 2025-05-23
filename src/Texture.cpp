#include "include/Texture.h"

std::unique_ptr<Texture> Texture::createTexture(VulkanContext* context, std::string path, TextureFormatType formatType) {
	std::unique_ptr<Texture> texture = std::unique_ptr<Texture>(new Texture());
	texture->init(context, path, formatType);
	return texture;
}

void Texture::init(VulkanContext* context, std::string path, TextureFormatType formatType) {
	this->context = context;

	VkFormat format = (formatType == TextureFormatType::ColorSRGB)
		? VK_FORMAT_R8G8B8A8_SRGB
		: VK_FORMAT_R8G8B8A8_UNORM;

	m_imageBuffer = ImageBuffer::createImageBuffer(context, path, format);
	m_imageView = VulkanUtil::createImageView(context, m_imageBuffer->getImage(), format,
		VK_IMAGE_ASPECT_COLOR_BIT, m_imageBuffer->getMipLevels());
	VkSamplerCreateInfo samplerInfo = createDefaultSamplerInfo();
	if (vkCreateSampler(context->getDevice(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS)
	{
		std::cerr << "failed to create image sampler" << std::endl;
	}
	m_format = format;
}

Texture::~Texture() {
	cleanup();
}

void Texture::cleanup() {
	if (m_sampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(context->getDevice(), m_sampler, nullptr);
		m_sampler = VK_NULL_HANDLE;
	}
	if (m_imageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(context->getDevice(), m_imageView, nullptr);
		m_imageView = VK_NULL_HANDLE;
	}
	if (m_cubeMapImageViews.size() > 0) {
		for (auto& view : m_cubeMapImageViews) {
			vkDestroyImageView(context->getDevice(), view, nullptr);
		}
		m_cubeMapImageViews.clear();
	}
}

VkSamplerCreateInfo Texture::createDefaultSamplerInfo() {
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(context->getPhysicalDevice(), &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(m_imageBuffer->getMipLevels());
	samplerInfo.mipLodBias = 0.0f;
	return samplerInfo;
}

std::unique_ptr<Texture> Texture::createDefaultTexture(VulkanContext* context, glm::vec4 color) {
	std::unique_ptr<Texture> texture = std::unique_ptr<Texture>(new Texture());
	texture->initDefaultTexture(context, color);
	return texture;
}

void Texture::initDefaultTexture(VulkanContext* context, glm::vec4 color) {
	this->context = context;

	m_imageBuffer = ImageBuffer::createDefaultImageBuffer(context, color);

	m_imageView = VulkanUtil::createImageView(
		context,
		m_imageBuffer->getImage(),
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_ASPECT_COLOR_BIT,
		m_imageBuffer->getMipLevels()
	);

	VkSamplerCreateInfo samplerInfo = createDefaultSamplerInfo();
	if (vkCreateSampler(context->getDevice(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create default image sampler!");
	}

	m_format = VK_FORMAT_R8G8B8A8_UNORM;
}

std::unique_ptr<Texture> Texture::createAttachmentTexture(VulkanContext* context, uint32_t width,
	uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags) {
	std::unique_ptr<Texture> texture = std::unique_ptr<Texture>(new Texture());
	texture->initAttachmentTexture(context, width, height, format, usage, aspectFlags);
	return texture;
}


void Texture::initAttachmentTexture(VulkanContext* context, uint32_t width,
	uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags) {
	this->context = context;

	m_imageBuffer = ImageBuffer::createAttachmentImageBuffer(context, width, height, format, usage, aspectFlags);

	m_imageView = VulkanUtil::createImageView(
		context,
		m_imageBuffer->getImage(),
		format,
		aspectFlags,
		m_imageBuffer->getMipLevels()
	);

	VkSamplerCreateInfo samplerInfo = createDefaultSamplerInfo();
	if (vkCreateSampler(context->getDevice(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create attachment image sampler!");
	}
	m_format = format;
}

std::unique_ptr<Texture> Texture::createCubeMapTexture(VulkanContext* context, uint32_t width,
	uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags) {
	std::unique_ptr<Texture> texture = std::unique_ptr<Texture>(new Texture());
	texture->initCubeMapTexture(context, width, height, format, usage, aspectFlags);
	return texture;
}

void Texture::initCubeMapTexture(VulkanContext* context, uint32_t width,
	uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags) {
	this->context = context;
	m_format = format;

	m_imageBuffer = ImageBuffer::createCubeMapImageBuffer(context, width, height, format, usage, aspectFlags);

	m_imageView = VulkanUtil::createImageView(
		context,
		m_imageBuffer->getImage(),
		format,
		aspectFlags,
		m_imageBuffer->getMipLevels(),
		true
	);

	m_cubeMapImageViews.resize(6);
	for (uint32_t face = 0; face < 6; ++face) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_imageBuffer->getImage();
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = face;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(context->getDevice(), &viewInfo, nullptr, &m_cubeMapImageViews[face]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create cubemap face image view!");
		}
	}

	VkSamplerCreateInfo samplerInfo = createDefaultSamplerInfo();
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

	if (vkCreateSampler(context->getDevice(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create cube map image sampler!");
	}
}

std::unique_ptr<Texture> Texture::createTextureFromMemory(VulkanContext* context, const aiTexture* aiTexture, TextureFormatType formatType) {
	std::unique_ptr<Texture> texture = std::unique_ptr<Texture>(new Texture());
	texture->initTextureFromMemory(context, aiTexture, formatType);
	return texture;
}

void Texture::initTextureFromMemory(VulkanContext* context, const aiTexture* aiTexture, TextureFormatType formatType) {
	this->context = context;

	VkFormat format = (formatType == TextureFormatType::ColorSRGB)
		? VK_FORMAT_R8G8B8A8_SRGB
		: VK_FORMAT_R8G8B8A8_UNORM;

	m_imageBuffer = ImageBuffer::createImageBufferFromMemory(context, aiTexture, format);
	m_imageView = VulkanUtil::createImageView(
		context,
		m_imageBuffer->getImage(),
		format,
		VK_IMAGE_ASPECT_COLOR_BIT,
		m_imageBuffer->getMipLevels()
	);
	m_format = format;
	VkSamplerCreateInfo samplerInfo = createDefaultSamplerInfo();
	if (vkCreateSampler(context->getDevice(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image sampler from memory!");
	}
}



std::unique_ptr<Texture> Texture::createTextureFromMemory(VulkanContext* context, const tinygltf::Image& image, TextureFormatType formatType) {
	std::unique_ptr<Texture> texture = std::unique_ptr<Texture>(new Texture());
	texture->initTextureFromMemory(context, image, formatType);
	return texture;
}


void Texture::initTextureFromMemory(VulkanContext* context, const tinygltf::Image& image, TextureFormatType formatType) {
	this->context = context;

	VkFormat format = (formatType == TextureFormatType::ColorSRGB)
		? VK_FORMAT_R8G8B8A8_SRGB
		: VK_FORMAT_R8G8B8A8_UNORM;

	m_imageBuffer = ImageBuffer::createImageBufferFromMemory(context, image, format);
	m_imageView = VulkanUtil::createImageView(
		context,
		m_imageBuffer->getImage(),
		format,
		VK_IMAGE_ASPECT_COLOR_BIT,
		m_imageBuffer->getMipLevels()
	);
	m_format = format;

	VkSamplerCreateInfo samplerInfo = createDefaultSamplerInfo();
	if (vkCreateSampler(context->getDevice(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image sampler from memory!");
	}
}
