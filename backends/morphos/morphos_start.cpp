/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 Rüdiger Hanke (MorphOS port)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * MorphOS startup handling
 *
 * $Header$
 *
 */

#include <exec/types.h>
#include <exec/devices.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/cdda.h>
#include <proto/icon.h>

#include "stdafx.h"
#include "scumm/scumm.h"
#include "common/scaler.h"
#include "sound/mididrv.h"
#include "morphos.h"
#include "morphos_scaler.h"
#include "morphos_sound.h"

extern "C" WBStartup *_WBenchMsg;

// For command line parsing
static STRPTR usageTemplate = "STORY,DATAPATH/K,WINDOW/S,SCALER/K,AMIGA/S,MIDIUNIT/K/N,MUSIC/K,MASTERVOL/K/N,MUSICVOL/K/N,SFXVOL/K/N,TEMPO/K/N,TALKSPEED/K/N,LANGUAGE/K,NOSUBTITLES=NST/S, DEBUGLEVEL=DBGLVL/K/N, DUMPSCRIPTS/S";
typedef enum { USG_STORY = 0, USG_DATAPATH, USG_WINDOW, USG_SCALER, USG_AMIGA, USG_MIDIUNIT, USG_MUSIC, USG_MASTERVOL, USG_MUSICVOL, USG_SFXVOL, USG_TEMPO, USG_TALKSPEED, USG_LANGUAGE, USG_NOSUBTITLES, USG_DEBUGLEVEL, USG_DUMPSCRIPTS, USG_MAX } usageFields;
static LONG	args[USG_MAX];
static RDArgs *ScummArgs = NULL;

static char*ScummStory = NULL;
static char*ScummPath = NULL;
static char*ScummLang = NULL;
	  STRPTR ScummMusicDriver = NULL;
MidiDriver* EtudeMidiDriver = NULL;
		 LONG ScummMidiUnit = 0;
static LONG ScummMasterVolume = 0;
static LONG ScummMidiVolume = 0;
static LONG ScummMidiTempo = 0;
static LONG ScummSfxVolume = 0;
static LONG ScummTalkSpeed = 0;
static LONG ScummDebugLevel = 0;
static SCALERTYPE ScummGfxScaler = ST_INVALID;

static BPTR OrigDirLock = 0;

Library *CDDABase = NULL;
Library *TimerBase = NULL;

OSystem_MorphOS *TheSystem = NULL;

OSystem *OSystem_MorphOS_create(int game_id, int gfx_mode, bool full_screen)
{
	if (TheSystem)
		delete TheSystem;

	SCALERTYPE gfx_scaler = ST_NONE;
	switch (gfx_mode)
	{
		case GFX_DOUBLESIZE:
			gfx_scaler = ST_POINT;
			break;

		case GFX_SUPEREAGLE:
			gfx_scaler = ST_SUPEREAGLE;
			break;

		case GFX_SUPER2XSAI:
			gfx_scaler = ST_SUPER2XSAI;
			break;
	}

	TheSystem = OSystem_MorphOS::create(game_id, gfx_scaler, full_screen);
	return TheSystem;
}

void close_resources()
{
	if (TheSystem)
		delete TheSystem;

	if (g_scumm)
		delete g_scumm;

	if (ScummPath)
		FreeVec(ScummPath);

	if (ScummStory)
		FreeVec(ScummStory);

	if (ScummArgs)
		FreeArgs(ScummArgs);

	if (OrigDirLock)
		CurrentDir(OrigDirLock);

	if (CDDABase)
		CloseLibrary(CDDABase);
}

static STRPTR FindMusicDriver(STRPTR argval)
{
	if (!stricmp(argval, "off"))  	return "-enull";
	if (!stricmp(argval, "midi"))	 	return "-eetude";
	if (!stricmp(argval, "adlib")) 	return "-eadlib";

	error("No such music driver supported. Possible values are off, Midi and Adlib.");
	return NULL;
}

static void ReadToolTypes(WBArg *OfFile)
{
	DiskObject *dobj;
	char *ToolValue;
	char IconPath[256];

	NameFromLock(OfFile->wa_Lock, IconPath, 256);
	AddPart(IconPath, (STRPTR) OfFile->wa_Name, 256);

	dobj = GetDiskObject(IconPath);
	if (dobj == NULL)
		return;

	ToolValue = (char *) FindToolType(dobj->do_ToolTypes, "STORY");
	if (ToolValue)
	{
		if (ScummStory)
			FreeVec(ScummStory);
		ScummStory = (char *) AllocVec(strlen(ToolValue)+1, MEMF_PUBLIC);
		strcpy(ScummStory, ToolValue);
	}

	ToolValue = (char *) FindToolType(dobj->do_ToolTypes, "DATAPATH");
	if (ToolValue)
	{
		if (ScummPath)
			FreeVec(ScummPath);
		ScummPath = (char *) AllocVec(strlen(ToolValue)+4, MEMF_PUBLIC);
		strcpy(ScummPath, "-p");
		strcat(ScummPath, ToolValue);
	}

	ToolValue = (char *) FindToolType(dobj->do_ToolTypes, "WINDOW");
	if (ToolValue)
	{
		if (MatchToolValue(ToolValue, "YES"))
			args[USG_WINDOW] = TRUE;
		else if (MatchToolValue(ToolValue, "NO"))
			args[USG_WINDOW] = FALSE;
	}

	ToolValue = (char *) FindToolType(dobj->do_ToolTypes, "SCALER");
	if (ToolValue)
	{
		if ((ScummGfxScaler = MorphOSScaler::FindByName(ToolValue)) == ST_INVALID)
		{
			FreeDiskObject(dobj);
			exit(1);
		}
	}

	ToolValue = (char *) FindToolType(dobj->do_ToolTypes, "MUSIC");
	if (ToolValue)
	{
		if (!(ScummMusicDriver = FindMusicDriver(ToolValue)))
		{
			FreeDiskObject(dobj);
			exit(1);
		}
		args[USG_MUSIC] = (ULONG) &ScummMusicDriver;
	}

	ToolValue = (char *) FindToolType(dobj->do_ToolTypes, "MIDIUNIT");
	if (ToolValue)
		ScummMidiUnit = atoi(ToolValue);

	ToolValue = (char *) FindToolType(dobj->do_ToolTypes, "MASTERVOL");
	if (ToolValue)
	{
		int vol = atoi(ToolValue);
		if (vol >= 0 && vol <= 100)
		{
			ScummMasterVolume = vol;
			args[USG_MASTERVOL] = (ULONG) &ScummMasterVolume;
		}
	}

	ToolValue = (char *) FindToolType(dobj->do_ToolTypes, "MUSICVOL");
	if (ToolValue)
	{
		int vol = atoi(ToolValue);
		if (vol >= 0 && vol <= 100)
		{
			ScummMidiVolume = vol;
			args[USG_MUSICVOL] = (ULONG) &ScummMidiVolume;
		}
	}

	ToolValue = (char *) FindToolType(dobj->do_ToolTypes, "SFXVOL");
	if (ToolValue)
	{
		int vol = atoi(ToolValue);
		if (vol >= 0 && vol <= 255)
		{
			ScummSfxVolume = vol;
			args[USG_SFXVOL] = (ULONG) &ScummSfxVolume;
		}
	}

	ToolValue = (char *) FindToolType(dobj->do_ToolTypes, "TEMPO");
	if (ToolValue)
	{
		ScummMidiTempo = atoi(ToolValue);
		args[USG_TEMPO] = (ULONG) &ScummMidiTempo;
	}

	ToolValue = (char *) FindToolType(dobj->do_ToolTypes, "TALKSPEED");
	if (ToolValue)
	{
		ScummTalkSpeed = atoi(ToolValue);
		args[USG_TALKSPEED] = (ULONG) &ScummMidiTempo;
	}

	ToolValue = (char *) FindToolType(dobj->do_ToolTypes, "LANGUAGE");
	if (ToolValue)
	{
		if (ScummLang)
			FreeVec(ScummLang);
		ScummLang = (char *) AllocVec(strlen(ToolValue)+4, MEMF_PUBLIC);
		strcpy(ScummLang, "-q");
		strcat(ScummLang, ToolValue);
	}

	ToolValue = (char *) FindToolType(dobj->do_ToolTypes, "SUBTITLES");
	if (ToolValue)
	{
		if (MatchToolValue(ToolValue, "YES"))
			args[USG_NOSUBTITLES] = FALSE;
		else if (MatchToolValue(ToolValue, "NO"))
			args[USG_NOSUBTITLES] = TRUE;
	}

	ToolValue = (char *) FindToolType(dobj->do_ToolTypes, "AMIGA");
	if (ToolValue)
	{
		if (MatchToolValue(ToolValue, "YES"))
			args[USG_AMIGA] = FALSE;
		else if (MatchToolValue(ToolValue, "NO"))
			args[USG_AMIGA] = TRUE;
	}

	ToolValue = (char *) FindToolType(dobj->do_ToolTypes, "DEBUGLEVEL");
	if (ToolValue)
		ScummDebugLevel = atoi(ToolValue);

	ToolValue = (char *) FindToolType(dobj->do_ToolTypes, "DUMPSCRIPTS");
	if (ToolValue)
	{
		if (MatchToolValue(ToolValue, "YES"))
			args[USG_DUMPSCRIPTS] = TRUE;
		else if (MatchToolValue(ToolValue, "NO"))
			args[USG_DUMPSCRIPTS] = FALSE;
	}

	FreeDiskObject(dobj);
}

#undef main

int main()
{
	char *argv[30];
	char mastervol[6], musicvol[6], sfxvol[6], talkspeed[12], tempo[12], scaler[14];
	char dbglvl[6];
	int argc = 0;

	InitSemaphore(&ScummSoundThreadRunning);
	InitSemaphore(&ScummMusicThreadRunning);

	g_scumm = NULL;
	atexit(&close_resources);

	memset(args, '\0', sizeof (args));
	if (_WBenchMsg == NULL)
	{
		/* Parse the command line here */
		ScummArgs = ReadArgs(usageTemplate, args, NULL);
		if (ScummArgs == NULL)
		{
			puts("Error in command line - type \"ScummVM ?\" for usage.");
			exit(1);
		}

		if (args[USG_STORY])
		{
			ScummStory = (char *) AllocVec(strlen((char *) args[USG_STORY])+1, MEMF_PUBLIC);
			strcpy(ScummStory, (char *) args[USG_STORY]);
		}

		if (args[USG_DATAPATH])
		{
			ScummPath = (char *) AllocVec(strlen((char *) args[USG_DATAPATH])+4, MEMF_PUBLIC);
			strcpy(ScummPath, "-p");
			strcat(ScummPath, (char *) args[USG_DATAPATH]);
		}

		if (args[USG_SCALER])
		{
			if ((ScummGfxScaler = MorphOSScaler::FindByName((char *) args[USG_SCALER])) == ST_INVALID)
				exit(1);
		}

		if (args[USG_MUSIC])
		{
			if (!(ScummMusicDriver = FindMusicDriver((char *) args[USG_MUSIC])))
				exit(1);
		}

		if (args[USG_MIDIUNIT])
			ScummMidiUnit = *((LONG *) args[USG_MIDIUNIT]);

		if (args[USG_TEMPO])
			ScummMidiTempo = *((LONG *) args[USG_TEMPO]);

		if (args[USG_MASTERVOL])
			ScummMasterVolume = *((LONG *) args[USG_MASTERVOL]);

		if (args[USG_MUSICVOL])
			ScummMidiVolume = *((LONG *) args[USG_MUSICVOL]);

		if (args[USG_SFXVOL])
			ScummSfxVolume = *((LONG *) args[USG_SFXVOL]);

		if (args[USG_TALKSPEED])
			ScummTalkSpeed = *((LONG *) args[USG_TALKSPEED]);

		if (args[USG_LANGUAGE])
		{
			ScummLang = (char *) AllocVec(strlen((char *) args[USG_LANGUAGE])+4, MEMF_PUBLIC);
			strcpy(ScummLang, "-q");
			strcat(ScummLang, (char *) args[USG_LANGUAGE]);
		}

		if (args[USG_DEBUGLEVEL])
			ScummDebugLevel = *((LONG *) args[USG_DEBUGLEVEL]);
	}
	else
	{
		/* We've been started from Workbench */
		ReadToolTypes(&_WBenchMsg->sm_ArgList[0]);
		if (_WBenchMsg->sm_NumArgs > 1)
		{
			ReadToolTypes(&_WBenchMsg->sm_ArgList[1]);
			OrigDirLock = CurrentDir(_WBenchMsg->sm_ArgList[1].wa_Lock);
		}
	}

	if (ScummPath)
	{
		char c = ScummPath[strlen(ScummPath)-1];
		if (c != '/' && c != ':')
			strcat(ScummPath, "/");
	}

	argv[argc++] = "ScummVM";
	if (ScummPath) 				argv[argc++] = ScummPath;
	if (!args[USG_WINDOW]) 		argv[argc++] = "-f";
	if (args[USG_NOSUBTITLES]) argv[argc++] = "-n";
	if (args[USG_AMIGA]) 		argv[argc++] = "-a";
	if (args[USG_MUSIC]) 		argv[argc++] = ScummMusicDriver;
	if (ScummGfxScaler != ST_INVALID)
	{
		sprintf(scaler, "-g%s", MorphOSScaler::GetParamName(ScummGfxScaler));
		argv[argc++] = scaler;
	}
	else
		argv[argc++] = "-gsuper2xsai";
	if (args[USG_MASTERVOL] && ScummMasterVolume >= 0 && ScummMasterVolume <= 255)
	{
		sprintf(mastervol, "-o%ld", ScummMasterVolume);
		argv[argc++] = mastervol;
	}
	if (args[USG_MUSICVOL] && ScummMidiVolume >= 0 && ScummMidiVolume <= 255)
	{
		sprintf(musicvol, "-m%ld", ScummMidiVolume);
		argv[argc++] = musicvol;
	}
	if (args[USG_SFXVOL] && ScummSfxVolume >= 0 && ScummSfxVolume <= 255)
	{
		sprintf(sfxvol, "-s%ld", ScummSfxVolume);
		argv[argc++] = sfxvol;
	}
	if (args[USG_TEMPO] && ScummMidiTempo > 0)
	{
		sprintf(tempo, "-t%lx", ScummMidiTempo);
		argv[argc++] = tempo;
	}
	if (args[USG_TALKSPEED] && ScummTalkSpeed >= 0 && ScummTalkSpeed <= 255)
	{
		sprintf(talkspeed, "-y%ld", ScummTalkSpeed);
		argv[argc++] = talkspeed;
	}
	if (ScummLang) 				argv[argc++] = ScummLang;
	if (args[USG_DUMPSCRIPTS]) argv[argc++] = "-u";
	if (args[USG_DEBUGLEVEL])
	{
		sprintf(dbglvl, "-d%ld", ScummDebugLevel);
		argv[argc++] = dbglvl;
	}
	if (ScummStory)
		argv[argc++] = ScummStory;

	return morphos_main(argc, argv);
}

