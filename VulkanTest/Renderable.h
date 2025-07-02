#pragma once
// Renderable.h
#include <memory>
#include <string>
#include <vector>
#include <optional>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "VulkanGlobals.h"
#include "Material.h"
#include "ModelLoader.h"

// Defines objects in the scene
struct SceneObjectDefinition
{
    std::string name;
    std::string meshPath;
    std::string materialName;
    std::string albedoPath;
    std::string normalPath;
    std::string ormPath;
    std::string aoPath;
    std::string roughnessPath;
    std::string metallnessPath;
    std::string displacementPath;

    bool useOrm = true;

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotationAngles = glm::vec3(0.0f); // in degrees
    glm::vec3 scale = glm::vec3(1.0f);

    PrimitiveModelType defaultModel = PrimitiveModelType::CREATE_NULL;
    MeshFileType meshFileType = MeshFileType::FILE_NULL;
};

class VulkanVertexBuffer;
class VulkanIndexBuffer;
class VulkanTexture;
class Camera;

struct RenderableObject
{
    // Pointers to shared resources
    VulkanVertexBuffer* vertexBuffer = nullptr;
    VulkanIndexBuffer* indexBuffer = nullptr;
    uint32_t indexCount = 0;
    //VulkanTexture* texture = nullptr;

    // Pointer to a shared material
    std::shared_ptr<Material> material = nullptr;

    // Object's transformation  
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    RenderableObject() = default;
};

struct SkyboxData {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;
    bool renderSkyBox = true;
};

struct RenderPacket {
    std::vector<RenderableObject> pbrRenderables;
    VkDeviceSize dynamicUboAlignment;
    VkPipeline pbrPipeline;
    VkPipeline pbrPipeline_doubleSided;
    VkPipelineLayout pbrLayout;

    std::optional<SkyboxData> skyboxData;
    // shadow pass for future
};

//struct TessellationUBO;
struct SceneLightingUBO;

struct SceneDebugContextPacket {
    bool& wireframeMode;
    //TessellationUBO& tessellationUbo;
    SceneLightingUBO& sceneLighingUbo;
    Camera* mainCamera = nullptr;
    float deltaTime = 0.0f;
    std::vector<RenderableObject>& pbrRenderables;
};