#include "include/Model.h"

void printModel(const Model& model) {
	std::cout << "Model Info\n";
	std::cout << "Meshes:\n";
	for (auto meshID : model.mesh)
		std::cout << "  Mesh ID: " << meshID << "\n";

	std::cout << "Materials:\n";
	for (auto matID : model.material)
		std::cout << "  Material ID: " << matID << "\n";
}