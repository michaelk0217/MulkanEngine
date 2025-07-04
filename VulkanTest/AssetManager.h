#pragma once
#include <string>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#include "VulkanDevice.h"
#include "VulkanCommandPool.h"
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"
#include "VulkanTexture.h"
#include "ModelLoader.h"
#include "Renderable.h"
#include "Material.h"

struct MeshData
{
	std::unique_ptr<VulkanVertexBuffer> vertexBuffer;
	std::unique_ptr<VulkanIndexBuffer> indexBuffer;
	uint32_t indexCount = 0;
};

struct ModelData
{
	std::vector<MeshData> meshes;
	std::vector<std::shared_ptr<Material>> materials;
	std::vector<int> meshMaterialIndices; // which material each mesh uses
	std::vector<glm::mat4> meshWorldMatrices; // transform for each mesh
};

class AssetManager
{
public:
	enum class TextureMap {
		ALBEDO,
		NORMAL,
		METAL_ROUGH,
		AMBIENT_OCC,
		EMISSIVE
	};

	AssetManager(VulkanDevice* device, VulkanCommandPool* commandPool);
	~AssetManager();

	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;
	AssetManager(AssetManager&&) = delete;
	AssetManager& operator=(AssetManager&&) = delete;

	//RenderableObject createRenderableObject(const SceneObjectDefinition& def);

	//std::shared_ptr<VulkanTexture> loadTexture(const std::string& path, const std::string& defaultPath = "", bool sRGB = false);

	std::map<std::string, std::shared_ptr<Material>>& getMaterials();

	std::shared_ptr<ModelData> loadGltfModel(const std::string& path);
	std::vector<RenderableObject> createRenderableObjectsFromGltf(
		const SceneObjectDefinition& def
	);

	std::shared_ptr<VulkanTexture> getOrLoadTexture(const std::string& path, bool sRGB = false);
private:

	void cleanup();
private:
	// Private helper methods that implement the caching logic.
	//std::shared_ptr<MeshData> getMesh(const SceneObjectDefinition& def);
	//std::shared_ptr<Material> getMaterial(const SceneObjectDefinition& def);

	// Pointers to essential Vulkan components (owned by VulkanEngine).
	VulkanDevice* m_pDevice;
	VulkanCommandPool* m_pCommandPool;

	// Caches for all loaded assets.
	// The string key is typically the file path.
	std::map<std::string, std::shared_ptr<MeshData>> m_Meshes;
	std::map<std::string, std::shared_ptr<Material>> m_Materials;
	std::map<std::string, std::shared_ptr<VulkanTexture>> m_Textures;
	std::map<std::string, std::shared_ptr<ModelData>> m_Models;

	std::string getTextureMapTypeDefaultFilePath(TextureMap texType);
};
