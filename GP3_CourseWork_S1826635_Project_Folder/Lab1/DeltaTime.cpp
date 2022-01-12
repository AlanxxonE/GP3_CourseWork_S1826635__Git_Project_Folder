#include "DeltaTime.h"

void DeltaTime::InitDeltaTime()
{
	deltaTime = 0;
	previousTick = 0;
	currentTick = SDL_GetPerformanceCounter();
}

void DeltaTime::UpdateDeltaTime()
{
	previousTick = currentTick;
	currentTick = SDL_GetPerformanceCounter();

	deltaTime = (double)((currentTick - previousTick) / (double)SDL_GetPerformanceFrequency());
}

double DeltaTime::GetDeltaTime()
{
	return deltaTime;
}