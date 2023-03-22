#pragma once

#include "ImageManager.h"
#include "MusicManager.h"
#include "common_header.h"
#include "game.h"
#include "gamestates.h"
#include "mainmenu.h"
#include "splash.h"
#include "timer.h"

class App {
	void LoadSettings(const std::string &filename);

public:
	App(void) {
#ifdef __APPLE__
		// OS X .app files need to set the working directory
		char *working_directory = SDL_GetBasePath();
		chdir(working_directory);
		SDL_free(working_directory);
#endif
	}
	~App(void);

	bool Init();
	void Run();
};