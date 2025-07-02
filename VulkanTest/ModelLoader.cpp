#include "ModelLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <vector>
#include <filesystem>
#include <iostream>
#include "Material.h"
#include "VulkanTexture.h"


void ModelLoader::loadModel(const std::string& path, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
	{
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex{};

			// Load Position
			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			// Load Texture Coordinates
			if (index.texcoord_index >= 0 && !attrib.texcoords.empty())
			{
				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}
			else
			{
				vertex.texCoord = { 0.0f, 0.0f };
			}

			// Load Normals
			if (index.normal_index >= 0 && !attrib.normals.empty())
			{
				vertex.inNormal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
			}
			else
			{
				vertex.inNormal = { 0.0f, 0.0f, 1.0f }; // default normal (pointing along z-axis)
			}

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}
}

void ModelLoader::loadGLTFModel(const std::string& path, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err, warn;

	// Determine file type based on extension
	bool isBinary = (path.substr(path.find_last_of(".") + 1) == "glb");
	bool ret = isBinary ? loader.LoadBinaryFromFile(&model, &err, &warn, path)
		: loader.LoadASCIIFromFile(&model, &err, &warn, path);

	if (!warn.empty()) {
		printf("Warning: %s\n", warn.c_str());
	}

	if (!err.empty() || !ret) {
		throw std::runtime_error("Failed to load glTF file: " + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	// Iterate through all meshes in the glTF model
	for (const auto& mesh : model.meshes) {
		for (const auto& primitive : mesh.primitives) {
			// Validate required attributes
			if (primitive.attributes.find("POSITION") == primitive.attributes.end()) {
				throw std::runtime_error("POSITION attribute is required but not found");
			}

			// Get accessors for vertex attributes
			const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];

			// Check for optional attributes
			bool hasNormals = primitive.attributes.find("NORMAL") != primitive.attributes.end();
			bool hasTexCoords = primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end();

			const tinygltf::Accessor* normAccessor = hasNormals ?
				&model.accessors[primitive.attributes.at("NORMAL")] : nullptr;
			const tinygltf::Accessor* texAccessor = hasTexCoords ?
				&model.accessors[primitive.attributes.at("TEXCOORD_0")] : nullptr;

			// Get position buffer data
			const auto& posBufferView = model.bufferViews[posAccessor.bufferView];
			const auto& posBuffer = model.buffers[posBufferView.buffer];
			const float* positions = reinterpret_cast<const float*>(
				&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);

			// Get normal buffer data (if available)
			const float* normals = nullptr;
			if (hasNormals && normAccessor) {
				const auto& normBufferView = model.bufferViews[normAccessor->bufferView];
				const auto& normBuffer = model.buffers[normBufferView.buffer];
				normals = reinterpret_cast<const float*>(
					&normBuffer.data[normBufferView.byteOffset + normAccessor->byteOffset]);
			}

			// Get texture coordinate buffer data (if available)
			const float* texCoords = nullptr;
			if (hasTexCoords && texAccessor) {
				const auto& texBufferView = model.bufferViews[texAccessor->bufferView];
				const auto& texBuffer = model.buffers[texBufferView.buffer];
				texCoords = reinterpret_cast<const float*>(
					&texBuffer.data[texBufferView.byteOffset + texAccessor->byteOffset]);
			}

			// Load vertices
			for (size_t i = 0; i < posAccessor.count; ++i) {
				Vertex vertex{};

				// Position (mandatory)
				vertex.pos = {
					positions[i * 3 + 0],
					positions[i * 3 + 1],
					positions[i * 3 + 2]
				};

				// Normals (optional)
				if (normals) {
					vertex.inNormal = {
						normals[i * 3 + 0],
						normals[i * 3 + 1],
						normals[i * 3 + 2]
					};
				}
				else {
					vertex.inNormal = { 0.0f, 0.0f, 1.0f };
				}

				// Texture coordinates (optional)
				if (texCoords) {
					vertex.texCoord = {
						texCoords[i * 2 + 0],
						texCoords[i * 2 + 1]
					};
				}
				else {
					vertex.texCoord = { 0.0f, 0.0f };
				}

				// Default color
				vertex.color = { 1.0f, 1.0f, 1.0f };

				// Add unique vertex
				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
			}

			if (primitive.indices >= 0) { // Check if indices exist
				const auto& indexAccessor = model.accessors[primitive.indices];
				const auto& indexBufferView = model.bufferViews[indexAccessor.bufferView];
				const auto& indexBuffer = model.buffers[indexBufferView.buffer];
				const uint8_t* indexData = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];

				for (size_t i = 0; i < indexAccessor.count; ++i) {
					uint32_t originalIndex;

					// Parse index based on component type
					if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
						originalIndex = indexData[i];
					}
					else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
						originalIndex = reinterpret_cast<const uint16_t*>(indexData)[i];
					}
					else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
						originalIndex = reinterpret_cast<const uint32_t*>(indexData)[i];
					}
					else {
						throw std::runtime_error("Unsupported index component type");
					}

					// Validate index bounds
					if (originalIndex >= posAccessor.count) {
						throw std::runtime_error("Index out of bounds");
					}

					// Reconstruct the vertex to find its unique index
					Vertex vertex{};
					vertex.pos = {
						positions[originalIndex * 3 + 0],
						positions[originalIndex * 3 + 1],
						positions[originalIndex * 3 + 2]
					};

					if (normals) {
						vertex.inNormal = {
							normals[originalIndex * 3 + 0],
							normals[originalIndex * 3 + 1],
							normals[originalIndex * 3 + 2]
						};
					}
					else {
						vertex.inNormal = { 0.0f, 0.0f, 1.0f };
					}

					if (texCoords) {
						vertex.texCoord = {
							texCoords[originalIndex * 2 + 0],
							texCoords[originalIndex * 2 + 1]
						};
					}
					else {
						vertex.texCoord = { 0.0f, 0.0f };
					}

					vertex.color = { 1.0f, 1.0f, 1.0f };

					// Find the unique vertex index
					auto it = uniqueVertices.find(vertex);
					if (it != uniqueVertices.end()) {
						indices.push_back(it->second);
					}
					else {
						throw std::runtime_error("Vertex not found in unique vertices map");
					}
				}
			}
		}
	}
}

void ModelLoader::createSphere(float radius, uint32_t latSegments, uint32_t lonSegments, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	vertices.clear();
	indices.clear();

	latSegments = std::max(3u, latSegments);
	lonSegments = std::max(3u, lonSegments);

	for (uint32_t lat = 0; lat <= latSegments; ++lat)
	{
		for (uint32_t lon = 0; lon <= lonSegments; ++lon)
		{
			Vertex vertex{};

			// Spherical coordinates to Cartesian (Y-Up)
			float theta = static_cast<float>(lat) / latSegments * glm::pi<float>(); // [0, pi]
			float phi = static_cast<float>(lon) / lonSegments * 2.0f * glm::pi<float>(); // [0, 2pi]

			// Position (Y-Up: Y is up, Z is depth, X is right)
			vertex.pos = {
				radius * std::sin(theta) * std::cos(phi),	// x
				radius * std::cos(theta),					// y (up)
				radius * std::sin(theta) * std::sin(phi)	// z (depth)
			};

			vertex.inNormal = glm::normalize(vertex.pos);
			
			vertex.texCoord = {
				static_cast<float>(lon) / lonSegments,	// u: [0, 1]
				static_cast<float>(lat) / latSegments	// v: [0, 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			vertices.push_back(vertex);
		}
	}

	for (uint32_t lat = 0; lat < latSegments; ++lat)
	{
		for (uint32_t lon = 0; lon < lonSegments; ++lon)
		{
			uint32_t first = lat * (lonSegments + 1) + lon;
			uint32_t second = first + lonSegments + 1;

			// Clockwise winding
			/*indices.push_back(first);
			indices.push_back(first + 1);
			indices.push_back(second);

			indices.push_back(second);
			indices.push_back(first + 1);
			indices.push_back(second + 1);*/

			// Counter-Clockwise winding
			indices.push_back(first);
			indices.push_back(second);
			indices.push_back(first + 1);

			indices.push_back(second);
			indices.push_back(second + 1);
			indices.push_back(first + 1);
		}
	}
}

/**
 * @brief Creates a plane mesh centered in the XZ plane.
 *
 * This function generates vertices and indices for a plane with the specified dimensions and segmentation.
 * The plane is created on the XZ axis, making it suitable for floors or flat surfaces in a Y-Up coordinate system.
 *
 * @param width The width of the plane along the X-axis.
 * @param height The height of the plane along the Z-axis.
 * @param widthSegments The number of segments along the plane's width.
 * @param heightSegments The number of segments along the plane's height.
 * @param vertices A reference to the vector that will be populated with the generated vertices.
 * @param indices A reference to the vector that will be populated with the generated indices.
 */
void ModelLoader::createPlane(float width, float height, uint32_t widthSegments, uint32_t heightSegments, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	// Clear any existing data in the vectors
	vertices.clear();
	indices.clear();

	// Ensure there is at least one segment
	widthSegments = std::max(1u, widthSegments);
	heightSegments = std::max(1u, heightSegments);

	// Calculate starting positions to center the plane at the origin
	const float halfWidth = width / 2.0f;
	const float halfHeight = height / 2.0f;

	// Calculate the size of each segment
	const float segmentWidth = width / static_cast<float>(widthSegments);
	const float segmentHeight = height / static_cast<float>(heightSegments);

	// Generate the grid of vertices
	for (uint32_t j = 0; j <= heightSegments; ++j)
	{
		for (uint32_t i = 0; i <= widthSegments; ++i)
		{
			Vertex vertex{};

			// Calculate vertex position
			const float x = (static_cast<float>(i) * segmentWidth) - halfWidth;
			const float z = (static_cast<float>(j) * segmentHeight) - halfHeight;
			vertex.pos = { x, 0.0f, z };

			// The normal for every vertex on a flat plane points directly up
			vertex.inNormal = { 0.0f, 1.0f, 0.0f };

			// Calculate texture coordinates
			vertex.texCoord = {
				static_cast<float>(i) / static_cast<float>(widthSegments),
				(static_cast<float>(j) / static_cast<float>(heightSegments))
			};

			// Assign a default white color
			vertex.color = { 1.0f, 1.0f, 1.0f };

			vertices.push_back(vertex);
		}
	}

	// Generate indices for the plane's triangles
	for (uint32_t j = 0; j < heightSegments; ++j)
	{
		for (uint32_t i = 0; i < widthSegments; ++i)
		{
			// Calculate the indices of the four vertices of the current quad
			const uint32_t topLeft = j * (widthSegments + 1) + i;
			const uint32_t topRight = topLeft + 1;
			const uint32_t bottomLeft = (j + 1) * (widthSegments + 1) + i;
			const uint32_t bottomRight = bottomLeft + 1;

			// for some reason plane renders only when its clock wise winding
			indices.push_back(topLeft);
			indices.push_back(topRight);
			indices.push_back(bottomRight);
			indices.push_back(topLeft);
			indices.push_back(bottomRight);
			indices.push_back(bottomLeft);
		}
	}
}
/**
 * @brief Creates a cube mesh centered at the origin.
 * @param halfSize The half-length of one side of the cube. The cube will span from -halfSize to +halfSize on each axis.
 * @param segments The number of segments per face. A value of 1 creates a simple cube with 2 triangles per face.
 * @param vertices A reference to the vector that will be populated with the generated vertices.
 * @param indices A reference to the vector that will be populated with the generated indices.
 */
void ModelLoader::createCube(float halfSize, uint32_t segments, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	vertices.clear();
	indices.clear();

	segments = std::max(1u, segments);

	// Define the 6 faces of the cube.
	// For each face, we define its normal and the two axes (u, v) that form its plane.
	// This allows us to correctly generate vertex positions and texture coordinates.
	// The u-axis corresponds to the texture's X direction, and v-axis to Y.
	glm::vec3 normals[] = {
		{ 0,  0,  1}, // Front
		{ 0,  0, -1}, // Back
		{ 1,  0,  0}, // Right
		{-1,  0,  0}, // Left
		{ 0,  1,  0}, // Top
		{ 0, -1,  0}  // Bottom
	};

	glm::vec3 u_axes[] = {
		{ 1,  0,  0}, // Front
		{-1,  0,  0}, // Back
		{ 0,  0, -1}, // Right
		{ 0,  0,  1}, // Left
		{ 1,  0,  0}, // Top
		{ 1,  0,  0}  // Bottom
	};

	glm::vec3 v_axes[] = {
		{ 0,  1,  0}, // Front
		{ 0,  1,  0}, // Back
		{ 0,  1,  0}, // Right
		{ 0,  1,  0}, // Left
		{ 0,  0, -1}, // Top
		{ 0,  0,  1}  // Bottom
	};

	for (int face = 0; face < 6; ++face) {
		uint32_t baseVertexIndex = static_cast<uint32_t>(vertices.size());
		glm::vec3 normal = normals[face];
		glm::vec3 u_axis = u_axes[face];
		glm::vec3 v_axis = v_axes[face];

		// Generate vertices for the current face
		for (uint32_t j = 0; j <= segments; ++j) {
			for (uint32_t i = 0; i <= segments; ++i) {
				Vertex vertex{};

				float u = (float)i / segments; // Varies from 0.0 to 1.0
				float v = (float)j / segments; // Varies from 0.0 to 1.0

				// Position is calculated from the center of the face, offset by the u and v axes
				vertex.pos = halfSize * normal + (2.0f * u - 1.0f) * halfSize * u_axis + (2.0f * v - 1.0f) * halfSize * v_axis;
				vertex.inNormal = normal;
				vertex.texCoord = { u, v };
				vertex.color = { 1.0f, 1.0f, 1.0f };

				vertices.push_back(vertex);
			}
		}

		// Generate indices for the current face
		for (uint32_t j = 0; j < segments; ++j) {
			for (uint32_t i = 0; i < segments; ++i) {
				uint32_t p00 = baseVertexIndex + j * (segments + 1) + i;
				uint32_t p10 = p00 + 1;
				uint32_t p01 = p00 + (segments + 1);
				uint32_t p11 = p01 + 1;

				// Create two triangles for the quad (Counter-Clockwise winding)
				indices.push_back(p01);
				indices.push_back(p10);
				indices.push_back(p00);

				indices.push_back(p11);
				indices.push_back(p10);
				indices.push_back(p01);
			}
		}
	}
}




void ModelLoader::createPrimitive(float radius, PrimitiveModelType modelType, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	if (modelType == PrimitiveModelType::CREATE_SPHERE)
	{
		createSphere(radius, 32, 32, vertices, indices);
	}
	else if (modelType == PrimitiveModelType::CREATE_PLANE)
	{
		createPlane(radius * 2, radius * 2, 1, 1, vertices, indices);
	}
}

GltfLoadResult ModelLoader::loadGLTFModelWithMaterials(const std::string& path, VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkCommandPool commandPool)
{
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err, warn;

	bool isBinary = (path.substr(path.find_last_of(".") + 1) == "glb");
	bool ret = isBinary ? loader.LoadBinaryFromFile(&model, &err, &warn, path)
		: loader.LoadASCIIFromFile(&model, &err, &warn, path);

	if (!warn.empty()) {
		printf("glTF Warning: %s\n", warn.c_str());
	}
	if (!err.empty() || !ret) {
		throw std::runtime_error("Failed to load glTF file: " + err);
	}

	GltfLoadResult result;

	// --- 1. Load Textures ---
	result.textures.resize(model.textures.size());
	for (size_t i = 0; i < model.textures.size(); ++i) {
		bool isSrgb = false;
		for (const auto& mat : model.materials) {
			if ((mat.pbrMetallicRoughness.baseColorTexture.index == i) || (mat.emissiveTexture.index == i)) {
				isSrgb = true;
				break;
			}
		}
		std::cout << "Loading texture: " << model.images[model.textures[i].source].uri << std::endl;
		result.textures[i] = loadGltfTexture(model, static_cast<int>(i), device, physicalDevice, graphicsQueue, commandPool, path, isSrgb);
	}
	// --- 2. Load Materials ---
	result.materials.reserve(model.materials.size());
	for (const auto& gltfMaterial : model.materials) {
		result.materials.push_back(createMaterialFromGltf(model, gltfMaterial, result.textures, path, device, physicalDevice, graphicsQueue, commandPool));
	}
	if (result.materials.empty()) {
		result.materials.push_back(createDefaultGltfMaterial("DefaultMaterial", device, physicalDevice, graphicsQueue, commandPool));
	}

	// --- 3. Load Meshes (Primitives) ---
	for (const auto& mesh : model.meshes) {
		for (const auto& primitive : mesh.primitives) {

			// We can only process indexed geometry with positions
			if (primitive.indices < 0 || primitive.attributes.find("POSITION") == primitive.attributes.end()) {
				continue;
			}

			// These will store the final, de-duplicated vertex and index data for this primitive
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			std::unordered_map<Vertex, uint32_t> uniqueVertices{};

			// Get pointers to the raw attribute data buffers in the glTF file
			const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
			const auto& posBufferView = model.bufferViews[posAccessor.bufferView];
			const float* positions = reinterpret_cast<const float*>(&model.buffers[posBufferView.buffer].data[posBufferView.byteOffset + posAccessor.byteOffset]);

			const float* normals = nullptr;
			if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
				const auto& normAccessor = model.accessors[primitive.attributes.at("NORMAL")];
				const auto& normBufferView = model.bufferViews[normAccessor.bufferView];
				normals = reinterpret_cast<const float*>(&model.buffers[normBufferView.buffer].data[normBufferView.byteOffset + normAccessor.byteOffset]);
			}

			const float* texCoords = nullptr;
			if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
				const auto& texAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
				const auto& texBufferView = model.bufferViews[texAccessor.bufferView];
				texCoords = reinterpret_cast<const float*>(&model.buffers[texBufferView.buffer].data[texBufferView.byteOffset + texAccessor.byteOffset]);
			}

			// Get a pointer to the raw index buffer data
			const auto& indexAccessor = model.accessors[primitive.indices];
			const auto& indexBufferView = model.bufferViews[indexAccessor.bufferView];
			const uint8_t* indexData = &model.buffers[indexBufferView.buffer].data[indexBufferView.byteOffset + indexAccessor.byteOffset];

			// === The Core Logic: Build vertices on-demand from the index buffer ===
			for (size_t i = 0; i < indexAccessor.count; ++i) {
				// Get the index into the attribute buffers
				uint32_t originalIndex;
				switch (indexAccessor.componentType) {
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
					originalIndex = indexData[i];
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
					originalIndex = reinterpret_cast<const uint16_t*>(indexData)[i];
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
					originalIndex = reinterpret_cast<const uint32_t*>(indexData)[i];
					break;
				default:
					throw std::runtime_error("Unsupported index component type!");
				}

				// Construct a full Vertex object from the raw attribute data
				Vertex vertex{};
				vertex.pos = glm::make_vec3(&positions[originalIndex * 3]);

				if (normals) {
					vertex.inNormal = glm::make_vec3(&normals[originalIndex * 3]);
				}
				else {
					// NOTE: Normals should ideally be calculated if missing.
					// For now, we use a placeholder.
					vertex.inNormal = glm::vec3(0.0f, 0.0f, 1.0f);
				}

				if (texCoords) {
					vertex.texCoord = glm::make_vec2(&texCoords[originalIndex * 2]);
				}
				else {
					vertex.texCoord = glm::vec2(0.0f);
				}

				vertex.color = glm::vec3(1.0f); // Default white

				// The de-duplication step
				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
			}

			// Store the processed mesh data for this primitive
			result.meshVertices.push_back(std::move(vertices));
			result.meshIndices.push_back(std::move(indices));

			// Store the material index for this primitive
			int materialIndex = primitive.material;
			if (materialIndex < 0 || materialIndex >= result.materials.size()) {
				materialIndex = 0; // Fallback to the first (or default) material
			}
			result.meshMaterialIndices.push_back(materialIndex);
		}
	}

	return result;
}

std::shared_ptr<VulkanTexture> ModelLoader::loadGltfTexture(
	const tinygltf::Model& model, 
	int textureIndex, 
	VkDevice device, 
	VkPhysicalDevice physicalDevice, 
	VkQueue graphicsQueue, 
	VkCommandPool commandPool, 
	const std::string& gltfFilePath,
	bool sRGB)
{

	//std::cout << "Loading gltf texture" << std::endl;
	if (textureIndex < 0 || textureIndex >= model.textures.size()) return nullptr;

	const auto& texture = model.textures[textureIndex];
	const auto& image = model.images[texture.source];

	auto vulkanTexture = std::make_shared<VulkanTexture>();

	std::string imagePath = resolveGltfTexturePath(gltfFilePath, image.uri);

	try {
		if (!imagePath.empty()) {
			if (!std::filesystem::exists(imagePath)) {
				std::cerr << "Warning: Texture file not found: " << imagePath << std::endl;
				return nullptr; // Return null if file is missing
			}
			// Load from file
			vulkanTexture->createTexture2D(device, physicalDevice, graphicsQueue, commandPool, imagePath, sRGB);

		}
		else if (!image.image.empty()) {
			// Load from embedded memory
			vulkanTexture->createTexture2DFromMemory(
				device, physicalDevice, graphicsQueue, commandPool,
				image.image.data(),
				image.width, image.height, image.component, sRGB
			);
		}
		else {
			std::cerr << "Warning: Texture " << textureIndex << " has no URI and no embedded data." << std::endl;
			return nullptr;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Warning: Failed to load texture for " << image.name << ". Reason: " << e.what() << std::endl;
		return nullptr;
	}

	return vulkanTexture;
}

std::shared_ptr<Material> ModelLoader::createMaterialFromGltf(
	const tinygltf::Model& model, 
	const tinygltf::Material& gltfMaterial, 
	const std::vector<std::shared_ptr<VulkanTexture>>& textures, 
	const std::string& modelPath,
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	VkQueue graphicsQueue, 
	VkCommandPool commandPool
)
{
	auto material = std::make_shared<Material>();
	material->name = gltfMaterial.name.empty() ? "Unnamed_Materal" : gltfMaterial.name;
	//material->gltfSourceFile = modelPath;

	// Base Color (albedo)
	material->uboData.baseColorFactor = glm::make_vec4(gltfMaterial.pbrMetallicRoughness.baseColorFactor.data());
	if (gltfMaterial.pbrMetallicRoughness.baseColorTexture.index >= 0)
	{
		material->albedoMap = textures[gltfMaterial.pbrMetallicRoughness.baseColorTexture.index];
	}
	material->uboData.hasAlbedoMap = (material->albedoMap != nullptr);

	// metallilc roughness
	material->uboData.metallicFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
	material->uboData.roughnessFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.roughnessFactor);
	if (gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0)
	{
		material->metallicRoughnessMap = textures[gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index];
	}
	material->uboData.hasMetallicRoughnessMap = (material->metallicRoughnessMap != nullptr);
	
	// noraml map
	if (gltfMaterial.normalTexture.index >= 0)
	{
		material->normalMap = textures[gltfMaterial.normalTexture.index];
	}
	material->uboData.hasNormalMap = (material->normalMap != nullptr);
	// maybe implement normal scale

	// Occlusion Map
	if (gltfMaterial.occlusionTexture.index >= 0)
	{
		material->occlusionMap = textures[gltfMaterial.occlusionTexture.index];
	}
	material->uboData.hasOcclusionMap = (material->occlusionMap != nullptr);

	// Emissive
	material->uboData.emissiveFactor = glm::make_vec4(gltfMaterial.emissiveFactor.data());
	if (gltfMaterial.emissiveTexture.index >= 0)
	{
		material->emissiveMap = textures[gltfMaterial.emissiveTexture.index];
	}
	material->uboData.hasEmissiveMap = (material->emissiveMap != nullptr);

	
	// load defaults for missing maps
	//if (!material->albedoMap)
	//{
	//	std::cout << "Loading default texture for gltf albedo" << std::endl;
	//	material->albedoMap = loadDefaultTexture("albedo", device, physicalDevice, graphicsQueue, commandPool);
	//}
	//if (!material->normalMap) {
	//	std::cout << "Loading default texture for gltf normal" << std::endl;
	//	material->normalMap = loadDefaultTexture("normal", device, physicalDevice, graphicsQueue, commandPool);
	//}
	//if (!material->ormMap) {
	//	std::cout << "Loading default texture for gltf orm" << std::endl;
	//	material->ormMap = loadDefaultTexture("orm", device, physicalDevice, graphicsQueue, commandPool);
	//	material->useOrm = false; // Set to false if we are using the default
	//}
	//if (!material->aoMap) {
	//	std::cout << "Loading default texture for gltf ao" << std::endl;
	//	material->aoMap = loadDefaultTexture("ao", device, physicalDevice, graphicsQueue, commandPool);
	//}
	//if (!material->emissiveMap) {
	//	std::cout << "Loading default texture for gltf emissive" << std::endl;
	//	material->emissiveMap = loadDefaultTexture("emissive", device, physicalDevice, graphicsQueue, commandPool);
	//}
	//if (!material->roughnessMap/* && !material->useOrm*/)
	//{
	//	std::cout << "Loading default texture for gltf roughness" << std::endl;
	//	material->roughnessMap = loadDefaultTexture("roughness", device, physicalDevice, graphicsQueue, commandPool);
	//}
	//if (!material->metallnessMap/* && !material->useOrm*/)
	//{
	//	std::cout << "Loading default texture for gltf metalness" << std::endl;
	//	material->metallnessMap = loadDefaultTexture("metalness", device, physicalDevice, graphicsQueue, commandPool);
	//}
	//if (!material->displacementMap)
	//{
	//	std::cout << "Loading default texture for gltf displacement" << std::endl;
	//	material->displacementMap = loadDefaultTexture("displacement", device, physicalDevice, graphicsQueue, commandPool);
	//}

	//// material properties
	//if (gltfMaterial.alphaMode == "OPAQUE")
	//{
	//	material->alphaMode = Material::AlphaMode::OPAQUE_MODE;
	//}
	//else if (gltfMaterial.alphaMode == "MASK")
	//{
	//	material->alphaMode = Material::AlphaMode::MASK_MODE;
	//}
	//else if (gltfMaterial.alphaMode == "BLEND")
	//{
	//	material->alphaMode = Material::AlphaMode::BLEND_MODE;
	//}

	//material->alphaCutoff = static_cast<float>(gltfMaterial.alphaCutoff);
	
	// ----------- TODO : DEBUG doublesided ---------------
	material->doubleSided = gltfMaterial.doubleSided;
	//std::cout << "Double sided: " << gltfMaterial.doubleSided << std::endl;
	material->doubleSided = true;

	return material;
}

std::string ModelLoader::resolveGltfTexturePath(const std::string& gltfFilePath, const std::string& textureUri)
{
	if (textureUri.empty())
	{
		return "";
	}

	std::filesystem::path texturePath(textureUri);
	if (texturePath.is_absolute())
	{
		return textureUri;
	}

	if (textureUri.find("data:") == 0) // for data URIs (embedded base64)
	{
		std::cout << "Info: Found embedded glTF texture data. In-memory loading will be used." << std::endl;
		return "";
	}

	// relative paths
	std::filesystem::path modelPath(gltfFilePath);
	std::filesystem::path modelDirectory = modelPath.parent_path();
	std::filesystem::path fullPath = modelDirectory / textureUri;
	std::filesystem::path normalizedPath = std::filesystem::absolute(fullPath).lexically_normal();
	return normalizedPath.string();

}

std::shared_ptr<VulkanTexture> ModelLoader::loadDefaultTexture(
	const std::string& textureType,
	VkDevice device, 
	VkPhysicalDevice physicalDevice, 
	VkQueue graphicsQueue, 
	VkCommandPool commandPool)
{
	std::string defaultPath;
	bool sRGB = false;
	if (textureType == "albedo")
	{
		defaultPath = "textures/defaults/default_albedo.png";
		sRGB = true;
	}
	else if (textureType == "normal")
	{
		defaultPath = "textures/defaults/default_normal.png";
	}
	else if (textureType == "orm")
	{
		defaultPath = "textures/defaults/default_orm.png";
	}
	else if (textureType == "ao")
	{
		defaultPath = "textures/defaults/default_ao.png";
	}
	else if (textureType == "roughness")
	{
		defaultPath = "textures/defaults/default_roughness.png";
	}
	else if (textureType == "metalness")
	{
		defaultPath = "textures/defaults/default_metalness.png";
	}
	else if (textureType == "metallicRoughness")
	{
		defaultPath = "textures/defaults/default_metallic_roughness.png";
	}
	else if (textureType == "displacement")
	{
		defaultPath = "textures/defaults/default_displacement.png";
	}
	else if (textureType == "emissive")
	{
		defaultPath = "textures/defaults/default_emissive.png";
		sRGB = true;
	}
	else
	{
		defaultPath = "textures/defaults/default_white.png";
		sRGB = true;
	}

	auto texture = std::make_shared<VulkanTexture>();
	
	try 
	{
		texture->createTexture2D(device, physicalDevice, graphicsQueue, commandPool, defaultPath);
		return texture;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Warning: Failed to load default texture '" 
			<< defaultPath << "' for type '" << textureType << "': " << e.what() << std::endl;
		return nullptr;
	}
}

std::shared_ptr<Material> ModelLoader::createDefaultGltfMaterial(const std::string& name, VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkCommandPool commandPool)
{
	std::cout << "Creating default Gltf Material" << std::endl;
	auto material = std::make_shared<Material>();
	material->name = name;
	//material->useOrm = false; // Use separate textures for default material

	material->albedoMap = loadDefaultTexture("albedo", device, physicalDevice, graphicsQueue, commandPool);
	material->normalMap = loadDefaultTexture("normal", device, physicalDevice, graphicsQueue, commandPool);
	material->occlusionMap = loadDefaultTexture("ao", device, physicalDevice, graphicsQueue, commandPool);
	material->metallicRoughnessMap = loadDefaultTexture("metallicRoughness", device, physicalDevice, graphicsQueue, commandPool);
	//material->metallnessMap = loadDefaultTexture("metalness", device, physicalDevice, graphicsQueue, commandPool);
	//material->displacementMap = loadDefaultTexture("displacement", device, physicalDevice, graphicsQueue, commandPool);
	material->emissiveMap = loadDefaultTexture("emissive", device, physicalDevice, graphicsQueue, commandPool);

	//material->baseColorFactor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	//material->metallicFactor = 0.0f;
	//material->roughnessFactor = 0.5f;
	//material->normalScale = 1.0f;
	//material->occlusioinStrength = 1.0f;
	//material->emissiveFactor = glm::vec3(0.0f);
	//material->alphaMode = Material::AlphaMode::OPAQUE_MODE;
	//material->alphaCutoff = 0.5f;
	material->doubleSided = false;

	return material;
}
