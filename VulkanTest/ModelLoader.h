#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <vulkan/vulkan.h>
#include <array>
#include <string>


struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 inNormal;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, inNormal);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

namespace std {
	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return  ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1) ^
				(hash<glm::vec3>()(vertex.inNormal) << 2);
		}
	};
}

enum class PrimitiveModelType
{
	CREATE_NULL,
	CREATE_SPHERE,
	CREATE_PLANE,
	CREATE_CUBE
};

enum class MeshFileType
{
	FILE_NULL,
	FILE_OBJ,
	FILE_GLTF
};

class ModelLoader
{
public:
	static void loadModel(const std::string& path, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	static void loadGLTFModel(const std::string& path, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	
	static void createSphere(float radius, uint32_t latSegments, uint32_t lonSegments, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	static void createPlane(float width, float height, uint32_t widthSegments, uint32_t heightSegments, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	static void createCube(float halfSize, uint32_t segments, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	//static void createCube(float halfSize, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	
	static void createPrimitive(float radius, PrimitiveModelType modelType, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
};

