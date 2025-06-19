#include "ModelLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include <cmath>
#include <vector>

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
			// Get accessor for vertex attributes
			const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
			const auto& normAccessor = primitive.attributes.find("NORMAL") != primitive.attributes.end()
				? model.accessors[primitive.attributes.at("NORMAL")]
				: tinygltf::Accessor{};
			const auto& texAccessor = primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()
				? model.accessors[primitive.attributes.at("TEXCOORD_0")]
				: tinygltf::Accessor{};

			// Get buffer views and buffers
			const auto& posBufferView = model.bufferViews[posAccessor.bufferView];
			const auto& posBuffer = model.buffers[posBufferView.buffer];
			const float* positions = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);

			const float* normals = nullptr;
			if (normAccessor.count > 0) {
				const auto& normBufferView = model.bufferViews[normAccessor.bufferView];
				const auto& normBuffer = model.buffers[normBufferView.buffer];
				normals = reinterpret_cast<const float*>(&normBuffer.data[normBufferView.byteOffset + normAccessor.byteOffset]);
			}

			const float* texCoords = nullptr;
			if (texAccessor.count > 0) {
				const auto& texBufferView = model.bufferViews[texAccessor.bufferView];
				const auto& texBuffer = model.buffers[texBufferView.buffer];
				texCoords = reinterpret_cast<const float*>(&texBuffer.data[texBufferView.byteOffset + texAccessor.byteOffset]);
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
				vertex.inNormal = normals ? glm::vec3{ normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2] }
				: glm::vec3{ 0.0f, 0.0f, 1.0f };

				// Texture coordinates (optional)
				vertex.texCoord = texCoords ? glm::vec2{ texCoords[i * 2 + 0], texCoords[i * 2 + 1] }
				: glm::vec2{ 0.0f, 0.0f };

				// Default color (same as OBJ loader)
				vertex.color = { 1.0f, 1.0f, 1.0f };

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
			}

			// Load indices
			const auto& indexAccessor = model.accessors[primitive.indices];
			const auto& indexBufferView = model.bufferViews[indexAccessor.bufferView];
			const auto& indexBuffer = model.buffers[indexBufferView.buffer];
			const uint8_t* indexData = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];

			for (size_t i = 0; i < indexAccessor.count; ++i) {
				uint32_t index;
				if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
					index = reinterpret_cast<const uint16_t*>(indexData)[i];
				}
				else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
					index = reinterpret_cast<const uint32_t*>(indexData)[i];
				}
				else {
					throw std::runtime_error("Unsupported index component type");
				}
				indices.push_back(uniqueVertices[vertices[index]]);
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
