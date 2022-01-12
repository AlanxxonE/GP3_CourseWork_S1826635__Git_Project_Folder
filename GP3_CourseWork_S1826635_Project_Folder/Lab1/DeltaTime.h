#pragma once
#include <SDL\SDL.h>

struct DeltaTime
{
public:
	void InitDeltaTime();

	void UpdateDeltaTime();

	double GetDeltaTime();

protected:
private:
	Uint64 previousTick;
	Uint64 currentTick;
	double deltaTime;
};

