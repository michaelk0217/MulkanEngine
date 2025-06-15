#pragma once
#include <string>
#include <map>
#include <memory>
#include <stdexcept>

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

class AssetManager
{
public:
	AssetManager(VulkanDevice* device, VulkanCommandPool* commandPool);
	~AssetManager();

	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;
	AssetManager(AssetManager&&) = delete;
	AssetManager& operator=(AssetManager&&) = delete;

	RenderableObject createRenderableObject(const SceneObjectDefinition& def);

	std::shared_ptr<VulkanTexture> loadTexture(const std::string& path, const std::string& defaultPath = "");

	std::map<std::string, std::shared_ptr<Material>>& getMaterials();

	void cleanup();
private:
	// Private helper methods that implement the caching logic.
	std::shared_ptr<MeshData> getMesh(const std::string& meshPath);
	std::shared_ptr<Material> getMaterial(const SceneObjectDefinition& def);

	// Pointers to essential Vulkan components (owned by VulkanEngine).
	VulkanDevice* m_pDevice;
	VulkanCommandPool* m_pCommandPool;

	// Caches for all loaded assets.
	// The string key is typically the file path.
	std::map<std::string, std::shared_ptr<MeshData>> m_Meshes;
	std::map<std::string, std::shared_ptr<Material>> m_Materials;
	std::map<std::string, std::shared_ptr<VulkanTexture>> m_Textures;
};
