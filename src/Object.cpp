#include "include/Object.h"
#include <iomanip>


void printObject(const Object& obj) {
	std::cout << "=== Object ===\n";
	std::cout << "Model Index         : " << obj.modelIndex << "\n";
	std::cout << "Position            : (" << obj.position.x << ", " << obj.position.y << ", " << obj.position.z << ")\n";
	std::cout << "Rotation            : (" << obj.rotation.x << ", " << obj.rotation.y << ", " << obj.rotation.z << ")\n";
	std::cout << "Scale               : (" << obj.scale.x << ", " << obj.scale.y << ", " << obj.scale.z << ")\n";
	std::cout << "Override Materials  : ";
	if (obj.overrideMaterialIndex.empty()) {
		std::cout << "(none)";
	}
	else {
		for (const auto& index : obj.overrideMaterialIndex) {
			std::cout << index << " ";
		}
	}
	std::cout << "\n";
}