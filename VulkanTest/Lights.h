#pragma once
#include <glm/glm.hpp>

struct DirectionalLight
{
	glm::vec4 direction; // w component is unused padding
	glm::vec4 color;     // w component is intensity
};


// this structure gets sent to the GPU
struct SceneLightingUBO
{
	DirectionalLight dirLight;
	glm::vec4 viewPosition; // camera
};