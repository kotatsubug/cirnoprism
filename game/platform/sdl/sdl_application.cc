#include "sdl_application.hh"

uint32_t SDLApplication::_numInstances = 0;

SDLApplication* SDLApplication::Create()
{
	const uint32_t flags = SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS;
	uint32_t wasInit = SDL_WasInit(flags);
	if ((wasInit != flags) && (SDL_Init(flags) != 0))
	{
		DEBUG_LOG("SDLApplication", LOG_FATAL, "SDL_Init: %s", SDL_GetError());
	}

	return new SDLApplication();
}

SDLApplication::SDLApplication() // private
{
	_numInstances++;
	_isAppRunning = true;
}

SDLApplication::~SDLApplication()
{
	_numInstances--;
	if (_numInstances == 0)
	{
		SDL_Quit();
	}
}

void SDLApplication::ProcessMessages(double delta)
{
	SDL_Event e;
	(void)delta;

	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case SDL_QUIT:
			_isAppRunning = false;
			break;
		default:
			break;
		};
	}
}

bool SDLApplication::IsRunning()
{
	return _isAppRunning;
}