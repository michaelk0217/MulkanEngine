#include "ModelLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

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

			// Z-up
			/*vertex.pos = {
				radius * std::sin(theta) * std::cos(phi),
				radius * std::sin(theta) * std::sin(phi),
				radius * std::cos(theta)
			};*/

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

			// Two Triangles per quad (counter-clockwise)
			/*indices.push_back(first);
			indices.push_back(second);
			indices.push_back(first + 1);

			indices.push_back(second);
			indices.push_back(second + 1);
			indices.push_back(first + 1);*/

			// Clockwise winding
			indices.push_back(first);
			indices.push_back(first + 1);
			indices.push_back(second);

			indices.push_back(second);
			indices.push_back(first + 1);
			indices.push_back(second + 1);
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
				static_cast<float>(j) / static_cast<float>(heightSegments)
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

			// Create two triangles for the quad with a clockwise winding order
			// First triangle
			indices.push_back(topLeft);
			indices.push_back(bottomLeft);
			indices.push_back(topRight);

			// Second triangle
			indices.push_back(topRight);
			indices.push_back(bottomLeft);
			indices.push_back(bottomRight);
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
