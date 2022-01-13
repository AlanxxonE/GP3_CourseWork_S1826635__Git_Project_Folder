#pragma once
#include <GL\glew.h>
#include <SDL\SDL.h>
#include <glm/glm.hpp>
#include "Camera.h"
#include "Display.h"

class RayCaster
{
public:

	void initRayCaster(glm::mat4 projMat, glm::mat4 viewMat);

	glm::vec3 GetCurrentRay()
	{
		return currentRay;
	}

	void UpdateRay(glm::mat4 viewMat, float displayWidth, float displayHeight);

	glm::vec3 CalculateMouseRay(float displayWidth, float displayHeight);

	glm::vec2 GetNormalizedDeviceCoords(int* mouseX, int* mouseY, float displayWidth, float displayHeight);

	glm::vec4 ConvertToEyeCoords(glm::vec4 clipCoords);

	glm::vec3 ConvertToWorldCoords(glm::vec4 eyeCoords);

protected:
private:

	glm::vec3 currentRay;
	glm::mat4 projMatrix;
	glm::mat4 viewMatrix;

	GLint mouseX, mouseY;
};

