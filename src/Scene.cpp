#include "include/Scene.h"

std::unique_ptr<Scene> Scene::createScene(uint32_t maxModelIndex, uint32_t maxMaterialIndex, uint32_t maxTextureIndex) {
	std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new Scene());
	scene->init(maxModelIndex, maxMaterialIndex, maxTextureIndex);
	return scene;
}

Scene::~Scene() {
}

void Scene::init(uint32_t maxModelIndex, uint32_t maxMaterialIndex, uint32_t maxTextureIndex) {
	this->maxModelIndex = maxModelIndex;
	this->maxMaterialIndex = maxMaterialIndex;
	this->maxTextureIndex = maxTextureIndex;

	m_ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
	{
		Light light;
		light.type = LIGHT_TYPE_DIRECTIONAL;
		light.castsShadow = true;
		light.direction = glm::vec4(0.0f, -1.0f, -0.5f, 0.0f);
		light.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		light.intensity = 3.0f;
		light.range = 100.0f;
		light.position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		light.spotInnerAngle = 30.0f;
		light.spotOuterAngle = 45.0f;
		m_lights.push_back(light);
	}
	// 0 - plane
	// 1 - cube
	// 2 - sphere


	/*{
		Object obj;
		obj.modelIndex = 2;
		obj.position = glm::vec3(-1.5f, 0.0f, 0.0f);
		obj.overrideMaterialIndex = { 4 };
		m_objects.push_back(obj);
	}*/
	{
		Object obj;
		obj.modelIndex = 2;
		obj.position = glm::vec3(0.0f, 0.0f, 0.0f);
		obj.overrideMaterialIndex = { 4 };
		m_objects.push_back(obj);
	}
	/*{
		Object obj;
		obj.modelIndex = 2;
		obj.position = glm::vec3(1.5f, 0.0f, 0.0f);
		obj.overrideMaterialIndex = { 6 };
		m_objects.push_back(obj);
	}*/
	
	{
		Object obj;
		obj.modelIndex = 0;
		obj.position = glm::vec3(0.0f, -2.5f, 0.0f);
		obj.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
		obj.scale = glm::vec3(5.0f, 5.0f, 1.0f);
		m_objects.push_back(obj);
	}
	{
		Object obj;
		obj.modelIndex = 0;
		obj.position = glm::vec3(0.0f, 0.0f, -2.5f);
		obj.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		obj.scale = glm::vec3(5.0f, 5.0f, 1.0f);
		obj.overrideMaterialIndex = {1};
		m_objects.push_back(obj);
	}
	{
		Object obj;
		obj.modelIndex = 0;
		obj.position = glm::vec3(-2.5f, 0.0f, 0.0f);
		obj.rotation = glm::vec3(0.0f, 90.0f, 0.0f);
		obj.scale = glm::vec3(5.0f, 5.0f, 1.0f);
		obj.overrideMaterialIndex = {2};
		m_objects.push_back(obj);
	}
	{
		Object obj;
		obj.modelIndex = 0;
		obj.position = glm::vec3(2.5f, 0.0f, 0.0f);
		obj.rotation = glm::vec3(0.0f, -90.0f, 0.0f);
		obj.scale = glm::vec3(5.0f, 5.0f, 1.0f);
		obj.overrideMaterialIndex = {3};
		m_objects.push_back(obj);
	}


	// sponza + knight

	 /*{
	 	Object obj;
	 	obj.modelIndex = 3;
	 	obj.position = glm::vec3(0.0f, 0.0f, 0.0f);
	 	m_objects.push_back(obj);
	 }
	 {
	 	Object obj;
	 	obj.modelIndex = 4;
	 	obj.position = glm::vec3(0.0f, -4.0f, -3.0f);
	 	obj.rotation = glm::vec3(90.0f, 0.0f, 0.0f);
	 	obj.scale = glm::vec3(0.01f, 0.01f, 0.01f);
	 	m_objects.push_back(obj);
	 }
	 {
	 	Object obj;
	 	obj.modelIndex = 4;
	 	obj.position = glm::vec3(-1.5f, -4.0f, -3.0f);
	 	obj.rotation = glm::vec3(90.0f, 0.0f, 0.0f);
	 	obj.scale = glm::vec3(0.01f, 0.01f, 0.01f);
	 	m_objects.push_back(obj);
	 }*/
	

}