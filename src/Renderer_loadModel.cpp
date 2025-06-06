#include "include/Renderer.h"
#define TINYGLTF_IMPLEMENTATION
// #define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>


// void Renderer::loadGLTFModel(const std::string& path) {
//     Assimp::Importer importer;
//     const aiScene* scene = importer.ReadFile(path,
//         aiProcess_Triangulate |
//         aiProcess_FlipUVs |
//         // aiProcess_CalcTangentSpace |
//         aiProcess_JoinIdenticalVertices);

//     if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
//         throw std::runtime_error("Failed to load glTF: " + path);

//     std::filesystem::path basePath = std::filesystem::path(path).parent_path();

// 	std::unordered_map<aiMaterial*, int32_t> materialMap;

//     Model model;
// 	model.name = std::filesystem::path(path).filename().string();
//     processNode(scene->mRootNode, scene, basePath, model, materialMap);
//     m_models.push_back(model);
// }

// void Renderer::processNode(aiNode* node, const aiScene* scene, const std::filesystem::path& basePath, Model& model, std::unordered_map<aiMaterial*, int32_t>& materialMap) {
    
// 	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
//         aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
//         auto newMesh = processMesh(mesh);
//         int32_t meshIndex = static_cast<int32_t>(m_meshes.size());
//         m_meshes.push_back(std::move(newMesh));
//         model.mesh.push_back(meshIndex);

//         aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];
// 		auto it = materialMap.find(aiMat);
// 		int32_t materialIndex;
// 		if (it == materialMap.end()) {
// 			auto mat = processMaterial(aiMat, scene, basePath);
// 			materialIndex = static_cast<int32_t>(m_materials.size());
// 			m_materials.push_back(mat);
// 			materialMap[aiMat] = materialIndex;
// 		}
// 		else {
// 			materialIndex = it->second;
// 		}
//         model.material.push_back(materialIndex);
//     }

//     for (unsigned int i = 0; i < node->mNumChildren; ++i) {
//         processNode(node->mChildren[i], scene, basePath, model, materialMap);
//     }
// }


// std::unique_ptr<Mesh> Renderer::processMesh(aiMesh* mesh) {
//     std::vector<Vertex> vertices;
// 	std::vector<uint32_t> indices;

// 	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
// 	{
// 		Vertex vertex{};
// 		vertex.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
// 		vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

// 		if (mesh->HasTextureCoords(0))
// 			vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
// 		else
// 			vertex.texCoord = glm::vec2(0.0f);

// 		if (mesh->HasTangentsAndBitangents()) {
// 			glm::vec3 tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
// 			glm::vec3 bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
// 			glm::vec3 normal = vertex.normal;

// 			float w = glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f ? -1.0f : 1.0f;
// 			vertex.tangent = glm::vec4(tangent, w);
// 		}
// 		else {
// 			vertex.tangent = glm::vec4(0.0f); // fallback
// 		}

// 		vertices.push_back(vertex);
// 	}

// 	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
// 	{
// 		const aiFace& face = mesh->mFaces[i];
// 		for (unsigned int j = 0; j < face.mNumIndices; ++j)
// 			indices.push_back(face.mIndices[j]);
// 	}

// 	return Mesh::createMesh(m_context.get(), vertices, indices);
// }

// MaterialGPU Renderer::processMaterial(aiMaterial* aiMat, const aiScene* scene, const std::filesystem::path& basePath) {
//     MaterialGPU mat{};

//     mat.albedoTexIndex = loadTexture(scene, aiMat, aiTextureType_BASE_COLOR, basePath, TextureFormatType::ColorSRGB);
//     mat.normalTexIndex = loadTexture(scene, aiMat, aiTextureType_NORMALS, basePath, TextureFormatType::LinearUNORM);
//     mat.metallicTexIndex = loadTexture(scene, aiMat, aiTextureType_METALNESS, basePath, TextureFormatType::LinearUNORM);
//     mat.roughnessTexIndex = loadTexture(scene, aiMat, aiTextureType_DIFFUSE_ROUGHNESS, basePath, TextureFormatType::LinearUNORM);
//     mat.aoTexIndex = loadTexture(scene, aiMat, aiTextureType_AMBIENT_OCCLUSION, basePath, TextureFormatType::LinearUNORM);
//     mat.emissiveTexIndex = loadTexture(scene, aiMat, aiTextureType_EMISSIVE, basePath, TextureFormatType::ColorSRGB);

//     float metallic = 0.0f, roughness = 0.5f;
//     aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
//     aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
//     mat.metallic = metallic;
//     mat.roughness = roughness;

//     aiColor4D baseColor;
//     if (aiMat->Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS)
//         mat.baseColor = glm::vec4(baseColor.r, baseColor.g, baseColor.b, baseColor.a);

//     aiColor3D emissive;
//     if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS)
//         mat.emissiveFactor = glm::vec3(emissive.r, emissive.g, emissive.b);

//     return mat;
// }

// int32_t Renderer::loadTexture(const aiScene* scene, aiMaterial* aiMat, aiTextureType type, const std::filesystem::path& basePath, TextureFormatType formatType) {
//     if (aiMat->GetTextureCount(type) > 0)
// 	{
// 		aiString texPath;
// 		if (aiMat->GetTexture(type, 0, &texPath) == AI_SUCCESS)
// 		{
// 			std::string texPathStr = texPath.C_Str();
			
// 			std::filesystem::path fullPath = basePath / texPath.C_Str();
// 			std::string pathStr = fullPath.string();
			
// 			auto it = m_texturePathMap.find(pathStr);
// 			if (it != m_texturePathMap.end()) {
// 				return it->second;
// 			}

// 			std::unique_ptr<Texture> texture = nullptr;
// 			if (texPathStr[0] == '*') {
// 				int index = std::stoi(texPathStr.substr(1));

// 				const aiTexture* embeddedTex = scene->mTextures[index];
// 				if (!embeddedTex) {
// 					std::cerr << "Failed to get embedded texture: " << texPathStr << std::endl;
// 					return -1;
// 				}
// 				std::cout << "glb texture" << pathStr << std::endl;
// 				texture = Texture::createTextureFromMemory(m_context.get(), embeddedTex, formatType);
// 			}
// 			else {
// 				texture = Texture::createTexture(m_context.get(), fullPath.string(), formatType);

// 			}
// 			m_textures.push_back(std::move(texture));
// 			m_texturePathMap[pathStr] = static_cast<int32_t>(m_textures.size()) - 1;
// 			return static_cast<int32_t>(m_textures.size()) - 1;
// 		}
// 	}
// 	return -1;
// }


void Renderer::printAllModelInfo() {
	//check parsing
	int i = 0;
	std::cout << "--------------------------------" << std::endl;
	std::cout << "<<<<<<<<< model count : " << m_models.size() << " >>>>>>>>>" << std::endl;
	for (auto& model : m_models) {
		std::cout << "model: " << i << std::endl;
		// mesh 랑 material 묶어서 보여주기
		std::cout << "mesh: " << std::endl;
		for (auto& mesh : model.mesh) {
			std::cout << " " << mesh;
		}
		std::cout << std::endl;
		std::cout << "material: " << std::endl;
		for (auto& material : model.material) {
			std::cout << " " << material;
		}
		std::cout << std::endl;
		std::cout << "--------------------------------" << std::endl;
		i++;
	}
	std::cout << "<<<<<<<<< material count : " << m_materials.size() << " >>>>>>>>>" << std::endl;
	i = 0;
	for (auto& material : m_materials) {
		std::cout << "material idx : " << i << std::endl;
		std::cout << "albedoTexIndex: " << material.albedoTexIndex << std::endl;
		std::cout << "normalTexIndex: " << material.normalTexIndex << std::endl;
		std::cout << "metallicTexIndex: " << material.metallicTexIndex << std::endl;
		std::cout << "roughnessTexIndex: " << material.roughnessTexIndex << std::endl;
		std::cout << "aoTexIndex: " << material.aoTexIndex << std::endl;
		std::cout << "emissiveTexIndex: " << material.emissiveTexIndex << std::endl;
		std::cout << "baseColor: " << material.baseColor.r << ", " << material.baseColor.g << ", " << material.baseColor.b << ", " << material.baseColor.a << std::endl;
		std::cout << "emissiveFactor: " << material.emissiveFactor.r << ", " << material.emissiveFactor.g << ", " << material.emissiveFactor.b << std::endl;
		std::cout << "metallic: " << material.metallic << std::endl;
		std::cout << "roughness: " << material.roughness << std::endl;
		std::cout << "ao: " << material.ao << std::endl;
		std::cout << "--------------------------------" << std::endl;
		i++;
	}
}

void Renderer::printAllInstanceInfo() {
	std::cout << "m_instanceGPU.size() : " << m_instanceGPU.size() << std::endl;

	for (auto& instance : m_instanceGPU) {
		std::cout << "instance.transform : " << instance.transform[0][0] << " " << instance.transform[0][1] << " " << instance.transform[0][2] << " " << instance.transform[0][3] << std::endl;
		std::cout << "instance.transform : " << instance.transform[1][0] << " " << instance.transform[1][1] << " " << instance.transform[1][2] << " " << instance.transform[1][3] << std::endl;
		std::cout << "instance.transform : " << instance.transform[2][0] << " " << instance.transform[2][1] << " " << instance.transform[2][2] << " " << instance.transform[2][3] << std::endl;
		std::cout << "instance.transform : " << instance.transform[3][0] << " " << instance.transform[3][1] << " " << instance.transform[3][2] << " " << instance.transform[3][3] << std::endl;

		std::cout << "instance.vertexAddress : " << instance.vertexAddress << std::endl;
		std::cout << "instance.indexAddress : " << instance.indexAddress << std::endl;
		std::cout << "instance.materialIndex : " << instance.materialIndex << std::endl;
		std::cout << "instance.lightIndex : " << instance.lightIndex << std::endl;
		std::cout << "--------------------------------" << std::endl;
	}
}

void Renderer::printAllAreaLightInfo() {
	std::cout << "m_areaLightGPU.size() : " << m_areaLightGPU.size() << std::endl;
	for (auto& areaLight : m_areaLightGPU) {
		std::cout << "areaLight.color : " << areaLight.color.x << " " << areaLight.color.y << " " << areaLight.color.z << std::endl;
		std::cout << "areaLight.intensity : " << areaLight.intensity << std::endl;
		std::cout << "areaLight.area : " << areaLight.area << std::endl;

		std::cout << "areaLight.p0 : " << areaLight.p0.r << " " << areaLight.p0.g << " " << areaLight.p0.b << std::endl;
		std::cout << "areaLight.p1 : " << areaLight.p1.r << " " << areaLight.p1.g << " " << areaLight.p1.b << std::endl;
		std::cout << "areaLight.p2 : " << areaLight.p2.r << " " << areaLight.p2.g << " " << areaLight.p2.b << std::endl;
		std::cout << "areaLight.p3 : " << areaLight.p3.r << " " << areaLight.p3.g << " " << areaLight.p3.b << std::endl;

		std::cout << "areaLight.normal : " << areaLight.normal.r << " " << areaLight.normal.g << " " << areaLight.normal.b << std::endl;

		std::cout << "--------------------------------" << std::endl;
	}
}


void Renderer::loadTinyGLTFModel(const std::string& path) {
    tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	std::string err, warn;

	std::filesystem::path fsPath(path);
    std::string ext = fsPath.extension().string();

    bool ret = false;
    if (ext == ".glb") {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);
    } else {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
    }



	// bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path); // .gltf
	// 또는
	// bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path); // .glb

	if (!ret)
		throw std::runtime_error("Failed to load glTF: " + path);

	std::filesystem::path basePath = std::filesystem::path(path).parent_path();

    // 새로운 Model 객체 준비
    Model newModel;
    newModel.name = std::filesystem::path(path).filename().string();

    // 중복 머티리얼 체크용 맵
    std::unordered_map<int, int32_t> materialMap;

    // 루트 노드부터 재귀 순회
    for (int i = 0; i < model.scenes[model.defaultScene].nodes.size(); ++i) {
        int nodeIndex = model.scenes[model.defaultScene].nodes[i];
        processTinyNode(nodeIndex, model, basePath, newModel, materialMap);
    }
	m_models.push_back(newModel);
}

void Renderer::processTinyNode(
	int nodeIndex,
	const tinygltf::Model& gltfModel,
	const std::filesystem::path& basePath,
	Model& model,
	std::unordered_map<int, int32_t>& materialMap)
{
	const auto& node = gltfModel.nodes[nodeIndex];

	if (node.mesh >= 0) {
		const auto& mesh = gltfModel.meshes[node.mesh];

		// glTF는 하나의 mesh에 여러 primitive가 있을 수 있음
		for (const auto& prim : mesh.primitives) {
			int32_t meshIndex = static_cast<int32_t>(m_meshes.size());
			auto newMesh = processTinyPrimitive(prim, gltfModel);
			m_meshes.push_back(std::move(newMesh));
			model.mesh.push_back(meshIndex);

			int materialIndex;
			auto it = materialMap.find(prim.material);
			if (it == materialMap.end()) {
				auto mat = processTinyMaterial(prim.material, gltfModel, basePath);
				materialIndex = static_cast<int32_t>(m_materials.size());
				m_materials.push_back(mat);
				materialMap[prim.material] = materialIndex;
			}
			else {
				materialIndex = it->second;
			}
			model.material.push_back(materialIndex);
		}
	}

	for (int child : node.children) {
		processTinyNode(child, gltfModel, basePath, model, materialMap);
	}
}

std::unique_ptr<Mesh> Renderer::processTinyPrimitive(
	const tinygltf::Primitive& primitive,
	const tinygltf::Model& model)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	const auto& posAccessor = model.accessors.at(primitive.attributes.at("POSITION"));
	const auto& posBufferView = model.bufferViews[posAccessor.bufferView];
	const auto& posBuffer = model.buffers[posBufferView.buffer];

	const float* posData = reinterpret_cast<const float*>(
		posBuffer.data.data() + posBufferView.byteOffset + posAccessor.byteOffset);

	// NORMAL (optional)
	const float* normData = nullptr;
	if (primitive.attributes.count("NORMAL")) {
		const auto& normAccessor = model.accessors.at(primitive.attributes.at("NORMAL"));
		const auto& normView = model.bufferViews[normAccessor.bufferView];
		const auto& normBuffer = model.buffers[normView.buffer];
		normData = reinterpret_cast<const float*>(normBuffer.data.data() + normView.byteOffset + normAccessor.byteOffset);
	}

	// TEXCOORD_0 (optional)
	const float* uvData = nullptr;
	if (primitive.attributes.count("TEXCOORD_0")) {
		const auto& uvAccessor = model.accessors.at(primitive.attributes.at("TEXCOORD_0"));
		const auto& uvView = model.bufferViews[uvAccessor.bufferView];
		const auto& uvBuffer = model.buffers[uvView.buffer];
		uvData = reinterpret_cast<const float*>(uvBuffer.data.data() + uvView.byteOffset + uvAccessor.byteOffset);
	}

	// TANGENT (optional)
	const float* tangentData = nullptr;
	bool hasTangent = false;

	if (primitive.attributes.count("TANGENT")) {
		const auto& tanAccessor = model.accessors.at(primitive.attributes.at("TANGENT"));
		const auto& tanView = model.bufferViews[tanAccessor.bufferView];
		const auto& tanBuffer = model.buffers[tanView.buffer];
		tangentData = reinterpret_cast<const float*>(tanBuffer.data.data() + tanView.byteOffset + tanAccessor.byteOffset);
		hasTangent = true;
	}

	for (size_t i = 0; i < posAccessor.count; ++i) {
		Vertex v{};
		v.pos = glm::make_vec3(posData + i * 3);
		v.normal = normData ? glm::make_vec3(normData + i * 3) : glm::vec3(0);
		v.texCoord = uvData ? glm::make_vec2(uvData + i * 2) : glm::vec2(0);
		if (hasTangent) {
			v.tangent = glm::make_vec4(tangentData + i * 4); // vec4(tangent.xyz, handedness)
		} else {
			v.tangent = glm::vec4(0.0f); // fallback
		}
		vertices.push_back(v);
	}

	// Index 처리
	const auto& indexAccessor = model.accessors[primitive.indices];
	const auto& indexView = model.bufferViews[indexAccessor.bufferView];
	const auto& indexBuffer = model.buffers[indexView.buffer];
	const unsigned char* data = indexBuffer.data.data() + indexView.byteOffset + indexAccessor.byteOffset;

	for (size_t i = 0; i < indexAccessor.count; ++i) {
		if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
			indices.push_back(((uint16_t*)data)[i]);
		}
		else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
			indices.push_back(((uint32_t*)data)[i]);
		}
		else {
			throw std::runtime_error("Unsupported index component type");
		}
	}

	return Mesh::createMesh(m_context.get(), vertices, indices, hasTangent);
}


MaterialGPU Renderer::processTinyMaterial(int materialIndex, const tinygltf::Model& model, const std::filesystem::path& basePath) {
	MaterialGPU mat{};

	if (materialIndex < 0 || materialIndex >= model.materials.size())
		return mat;

	const auto& material = model.materials[materialIndex];

	// baseColorFactor
	if (material.pbrMetallicRoughness.baseColorFactor.size() == 4) {
		mat.baseColor = glm::vec4(
			static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[0]),
			static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[1]),
			static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[2]),
			static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[3])
		);
	}

	// emissiveFactor
	if (material.emissiveFactor.size() == 3) {
		mat.emissiveFactor = glm::vec3(
			static_cast<float>(material.emissiveFactor[0]),
			static_cast<float>(material.emissiveFactor[1]),
			static_cast<float>(material.emissiveFactor[2])
		);
	}

	// occlusionTexture.strength
	if (material.occlusionTexture.strength > 0.0) {
		mat.ao = static_cast<float>(material.occlusionTexture.strength);
	}

	// transmissionFactor (KHR_materials_transmission)
	auto extTransmission = material.extensions.find("KHR_materials_transmission");
	if (extTransmission != material.extensions.end()) {
		const auto& transmission = extTransmission->second;
		if (transmission.Has("transmissionFactor")) {
			mat.transmissionFactor = static_cast<float>(transmission.Get("transmissionFactor").Get<double>());
		}
	}

	// ior (KHR_materials_ior)
	auto extIor = material.extensions.find("KHR_materials_ior");
	if (extIor != material.extensions.end()) {
		const auto& ior = extIor->second;
		if (ior.Has("ior")) {
			mat.ior = static_cast<float>(ior.Get("ior").Get<double>());
		}
	}


	mat.doubleSided = material.doubleSided ? 1 : 0;

	mat.roughness = static_cast<float>(material.pbrMetallicRoughness.roughnessFactor);
	mat.metallic = static_cast<float>(material.pbrMetallicRoughness.metallicFactor);


	// 텍스처 인덱스만 로드 (속성은 무시)
	mat.albedoTexIndex = loadTinyTexture(material.pbrMetallicRoughness.baseColorTexture.index, model, basePath, TextureFormatType::ColorSRGB);
	mat.normalTexIndex = loadTinyTexture(material.normalTexture.index, model, basePath, TextureFormatType::LinearUNORM);
	mat.metallicTexIndex = loadTinyTexture(material.pbrMetallicRoughness.metallicRoughnessTexture.index, model, basePath, TextureFormatType::LinearUNORM);
	mat.roughnessTexIndex = mat.metallicTexIndex;
	mat.aoTexIndex = loadTinyTexture(material.occlusionTexture.index, model, basePath, TextureFormatType::LinearUNORM);
	mat.emissiveTexIndex = loadTinyTexture(material.emissiveTexture.index, model, basePath, TextureFormatType::ColorSRGB);

	return mat;
}

int32_t Renderer::loadTinyTexture(int textureIndex, const tinygltf::Model& model, const std::filesystem::path& basePath, TextureFormatType formatType) {
	if (textureIndex < 0 || textureIndex >= model.textures.size()) return -1;

	const auto& tex = model.textures[textureIndex];
	const auto& image = model.images[tex.source];

	std::string pathStr;

	if (!image.uri.empty()) {
		std::filesystem::path fullPath = basePath / image.uri;
		pathStr = fullPath.string();
	}
	else {
		pathStr = "embedded_" + std::to_string(tex.source);
	}

	auto it = m_texturePathMap.find(pathStr);
	if (it != m_texturePathMap.end()) {
		return it->second;
	}

	std::unique_ptr<Texture> texture;
	if (image.uri.empty()) {
		std::cout << "createTextureFromMemory" << std::endl;
		texture = Texture::createTextureFromMemory(m_context.get(), image, formatType);
	}
	else {
		texture = Texture::createTexture(m_context.get(), pathStr, formatType);
	}

	m_textures.push_back(std::move(texture));
	m_texturePathMap[pathStr] = static_cast<int32_t>(m_textures.size()) - 1;
	return static_cast<int32_t>(m_textures.size()) - 1;
}
