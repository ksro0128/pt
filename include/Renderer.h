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
	std::vector<std::unique_ptr<TopLevelAS>> m_tlas;
	std::unique_ptr<TopLevelAS> m_emptyTLAS;
	
	
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

	std::vector<AreaLightGPU> m_areaLightList;

	std::vector<ShapeGPU> m_shapeList;

	// descriptorset layout
	std::unique_ptr<DescriptorSetLayout> m_globalLayout;
	std::unique_ptr<DescriptorSetLayout> m_objectMaterialLayout;
	std::unique_ptr<DescriptorSetLayout> m_bindlessLayout;
	std::unique_ptr<DescriptorSetLayout> m_attachmentLayout;
	std::unique_ptr<DescriptorSetLayout> m_shadowLayout;
	std::unique_ptr<DescriptorSetLayout> m_rayTracingLayout;

	// buffer
	std::array<std::unique_ptr<UniformBuffer>, MAX_FRAMES_IN_FLIGHT> m_cameraBuffers;
	std::array<std::unique_ptr<StorageBuffer>, MAX_FRAMES_IN_FLIGHT> m_lightBuffers;
	std::array<std::unique_ptr<StorageBuffer>, MAX_FRAMES_IN_FLIGHT> m_objectInstanceBuffers;
	std::array<std::unique_ptr<StorageBuffer>, MAX_FRAMES_IN_FLIGHT> m_modelBuffers;
	std::array<std::unique_ptr<StorageBuffer>, MAX_FRAMES_IN_FLIGHT> m_materialBuffers;
	std::array<std::unique_ptr<UniformBuffer>, MAX_FRAMES_IN_FLIGHT> m_lightMatrixBuffers;
	std::array<std::unique_ptr<UniformBuffer>, MAX_FRAMES_IN_FLIGHT> m_renderOptionsBuffers;

	// renderpass
	std::unique_ptr<RenderPass> m_gbufferRenderPass;
	std::unique_ptr<RenderPass> m_imguiRenderPass;
	std::unique_ptr<RenderPass> m_lightPassRenderPass;
	std::unique_ptr<RenderPass> m_shadowMapRenderPass;

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

	// descriptorset
	std::array<std::unique_ptr<DescriptorSet>, MAX_FRAMES_IN_FLIGHT> m_globlaDescSets;
	std::array<std::unique_ptr<DescriptorSet>, MAX_FRAMES_IN_FLIGHT> m_objectMaterialDescSets;
	std::array<std::unique_ptr<DescriptorSet>, MAX_FRAMES_IN_FLIGHT> m_bindlessDescSets;
	std::vector< std::unique_ptr<DescriptorSet> > m_attachmentDescSets;
	std::array<std::unique_ptr<DescriptorSet>, MAX_FRAMES_IN_FLIGHT> m_shadowDescSets;
	std::vector<std::unique_ptr<DescriptorSet>> m_rtDescSets;

	// pipeline
	std::unique_ptr<Pipeline> m_gbufferPipeline;
	std::unique_ptr<Pipeline> m_lightPassPipeline;
	std::unique_ptr<Pipeline> m_shadowMapPipeline;
	std::unique_ptr<RayTracingPipeline> m_reflectionPipeline;
	std::unique_ptr<RayTracingPipeline> m_giPipeline;

	// command buffer
	std::unique_ptr<CommandBuffers> m_commandBuffers;

	// gui renderer
	std::unique_ptr<GuiRenderer> m_guiRenderer;

	// scene
	std::unique_ptr<Scene> m_scene;

	// new
	minipbrt::Scene* m_pbrtScene = nullptr;
	CameraGPU m_camera;

	void loadScene(std::string scenePath);
	//





	void cleanup();
	void init(GLFWwindow* window);
	void recreateSwapChain();
	void recreateViewport(ImVec2 newExtent);
	void loadModel(const std::string& modelPath);
	void createDefaultModels();
	void createObjDesc(std::vector<ObjectInstance>& ObjDescs, std::vector<ModelBuffer>& modelBuffers);
	void updateTLAS(std::vector<ObjectInstance>& objDescs, std::vector<ModelBuffer>& modelBuffers);


	// record command buffer
	void recordGbufferCommandBuffer(std::vector<ObjectInstance>& objDescs);
	void recordLightPassCommandBuffer();
	void recordImGuiCommandBuffer(uint32_t imageIndex, float deltaTime);
	void recordShadowMapCommandBuffer(std::vector<ObjectInstance>& objDescs);
	void recordReflectionCommandBuffer();
	void recordGICmdBuffer();

	void printAllResources();
	void printObjectInstances(const std::vector<ObjectInstance>& instances);

	void transferImageLayout(VkCommandBuffer cmd, Texture* texture, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, uint32_t layerCount = 1);

	void updateCamera(float deltaTime);

	glm::mat4 computeLightMatrix(Light& light);
	glm::mat4 computePointLightMatrix(Light& light, uint32_t faceIndex);
};