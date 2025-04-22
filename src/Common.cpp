#include "include/Common.h"

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}


void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}


void printMaterial(const Material& mat) {
	std::cout << "Material Info:\n";
	std::cout << "  Albedo Tex Index    : " << mat.albedoTexIndex << "\n";
	std::cout << "  Normal Tex Index    : " << mat.normalTexIndex << "\n";
	std::cout << "  Metallic Tex Index  : " << mat.metallicTexIndex << "\n";
	std::cout << "  Roughness Tex Index : " << mat.roughnessTexIndex << "\n";
	std::cout << "  AO Tex Index        : " << mat.aoTexIndex << "\n";
	std::cout << "  Emissive Tex Index  : " << mat.emissiveTexIndex << "\n";
	std::cout << "  Base Color          : (" << mat.baseColor.r << ", " << mat.baseColor.g
		<< ", " << mat.baseColor.b << ", " << mat.baseColor.a << ")\n";
	std::cout << "  Metallic Factor     : " << mat.metallic << "\n";
	std::cout << "  Roughness Factor    : " << mat.roughness << "\n";
	std::cout << "  AO Factor           : " << mat.ao << "\n";
	std::cout << "  Emissive Factor     : (" << mat.emissiveFactor.r << ", "
		<< mat.emissiveFactor.g << ", " << mat.emissiveFactor.b << ")\n";
}