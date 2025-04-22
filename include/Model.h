#pragma once

#include "Common.h"
#include "Mesh.h"

struct Model {
	std::vector<int32_t> mesh;
	std::vector<int32_t> material;
};

void printModel(const Model& model);