#include "stdafx.h"
#include "scumm.h"
#include "gameDetector.h"
#include "gui.h"
#include "simon/simon.h"

GameDetector detector;
Gui gui;

Scumm *g_scumm;
SoundEngine sound;
SOUND_DRIVER_TYPE snd_driv;


#if !defined(__APPLE__)
#undef main
#endif

int main(int argc, char *argv[])
{
#if defined(MACOS)
	/* support for config file on macos */

	char *argitem;
	char *argstr;
	FILE *argf;

	if ((argf = fopen("configuration.macos", "r")) == NULL) {
		error("Can't open configuration file.\n");
		exit(1);
	}

	argc = 0;
	argstr = (char *)malloc(64);
	argstr = fgets(argstr, 64, argf);
	if ((argitem = strchr(argstr, '\n')) != NULL)
		*argitem = '\0';

	argitem = strtok(argstr, " ");

	while (argitem != NULL) {
		argv = (char **)realloc(argv, (argc + 1) * 8);
		argv[argc] = (char *)malloc(64);
		strcpy(argv[argc], argitem);
		argc++;

		argitem = strtok(NULL, " ");
	}

	free(argstr);
	fclose(argf);

#endif

	if (detector.detectMain(argc, argv))
		return (-1);

	OSystem *system = OSystem_SDL_create(detector._gfx_mode, detector._fullScreen);

	{
		char *s = detector.getGameName();
		system->set_param(OSystem::PARAM_WINDOW_CAPTION, (long)s);
		free(s);
	}

	/* Simon the Sorcerer? */
	if (detector._gameId >= GID_SIMON_FIRST && detector._gameId <= GID_SIMON_LAST) {
		/* Simon the Sorcerer. Completely different initialization */
		SimonState *simon = SimonState::create();
		simon->_game = detector._gameId - GID_SIMON_FIRST;
		simon->go(system);

	} else {
		Scumm *scumm = Scumm::createFromDetector(&detector, system);
		g_scumm = scumm;

		sound.initialize(scumm, &snd_driv);
		
		/* bind to Gui */
		scumm->_gui = &gui;
		gui.init(scumm);							/* Reinit GUI after loading a game */

		scumm->go();
	}
	
	return 0;
}
