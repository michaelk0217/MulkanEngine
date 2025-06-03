#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera
{
public:
	Camera();
	Camera(glm::vec3 startPosition, glm::vec3 worldUpVector, float startYaw, float startPitch, float startMoveSpeed, float startTurnSpeed, float fov, float aspectRatio, float near, float far);
	~Camera();

	void processKeyboard(bool* keys, float deltaTime);
	void processMouseMovement(float xoffset, float yoffset, bool constrainPitch);

	glm::vec3 getCameraPosition() const;
	glm::vec3 getCameraDirection() const;
	glm::mat4 calculateViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	float yaw;
	float pitch;

	float moveSpeed;
	float turnSpeed;

	float fov;
	float aspectRatio;
	float near;
	float far;

	void updateCameraVectors();
};

