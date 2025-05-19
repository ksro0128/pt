#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>
#include "imgui_internal.h"


#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>
#include <cmath>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <functional>
#include <iomanip>


const int MAX_FRAMES_IN_FLIGHT = 1;
constexpr uint32_t MAX_LIGHT_COUNT = 64;

constexpr uint32_t MAX_OBJECT_COUNT = 1000;
constexpr uint32_t MAX_MESH_COUNT = 10000;
constexpr uint32_t MAX_MATERIAL_COUNT = 512;
constexpr uint32_t MAX_TEXTURE_COUNT = 128;

// Ray Tracing Acceleration Structure
extern PFN_vkCreateAccelerationStructureKHR g_vkCreateAccelerationStructureKHR;
extern PFN_vkDestroyAccelerationStructureKHR g_vkDestroyAccelerationStructureKHR;
extern PFN_vkGetAccelerationStructureBuildSizesKHR g_vkGetAccelerationStructureBuildSizesKHR;
extern PFN_vkCmdBuildAccelerationStructuresKHR g_vkCmdBuildAccelerationStructuresKHR;
extern PFN_vkGetAccelerationStructureDeviceAddressKHR g_vkGetAccelerationStructureDeviceAddressKHR;

// Ray Tracing Pipeline
extern PFN_vkCreateRayTracingPipelinesKHR g_vkCreateRayTracingPipelinesKHR;
extern PFN_vkGetRayTracingShaderGroupHandlesKHR g_vkGetRayTracingShaderGroupHandlesKHR;
extern PFN_vkCmdTraceRaysKHR g_vkCmdTraceRaysKHR;

inline VkTransformMatrixKHR glmToVkTransform(const glm::mat4& mat) {
	VkTransformMatrixKHR out{};
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 4; ++j) {
			out.matrix[i][j] = mat[j][i];
		}
	}
	return out;
}

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};


const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
	VK_KHR_MAINTENANCE3_EXTENSION_NAME,
	VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
	VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
	VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
	VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
	VK_KHR_SPIRV_1_4_EXTENSION_NAME,
	VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
	VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

struct Vertex {
	// glm::vec3 pos;
	// glm::vec3 normal;
	// glm::vec2 texCoord;
	// glm::vec3 tangent;
	glm::vec3 pos;     float _pad0 = 0.0f;
	glm::vec3 normal;  float _pad1 = 0.0f;
	glm::vec2 texCoord; glm::vec2 _pad2 = glm::vec2(0.0f);
	glm::vec3 tangent; float _pad3 = 0.0f;

	// 정점 데이터가 전달되는 방법을 알려주는 구조체 반환하는 함수
	static VkVertexInputBindingDescription getBindingDescription() {
		// 파이프라인에 정점 데이터가 전달되는 방법을 알려주는 구조체
		VkVertexInputBindingDescription bindingDescription{};		
		bindingDescription.binding = 0;								// 버텍스 바인딩 포인트 (현재 0번에 vertex 정보 바인딩)
		bindingDescription.stride = sizeof(Vertex);					// 버텍스 1개 단위의 정보 크기
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // 정점 데이터 처리 방법
																	// 1. VK_VERTEX_INPUT_RATE_VERTEX : 정점별로 데이터 처리
																	// 2. VK_VERTEX_INPUT_RATE_INSTANCE : 인스턴스별로 데이터 처리
		return bindingDescription;
	}

	// 정점 속성별 데이터 형식과 위치를 지정하는 구조체 반환하는 함수
	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
		// 정점 속성의 데이터 형식과 위치를 지정하는 구조체
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

		// pos 속성 정보 입력
		attributeDescriptions[0].binding = 0;							// 버텍스 버퍼의 바인딩 포인트
		attributeDescriptions[0].location = 0;							// 버텍스 셰이더의 어떤 location에 대응되는지 지정
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;	// 저장되는 데이터 형식 (VK_FORMAT_R32G32B32_SFLOAT = vec3)
		attributeDescriptions[0].offset = offsetof(Vertex, pos);		// 버텍스 구조체에서 해당 속성이 시작되는 위치

		// color 속성 정보 입력
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		// texCoord 속성 정보 입력
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		// tangent 속성 정보 입력
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, tangent);

		return attributeDescriptions;
	}
};


struct Model {
	std::string name = "";
    std::vector<int> mesh;
    std::vector<int> material;
};

struct alignas(16) MaterialGPU {
	glm::vec4 baseColor = glm::vec4(1.0f);
	glm::vec3 emissiveFactor = glm::vec3(0.0f);
	float roughness = 0.5f;
	float metallic = 0.04f;
	float ao = 1.0f;

	int albedoTexIndex = -1;
	int normalTexIndex = -1;
	int metallicTexIndex = -1;
	int roughnessTexIndex = -1;
	int aoTexIndex = -1;
	int emissiveTexIndex = -1;
};

struct ObjectInstance {
	int modelIndex = -1;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    int overrideMaterial = -1;
};

struct alignas(16) CameraGPU {
	glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 10.0f);
	float pad0 = 0.0f;
	glm::vec3 camDir = glm::vec3(0.0f, 0.0f, -1.0f);
	float pad1 = 0.0f;
	glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
	float pad2 = 0.0f;
	glm::vec3 camRight = glm::vec3(1.0f, 0.0f, 0.0f);
	float fovY = 50.0f;
};

struct alignas(16) OptionsGPU {
	int frameCount = 0;
	int maxSpp = 99999;
	int currentSpp = 0;
	int lightCount = 0;
};

struct AreaLight {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	glm::vec3 color = glm::vec3(1.0f);
	float intensity = 10.0f;

	float temperature = 6500.0f;
	bool useTemperature = false;
};

struct alignas(16) AreaLightGPU {
	glm::vec3 color = glm::vec3(1.0f);
	float intensity = 10.0f;

	glm::vec3 p0 = glm::vec3(0.0f);
	float area = 1.0f;
	glm::vec3 p1 = glm::vec3(0.0f);
	float pad0 = 0.0f;
	glm::vec3 p2 = glm::vec3(0.0f);
	float pad1 = 0.0f;
	glm::vec3 p3 = glm::vec3(0.0f);
	float pad2 = 0.0f;
	glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
	float pad3 = 0.0f;
};

struct Object {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	int modelIndex = -1;
	int overrideMaterialIndex = -1;
};

struct alignas(16) InstanceGPU {
	glm::mat4 transform = glm::mat4(1.0f);

	uint64_t vertexAddress = 0;
	uint64_t indexAddress = 0;

	int lightIndex = -1;
	int materialIndex = -1;
	int meshIndex = -1;
	float pad0 = 0.0f;
};

struct Scene {
	std::vector<Object> objects;
	std::vector<AreaLight> areaLights;
};