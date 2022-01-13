#include "RayCaster.h"

void RayCaster::initRayCaster(glm::mat4 projMat, glm::mat4 viewMat)
{
	this->projMatrix = projMat;
	this->viewMatrix = viewMat;
}

void RayCaster::UpdateRay(glm::mat4 viewMat, float displayWidth, float displayHeight)
{
	viewMatrix = viewMat;
	currentRay = CalculateMouseRay(displayWidth, displayHeight);
}

glm::vec3 RayCaster::CalculateMouseRay(float displayWidth, float displayHeight)
{
	SDL_PumpEvents();
	SDL_GetMouseState(&mouseX, &mouseY);

	glm::vec2 normalizedCoord = GetNormalizedDeviceCoords(&mouseX, &mouseY, displayWidth, displayHeight);
	glm::vec4 clipCoords = glm::vec4(normalizedCoord.x, normalizedCoord.y, -1.0f, 1.0f);
	glm::vec4 eyeCoords = ConvertToEyeCoords(clipCoords);
	glm::vec3 worldRay = ConvertToWorldCoords(eyeCoords);

	return worldRay;
}

glm::vec2 RayCaster::GetNormalizedDeviceCoords(int* mouseX, int* mouseY, float displayWidth, float displayHeight)
{
	float x = (2.0f * *mouseX) / displayWidth - 1;
	float y = (2.0f * *mouseY) / displayHeight - 1.0f;

	return glm::vec2(x, -y);
}

glm::vec4 RayCaster::ConvertToEyeCoords(glm::vec4 clipCoords)
{
	glm::mat4 invertedProjection = glm::inverse(projMatrix);
	glm::vec4 eyeCoords = invertedProjection * clipCoords;

	return glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);
}

glm::vec3 RayCaster::ConvertToWorldCoords(glm::vec4 eyeCoords)
{
	glm::mat4 invertedView = glm::inverse(viewMatrix);
	glm::vec4 rayWorld = invertedView * eyeCoords;
	glm::vec3 mouseRay = glm::vec3(rayWorld.x, rayWorld.y, rayWorld.z);

	glm::vec3 normalizedMouseRay = glm::normalize(mouseRay);

	return normalizedMouseRay;
}
