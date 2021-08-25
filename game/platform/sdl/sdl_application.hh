#pragma once

#include <stdint.h>

#include <SDL2/SDL.h>

#include "../../common.hh"

class SDLApplication
{
private:
	bool _isAppRunning;
	static uint32_t _numInstances;

	SDLApplication(); // private constructor
public:
	static SDLApplication* Create();

	virtual ~SDLApplication();
	virtual void ProcessMessages(double delta);
	virtual bool IsRunning();
};