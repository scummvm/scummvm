/** @file main.h
	@brief
	This file contains the main engine functions. It also contains
	configurations file definitions.

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

#ifndef MAIN_H
#define MAIN_H

#include "sys.h"

/** Definition for European version */
#define EUROPE_VERSION 0
/** Definition for American version */
#define USA_VERSION 1
/** Definition for Modification version */
#define MODIFICATION_VERSION 2

/** Original screen width */
#define DEFAULT_SCREEN_WIDTH			640
/** Original screen height */
#define DEFAULT_SCREEN_HEIGHT			480
/** Scale screen to double size */
#define SCALE					1
/** Original screen width */
#define SCREEN_WIDTH			DEFAULT_SCREEN_WIDTH * SCALE
/** Original screen height */
#define SCREEN_HEIGHT			DEFAULT_SCREEN_HEIGHT * SCALE
/** Original FLA screen width */
#define FLASCREEN_WIDTH			320
/** Original FLA screen height */
#define FLASCREEN_HEIGHT		200
/** Default frames per second */
#define DEFAULT_FRAMES_PER_SECOND	19

/** Number of colors used in the game */
#define NUMOFCOLORS		256


/** Configuration file structure

	Used in the engine to load/use certain parts of code according with
	this settings. Check \a lba.cfg file for valid values for each settings.\n
	All the settings with (*) means they are new and only exist in this engine. */
typedef struct ConfigFile {
	/** Language name */
	int8 Language[10];
	/** Language CD name */
	int8 LanguageCD[10];
	/** Language Identification according with Language setting. */
	int32 LanguageId;
	/** Language Identification according with Language setting. */
	int32 LanguageCDId;
	/** Enable/Disable game dialogues */
	int8 FlagDisplayTextStr[3];
	/** Enable/Disable game dialogues */
	int32 FlagDisplayText;
	/** Save voice files on hard disk */
	int8 FlagKeepVoiceStr[3];
	/** Save voice files on hard disk */
	int32 FlagKeepVoice;
	/** Type of music file to be used */
	int8 MidiType;
	/** Wave volume */
	int32 WaveVolume;
	/** Chacters voices volume */
	int32 VoiceVolume;
	/** Music volume */
	int32 MusicVolume;
	/** CD volume */
	int32 CDVolume;
	/** Line-In volume */
	int32 LineVolume;
	/** Main volume controller */
	int32 MasterVolume;
	/** *Game version */
	int32 Version;
	/** To allow fullscreen or window mode. */
	int32 FullScreen;
	/** If you want to use the LBA CD or not */
	int32 UseCD;
	/** Allow various sound types */
	int32 Sound;
	/** Allow various movie types */
	int32 Movie;
	/** Use cross fade effect while changing images, or be as the original */
	int32 CrossFade;
	/** Flag used to keep the game frames per second */
	int32 Fps;
	/** Flag to display game debug */
	int32 Debug;
	/** Use original autosaving system or save when you want */
	int32 UseAutoSaving;
	/** Shadow mode type */
	int32 ShadowMode;
	/** AutoAgressive mode type */
	int32 AutoAgressive;
	/** SceZoom mode type */
	int32 SceZoom;
	/** FillDetails mode type */
	int32 FillDetails;
	/** Flag to quit the game */
	int32 Quit;
	/** Flag to change interface style */
	int32 InterfaceStyle;
	/** Flag to toggle Wall Collision */
	int32 WallCollision;
} ConfigFile;

/** Configuration file structure

	Contains all the data used in the engine to configurated the game in particulary ways.\n
	A detailled information of all types are in \a main.h header file. */
extern ConfigFile cfgfile;

/** CD Game directory */
int8 * cdDir;

void initMCGA();
void initSVGA();

int8* ITOA(int32 number);

#endif
