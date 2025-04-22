#pragma once

#include "Common.h"
#include "Model.h"


struct Object {
	int32_t modelIndex = -1;
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
	std::vector<int32_t> overrideMaterialIndex;
};

void printObject(const Object& obj);