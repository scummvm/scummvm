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
#include <exec/memory.h>
#include <exec/libraries.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/cdda.h>
#include <proto/cybergraphics.h>
#include <proto/icon.h>

#include "stdafx.h"
#include "scumm.h"
#include "morphos.h"

extern "C" struct WBStartup *_WBenchMsg;

// For command line parsing
static STRPTR usageTemplate = "STORY/A,DATAPATH/K,WBWINDOW/S,SCALER/K,AMIGA/S,MIDIUNIT/K/N,MUSIC/K,VOLUME/K/N,TEMPO/K/N,ROLANDEMU/S,NOSUBTITLES=NST/S";
typedef enum 					{ USG_STORY = 0,	USG_DATAPATH, 	USG_WBWINDOW,	USG_SCALER, 	USG_AMIGA,	USG_MIDIUNIT,	USG_MUSIC,	 USG_VOLUME,	 USG_TEMPO,	  USG_ROLANDEMU, USG_NOSUBTITLES } usageFields;
static LONG	  args[ 11 ] =  { (ULONG)NULL, 	 (ULONG)NULL,	 FALSE, 			 (ULONG)NULL,	false,      (ULONG)NULL,   (ULONG)NULL, (ULONG)NULL,	 (ULONG)NULL, false, false };
static struct RDArgs *ScummArgs = NULL;

static char*ScummStory = NULL;
static char*ScummPath = NULL;
static STRPTR ScummMusicDriver = NULL;
static LONG ScummMidiUnit = 0;
static LONG ScummMidiVolume = 0;
static LONG ScummMidiTempo = 0;
static OSystem_MorphOS::SCALERTYPE ScummGfxScaler = OSystem_MorphOS::ST_INVALID;

static BPTR OrigDirLock = 0;

struct Library *CDDABase = NULL;
struct Library *CyberGfxBase = NULL;

OSystem_MorphOS *TheSystem = NULL;

OSystem *OSystem_MorphOS_create( int game_id, int gfx_mode, bool full_screen)
{
	if( TheSystem )
		delete TheSystem;

	OSystem_MorphOS::SCALERTYPE gfx_scaler = OSystem_MorphOS::ST_NONE;
	switch( gfx_mode )
	{
		case GFX_DOUBLESIZE:
			gfx_scaler = OSystem_MorphOS::ST_POINT;
			break;

		case GFX_SUPEREAGLE:
			gfx_scaler = OSystem_MorphOS::ST_SUPEREAGLE;
			break;

		case GFX_SUPER2XSAI:
			gfx_scaler = OSystem_MorphOS::ST_SUPER2XSAI;
			break;
	}

	TheSystem = OSystem_MorphOS::create( game_id, gfx_scaler, full_screen );
	return TheSystem;
}

void close_resources()
{
	if( TheSystem )
		delete TheSystem;

	if( ScummPath )
		FreeVec( ScummPath );

	if( ScummStory )
		FreeVec( ScummStory );

	if( ScummArgs )
		FreeArgs( ScummArgs );

	if( OrigDirLock )
		CurrentDir( OrigDirLock );

	if( CDDABase )
		CloseLibrary( CDDABase );

	if( CyberGfxBase )
		CloseLibrary( CyberGfxBase );
}

static STRPTR FindMusicDriver( STRPTR argval )
{
	if( !stricmp( argval, "off" ) )   return "-enull";
	if( !stricmp( argval, "midi" ) )	 return "-eamidi";
	if( !stricmp( argval, "adlib" ) ) return "-eadlib";

	error( "No such music driver supported. Possible values are off, Midi and Adlib." );
	return NULL;
}

static void ReadToolTypes( struct WBArg *OfFile )
{
	struct DiskObject *dobj;
	char 	*ToolValue;
	char IconPath[ 256 ];

	NameFromLock( OfFile->wa_Lock, IconPath, 256 );
	AddPart( IconPath, OfFile->wa_Name, 256 );

	dobj = GetDiskObject( IconPath );
	if( dobj == NULL )
		return;

	if( ToolValue = (char *)FindToolType( dobj->do_ToolTypes, "STORY" ) )
	{
		if( ScummStory )
			FreeVec( ScummStory );
		ScummStory = (char *)AllocVec( strlen( ToolValue )+1, MEMF_PUBLIC );
		strcpy( ScummStory, ToolValue );
	}

	if( ToolValue = (char *)FindToolType( dobj->do_ToolTypes, "DATAPATH" ) )
	{
		if( ScummPath )
			FreeVec( ScummPath );
		ScummPath = (char *)AllocVec( strlen( ToolValue )+4, MEMF_PUBLIC );
		strcpy( ScummPath, "-p" );
		strcat( ScummPath, ToolValue );
	}

	if( ToolValue = (char *)FindToolType( dobj->do_ToolTypes, "WBWINDOW" ) )
	{
		if( MatchToolValue( ToolValue, "YES" ) )
			args[ USG_WBWINDOW ] = TRUE;
		else if( MatchToolValue( ToolValue, "NO" ) )
			args[ USG_WBWINDOW ] = FALSE;
	}

	if( ToolValue = (char *)FindToolType( dobj->do_ToolTypes, "SCALER" ) )
	{
		if( (ScummGfxScaler = OSystem_MorphOS::FindScaler( ToolValue )) == OSystem_MorphOS::ST_INVALID )
		{
			FreeDiskObject( dobj );
			exit( 1 );
		}
	}

	if( ToolValue = (char *)FindToolType( dobj->do_ToolTypes, "MUSIC" ) )
	{
		if( !(ScummMusicDriver = FindMusicDriver( ToolValue )) )
		{
			FreeDiskObject( dobj );
			exit( 1 );
		}
		args[ USG_MUSIC ] = (ULONG)&ScummMusicDriver;
	}

	if( ToolValue = (char *)FindToolType( dobj->do_ToolTypes, "MIDIUNIT" ) )
		ScummMidiUnit = atoi( ToolValue );

	if( ToolValue = (char *)FindToolType( dobj->do_ToolTypes, "VOLUME" ) )
	{
		int vol = atoi( ToolValue );
		if( vol >= 0 && vol <= 100 )
		{
			ScummMidiVolume = vol;
			args[ USG_VOLUME ] = (ULONG)&ScummMidiVolume;
		}
	}

	if( ToolValue = (char *)FindToolType( dobj->do_ToolTypes, "TEMPO" ) )
	{
		ScummMidiTempo = atoi( ToolValue );
		args[ USG_TEMPO ] = (ULONG)&ScummMidiTempo;
	}

	if( ToolValue = (char *)FindToolType( dobj->do_ToolTypes, "ROLANDEMU" ) )
	{
		if( MatchToolValue( ToolValue, "YES" ) )
			args[ USG_ROLANDEMU ] = FALSE;
		else if( MatchToolValue( ToolValue, "NO" ) )
			args[ USG_ROLANDEMU ] = TRUE;
	}

	if( ToolValue = (char *)FindToolType( dobj->do_ToolTypes, "SUBTITLES" ) )
	{
		if( MatchToolValue( ToolValue, "YES" ) )
			args[ USG_NOSUBTITLES ] = FALSE;
		else if( MatchToolValue( ToolValue, "NO" ) )
			args[ USG_NOSUBTITLES ] = TRUE;
	}

	if( ToolValue = (char *)FindToolType( dobj->do_ToolTypes, "AMIGA" ) )
	{
		if( MatchToolValue( ToolValue, "YES" ) )
			args[ USG_AMIGA ] = FALSE;
		else if( MatchToolValue( ToolValue, "NO" ) )
			args[ USG_AMIGA ] = TRUE;
	}

	FreeDiskObject( dobj );
}

#undef main

int main()
{
	int delta;
	int last_time, new_time;
	char *argv[ 15 ];
	char volume[ 6 ], tempo[ 12 ], scaler[ 14 ];
	char *SVMScalers[] = { "", "normal", "2x", "supereagle", "super2xsai" };
	int argc = 0;

	InitSemaphore( &ScummSoundThreadRunning );
	InitSemaphore( &ScummMusicThreadRunning );

	CyberGfxBase = OpenLibrary( "cybergraphics.library", 41 );
	if( CyberGfxBase == NULL )
	{
		puts( "Failed to open cybergraphics.library" );
		exit( 1 );
	}

	atexit( &close_resources );

	if( _WBenchMsg == NULL )
	{
		/* Parse the command line here */
		ScummArgs = ReadArgs( usageTemplate, args, NULL );
		if( ScummArgs == NULL )
		{
			puts( "Error in command line - type \"ScummVM ?\" for usage.\n" );
			exit( 1 );
		}

		if( args[ USG_STORY ] )
		{
			ScummStory = (char *)AllocVec( strlen( (char *)args[ USG_STORY ] )+1, MEMF_PUBLIC );
			strcpy( ScummStory, (char *)args[ USG_STORY ] );
		}

		if( args[ USG_DATAPATH ] )
		{
			ScummPath = (char *)AllocVec( strlen( (char *)args[ USG_DATAPATH ] )+4, MEMF_PUBLIC );
			strcpy( ScummPath, "-p" );
			strcat( ScummPath, (char *)args[ USG_DATAPATH ] );
		}

		if( args[ USG_SCALER ] )
		{
			if( (ScummGfxScaler = OSystem_MorphOS::FindScaler( (char *)args[ USG_SCALER ] )) == OSystem_MorphOS::ST_INVALID )
				exit( 1 );
		}

		if( args[ USG_MUSIC ] )
		{
			if( !(ScummMusicDriver = FindMusicDriver( (char *)args[ USG_MUSIC ] )) )
				exit( 1 );
		}

		if( args[ USG_MIDIUNIT ] )
			ScummMidiUnit = *((LONG *)args[ USG_MIDIUNIT ]);

		if( args[ USG_TEMPO ] )
			ScummMidiTempo = *((LONG *)args[ USG_TEMPO ]);

		if( args[ USG_VOLUME ] )
			ScummMidiVolume = *((LONG *)args[ USG_VOLUME ]);
	}
	else
	{
		/* We've been started from Workbench */
		ReadToolTypes( &_WBenchMsg->sm_ArgList[ 0 ] );
		if( _WBenchMsg->sm_NumArgs > 1 )
		{
			ReadToolTypes( &_WBenchMsg->sm_ArgList[ 1 ] );
			OrigDirLock = CurrentDir( _WBenchMsg->sm_ArgList[ 1 ].wa_Lock );
		}
	}

	if( ScummPath )
	{
		char c = ScummPath[ strlen( ScummPath )-1 ];
		if( c != '/' && c != ':' )
			strcat( ScummPath, "/" );
	}

	argv[ argc++ ] = "ScummVM";
	argv[ argc++ ] = ScummStory;
	if( ScummPath ) 					argv[ argc++ ] = ScummPath;
	if( !args[ USG_WBWINDOW ]   ) argv[ argc++ ] = "-f";
	if( args[ USG_NOSUBTITLES ] ) argv[ argc++ ] = "-n";
	if( args[ USG_ROLANDEMU ] 	 ) argv[ argc++ ] = "-r";
	if( args[ USG_AMIGA ]		 ) argv[ argc++ ] = "-a";
	if( args[ USG_MUSIC ]	    ) argv[ argc++ ] = ScummMusicDriver;
	if( ScummGfxScaler != OSystem_MorphOS::ST_INVALID )
	{
		sprintf( scaler, "-g%s", SVMScalers[ (int)ScummGfxScaler ] );
		argv[ argc++ ] = scaler;
	}
	else
		argv[ argc++ ] = "-gsuper2xsai";
	if( args[ USG_VOLUME ] && ScummMidiVolume >= 0 && ScummMidiVolume <= 100 )
	{
		sprintf( volume, "-m%d", ScummMidiVolume );
		argv[ argc++ ] = volume;
	}
	if( args[ USG_TEMPO ] && ScummMidiTempo > 0 )
	{
		sprintf( tempo, "-t%lx", ScummMidiTempo );
		argv[ argc++ ] = tempo;
	}

	return morphos_main( argc, argv );
}

