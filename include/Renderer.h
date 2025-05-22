#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "SwapChain.h"
#include "SyncObjects.h"
#include "Buffer.h"
#include "Mesh.h"
#include "Texture.h"
#include "DescriptorSetLayout.h"
#include "DescriptorSet.h"
#include "RenderPass.h"
#include "FrameBuffer.h"
#include "Pipeline.h"
#include "CommandBuffers.h"
#include "GuiRenderer.h"
#include "AccelerationStructure.h"
#include "RayTracingPipeline.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>
#include <tiny_gltf.h>

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

	CameraGPU m_camera;
	OptionsGPU m_options;

	// camera
	bool m_mousePressed = false;
	double m_lastMouseX = 0.0, m_lastMouseY = 0.0;
	float m_yaw = -90.0f;
	float m_pitch = 0.0f;
	float m_mouseSensitivity = 0.2f;
	float m_moveSpeed = 3.0f;

	std::vector<AreaLight> m_areaLights;
	std::vector<Object> m_objects;

	std::vector<InstanceGPU> m_instanceGPU;
	std::vector<AreaLightGPU> m_areaLightGPU;

	Scene m_scene;

	// models
	std::vector<std::unique_ptr<Mesh>> m_meshes;
	std::vector<std::unique_ptr<Texture>> m_textures;
	std::vector<MaterialGPU> m_materials;
	std::vector<Model> m_models;
	std::unordered_map<std::string, int32_t> m_texturePathMap;
	std::unordered_map<aiMaterial*, int32_t> m_materialMap;

	// descriptorset layout
	std::unique_ptr<DescriptorSetLayout> m_set0Layout;
	std::unique_ptr<DescriptorSetLayout> m_set1Layout;
	std::unique_ptr<DescriptorSetLayout> m_set2Layout;
	std::unique_ptr<DescriptorSetLayout> m_set3Layout;
	std::unique_ptr<DescriptorSetLayout> m_set4Layout;
	std::unique_ptr<DescriptorSetLayout> m_set5Layout;

	// buffers
	std::unique_ptr<UniformBuffer> m_cameraBuffer;
	std::unique_ptr<UniformBuffer> m_optionsBuffer;
	std::unique_ptr<StorageBuffer> m_materialBuffer;
	std::unique_ptr<StorageBuffer> m_instanceBuffer;
	std::unique_ptr<StorageBuffer> m_areaLightBuffer;

	// acceleration structure
	std::vector<std::unique_ptr<BottomLevelAS>> m_blas;
	std::unique_ptr<TopLevelAS> m_tlas;

	// pipeline
	std::unique_ptr<RayTracingPipeline> m_ptPipeline;

	// descriptor set
	std::unique_ptr<DescriptorSet> m_set0DescSet;
	std::unique_ptr<DescriptorSet> m_set1DescSet;
	std::unique_ptr<DescriptorSet> m_set2DescSet;
	std::unique_ptr<DescriptorSet> m_set3DescSet;
	std::unique_ptr<DescriptorSet> m_set4DescSet;
	std::array<std::unique_ptr<DescriptorSet>, 2> m_set5DescSets;

	// command buffer
	std::unique_ptr<CommandBuffers> m_commandBuffers;

	// texture
	std::unique_ptr<Texture> m_outputTexture;
	std::unique_ptr<Texture> m_accum0Texture;
	std::unique_ptr<Texture> m_accum1Texture;

	// gui renderer
	std::unique_ptr<GuiRenderer> m_guiRenderer;
	std::vector<std::unique_ptr<FrameBuffer>> m_imguiFrameBuffers;
	std::unique_ptr<RenderPass> m_imguiRenderPass;

	void cleanup();
	void init(GLFWwindow* window);
	void recreateSwapChain();
	void recreateViewport(ImVec2 newExtent);
	void transferImageLayout(VkCommandBuffer cmd, Texture* texture, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, uint32_t layerCount = 1);


	// record command buffer
	void recordImGuiCommandBuffer(uint32_t imageIndex, float deltaTime);
	void recordPathTracingCommandBuffer();


	// model loading
	void updateAssets();

	// legacy
	// void loadGLTFModel(const std::string& path);
	// void processNode(aiNode* node, const aiScene* scene, const std::filesystem::path& basePath, Model& model, std::unordered_map<aiMaterial*, int32_t>& materialMap);
	// std::unique_ptr<Mesh> processMesh(aiMesh* mesh);
	// MaterialGPU processMaterial(aiMaterial* aiMat, const aiScene* scene, const std::filesystem::path& basePath);
	// int32_t loadTexture(const aiScene* scene, aiMaterial* aiMat, aiTextureType type, const std::filesystem::path& basePath, TextureFormatType formatType);

	// tiny_gltf
	void loadTinyGLTFModel(const std::string& path);
	void processTinyNode(int nodeIndex, const tinygltf::Model& gltfModel, const std::filesystem::path& basePath, Model& model, std::unordered_map<int, int32_t>& materialMap);
	std::unique_ptr<Mesh> processTinyPrimitive(const tinygltf::Primitive& primitive, const tinygltf::Model& model);
	MaterialGPU processTinyMaterial(int materialIndex, const tinygltf::Model& model, const std::filesystem::path& basePath);
	int32_t loadTinyTexture(int textureIndex, const tinygltf::Model& model, const std::filesystem::path& basePath, TextureFormatType formatType);

	// scene
	void createScene();

	void uploadSceneToGPU();

	// debug
	void printAllModelInfo();
	void printAllInstanceInfo();
	void printAllAreaLightInfo();
};