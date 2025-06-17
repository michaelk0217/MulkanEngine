#include "AssetManager.h"
#include <iostream>

AssetManager::AssetManager(VulkanDevice* device, VulkanCommandPool* commandPool) : m_pDevice(device), m_pCommandPool(commandPool)
{
	if (!m_pDevice || !m_pCommandPool)
	{
		throw std::runtime_error("AssetManager requires valid VulkanDevice and VulkanCommandPool pointers!");
	}
}

AssetManager::~AssetManager()
{
	cleanup();
}

void AssetManager::cleanup()
{
    
    m_Materials.clear();

    for (auto& pair : m_Meshes) {
        if (pair.second) {
            if (pair.second->vertexBuffer) pair.second->vertexBuffer->destroy();
            if (pair.second->indexBuffer) pair.second->indexBuffer->destroy();
        }
    }
    m_Meshes.clear();

    // The unique_ptr/shared_ptr destructors will handle memory, but we need
    // to explicitly call destroy() on the Vulkan objects.
    for (auto& pair : m_Textures) {
        if (pair.second) pair.second->destroy();
        pair.second.reset();
    }
    m_Textures.clear();
}

RenderableObject AssetManager::createRenderableObject(const SceneObjectDefinition& def)
{
    std::shared_ptr<MeshData> mesh = getMesh(def);
    std::shared_ptr<Material> material = getMaterial(def);

    RenderableObject renderable{};
    renderable.vertexBuffer = mesh->vertexBuffer.get(); //raw pointer
    renderable.indexBuffer = mesh->indexBuffer.get();
    renderable.indexCount = mesh->indexCount;
    renderable.material = material;

    // Calculate the object's model matrix from its definition
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, def.position);
    model = glm::rotate(model, glm::radians(def.rotationAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::rotate(model, glm::radians(def.rotationAngles.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(def.rotationAngles.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, def.scale);
    renderable.modelMatrix = model;

    return renderable;
}

std::shared_ptr<MeshData> AssetManager::getMesh(const SceneObjectDefinition& def)
{
    // Use the mesh path as a key, but if it's empty (for a primitive),
    // generate a unique key from the object's name to enable caching.
    std::string meshKey = def.meshPath;
    if (meshKey.empty())
    {
        // Example key for a sphere named "MetalBall": "primitive_mesh_MetalBall"
        meshKey = "primitive_mesh_" + def.name;
    }

    // Check if a mesh with this key is already cached.
    if (m_Meshes.count(meshKey))
    {
        return m_Meshes.at(meshKey);
    }

    std::cout << "Loading new mesh: " << (def.meshPath.empty() ? def.name + " (Generated Primitive)" : def.meshPath) << std::endl;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    if (def.meshPath.empty())
    {
        if (def.defaultModel == PrimitiveModelType::CREATE_SPHERE)
        {
            ModelLoader::createSphere(2.5, 32, 32, vertices, indices);
        }
        else if (def.defaultModel == PrimitiveModelType::CREATE_PLANE)
        {
            ModelLoader::createPlane(50, 50, 1, 1, vertices, indices);
        }
        else
        {
            throw std::runtime_error("Invalid PrimitiveModelType in SceneObjectDefinition");
        }
    }
    else
    {
        ModelLoader::loadModel(def.meshPath, vertices, indices);
    }

    auto newMesh = std::make_shared<MeshData>();
    newMesh->vertexBuffer = std::make_unique<VulkanVertexBuffer>();
    newMesh->vertexBuffer->create(m_pDevice->getLogicalDevice(), m_pDevice->getPhysicalDevice(), m_pDevice->getGraphicsQueue(), m_pCommandPool->getVkCommandPool(), vertices);

    newMesh->indexBuffer = std::make_unique<VulkanIndexBuffer>();
    newMesh->indexBuffer->create(m_pDevice->getLogicalDevice(), m_pDevice->getPhysicalDevice(), m_pDevice->getGraphicsQueue(), m_pCommandPool->getVkCommandPool(), indices);

    newMesh->indexCount = static_cast<uint32_t>(indices.size());

    // **THE FIX**: Always cache the newly created mesh using its unique key.
    m_Meshes[meshKey] = newMesh;

    return newMesh;
}

std::shared_ptr<Material> AssetManager::getMaterial(const SceneObjectDefinition& def)
{
    if (m_Materials.count(def.materalName))
    {
        return m_Materials.at(def.materalName);
    }

    std::cout << "Creating new material: " << def.materalName << std::endl;

    auto newMaterial = std::make_shared<Material>();
    newMaterial->name = def.materalName;

    newMaterial->albedoMap = loadTexture(def.albedoPath);
    newMaterial->normalMap = loadTexture(def.normalPath, "textures/default_normal.png");

    newMaterial->ormMap = loadTexture(def.ormPath, "textures/default_orm.png");
    newMaterial->displacementMap = loadTexture(def.displacementPath, "textures/default_orm.png"); // Use a neutral default for displacement

    m_Materials[def.materalName] = newMaterial;
    return newMaterial;
}

std::shared_ptr<VulkanTexture> AssetManager::loadTexture(const std::string& path, const std::string& defaultPath)
{
    const std::string& finalPath = (path.empty() && !defaultPath.empty()) ? defaultPath : path;

    if (finalPath.empty())
    {
        throw std::runtime_error("Texture path and default path are both empty.");
    }

    if (m_Textures.count(finalPath))
    {
        return m_Textures.at(finalPath);
    }

    std::cout << "Loading new texture: " << finalPath << std::endl;

    auto newTexture = std::make_shared<VulkanTexture>();
    newTexture->createTexture2D(
        m_pDevice->getLogicalDevice(),
        m_pDevice->getPhysicalDevice(),
        m_pDevice->getGraphicsQueue(),
        m_pCommandPool->getVkCommandPool(),
        finalPath
    );

    m_Textures[finalPath] = newTexture;
    return newTexture;
}

std::map<std::string, std::shared_ptr<Material>>& AssetManager::getMaterials()
{
    return m_Materials;
}
