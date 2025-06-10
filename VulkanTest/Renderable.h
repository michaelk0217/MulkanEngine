#pragma once
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "VulkanGlobals.h"
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"
#include "VulkanTexture.h"
#include "VulkanUniformBuffers.h"
#include "Material.h"

// Defines objects in the scene
struct SceneObjectDefinition
{
    std::string name;
    std::string meshPath;
    std::string materalName;
    //std::string texturePath;
    std::string albedoPath;
    std::string normalPath;
    std::string ormPath;

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotationAngles = glm::vec3(0.0f); // in degrees
    glm::vec3 scale = glm::vec3(1.0f);
};

class VulkanVertexBuffer;
class VulkanIndexBuffer;
class VulkanTexture;

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

    // Each object has its own descriptor sets --- Removed: the descriptor sets are now in the Material struct
    //std::vector<VkDescriptorSet> frameSpecificDescriptorSets;

    //RenderableObject() : frameSpecificDescriptorSets(VulkanGlobals::MAX_FRAMES_IN_FLIGHT) {}
    RenderableObject() = default;
};