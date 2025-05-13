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
	m_extent = {1200, 760};

	loadScene("assets/bathroom/bathroom.pbrt");

	m_blasList.resize(m_meshList.size());
	for (int i = 0; i < m_meshList.size(); i++) {
		m_blasList[i] = BottomLevelAS::createBottomLevelAS(m_context.get(), m_meshList[i].get());
	}
	m_emptyTLAS = TopLevelAS::createEmptyTopLevelAS(m_context.get());

	m_tlas = TopLevelAS::createTopLevelAS(m_context.get(), m_blasList, m_shapeList);
	m_directHistoryTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_directCurrentTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_indirectHistoryTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT); 
	m_indirectCurrentTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	m_directFilteredTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_indirectFilteredTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	m_directM1Texture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_directM2Texture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_indirectM1Texture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_indirectM2Texture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	m_directVarianceTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_indirectVarianceTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	m_directBlurredVarianceTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_indirectBlurredVarianceTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	m_directUpdatedVarianceTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_indirectUpdatedVarianceTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	m_compositeTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT); 

	m_ptTexture0 = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_ptTexture1 = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_outputTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	m_brightTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_blurHTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_blurVTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_bloomTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_aTorusTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferNormalTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferDepthTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferAlbedoTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferMeshIDTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferSampleCountTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferMotionVectorTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferPrevNormalTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferPrevDepthTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferPrevMeshIDTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	// layout
	m_set0Layout = DescriptorSetLayout::createSet0DescLayout(m_context.get()); // camera, options
	m_set1Layout = DescriptorSetLayout::createSet1DescLayout(m_context.get()); // instance buffer
	m_set2Layout = DescriptorSetLayout::createSet2DescLayout(m_context.get()); // material buffer
	m_set3Layout = DescriptorSetLayout::createSet3DescLayout(m_context.get()); // texture buffer
	m_set4Layout = DescriptorSetLayout::createSet4DescLayout(m_context.get()); // tlas, direct history, direct current, indirect history, indirect current, direct m1, direct m2, indirect m1, indirect m2, direct variance, indirect variance
	m_set5Layout = DescriptorSetLayout::createSet5DescLayout(m_context.get()); // exposure buffer
	m_set6Layout = DescriptorSetLayout::createSet6DescLayout(m_context.get()); // bloom texture
	m_set7Layout = DescriptorSetLayout::createSet7DescLayout(m_context.get()); // aTorus texture
	m_set8Layout = DescriptorSetLayout::createSet8DescLayout(m_context.get()); // g buffer - 0: normal, 1: depth, 2: albedo
	m_set9Layout = DescriptorSetLayout::createSet9DescLayout(m_context.get()); // direct, indirect, composite
	m_set10Layout = DescriptorSetLayout::createSet10DescLayout(m_context.get()); // gaussian blur variance input, output

	// buffer
	m_cameraBuffer = UniformBuffer::createUniformBuffer(m_context.get(), sizeof(CameraGPU));
	m_prevCameraBuffer = UniformBuffer::createUniformBuffer(m_context.get(), sizeof(CameraGPU));
	m_optionsBuffer = UniformBuffer::createUniformBuffer(m_context.get(), sizeof(OptionsGPU));
	m_instanceBuffer = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(ShapeGPU), m_shapeList.size());

	m_areaLightBuffer = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(AreaLightGPU), m_areaLightList.size());
	m_materialBuffer = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(MaterialGPU), m_materialList.size());
	m_uberBuffer = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(UberGPU), m_uberList.size());
	m_matteBuffer = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(MatteGPU), m_matteList.size());
	m_metalBuffer = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(MetalGPU), m_metalList.size());
	m_glassBuffer = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(GlassGPU), m_glassList.size());
	m_mirrorBuffer = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(MirrorGPU), m_mirrorList.size());
	m_substrateBuffer = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(SubstrateGPU), m_substrateList.size());
	m_plasticBuffer = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(PlasticGPU), m_plasticList.size());
	m_exposureBuffer = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(ExposureGPU), 1);
	m_areaLightTriangleBuffer = StorageBuffer::createStorageBuffer(m_context.get(), sizeof(AreaLightTriangleGPU), m_areaLightTriangleList.size());

	// 각 리스트 사이즈 확인
	std::cout << "m_materialList.size() = " << m_materialList.size() << std::endl;
	std::cout << "m_uberList.size() = " << m_uberList.size() << std::endl;
	std::cout << "m_matteList.size() = " << m_matteList.size() << std::endl;
	std::cout << "m_metalList.size() = " << m_metalList.size() << std::endl;
	std::cout << "m_glassList.size() = " << m_glassList.size() << std::endl;
	std::cout << "m_mirrorList.size() = " << m_mirrorList.size() << std::endl;
	std::cout << "m_substrateList.size() = " << m_substrateList.size() << std::endl;
	std::cout << "m_plasticList.size() = " << m_plasticList.size() << std::endl;
	std::cout << "m_areaLightList.size() = " << m_areaLightList.size() << std::endl;
	std::cout << "m_shapeList.size() = " << m_shapeList.size() << std::endl;

	updateInitialBuffers();

	// descriptorset
	m_set0DescSet = DescriptorSet::createSet0DescSet(m_context.get(), m_set0Layout.get(), m_cameraBuffer.get(), m_optionsBuffer.get(), m_prevCameraBuffer.get());
	m_set1DescSet = DescriptorSet::createSet1DescSet(m_context.get(), m_set1Layout.get(), m_instanceBuffer.get());

	if (m_textureList.size() == 0) {
		auto defalutTexture = Texture::createDefaultTexture(m_context.get(), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		m_textureList.push_back(std::move(defalutTexture));
		m_textureNameMap["default"] = 0;
	}
	m_set2DescSet = DescriptorSet::createSet2DescSet(m_context.get(), m_set2Layout.get(), 
		{m_areaLightBuffer.get(), 
		m_materialBuffer.get(),
		m_uberBuffer.get(),
		m_matteBuffer.get(),
		m_metalBuffer.get(),
		m_glassBuffer.get(),
		m_mirrorBuffer.get(),
		m_substrateBuffer.get(),
		m_plasticBuffer.get(),
		m_areaLightTriangleBuffer.get()});
	m_set3DescSet = DescriptorSet::createSet3DescSet(m_context.get(), m_set3Layout.get(), m_textureList);
	m_set4DescSet = DescriptorSet::createSet4DescSet(m_context.get(), m_set4Layout.get(), m_tlas->getHandle(), m_directHistoryTexture.get(), m_directCurrentTexture.get(), m_indirectHistoryTexture.get(), m_indirectCurrentTexture.get(),
		m_directM1Texture.get(), m_directM2Texture.get(), m_indirectM1Texture.get(), m_indirectM2Texture.get(), m_directVarianceTexture.get(), m_indirectVarianceTexture.get());
	// m_set5DescSet = DescriptorSet::createSet5DescSet(m_context.get(), m_set5Layout.get(), m_exposureBuffer.get(), m_ptTexture0.get());
	// m_set5DescSet = DescriptorSet::createSet5DescSet(m_context.get(), m_set5Layout.get(), m_exposureBuffer.get(), m_bloomTexture.get());
	m_set5DescSet = DescriptorSet::createSet5DescSet(m_context.get(), m_set5Layout.get(), m_exposureBuffer.get(), m_aTorusTexture.get());
	m_set6DescSet = DescriptorSet::createSet6DescSet(m_context.get(), m_set6Layout.get(), m_ptTexture0.get(), m_brightTexture.get(), m_blurHTexture.get(), m_blurVTexture.get(), m_bloomTexture.get());
	m_set7DescSets[0] = DescriptorSet::createSet7DescSet(m_context.get(), m_set7Layout.get(), m_directCurrentTexture.get(), m_directFilteredTexture.get(), m_directHistoryTexture.get(), m_directBlurredVarianceTexture.get(), m_directUpdatedVarianceTexture.get());
	m_set7DescSets[1] = DescriptorSet::createSet7DescSet(m_context.get(), m_set7Layout.get(), m_directFilteredTexture.get(), m_directCurrentTexture.get(), m_directHistoryTexture.get(), m_directUpdatedVarianceTexture.get(), m_directBlurredVarianceTexture.get());
	m_set7DescSets[2] = DescriptorSet::createSet7DescSet(m_context.get(), m_set7Layout.get(), m_indirectCurrentTexture.get(), m_indirectFilteredTexture.get(), m_indirectHistoryTexture.get(), m_indirectBlurredVarianceTexture.get(), m_indirectUpdatedVarianceTexture.get());
	m_set7DescSets[3] = DescriptorSet::createSet7DescSet(m_context.get(), m_set7Layout.get(), m_indirectFilteredTexture.get(), m_indirectCurrentTexture.get(), m_indirectHistoryTexture.get(), m_indirectUpdatedVarianceTexture.get(), m_indirectBlurredVarianceTexture.get());
	m_set8DescSet = DescriptorSet::createSet8DescSet(m_context.get(), m_set8Layout.get(), m_gBufferNormalTexture.get(), m_gBufferDepthTexture.get(), m_gBufferAlbedoTexture.get(), m_gBufferMeshIDTexture.get(), m_gBufferSampleCountTexture.get(), m_gBufferMotionVectorTexture.get(),
		m_gBufferPrevNormalTexture.get(), m_gBufferPrevDepthTexture.get(), m_gBufferPrevMeshIDTexture.get());
	m_set9DescSet = DescriptorSet::createSet9DescSet(m_context.get(), m_set9Layout.get(), m_directFilteredTexture.get(), m_indirectFilteredTexture.get(), m_compositeTexture.get());
	m_set10DescSets[0] = DescriptorSet::createSet10DescSet(m_context.get(), m_set10Layout.get(), m_directVarianceTexture.get(), m_directBlurredVarianceTexture.get());
	m_set10DescSets[1] = DescriptorSet::createSet10DescSet(m_context.get(), m_set10Layout.get(), m_indirectVarianceTexture.get(), m_indirectBlurredVarianceTexture.get());

	// renderpass
	m_toneMappingRenderPass = RenderPass::createToneMappingRenderPass(m_context.get());
	m_imguiRenderPass = RenderPass::createImGuiRenderPass(m_context.get(), m_swapChain.get());

	// pipeline
	m_ptPipeline = RayTracingPipeline::createPtPipeline(m_context.get(), {m_set0Layout.get(), m_set1Layout.get(), m_set2Layout.get(), m_set3Layout.get(), m_set4Layout.get(), m_set8Layout.get()});
	m_computeExposurePipeline = Pipeline::createComputeExposurePipeline(m_context.get(), {m_set4Layout.get(), m_set5Layout.get()});
	m_toneMappingPipeline = Pipeline::createToneMappingPipeline(m_context.get(), m_toneMappingRenderPass.get(), {m_set5Layout.get()});
	m_thresholdPipeline = Pipeline::createThresholdPipeline(m_context.get(), {m_set6Layout.get()});
	m_blurHPipeline = Pipeline::createBlurHPipeline(m_context.get(), {m_set6Layout.get()});
	m_blurVPipeline = Pipeline::createBlurVPipeline(m_context.get(), {m_set6Layout.get()});
	m_compositeBloomPipeline = Pipeline::createCompositeBloomPipeline(m_context.get(), {m_set6Layout.get()});
	m_aTorusFilterPipeline = Pipeline::createATorusFilterPipeline(m_context.get(), {m_set7Layout.get(), m_set8Layout.get(), m_set0Layout.get()});
	m_compositePipeline = Pipeline::createCompositePipeline(m_context.get(), {m_set8Layout.get(), m_set9Layout.get()});
	m_gaussianBlurPipeline = Pipeline::createGaussianBlurPipeline(m_context.get(), {m_set10Layout.get()});

	auto cmd = VulkanUtil::beginSingleTimeCommands(m_context.get());
	transferImageLayout(cmd, m_ptTexture0.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_ptTexture1.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_brightTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_blurHTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_blurVTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_bloomTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_aTorusTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_gBufferNormalTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_gBufferDepthTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_gBufferAlbedoTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_gBufferMeshIDTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_gBufferSampleCountTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_gBufferMotionVectorTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_gBufferPrevNormalTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_gBufferPrevDepthTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_gBufferPrevMeshIDTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_directHistoryTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_directCurrentTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_indirectHistoryTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_indirectCurrentTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_directM1Texture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_directM2Texture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_indirectM1Texture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_indirectM2Texture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_directVarianceTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_indirectVarianceTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_directFilteredTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_indirectFilteredTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_compositeTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_directBlurredVarianceTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_indirectBlurredVarianceTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_directUpdatedVarianceTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_indirectUpdatedVarianceTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	VulkanUtil::endSingleTimeCommands(m_context.get(), cmd);

	
	m_imguiFrameBuffers.resize(m_swapChain->getSwapChainImages().size());
	for (int i = 0; i < m_swapChain->getSwapChainImages().size(); i++) {
		m_imguiFrameBuffers[i] = FrameBuffer::createImGuiFrameBuffer(m_context.get(), m_imguiRenderPass.get(), m_swapChain->getSwapChainImageViews()[i], m_swapChain->getSwapChainExtent());
	}
	m_guiRenderer = GuiRenderer::createGuiRenderer(m_context.get(), window, m_imguiRenderPass.get(), m_swapChain.get());
	m_guiRenderer->createViewPortDescriptorSet({m_compositeTexture.get(), m_directFilteredTexture.get(), m_indirectFilteredTexture.get()});
	// m_guiRenderer->createViewPortDescriptorSet({m_ptTexture0.get(), m_ptTexture1.get()});
	m_guiRenderer->createGBufferDescriptorSet({m_gBufferNormalTexture.get(), m_gBufferDepthTexture.get(), m_gBufferAlbedoTexture.get()});
	
	m_toneMappingFrameBuffer = FrameBuffer::createToneMappingFrameBuffer(m_context.get(), m_toneMappingRenderPass.get(), m_outputTexture.get(), m_extent);

	printAllResources();
}

glm::mat4 Renderer::toGlm(const minipbrt::Transform& t) {
    glm::mat4 result(1.0f);
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            result[j][i] = t.start[i][j];
    return result;
}

void Renderer::loadScene(std::string scenePath) {
	minipbrt::Loader loader;
	if (loader.load(scenePath.c_str()) == false) {
		std::cout << "Failed to load scene!" << std::endl;
		return;
	}

	m_pbrtScene = loader.take_scene();
	const auto* pbrtCam = static_cast<minipbrt::PerspectiveCamera*>(m_pbrtScene->camera);
	const glm::mat4 camToWorld = toGlm(pbrtCam->cameraToWorld);

	m_camera.camPos = glm::vec3(camToWorld[3]);

	glm::vec3 forward = glm::normalize(glm::vec3(camToWorld[2]));
	glm::vec3 up = glm::normalize(glm::vec3(camToWorld[1]));

	glm::vec3 right = glm::normalize(glm::cross(forward, up));
	up = glm::normalize(glm::cross(right, forward));

	m_camera.camDir = forward;
	m_camera.camUp = up;
	m_camera.camRight = right;

	if (pbrtCam->type() == minipbrt::CameraType::Perspective) {
		m_camera.fovY = static_cast<minipbrt::PerspectiveCamera*>(m_pbrtScene->camera)->fov;
	}
	
	for (auto& mat : m_pbrtScene->materials) {
		MaterialGPU material;
		material.type = static_cast<int>(mat->type());
		m_materialNameMap[mat->name] = m_materialList.size();
		if (mat->type() == minipbrt::MaterialType::Uber) {
			material.index = m_uberList.size();
			m_materialList.push_back(material);

			const auto* uberMat = static_cast<const minipbrt::UberMaterial*>(mat);
			UberGPU uber;
			uber.Kd.r         = uberMat->Kd.value[0];
			uber.Kd.g         = uberMat->Kd.value[1];
			uber.Kd.b         = uberMat->Kd.value[2];
			uber.KdIdx      = static_cast<int>(uberMat->Kd.texture);

			uber.Ks.r         = uberMat->Ks.value[0];
			uber.Ks.g         = uberMat->Ks.value[1];
			uber.Ks.b         = uberMat->Ks.value[2];
			uber.KsIdx      = static_cast<int>(uberMat->Ks.texture);

			uber.Kr.r         = uberMat->Kr.value[0];
			uber.Kr.g         = uberMat->Kr.value[1];
			uber.Kr.b         = uberMat->Kr.value[2];
			uber.KrIdx      = static_cast<int>(uberMat->Kr.texture);

			uber.Kt.r         = uberMat->Kt.value[0];
			uber.Kt.g         = uberMat->Kt.value[1];
			uber.Kt.b         = uberMat->Kt.value[2];
			uber.KtIdx      = static_cast<int>(uberMat->Kt.texture);

			uber.opacity.r    = uberMat->opacity.value[0];
			uber.opacity.g    = uberMat->opacity.value[1];
			uber.opacity.b    = uberMat->opacity.value[2];
			uber.opacityIdx = static_cast<int>(uberMat->opacity.texture);

			uber.eta        = uberMat->eta.value;
			uber.etaIdx     = static_cast<int>(uberMat->eta.texture);

			uber.uroughness    = uberMat->uroughness.value;
			uber.uroughnessIdx = static_cast<int>(uberMat->uroughness.texture);

			uber.vroughness    = uberMat->vroughness.value;
			uber.vroughnessIdx = static_cast<int>(uberMat->vroughness.texture);

			uber.remaproughness = uberMat->remaproughness ? 1 : 0;
			m_uberList.push_back(uber);
		}
		else if (mat->type() == minipbrt::MaterialType::Matte) {
			material.index = m_matteList.size();
			m_materialList.push_back(material);

			const auto* matteMat = static_cast<const minipbrt::MatteMaterial*>(mat);
			MatteGPU matte;
			matte.Kd.r = matteMat->Kd.value[0];
			matte.Kd.g = matteMat->Kd.value[1];
			matte.Kd.b = matteMat->Kd.value[2];
			matte.KdIdx = static_cast<int>(matteMat->Kd.texture);
			matte.sigma = matteMat->sigma.value;
			m_matteList.push_back(matte);
		}
		else if (mat->type() == minipbrt::MaterialType::Metal) {
			material.index = m_metalList.size();
			m_materialList.push_back(material);

			const auto* metalMat = static_cast<const minipbrt::MetalMaterial*>(mat);
			MetalGPU metal;

			metal.eta.r = metalMat->eta.value[0];
			metal.eta.g = metalMat->eta.value[1];
			metal.eta.b = metalMat->eta.value[2];
			metal.etaIdx = static_cast<int>(metalMat->eta.texture);

			metal.k.r = metalMat->k.value[0];
			metal.k.g = metalMat->k.value[1];
			metal.k.b = metalMat->k.value[2];
			metal.kIdx = static_cast<int>(metalMat->k.texture);

			metal.uroughness = metalMat->uroughness.value;
			metal.uroughnessIdx = static_cast<int>(metalMat->uroughness.texture);

			metal.vroughness = metalMat->vroughness.value;
			metal.vroughnessIdx = static_cast<int>(metalMat->vroughness.texture);

			metal.remaproughness = metalMat->remaproughness ? 1 : 0;
			m_metalList.push_back(metal);
		}
		else if (mat->type() == minipbrt::MaterialType::Glass) {
			material.index = m_glassList.size();
			m_materialList.push_back(material);

			const auto* glassMat = static_cast<const minipbrt::GlassMaterial*>(mat);
			GlassGPU glass;
			glass.Kr.r = glassMat->Kr.value[0];
			glass.Kr.g = glassMat->Kr.value[1];
			glass.Kr.b = glassMat->Kr.value[2];
			glass.KrIdx = static_cast<int>(glassMat->Kr.texture);

			glass.Kt.r = glassMat->Kt.value[0];
			glass.Kt.g = glassMat->Kt.value[1];
			glass.Kt.b = glassMat->Kt.value[2];
			glass.KtIdx = static_cast<int>(glassMat->Kt.texture);

			glass.eta = glassMat->eta.value;
			glass.etaIdx = static_cast<int>(glassMat->eta.texture);

			glass.uroughness = glassMat->uroughness.value;
			glass.uroughnessIdx = static_cast<int>(glassMat->uroughness.texture);

			glass.vroughness = glassMat->vroughness.value;
			glass.vroughnessIdx = static_cast<int>(glassMat->vroughness.texture);

			glass.remaproughness = glassMat->remaproughness ? 1 : 0;

			m_glassList.push_back(glass);
		}
		else if (mat->type() == minipbrt::MaterialType::Mirror) {
			material.index = m_mirrorList.size();
			m_materialList.push_back(material);

			const auto* mirrorMat = static_cast<const minipbrt::MirrorMaterial*>(mat);

			MirrorGPU mirror;

			mirror.Kr.r = mirrorMat->Kr.value[0];
			mirror.Kr.g = mirrorMat->Kr.value[1];
			mirror.Kr.b = mirrorMat->Kr.value[2];
			mirror.KrIdx = static_cast<int>(mirrorMat->Kr.texture);

			m_mirrorList.push_back(mirror);
		}
		else if (mat->type() == minipbrt::MaterialType::Substrate) {
			material.index = m_substrateList.size();
			m_materialList.push_back(material);
			
			const auto* substrateMat = static_cast<const minipbrt::SubstrateMaterial*>(mat);
			SubstrateGPU substrate;
			substrate.Kd.r = substrateMat->Kd.value[0];
			substrate.Kd.g = substrateMat->Kd.value[1];
			substrate.Kd.b = substrateMat->Kd.value[2];
			substrate.KdIdx = static_cast<int>(substrateMat->Kd.texture);

			substrate.Ks.r = substrateMat->Ks.value[0];
			substrate.Ks.g = substrateMat->Ks.value[1];
			substrate.Ks.b = substrateMat->Ks.value[2];
			substrate.KsIdx = static_cast<int>(substrateMat->Ks.texture);

			substrate.uroughness = substrateMat->uroughness.value;
			substrate.uroughnessIdx = static_cast<int>(substrateMat->uroughness.texture);

			substrate.vroughness = substrateMat->vroughness.value;
			substrate.vroughnessIdx = static_cast<int>(substrateMat->vroughness.texture);

			substrate.remaproughness = substrateMat->remaproughness ? 1 : 0;
			m_substrateList.push_back(substrate);
		}
		else if (mat->type() == minipbrt::MaterialType::Fourier) {
			material.index = m_matteList.size();
			material.type = static_cast<int>(minipbrt::MaterialType::Matte);
			m_materialList.push_back(material);

			MatteGPU matte;
			m_matteList.push_back(matte);
		}
		else if (mat->type() == minipbrt::MaterialType::Plastic) {
			material.index = m_plasticList.size();
			m_materialList.push_back(material);

			const auto* plasticMat = static_cast<const minipbrt::PlasticMaterial*>(mat);
			PlasticGPU plastic;

			plastic.Kd.r = plasticMat->Kd.value[0];
			plastic.Kd.g = plasticMat->Kd.value[1];
			plastic.Kd.b = plasticMat->Kd.value[2];
			plastic.KdIdx = static_cast<int>(plasticMat->Kd.texture);

			plastic.Ks.r = plasticMat->Ks.value[0];
			plastic.Ks.g = plasticMat->Ks.value[1];
			plastic.Ks.b = plasticMat->Ks.value[2];
			plastic.KsIdx = static_cast<int>(plasticMat->Ks.texture);

			plastic.roughness = plasticMat->roughness.value;
			plastic.roughnessIdx = static_cast<int>(plasticMat->roughness.texture);

			plastic.remaproughness = plasticMat->remaproughness ? 1 : 0;

			m_plasticList.push_back(plastic);
		}
		else {
			std::cout << "Unknown Material " << static_cast<int>(mat->type()) << std::endl;
			material.type = static_cast<int>(minipbrt::MaterialType::Matte);
			material.index = m_matteList.size();
			m_materialList.push_back(material);

			MatteGPU matte;
			m_matteList.push_back(matte);
		}
	}

	for (auto& tex : m_pbrtScene->textures) {
		m_textureNameMap[tex->name] = m_textureList.size();
		if (tex->type() == minipbrt::TextureType::ImageMap) {
			const auto* imageMapTex = static_cast<const minipbrt::ImageMapTexture*>(tex);
			// auto texture = Texture::createTexture(m_context.get(), imageMapTex->filename, TextureFormatType::LinearUNORM);
			auto texture = Texture::createTexture(m_context.get(), imageMapTex->filename, TextureFormatType::ColorSRGB);
			m_textureList.push_back(std::move(texture));
		}
		else if (tex->type() == minipbrt::TextureType::FBM) {
			auto texture = Texture::createDefaultTexture(m_context.get(), glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
			m_textureList.push_back(std::move(texture));
		}
		else {
			std::cout << "Unknown Texture " << static_cast<int>(tex->type()) << std::endl;
			auto texture = Texture::createDefaultTexture(m_context.get(), glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
			m_textureList.push_back(std::move(texture));
		}
	}


	// area light
	for (auto& aL : m_pbrtScene->areaLights) {
		AreaLightGPU areaLight;
		areaLight.scale.r = aL->scale[0];
		areaLight.scale.g = aL->scale[1];
		areaLight.scale.b = aL->scale[2];

		auto diffuseAreaLight = static_cast<minipbrt::DiffuseAreaLight*>(aL);
		areaLight.L.r = diffuseAreaLight->L[0];
		areaLight.L.g = diffuseAreaLight->L[1];
		areaLight.L.b = diffuseAreaLight->L[2];
		areaLight.twosided = diffuseAreaLight->twosided ? 1 : 0;
		areaLight.samples = diffuseAreaLight->samples;
		m_areaLightList.push_back(areaLight);
	}


	//shape
	for (auto& shape : m_pbrtScene->shapes) {
		if (shape->type() == minipbrt::ShapeType::PLYMesh) {

			ShapeGPU s;
			s.modelMatrix = toGlm(shape->shapeToWorld);
			// scale -1 1 1
			s.materialIdx = shape->material;
			s.areaLightIdx = shape->areaLight;
			s.reverseOrientation = shape->reverseOrientation ? 1 : 0;

			auto plyMesh = static_cast<minipbrt::PLYMesh*>(shape);
			s.alphaIdx = plyMesh->alpha;
			s.shadowAlphaIdx = plyMesh->shadowalpha;
			
			if (!plyMesh->can_convert_to_triangle_mesh()) {
				std::cout << "Cannot convert to triangle mesh" << std::endl;
				continue;
			}
			auto mesh = plyMesh->triangle_mesh();

			AreaLightTriangleGPU areaLightTriangle;
			
			std::vector<Vertex> vertices(mesh->num_vertices);
			for (int i = 0; i < mesh->num_vertices; i++) {
				if (mesh->P != nullptr) {
					vertices[i].pos.x = mesh->P[3 * i + 0];
					vertices[i].pos.y = mesh->P[3 * i + 1];
					vertices[i].pos.z = mesh->P[3 * i + 2];
				}
				if (mesh->N != nullptr) {
					vertices[i].normal.x = mesh->N[3 * i + 0];
					vertices[i].normal.y = mesh->N[3 * i + 1];
					vertices[i].normal.z = mesh->N[3 * i + 2];
				}
				if (mesh->S != nullptr) {
					vertices[i].tangent.x = mesh->S[3 * i + 0];
					vertices[i].tangent.y = mesh->S[3 * i + 1];
					vertices[i].tangent.z = mesh->S[3 * i + 2];
				}
				if (mesh->uv != nullptr) {
					vertices[i].texCoord.x = mesh->uv[2 * i + 0];
					vertices[i].texCoord.y = mesh->uv[2 * i + 1];
				}
			}

			std::vector<uint32_t> indices(mesh->num_indices);
			for (int i = 0; i < mesh->num_indices; i++) {
				indices[i] = mesh->indices[i];
			}

			if (s.areaLightIdx != -1) {
				for (int i = 0; i < mesh->num_indices; i += 3) {
					areaLightTriangle.worldPos0 = glm::vec3(s.modelMatrix * glm::vec4(vertices[indices[i]].pos, 1.0f));
					areaLightTriangle.worldPos1 = glm::vec3(s.modelMatrix * glm::vec4(vertices[indices[i + 1]].pos, 1.0f));
					areaLightTriangle.worldPos2 = glm::vec3(s.modelMatrix * glm::vec4(vertices[indices[i + 2]].pos, 1.0f));

					glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(s.modelMatrix)));
					areaLightTriangle.worldNormal = glm::normalize(normalMatrix * vertices[indices[i]].normal);
					areaLightTriangle.area = glm::length(glm::cross(areaLightTriangle.worldPos0 - areaLightTriangle.worldPos1, areaLightTriangle.worldPos0 - areaLightTriangle.worldPos2)) * 0.5f;
					areaLightTriangle.L = m_areaLightList[s.areaLightIdx].L;
					
					m_areaLightTriangleList.push_back(areaLightTriangle);
				}
			}

			auto meshClass = Mesh::createMesh(m_context.get(), vertices, indices);
			s.vertexAddress = meshClass->getVertexBuffer()->getDeviceAddress();
			s.indexAddress = meshClass->getIndexBuffer()->getDeviceAddress();
			m_meshList.push_back(std::move(meshClass));
			m_shapeList.push_back(s);
		}
		else {
			std::cout << "Unknown Shape" << static_cast<int>(shape->type()) << std::endl;
		}
	}
	// std::vector<int> idx = {52, 67, 13, 16, 51, 8, 34, 25, 31, 14, 15};
	
	// std::vector<int> idx = {8, 13, 16, 25, 31, 34, 51, 52, 59};
	// for (int i = 0; i < idx.size(); i++) {
	// 	m_shapeList[idx[i]].reverseOrientation = 1;
	// }

	glm::vec3 dir = glm::normalize(m_camera.camDir);
	m_pitch = glm::degrees(asin(dir.y));
	m_yaw = glm::degrees(atan2(dir.z, dir.x));
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	m_camera.camRight = glm::normalize(glm::cross(m_camera.camDir, worldUp));
	m_camera.camUp = glm::normalize(glm::cross(m_camera.camRight, m_camera.camDir));
	m_camera.areaLightTriangleCount = m_areaLightTriangleList.size();
	m_initalCamera = m_camera;

	// scene size
	// std::cout << "shape size: " << m_pbrtScene->shapes.size() << std::endl;
	// std::cout << "material size: " << m_pbrtScene->materials.size() << std::endl;
	// std::cout << "texture size: " << m_pbrtScene->textures.size() << std::endl;
	// std::cout << "area light size: " << m_pbrtScene->areaLights.size() << std::endl;
	// std::cout << "object size: " << m_pbrtScene->objects.size() << std::endl;
	// std::cout << "instance size: " << m_pbrtScene->instances.size() << std::endl;
	// std::cout << "light size: " << m_pbrtScene->lights.size() << std::endl;
	// std::cout << "medium size: " << m_pbrtScene->mediums.size() << std::endl;
	// std::cout << "-------------------------" << std::endl;
	// std::cout << "Scene loaded!" << std::endl;
}


void Renderer::update(float deltaTime) {
	m_prevCamera = m_camera;

	glm::vec3 prevCamPos = m_camera.camPos;
	glm::vec3 prevCamDir = m_camera.camDir;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		if (!m_mousePressed) {
			m_mousePressed = true;
			m_lastMouseX = xpos;
			m_lastMouseY = ypos;
		}

		float xoffset = static_cast<float>(xpos - m_lastMouseX);
		float yoffset = static_cast<float>(m_lastMouseY - ypos);

		m_lastMouseX = xpos;
		m_lastMouseY = ypos;

		xoffset *= m_mouseSensitivity;
		yoffset *= m_mouseSensitivity;

		m_yaw -= xoffset;
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
	else {
		m_mousePressed = false;
	}

	glm::vec3 right = m_camera.camRight;
	glm::vec3 move = glm::vec3(0.0f);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += m_camera.camDir;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= m_camera.camDir;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move -= right;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move += right;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) move -= m_camera.camUp;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) move += m_camera.camUp;

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		m_camera.camPos = m_initalCamera.camPos;
		m_camera.camDir = m_initalCamera.camDir;
		m_camera.camUp = m_initalCamera.camUp;
		m_camera.camRight = m_initalCamera.camRight;
		m_pitch = glm::degrees(asin(m_camera.camDir.y));
		m_yaw = glm::degrees(atan2(m_camera.camDir.z, m_camera.camDir.x));
	}

	if (glm::length(move) > 0.0f) {
		move = glm::normalize(move);
		m_camera.camPos += move * m_moveSpeed * deltaTime;
	}

	if (m_camera.camPos != prevCamPos || m_camera.camDir != prevCamDir) {
		m_options.sampleCount = -1;
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

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(m_commandBuffers->getCommandBuffers()[currentFrame], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	// start record

	m_options.frameCount = m_options.frameCount + 1;
	m_options.sampleCount = m_options.sampleCount + 1;
	if (m_options.sampleCount >= m_options.maxSampleCount) {
		m_options.sampleCount = m_options.maxSampleCount;
	}
	m_optionsBuffer->updateUniformBuffer(&m_options, sizeof(OptionsGPU));
	m_camera.areaLightTriangleCount = m_areaLightTriangleList.size();
	m_cameraBuffer->updateUniformBuffer(&m_camera, sizeof(CameraGPU));
	m_prevCameraBuffer->updateUniformBuffer(&m_prevCamera, sizeof(CameraGPU));

	ExposureGPU exposureGPU;
	memset(&exposureGPU, 0, sizeof(ExposureGPU));
	m_exposureBuffer->updateStorageBuffer(&exposureGPU, sizeof(ExposureGPU));
	
	recordPTCommandBuffer();

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_ptTexture0.get(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_ptTexture1.get(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	recordGaussianBlurCommandBuffer();
	
	// recordComputeExposureCommandBuffer();
	// recordBloomCommandBuffer();
	// if (m_options.sampleCount < m_options.maxSampleCount) 
	recordATorusFilterCommandBuffer();

	recordCompositeCommandBuffer();
	
	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_ptTexture0.get(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_ptTexture1.get(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		
		transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_bloomTexture.get(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_aTorusTexture.get(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);


	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_gBufferAlbedoTexture.get(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	
	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_gBufferNormalTexture.get(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_gBufferDepthTexture.get(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_directCurrentTexture.get(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_indirectCurrentTexture.get(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_directFilteredTexture.get(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	
	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_indirectFilteredTexture.get(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_compositeTexture.get(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	// recordToneMappingCommandBuffer();
	recordImGuiCommandBuffer(imageIndex, deltaTime);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_ptTexture0.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_ptTexture1.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_bloomTexture.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_aTorusTexture.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_gBufferAlbedoTexture.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_gBufferNormalTexture.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_gBufferDepthTexture.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);


	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_directCurrentTexture.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_indirectCurrentTexture.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_directFilteredTexture.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_indirectFilteredTexture.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	transferImageLayout(m_commandBuffers->getCommandBuffers()[currentFrame],
		m_compositeTexture.get(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	// end record

	if (vkEndCommandBuffer(m_commandBuffers->getCommandBuffers()[currentFrame]) != VK_SUCCESS) {
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
	submitInfo.pCommandBuffers = &m_commandBuffers->getCommandBuffers()[currentFrame];

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


	m_set4DescSet.reset();
	m_set5DescSet.reset();
	m_set6DescSet.reset();
	m_set7DescSets[0].reset();
	m_set7DescSets[1].reset();
	m_set7DescSets[2].reset();
	m_set7DescSets[3].reset();

	m_set8DescSet.reset();
	m_set9DescSet.reset();


	m_directHistoryTexture.reset();
	m_directCurrentTexture.reset();
	m_indirectHistoryTexture.reset();
	m_indirectCurrentTexture.reset();
	m_directFilteredTexture.reset();
	m_indirectFilteredTexture.reset();
	m_compositeTexture.reset();

	m_directM1Texture.reset();
	m_directM2Texture.reset();
	m_indirectM1Texture.reset();
	m_indirectM2Texture.reset();
	m_directVarianceTexture.reset();
	m_indirectVarianceTexture.reset();
	m_directBlurredVarianceTexture.reset();
	m_indirectBlurredVarianceTexture.reset();

	m_ptTexture0.reset();
	m_ptTexture1.reset();
	m_toneMappingFrameBuffer.reset();
	m_outputTexture.reset();

	m_brightTexture.reset();
	m_blurHTexture.reset();
	m_blurVTexture.reset();
	m_bloomTexture.reset();
	m_aTorusTexture.reset();
	m_gBufferNormalTexture.reset();
	m_gBufferDepthTexture.reset();
	m_gBufferAlbedoTexture.reset();


	m_brightTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_blurHTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_blurVTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_bloomTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_aTorusTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferNormalTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferDepthTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferAlbedoTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferMeshIDTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferSampleCountTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferMotionVectorTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferPrevNormalTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferPrevDepthTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_gBufferPrevMeshIDTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	m_directHistoryTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_directCurrentTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_indirectHistoryTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_indirectCurrentTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_directFilteredTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_indirectFilteredTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	m_directM1Texture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_directM2Texture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_indirectM1Texture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_indirectM2Texture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	m_directVarianceTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_indirectVarianceTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	m_directBlurredVarianceTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_indirectBlurredVarianceTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	m_compositeTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	m_ptTexture0 = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_ptTexture1 = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_set4DescSet = DescriptorSet::createSet4DescSet(m_context.get(), m_set4Layout.get(), m_tlas->getHandle(), m_directHistoryTexture.get(), m_directCurrentTexture.get(), m_indirectHistoryTexture.get(), m_indirectCurrentTexture.get(),
		m_directM1Texture.get(), m_directM2Texture.get(), m_indirectM1Texture.get(), m_indirectM2Texture.get(), m_directVarianceTexture.get(), m_indirectVarianceTexture.get());
	// m_set5DescSet = DescriptorSet::createSet5DescSet(m_context.get(), m_set5Layout.get(), m_exposureBuffer.get(), m_ptTexture0.get());
	// m_set5DescSet = DescriptorSet::createSet5DescSet(m_context.get(), m_set5Layout.get(), m_exposureBuffer.get(), m_bloomTexture.get());
	m_set5DescSet = DescriptorSet::createSet5DescSet(m_context.get(), m_set5Layout.get(), m_exposureBuffer.get(), m_aTorusTexture.get());
	
	m_set7DescSets[0] = DescriptorSet::createSet7DescSet(m_context.get(), m_set7Layout.get(), m_directCurrentTexture.get(), m_directFilteredTexture.get(), m_directHistoryTexture.get(), m_directVarianceTexture.get(), m_directUpdatedVarianceTexture.get());
	m_set7DescSets[1] = DescriptorSet::createSet7DescSet(m_context.get(), m_set7Layout.get(), m_directFilteredTexture.get(), m_directCurrentTexture.get(), m_directHistoryTexture.get(), m_directUpdatedVarianceTexture.get(), m_directVarianceTexture.get());
	m_set7DescSets[2] = DescriptorSet::createSet7DescSet(m_context.get(), m_set7Layout.get(), m_indirectCurrentTexture.get(), m_indirectFilteredTexture.get(), m_indirectHistoryTexture.get(), m_indirectVarianceTexture.get(), m_indirectUpdatedVarianceTexture.get());
	m_set7DescSets[3] = DescriptorSet::createSet7DescSet(m_context.get(), m_set7Layout.get(), m_indirectFilteredTexture.get(), m_indirectCurrentTexture.get(), m_indirectHistoryTexture.get(), m_indirectUpdatedVarianceTexture.get(), m_indirectVarianceTexture.get());
	m_set8DescSet = DescriptorSet::createSet8DescSet(m_context.get(), m_set8Layout.get(), m_gBufferNormalTexture.get(), m_gBufferDepthTexture.get(), m_gBufferAlbedoTexture.get(), m_gBufferMeshIDTexture.get(), m_gBufferSampleCountTexture.get(), m_gBufferMotionVectorTexture.get(),
		m_gBufferPrevNormalTexture.get(), m_gBufferPrevDepthTexture.get(), m_gBufferPrevMeshIDTexture.get());
	m_set9DescSet = DescriptorSet::createSet9DescSet(m_context.get(), m_set9Layout.get(), m_directFilteredTexture.get(), m_indirectFilteredTexture.get(), m_compositeTexture.get());
	m_set10DescSets[0] = DescriptorSet::createSet10DescSet(m_context.get(), m_set10Layout.get(), m_directVarianceTexture.get(), m_directBlurredVarianceTexture.get());
	m_set10DescSets[1] = DescriptorSet::createSet10DescSet(m_context.get(), m_set10Layout.get(), m_indirectVarianceTexture.get(), m_indirectBlurredVarianceTexture.get());

	m_outputTexture = Texture::createAttachmentTexture(m_context.get(), m_extent.width, m_extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_toneMappingFrameBuffer = FrameBuffer::createToneMappingFrameBuffer(m_context.get(), m_toneMappingRenderPass.get(), m_outputTexture.get(), m_extent);

	m_set6DescSet = DescriptorSet::createSet6DescSet(m_context.get(), m_set6Layout.get(), m_ptTexture0.get(), m_brightTexture.get(), m_blurHTexture.get(), m_blurVTexture.get(), m_bloomTexture.get());

	m_guiRenderer->createViewPortDescriptorSet({m_compositeTexture.get(), m_directFilteredTexture.get(), m_indirectFilteredTexture.get()});
	// m_guiRenderer->createViewPortDescriptorSet({m_ptTexture0.get(), m_ptTexture1.get()});
	m_guiRenderer->createGBufferDescriptorSet({m_gBufferNormalTexture.get(), m_gBufferDepthTexture.get(), m_gBufferAlbedoTexture.get()});

	auto cmd = VulkanUtil::beginSingleTimeCommands(m_context.get());
	transferImageLayout(cmd, m_ptTexture0.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_ptTexture1.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_brightTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_blurHTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_blurVTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_bloomTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_aTorusTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_gBufferNormalTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_gBufferDepthTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_gBufferAlbedoTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_gBufferMeshIDTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_gBufferSampleCountTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_gBufferMotionVectorTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_gBufferPrevNormalTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_gBufferPrevDepthTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_gBufferPrevMeshIDTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_directHistoryTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_directCurrentTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_indirectHistoryTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_indirectCurrentTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_directFilteredTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_indirectFilteredTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_compositeTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_directM1Texture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_directM2Texture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_indirectM1Texture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_indirectM2Texture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_directVarianceTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_indirectVarianceTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_directBlurredVarianceTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_indirectBlurredVarianceTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	transferImageLayout(cmd, m_directUpdatedVarianceTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	transferImageLayout(cmd, m_indirectUpdatedVarianceTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_NONE_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
	VulkanUtil::endSingleTimeCommands(m_context.get(), cmd);
}


void Renderer::recordImGuiCommandBuffer(uint32_t imageIndex, float deltaTime) {
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

	vkCmdBeginRenderPass(m_commandBuffers->getCommandBuffers()[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_guiRenderer->newFrame();
	m_guiRenderer->render(m_commandBuffers->getCommandBuffers()[currentFrame], deltaTime, m_options);
	vkCmdEndRenderPass(m_commandBuffers->getCommandBuffers()[currentFrame]);
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


void Renderer::recordReflectionCommandBuffer() {
	// VkCommandBuffer cmd = m_commandBuffers->getCommandBuffers()[currentFrame];

	// vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_reflectionPipeline->getPipeline());

	// VkDescriptorSet sets[] = {
	// 	m_globlaDescSets[currentFrame]->getDescriptorSet(),  // set=0 (camera)
	// 	m_rtDescSets[currentFrame]->getDescriptorSet(),       // set=1 (outputImage + TLAS)
	// 	m_objectMaterialDescSets[currentFrame]->getDescriptorSet(), // set=2 (object material)
	// 	m_bindlessDescSets[currentFrame]->getDescriptorSet(), // set=3 (bindless)
	// 	m_attachmentDescSets[currentFrame]->getDescriptorSet() // set=4 (gbuffer)
	// };
	// vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
	// 	m_reflectionPipeline->getPipelineLayout(), 0, 5, sets, 0, nullptr);

	// VkStridedDeviceAddressRegionKHR emptyRegion{};
	// g_vkCmdTraceRaysKHR(
	// 	cmd,
	// 	&m_reflectionPipeline->getRaygenRegion(),
	// 	&m_reflectionPipeline->getMissRegion(),
	// 	&m_reflectionPipeline->getHitRegion(),
	// 	&emptyRegion,
	// 	m_extent.width,
	// 	m_extent.height,
	// 	1);
}

void Renderer::recordPTCommandBuffer(){
	VkCommandBuffer cmd = m_commandBuffers->getCommandBuffers()[currentFrame];

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_ptPipeline->getPipeline());

	VkDescriptorSet sets[] = {
		m_set0DescSet->getDescriptorSet(),
		m_set1DescSet->getDescriptorSet(),
		m_set2DescSet->getDescriptorSet(),
		m_set3DescSet->getDescriptorSet(),
		m_set4DescSet->getDescriptorSet(),
		m_set8DescSet->getDescriptorSet()
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


void Renderer::recordComputeExposureCommandBuffer() {
	VkCommandBuffer cmd = m_commandBuffers->getCommandBuffers()[currentFrame];
	
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_computeExposurePipeline->getPipeline());

	VkDescriptorSet sets[] = {
		m_set4DescSet->getDescriptorSet(), // pingImage (binding 1)
		m_set5DescSet->getDescriptorSet()  // exposureBuffer (binding 0)
	};

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
		m_computeExposurePipeline->getPipelineLayout(),
		0,
		2, sets,
		0, nullptr);

	glm::uvec2 imageSize = { m_extent.width, m_extent.height };
	vkCmdPushConstants(cmd,
		m_computeExposurePipeline->getPipelineLayout(),
		VK_SHADER_STAGE_COMPUTE_BIT,
		0, sizeof(imageSize),
		&imageSize);

	uint32_t groupX = (m_extent.width + 15) / 16;
	uint32_t groupY = (m_extent.height + 15) / 16;
	vkCmdDispatch(cmd, groupX, groupY, 1);
}

void Renderer::recordToneMappingCommandBuffer() {
	VkCommandBuffer cmd = m_commandBuffers->getCommandBuffers()[currentFrame];

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_toneMappingRenderPass->getRenderPass();
    renderPassInfo.framebuffer = m_toneMappingFrameBuffer->getFrameBuffer();
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_extent;

    VkClearValue clearColor{};
    clearColor.color = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width  = static_cast<float>(m_extent.width);
    viewport.height = static_cast<float>(m_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_toneMappingPipeline->getPipeline());

    VkDescriptorSet sets[] = {
        m_set5DescSet->getDescriptorSet()
    };

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_toneMappingPipeline->getPipelineLayout(), 0, 1, sets, 0, nullptr);

    glm::uvec2 imageSize = { m_extent.width, m_extent.height };
    vkCmdPushConstants(cmd,
        m_toneMappingPipeline->getPipelineLayout(),
        VK_SHADER_STAGE_FRAGMENT_BIT,
        0, sizeof(imageSize),
        &imageSize);

    vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdEndRenderPass(cmd);
}

void Renderer::recordBloomCommandBuffer() {
	VkCommandBuffer cmd = m_commandBuffers->getCommandBuffers()[currentFrame];

	
	// --- 1. Threshold Pass ---
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_thresholdPipeline->getPipeline());

	VkDescriptorSet thresholdSet = m_set6DescSet->getDescriptorSet(); // set6

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
		m_thresholdPipeline->getPipelineLayout(),
		0, 1, &thresholdSet,
		0, nullptr);

	ThresholdGPU thresholdPush{};
	thresholdPush.screenSize = glm::vec2(m_extent.width, m_extent.height);
	thresholdPush.threshold  = 2.0f;

	vkCmdPushConstants(cmd,
		m_thresholdPipeline->getPipelineLayout(),
		VK_SHADER_STAGE_COMPUTE_BIT,
		0, sizeof(ThresholdGPU),
		&thresholdPush);

	uint32_t groupX = (m_extent.width + 15) / 16;
	uint32_t groupY = (m_extent.height + 15) / 16;
	vkCmdDispatch(cmd, groupX, groupY, 1);

	// --- 2. Blur H Pass ---
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_blurHPipeline->getPipeline());

	BlurGPU blurPush{};
	blurPush.radius    = 5;    // int
	blurPush.sigma     = 1.0f;     // float
	blurPush.imageSize = glm::ivec2(m_extent.width, m_extent.height);

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
		m_blurHPipeline->getPipelineLayout(),
		0, 1, &thresholdSet,
		0, nullptr);

	vkCmdPushConstants(cmd,
		m_blurHPipeline->getPipelineLayout(),
		VK_SHADER_STAGE_COMPUTE_BIT,
		0, sizeof(BlurGPU),
		&blurPush);

	vkCmdDispatch(cmd, groupX, groupY, 1);

	// --- 3. Blur V Pass ---
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_blurVPipeline->getPipeline());

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
		m_blurVPipeline->getPipelineLayout(),
		0, 1, &thresholdSet,
		0, nullptr);

	vkCmdPushConstants(cmd,
		m_blurVPipeline->getPipelineLayout(),
		VK_SHADER_STAGE_COMPUTE_BIT,
		0, sizeof(BlurGPU),
		&blurPush);

	vkCmdDispatch(cmd, groupX, groupY, 1);

	// --- 4. Composite Pass ---
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_compositeBloomPipeline->getPipeline());

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
		m_compositeBloomPipeline->getPipelineLayout(),
		0, 1, &thresholdSet,
		0, nullptr);

	CompositeGPU compositePush{};
	compositePush.screenSize    = glm::vec2(m_extent.width, m_extent.height);
	compositePush.bloomStrength = 0.5f;

	vkCmdPushConstants(cmd,
		m_compositeBloomPipeline->getPipelineLayout(),
		VK_SHADER_STAGE_COMPUTE_BIT,
		0, sizeof(CompositeGPU),
		&compositePush);

	vkCmdDispatch(cmd, groupX, groupY, 1);
}

void Renderer::recordATorusFilterCommandBuffer() {
	VkCommandBuffer cmd = m_commandBuffers->getCommandBuffers()[currentFrame];

	const uint32_t groupX = (m_extent.width + 7) / 8;
	const uint32_t groupY = (m_extent.height + 7) / 8;

	const int iterationCount = 5;
	for (int i = 0; i < iterationCount; ++i) {
		VkDescriptorSet set7 = m_set7DescSets[i % 2]->getDescriptorSet(); // 0 / 1
		VkDescriptorSet set8 = m_set8DescSet->getDescriptorSet();
		VkDescriptorSet set0 = m_set0DescSet->getDescriptorSet();

		VkDescriptorSet sets[] = { set7, set8, set0 };

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_aTorusFilterPipeline->getPipeline());
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_aTorusFilterPipeline->getPipelineLayout(), 0, 3, sets, 0, nullptr);

		int stepSize = 1 << i;
		vkCmdPushConstants(cmd, m_aTorusFilterPipeline->getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(int), &stepSize);
		vkCmdDispatch(cmd, groupX, groupY, 1);
	}

	for (int i = 0; i < iterationCount; ++i) {
		VkDescriptorSet set7 = m_set7DescSets[2 + (i % 2)]->getDescriptorSet(); // 2 / 3
		VkDescriptorSet set8 = m_set8DescSet->getDescriptorSet();
		VkDescriptorSet set0 = m_set0DescSet->getDescriptorSet();

		VkDescriptorSet sets[] = { set7, set8, set0 };

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_aTorusFilterPipeline->getPipeline());
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_aTorusFilterPipeline->getPipelineLayout(), 0, 3, sets, 0, nullptr);

		int stepSize = 1 << i;
		vkCmdPushConstants(cmd, m_aTorusFilterPipeline->getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(int), &stepSize);
		vkCmdDispatch(cmd, groupX, groupY, 1);
	}
}


void Renderer::recordCompositeCommandBuffer() {
	VkCommandBuffer cmd = m_commandBuffers->getCommandBuffers()[currentFrame];

	const uint32_t groupX = (m_extent.width + 7) / 8;
	const uint32_t groupY = (m_extent.height + 7) / 8;

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_compositePipeline->getPipeline());

	// set 0: albedo, set 1: filtered+output
	VkDescriptorSet sets[] = {
		m_set8DescSet->getDescriptorSet(), // set = 0 (albedo)
		m_set9DescSet->getDescriptorSet()  // set = 1 (filtered + output)
	};

	vkCmdBindDescriptorSets(cmd,
		VK_PIPELINE_BIND_POINT_COMPUTE,
		m_compositePipeline->getPipelineLayout(),
		0,
		2,
		sets,
		0, nullptr);

	vkCmdDispatch(cmd, groupX, groupY, 1);
}

void Renderer::recordGaussianBlurCommandBuffer() {
	VkCommandBuffer cmd = m_commandBuffers->getCommandBuffers()[currentFrame];

	const uint32_t groupX = (m_extent.width + 7) / 8;
	const uint32_t groupY = (m_extent.height + 7) / 8;

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_gaussianBlurPipeline->getPipeline());

	VkDescriptorSet sets[] = {
		m_set10DescSets[0]->getDescriptorSet()
	};

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_gaussianBlurPipeline->getPipelineLayout(), 0, 1, sets, 0, nullptr);

	vkCmdDispatch(cmd, groupX, groupY, 1);

	sets[0] = m_set10DescSets[1]->getDescriptorSet();

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_gaussianBlurPipeline->getPipelineLayout(), 0, 1, sets, 0, nullptr);

	vkCmdDispatch(cmd, groupX, groupY, 1);
}


glm::mat4 Renderer::computeLightMatrix(Light& light) {
	glm::mat4 lightView;
	glm::mat4 lightProj;
	glm::vec3 lightDir = glm::normalize(light.direction);
	glm::vec3 up = (glm::abs(lightDir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

	if (light.type == LIGHT_TYPE_DIRECTIONAL) {
		/*glm::vec3 eye = glm::vec3(0.0f) - glm::normalize(lightDir) * 120.0f;
		glm::vec3 center = glm::vec3(0.0f);*/
		/*lightView = glm::lookAt(eye, center, up);
		lightProj = glm::ortho(-80.f, 80.f, -80.f, 80.f, -50.0f, 250.f);
		lightProj[1][1] *= -1.0f;*/
		glm::vec3 eye = glm::vec3(0.0f) - glm::normalize(lightDir) * 10.0f;
		glm::vec3 center = glm::vec3(0.0f);
		lightView = glm::lookAt(eye, center, up);
		lightProj = glm::ortho(-10.f, 10.f, -10.f, 10.f, -10.0f, 20.f);
		lightProj[1][1] *= -1.0f;
	}
	else if (light.type == LIGHT_TYPE_SPOT) {
		glm::vec3 eye = light.position;
		glm::vec3 target = light.position + glm::normalize(lightDir);

		lightView = glm::lookAt(eye, target, up);
		lightProj = glm::perspective(glm::radians(light.spotOuterAngle * 2.0f), 1.0f, 0.1f, 100.0f);
		lightProj[1][1] *= -1.0f;
	}

	return lightProj * lightView;
}

glm::mat4 Renderer::computePointLightMatrix(Light& light, uint32_t faceIndex) {
	static const glm::vec3 targets[6] = {
		glm::vec3(1,  0,  0), // +X
		glm::vec3(-1,  0,  0), // -X
		glm::vec3(0,  1,  0), // +Y
		glm::vec3(0, -1,  0), // -Y
		glm::vec3(0,  0,  1), // +Z
		glm::vec3(0,  0, -1)  // -Z
	};
	static const glm::vec3 ups[6] = {
		glm::vec3(0, -1,  0), // +X
		glm::vec3(0, -1,  0), // -X
		glm::vec3(0,  0,  1), // +Y
		glm::vec3(0,  0, -1), // -Y
		glm::vec3(0, -1,  0), // +Z
		glm::vec3(0, -1,  0)  // -Z
	};

	glm::vec3 lightPos = light.position;
	glm::mat4 view = glm::lookAt(lightPos, lightPos + targets[faceIndex], ups[faceIndex]);
	glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 50.0f);
	//proj[1][1] *= -1.0f;

	return proj * view;
}

//void Renderer::updateTLAS(std::unordered_map<int32_t, std::vector<int32_t>>& modelToMatrixIndices, std::vector<ModelBuffer>& modelBuffers) {
void Renderer::updateTLAS(std::vector<ObjectInstance>& objDescs, std::vector<ModelBuffer>& modelBuffers) {

	/*if (modelToMatrixIndices.size() > 0) {
		m_tlas[currentFrame]->rebuild(m_blasList, m_modelList, modelToMatrixIndices, modelBuffers, m_scene->getObjects());
		m_rtDescSets[currentFrame]->updateTLAS(m_tlas[currentFrame]->getHandle());
	}
	else {
		m_rtDescSets[currentFrame]->updateTLAS(m_emptyTLAS->getHandle());
	}*/

	// if (objDescs.size() > 0) {
	// 	m_tlas[currentFrame]->rebuild(m_blasList, m_shapeList);
	// 	m_rtDescSets[currentFrame]->updateTLAS(m_tlas[currentFrame]->getHandle());
	// }
	// else {
	// 	m_rtDescSets[currentFrame]->updateTLAS(m_emptyTLAS->getHandle());
	// }
}


void Renderer::printObjectInstances(const std::vector<ObjectInstance>& instances) {
	std::cout << "========== ObjectInstance List ==========" << std::endl;
	for (size_t i = 0; i < instances.size(); ++i) {
		const auto& inst = instances[i];
		std::cout << "[" << std::setw(3) << i << "] "
			<< "vertexAddr: 0x" << std::hex << inst.vertexAddress << std::dec << ", "
			<< "indexAddr:  0x" << std::hex << inst.indexAddress << std::dec << ", "
			<< "modelMatIdx: " << inst.modelMatrixIndex << ", "
			<< "materialIdx: " << inst.materialIndex << ", "
			<< "meshIdx: " << inst.meshIndex
			<< std::endl;
	}
	std::cout << "==========================================" << std::endl;
}

void Renderer::printAllResources(){
	// for (auto& tex : m_textureList) {
	// 	std::cout << "texture view: " << tex->getImageView() << std::endl;
	// 	std::cout << "texture sampler: " << tex->getSampler() << std::endl;
	// 	std::cout << "texture format: " << static_cast<int>(tex->getFormat()) << std::endl;
	// 	std::cout << "----------------------------" << std::endl;
	// }

	// // m_textureNameMap: texture name, index
	// for (auto& tex : m_textureNameMap) {
	// 	std::cout << "texture name: " << tex.first << std::endl;
	// 	std::cout << "texture index: " << tex.second << std::endl;
	// 	std::cout << "----------------------------" << std::endl;
	// }

	// // m_materialList: material type, index
	int i = 0;
	for (auto& mat : m_materialList) {
		std::cout << "material index: " << i++ << std::endl;
		std::cout << "material type: " << mat.type << std::endl;
		std::cout << "material index: " << mat.index << std::endl;
		std::cout << "----------------------------" << std::endl;
	}
	

	// // m_materialNameMap
	// for (auto& mat : m_materialNameMap) {
	// 	std::cout << "material name: " << mat.first << std::endl;
	// 	std::cout << "material index: " << mat.second << std::endl;
	// 	std::cout << "----------------------------" << std::endl;
	// }

	// m_uberList: Kd, Ks, Kr, Kt, opacity, eta, uroughness, vroughness, remaproughness
	i = 0;
	for (auto& mat : m_uberList) {
		std::cout << "Uber Material index: " << i++ << std::endl;
		std::cout << "Uber Material Kd: " << mat.Kd.r << ", " << mat.Kd.g << ", " << mat.Kd.b << std::endl;
		std::cout << "Uber Material KdIdx: " << mat.KdIdx << std::endl;
		std::cout << "Uber Material Ks: " << mat.Ks.r << ", " << mat.Ks.g << ", " << mat.Ks.b << std::endl;
		std::cout << "Uber Material Kr: " << mat.Kr.r << ", " << mat.Kr.g << ", " << mat.Kr.b << std::endl;
		std::cout << "Uber Material Kt: " << mat.Kt.r << ", " << mat.Kt.g << ", " << mat.Kt.b << std::endl;
		std::cout << "Uber Material Opacity: " << mat.opacity.r << ", " << mat.opacity.g << ", " << mat.opacity.b << std::endl;
		std::cout << "Uber Material Eta: " << mat.eta << std::endl;
		std::cout << "Uber Material Uroughness: " << mat.uroughness << std::endl;
		std::cout << "Uber Material Vroughness: " << mat.vroughness << std::endl;
		std::cout << "Uber Material Remaproughness: " << mat.remaproughness << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_matteList: Kd, sigma
	i = 0;
	for (auto& mat : m_matteList) {
		std::cout << "Matte Material index: " << i++ << std::endl;
		std::cout << "Matte Material Kd: " << mat.Kd.r << ", " << mat.Kd.g << ", " << mat.Kd.b << std::endl;
		std::cout << "Matte Material Sigma: " << mat.sigma << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_metalList: eta, k, uroughness, vroughness, remaproughness
	i = 0;
	for (auto& mat : m_metalList) {
		std::cout << "Metal Material index: " << i++ << std::endl;
		std::cout << "Metal Material Eta: " << mat.eta.r << ", " << mat.eta.g << ", " << mat.eta.b << std::endl;
		std::cout << "Metal Material K: " << mat.k.r << ", " << mat.k.g << ", " << mat.k.b << std::endl;
		std::cout << "Metal Material Uroughness: " << mat.uroughness << std::endl;
		std::cout << "Metal Material Vroughness: " << mat.vroughness << std::endl;
		std::cout << "Metal Material Remaproughness: " << mat.remaproughness << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_glassList: Kr, Kt, eta, uroughness, vroughness, remaproughness
	i = 0;
	for (auto& mat : m_glassList) {
		std::cout << "Glass Material index: " << i++ << std::endl;
		std::cout << "Glass Material Kr: " << mat.Kr.r << ", " << mat.Kr.g << ", " << mat.Kr.b << std::endl;
		std::cout << "Glass Material Kt: " << mat.Kt.r << ", " << mat.Kt.g << ", " << mat.Kt.b << std::endl;
		std::cout << "Glass Material Eta: " << mat.eta << std::endl;
		std::cout << "Glass Material Uroughness: " << mat.uroughness << std::endl;
		std::cout << "Glass Material Vroughness: " << mat.vroughness << std::endl;
		std::cout << "Glass Material Remaproughness: " << mat.remaproughness << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_mirrorList: Kr
	i = 0;
	for (auto& mat : m_mirrorList) {
		std::cout << "Mirror Material index: " << i++ << std::endl;
		std::cout << "Mirror Material Kr: " << mat.Kr.r << ", " << mat.Kr.g << ", " << mat.Kr.b << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_substrateList: Kd, Ks, uroughness, vroughness, remaproughness
	i = 0;
	for (auto& mat : m_substrateList) {
		std::cout << "Substrate Material index: " << i++ << std::endl;
		std::cout << "Substrate Material Kd: " << mat.Kd.r << ", " << mat.Kd.g << ", " << mat.Kd.b << std::endl;
		std::cout << "Substrate Material KdIdx: " << mat.KdIdx << std::endl;
		std::cout << "Substrate Material Ks: " << mat.Ks.r << ", " << mat.Ks.g << ", " << mat.Ks.b << std::endl;
		std::cout << "Substrate Material Uroughness: " << mat.uroughness << std::endl;
		std::cout << "Substrate Material Vroughness: " << mat.vroughness << std::endl;
		std::cout << "Substrate Material Remaproughness: " << mat.remaproughness << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_plasticList: Kd, Ks, roughness, remaproughness
	i = 0;
	for (auto& mat : m_plasticList) {
		std::cout << "Plastic Material index: " << i++ << std::endl;
		std::cout << "Plastic Material Kd: " << mat.Kd.r << ", " << mat.Kd.g << ", " << mat.Kd.b << std::endl;
		std::cout << "Plastic Material KdIdx: " << mat.KdIdx << std::endl;
		std::cout << "Plastic Material Ks: " << mat.Ks.r << ", " << mat.Ks.g << ", " << mat.Ks.b << std::endl;
		std::cout << "Plastic Material KsIdx: " << mat.KsIdx << std::endl;
		std::cout << "Plastic Material Roughness: " << mat.roughness << std::endl;
		std::cout << "Plastic Material Remaproughness: " << mat.remaproughness << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_areaLightList: scale, L, twosided, samples
	i = 0;
	for (auto& mat : m_areaLightList) {
		std::cout << "Area Light index: " << i++ << std::endl;
		std::cout << "Area Light Scale: " << mat.scale.r << ", " << mat.scale.g << ", " << mat.scale.b << std::endl;
		std::cout << "Area Light L: " << mat.L.r << ", " << mat.L.g << ", " << mat.L.b << std::endl;
		std::cout << "Area Light Twosided: " << mat.twosided << std::endl;
		std::cout << "Area Light Samples: " << mat.samples << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_shapeList: modelMatrix, materialIdx, areaLightIdx, reverseOrientation
	// i = 0;
	// for (auto& mat : m_shapeList) {
	// 	std::cout << "Shape Model Matrix: " << std::endl;
	// 	for (int i = 0; i < 4; i++) {
	// 		// for (int j = 0; j < 4; j++) {
	// 		// 	std::cout << mat.modelMatrix[i][j] << " ";
	// 		// }
	// 		// std::cout << std::endl;
	// 	}
	// 	std::cout << "Shape Model idx : " << i++ << std::endl;
	// 	std::cout << "Shape Material Index: " << mat.materialIdx << std::endl;
	// 	std::cout << "Shape Area Light Index: " << mat.areaLightIdx << std::endl;
	// 	std::cout << "Shape Reverse Orientation: " << mat.reverseOrientation << std::endl;
	// 	std::cout << "----------------------------" << std::endl;
	// }


	// // m_meshList: vertexBuffer, indexBuffer sizes
	// for (auto& mesh : m_meshList) {
	// 	std::cout << "Mesh Vertex Buffer Size: " << mesh->getVertexBuffer()->getVertexCount() << std::endl;
	// 	std::cout << "Mesh Index Buffer Size: " << mesh->getIndexBuffer()->getIndexCount() << std::endl;
	// 	std::cout << "----------------------------" << std::endl;
	// }

	i = 0;
	for (auto& aL : m_areaLightTriangleList) {
		std::cout << "areaLightTriangle: " << i++ << std::endl;
		std::cout << "worldPos0: " << aL.worldPos0.x << " " << aL.worldPos0.y << " " << aL.worldPos0.z << std::endl;
		std::cout << "worldPos1: " << aL.worldPos1.x << " " << aL.worldPos1.y << " " << aL.worldPos1.z << std::endl;
		std::cout << "worldPos2: " << aL.worldPos2.x << " " << aL.worldPos2.y << " " << aL.worldPos2.z << std::endl;
		std::cout << "worldNormal: " << aL.worldNormal.x << " " << aL.worldNormal.y << " " << aL.worldNormal.z << std::endl;
		std::cout << "area: " << aL.area << std::endl;
		std::cout << "L: " << aL.L.x << " " << aL.L.y << " " << aL.L.z << std::endl;
		std::cout << "----------------------------" << std::endl;
	}
}

void Renderer::updateInitialBuffers() {
	m_cameraBuffer->updateUniformBuffer(&m_camera, sizeof(CameraGPU));
	m_prevCameraBuffer->updateUniformBuffer(&m_prevCamera, sizeof(CameraGPU));
	m_instanceBuffer->updateStorageBuffer(&m_shapeList[0], sizeof(ShapeGPU) * m_shapeList.size());
	m_materialBuffer->updateStorageBuffer(&m_materialList[0], sizeof(MaterialGPU) * m_materialList.size());
	m_uberBuffer->updateStorageBuffer(&m_uberList[0], sizeof(UberGPU) * m_uberList.size());
	m_matteBuffer->updateStorageBuffer(&m_matteList[0], sizeof(MatteGPU) * m_matteList.size());
	m_metalBuffer->updateStorageBuffer(&m_metalList[0], sizeof(MetalGPU) * m_metalList.size());
	m_glassBuffer->updateStorageBuffer(&m_glassList[0], sizeof(GlassGPU) * m_glassList.size());
	m_mirrorBuffer->updateStorageBuffer(&m_mirrorList[0], sizeof(MirrorGPU) * m_mirrorList.size());
	m_substrateBuffer->updateStorageBuffer(&m_substrateList[0], sizeof(SubstrateGPU) * m_substrateList.size());
	m_plasticBuffer->updateStorageBuffer(&m_plasticList[0], sizeof(PlasticGPU) * m_plasticList.size());
	m_areaLightBuffer->updateStorageBuffer(&m_areaLightList[0], sizeof(AreaLightGPU) * m_areaLightList.size());
	m_areaLightTriangleBuffer->updateStorageBuffer(&m_areaLightTriangleList[0], sizeof(AreaLightTriangleGPU) * m_areaLightTriangleList.size());
}

glm::vec3 Renderer::Uncharted2Tonemap(glm::vec3 x) {
    const float A = 0.15f;
    const float B = 0.50f;
    const float C = 0.10f;
    const float D = 0.20f;
    const float E = 0.02f;
    const float F = 0.30f;
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F)) - E/F;
}