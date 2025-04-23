#include "include/Renderer.h"

std::unique_ptr<Renderer> Renderer::createRenderer(GLFWwindow* window) {
	std::unique_ptr<Renderer> renderer = std::unique_ptr<Renderer>(new Renderer());
	renderer->init(window);
	return renderer;
}

Renderer::~Renderer() {
	cleanup();
}

void Renderer::cleanup() {
	std::cout << "Renderer::cleanup" << std::endl;
	vkDeviceWaitIdle(m_context->getDevice());
}

void Renderer::init(GLFWwindow* window) {
	std::cout << "Renderer::init" << std::endl;
	this->window = window;
	m_context = VulkanContext::createVulkanContext(window);
	m_swapChain = SwapChain::createSwapChain(window, m_context.get());
	m_syncObjects = SyncObjects::createSyncObjects(m_context.get());
	m_commandBuffers = CommandBuffers::createCommandBuffers(m_context.get());
	m_extent = {1024, 1024};

	loadScene("assets/bathroom/bathroom.pbrt");

	std::cout << "mesh list size: " << m_meshList.size() << std::endl;
	std::cout << "shape list size: " << m_shapeList.size() << std::endl;

	m_blasList.resize(m_meshList.size());
	for (int i = 0; i < m_meshList.size(); i++) {
		m_blasList[i] = BottomLevelAS::createBottomLevelAS(m_context.get(), m_meshList[i].get());
	}
	m_emptyTLAS = TopLevelAS::createEmptyTopLevelAS(m_context.get());

	m_tlas.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_tlas[i] = TopLevelAS::createTopLevelAS(m_context.get(), m_blasList, m_shapeList);
	}


	m_imguiRenderPass = RenderPass::createImGuiRenderPass(m_context.get(), m_swapChain.get());
	m_imguiFrameBuffers.resize(m_swapChain->getSwapChainImages().size());
	for (int i = 0; i < m_swapChain->getSwapChainImages().size(); i++) {
		m_imguiFrameBuffers[i] = FrameBuffer::createImGuiFrameBuffer(m_context.get(), m_imguiRenderPass.get(), m_swapChain->getSwapChainImageViews()[i], m_swapChain->getSwapChainExtent());
	}
	m_guiRenderer = GuiRenderer::createGuiRenderer(m_context.get(), window, m_imguiRenderPass.get(), m_swapChain.get());
}

glm::mat4 toGlm(const minipbrt::Transform& t) {
    glm::mat4 result(1.0f);
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            result[j][i] = t.start[i][j]; // minipbrt는 col-major이므로 이 순서
    return result;
}

void Renderer::loadScene(std::string scenePath) {
	minipbrt::Loader loader;
	if (loader.load(scenePath.c_str()) == false) {
		std::cout << "Failed to load scene!" << std::endl;
		return;
	}

	m_pbrtScene = loader.take_scene();
	const auto* pbrtCam = static_cast<minipbrt::PerspectiveCamera*>(m_pbrtScene->camera);
	const glm::mat4 camToWorld = toGlm(pbrtCam->cameraToWorld);

	m_camera.camPos = glm::vec3(camToWorld[3]);
	m_camera.camDir = glm::normalize(glm::vec3(camToWorld[2]) * -1.0f);
	m_camera.camUp = glm::normalize(glm::vec3(camToWorld[1]));
	m_camera.camRight = glm::normalize(glm::vec3(camToWorld[0]));
	if (pbrtCam->type() == minipbrt::CameraType::Perspective) {
		m_camera.fovY = static_cast<minipbrt::PerspectiveCamera*>(m_pbrtScene->camera)->fov;
	}
	
	std::cout << "Camera Position: " << m_camera.camPos.x << ", " << m_camera.camPos.y << ", " << m_camera.camPos.z << std::endl;
	std::cout << "Camera Direction: " << m_camera.camDir.x << ", " << m_camera.camDir.y << ", " << m_camera.camDir.z << std::endl;
	std::cout << "Camera Up: " << m_camera.camUp.x << ", " << m_camera.camUp.y << ", " << m_camera.camUp.z << std::endl;
	std::cout << "Camera Right: " << m_camera.camRight.x << ", " << m_camera.camRight.y << ", " << m_camera.camRight.z << std::endl;
	std::cout << "Camera FOV: " << m_camera.fovY << std::endl;


	for (auto& mat : m_pbrtScene->materials) {

		MaterialGPU material;
		material.type = static_cast<int>(mat->type());
		m_materialNameMap[mat->name] = m_materialList.size();
		if (mat->type() == minipbrt::MaterialType::Uber) {
			std::cout << "Uber Material" << std::endl;
			material.index = m_uberList.size();
			m_materialList.push_back(material);

			const auto* uberMat = static_cast<const minipbrt::UberMaterial*>(mat);
			UberGPU uber;
			uber.Kd.r         = uberMat->Kd.value[0];
			uber.Kd.g         = uberMat->Kd.value[1];
			uber.Kd.b         = uberMat->Kd.value[2];
			uber.KdIdx      = static_cast<int>(uberMat->Kd.texture);

			uber.Ks.r         = uberMat->Ks.value[0];
			uber.Ks.g         = uberMat->Ks.value[1];
			uber.Ks.b         = uberMat->Ks.value[2];
			uber.KsIdx      = static_cast<int>(uberMat->Ks.texture);

			uber.Kr.r         = uberMat->Kr.value[0];
			uber.Kr.g         = uberMat->Kr.value[1];
			uber.Kr.b         = uberMat->Kr.value[2];
			uber.KrIdx      = static_cast<int>(uberMat->Kr.texture);

			uber.Kt.r         = uberMat->Kt.value[0];
			uber.Kt.g         = uberMat->Kt.value[1];
			uber.Kt.b         = uberMat->Kt.value[2];
			uber.KtIdx      = static_cast<int>(uberMat->Kt.texture);

			uber.opacity.r    = uberMat->opacity.value[0];
			uber.opacity.g    = uberMat->opacity.value[1];
			uber.opacity.b    = uberMat->opacity.value[2];
			uber.opacityIdx = static_cast<int>(uberMat->opacity.texture);

			uber.eta        = uberMat->eta.value;
			uber.etaIdx     = static_cast<int>(uberMat->eta.texture);

			uber.uroughness    = uberMat->uroughness.value;
			uber.uroughnessIdx = static_cast<int>(uberMat->uroughness.texture);

			uber.vroughness    = uberMat->vroughness.value;
			uber.vroughnessIdx = static_cast<int>(uberMat->vroughness.texture);

			uber.remaproughness = uberMat->remaproughness ? 1 : 0;
			m_uberList.push_back(uber);


			std::cout << "Uber Material Kd: " << uber.Kd.r << ", " << uber.Kd.g << ", " << uber.Kd.b << std::endl;
			std::cout << "Uber Material KdIdx: " << uber.KdIdx << std::endl;
			std::cout << "Uber Material Ks: " << uber.Ks.r << ", " << uber.Ks.g << ", " << uber.Ks.b << std::endl;
			std::cout << "Uber Material Kr: " << uber.Kr.r << ", " << uber.Kr.g << ", " << uber.Kr.b << std::endl;
			std::cout << "Uber Material Kt: " << uber.Kt.r << ", " << uber.Kt.g << ", " << uber.Kt.b << std::endl;
			std::cout << "Uber Material Opacity: " << uber.opacity.r << ", " << uber.opacity.g << ", " << uber.opacity.b << std::endl;
			std::cout << "Uber Material Eta: " << uber.eta << std::endl;
			std::cout << "Uber Material Uroughness: " << uber.uroughness << std::endl;
			std::cout << "Uber Material Vroughness: " << uber.vroughness << std::endl;
			std::cout << "Uber Material Remaproughness: " << uber.remaproughness << std::endl;
			std::cout << "-------------------------" << std::endl;
		}
		else if (mat->type() == minipbrt::MaterialType::Matte) {
			std::cout << "Matte Material" << std::endl;
			material.index = m_matteList.size();
			m_materialList.push_back(material);

			const auto* matteMat = static_cast<const minipbrt::MatteMaterial*>(mat);
			MatteGPU matte;
			matte.Kd.r = matteMat->Kd.value[0];
			matte.Kd.g = matteMat->Kd.value[1];
			matte.Kd.b = matteMat->Kd.value[2];
			matte.KdIdx = static_cast<int>(matteMat->Kd.texture);
			matte.sigma = matteMat->sigma.value;
			m_matteList.push_back(matte);

			std::cout << "Matte Material Kd: " << matte.Kd.r << ", " << matte.Kd.g << ", " << matte.Kd.b << std::endl;
			std::cout << "Matte Material Sigma: " << matte.sigma << std::endl;
			std::cout << "-------------------------" << std::endl;
		}
		else if (mat->type() == minipbrt::MaterialType::Metal) {
			std::cout << "Metal Material" << std::endl;
			material.index = m_metalList.size();
			m_materialList.push_back(material);

			const auto* metalMat = static_cast<const minipbrt::MetalMaterial*>(mat);
			MetalGPU metal;

			metal.eta.r = metalMat->eta.value[0];
			metal.eta.g = metalMat->eta.value[1];
			metal.eta.b = metalMat->eta.value[2];
			metal.etaIdx = static_cast<int>(metalMat->eta.texture);

			metal.k.r = metalMat->k.value[0];
			metal.k.g = metalMat->k.value[1];
			metal.k.b = metalMat->k.value[2];
			metal.kIdx = static_cast<int>(metalMat->k.texture);

			metal.uroughness = metalMat->uroughness.value;
			metal.uroughnessIdx = static_cast<int>(metalMat->uroughness.texture);

			metal.vroughness = metalMat->vroughness.value;
			metal.vroughnessIdx = static_cast<int>(metalMat->vroughness.texture);

			metal.remaproughness = metalMat->remaproughness ? 1 : 0;
			m_metalList.push_back(metal);

			std::cout << "Metal Material Eta: " << metal.eta.r << ", " << metal.eta.g << ", " << metal.eta.b << std::endl;
			std::cout << "Metal Material K: " << metal.k.r << ", " << metal.k.g << ", " << metal.k.b << std::endl;
			std::cout << "Metal Material Uroughness: " << metal.uroughness << std::endl;
			std::cout << "Metal Material Vroughness: " << metal.vroughness << std::endl;
			std::cout << "Metal Material Remaproughness: " << metal.remaproughness << std::endl;
			std::cout << "-------------------------" << std::endl;

		}
		else if (mat->type() == minipbrt::MaterialType::Glass) {
			std::cout << "Glass Material" << std::endl;
			material.index = m_glassList.size();
			m_materialList.push_back(material);

			const auto* glassMat = static_cast<const minipbrt::GlassMaterial*>(mat);
			GlassGPU glass;
			glass.Kr.r = glassMat->Kr.value[0];
			glass.Kr.g = glassMat->Kr.value[1];
			glass.Kr.b = glassMat->Kr.value[2];
			glass.KrIdx = static_cast<int>(glassMat->Kr.texture);

			glass.Kt.r = glassMat->Kt.value[0];
			glass.Kt.g = glassMat->Kt.value[1];
			glass.Kt.b = glassMat->Kt.value[2];
			glass.KtIdx = static_cast<int>(glassMat->Kt.texture);

			glass.eta = glassMat->eta.value;
			glass.etaIdx = static_cast<int>(glassMat->eta.texture);

			glass.uroughness = glassMat->uroughness.value;
			glass.uroughnessIdx = static_cast<int>(glassMat->uroughness.texture);

			glass.vroughness = glassMat->vroughness.value;
			glass.vroughnessIdx = static_cast<int>(glassMat->vroughness.texture);

			glass.remaproughness = glassMat->remaproughness ? 1 : 0;

			m_glassList.push_back(glass);

			std::cout << "Glass Material Kr: " << glass.Kr.r << ", " << glass.Kr.g << ", " << glass.Kr.b << std::endl;
			std::cout << "Glass Material Kt: " << glass.Kt.r << ", " << glass.Kt.g << ", " << glass.Kt.b << std::endl;
			std::cout << "Glass Material Eta: " << glass.eta << std::endl;
			std::cout << "Glass Material Uroughness: " << glass.uroughness << std::endl;
			std::cout << "Glass Material Vroughness: " << glass.vroughness << std::endl;
			std::cout << "Glass Material Remaproughness: " << glass.remaproughness << std::endl;
			std::cout << "-------------------------" << std::endl;
		}
		else if (mat->type() == minipbrt::MaterialType::Mirror) {
			std::cout << "Mirror Material" << std::endl;
			material.index = m_mirrorList.size();
			m_materialList.push_back(material);

			const auto* mirrorMat = static_cast<const minipbrt::MirrorMaterial*>(mat);

			MirrorGPU mirror;

			mirror.Kr.r = mirrorMat->Kr.value[0];
			mirror.Kr.g = mirrorMat->Kr.value[1];
			mirror.Kr.b = mirrorMat->Kr.value[2];
			mirror.KrIdx = static_cast<int>(mirrorMat->Kr.texture);

			m_mirrorList.push_back(mirror);
			
			std::cout << "Mirror Material Kr: " << mirror.Kr.r << ", " << mirror.Kr.g << ", " << mirror.Kr.b << std::endl;
			std::cout << "-------------------------" << std::endl;

		}
		else if (mat->type() == minipbrt::MaterialType::Substrate) {
			std::cout << "Substrate Material" << std::endl;
			material.index = m_substrateList.size();
			m_materialList.push_back(material);
			
			const auto* substrateMat = static_cast<const minipbrt::SubstrateMaterial*>(mat);
			SubstrateGPU substrate;
			substrate.Kd.r = substrateMat->Kd.value[0];
			substrate.Kd.g = substrateMat->Kd.value[1];
			substrate.Kd.b = substrateMat->Kd.value[2];
			substrate.KdIdx = static_cast<int>(substrateMat->Kd.texture);

			substrate.Ks.r = substrateMat->Ks.value[0];
			substrate.Ks.g = substrateMat->Ks.value[1];
			substrate.Ks.b = substrateMat->Ks.value[2];
			substrate.KsIdx = static_cast<int>(substrateMat->Ks.texture);

			substrate.uroughness = substrateMat->uroughness.value;
			substrate.uroughnessIdx = static_cast<int>(substrateMat->uroughness.texture);

			substrate.vroughness = substrateMat->vroughness.value;
			substrate.vroughnessIdx = static_cast<int>(substrateMat->vroughness.texture);

			substrate.remaproughness = substrateMat->remaproughness ? 1 : 0;
			m_substrateList.push_back(substrate);

			std::cout << "Substrate Material Kd: " << substrate.Kd.r << ", " << substrate.Kd.g << ", " << substrate.Kd.b << std::endl;
			std::cout << "Substrate Material KdIdx: " << substrate.KdIdx << std::endl;
			std::cout << "Substrate Material Ks: " << substrate.Ks.r << ", " << substrate.Ks.g << ", " << substrate.Ks.b << std::endl;
			std::cout << "Substrate Material Uroughness: " << substrate.uroughness << std::endl;
			std::cout << "Substrate Material Vroughness: " << substrate.vroughness << std::endl;
			std::cout << "Substrate Material Remaproughness: " << substrate.remaproughness << std::endl;
			std::cout << "-------------------------" << std::endl;

		}
		else if (mat->type() == minipbrt::MaterialType::Fourier) {
			std::cout << "Fourier Material -> Matte Material" << std::endl;
			material.index = m_matteList.size();
			m_materialList.push_back(material);

			MatteGPU matte;
			m_matteList.push_back(matte);

			std::cout << "Fourier Material Kd: " << matte.Kd.r << ", " << matte.Kd.g << ", " << matte.Kd.b << std::endl;
			std::cout << "Fourier Material Sigma: " << matte.sigma << std::endl;
			std::cout << "-------------------------" << std::endl;

		}
		else if (mat->type() == minipbrt::MaterialType::Plastic) {
			std::cout << "Plastic Material" << std::endl;
			material.index = m_plasticList.size();
			m_materialList.push_back(material);

			const auto* plasticMat = static_cast<const minipbrt::PlasticMaterial*>(mat);
			PlasticGPU plastic;

			plastic.Kd.r = plasticMat->Kd.value[0];
			plastic.Kd.g = plasticMat->Kd.value[1];
			plastic.Kd.b = plasticMat->Kd.value[2];
			plastic.KdIdx = static_cast<int>(plasticMat->Kd.texture);

			plastic.Ks.r = plasticMat->Ks.value[0];
			plastic.Ks.g = plasticMat->Ks.value[1];
			plastic.Ks.b = plasticMat->Ks.value[2];
			plastic.KsIdx = static_cast<int>(plasticMat->Ks.texture);

			plastic.roughness = plasticMat->roughness.value;
			plastic.roughnessIdx = static_cast<int>(plasticMat->roughness.texture);

			plastic.remaproughness = plasticMat->remaproughness ? 1 : 0;

			m_plasticList.push_back(plastic);

			std::cout << "Plastic Material Kd: " << plastic.Kd.r << ", " << plastic.Kd.g << ", " << plastic.Kd.b << std::endl;
			std::cout << "Plastic Material Ks: " << plastic.Ks.r << ", " << plastic.Ks.g << ", " << plastic.Ks.b << std::endl;
			std::cout << "Plastic Material Roughness: " << plastic.roughness << std::endl;
			std::cout << "Plastic Material Remaproughness: " << plastic.remaproughness << std::endl;
			std::cout << "-------------------------" << std::endl;
		}
		else {
			std::cout << "Unknown Material" << static_cast<int>(mat->type()) << std::endl;
		}
	}

	for (auto& tex : m_pbrtScene->textures) {
		std::cout << "texture name: " << tex->name << std::endl;
		m_textureNameMap[tex->name] = m_textureList.size();
		if (tex->type() == minipbrt::TextureType::ImageMap) {
			std::cout << "ImageMap Texture" << std::endl;
			const auto* imageMapTex = static_cast<const minipbrt::ImageMapTexture*>(tex);
			std::cout << "ImageMap Texture file: " << imageMapTex->filename << std::endl;
			auto texture = Texture::createTexture(m_context.get(), imageMapTex->filename, TextureFormatType::ColorSRGB);
			m_textureList.push_back(std::move(texture));
		}
		else if (tex->type() == minipbrt::TextureType::FBM) {
			std::cout << "FBM Texture" << std::endl;
			auto texture = Texture::createDefaultTexture(m_context.get(), glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
			m_textureList.push_back(std::move(texture));
		}
		else {
			std::cout << "Unknown Texture" << static_cast<int>(tex->type()) << std::endl;
			auto texture = Texture::createDefaultTexture(m_context.get(), glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
			m_textureList.push_back(std::move(texture));
		}
	}

	//cout texture name map
	for (auto& tex : m_textureNameMap) {
		std::cout << "Texture Name: " << tex.first << ", Index: " << tex.second << std::endl;
	}

	// area light
	for (auto& aL : m_pbrtScene->areaLights) {
		AreaLightGPU areaLight;
		areaLight.scale.r = aL->scale[0];
		areaLight.scale.g = aL->scale[1];
		areaLight.scale.b = aL->scale[2];

		auto diffuseAreaLight = static_cast<minipbrt::DiffuseAreaLight*>(aL);
		areaLight.L.r = diffuseAreaLight->L[0];
		areaLight.L.g = diffuseAreaLight->L[1];
		areaLight.L.b = diffuseAreaLight->L[2];
		areaLight.twosided = diffuseAreaLight->twosided ? 1 : 0;
		areaLight.samples = diffuseAreaLight->samples;
		m_areaLightList.push_back(areaLight);

		std::cout << "Area Light Scale: " << areaLight.scale.r << ", " << areaLight.scale.g << ", " << areaLight.scale.b << std::endl;
		std::cout << "Area Light L: " << areaLight.L.r << ", " << areaLight.L.g << ", " << areaLight.L.b << std::endl;
		std::cout << "Area Light Twosided: " << areaLight.twosided << std::endl;
		std::cout << "Area Light Samples: " << areaLight.samples << std::endl;
		std::cout << "-------------------------" << std::endl;
	}


	//shape
	for (auto& shape : m_pbrtScene->shapes) {
		if (shape->type() == minipbrt::ShapeType::PLYMesh) {
			std::cout << "PLYMesh Shape" << std::endl;

			ShapeGPU s;
			s.modelMatrix = toGlm(shape->shapeToWorld);
			s.materialIdx = shape->material;
			s.areaLightIdx = shape->areaLight;
			s.reverseOrientation = shape->reverseOrientation ? 1 : 0;

			auto plyMesh = static_cast<minipbrt::PLYMesh*>(shape);
			s.alphaIdx = plyMesh->alpha;
			s.shadowAlphaIdx = plyMesh->shadowalpha;
			
			std::cout << "PLYMesh Shape file: " << plyMesh->filename << std::endl;
			std::cout << "can load ply mesh: " << plyMesh->can_convert_to_triangle_mesh() << std::endl;
			if (!plyMesh->can_convert_to_triangle_mesh()) {
				std::cout << "Cannot convert to triangle mesh" << std::endl;
				continue;
			}
			auto mesh = plyMesh->triangle_mesh();
			std::cout << "Mesh Vertices: " << mesh->num_vertices << std::endl;
			std::cout << "Mesh Indices: " << mesh->num_indices << std::endl;

			std::vector<Vertex> vertices(mesh->num_vertices);
			for (int i = 0; i < mesh->num_vertices; i++) {
				if (mesh->P != nullptr) {
					vertices[i].pos.x = mesh->P[3 * i + 0];
					vertices[i].pos.y = mesh->P[3 * i + 1];
					vertices[i].pos.z = mesh->P[3 * i + 2];
				}
				if (mesh->N != nullptr) {
					vertices[i].normal.x = mesh->N[3 * i + 0];
					vertices[i].normal.y = mesh->N[3 * i + 1];
					vertices[i].normal.z = mesh->N[3 * i + 2];
				}
				if (mesh->S != nullptr) {
					vertices[i].tangent.x = mesh->S[3 * i + 0];
					vertices[i].tangent.y = mesh->S[3 * i + 1];
					vertices[i].tangent.z = mesh->S[3 * i + 2];
				}
				if (mesh->uv != nullptr) {
					vertices[i].texCoord.x = mesh->uv[2 * i + 0];
					vertices[i].texCoord.y = mesh->uv[2 * i + 1];
				}
			}

			std::vector<uint32_t> indices(mesh->num_indices);
			for (int i = 0; i < mesh->num_indices; i++) {
				indices[i] = mesh->indices[i];
			}

			auto meshClass = Mesh::createMesh(m_context.get(), vertices, indices);
			s.vertexAddress = meshClass->getVertexBuffer()->getDeviceAddress();
			s.indexAddress = meshClass->getIndexBuffer()->getDeviceAddress();
			m_meshList.push_back(std::move(meshClass));

			std::cout << "PLYMesh Transform Matrix: " << std::endl;
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					std::cout << s.modelMatrix[i][j] << " ";
				}
				std::cout << std::endl;
			}
			std::cout << "PLYMesh Shape Vertex Address: " << s.vertexAddress << std::endl;
			std::cout << "PLYMesh Shape Index Address: " << s.indexAddress << std::endl;
			std::cout << "PLYMesh Shape Material Index: " << s.materialIdx << std::endl;
			std::cout << "PLYMesh Shape Area Light Index: " << s.areaLightIdx << std::endl;
			std::cout << "PLYMesh Shape Reverse Orientation: " << s.reverseOrientation << std::endl;
			std::cout << "PLYMesh Shape Alpha Index: " << s.alphaIdx << std::endl;
			std::cout << "PLYMesh Shape Shadow Alpha Index: " << s.shadowAlphaIdx << std::endl;
			m_shapeList.push_back(s);
		}
		else {
			std::cout << "Unknown Shape" << static_cast<int>(shape->type()) << std::endl;
		}
	}

	// scene size
	std::cout << "shape size: " << m_pbrtScene->shapes.size() << std::endl;
	std::cout << "material size: " << m_pbrtScene->materials.size() << std::endl;
	std::cout << "texture size: " << m_pbrtScene->textures.size() << std::endl;
	std::cout << "area light size: " << m_pbrtScene->areaLights.size() << std::endl;
	std::cout << "object size: " << m_pbrtScene->objects.size() << std::endl;
	std::cout << "instance size: " << m_pbrtScene->instances.size() << std::endl;
	std::cout << "light size: " << m_pbrtScene->lights.size() << std::endl;
	std::cout << "medium size: " << m_pbrtScene->mediums.size() << std::endl;
	std::cout << "-------------------------" << std::endl;
	std::cout << "Scene loaded!" << std::endl;




	// check all resource
	// m_textureList: view sampler format
	for (auto& tex : m_textureList) {
		std::cout << "texture view: " << tex->getImageView() << std::endl;
		std::cout << "texture sampler: " << tex->getSampler() << std::endl;
		std::cout << "texture format: " << static_cast<int>(tex->getFormat()) << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_textureNameMap: texture name, index
	for (auto& tex : m_textureNameMap) {
		std::cout << "texture name: " << tex.first << std::endl;
		std::cout << "texture index: " << tex.second << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_materialList: material type, index
	for (auto& mat : m_materialList) {
		std::cout << "material type: " << mat.type << std::endl;
		std::cout << "material index: " << mat.index << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_materialNameMap
	for (auto& mat : m_materialNameMap) {
		std::cout << "material name: " << mat.first << std::endl;
		std::cout << "material index: " << mat.second << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_uberList: Kd, Ks, Kr, Kt, opacity, eta, uroughness, vroughness, remaproughness
	for (auto& mat : m_uberList) {
		std::cout << "Uber Material Kd: " << mat.Kd.r << ", " << mat.Kd.g << ", " << mat.Kd.b << std::endl;
		std::cout << "Uber Material KdIdx: " << mat.KdIdx << std::endl;
		std::cout << "Uber Material Ks: " << mat.Ks.r << ", " << mat.Ks.g << ", " << mat.Ks.b << std::endl;
		std::cout << "Uber Material Kr: " << mat.Kr.r << ", " << mat.Kr.g << ", " << mat.Kr.b << std::endl;
		std::cout << "Uber Material Kt: " << mat.Kt.r << ", " << mat.Kt.g << ", " << mat.Kt.b << std::endl;
		std::cout << "Uber Material Opacity: " << mat.opacity.r << ", " << mat.opacity.g << ", " << mat.opacity.b << std::endl;
		std::cout << "Uber Material Eta: " << mat.eta << std::endl;
		std::cout << "Uber Material Uroughness: " << mat.uroughness << std::endl;
		std::cout << "Uber Material Vroughness: " << mat.vroughness << std::endl;
		std::cout << "Uber Material Remaproughness: " << mat.remaproughness << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_matteList: Kd, sigma
	for (auto& mat : m_matteList) {
		std::cout << "Matte Material Kd: " << mat.Kd.r << ", " << mat.Kd.g << ", " << mat.Kd.b << std::endl;
		std::cout << "Matte Material Sigma: " << mat.sigma << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_metalList: eta, k, uroughness, vroughness, remaproughness
	for (auto& mat : m_metalList) {
		std::cout << "Metal Material Eta: " << mat.eta.r << ", " << mat.eta.g << ", " << mat.eta.b << std::endl;
		std::cout << "Metal Material K: " << mat.k.r << ", " << mat.k.g << ", " << mat.k.b << std::endl;
		std::cout << "Metal Material Uroughness: " << mat.uroughness << std::endl;
		std::cout << "Metal Material Vroughness: " << mat.vroughness << std::endl;
		std::cout << "Metal Material Remaproughness: " << mat.remaproughness << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_glassList: Kr, Kt, eta, uroughness, vroughness, remaproughness
	for (auto& mat : m_glassList) {
		std::cout << "Glass Material Kr: " << mat.Kr.r << ", " << mat.Kr.g << ", " << mat.Kr.b << std::endl;
		std::cout << "Glass Material Kt: " << mat.Kt.r << ", " << mat.Kt.g << ", " << mat.Kt.b << std::endl;
		std::cout << "Glass Material Eta: " << mat.eta << std::endl;
		std::cout << "Glass Material Uroughness: " << mat.uroughness << std::endl;
		std::cout << "Glass Material Vroughness: " << mat.vroughness << std::endl;
		std::cout << "Glass Material Remaproughness: " << mat.remaproughness << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_mirrorList: Kr
	for (auto& mat : m_mirrorList) {
		std::cout << "Mirror Material Kr: " << mat.Kr.r << ", " << mat.Kr.g << ", " << mat.Kr.b << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_substrateList: Kd, Ks, uroughness, vroughness, remaproughness
	for (auto& mat : m_substrateList) {
		std::cout << "Substrate Material Kd: " << mat.Kd.r << ", " << mat.Kd.g << ", " << mat.Kd.b << std::endl;
		std::cout << "Substrate Material KdIdx: " << mat.KdIdx << std::endl;
		std::cout << "Substrate Material Ks: " << mat.Ks.r << ", " << mat.Ks.g << ", " << mat.Ks.b << std::endl;
		std::cout << "Substrate Material Uroughness: " << mat.uroughness << std::endl;
		std::cout << "Substrate Material Vroughness: " << mat.vroughness << std::endl;
		std::cout << "Substrate Material Remaproughness: " << mat.remaproughness << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_plasticList: Kd, Ks, roughness, remaproughness
	for (auto& mat : m_plasticList) {
		std::cout << "Plastic Material Kd: " << mat.Kd.r << ", " << mat.Kd.g << ", " << mat.Kd.b << std::endl;
		std::cout << "Plastic Material KdIdx: " << mat.KdIdx << std::endl;
		std::cout << "Plastic Material Ks: " << mat.Ks.r << ", " << mat.Ks.g << ", " << mat.Ks.b << std::endl;
		std::cout << "Plastic Material KsIdx: " << mat.KsIdx << std::endl;
		std::cout << "Plastic Material Roughness: " << mat.roughness << std::endl;
		std::cout << "Plastic Material Remaproughness: " << mat.remaproughness << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_areaLightList: scale, L, twosided, samples
	for (auto& mat : m_areaLightList) {
		std::cout << "Area Light Scale: " << mat.scale.r << ", " << mat.scale.g << ", " << mat.scale.b << std::endl;
		std::cout << "Area Light L: " << mat.L.r << ", " << mat.L.g << ", " << mat.L.b << std::endl;
		std::cout << "Area Light Twosided: " << mat.twosided << std::endl;
		std::cout << "Area Light Samples: " << mat.samples << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

	// m_shapeList: modelMatrix, materialIdx, areaLightIdx, reverseOrientation
	for (auto& mat : m_shapeList) {
		std::cout << "Shape Model Matrix: " << std::endl;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				std::cout << mat.modelMatrix[i][j] << " ";
			}
			std::cout << std::endl;
		}
		std::cout << "Shape Material Index: " << mat.materialIdx << std::endl;
		std::cout << "Shape Area Light Index: " << mat.areaLightIdx << std::endl;
		std::cout << "Shape Reverse Orientation: " << mat.reverseOrientation << std::endl;
		std::cout << "----------------------------" << std::endl;
	}


	// m_meshList: vertexBuffer, indexBuffer sizes
	for (auto& mesh : m_meshList) {
		std::cout << "Mesh Vertex Buffer Size: " << mesh->getVertexBuffer()->getVertexCount() << std::endl;
		std::cout << "Mesh Index Buffer Size: " << mesh->getIndexBuffer()->getIndexCount() << std::endl;
		std::cout << "----------------------------" << std::endl;
	}

}


void Renderer::update(float deltaTime) {
}

void Renderer::render(float deltaTime) {
	vkWaitForFences(m_context->getDevice(), 1, &m_syncObjects->getInFlightFences()[currentFrame], VK_TRUE, UINT64_MAX);
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_context->getDevice(), m_swapChain->getSwapChain(), UINT64_MAX, 
		m_syncObjects->getImageAvailableSemaphores()[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		std::cout << "Swapchain out of date!" << std::endl;
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}


	vkResetFences(m_context->getDevice(), 1, &m_syncObjects->getInFlightFences()[currentFrame]);

	vkResetCommandBuffer(m_commandBuffers->getCommandBuffers()[currentFrame], 0);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(m_commandBuffers->getCommandBuffers()[currentFrame], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	// start record
	recordImGuiCommandBuffer(imageIndex, deltaTime);
	// end record

	if (vkEndCommandBuffer(m_commandBuffers->getCommandBuffers()[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_syncObjects->getImageAvailableSemaphores()[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers->getCommandBuffers()[currentFrame];

	VkSemaphore signalSemaphores[] = { m_syncObjects->getRenderFinishedSemaphores()[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_context->getGraphicsQueue(), 1, &submitInfo, m_syncObjects->getInFlightFences()[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_swapChain->getSwapChain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(m_context->getPresentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		recreateSwapChain();
		std::cout << "Swapchain out of date!" << std::endl;
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::recreateSwapChain() {
	vkDeviceWaitIdle(m_context->getDevice());

	int32_t width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	m_imguiFrameBuffers.clear();
	m_swapChain.reset();
	m_swapChain = SwapChain::createSwapChain(window, m_context.get());


	m_imguiFrameBuffers.resize(m_swapChain->getSwapChainImages().size());
	for (int i = 0; i < m_swapChain->getSwapChainImages().size(); i++) {
		m_imguiFrameBuffers[i] = FrameBuffer::createImGuiFrameBuffer(m_context.get(), m_imguiRenderPass.get(), m_swapChain->getSwapChainImageViews()[i], m_swapChain->getSwapChainExtent());
	}

}

void Renderer::recreateViewport(ImVec2 newExtent) {
	vkDeviceWaitIdle(m_context->getDevice());


}


void Renderer::recordImGuiCommandBuffer(uint32_t imageIndex, float deltaTime) {
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_imguiRenderPass->getRenderPass();
	renderPassInfo.framebuffer = m_imguiFrameBuffers[imageIndex]->getFrameBuffer();
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();

	std::array<VkClearValue, 1> clearValues{};
	clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(m_commandBuffers->getCommandBuffers()[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_guiRenderer->newFrame();
	m_guiRenderer->render(currentFrame, m_commandBuffers->getCommandBuffers()[currentFrame], m_scene.get(), m_modelList, deltaTime);
	vkCmdEndRenderPass(m_commandBuffers->getCommandBuffers()[currentFrame]);
}

void Renderer::transferImageLayout( VkCommandBuffer cmd, Texture* texture, VkImageLayout oldLayout, VkImageLayout newLayout,
	VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, uint32_t layerCount) {
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;
	barrier.image = texture->getImageBuffer()->getImage();
	if (texture->getFormat() == VK_FORMAT_D32_SFLOAT) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layerCount;

	vkCmdPipelineBarrier(
		cmd,
		srcStage,
		dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}


void Renderer::recordReflectionCommandBuffer() {
	VkCommandBuffer cmd = m_commandBuffers->getCommandBuffers()[currentFrame];

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_reflectionPipeline->getPipeline());

	VkDescriptorSet sets[] = {
		m_globlaDescSets[currentFrame]->getDescriptorSet(),  // set=0 (camera)
		m_rtDescSets[currentFrame]->getDescriptorSet(),       // set=1 (outputImage + TLAS)
		m_objectMaterialDescSets[currentFrame]->getDescriptorSet(), // set=2 (object material)
		m_bindlessDescSets[currentFrame]->getDescriptorSet(), // set=3 (bindless)
		m_attachmentDescSets[currentFrame]->getDescriptorSet() // set=4 (gbuffer)
	};
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		m_reflectionPipeline->getPipelineLayout(), 0, 5, sets, 0, nullptr);

	VkStridedDeviceAddressRegionKHR emptyRegion{};
	g_vkCmdTraceRaysKHR(
		cmd,
		&m_reflectionPipeline->getRaygenRegion(),
		&m_reflectionPipeline->getMissRegion(),
		&m_reflectionPipeline->getHitRegion(),
		&emptyRegion,
		m_extent.width,
		m_extent.height,
		1);
}


glm::mat4 Renderer::computeLightMatrix(Light& light) {
	glm::mat4 lightView;
	glm::mat4 lightProj;
	glm::vec3 lightDir = glm::normalize(light.direction);
	glm::vec3 up = (glm::abs(lightDir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

	if (light.type == LIGHT_TYPE_DIRECTIONAL) {
		/*glm::vec3 eye = glm::vec3(0.0f) - glm::normalize(lightDir) * 120.0f;
		glm::vec3 center = glm::vec3(0.0f);*/
		/*lightView = glm::lookAt(eye, center, up);
		lightProj = glm::ortho(-80.f, 80.f, -80.f, 80.f, -50.0f, 250.f);
		lightProj[1][1] *= -1.0f;*/
		glm::vec3 eye = glm::vec3(0.0f) - glm::normalize(lightDir) * 10.0f;
		glm::vec3 center = glm::vec3(0.0f);
		lightView = glm::lookAt(eye, center, up);
		lightProj = glm::ortho(-10.f, 10.f, -10.f, 10.f, -10.0f, 20.f);
		lightProj[1][1] *= -1.0f;
	}
	else if (light.type == LIGHT_TYPE_SPOT) {
		glm::vec3 eye = light.position;
		glm::vec3 target = light.position + glm::normalize(lightDir);

		lightView = glm::lookAt(eye, target, up);
		lightProj = glm::perspective(glm::radians(light.spotOuterAngle * 2.0f), 1.0f, 0.1f, 100.0f);
		lightProj[1][1] *= -1.0f;
	}

	return lightProj * lightView;
}

glm::mat4 Renderer::computePointLightMatrix(Light& light, uint32_t faceIndex) {
	static const glm::vec3 targets[6] = {
		glm::vec3(1,  0,  0), // +X
		glm::vec3(-1,  0,  0), // -X
		glm::vec3(0,  1,  0), // +Y
		glm::vec3(0, -1,  0), // -Y
		glm::vec3(0,  0,  1), // +Z
		glm::vec3(0,  0, -1)  // -Z
	};
	static const glm::vec3 ups[6] = {
		glm::vec3(0, -1,  0), // +X
		glm::vec3(0, -1,  0), // -X
		glm::vec3(0,  0,  1), // +Y
		glm::vec3(0,  0, -1), // -Y
		glm::vec3(0, -1,  0), // +Z
		glm::vec3(0, -1,  0)  // -Z
	};

	glm::vec3 lightPos = light.position;
	glm::mat4 view = glm::lookAt(lightPos, lightPos + targets[faceIndex], ups[faceIndex]);
	glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 50.0f);
	//proj[1][1] *= -1.0f;

	return proj * view;
}

//void Renderer::updateTLAS(std::unordered_map<int32_t, std::vector<int32_t>>& modelToMatrixIndices, std::vector<ModelBuffer>& modelBuffers) {
void Renderer::updateTLAS(std::vector<ObjectInstance>& objDescs, std::vector<ModelBuffer>& modelBuffers) {

	/*if (modelToMatrixIndices.size() > 0) {
		m_tlas[currentFrame]->rebuild(m_blasList, m_modelList, modelToMatrixIndices, modelBuffers, m_scene->getObjects());
		m_rtDescSets[currentFrame]->updateTLAS(m_tlas[currentFrame]->getHandle());
	}
	else {
		m_rtDescSets[currentFrame]->updateTLAS(m_emptyTLAS->getHandle());
	}*/

	if (objDescs.size() > 0) {
		m_tlas[currentFrame]->rebuild(m_blasList, m_shapeList);
		m_rtDescSets[currentFrame]->updateTLAS(m_tlas[currentFrame]->getHandle());
	}
	else {
		m_rtDescSets[currentFrame]->updateTLAS(m_emptyTLAS->getHandle());
	}
}


void Renderer::printObjectInstances(const std::vector<ObjectInstance>& instances) {
	std::cout << "========== ObjectInstance List ==========" << std::endl;
	for (size_t i = 0; i < instances.size(); ++i) {
		const auto& inst = instances[i];
		std::cout << "[" << std::setw(3) << i << "] "
			<< "vertexAddr: 0x" << std::hex << inst.vertexAddress << std::dec << ", "
			<< "indexAddr:  0x" << std::hex << inst.indexAddress << std::dec << ", "
			<< "modelMatIdx: " << inst.modelMatrixIndex << ", "
			<< "materialIdx: " << inst.materialIndex << ", "
			<< "meshIdx: " << inst.meshIndex
			<< std::endl;
	}
	std::cout << "==========================================" << std::endl;
}