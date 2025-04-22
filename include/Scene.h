#pragma once

#include "Common.h"
#include "Object.h"



class Scene {
public:
	static std::unique_ptr<Scene> createScene(uint32_t maxModelIndex, uint32_t maxMaterialIndex, uint32_t maxTextureIndex);
	~Scene();

	glm::vec3& getAmbientColor() { return m_ambientColor; }
	std::vector<Light>& getLights() { return m_lights; }
	std::vector<Object>& getObjects() { return m_objects; }
	uint32_t& getMaxModelIndex() { return maxModelIndex; }
	uint32_t& getMaxMaterialIndex() { return maxMaterialIndex; }
	uint32_t& getMaxTextureIndex() { return maxTextureIndex; }

private:
	glm::vec3 m_ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
	std::vector<Light> m_lights;
	std::vector<Object> m_objects;

	uint32_t maxModelIndex = 0;
	uint32_t maxMaterialIndex = 0;
	uint32_t maxTextureIndex = 0;

	void init(uint32_t maxModelIndex, uint32_t maxMaterialIndex, uint32_t maxTextureIndex);
};