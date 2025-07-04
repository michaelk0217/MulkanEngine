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

std::string AssetManager::getTextureMapTypeDefaultFilePath(TextureMap texType)
{
    if (texType == TextureMap::ALBEDO)
    {
        return "textures/defaults/default_albedo.png";
    }
    else if (texType == TextureMap::NORMAL)
    {
        return "textures/defaults/default_normal.png";
    }
    else if (texType == TextureMap::METAL_ROUGH)
    {
        return "textures/defaults/default_orm.png";;
    }
    else if (texType == TextureMap::AMBIENT_OCC)
    {
        return "textures/defaults/default_emissive.png";
    }
    else if (texType == TextureMap::EMISSIVE)
    {
        return "textures/defaults/default_emissive.png";
    }
    else
    {
        throw std::runtime_error("Invalid TextureMap ENUM");
    }
}

//RenderableObject AssetManager::createRenderableObject(const SceneObjectDefinition& def)
//{
//    std::shared_ptr<MeshData> mesh = getMesh(def);
//    std::shared_ptr<Material> material = getMaterial(def);
//
//    RenderableObject renderable{};
//    renderable.vertexBuffer = mesh->vertexBuffer.get(); //raw pointer
//    renderable.indexBuffer = mesh->indexBuffer.get();
//    renderable.indexCount = mesh->indexCount;
//    renderable.material = material;
//
//    // Calculate the object's model matrix from its definition
//    glm::mat4 model = glm::mat4(1.0f);
//    model = glm::translate(model, def.position);
//    model = glm::rotate(model, glm::radians(def.rotationAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));
//    model = glm::rotate(model, glm::radians(def.rotationAngles.y), glm::vec3(0.0f, 1.0f, 0.0f));
//    model = glm::rotate(model, glm::radians(def.rotationAngles.x), glm::vec3(1.0f, 0.0f, 0.0f));
//    model = glm::scale(model, def.scale);
//    renderable.modelMatrix = model;
//
//    return renderable;
//}

//std::shared_ptr<MeshData> AssetManager::getMesh(const SceneObjectDefinition& def)
//{
//    std::string meshKey = def.meshPath;
//    if (meshKey.empty())
//    {
//        meshKey = "primitive_mesh_" + def.name;
//    }
//    if (m_Meshes.count(meshKey))
//    {
//        return m_Meshes.at(meshKey);
//    }
//
//    std::cout << "Loading new mesh: " << (def.meshPath.empty() ? def.name + " (Generated Primitive)" : def.meshPath) << std::endl;
//
//    std::vector<Vertex> vertices;
//    std::vector<uint32_t> indices;
//
//    if (def.meshPath.empty())
//    {
//        if (def.defaultModel == PrimitiveModelType::CREATE_SPHERE)
//        {
//            ModelLoader::createSphere(2.5, 32, 32, vertices, indices);
//        }
//        else if (def.defaultModel == PrimitiveModelType::CREATE_PLANE)
//        {
//            ModelLoader::createPlane(50, 50, 32, 32, vertices, indices);
//        }
//        else if (def.defaultModel == PrimitiveModelType::CREATE_CUBE)
//        {
//            ModelLoader::createCube(2.5, 1, vertices, indices);
//        }
//        else
//        {
//            throw std::runtime_error("Invalid PrimitiveModelType in SceneObjectDefinition");
//        }
//    }
//    else
//    {
//        if (def.meshFileType == MeshFileType::FILE_OBJ)
//        {
//            ModelLoader::loadModel(def.meshPath, vertices, indices);
//        }
//        else if (def.meshFileType == MeshFileType::FILE_GLTF)
//        {
//            ModelLoader::loadGLTFModel(def.meshPath, vertices, indices);
//        }
//        else
//        {
//            throw std::runtime_error("Invalid MeshFileType in SceneObjectDefinition");
//        }
//    }
//
//    auto newMesh = std::make_shared<MeshData>();
//    newMesh->vertexBuffer = std::make_unique<VulkanVertexBuffer>();
//    newMesh->vertexBuffer->create(m_pDevice->getLogicalDevice(), m_pDevice->getPhysicalDevice(), m_pDevice->getGraphicsQueue(), m_pCommandPool->getVkCommandPool(), vertices);
//
//    newMesh->indexBuffer = std::make_unique<VulkanIndexBuffer>();
//    newMesh->indexBuffer->create(m_pDevice->getLogicalDevice(), m_pDevice->getPhysicalDevice(), m_pDevice->getGraphicsQueue(), m_pCommandPool->getVkCommandPool(), indices);
//
//    newMesh->indexCount = static_cast<uint32_t>(indices.size());
//
//    m_Meshes[meshKey] = newMesh;
//
//    return newMesh;
//}

//std::shared_ptr<Material> AssetManager::getMaterial(const SceneObjectDefinition& def)
//{
//    if (m_Materials.count(def.materialName))
//    {
//        return m_Materials.at(def.materialName);
//    }
//
//    std::cout << "Creating new material: " << def.materialName << std::endl;
//
//    auto newMaterial = std::make_shared<Material>();
//    newMaterial->name = def.materialName;
//
//    newMaterial->albedoMap = loadTexture(def.albedoPath, "texture/defaults/default_albedo.png", true);
//    newMaterial->normalMap = loadTexture(def.normalPath, "textures/default_normal.png");
//
//    if (def.useOrm)
//    {
//        newMaterial->ormMap = loadTexture(def.ormPath, "textures/default_orm.png");
//        newMaterial->aoMap = loadTexture("textures/defaults/default_ao.png", "textures/defaults/default_ao.png");
//        newMaterial->roughnessMap = loadTexture("textures/defaults/default_roughness.png", "textures/defaults/default_roughness.png");
//        newMaterial->metallnessMap = loadTexture("textures/defaults/default_metalness.png", "textures/defaults/default_metalness.png");
//    }
//    else
//    {
//        newMaterial->ormMap = loadTexture("textures/default_orm.png", "textures/default_orm.png");
//        newMaterial->aoMap = loadTexture(def.aoPath, "textures/defaults/default_ao.png");
//        newMaterial->roughnessMap = loadTexture(def.roughnessPath, "textures/defaults/default_roughness.png");
//        newMaterial->metallnessMap = loadTexture(def.metallnessPath, "textures/defaults/default_metalness.png");
//    }
//
//    
//    newMaterial->displacementMap = loadTexture(def.displacementPath, "textures/default_displacement.png"); // Use a neutral default for displacement
//
//    m_Materials[def.materialName] = newMaterial;
//    return newMaterial;
//}

//std::shared_ptr<VulkanTexture> AssetManager::loadTexture(const std::string& path, const std::string& defaultPath, bool sRGB)
//{
//    const std::string& finalPath = (path.empty() && !defaultPath.empty()) ? defaultPath : path;
//
//    if (finalPath.empty())
//    {
//        throw std::runtime_error("Texture path and default path are both empty.");
//    }
//
//    if (m_Textures.count(finalPath))
//    {
//        return m_Textures.at(finalPath);
//    }
//
//    std::cout << "Loading new texture: " << finalPath << std::endl;
//
//    auto newTexture = std::make_shared<VulkanTexture>();
//    newTexture->createTexture2D(
//        m_pDevice->getLogicalDevice(),
//        m_pDevice->getPhysicalDevice(),
//        m_pDevice->getGraphicsQueue(),
//        m_pCommandPool->getVkCommandPool(),
//        finalPath,
//        sRGB
//    );
//
//    m_Textures[finalPath] = newTexture;
//    return newTexture;
//}

std::map<std::string, std::shared_ptr<Material>>& AssetManager::getMaterials()
{
    return m_Materials;
}

std::shared_ptr<ModelData> AssetManager::loadGltfModel(const std::string& path)
{
    if (m_Models.count(path))
    {
        return m_Models[path];
    }

    std::cout << "Loading glTF model with materials: " << path << std::endl;

    auto gltfResult = ModelLoader::loadGLTFModelWithMaterials(
        path,
        m_pDevice->getLogicalDevice(),
        m_pDevice->getPhysicalDevice(),
        m_pDevice->getGraphicsQueue(),
        m_pCommandPool->getVkCommandPool()    
    );

    auto modelData = std::make_shared<ModelData>();
    modelData->materials = std::move(gltfResult.materials);
    modelData->meshMaterialIndices = std::move(gltfResult.meshMaterialIndices);
    modelData->meshWorldMatrices = std::move(gltfResult.meshWorldMatrices);

    for (const auto& material : modelData->materials)
    {
        if (m_Materials.find(material->name) == m_Materials.end())
        {
            m_Materials[material->name] = material;
        }
    }

    for (size_t i = 0; i < gltfResult.meshVertices.size(); ++i)
    {
        MeshData meshData;
        meshData.vertexBuffer = std::make_unique<VulkanVertexBuffer>();
        meshData.vertexBuffer->create(
            m_pDevice->getLogicalDevice(),
            m_pDevice->getPhysicalDevice(),
            m_pDevice->getGraphicsQueue(),
            m_pCommandPool->getVkCommandPool(),
            gltfResult.meshVertices[i]
        );

        meshData.indexBuffer = std::make_unique<VulkanIndexBuffer>();
        meshData.indexBuffer->create(
            m_pDevice->getLogicalDevice(),
            m_pDevice->getPhysicalDevice(),
            m_pDevice->getGraphicsQueue(),
            m_pCommandPool->getVkCommandPool(),
            gltfResult.meshIndices[i]
        );

        meshData.indexCount = static_cast<uint32_t>(gltfResult.meshIndices[i].size());
        modelData->meshes.push_back(std::move(meshData));
    }

    // de-duplication and caching for materials
    for (size_t i = 0; i < modelData->materials.size(); ++i) {
        std::shared_ptr<Material>& mat = modelData->materials[i];

        if (m_Materials.count(mat->name)) {
            // A material with this name already exists in the cache.
            // Replace the pointer in our local list with the one from the cache.
            mat = m_Materials.at(mat->name);
        }
        else {
            // This is a brand new material. Add it to the cache.
            m_Materials[mat->name] = mat;
        }
    }

    m_Models[path] = modelData;
    return modelData;
}

std::vector<RenderableObject> AssetManager::createRenderableObjectsFromGltf(const SceneObjectDefinition& def)
{
    auto modelData = loadGltfModel(def.meshPath);
    std::vector<RenderableObject> renderables;
    glm::mat4 globalObjectTransform = glm::mat4(1.0f);
    globalObjectTransform = glm::translate(globalObjectTransform, def.position);
    globalObjectTransform = glm::rotate(globalObjectTransform, glm::radians(def.rotationAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));
    globalObjectTransform = glm::rotate(globalObjectTransform, glm::radians(def.rotationAngles.y), glm::vec3(0.0f, 1.0f, 0.0f));
    globalObjectTransform = glm::rotate(globalObjectTransform, glm::radians(def.rotationAngles.x), glm::vec3(1.0f, 0.0f, 0.0f));
    globalObjectTransform = glm::scale(globalObjectTransform, def.scale);

    for (size_t i = 0; i < modelData->meshes.size(); ++i)
    {
        RenderableObject renderable{};
        renderable.vertexBuffer = modelData->meshes[i].vertexBuffer.get();
        renderable.indexBuffer = modelData->meshes[i].indexBuffer.get();
        renderable.indexCount = modelData->meshes[i].indexCount;

        int materialIndex = modelData->meshMaterialIndices[i];
        renderable.material = modelData->materials[materialIndex];
        if (!renderable.material->albedoMap)
        {
            //std::cout << "Loading default texture for gltf albedo" << std::endl;
            renderable.material->albedoMap = getOrLoadTexture(getTextureMapTypeDefaultFilePath(TextureMap::ALBEDO), true);
        }
        if (!renderable.material->normalMap)
        {
            //std::cout << "Loading default texture for gltf normal" << std::endl;
            renderable.material->normalMap = getOrLoadTexture(getTextureMapTypeDefaultFilePath(TextureMap::NORMAL));
        }
        if (!renderable.material->metallicRoughnessMap)
        {
            //std::cout << "Loading default texture for gltf metallic roughness" << std::endl;
            renderable.material->metallicRoughnessMap = getOrLoadTexture(getTextureMapTypeDefaultFilePath(TextureMap::METAL_ROUGH));
        }
        if (!renderable.material->occlusionMap)
        {
            //std::cout << "Loading default texture for gltf occlusion" << std::endl;
            renderable.material->occlusionMap = getOrLoadTexture(getTextureMapTypeDefaultFilePath(TextureMap::AMBIENT_OCC));
        }
        if (!renderable.material->emissiveMap)
        {
            //std::cout << "Loading default texture for gltf emissive" << std::endl;
            renderable.material->emissiveMap = getOrLoadTexture(getTextureMapTypeDefaultFilePath(TextureMap::EMISSIVE), true);
        }

        //// applying transformations here (maybe i'll handle them elsewhere later)
        //glm::mat4 model = glm::mat4(1.0f);
        //model = glm::translate(model, def.position);
        //model = glm::rotate(model, glm::radians(def.rotationAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));
        //model = glm::rotate(model, glm::radians(def.rotationAngles.y), glm::vec3(0.0f, 1.0f, 0.0f));
        //model = glm::rotate(model, glm::radians(def.rotationAngles.x), glm::vec3(1.0f, 0.0f, 0.0f));
        //model = glm::scale(model, def.scale);
        //renderable.modelMatrix = model;
        renderable.modelMatrix = globalObjectTransform * modelData->meshWorldMatrices[i];

        renderables.push_back(renderable);
    }

    return renderables;
}

std::shared_ptr<VulkanTexture> AssetManager::getOrLoadTexture(const std::string& path, bool sRGB)
{
    if (m_Textures.count(path)) {
        return m_Textures.at(path);
    }

    // Otherwise, load it, cache it, and return it
    std::cout << "Loading new texture: " << path << std::endl;
    auto newTexture = std::make_shared<VulkanTexture>();
    newTexture->createTexture2D(
        m_pDevice->getLogicalDevice(),
        m_pDevice->getPhysicalDevice(),
        m_pDevice->getGraphicsQueue(),
        m_pCommandPool->getVkCommandPool(),
        path,
        sRGB
    );

    m_Textures[path] = newTexture;
    return newTexture;
}
