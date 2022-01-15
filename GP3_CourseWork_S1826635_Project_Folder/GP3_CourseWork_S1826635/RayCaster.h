#pragma once
#include <GL\glew.h>
#include <SDL\SDL.h>
#include <glm/glm.hpp>
#include "Camera.h"
#include "Display.h"

class RayCaster
{
public:

	void initRayCaster(glm::mat4 projMat, glm::mat4 viewMat, glm::vec3 cameraPos);

	glm::vec3 GetCurrentRay()
	{
		return currentRay;
	}

	glm::vec3 GetCurrentPlanePoint()
	{
		return currentPlanePoint;
	}

	float GetRayRange()
	{
		return rayRange;
	}

	void UpdateRay(glm::mat4 viewMat, float displayWidth, float displayHeight, glm::vec3 cameraPos);

	glm::vec3 CalculateMouseRay(float displayWidth, float displayHeight);

	glm::vec2 GetNormalizedDeviceCoords(int* mouseX, int* mouseY, float displayWidth, float displayHeight);

	glm::vec4 ConvertToEyeCoords(glm::vec4 clipCoords);

	glm::vec3 ConvertToWorldCoords(glm::vec4 eyeCoords);

	bool IntersectionInRange(float start, float finish, glm::vec3 ray);

	glm::vec3 GetPointOnRay(glm::vec3 ray, float distance);
	
	bool IsPointOnPlane(glm::vec3 testPoint);

protected:
private:

	glm::vec3 currentRay;
	glm::mat4 projMatrix;
	glm::mat4 viewMatrix;

	GLint mouseX, mouseY;

	float rayRange;
	glm::vec3 currentPlanePoint;
	glm::vec3 pointOnPlane;
	glm::vec3 currentCameraPos;
};

