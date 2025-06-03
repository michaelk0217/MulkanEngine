#include "Camera.h"

Camera::Camera()
{
	position = glm::vec3(0.0f, 0.0f, 3.0f);
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f); // Y-up
	yaw = -90.0f; // To look along -Z axis initially
	pitch = 0.0f;
	front = glm::vec3(0.0f, 0.0f, -1.0f); // Will be properly set by updateCameraVectors

	moveSpeed = 5.0f;   // Sensible default
	turnSpeed = 0.1f;   // Sensible default

	fov = 45.0f;
	// A common default, but will be overridden if parameterized constructor is used
	aspectRatio = 16.0f / 9.0f;
	near = 0.1f;
	far = 100.0f;
	updateCameraVectors();
}

Camera::Camera(glm::vec3 startPosition, glm::vec3 worldUpVector, 
	float startYaw, float startPitch, 
	float startMoveSpeed, float startTurnSpeed, 
	float fieldOfView, float aspectRatioF, float nearF, float farF)
{
	position = startPosition;
	worldUp = worldUpVector;
	yaw = startYaw;
	pitch = startPitch;
	front = glm::vec3(0.0f, 0.0f, -1.0f);

	moveSpeed = startMoveSpeed;
	turnSpeed = startTurnSpeed;

	fov = fieldOfView;
	aspectRatio = aspectRatioF;
	near = nearF;
	far = farF;
	updateCameraVectors();
}

Camera::~Camera()
{
}

void Camera::processKeyboard(bool* keys, float deltaTime)
{
	float velocity = moveSpeed * deltaTime;
	if (keys[GLFW_KEY_W])
	{
		position += front * velocity;
	}
	if (keys[GLFW_KEY_S])
	{
		position -= front * velocity;
	}
	if (keys[GLFW_KEY_D])
	{
		position += right * velocity;
	}
	if (keys[GLFW_KEY_A])
	{
		position -= right * velocity;
	}
	if (keys[GLFW_KEY_LEFT_SHIFT])
	{
		position -= worldUp * velocity;
	}
	if (keys[GLFW_KEY_SPACE])
	{
		position += worldUp * velocity;
	}
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
	xoffset *= turnSpeed;
	yoffset *= turnSpeed;

	yaw += xoffset;
	pitch += yoffset;

	if (constrainPitch)
	{
		if (pitch > 89.0f)
		{
			pitch = 89.0f;
		}

		if (pitch < -89.0)
		{
			pitch = -89.0f;
		}
	}
	
	updateCameraVectors();
}

glm::vec3 Camera::getCameraPosition() const
{
	return position;
}

glm::vec3 Camera::getCameraDirection() const
{
	return glm::normalize(front);
}

glm::mat4 Camera::calculateViewMatrix() const
{
	return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix() const
{
	return glm::perspective(glm::radians(fov), aspectRatio, near, far);
}

void Camera::updateCameraVectors()
{
    // Calculate the new Front vector based on Yaw and Pitch (Y-UP system)
    glm::vec3 tempFront;
    tempFront.x = glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
    tempFront.y = glm::sin(glm::radians(pitch));
    tempFront.z = glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
    front = glm::normalize(tempFront);

    // Also re-calculate the Right and Up vector
    // 'worldUp' is the up direction of the world (e.g., glm::vec3(0.0f, 1.0f, 0.0f))
    right = glm::normalize(glm::cross(front, worldUp));
    up    = glm::normalize(glm::cross(right, front)); // Camera's local up
}
