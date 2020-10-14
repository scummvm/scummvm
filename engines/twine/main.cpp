/** @file main.cpp
	@brief
	This file contains the main engine functions.
	It also contains configurations file definitions.

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "main.h"
#include "resources.h"
#include "sdlengine.h"
#include "screens.h"
#include "music.h"
#include "menu.h"
#include "interface.h"
#include "flamovies.h"
#include "hqrdepack.h"
#include "scene.h"
#include "grid.h"
#include "lbaengine.h"
#include "redraw.h"
#include "text.h"
#include "renderer.h"
#include "animations.h"
#include "gamestate.h"
#include "keyboard.h"
#include "actor.h"
#include "sound.h"
#include "fcaseopen.h"

/** Engine current version */
const int8* ENGINE_VERSION = (int8*) "0.2.0";

/** Engine configuration filename */
const int8* CONFIG_FILENAME = (int8*) "lba.cfg";

/** Engine install setup filename

	This file contains informations about the game version.
	This is only used for original games. For mod games project you can
	used \a lba.cfg file \b Version tag. If this tag is set for original game
	it will be used instead of \a setup.lst file. */
const int8* SETUP_FILENAME = (int8*) "setup.lst";

/** Configuration types at \a lba.cfg file

	Fill this with all needed configurations at \a lba.cfg file.
	This engine version allows new type of configurations.
	Check new config lines at \a lba.cfg file after the first game execution */
int8 CFGList[][22] = {
	"Language:",
	"LanguageCD:",
	"FlagDisplayText:",
	"FlagKeepVoice:",
	"SvgaDriver:",
	"MidiDriver:",
	"MidiExec:",
	"MidiBase:",
	"MidiType:",
	"MidiIRQ:",
	"MidiDMA:", // 10
	"WaveDriver:",
	"WaveExec:",
	"WaveBase:",
	"WaveIRQ:",
	"WaveDMA:",
	"WaveRate:",
	"MixerDriver:",
	"MixerBase:",
	"WaveVolume:",
	"VoiceVolume:", // 20
	"MusicVolume:",
	"CDVolume:",
	"LineVolume:",
	"MasterVolume:",
	"Version:",
	"FullScreen:",
	"UseCD:",
	"Sound:",
	"Movie:",
	"CrossFade:", // 30
	"Fps:",
	"Debug:",
	"UseAutoSaving:",
	"CombatAuto:",
	"Shadow:",
	"SceZoom:",
	"FillDetails:",
	"InterfaceStyle",
	"WallCollision" // 39
};

int8 LanguageTypes[][10] = {
	"English",
	"Français",
	"Deutsch",
	"Español",
	"Italiano",
	"Português"
};
ConfigFile cfgfile;

/** Allocate video memory, both front and back buffers */
void allocVideoMemory() {
	int32 i, j, k;

	workVideoBuffer = (uint8 *) malloc((SCREEN_WIDTH * SCREEN_HEIGHT) * sizeof(uint8));
	frontVideoBuffer = frontVideoBufferbis = (uint8 *) malloc(sizeof(uint8) * SCREEN_WIDTH * SCREEN_HEIGHT);
	initScreenBuffer(frontVideoBuffer, SCREEN_WIDTH, SCREEN_HEIGHT);

	j = 0;
	k = 0;
	for (i = SCREEN_HEIGHT; i > 0; i--) {
		screenLookupTable[j] = k;
		j++;
		k += SCREEN_WIDTH;
	}

	// initVideoVar1 = -1;
}

/** Gets configuration type index from lba.cfg config file
	@param lineBuffer buffer with config line
	@return config type index */
int getConfigTypeIndex(int8* lineBuffer) {
	int32 i;
	char buffer[256];
	char* ptr;

	strcpy(buffer, lineBuffer);

	ptr = strchr(buffer, ' ');

	if (ptr) {
		*ptr = 0;
	}

	for (i = 0; i < (sizeof(CFGList) / 22); i++) {
		if (strlen(CFGList[i])) {
			if (!strcmp(CFGList[i], buffer)) {
				return i;
			}
		}
	}

	return -1;
}

/** Gets configuration type index from lba.cfg config file
	@param lineBuffer buffer with config line
	@return config type index */
int getLanguageTypeIndex(int8* language) {
	int32 i;
	char buffer[256];
	char* ptr;

	strcpy(buffer, language);

	ptr = strchr(buffer, ' ');

	if (ptr) {
		*ptr = 0;
	}

	for (i = 0; i < (sizeof(LanguageTypes) / 10); i++) {
		if (strlen(LanguageTypes[i])) {
			if (!strcmp(LanguageTypes[i], buffer)) {
				return i;
			}
		}
	}

	return 0; // English
}

/** Init configuration file \a lba.cfg */
void initConfigurations() {
	FILE *fd, *fd_test;
	int8 buffer[256], tmp[16];
	int32 cfgtype = -1;

	fd = fcaseopen(CONFIG_FILENAME, "rb");
	if (!fd)
		printf("Error: Can't find config file %s\n", CONFIG_FILENAME);

	// make sure it quit when it reaches the end of file
	while (fgets(buffer, 256, fd) != NULL) {
		*strchr(buffer, 0x0D0A) = 0;
		cfgtype = getConfigTypeIndex(buffer);
		if (cfgtype != -1) {
			switch (cfgtype) {
			case 0:
				sscanf(buffer, "Language: %s", cfgfile.Language);
				cfgfile.LanguageId = getLanguageTypeIndex(cfgfile.Language);
				break;
			case 1:
				sscanf(buffer, "LanguageCD: %s", cfgfile.LanguageCD);
				cfgfile.LanguageCDId = getLanguageTypeIndex(cfgfile.Language) + 1;
				break;
			case 2:
				sscanf(buffer, "FlagDisplayText: %s", cfgfile.FlagDisplayTextStr);
				if (!strcmp(cfgfile.FlagDisplayTextStr,"ON")) {
					cfgfile.FlagDisplayText = 1;
				} else {
					cfgfile.FlagDisplayText = 0;
				}
				break;
			case 3:
				sscanf(buffer, "FlagKeepVoice: %s", cfgfile.FlagKeepVoiceStr);
				break;
			case 8:
				sscanf(buffer, "MidiType: %s", tmp);
				if (strcmp(tmp, "auto") == 0) {
					fd_test = fcaseopen(HQR_MIDI_MI_WIN_FILE, "rb");
					if (fd_test) {
						fclose(fd_test);
						cfgfile.MidiType = 1;
					}
					else
						cfgfile.MidiType = 0;
				}
				else if (strcmp(tmp, "midi") == 0)
					cfgfile.MidiType = 1;
				else
					cfgfile.MidiType = 0;
				break;
			case 19:
				sscanf(buffer, "WaveVolume: %d", &cfgfile.WaveVolume);
				cfgfile.VoiceVolume = cfgfile.WaveVolume;
				break;
			case 20:
				sscanf(buffer, "VoiceVolume: %d", &cfgfile.VoiceVolume);
				break;
			case 21:
				sscanf(buffer, "MusicVolume: %d", &cfgfile.MusicVolume);
				break;
			case 22:
				sscanf(buffer, "CDVolume: %d", &cfgfile.CDVolume);
				break;
			case 23:
				sscanf(buffer, "LineVolume: %d", &cfgfile.LineVolume);
				break;
			case 24:
				sscanf(buffer, "MasterVolume: %d", &cfgfile.MasterVolume);
				break;
			case 25:
				sscanf(buffer, "Version: %d", &cfgfile.Version);
				break;
			case 26:
				sscanf(buffer, "FullScreen: %d", &cfgfile.FullScreen);
				break;
			case 27:
				sscanf(buffer, "UseCD: %d", &cfgfile.UseCD);
				break;
			case 28:
				sscanf(buffer, "Sound: %d", &cfgfile.Sound);
				break;
			case 29:
				sscanf(buffer, "Movie: %d", &cfgfile.Movie);
				break;
			case 30:
				sscanf(buffer, "CrossFade: %d", &cfgfile.CrossFade);
				break;
			case 31:
				sscanf(buffer, "Fps: %d", &cfgfile.Fps);
				break;
			case 32:
				sscanf(buffer, "Debug: %d", &cfgfile.Debug);
				break;
			case 33:
				sscanf(buffer, "UseAutoSaving: %d", &cfgfile.UseAutoSaving);
				break;
			case 34:
				sscanf(buffer, "CombatAuto: %d", &cfgfile.AutoAgressive);
				break;
			case 35:
				sscanf(buffer, "Shadow: %d", &cfgfile.ShadowMode);
				break;
			case 36:
				sscanf(buffer, "SceZoom: %d", &cfgfile.SceZoom);
				break;
			case 37:
				sscanf(buffer, "FillDetails: %d", &cfgfile.FillDetails);
				break;
			case 38:
				sscanf(buffer, "InterfaceStyle: %d", &cfgfile.InterfaceStyle);
				break;
			case 39:
				sscanf(buffer, "WallCollision: %d", &cfgfile.WallCollision);
				break;
			}
		}
	}

	if (!cfgfile.Fps)
		cfgfile.Fps = DEFAULT_FRAMES_PER_SECOND;

	fclose(fd);
}

/** Initialize LBA engine */
void initEngine() {
	// getting configuration file
	initConfigurations();

	// Show engine information
	printf("TwinEngine v%s\n\n", ENGINE_VERSION);
	printf("(c)2002 The TwinEngine team. Refer to AUTHORS file for further details.\n");
	printf("Released under the terms of the GNU GPL license version 2 (or, at your opinion, any later). See COPYING file.\n\n");
	printf("The original Little Big Adventure game is:\n\t(c)1994 by Adeline Software International, All Rights Reserved.\n\n");

	if (cfgfile.Debug)
		printf("Compiled the %s at %s\n", __DATE__, __TIME__);

	sdlInitialize();

	srand(SDL_GetTicks()); // always get a different seed while starting the game

	allocVideoMemory();
	clearScreen();

	// Toggle fullscreen if Fullscreen flag is set
	toggleFullscreen();

	// Check if LBA CD-Rom is on drive
	initCdrom();

#ifndef _DEBUG

	// Display company logo
	adelineLogo();

	// verify game version screens
	if (cfgfile.Version == EUROPE_VERSION) {
		// Little Big Adventure screen
		loadImageDelay(RESSHQR_LBAIMG, 3);
		// Electronic Arts Logo
		loadImageDelay(RESSHQR_EAIMG, 2);
	} else if (cfgfile.Version == USA_VERSION) {
		// Relentless screen
		loadImageDelay(RESSHQR_RELLENTIMG, 3);
		// Electronic Arts Logo
		loadImageDelay(RESSHQR_EAIMG, 2);
	} else if (cfgfile.Version == MODIFICATION_VERSION) {
		// Modification screen
		loadImageDelay(RESSHQR_RELLENTIMG, 2);
	}

	playFlaMovie(FLA_DRAGON3);

#endif

	loadMenuImage(1);

	mainMenu();
}

void initMCGA() {
    drawInGameTransBox = 1;
}

void initSVGA() {
    drawInGameTransBox = 0;
}

/** Initialize all needed stuffs at first time running engine */
void initAll() {
	blockBuffer = (uint8 *) malloc(64*64*25*2 * sizeof(uint8));
	animBuffer1 = animBuffer2 = (uint8 *) malloc(5000 * sizeof(uint8));
	memset(samplesPlaying, -1, sizeof(int32) * NUM_CHANNELS);
	memset(itemAngle, 256, sizeof(itemAngle)); // reset inventory items angles

    bubbleSpriteIndex = SPRITEHQR_DIAG_BUBBLE_LEFT;
    bubbleActor = -1;
    showDialogueBubble = 1;

	currentTextBank = -1;
	currMenuTextIndex = -1;
	currMenuTextBank = -1;
	autoAgressive = 1;

	sceneHero = &sceneActors[0];

	renderLeft = 0;
	renderTop = 0;
	renderRight = SCREEN_TEXTLIMIT_RIGHT;
	renderBottom = SCREEN_TEXTLIMIT_BOTTOM;
	// Set clip to fullscreen by default, allows main menu to render properly after load
	resetClip();

	rightMouse = 0;
	leftMouse = 0;

	initResources();

    initSVGA();
}

/** Main engine function
	@param argc numner of arguments
	@param argv array with all arguments strings */
int main(int argc, char *argv[]) {
	initAll();
	initEngine();
	sdlClose();
	printf("\n\nLBA/Relentless < %s / %s >\n\nOK.\n\n", __DATE__, __TIME__);
	printf("TwinEngine v%s closed\n", ENGINE_VERSION);
	if (cfgfile.Debug) {
		printf("\nPress <ENTER> to quit debug mode\n");
		getchar();
	}
	return 0;
}

// AUX FUNC

int8* ITOA(int32 number) {
	int32 numDigits = 1;
	int8* text;

	if (number >=10 && number <= 99) {
		numDigits = 2;
	} else if (number >=100 && number <= 999) {
		numDigits = 3;
	}

	text = (int8 *)malloc(sizeof(int8) * (numDigits + 1));
	sprintf(text, "%d", number);
	return text;
}


/** \mainpage Twinsen's Engine Doxxy Documentation

	\section intro_sec Introduction

	TwinEngine is a reimplementation project upon the popular
	Little Big Adventure games released respectively in
	1994 (Relentless in North America) and 	1997 (Twinsen's Odyssey).

	\section doc_sec Documentation

	This document, allows you to easily search for particulary things among
	the code. We plan to comment as best as we can and with the most necessary
	informations. The source code is also shared in this document and you can
	use it in the terms of the license.

	Feel free to contact us if you wanna help improving this documentation and
	the engine itself.

	\section copy_sec Copyright

	Copyright (c) Adeline Software International 1994, All Rights Reserved.\n
	Copyright (c) 2002-2007 The TwinEngine team.\n
	Copyright (c) 2008-2013 Prequengine team \n
	Copyright (c) 2013 The TwinEngine team

	\section licenc_sec License

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

	For a full license, check the license file in source code.
*/
