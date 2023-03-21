#pragma once

#include "common_header.h"
#include "gamestates.h"
#include "ImageManager.h"
#include "MusicManager.h"
#include "timer.h"
#include "splash.h"
#include "mainmenu.h"
#include "game.h"

class App
{
	void LoadSettings(const std::string &filename);

public:
	App(void)
	{
#ifdef __APPLE__
		//OS X .app files need to set the working directory
		char *working_directory = SDL_GetBasePath();
		chdir(working_directory);
		SDL_free(working_directory);
#endif
	}
	~App(void);

	bool Init();
	void Run();
};