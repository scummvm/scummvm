/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 Rüdiger Hanke
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
 * MorphOS interface
 *
 * $Header$
 *
 */

#include <devices/timer.h>

#include "stdafx.h"
#include "scumm.h"
#include "gui.h"
#include "sound.h"

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <dos/dostags.h>
#include <intuition/screens.h>
#include <cybergraphics/cybergraphics.h>
#include <devices/inputevent.h>
#include <intuition/intuition.h>
#include <workbench/startup.h>

#define NO_PPCINLINE_STDARG
#define NO_PPCINLINE_VARARGS
#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/intuition.h>
#include <proto/keymap.h>
#include <proto/timer.h>
#include <proto/cdda.h>
#include <proto/cybergraphics.h>

#include <emul/emulinterface.h>

#include <time.h>

extern "C" struct WBStartup *_WBenchMsg;

Scumm scumm;
ScummDebugger debugger;
Gui gui;

SoundEngine sound;
SOUND_DRIVER_TYPE snd_driv;

typedef void (*ScalerFunc)( uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height );

void Super2xSaI( uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height );
void SuperEagle( uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height );
void PointScaler( uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height );

static struct Screen  *ScummScreen = NULL;
static struct Window  *ScummWindow = NULL;
static APTR            ScummBuffer = NULL;
static struct ScreenBuffer *ScummScreenBuffer[ 2 ] = { NULL, NULL };
static struct BitMap  *ScummRenderTo = NULL;
static ULONG			  ScummPaintBuffer;
static UWORD 			 *ScummNoCursor = NULL;
static struct Process *ScummMusicThread = NULL;
static ScalerFunc      ScummScaler = &Super2xSaI;
static ULONG 			  ScummColors[256];
static USHORT 			  ScummColors16[256];
static WORD				  ScummWinX = -1;
static WORD				  ScummWinY = -1;
static bool				  ScummOrigMouse = false;
static int 				  ScummShakePos = 0;

static struct MsgPort     *TimerMsgPort = NULL;
static struct timerequest *TimerIORequest = NULL;
static bool					 	TimerStarted = false;

static char*ScummStory = NULL;
static char*ScummPath = NULL;
static char ScummWndTitle[ 125 ];
static int  ScummDepth = 0;
static int  ScummScale = 1;
static bool Scumm16ColFmt16 = false;
static int  ScummScrWidth = 0;
static int  ScummScrHeight = 0;
static LONG ScummMidiUnit = 0;
static LONG ScummMidiVolume = 0;
static LONG ScummMidiTempo = 0;

struct Library *CDDABase = NULL;
static CDRIVEPTR CDrive = NULL;
static struct TagItem FindCDTags[] =   {  {  CDFA_VolumeName, (ULONG)"LoomCD"  },
														{  TAG_DONE, 0 } };

static struct TagItem PlayTags[] =   { {  CDPA_StartTrack,  1     },
													{  CDPA_StartFrame,	0     },
													{  CDPA_EndTrack, 	1     },
													{  CDPA_EndFrame, 	0     },
													{	TAG_DONE,			0		}
												 };
static ULONG CDDATrackOffset = 0;


static struct RDArgs *ScummArgs = NULL;
static BPTR OrigDirLock = 0;

struct GfxScaler
{
	STRPTR 		gs_Name;
	ScalerFunc  gs_Function;
};

static struct GfxScaler ScummScalers[] = {	{ "none", NULL	},
															{ "Point", PointScaler },
															{ "SuperEagle", SuperEagle },
															{ "Super2xSaI", Super2xSaI },
															{ NULL, NULL },
															{ NULL, NULL },
															{ NULL, NULL },
															{ NULL, NULL },
															{ NULL, NULL },
															{ NULL, NULL }
													  };

// For command line parsing
static STRPTR usageTemplate = "STORY/A,DATAPATH/K,WBWINDOW/S,SCALER/K,MIDIUNIT/K/N,NOMUSIC/S,VOLUME/K/N,TEMPO/K,NOSUBTITLES=NST/S";
typedef enum 					{ USG_STORY = 0,	USG_DATAPATH, 	USG_WBWINDOW,	USG_SCALER, 	USG_MIDIUNIT,	USG_NOMUSIC,	USG_VOLUME,		USG_TEMPO,	 USG_NOSUBTITLES } usageFields;
static LONG	  args[ 9 ] = 	{ (ULONG)NULL, 	(ULONG)NULL,	FALSE, 			(ULONG)NULL,	(ULONG)NULL, 	false,			(ULONG)NULL,	(ULONG)NULL, false };

// These are for the scaling engine
static uint32 colorMask = 0xF7DEF7DE;
static uint32 lowPixelMask = 0x08210821;
static uint32 qcolorMask = 0xE79CE79C;
static uint32 qlowpixelMask = 0x18631863;
static uint32 redblueMask = 0xF81F;
static uint32 greenMask = 0x7E0;
static int PixelsPerMask = 2;
static byte *src_line[4];
static byte *dst_line[2];

struct EmulFunc MyEmulFunc;
struct TagItem musicProcTags[] = { { NP_Entry, 	     (ULONG)&MyEmulFunc.Trap    		},
											  { NP_Name, 	     (ULONG)"ScummVM Music Thread" 	},
											  { NP_StackSize,	  16000									},
											  { NP_Priority,    30 								      },
											  { TAG_DONE,       0 						     			}
											};

extern int morphos_music_thread( Scumm *s, ULONG MidiUnit, bool NoMusic );

extern struct SignalSemaphore ScummMusicThreadRunning;

struct Library *CyberGfxBase = NULL;
struct Device  *TimerBase = NULL;

void updateScreen(Scumm *s);

void BoxTest(int num)
{
	/* Debug only, remove */
}

int GetTicks()
{
/*	  unsigned long long time64;
	ULONG ClockTicks;
	struct EClockVal ClockVal;

	ClockTicks = ReadEClock( &ClockVal );
	time64 = ClockVal.ev_hi;

	time64 = (time64 << 32) | ClockVal.ev_lo;
	return (time64/ClockTicks)*1000;*/

	int ticks = clock();
	ticks *= (1000/CLOCKS_PER_SEC);
	return ticks;
}

static int   cd_track = 0, cd_num_loops = 0, cd_start_frame = 0;
static ULONG cd_end_time = 0;
static ULONG cd_stop_time = 0;

void cd_play( int track, int num_loops, int start_frame, int length )
{
	if( CDrive && start_frame >= 0 )
	{
		struct CDS_TrackInfo ti;

		if( start_frame > 0 )
			start_frame -= CDDATrackOffset;

		cd_track = track;
		cd_num_loops = num_loops;
		cd_start_frame = start_frame;

		PlayTags[ 0 ].ti_Data = track;
		PlayTags[ 1 ].ti_Data = start_frame;
		PlayTags[ 2 ].ti_Data = (length == 0) ? track+1 : track;
		PlayTags[ 3 ].ti_Data = length ? start_frame+length : 0;
		CDDA_Play( CDrive, PlayTags );
		cd_stop_time = 0;
		
		CDDA_GetTrackInfo( CDrive, track, 0, &ti );
		cd_end_time = GetTicks() + ti.ti_TrackLength.tm_Format.tm_Frame * 1000 / 75;
	}
}

// Schedule the music to be stopped after 1/10 sec, unless another
// track is started in the meantime.
void cd_stop()
{
	cd_stop_time = GetTicks() + 100;
	cd_num_loops = 0;
}

int cd_is_running()
{
	ULONG status;

	if( CDrive == NULL )
		return 0;

	CDDA_GetAttr( CDDA_Status, CDrive, &status );
	return (cd_num_loops != 0 && (GetTicks() < cd_end_time || status != CDDA_Status_Ready));
}

void cd_music_loop()
{
	if( CDrive )
	{
		if( cd_stop_time != 0 && GetTicks() >= cd_stop_time )
		{
			CDDA_Stop( CDrive );
			cd_num_loops = 0;
			cd_stop_time = 0;
			return;
		}
		if( cd_num_loops == 0 || GetTicks() < cd_end_time )
			return;

		ULONG status;
		CDDA_GetAttr( CDDA_Status, CDrive, &status );
		if( cd_num_loops != 1 && status != CDDA_Status_Ready )
		{
			// Wait another second for it to be done
			cd_end_time += 1000;
			return;
		}

		if( cd_num_loops > 0 )
			cd_num_loops--;

		if( cd_num_loops != 0 )
		{
			struct CDS_TrackInfo ti;

			PlayTags[ 0 ].ti_Data = cd_track;
			PlayTags[ 1 ].ti_Data = cd_start_frame;
			CDDA_Play( CDrive, PlayTags );
		
			CDDA_GetTrackInfo( CDrive, cd_track, 0, &ti );
			cd_end_time = GetTicks() + ti.ti_TrackLength.tm_Format.tm_Frame * 1000 / 75;
		}
	}
}

void closeResources()
{
	if( ScummMusicThread )
	{
		Signal( (struct Task *)ScummMusicThread, SIGBREAKF_CTRL_F );
		ObtainSemaphore( &ScummMusicThreadRunning );		/* Wait for thread to finish */
		ReleaseSemaphore( &ScummMusicThreadRunning );
	}

	if( TimerStarted )
	{
		AbortIO( (struct IORequest *)TimerIORequest );
		WaitIO( (struct IORequest *)TimerIORequest );
	}

	if( TimerIORequest )
	{
		CloseDevice( (struct IORequest *)TimerIORequest );
		DeleteIORequest( (struct IORequest *)TimerIORequest );
	}

	if( TimerMsgPort )
		DeleteMsgPort( TimerMsgPort );

	if( OrigDirLock )
		CurrentDir( OrigDirLock );

	if( ScummPath )
		FreeVec( ScummPath );

	if( ScummStory )
		FreeVec( ScummStory );

	if( ScummArgs )
		FreeArgs( ScummArgs );

	if( ScummNoCursor )
		FreeVec( ScummNoCursor );

	if( ScummBuffer )
		FreeVec( ScummBuffer );

	if( ScummRenderTo && !ScummScreen )
		FreeBitMap( ScummRenderTo );
	
	if( ScummWindow )
		CloseWindow( ScummWindow );

	if( ScummScreen )
	{
		if( ScummScreenBuffer[ 0 ] )
			FreeScreenBuffer( ScummScreen, ScummScreenBuffer[ 0 ] );
		if( ScummScreenBuffer[ 1 ] )
			FreeScreenBuffer( ScummScreen, ScummScreenBuffer[ 1 ] );
		CloseScreen( ScummScreen );
	}

	if( CDDABase )
	{
		if( CDrive && CDDABase )
		{
			CDDA_Stop( CDrive );
			CDDA_ReleaseDrive( CDrive );
		}
		CloseLibrary( CDDABase );
	}

	if( CyberGfxBase )
		CloseLibrary( CyberGfxBase );
}

uint32 makeColForPixFmt( int pixfmt, int r, int g, int b )
{
	uint32 col = 0;

	switch( pixfmt )
	{
		case PIXFMT_RGB15:
		case PIXFMT_RGB15PC:
			col = (((r*31)/255) << 10) | (((g*31)/255) << 5) | ((b*31)/255);
			break;

		case PIXFMT_BGR15:
		case PIXFMT_BGR15PC:
			col = (((b*31)/255) << 10) | (((g*31)/255) << 5) | ((r*31)/255);
			break;

		case PIXFMT_RGB16:
		case PIXFMT_RGB16PC:
			col = (((r*31)/255) << 11) | (((g*63)/255) << 5) | ((b*31)/255);
			break;

		case PIXFMT_BGR16:
		case PIXFMT_BGR16PC:
			col = (((b*31)/255) << 11) | (((g*63)/255) << 5) | ((r*31)/255);
			break;

		case PIXFMT_RGB24:
		case PIXFMT_ARGB32:
			col = (r << 16) | (g << 8) | b;
			break;

		case PIXFMT_BGR24:
			col = (b << 16) | (g << 8) | r;
			break;

		case PIXFMT_BGRA32:
			col = (b << 24) | (g << 16) | (r << 8);
			break;

		case PIXFMT_RGBA32:
			col = (r << 24) | (g << 16) | (b << 8);
			break;

		default:
			error( "Unsupported pixel format: %d. Please contact author at tomjoad@muenster.de.", pixfmt );
			exit( 1 );
	}

	if( pixfmt == PIXFMT_RGB15PC || pixfmt == PIXFMT_BGR15PC ||
		 pixfmt == PIXFMT_RGB16PC || pixfmt == PIXFMT_BGR16PC
	  )
		col = ((col >> 8) & 0xff) | ((col << 8) & 0xff00);	/* Not really sure about this?!?! */


	return col;
}

#define CVT8TO32( byte )   ((byte<<24) | (byte<<16) | (byte<<8) | byte)

void updatePalette( Scumm *s )
{
	int first = s->_palDirtyMin;
	int num = s->_palDirtyMax - first + 1;
	int i;
	byte *data = s->_currentPalette;

	data += first*3;
	for( i = first; i < first+num; i++, data+=3 )
	{
		if( ScummDepth == 8 )
			SetRGB32( &ScummScreen->ViewPort, i, CVT8TO32( data[ 0 ] ), CVT8TO32( data[ 1 ] ), CVT8TO32( data[ 2 ] ) );
		ScummColors16[ i ] = Scumm16ColFmt16 ? (((data[ 0 ]*31)/255) << 11) | (((data[ 1 ]*63)/255) << 5) | ((data[ 2 ]*31)/255) : (((data[ 0 ]*31)/255) << 10) | (((data[ 1 ]*31)/255) << 5) | ((data[ 2 ]*31)/255);
		ScummColors[ i ] = (data[ 0 ] << 16) | (data[ 1 ] << 8) | data[ 2 ];
	}

	s->_palDirtyMax = -1;
	s->_palDirtyMin = 0x3E8;
}

typedef enum { CSDSPTYPE_WINDOWED, CSDSPTYPE_FULLSCREEN, CSDSPTYPE_TOGGLE } CS_DSPTYPE;

void createScreen( CS_DSPTYPE dspType )
{
	ULONG mode = INVALID_ID;
	int   depths[] = { 8, 15, 16, 24, 32, 0 };
	int   i;
	struct Screen *wb = NULL;
	bool  fullScreen;

	fullScreen = (dspType == CSDSPTYPE_FULLSCREEN) || (dspType == CSDSPTYPE_TOGGLE && ScummScreen == NULL);

	if( ScummRenderTo && !ScummScreen )
		FreeBitMap( ScummRenderTo );
	ScummRenderTo = NULL;

	if( ScummWindow )
	{
		if( ScummScreen == NULL )
		{
			ScummWinX = ScummWindow->LeftEdge;
			ScummWinY = ScummWindow->TopEdge;
		}
		CloseWindow( ScummWindow );
		ScummWindow = NULL;
	}

	if( ScummScreen )
	{
		if( ScummScreenBuffer[ 0 ] )
			FreeScreenBuffer( ScummScreen, ScummScreenBuffer[ 0 ] );
		if( ScummScreenBuffer[ 1 ] )
			FreeScreenBuffer( ScummScreen, ScummScreenBuffer[ 1 ] );
		CloseScreen( ScummScreen );
		ScummScreen = NULL;
	}
	
	ScummScrWidth  = 320 << ScummScale;
	ScummScrHeight = 200 << ScummScale;

	if( fullScreen )
	{
		for( i = ScummScale; mode == INVALID_ID && depths[ i ]; i++ )
			mode = BestCModeIDTags( CYBRBIDTG_NominalWidth, 	ScummScrWidth,
											CYBRBIDTG_NominalHeight,   ScummScrHeight,
											CYBRBIDTG_Depth,   			depths[ i ],
											TAG_DONE
										 );
		ScummDepth = depths[ i-1 ];

		if( mode == INVALID_ID )
		{
			error( "Could not find suitable screenmode" );
			exit(1);
		}

		ScummScreen = OpenScreenTags( NULL, SA_AutoScroll, TRUE,
														SA_Depth,		ScummDepth,
														SA_Width,		ScummScrWidth,
														SA_Height,		ScummScrHeight,
														SA_DisplayID,	mode,
														SA_ShowTitle,	FALSE,
														SA_Type,			CUSTOMSCREEN,
														SA_Title,		"ScummVM MorphOS",
														TAG_DONE
											 );

		if( ScummScreen == NULL )
		{
			error( "Failed to open screen" );
			exit( 1 );
		}

		ScummScreenBuffer[ 0 ] = AllocScreenBuffer( ScummScreen, NULL, SB_SCREEN_BITMAP );
		ScummScreenBuffer[ 1 ] = AllocScreenBuffer( ScummScreen, NULL, 0 );
		ScummRenderTo = ScummScreenBuffer[ 1 ]->sb_BitMap;
		ScummPaintBuffer = 1;

		if( ScummScreenBuffer[ 0 ] == NULL || ScummScreenBuffer[ 1 ] == NULL )
		{
			error( "Failed to allocate screen buffer" );
			exit( 1 );
		}

		if( ScummDepth == 8 )
		{
			for( int color = 0; color < 256; color++ )
			{
				ULONG r, g, b;

				r = (ScummColors[ color ] >> 16) & 0xff;
				g = (ScummColors[ color ] >> 8) & 0xff;
				b = (ScummColors[ color ] >> 0) & 0xff;
				SetRGB32( &ScummScreen->ViewPort, color, CVT8TO32( r ), CVT8TO32( g ), CVT8TO32( b ) );
			}
		}
	}
	else
	{
		wb = LockPubScreen( NULL );
		if( wb == NULL )
		{
			error( "Could not lock default public screen" );
			exit( 1 );
		}

		ScreenToFront( wb );
	}

	ScummWindow = OpenWindowTags( NULL,	WA_Left,				(wb && ScummWinX >= 0) ? ScummWinX : 0,
													WA_Top,				wb ? ((ScummWinY >= 0) ? ScummWinY : wb->BarHeight+1) : 0,
													WA_InnerWidth,	   ScummScrWidth,
													WA_InnerHeight,   ScummScrHeight,
													WA_Activate,		TRUE,
													WA_Title,		   wb ? ScummWndTitle : NULL,
													WA_ScreenTitle,	wb ? ScummWndTitle : NULL,
													WA_Borderless,		fullScreen,
													WA_CloseGadget,	!fullScreen,
													WA_DepthGadget,	!fullScreen,
													WA_DragBar,			!fullScreen,
													WA_ReportMouse,	TRUE,
													WA_RMBTrap,			TRUE,
													WA_IDCMP,			IDCMP_RAWKEY 		|
																			IDCMP_MOUSEMOVE 	|
																			IDCMP_CLOSEWINDOW |
																			IDCMP_MOUSEBUTTONS,
													WA_CustomScreen,  fullScreen ? (ULONG)ScummScreen : (ULONG)wb,
													TAG_DONE
										 );

	if( wb )
		UnlockPubScreen( NULL, wb );

	if( ScummWindow == NULL )
	{
		error( "Failed to open window" );
		exit(1);
	}

	SetPointer( ScummWindow, ScummNoCursor, 1, 1, 0, 0 );
	ScummOrigMouse = false;

	if( ScummScreen == NULL )
	{
		ScummDepth = GetCyberMapAttr( ScummWindow->RPort->BitMap, CYBRMATTR_DEPTH );
		if( ScummDepth == 8 )
		{
			error( "Workbench screen must be running in 15 bit or higher" );
			exit( 1 );
		}

		ScummRenderTo = AllocBitMap( ScummScrWidth, ScummScrHeight, ScummDepth, BMF_MINPLANES, ScummWindow->RPort->BitMap );
		if( ScummRenderTo == NULL )
		{
			error( "Failed to allocate bitmap" );
			exit( 1 );
		}
	}

	if( ScummScale )
	{
		/* Initialize scaling stuff */
		int minr, ming, minb;

		if( ScummDepth > 16 )
		{
			minr = 1 << 16;
			ming = 1 << 8;
			minb = 1;
		}
		else
		{
			minr = 1 << ((ScummDepth == 15) ? 10 : 11);
			ming = 1 << 5;
			minb = 1;

			if( (ScummDepth == 15 && Scumm16ColFmt16) || (ScummDepth == 16 && !Scumm16ColFmt16) )
			{
				for( int col = 0; col < 256; col++ )
				{
					int r = (ScummColors[ col ] >> 16) & 0xff;
					int g = (ScummColors[ col ] >> 8) & 0xff;
					int b = ScummColors[ col ] & 0xff;
					ScummColors16[ col ] = (Scumm16ColFmt16 == false) ? (((r*31)/255) << 11) | (((g*63)/255) << 5) | ((b*31)/255) : (((r*31)/255) << 10) | (((g*31)/255) << 5) | ((b*31)/255);
				}
			}

			Scumm16ColFmt16 = (ScummDepth == 16);
		}

		int pixfmt = GetCyberMapAttr( ScummRenderTo, CYBRMATTR_PIXFMT );

		debug( 1, "Pixelformat = %d", pixfmt );

		colorMask = (makeColForPixFmt( pixfmt, 255, 0, 0 ) - minr) | (makeColForPixFmt( pixfmt, 0, 255, 0 ) - ming) | (makeColForPixFmt( pixfmt, 0, 0, 255 ) - minb);
		lowPixelMask = minr | ming | minb;
		qcolorMask = (makeColForPixFmt( pixfmt, 255, 0, 0 ) - 3*minr) | (makeColForPixFmt( pixfmt, 0, 255, 0) - 3*ming) | (makeColForPixFmt( pixfmt, 0, 0, 255 ) - 3*minb);
		qlowpixelMask = (minr * 3) | (ming * 3) | (minb * 3);
		redblueMask = makeColForPixFmt( pixfmt, 255, 0, 255 );
		greenMask = makeColForPixFmt( pixfmt, 0, 255, 0 );

		PixelsPerMask = (ScummDepth <= 16) ? 2 : 1;

		if( PixelsPerMask == 2 )
		{
			colorMask |= (colorMask << 16);
			qcolorMask |= (qcolorMask << 16);
			lowPixelMask |= (lowPixelMask << 16);
			qlowpixelMask |= (qlowpixelMask << 16);
		}
	}
}

int old_mouse_x, old_mouse_y;
int old_mouse_h, old_mouse_w;
bool has_mouse,hide_mouse;

#define BAK_WIDTH 40
#define BAK_HEIGHT 40
byte old_backup[ BAK_WIDTH*BAK_HEIGHT*2 ];

void SwitchScalerTo( ScalerFunc newScaler )
{
	if( newScaler == NULL && ScummScale )
	{
		ScummScale = false;
		ScummScaler = NULL;
		createScreen( ScummScreen ? CSDSPTYPE_FULLSCREEN : CSDSPTYPE_WINDOWED );
	}
	else
	{
		if( !ScummScale )
		{
			ScummScale = true;
			createScreen( ScummScreen ? CSDSPTYPE_FULLSCREEN : CSDSPTYPE_WINDOWED );
		}

		if( ScummScaler != newScaler )
			ScummScaler = newScaler;
	}
}

void waitForTimer( Scumm *s, int msec_delay )
{
	struct IntuiMessage *ScummMsg;
	ULONG signals;
	bool  AllDone = false;

	if (s->_fastMode&2)
		msec_delay = 0;
	else if (s->_fastMode&1)
		msec_delay = 10;

	TimerIORequest->tr_node.io_Command  = TR_ADDREQUEST;
	TimerIORequest->tr_time.tv_secs  = 0;
	TimerIORequest->tr_time.tv_micro = (msec_delay > 0 ? msec_delay : 1)*1000;
	SendIO( (struct IORequest *)TimerIORequest );
	TimerStarted = true;

	do
	{
		while( ScummMsg = (struct IntuiMessage *)GetMsg( ScummWindow->UserPort ) )
		{
			switch( ScummMsg->Class )
			{
				case IDCMP_RAWKEY:
				{
					struct InputEvent FakedIEvent;
					char charbuf;

					memset( &FakedIEvent, 0, sizeof( struct InputEvent ) );
					FakedIEvent.ie_Class = IECLASS_RAWKEY;
					FakedIEvent.ie_Code = ScummMsg->Code;

					if( ScummMsg->Code >= 0x50 && ScummMsg->Code <= 0x59 )
					{
						// Function key
						s->_keyPressed = (ScummMsg->Code-0x50)+315;
					}
					else if( MapRawKey( &FakedIEvent, &charbuf, 1, NULL ) == 1 )
					{
						if( charbuf >= '0' && charbuf <= '9' && !(ScummMsg->Qualifier & (IEQUALIFIER_LALT | IEQUALIFIER_RALT)) )
						{
							s->_saveLoadSlot = charbuf - '0';
							if( ScummMsg->Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT) )
							{
								sprintf(s->_saveLoadName, "Quicksave %d", s->_saveLoadSlot);
								s->_saveLoadFlag = 1;
							}
							else if( ScummMsg->Qualifier & IEQUALIFIER_CONTROL )
								s->_saveLoadFlag = 2;
							s->_keyPressed = charbuf;
							s->_saveLoadCompatible = false;
						}
						else if( ScummMsg->Qualifier & IEQUALIFIER_CONTROL )
						{
							switch( charbuf )
							{
								case 'z':
									ReplyMsg( (struct Message *)ScummMsg );
									exit(1);

								case 'd':
									debugger.attach(s);
									break;

								case 's':
									s->resourceStats();
									break;
							}
						}
						else if( ScummMsg->Qualifier & (IEQUALIFIER_RALT | IEQUALIFIER_LALT) )
						{
							if( charbuf >= '0' && charbuf <= '9' && ScummScalers[ charbuf-'0' ].gs_Name )
							{
								ReplyMsg( (struct Message *)ScummMsg );
								ScummMsg = NULL;
								SwitchScalerTo( ScummScalers[ charbuf-'0' ].gs_Function );
							}
							else if( charbuf == 0x0d )
							{
								ReplyMsg( (struct Message *)ScummMsg );
								ScummMsg = NULL;
								createScreen( CSDSPTYPE_TOGGLE );
							}
						}
						else
							s->_keyPressed = charbuf;
					}
					break;
				}

				case IDCMP_MOUSEMOVE:
				{
					int newx,newy;

					newx = (ScummMsg->MouseX-ScummWindow->BorderLeft) >> ScummScale;
					newy = (ScummMsg->MouseY-ScummWindow->BorderTop) >> ScummScale;

					if (newx != s->mouse.x || newy != s->mouse.y)
					{
						if( newx < 0 || newx > 320 ||
							 newy < 0 || newy > 200
						  )
						{
							if( !ScummOrigMouse )
							{
								ScummOrigMouse = true;
								ClearPointer( ScummWindow );
							}
						}
						else if( ScummOrigMouse )
						{
							ScummOrigMouse = false;
							SetPointer( ScummWindow, ScummNoCursor, 1, 1, 0, 0 );
						}
						s->mouse.x = newx;
						s->mouse.y = newy;
						s->drawMouse();
					}
					break;
				}

				case IDCMP_MOUSEBUTTONS:
					switch( ScummMsg->Code )
					{
						case SELECTDOWN:
							s->_leftBtnPressed |= msClicked|msDown;
							break;

						case SELECTUP:
							s->_leftBtnPressed &= ~msDown;
							break;

						case MENUDOWN:
							s->_rightBtnPressed |= msClicked|msDown;
							break;

						case MENUUP:
							s->_rightBtnPressed &= ~msDown;
							break;
					}
					break;

				case IDCMP_CLOSEWINDOW:
					ReplyMsg( (struct Message *)ScummMsg );
					exit(1);
					break;
			}

			if( ScummMsg )
				ReplyMsg( (struct Message *)ScummMsg );

			cd_music_loop();

			if( GetMsg( TimerMsgPort ) )
			{
				TimerStarted = false;
				AllDone = true;
				break;
			}
		}

		if( !AllDone )
		{
			signals = Wait( (1 << (ScummWindow->UserPort->mp_SigBit)) | (1 << (TimerMsgPort->mp_SigBit) ) );

			if( signals & (1 << (TimerMsgPort->mp_SigBit)) )
			{
				AllDone = true;
				TimerStarted = false;
				GetMsg( TimerMsgPort );
			}
		}
	} while ( !AllDone );
}

void setShakePos( Scumm *s, int shake_pos )
{
	ScummShakePos = shake_pos;
}

#define GET_RESULT(A, B, C, D) ((A != C || A != D) - (B != C || B != D))

#define INTERPOLATE(A, B) (((A & colorMask) >> 1) + ((B & colorMask) >> 1) + (A & B & lowPixelMask))

#define Q_INTERPOLATE(A, B, C, D) ((A & qcolorMask) >> 2) + ((B & qcolorMask) >> 2) + ((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2) + ((((A & qlowpixelMask) + (B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask)) >> 2) & qlowpixelMask)

void Super2xSaI( uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height )
{
	unsigned int x, y;
	unsigned long color[16];
	byte *src;
	byte *dest;
	uint32 dest_bpp;
	uint32 dest_pitch;
	APTR handle;

	if( (handle = LockBitMapTags( ScummRenderTo, LBMI_BYTESPERPIX, &dest_bpp, LBMI_BYTESPERROW, &dest_pitch, LBMI_BASEADDRESS, &dest, TAG_DONE )) == 0 )
		return;

	src = ((byte *)ScummBuffer)+src_y*320+src_x;

	/* Point to the first 3 lines. */
	src_line[0] = src;
	src_line[1] = src;
	src_line[2] = src + 320;
	src_line[3] = src + 320 * 2;

	dst_line[0] = dest+dest_y*2*dest_pitch+dest_x*2*dest_bpp;
	dst_line[1] = dst_line[0]+dest_pitch;

	x = 0, y = 0;

	if( PixelsPerMask == 2 )
	{
		byte *sbp;
		sbp = src_line[0];
		color[0] = ScummColors16[ *sbp ];       color[1] = color[0];   color[2] = color[0];    color[3] = color[0];
		color[4] = color[0];   color[5] = color[0];   color[6] = ScummColors16[ *(sbp + 1) ];  color[7] = ScummColors16[ *(sbp + 2) ];
		sbp = src_line[2];
		color[8] = ScummColors16[ *sbp ];     color[9] = color[8];     color[10] = ScummColors16[ *(sbp + 1) ]; color[11] = ScummColors16[ *(sbp + 2) ];
		sbp = src_line[3];
		color[12] = ScummColors16[ *sbp ];    color[13] = color[12];   color[14] = ScummColors16[ *(sbp + 1) ]; color[15] = ScummColors16[ *(sbp + 2) ];
	}
	else
	{
		byte *lbp;
		lbp = src_line[0];
		color[0] = ScummColors[ *lbp ];       color[1] = color[0];   color[2] = color[0];    color[3] = color[0];
		color[4] = color[0];   color[5] = color[0];   color[6] = ScummColors[ *(lbp + 1) ];  color[7] = ScummColors[ *(lbp + 2) ];
		lbp = src_line[2];
		color[8] = ScummColors[ *lbp ];     color[9] = color[8];     color[10] = ScummColors[ *(lbp + 1) ]; color[11] = ScummColors[ *(lbp + 2) ];
		lbp = src_line[3];
		color[12] = ScummColors[ *lbp ];    color[13] = color[12];   color[14] = ScummColors[ *(lbp + 1) ]; color[15] = ScummColors[ *(lbp + 2) ];
	}

	for (y = 0; y < height; y++)
	{
		/* Todo: x = width - 2, x = width - 1 */
		for( x = 0; x < width; x++ )
		{
			unsigned long product1a, product1b, product2a, product2b;

//---------------------------------------  B0 B1 B2 B3    0  1  2  3
//                                         4  5* 6  S2 -> 4  5* 6  7
//                                         1  2  3  S1    8  9 10 11
//                                         A0 A1 A2 A3   12 13 14 15
//--------------------------------------
			if( color[9] == color[6] && color[5] != color[10] )
			{
				product2b = color[9];
				product1b = product2b;
			}
			else if( color[5] == color[10] && color[9] != color[6] )
			{
				product2b = color[5];
				product1b = product2b;
			}
			else if( color[5] == color[10] && color[9] == color[6] )
			{
				int r = 0;

				r += GET_RESULT(color[6], color[5], color[8], color[13]);
				r += GET_RESULT(color[6], color[5], color[4], color[1]);
				r += GET_RESULT(color[6], color[5], color[14], color[11]);
				r += GET_RESULT(color[6], color[5], color[2], color[7]);

				if (r > 0)
					product1b = color[6];
				else if (r < 0)
					product1b = color[5];
				else
					product1b = INTERPOLATE(color[5], color[6]);

				product2b = product1b;

			}
			else
			{
				if( color[6] == color[10] && color[10] == color[13] && color[9] != color[14] && color[10] != color[12] )
					product2b = Q_INTERPOLATE(color[10], color[10], color[10], color[9]);
				else if( color[5] == color[9] && color[9] == color[14] && color[13] != color[10] && color[9] != color[15] )
					product2b = Q_INTERPOLATE(color[9], color[9], color[9], color[10]);
				else
					product2b = INTERPOLATE(color[9], color[10]);

				if (color[6] == color[10] && color[6] == color[1] && color[5] != color[2] && color[6] != color[0])
					product1b = Q_INTERPOLATE(color[6], color[6], color[6], color[5]);
				else if (color[5] == color[9] && color[5] == color[2] && color[1] != color[6] && color[5] != color[3])
					product1b = Q_INTERPOLATE(color[6], color[5], color[5], color[5]);
				else
					product1b = INTERPOLATE(color[5], color[6]);
			}

			if (color[5] == color[10] && color[9] != color[6] && color[4] == color[5] && color[5] != color[14])
				product2a = INTERPOLATE(color[9], color[5]);
			else if (color[5] == color[8] && color[6] == color[5] && color[4] != color[9] && color[5] != color[12])
				product2a = INTERPOLATE(color[9], color[5]);
			else
				product2a = color[9];

			if (color[9] == color[6] && color[5] != color[10] && color[8] == color[9] && color[9] != color[2])
				product1a = INTERPOLATE(color[9], color[5]);
			else if (color[4] == color[9] && color[10] == color[9] && color[8] != color[5] && color[9] != color[0])
				product1a = INTERPOLATE(color[9], color[5]);
			else
				product1a = color[5];

			if (PixelsPerMask == 2)
			{
				*((unsigned long *) (&dst_line[0][x * 4])) = (product1a << 16) | product1b;
				*((unsigned long *) (&dst_line[1][x * 4])) = (product2a << 16) | product2b;
			}
			else
			{
				*((unsigned long *) (&dst_line[0][x * 8])) = product1a;
				*((unsigned long *) (&dst_line[0][x * 8 + 4])) = product1b;
				*((unsigned long *) (&dst_line[1][x * 8])) = product2a;
				*((unsigned long *) (&dst_line[1][x * 8 + 4])) = product2b;
			}

			/* Move color matrix forward */
			color[0] = color[1]; color[4] = color[5]; color[8] = color[9];   color[12] = color[13];
			color[1] = color[2]; color[5] = color[6]; color[9] = color[10];  color[13] = color[14];
			color[2] = color[3]; color[6] = color[7]; color[10] = color[11]; color[14] = color[15];

			if (x < width - 3)
			{
				x += 3;
				if (PixelsPerMask == 2)
				{
					color[3] = ScummColors16[ *(src_line[0] + x) ];
					color[7] = ScummColors16[ *(src_line[1] + x) ];
					color[11] = ScummColors16[ *(src_line[2] + x) ];
					color[15] = ScummColors16[ *(src_line[3] + x) ];
				}
				else
				{
					color[3] = ScummColors[ *(src_line[0] + x) ];
					color[7] = ScummColors[ *(src_line[1] + x) ];
					color[11] = ScummColors[ *(src_line[2] + x) ];
					color[15] = ScummColors[ *(src_line[3] + x) ];
				}
				x -= 3;
			}
		}

		/* We're done with one line, so we shift the source lines up */
		src_line[0] = src_line[1];
		src_line[1] = src_line[2];
		src_line[2] = src_line[3];

		/* Read next line */
		if (y + 3 >= height)
			src_line[3] = src_line[2];
		else
			src_line[3] = src_line[2] + 320;

		/* Then shift the color matrix up */
		if (PixelsPerMask == 2)
		{
			byte *sbp;
			sbp = src_line[0];
			color[0] = ScummColors16[ *sbp ];     color[1] = color[0];    color[2] = ScummColors16[ *(sbp + 1) ];  color[3] = ScummColors16[ *(sbp + 2) ];
			sbp = src_line[1];
			color[4] = ScummColors16[ *sbp ];     color[5] = color[4];    color[6] = ScummColors16[ *(sbp + 1) ];  color[7] = ScummColors16[ *(sbp + 2) ];
			sbp = src_line[2];
			color[8] = ScummColors16[ *sbp ];     color[9] = color[8];    color[10] = ScummColors16[ *(sbp + 1) ]; color[11] = ScummColors16[ *(sbp + 2) ];
			sbp = src_line[3];
			color[12] = ScummColors16[ *sbp ];    color[13] = color[12];  color[14] = ScummColors16[ *(sbp + 1) ]; color[15] = ScummColors16[ *(sbp + 2) ];

			if( src_x > 0 )
			{
				color[0] = ScummColors16[ src_line[0][-1] ];
				color[4] = ScummColors16[ src_line[1][-1] ];
				color[8] = ScummColors16[ src_line[2][-1] ];
				color[12] = ScummColors16[ src_line[3][-1] ];
			}
		}
		else
		{
			byte *lbp;
			lbp = src_line[0];
			color[0] = ScummColors[ *lbp ];     color[1] = color[0];    color[2] = ScummColors[ *(lbp + 1) ];  color[3] = ScummColors[ *(lbp + 2) ];
			lbp = src_line[1];
			color[4] = ScummColors[ *lbp ];     color[5] = color[4];    color[6] = ScummColors[ *(lbp + 1) ];  color[7] = ScummColors[ *(lbp + 2) ];
			lbp = src_line[2];
			color[8] = ScummColors[ *lbp ];     color[9] = color[8];    color[10] = ScummColors[ *(lbp + 1) ]; color[11] = ScummColors[ *(lbp + 2) ];
			lbp = src_line[3];
			color[12] = ScummColors[ *lbp ];    color[13] = color[12];  color[14] = ScummColors[ *(lbp + 1) ]; color[15] = ScummColors[ *(lbp + 2) ];
		}

		if (y < height - 1)
		{
			dst_line[0] = dst_line[1]+dest_pitch;
			dst_line[1] = dst_line[0]+dest_pitch;
		}
	}

	UnLockBitMap( handle );
}

void SuperEagle( uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height )
{
	unsigned int x, y;
	unsigned long color[12];
	byte *src;
	byte *dest;
	uint32 dest_bpp;
	uint32 dest_pitch;
	APTR handle;

	if( (handle = LockBitMapTags( ScummRenderTo, LBMI_BYTESPERPIX, &dest_bpp, LBMI_BYTESPERROW, &dest_pitch, LBMI_BASEADDRESS, &dest, TAG_DONE )) == 0 )
		return;

	src = (byte *)ScummBuffer+src_y*320+src_x;

	/* Point to the first 3 lines. */
	src_line[0] = src;
	src_line[1] = src;
	src_line[2] = src + 320;
	src_line[3] = src + 320 * 2;

	dst_line[0] = dest+dest_y*2*dest_pitch+dest_x*2*dest_bpp;
	dst_line[1] = dst_line[0]+dest_pitch;

	x = 0, y = 0;

	if (PixelsPerMask == 2)
	{
		byte *sbp;
		sbp = src_line[0];
		color[0] = ScummColors16[ *sbp ];       color[1] = color[0];   color[2] = color[0];    color[3] = color[0];
		color[4] = ScummColors16[ *(sbp + 1) ]; color[5] = ScummColors16[ *(sbp + 2) ];
		sbp = src_line[2];
		color[6] = ScummColors16[ *sbp ];     color[7] = color[6];     color[8] = ScummColors16[ *(sbp + 1) ]; color[9] = ScummColors16[ *(sbp + 2) ];
		sbp = src_line[3];
		color[10] = ScummColors16[ *sbp ];    color[11] = ScummColors16[ *(sbp + 1) ];
	}
	else
	{
		byte *lbp;
		lbp = src_line[0];
		color[0] = ScummColors[ *lbp ];       color[1] = color[0];   color[2] = color[0];    color[3] = color[0];
		color[4] = ScummColors[ *(lbp + 1) ]; color[5] = ScummColors[ *(lbp + 2) ];
		lbp = src_line[2];
		color[6] = ScummColors[ *lbp ];     color[7] = color[6];     color[8] = ScummColors[ *(lbp + 1) ]; color[9] = ScummColors[ *(lbp + 2) ];
		lbp = src_line[3];
		color[10] = ScummColors[ *lbp ];    color[11] = ScummColors[ *(lbp + 1) ];
	}

	for (y = 0; y < height; y++)
	{
		/* Todo: x = width - 2, x = width - 1 */
	
		for (x = 0; x < width; x++)
		{
			unsigned long product1a, product1b, product2a, product2b;

//---------------------------------------     B1 B2           0  1
//                                         4  5  6  S2 ->  2  3  4  5
//                                         1  2  3  S1     6  7  8  9
//                                            A1 A2          10 11

			if (color[7] == color[4] && color[3] != color[8])
			{
				product1b = product2a = color[7];

				if ((color[6] == color[7]) || (color[4] == color[1]))
					product1a = INTERPOLATE(color[7], INTERPOLATE(color[7], color[3]));
				else
					product1a = INTERPOLATE(color[3], color[4]);

				if ((color[4] == color[5]) || (color[7] == color[10]))
					product2b = INTERPOLATE(color[7], INTERPOLATE(color[7], color[8]));
				else
					product2b = INTERPOLATE(color[7], color[8]);
			}
			else if (color[3] == color[8] && color[7] != color[4])
			{
				product2b = product1a = color[3];

				if ((color[0] == color[3]) || (color[5] == color[9]))
					product1b = INTERPOLATE(color[3], INTERPOLATE(color[3], color[4]));
				else
					product1b = INTERPOLATE(color[3], color[1]);

				if ((color[8] == color[11]) || (color[2] == color[3]))
					product2a = INTERPOLATE(color[3], INTERPOLATE(color[3], color[2]));
				else
					product2a = INTERPOLATE(color[7], color[8]);

			}
			else if (color[3] == color[8] && color[7] == color[4])
			{
				register int r = 0;

				r += GET_RESULT(color[4], color[3], color[6], color[10]);
				r += GET_RESULT(color[4], color[3], color[2], color[0]);
				r += GET_RESULT(color[4], color[3], color[11], color[9]);
				r += GET_RESULT(color[4], color[3], color[1], color[5]);

				if (r > 0)
				{
					product1b = product2a = color[7];
					product1a = product2b = INTERPOLATE(color[3], color[4]);
				}
				else if (r < 0)
				{
					product2b = product1a = color[3];
					product1b = product2a = INTERPOLATE(color[3], color[4]);
				}
				else
				{
					product2b = product1a = color[3];
					product1b = product2a = color[7];
				}
			}
			else
			{
				product2b = product1a = INTERPOLATE(color[7], color[4]);
				product2b = Q_INTERPOLATE(color[8], color[8], color[8], product2b);
				product1a = Q_INTERPOLATE(color[3], color[3], color[3], product1a);

				product2a = product1b = INTERPOLATE(color[3], color[8]);
				product2a = Q_INTERPOLATE(color[7], color[7], color[7], product2a);
				product1b = Q_INTERPOLATE(color[4], color[4], color[4], product1b);
			}

			if (PixelsPerMask == 2)
			{
				*((unsigned long *) (&dst_line[0][x * 4])) = (product1a << 16) | product1b;
				*((unsigned long *) (&dst_line[1][x * 4])) = (product2a << 16) | product2b;
			}
			else
			{
				*((unsigned long *) (&dst_line[0][x * 8])) = product1a;
				*((unsigned long *) (&dst_line[0][x * 8 + 4])) = product1b;
				*((unsigned long *) (&dst_line[1][x * 8])) = product2a;
				*((unsigned long *) (&dst_line[1][x * 8 + 4])) = product2b;
			}

			/* Move color matrix forward */
			color[0] = color[1];
			color[2] = color[3]; color[3] = color[4]; color[4] = color[5];
			color[6] = color[7]; color[7] = color[8]; color[8] = color[9];
			color[10] = color[11];

			if (x < width - 2)
			{
				x += 2;
				if (PixelsPerMask == 2)
				{
					color[1] = ScummColors16[ *(src_line[0] + x) ];
					if( x < width-1 )
					{
						color[5] = ScummColors16[ *(src_line[1] + x + 1) ];
						color[9] = ScummColors16[ *(src_line[2] + x + 1) ];
					}
					color[11] = ScummColors16[ *(src_line[3] + x) ];
				}
				else
				{
					color[1] = ScummColors[ *(src_line[0] + x) ];
					if( x < width-1 )
					{
						color[5] = ScummColors[ *(src_line[1] + x + 1) ];
						color[9] = ScummColors[ *(src_line[2] + x + 1) ];
					}
					color[11] = ScummColors[ *(src_line[3] + x) ];
				}
				x -= 2;
			}
		}

		/* We're done with one line, so we shift the source lines up */
		src_line[0] = src_line[1];
		src_line[1] = src_line[2];
		src_line[2] = src_line[3];

		/* Read next line */
		if (y + 3 >= height)
			src_line[3] = src_line[2];
		else
			src_line[3] = src_line[2] + 320;

		/* Then shift the color matrix up */
		if (PixelsPerMask == 2)
		{
			byte *sbp;
			sbp = src_line[0];
			color[0] = ScummColors16[ *sbp ];     color[1] = ScummColors16[ *(sbp + 1) ];
			sbp = src_line[1];
			color[2] = ScummColors16[ *sbp ];     color[3] = color[2];    color[4] = ScummColors16[ *(sbp + 1) ];  color[5] = ScummColors16[ *(sbp + 2) ];
			sbp = src_line[2];
			color[6] = ScummColors16[ *sbp ];     color[7] = color[6];    color[8] = ScummColors16[ *(sbp + 1) ];  color[9] = ScummColors16[ *(sbp + 2) ];
			sbp = src_line[3];
			color[10] = ScummColors16[ *sbp ];    color[11] = ScummColors16[ *(sbp + 1) ];
		}
		else
		{
			byte *lbp;
			lbp = src_line[0];
			color[0] = ScummColors[ *lbp ];     color[1] = ScummColors[ *(lbp + 1) ];
			lbp = src_line[1];
			color[2] = ScummColors[ *lbp ];     color[3] = color[2];    color[4] = ScummColors[ *(lbp + 1) ];  color[5] = ScummColors[ *(lbp + 2) ];
			lbp = src_line[2];
			color[6] = ScummColors[ *lbp ];     color[7] = color[6];    color[8] = ScummColors[ *(lbp + 1) ];  color[9] = ScummColors[ *(lbp + 2) ];
			lbp = src_line[3];
			color[10] = ScummColors[ *lbp ];    color[11] = ScummColors[ *(lbp + 1) ];
		}


		if (y < height - 1)
		{
			dst_line[0] = dst_line[1]+dest_pitch;
			dst_line[1] = dst_line[0]+dest_pitch;
		}
	}

	UnLockBitMap( handle );
}

void PointScaler( uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height )
{
	byte *src;
	byte *dest;
	uint32 dest_bpp;
	uint32 dest_pixfmt;
	uint32 dest_pitch;
	uint32 color;
	uint32 r, g, b;
	uint32 x, y;
	APTR handle;

	if( (handle = LockBitMapTags( ScummRenderTo, LBMI_BYTESPERPIX, &dest_bpp,
																LBMI_BYTESPERROW, &dest_pitch,
																LBMI_BASEADDRESS, &dest,
																LBMI_PIXFMT, &dest_pixfmt,
																TAG_DONE )) == 0 )
		return;

	src = (byte *)ScummBuffer+src_y*320+src_x;

	dst_line[0] = dest+dest_y*2*dest_pitch+dest_x*2*dest_bpp;
	dst_line[1] = dst_line[0]+dest_pitch;

	for( y = 0; y < height; y++ )
	{
		for( x = 0; x < width; x++ )
		{
			r = (ScummColors[ *(src+x) ] >> 16) & 0xff;
			g = (ScummColors[ *(src+x) ] >> 8) & 0xff;
			b = ScummColors[ *(src+x) ] & 0xff;
			
			color = makeColForPixFmt( dest_pixfmt, r, g, b );
			if( PixelsPerMask == 2 )
			{
				*((unsigned long *) (&dst_line[0][x * 4])) = (color << 16) | color;
				*((unsigned long *) (&dst_line[1][x * 4])) = (color << 16) | color;
			}
			else
			{
				*((unsigned long *) (&dst_line[0][x * 8])) = color;
				*((unsigned long *) (&dst_line[0][x * 8 + 4])) = color;
				*((unsigned long *) (&dst_line[1][x * 8])) = color;
				*((unsigned long *) (&dst_line[1][x * 8 + 4])) = color;
			}
		}

		src += 320;
		
		if (y < height - 1)
		{
			dst_line[0] = dst_line[1]+dest_pitch;
			dst_line[1] = dst_line[0]+dest_pitch;
		}
	}

	UnLockBitMap( handle );
}

/* Copy part of bitmap */
void blitToScreen(Scumm *s, byte *src, int x, int y, int w, int h)
{
	byte *dst;

	hide_mouse = true;
	if( has_mouse )
		s->drawMouse();

	if( y < 0 )
	{
		h += y;
		src -= y*320;
		y = 0;
	}

	if( h > 200 - y )
		h = 200 - y;

	if( h<=0 )
		return;

	dst = (byte *)ScummBuffer+y*320 + x;
	do
	{
		memcpy( dst, src, w );
		dst += 320;
		src += 320;
	} while( --h );
}

void updateScreen(Scumm *s)
{
	if( s->_fastMode & 2 )
		return;

	if( hide_mouse )
	{
		hide_mouse = false;
		s->drawMouse();
	}

	if(s->_palDirtyMax != -1)
		updatePalette(s);

	if( !ScummScale )
	{
		struct RastPort rp;

		InitRastPort( &rp );
		rp.BitMap = ScummRenderTo;

		if( ScummDepth == 8 )
			WritePixelArray( ScummBuffer, 0, 0, 320, &rp, 0, ScummShakePos, 320, 200, RECTFMT_LUT8 );
		else
			WriteLUTPixelArray( ScummBuffer, 0, 0, 320, &rp, ScummColors, 0, ScummShakePos, 320, 200, CTABFMT_XRGB8 );
	}
	else
	{
		uint32 src_y = 0;
		uint32 dest_y = 0;
		if( ScummShakePos < 0 )
			src_y = -ScummShakePos;
		else
			dest_y = ScummShakePos;
		(*ScummScaler)( 0, src_y, 0, dest_y, 320, 200-src_y-dest_y );
	}

	/* Account for shaking (blacken rest of screen) */
	if( ScummShakePos )
	{
		struct RastPort rp;

		InitRastPort( &rp );
		rp.BitMap = ScummRenderTo;

		if( ScummShakePos < 0 )
			FillPixelArray( &rp, 0, 199 << ScummScale, ScummScrWidth, -ScummShakePos << ScummScale, 0 );
		else
			FillPixelArray( &rp, 0, 0, ScummScrWidth, ScummShakePos << ScummScale, 0 );
	}

	if( ScummScreen )
	{
		while( !ChangeScreenBuffer( ScummScreen, ScummScreenBuffer[ ScummPaintBuffer ] ) );
		WaitTOF();
		ScummPaintBuffer = !ScummPaintBuffer;
		ScummRenderTo = ScummScreenBuffer[ ScummPaintBuffer ]->sb_BitMap;
	}
	else
	{
		BltBitMapRastPort( ScummRenderTo, 0, 0, ScummWindow->RPort, ScummWindow->BorderLeft, ScummWindow->BorderTop, ScummScrWidth, ScummScrHeight, ABNC | ABC );
		WaitBlit();
	}
}

void drawMouse( Scumm *s, int xdraw, int ydraw, int w, int h, byte *buf, bool visible )
{
	int x,y;
	byte *dst,*bak;
	byte color;

	if( hide_mouse )
		visible = false;

	assert( w<=BAK_WIDTH && h<=BAK_HEIGHT );

	if( has_mouse )
	{
		dst = (byte*)ScummBuffer + old_mouse_y*320 + old_mouse_x;
		bak = old_backup;
	
		for( y = 0; y < old_mouse_h; y++, bak+=BAK_WIDTH, dst+=320 )
		{
			if( (uint)(old_mouse_y + y) < 200 )
			{
				for( x=0; x < old_mouse_w; x++ )
				{
					if( (uint)(old_mouse_x + x) < 320 )
						dst[ x ] = bak[ x ];
				}
			}
		}
	}

	if( visible )
	{
		dst = (byte*)ScummBuffer + ydraw*320 + xdraw;
		bak = old_backup;

		for( y = 0; y < h; y++, dst += 320, bak += BAK_WIDTH, buf += w )
		{
			if( (uint)(ydraw+y) < 200 )
			{
				for( x=0; x<w; x++ )
				{
					if( (uint)(xdraw+x)<320 )
					{
						bak[x] = dst[x];
						if( (color=buf[x])!=0xFF )
							dst[ x ] = color;
					}
				}
			}
		}
	}

	if( has_mouse )
		has_mouse = false;

	if( visible )
	{
		has_mouse = true;
		old_mouse_x = xdraw;
		old_mouse_y = ydraw;
		old_mouse_w = w;
		old_mouse_h = h;
	}
}


void launcherLoop()
{
#if 0
	int last_time, new_time;
	int delta = 0;
	last_time = GetTicks();

	gui.launcher();
	do
	{
		updateScreen(&scumm);

		new_time = GetTicks();
		waitForTimer( &scumm, delta * 15 + last_time - new_time );
		last_time = GetTicks();

		if (gui._active)
		{
			gui.loop();
			delta = 5;
		} else
			error("gui closed!");
	} while(1);
#endif
};

void setWindowName( Scumm *s )
{
	/* this is done in initGraphics() ... */
}

void initGraphics( Scumm *s, bool fullScreen, unsigned int scaleFactor )
{
	ScummNoCursor = (UWORD *)AllocVec( 16, MEMF_CHIP | MEMF_CLEAR );

	/*
	 * Allocate image buffer
	 */
	ScummBuffer = AllocVec( 320*200, MEMF_ANY | MEMF_CLEAR );

	if( ScummBuffer == NULL )
	{
		puts( "Couldn't allocate image buffer" );
		exit( 1 );
	}

	memset( ScummColors, 0, 256*sizeof( ULONG ) );

	char *gameName;
	sprintf( ScummWndTitle, "ScummVM MorphOS - %s", gameName = s->getGameName());
	free( gameName );

	createScreen( args[ USG_WBWINDOW ] ? CSDSPTYPE_WINDOWED : CSDSPTYPE_FULLSCREEN );

	InitSemaphore( &ScummMusicThreadRunning );

	// Prepare for CD audio if game is Loom CD version
	if( (s->_features & GF_AUDIOTRACKS) && !args[ USG_NOMUSIC ] )
	{
		FindCDTags[ 0 ].ti_Data = (ULONG)((s->_gameId == GID_LOOM256) ? "LoomCD" : "Monkey1CD");
		CDDABase = OpenLibrary( "cdda.library", 0 );
		if( CDDABase )
		{
			CDrive = CDDA_FindNextDrive( NULL, FindCDTags );
			if( CDrive )
			{
				if( !CDDA_ObtainDrive( CDrive, CDDA_SHARED_ACCESS, NULL ) )
				{
					CDrive = NULL;
					warning( "Failed to obtain CD drive - music will not play" );
				}
				else if( s->_gameId == GID_LOOM256 )
				{
					// Offset correction *may* be required
					struct CDS_TrackInfo ti;

					if( CDDA_GetTrackInfo( CDrive, 1, 0, &ti ) )
						CDDATrackOffset = ti.ti_TrackStart.tm_Format.tm_Frame-22650;
				}
			}
			else
				warning( "Could not find game CD inserted in CD-ROM drive - music will not play" );
		}
		else
			warning( "Failed to open cdda.library - music will not play" );
		args[ USG_NOMUSIC ] = TRUE;	// this avoids AMidi being opened ...
	}

	// Create Music Thread
	MyEmulFunc.Trap      = TRAP_FUNC;
	MyEmulFunc.Address	= (ULONG)&morphos_music_thread;
	MyEmulFunc.StackSize	= 8192;
	MyEmulFunc.Extension	= 0;
	MyEmulFunc.Arg1	   = (ULONG)&scumm;
	MyEmulFunc.Arg2	   = (ULONG)ScummMidiUnit;
	MyEmulFunc.Arg3	   = (ULONG)args[ USG_NOMUSIC ];
	ScummMusicThread = CreateNewProc( musicProcTags );
	if( !ScummMusicThread )
	{
		puts( "Failed to create music thread" );
		exit( 1 );
	}
}

static bool FindScaler( const char *ScalerName )
{
	int scaler = 0;
	
	while( ScummScalers[ scaler ].gs_Name )
	{
		if( !stricmp( ScalerName, ScummScalers[ scaler ].gs_Name ) )
		{
			ScummScaler = ScummScalers[ scaler ].gs_Function;
			break;
		}
		scaler++;
	}

	if( ScummScalers[ scaler ].gs_Name == NULL )
	{
		puts( "Invalid scaler name. Please use one of the following:" );
		for( scaler = 0; ScummScalers[ scaler ].gs_Name != NULL; scaler++ )
			printf( "  %s\n", ScummScalers[ scaler ].gs_Name );
		return false;
	}

	ScummScaler = ScummScalers[ scaler ].gs_Function;
	if( ScummScaler == NULL )
		ScummScale = 0;
	else
		ScummScale = 1;

	return true;
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
		if( !FindScaler( ToolValue ) )
		{
			FreeDiskObject( dobj );
			exit( 1 );
		}
	}

	if( ToolValue = (char *)FindToolType( dobj->do_ToolTypes, "MUSIC" ) )
	{
		if( MatchToolValue( ToolValue, "YES" ) )
			args[ USG_NOMUSIC ] = FALSE;
		else if( MatchToolValue( ToolValue, "NO" ) )
			args[ USG_NOMUSIC ] = TRUE;
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

	if( ToolValue = (char *)FindToolType( dobj->do_ToolTypes, "SUBTITLES" ) )
	{
		if( MatchToolValue( ToolValue, "YES" ) )
			args[ USG_NOSUBTITLES ] = FALSE;
		else if( MatchToolValue( ToolValue, "NO" ) )
			args[ USG_NOSUBTITLES ] = TRUE;
	}

	FreeDiskObject( dobj );
}

#undef main

int main( int argc, char *argv[] )
{
	int delta;
	int last_time, new_time;

	CyberGfxBase = OpenLibrary( "cybergraphics.library", 41 );
	if( CyberGfxBase == NULL )
	{
		puts( "Failed to open cybergraphics.library" );
		exit( 1 );
	}

	atexit( &closeResources );

	TimerMsgPort = CreateMsgPort();
	if( TimerMsgPort == NULL )
	{
		puts( "Failed to create message port" );
		exit( 1 );
	}

	TimerIORequest = (struct timerequest *)CreateIORequest( TimerMsgPort, sizeof( struct timerequest ) );
	if( TimerIORequest == NULL )
	{
		puts( "Failed to create IO request" );
		exit( 1 );
	}

	if( OpenDevice( "timer.device", UNIT_MICROHZ, (struct IORequest *)TimerIORequest, 0 ) )
	{
		DeleteIORequest( (struct IORequest *)TimerIORequest );
		TimerIORequest = NULL;
		puts( "Failed to open timer device" );
		exit( 1 );
	}

	TimerBase = TimerIORequest->tr_node.io_Device;

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
			if( !FindScaler( (char *)args[ USG_SCALER ] ) )
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

	scumm._gui = &gui;
	sound.initialize( &scumm, &snd_driv );
	char *argvfake[] = { "ScummVM", (char *)ScummStory, "-e5", (char *)ScummPath };
	scumm.scummMain( ScummPath ? 4 : 3, argvfake);

	/* Fix up argument flags */
	if( args[ USG_NOSUBTITLES ] )
		scumm._noSubtitles = true;
	if( args[ USG_VOLUME ] )
	{
		int vol = *(LONG *)args[ USG_VOLUME ];
		if( vol >= 0 && vol <= 100 )
			sound.set_music_volume( vol );
	}
	if( args[ USG_TEMPO ] )
		scumm._gameTempo = *(LONG *)args[ USG_TEMPO ];

	gui.init(&scumm);

	last_time = GetTicks();
	delta = 0;
	do
	{
		updateScreen( &scumm );

		new_time = GetTicks();
		waitForTimer( &scumm, delta * 15 + last_time - new_time );
		last_time = GetTicks();

		if( gui._active )
		{
			gui.loop();
			delta = 5;
		}
		else
			delta = scumm.scummLoop( delta );
	} while( 1 );

	return 0;
}

