#include "include/Mesh.h"

std::unique_ptr<Mesh> Mesh::createMesh(VulkanContext* context, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, bool hasTangent) {
	std::unique_ptr<Mesh> mesh = std::unique_ptr<Mesh>(new Mesh());
	mesh->init(context, vertices, indices);
	return mesh;
}

void Mesh::init(VulkanContext* context, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, bool hasTangent) {
	this->context = context;
	if (!hasTangent)
		calculateTangents(vertices, indices);

	m_vertexBuffer = VertexBuffer::createVertexBuffer(context, vertices);
	m_indexBuffer = IndexBuffer::createIndexBuffer(context, indices);
}

void Mesh::calculateTangents(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	for (auto& v : vertices)
		v.tangent = glm::vec4(0.0f);

	for (size_t i = 0; i < indices.size(); i += 3)
	{
		if (indices[i] >= vertices.size() || indices[i + 1] >= vertices.size() || indices[i + 2] >= vertices.size())
		{
			continue;
		}
		Vertex& v0 = vertices[indices[i]];
		Vertex& v1 = vertices[indices[i + 1]];
		Vertex& v2 = vertices[indices[i + 2]];

		glm::vec3 edge1 = v1.pos - v0.pos;
		glm::vec3 edge2 = v2.pos - v0.pos;

		glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
		glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		if (std::isnan(f))
		{
			f = 0.0f;
		}

		glm::vec3 tangent;
		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		tangent = glm::normalize(tangent);

		v0.tangent += glm::vec4(tangent, 0.0f);
		v1.tangent += glm::vec4(tangent, 0.0f);
		v2.tangent += glm::vec4(tangent, 0.0f);
	}


	for (auto& v : vertices)
	{
		glm::vec3 T = glm::normalize(glm::vec3(v.tangent));
		glm::vec3 N = glm::normalize(v.normal);
		glm::vec3 B = glm::normalize(glm::cross(N, T));

		float w = (glm::dot(glm::cross(N, T), B) < 0.0f) ? -1.0f : 1.0f;
		v.tangent = glm::vec4(T, w);
		
	}
}

Mesh::~Mesh() {
	cleanup();
}

void Mesh::cleanup() {

}

void Mesh::draw(VkCommandBuffer commandBuffer) {
	m_vertexBuffer->bind(commandBuffer);
	m_indexBuffer->bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, m_indexBuffer->getIndexCount(), 1, 0, 0, 0);
}

void Mesh::drawInstance(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance) {
	m_vertexBuffer->bind(commandBuffer);
	m_indexBuffer->bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, m_indexBuffer->getIndexCount(), instanceCount, 0, 0, firstInstance);
}

std::unique_ptr<Mesh> Mesh::createBoxMesh(VulkanContext* context) {
	std::vector<Vertex> vertices = {
		Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), 0.0f, glm::vec3(0.0f, 0.0f, -1.0f), 0.0f, glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, -0.5f, -0.5f),0.0f, glm::vec3(0.0f, 0.0f, -1.0f), 0.0f, glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, 0.5f, -0.5f), 0.0f, glm::vec3(0.0f, 0.0f, -1.0f), 0.0f, glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), 0.0f, glm::vec3(0.0f, 0.0f, -1.0f), 0.0f, glm::vec2(0.0f, 1.0f)},

		Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, -0.5f, 0.5f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, 0.5f, 0.5f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, glm::vec2(0.0f, 1.0f)},

		Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), 0.0f, glm::vec3(-1.0f, 0.0f, 0.0f), 0.0f, glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), 0.0f, glm::vec3(-1.0f, 0.0f, 0.0f), 0.0f, glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), 0.0f, glm::vec3(-1.0f, 0.0f, 0.0f), 0.0f, glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), 0.0f, glm::vec3(-1.0f, 0.0f, 0.0f), 0.0f, glm::vec2(0.0f, 0.0f)},

		Vertex{glm::vec3(0.5f, 0.5f, 0.5f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, 0.5f, -0.5f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(0.5f, -0.5f, -0.5f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3(0.5f, -0.5f, 0.5f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, glm::vec2(0.0f, 0.0f)},

		Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), 0.0f, glm::vec3(0.0f, -1.0f, 0.0f), 0.0f, glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3(0.5f, -0.5f, -0.5f), 0.0f, glm::vec3(0.0f, -1.0f, 0.0f), 0.0f, glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(0.5f, -0.5f, 0.5f), 0.0f, glm::vec3(0.0f, -1.0f, 0.0f), 0.0f, glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), 0.0f, glm::vec3(0.0f, -1.0f, 0.0f), 0.0f, glm::vec2(0.0f, 0.0f)},

		Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3(0.5f, 0.5f, -0.5f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(0.5f, 0.5f, 0.5f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec2(0.0f, 0.0f)},
	};

	std::vector<uint32_t> indices = {
		0,	2,	1,	2,	0,	3,	4,	5,	6,	6,	7,	4,	8,	9,	10, 10, 11, 8,
		12, 14, 13, 14, 12, 15, 16, 17, 18, 18, 19, 16, 20, 22, 21, 22, 20, 23,
	};

	return createMesh(context, vertices, indices);
}

std::unique_ptr<Mesh> Mesh::createSphereMesh(VulkanContext* context) {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	uint32_t latiSegmentCount = 16;
	uint32_t longiSegmentCount = 32;

	uint32_t circleVertCount = longiSegmentCount + 1;
	vertices.resize((latiSegmentCount + 1) * circleVertCount);
	for (uint32_t i = 0; i <= latiSegmentCount; i++)
	{
		float v = (float)i / (float)latiSegmentCount;
		float phi = (v - 0.5f) * glm::pi<float>();
		auto cosPhi = cosf(phi);
		auto sinPhi = sinf(phi);
		for (uint32_t j = 0; j <= longiSegmentCount; j++)
		{
			float u = (float)j / (float)longiSegmentCount;
			float theta = u * glm::pi<float>() * 2.0f;
			auto cosTheta = cosf(theta);
			auto sinTheta = sinf(theta);
			auto point = glm::vec3(cosPhi * cosTheta, sinPhi, -cosPhi * sinTheta);

			vertices[i * circleVertCount + j] = Vertex{ point * 0.5f, 0.0f, point, 0.0f, glm::vec2(u, v) };
		}
	}

	indices.resize(latiSegmentCount * longiSegmentCount * 6);
	for (uint32_t i = 0; i < latiSegmentCount; i++)
	{
		for (uint32_t j = 0; j < longiSegmentCount; j++)
		{
			uint32_t vertexOffset = i * circleVertCount + j;
			uint32_t indexOffset = (i * longiSegmentCount + j) * 6;
			indices[indexOffset] = vertexOffset;
			indices[indexOffset + 1] = vertexOffset + 1;
			indices[indexOffset + 2] = vertexOffset + 1 + circleVertCount;
			indices[indexOffset + 3] = vertexOffset;
			indices[indexOffset + 4] = vertexOffset + 1 + circleVertCount;
			indices[indexOffset + 5] = vertexOffset + circleVertCount;
		}
	}
	return createMesh(context, vertices, indices);
}


std::unique_ptr<Mesh> Mesh::createPlaneMesh(VulkanContext* context)
{
	std::vector<Vertex> vertices = {
		Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, -0.5f, 0.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, 0.5f, 0.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(-0.5f, 0.5f, 0.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, glm::vec2(0.0f, 1.0f)},
	};

	std::vector<uint32_t> indices = {
		0, 1, 2, 2, 3, 0,
	};

	return createMesh(context, vertices, indices);
}