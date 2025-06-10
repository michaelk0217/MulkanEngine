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
