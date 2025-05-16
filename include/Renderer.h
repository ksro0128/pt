#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "SwapChain.h"
#include "SyncObjects.h"
#include "Buffer.h"
#include "Mesh.h"
#include "Model.h"
#include "Texture.h"
#include "Object.h"
#include "DescriptorSetLayout.h"
#include "DescriptorSet.h"
#include "RenderPass.h"
#include "FrameBuffer.h"
#include "Pipeline.h"
#include "CommandBuffers.h"
#include "GuiRenderer.h"
#include "Scene.h"
#include "AccelerationStructure.h"
#include "RayTracingPipeline.h"
#include "minipbrt.h"

class Renderer {
public:
	static std::unique_ptr<Renderer> createRenderer(GLFWwindow* window);
	~Renderer();

	void update(float deltaTime);
	void render(float deltaTime);
	bool isBenchmarkRunning() const { return m_guiRenderer->isBenchmarkRunning(); }
private:
	GLFWwindow* window;
	std::unique_ptr<VulkanContext> m_context;
	std::unique_ptr<SwapChain> m_swapChain;
	std::unique_ptr<SyncObjects> m_syncObjects;

	VkExtent2D m_extent;
	uint32_t currentFrame = 0;

	// resources list
	std::vector<std::unique_ptr<Mesh>> m_meshList;
	std::vector<Model> m_modelList;
	std::vector<std::unique_ptr<BottomLevelAS>> m_blasList;
	std::unique_ptr<TopLevelAS> m_tlas;
	std::unique_ptr<TopLevelAS> m_emptyTLAS;

	std::unique_ptr<Texture> m_ptTexture0;
	std::unique_ptr<Texture> m_ptTexture1;

	// desc4 
	std::unique_ptr<Texture> m_directHistoryTexture;
	std::unique_ptr<Texture> m_directCurrentTexture;
	std::unique_ptr<Texture> m_indirectHistoryTexture;
	std::unique_ptr<Texture> m_indirectCurrentTexture;

	std::unique_ptr<Texture> m_directFilteredTexture;
	std::unique_ptr<Texture> m_indirectFilteredTexture;

	std::unique_ptr<Texture> m_directM1Texture;
	std::unique_ptr<Texture> m_directM2Texture;
	std::unique_ptr<Texture> m_indirectM1Texture;
	std::unique_ptr<Texture> m_indirectM2Texture;

	std::unique_ptr<Texture> m_directVarianceTexture;
	std::unique_ptr<Texture> m_indirectVarianceTexture;

	std::unique_ptr<Texture> m_directBlurredVarianceTexture;
	std::unique_ptr<Texture> m_indirectBlurredVarianceTexture;

	std::unique_ptr<Texture> m_directUpdatedVarianceTexture;
	std::unique_ptr<Texture> m_indirectUpdatedVarianceTexture;

	std::unique_ptr<Texture> m_compositeTexture;



	std::unique_ptr<Texture> m_outputTexture;

	std::unique_ptr<Texture> m_brightTexture;
	std::unique_ptr<Texture> m_blurHTexture;
	std::unique_ptr<Texture> m_blurVTexture;
	std::unique_ptr<Texture> m_bloomTexture;

	std::unique_ptr<Texture> m_aTorusTexture;
	
	std::unique_ptr<Texture> m_gBufferNormalTexture;
	std::unique_ptr<Texture> m_gBufferDepthTexture;
	std::unique_ptr<Texture> m_gBufferAlbedoTexture;
	std::unique_ptr<Texture> m_gBufferMeshIDTexture;
	std::unique_ptr<Texture> m_gBufferSampleCountTexture;
	std::unique_ptr<Texture> m_gBufferMotionVectorTexture;
	std::unique_ptr<Texture> m_gBufferPrevNormalTexture;
	std::unique_ptr<Texture> m_gBufferPrevDepthTexture;
	std::unique_ptr<Texture> m_gBufferPrevMeshIDTexture;
	std::unique_ptr<Texture> m_gBufferJitterTexture;
	std::unique_ptr<Texture> m_gBufferPrevJitterTexture;

	std::unique_ptr<Texture> m_depthTexture;

	std::unique_ptr<Texture> m_prevCompositeTexture;
	std::unique_ptr<Texture> m_curCompositeTexture;
	
	
	
	std::vector<std::unique_ptr<Texture>> m_textureList;
	std::unordered_map<std::string, int32_t> m_textureNameMap;
	std::vector<MaterialGPU> m_materialList;
	std::unordered_map<std::string, int32_t> m_materialNameMap;
	std::vector<UberGPU> m_uberList;
	std::vector<MatteGPU> m_matteList;
	std::vector<MetalGPU> m_metalList;
	std::vector<GlassGPU> m_glassList;
	std::vector<MirrorGPU> m_mirrorList;
	std::vector<SubstrateGPU> m_substrateList;
	std::vector<PlasticGPU> m_plasticList;
	std::vector<AreaLightTriangleGPU> m_areaLightTriangleList;

	std::vector<AreaLightGPU> m_areaLightList;

	std::vector<ShapeGPU> m_shapeList;

	// descriptorset layout
	std::unique_ptr<DescriptorSetLayout> m_set0Layout;
	std::unique_ptr<DescriptorSetLayout> m_set1Layout;
	std::unique_ptr<DescriptorSetLayout> m_set2Layout;
	std::unique_ptr<DescriptorSetLayout> m_set3Layout;
	std::unique_ptr<DescriptorSetLayout> m_set4Layout;
	std::unique_ptr<DescriptorSetLayout> m_set5Layout;
	std::unique_ptr<DescriptorSetLayout> m_set6Layout;
	std::unique_ptr<DescriptorSetLayout> m_set7Layout;
	std::unique_ptr<DescriptorSetLayout> m_set8Layout;
	std::unique_ptr<DescriptorSetLayout> m_set9Layout;
	std::unique_ptr<DescriptorSetLayout> m_set10Layout;

	// buffer
	std::unique_ptr<UniformBuffer> m_cameraBuffer;
	std::unique_ptr<UniformBuffer> m_prevCameraBuffer;
	std::unique_ptr<UniformBuffer> m_optionsBuffer;
	std::unique_ptr<UniformBuffer> m_gbufferCameraBuffer;
	std::unique_ptr<StorageBuffer> m_instanceBuffer;

	std::unique_ptr<StorageBuffer> m_materialBuffer;
	std::unique_ptr<StorageBuffer> m_uberBuffer;
	std::unique_ptr<StorageBuffer> m_matteBuffer;
	std::unique_ptr<StorageBuffer> m_metalBuffer;
	std::unique_ptr<StorageBuffer> m_glassBuffer;
	std::unique_ptr<StorageBuffer> m_mirrorBuffer;
	std::unique_ptr<StorageBuffer> m_substrateBuffer;
	std::unique_ptr<StorageBuffer> m_plasticBuffer;
	std::unique_ptr<StorageBuffer> m_areaLightBuffer;
	std::unique_ptr<StorageBuffer> m_exposureBuffer;
	std::unique_ptr<StorageBuffer> m_areaLightTriangleBuffer;

	// renderpass
	std::unique_ptr<RenderPass> m_gbufferRenderPass;
	std::unique_ptr<RenderPass> m_imguiRenderPass;
	std::unique_ptr<RenderPass> m_lightPassRenderPass;
	std::unique_ptr<RenderPass> m_shadowMapRenderPass;
	std::unique_ptr<RenderPass> m_toneMappingRenderPass;

	// attachment
	std::vector<GbufferAttachment> m_gbufferAttachments;
	std::vector<std::unique_ptr<Texture>> m_outputTextures;
	std::vector<std::vector<std::unique_ptr<Texture>>> m_shadowMapTextures;
	std::vector<std::unique_ptr<Texture>> m_shadowCubeMapTextures;
	std::vector<std::unique_ptr<Texture>> m_rtReflectionTextures;

	// framebuffer
	std::vector<std::unique_ptr<FrameBuffer>> m_gbufferFrameBuffers;
	std::vector<std::unique_ptr<FrameBuffer>> m_imguiFrameBuffers;
	std::vector<std::unique_ptr<FrameBuffer>> m_outputFrameBuffers;
	std::vector<std::vector<std::unique_ptr<FrameBuffer>>> m_shadowMapFrameBuffers;
	std::vector<std::vector<std::unique_ptr<FrameBuffer>>> m_shadowCubeMapFrameBuffers;
	std::unique_ptr<FrameBuffer> m_toneMappingFrameBuffer;

	// descriptorset
	std::unique_ptr<DescriptorSet> m_set0DescSet;
	std::unique_ptr<DescriptorSet> m_set1DescSet;
	std::unique_ptr<DescriptorSet> m_set2DescSet;
	std::unique_ptr<DescriptorSet> m_set3DescSet;
	std::unique_ptr<DescriptorSet> m_set4DescSet;
	std::unique_ptr<DescriptorSet> m_set5DescSet;
	std::unique_ptr<DescriptorSet> m_set6DescSet;
	std::array<std::unique_ptr<DescriptorSet>, 4> m_set7DescSets;
	std::unique_ptr<DescriptorSet> m_set8DescSet;
	std::array<std::unique_ptr<DescriptorSet>, 2> m_set8DescSets;
	std::unique_ptr<DescriptorSet> m_set9DescSet;
	std::array<std::unique_ptr<DescriptorSet>, 2> m_set9DescSets;
	std::array<std::unique_ptr<DescriptorSet>, 2> m_set10DescSets;

	// pipeline
	std::unique_ptr<Pipeline> m_gbufferPipeline;
	std::unique_ptr<Pipeline> m_lightPassPipeline;
	std::unique_ptr<Pipeline> m_shadowMapPipeline;
	std::unique_ptr<Pipeline> m_computeExposurePipeline;
	std::unique_ptr<RayTracingPipeline> m_reflectionPipeline;
	std::unique_ptr<RayTracingPipeline> m_giPipeline;

	std::unique_ptr<RayTracingPipeline> m_ptPipeline;
	std::unique_ptr<Pipeline> m_toneMappingPipeline;
	std::unique_ptr<Pipeline> m_thresholdPipeline;
	std::unique_ptr<Pipeline> m_blurHPipeline;
	std::unique_ptr<Pipeline> m_blurVPipeline;
	std::unique_ptr<Pipeline> m_compositeBloomPipeline;
	std::unique_ptr<Pipeline> m_aTorusFilterPipeline;
	std::unique_ptr<Pipeline> m_compositePipeline;
	std::unique_ptr<Pipeline> m_gaussianBlurPipeline;



	// command buffer
	std::unique_ptr<CommandBuffers> m_commandBuffers;

	// gui renderer
	std::unique_ptr<GuiRenderer> m_guiRenderer;

	// scene
	std::unique_ptr<Scene> m_scene;

	// new
	minipbrt::Scene* m_pbrtScene = nullptr;
	CameraGPU m_camera;
	CameraGPU m_prevCamera;
	bool m_mousePressed = false;
	double m_lastMouseX = 0.0, m_lastMouseY = 0.0;
	float m_yaw = -90.0f;
	float m_pitch = 0.0f;
	float m_mouseSensitivity = 0.2f;
	float m_moveSpeed = 10.0f;

	CameraGPU m_initalCamera;


	OptionsGPU m_options;

	glm::vec2 curJitter;
	glm::vec2 prevJitter;

	void loadScene(std::string scenePath);
	glm::mat4 toGlm(const minipbrt::Transform& t);

	void cleanup();
	void init(GLFWwindow* window);
	void recreateSwapChain();
	void recreateViewport(ImVec2 newExtent);
	void loadModel(const std::string& modelPath);
	void createDefaultModels();
	void createObjDesc(std::vector<ObjectInstance>& ObjDescs, std::vector<ModelBuffer>& modelBuffers);
	void updateTLAS(std::vector<ObjectInstance>& objDescs, std::vector<ModelBuffer>& modelBuffers);


	// record command buffer
	void recordGbufferCommandBuffer();
	void recordLightPassCommandBuffer();
	void recordImGuiCommandBuffer(uint32_t imageIndex, float deltaTime);
	void recordShadowMapCommandBuffer(std::vector<ObjectInstance>& objDescs);
	void recordReflectionCommandBuffer();
	void recordGICmdBuffer();
	void recordComputeExposureCommandBuffer();
	void recordToneMappingCommandBuffer();
	void recordBloomCommandBuffer();
	void recordATorusFilterCommandBuffer();
	void recordCompositeCommandBuffer();
	void recordGaussianBlurCommandBuffer();

	void recordPTCommandBuffer();

	void printObjectInstances(const std::vector<ObjectInstance>& instances);

	void transferImageLayout(VkCommandBuffer cmd, Texture* texture, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, uint32_t layerCount = 1);

	void updateCamera(float deltaTime);

	glm::mat4 computeLightMatrix(Light& light);
	glm::mat4 computePointLightMatrix(Light& light, uint32_t faceIndex);

	void printAllResources();
	void updateInitialBuffers();

	glm::vec3 Uncharted2Tonemap(glm::vec3 x);
};