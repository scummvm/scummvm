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

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <devices/ahi.h>
#include <dos/dostags.h>
#include <intuition/screens.h>
#include <cybergraphics/cybergraphics.h>
#include <devices/inputevent.h>
#include <intuition/intuition.h>

#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/keymap.h>
#include <proto/timer.h>
#include <proto/cdda.h>
#include <proto/cybergraphics.h>

#include <emul/emulinterface.h>

#include <time.h>

#include "morphos.h"

static struct TagItem FindCDTags[] = { { CDFA_VolumeName, 0 },
													{ TAG_DONE,        0 }
												 };
static struct TagItem PlayTags[] =   { { CDPA_StartTrack, 1 },
													{ CDPA_StartFrame, 0 },
													{ CDPA_EndTrack, 	 1 },
													{ CDPA_EndFrame, 	 0 },
													{ CDPA_Loops, 	    1 },
													{ TAG_DONE,			 0	}
												 };

OSystem_MorphOS::GfxScaler OSystem_MorphOS::ScummScalers[ 10 ] = { { "none", ST_NONE },
																  { "Point", 		ST_POINT },
																  { "AdvMame2x",  ST_ADVMAME2X },
																  { "SuperEagle", ST_SUPEREAGLE },
																  { "Super2xSaI", ST_SUPER2XSAI },
																  { NULL, ST_INVALID },
																  { NULL, ST_INVALID },
																  { NULL, ST_INVALID },
																  { NULL, ST_INVALID },
																  { NULL, ST_INVALID }
																};

struct TagItem musicProcTags[] = { { NP_Entry, 	     0							     		},
											  { NP_Name, 	     (ULONG)"ScummVM Music Thread" 	},
											  { NP_Priority,    0  								      },
											  { TAG_DONE,       0 						     			}
											};
struct TagItem soundProcTags[] = { { NP_Entry, 	     0							     		},
											  { NP_Name, 	     (ULONG)"ScummVM Sound Thread" 	},
											  { NP_Priority,    0  								      },
											  { TAG_DONE,       0 						     			}
											};

OSystem_MorphOS *OSystem_MorphOS::create( int game_id, SCALERTYPE gfx_scaler, bool full_screen )
{
	OSystem_MorphOS *syst = new OSystem_MorphOS( game_id, gfx_scaler, full_screen );

	return syst;
}

OSystem_MorphOS::OSystem_MorphOS( int game_id, SCALERTYPE gfx_mode, bool full_screen )
{
	GameID = game_id;
	ScummScreen = NULL;
	ScummWindow = NULL;
	ScummBuffer = NULL;
	ScummScreenBuffer[ 0 ] = NULL;
	ScummScreenBuffer[ 1 ] = NULL;
	ScummRenderTo = NULL;
	ScummNoCursor = NULL;
	ScummMusicThread = NULL;
	ScummSoundThread = NULL;
	ScummWinX = -1;
	ScummWinY = -1;
	ScummDefaultMouse = false;
	ScummOrigMouse = false;
	ScummShakePos = 0;
	ScummPCMode = false;
	ScummScaler = gfx_mode;
	ScummScale  = (gfx_mode == ST_NONE) ? 0 : 1;
	ScummDepth = 0;
	Scumm16ColFmt16 = false;
	ScummScrWidth = 0;
	ScummScrHeight = 0;
	ScreenChanged = false;
	FullScreenMode = full_screen;
	CDrive = NULL;
	CDDATrackOffset = 0;
	strcpy( ScummWndTitle, "ScummVM MorphOS" );
	TimerMsgPort = NULL;
	TimerIORequest = NULL;

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

	ScummNoCursor = (UWORD *)AllocVec( 16, MEMF_CHIP | MEMF_CLEAR );
}

OSystem_MorphOS::~OSystem_MorphOS()
{
	if( CDrive && CDDABase )
	{
		CDDA_Stop( CDrive );
		CDDA_ReleaseDrive( CDrive );
	}

	if( TimerIORequest )
	{
		CloseDevice( (struct IORequest *)TimerIORequest );
		DeleteIORequest( (struct IORequest *)TimerIORequest );
	}

	if( TimerMsgPort )
		DeleteMsgPort( TimerMsgPort );

	if( ScummMusicThread )
	{
		Signal( (struct Task *)ScummMusicThread, SIGBREAKF_CTRL_C );
		ObtainSemaphore( &ScummMusicThreadRunning );		/* Wait for thread to finish */
		ReleaseSemaphore( &ScummMusicThreadRunning );
	}

	if( ScummSoundThread )
	{
		Signal( (struct Task *)ScummSoundThread, SIGBREAKF_CTRL_C );
		ObtainSemaphore( &ScummSoundThreadRunning );		/* Wait for thread to finish */
		ReleaseSemaphore( &ScummSoundThreadRunning );
	}

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
}

uint32 OSystem_MorphOS::get_msecs()
{
	int ticks = clock();
	ticks *= (1000/CLOCKS_PER_SEC);
	return ticks;
}

void OSystem_MorphOS::delay_msecs(uint msecs)
{
	TimerIORequest->tr_node.io_Command  = TR_ADDREQUEST;
	TimerIORequest->tr_time.tv_secs  = 0;
	TimerIORequest->tr_time.tv_micro = msecs*1000;
	DoIO( (struct IORequest *)TimerIORequest );
}

void *OSystem_MorphOS::create_thread(ThreadProc *proc, void *param)
{
	static EmulFunc ThreadEmulFunc;

	ThreadEmulFunc.Trap      = TRAP_FUNC;
	ThreadEmulFunc.Address	 = (ULONG)proc;
	ThreadEmulFunc.StackSize = 16000;
	ThreadEmulFunc.Extension = 0;
	ThreadEmulFunc.Arg1	    = (ULONG)param;
	musicProcTags[ 0 ].ti_Data = (ULONG)&ThreadEmulFunc;
	ScummMusicThread = CreateNewProc( musicProcTags );
	return ScummMusicThread;
}

uint32 OSystem_MorphOS::property(int param, Property *value)
{
	switch( param )
	{
		case PROP_TOGGLE_FULLSCREEN:
			create_screen( CSDSPTYPE_TOGGLE );
			return 1;

		case PROP_SET_WINDOW_CAPTION:
			sprintf( ScummWndTitle, "ScummVM MorphOS - %s", value->caption );
			if( ScummWindow )
				SetWindowTitles( ScummWindow, ScummWndTitle, ScummWndTitle );
			return 1;

		case PROP_OPEN_CD:
			FindCDTags[ 0 ].ti_Data = (ULONG)((GameID == GID_LOOM256) ? "LoomCD" : "Monkey1CD");
			if( !CDDABase ) CDDABase = OpenLibrary( "cdda.library", 2 );
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
					else if( GameID == GID_LOOM256 )
					{
						// Offset correction *may* be required
						struct CDS_TrackInfo ti = { sizeof( struct CDS_TrackInfo ) };

						if( CDDA_GetTrackInfo( CDrive, 1, 0, &ti ) )
							CDDATrackOffset = ti.ti_TrackStart.tm_Format.tm_Frame-22650;
					}
				}
				else
					warning( "Could not find game CD inserted in CD-ROM drive - cd audio will not play" );
			}
			else
				warning( "Failed to open cdda.library - cd audio will not play" );
			break;

		case PROP_SHOW_DEFAULT_CURSOR:
			if( value->show_cursor )
				ClearPointer( ScummWindow );
			else
				SetPointer( ScummWindow, ScummNoCursor, 1, 1, 0, 0 );
			ScummOrigMouse = ScummDefaultMouse = value->show_cursor;
			break;

		case PROP_GET_SAMPLE_RATE:
			return SAMPLES_PER_SEC;
	}

	return 0;
}

void OSystem_MorphOS::play_cdrom( int track, int num_loops, int start_frame, int length )
{
	if( CDrive && start_frame >= 0 )
	{
		if( start_frame > 0 )
			start_frame -= CDDATrackOffset;

		PlayTags[ 0 ].ti_Data = track;
		PlayTags[ 1 ].ti_Data = start_frame;
		PlayTags[ 2 ].ti_Data = (length == 0) ? track+1 : track;
		PlayTags[ 3 ].ti_Data = length ? start_frame+length : 0;
		PlayTags[ 4 ].ti_Data = (num_loops == 0) ? 1 : num_loops;
		CDDA_Play( CDrive, PlayTags );
	}
}

// Schedule the music to be stopped after 1/10 sec, unless another
// track is started in the meantime.
void OSystem_MorphOS::stop_cdrom()
{
	CDDA_Stop( CDrive );
}

bool OSystem_MorphOS::poll_cdrom()
{
	ULONG status;

	if( CDrive == NULL )
		return false;

	CDDA_GetAttr( CDDA_Status, CDrive, &status );
	return status == CDDA_Status_Busy;
}

void OSystem_MorphOS::update_cdrom()
{
}

void OSystem_MorphOS::quit()
{
	exit( 0 );
}

uint32 OSystem_MorphOS::make_color( int pixfmt, int r, int g, int b )
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


	return col;
}

#define CVT8TO32( col )   ((col<<24) | (col<<16) | (col<<8) | col)

void OSystem_MorphOS::set_palette(const byte *colors, uint start, uint num)
{
	const byte *data = colors;
	for( uint i = start; i != start+num; i++ )
	{
		if( ScummDepth == 8 )
			SetRGB32( &ScummScreen->ViewPort, i, CVT8TO32( data[ 0 ] ), CVT8TO32( data[ 1 ] ), CVT8TO32( data[ 2 ] ) );
		ScummColors16[ i ] = Scumm16ColFmt16 ? (((data[ 0 ]*31)/255) << 11) | (((data[ 1 ]*63)/255) << 5) | ((data[ 2 ]*31)/255) : (((data[ 0 ]*31)/255) << 10) | (((data[ 1 ]*31)/255) << 5) | ((data[ 2 ]*31)/255);
		ScummColors[ i ] = (data[ 0 ] << 16) | (data[ 1 ] << 8) | data[ 2 ];
		data += 4;
	}
	ScreenChanged = true;
}

void OSystem_MorphOS::create_screen( CS_DSPTYPE dspType )
{
	ULONG mode = INVALID_ID;
	int   depths[] = { 8, 15, 16, 24, 32, 0 };
	int   i;
	struct Screen *wb = NULL;

	if( dspType != CSDSPTYPE_KEEP )
		FullScreenMode = (dspType == CSDSPTYPE_FULLSCREEN) || (dspType == CSDSPTYPE_TOGGLE && !FullScreenMode);

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
	
	ScummScrWidth  = ScummBufferWidth << ScummScale;
	ScummScrHeight = ScummBufferHeight << ScummScale;

	if( FullScreenMode )
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
														SA_Width,		STDSCREENWIDTH,
														SA_Height,		STDSCREENHEIGHT,
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

		ULONG	RealDepth = GetBitMapAttr( &ScummScreen->BitMap, BMA_DEPTH );
		if( RealDepth != ScummDepth )
		{
			warning( "Screen did not open in expected depth" );
			ScummDepth = RealDepth;
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

		// Make both buffers black to avoid grey strip on bottom of screen
		struct RastPort rp;
		InitRastPort( &rp );
		SetRGB32( &ScummScreen->ViewPort, 0, 0, 0, 0 );
		rp.BitMap = ScummScreenBuffer[ 0 ]->sb_BitMap;
		FillPixelArray( &ScummScreen->RastPort, 0, 0, ScummScreen->Width, ScummScreen->Height, 0 );
		rp.BitMap = ScummRenderTo;
		FillPixelArray( &rp, 0, 0, ScummScreen->Width, ScummScreen->Height, 0 );

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
													WA_InnerWidth,	   FullScreenMode ? ScummScreen->Width : ScummScrWidth,
													WA_InnerHeight,   FullScreenMode ? ScummScreen->Height : ScummScrHeight,
													WA_Activate,		TRUE,
													WA_Title,		   wb ? ScummWndTitle : NULL,
													WA_ScreenTitle,	wb ? ScummWndTitle : NULL,
													WA_Borderless,		FullScreenMode,
													WA_CloseGadget,	!FullScreenMode,
													WA_DepthGadget,	!FullScreenMode,
													WA_DragBar,			!FullScreenMode,
													WA_ReportMouse,	TRUE,
													WA_RMBTrap,			TRUE,
													WA_IDCMP,			IDCMP_RAWKEY 		|
																			IDCMP_MOUSEMOVE 	|
																			IDCMP_CLOSEWINDOW |
																			IDCMP_MOUSEBUTTONS,
													WA_CustomScreen,  FullScreenMode ? (ULONG)ScummScreen : (ULONG)wb,
													TAG_DONE
										 );

	if( wb )
		UnlockPubScreen( NULL, wb );

	if( ScummWindow == NULL )
	{
		error( "Failed to open window" );
		exit(1);
	}

	if( !ScummDefaultMouse )
	{
		SetPointer( ScummWindow, ScummNoCursor, 1, 1, 0, 0 );
		ScummOrigMouse = false;
	}

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

		ScummPCMode = false;
		if( pixfmt == PIXFMT_RGB15PC || pixfmt == PIXFMT_BGR15PC ||
			 pixfmt == PIXFMT_RGB16PC || pixfmt == PIXFMT_BGR16PC
		  )
			ScummPCMode = true;
		debug( 1, "Pixelformat = %d", pixfmt );

		colorMask = (make_color( pixfmt, 255, 0, 0 ) - minr) | (make_color( pixfmt, 0, 255, 0 ) - ming) | (make_color( pixfmt, 0, 0, 255 ) - minb);
		lowPixelMask = minr | ming | minb;
		qcolorMask = (make_color( pixfmt, 255, 0, 0 ) - 3*minr) | (make_color( pixfmt, 0, 255, 0) - 3*ming) | (make_color( pixfmt, 0, 0, 255 ) - 3*minb);
		qlowpixelMask = (minr * 3) | (ming * 3) | (minb * 3);
		redblueMask = make_color( pixfmt, 255, 0, 255 );
		greenMask = make_color( pixfmt, 0, 255, 0 );

		PixelsPerMask = (ScummDepth <= 16) ? 2 : 1;

		if( PixelsPerMask == 2 )
		{
			colorMask |= (colorMask << 16);
			qcolorMask |= (qcolorMask << 16);
			lowPixelMask |= (lowPixelMask << 16);
			qlowpixelMask |= (qlowpixelMask << 16);
		}
	}
	ScreenChanged = true;
}

void OSystem_MorphOS::SwitchScalerTo( SCALERTYPE newScaler )
{
	if( newScaler == ST_NONE && ScummScale != 0 )
	{
		ScummScale = 0;
		ScummScaler = ST_NONE;
		create_screen( ScummScreen ? CSDSPTYPE_FULLSCREEN : CSDSPTYPE_WINDOWED );
	}
	else
	{
		if( ScummScale == 0 )
		{
			ScummScale = 1;
			create_screen( ScummScreen ? CSDSPTYPE_FULLSCREEN : CSDSPTYPE_WINDOWED );
		}

		if( ScummScaler != newScaler )
			ScummScaler = newScaler;
	}
}

bool OSystem_MorphOS::poll_event( Event *event )
{
	struct IntuiMessage *ScummMsg;

	if( ScummMsg = (struct IntuiMessage *)GetMsg( ScummWindow->UserPort ) )
	{
		switch( ScummMsg->Class )
		{
			case IDCMP_RAWKEY:
			{
				struct InputEvent FakedIEvent;
				char charbuf;
            int  qual = 0;

				memset( &FakedIEvent, 0, sizeof( struct InputEvent ) );
				FakedIEvent.ie_Class = IECLASS_RAWKEY;
				FakedIEvent.ie_Code = ScummMsg->Code;

				if( ScummMsg->Qualifier & (IEQUALIFIER_LALT | IEQUALIFIER_RALT) )
					qual |= KBD_ALT;
				if( ScummMsg->Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT) )
					qual |= KBD_SHIFT;
				if( ScummMsg->Qualifier & IEQUALIFIER_CONTROL )
					qual |= KBD_CTRL;
				event->kbd.flags = qual;

				event->event_code = EVENT_KEYDOWN;

				if( ScummMsg->Code >= 0x50 && ScummMsg->Code <= 0x59 )
				{
					/*
					 * Function key
					 */
					event->kbd.ascii = (ScummMsg->Code-0x50)+315;
					event->kbd.keycode = 0;
				}
				else if( MapRawKey( &FakedIEvent, &charbuf, 1, NULL ) == 1 )
				{
					if( qual == KBD_CTRL )
					{
						switch( charbuf )
						{
							case 'z':
								ReplyMsg( (struct Message *)ScummMsg );
								exit(1);
						}
					}
					else if( qual == KBD_ALT )
					{
						if( charbuf >= '0' && charbuf <= '9' && ScummScalers[ charbuf-'0' ].gs_Name )
						{
							ReplyMsg( (struct Message *)ScummMsg );
							SwitchScalerTo( ScummScalers[ charbuf-'0' ].gs_Type );
                     return false;
						}
						else if( charbuf == 0x0d )
						{
							ReplyMsg( (struct Message *)ScummMsg );
							create_screen( CSDSPTYPE_TOGGLE );
                     return false;
						}
					}

					event->kbd.ascii = charbuf;
					event->kbd.keycode = event->kbd.ascii;
				}
				break;
			}

			case IDCMP_MOUSEMOVE:
			{
				int newx,newy;

				newx = (ScummMsg->MouseX-ScummWindow->BorderLeft) >> ScummScale;
				newy = (ScummMsg->MouseY-ScummWindow->BorderTop) >> ScummScale;

				if( !FullScreenMode && !ScummDefaultMouse )
				{
					if( newx < 0 || newx > ScummBufferWidth ||
						 newy < 0 || newy > ScummBufferHeight
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
				}
				else if( FullScreenMode )
					newy = newy <? (ScummScrHeight >> ScummScale)-2;

				event->event_code = EVENT_MOUSEMOVE;
				event->mouse.x = newx;
				event->mouse.y = newy;
				break;
			}

			case IDCMP_MOUSEBUTTONS:
         {
				int newx,newy;

				newx = (ScummMsg->MouseX-ScummWindow->BorderLeft) >> ScummScale;
				newy = (ScummMsg->MouseY-ScummWindow->BorderTop) >> ScummScale;

				switch( ScummMsg->Code )
           	{
               case SELECTDOWN:
						event->event_code = EVENT_LBUTTONDOWN;
                  break;

               case SELECTUP:
						event->event_code = EVENT_LBUTTONUP;
                  break;

					case MENUDOWN:
						event->event_code = EVENT_RBUTTONDOWN;
                  break;

               case MENUUP:
						event->event_code = EVENT_RBUTTONUP;
                  break;

               default:
               	ReplyMsg( (struct Message *)ScummMsg );
                  return false;
            }
				event->mouse.x = newx;
				event->mouse.y = newy;
				break;
         }

			case IDCMP_CLOSEWINDOW:
				ReplyMsg( (struct Message *)ScummMsg );
				exit( 0 );
		}

		if( ScummMsg )
			ReplyMsg( (struct Message *)ScummMsg );

		return true;
	}

	return false;
}

void OSystem_MorphOS::set_shake_pos( int shake_pos )
{
	ScummShakePos = shake_pos;
	ScreenChanged = true;
}

#define GET_RESULT(A, B, C, D) ((A != C || A != D) - (B != C || B != D))

#define INTERPOLATE(A, B) (((A & colorMask) >> 1) + ((B & colorMask) >> 1) + (A & B & lowPixelMask))

#define Q_INTERPOLATE(A, B, C, D) ((A & qcolorMask) >> 2) + ((B & qcolorMask) >> 2) + ((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2) + ((((A & qlowpixelMask) + (B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask)) >> 2) & qlowpixelMask)

#define SWAP_WORD( word ) word = ((word & 0xff) << 8) | (word >> 8)

void OSystem_MorphOS::Super2xSaI( uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height )
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

	src = ((byte *)ScummBuffer)+src_y*ScummBufferWidth+src_x;

	/* Point to the first 3 lines. */
	src_line[0] = src;
	src_line[1] = src;
	src_line[2] = src + ScummBufferWidth;
	src_line[3] = src + ScummBufferWidth * 2;

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
				if( ScummPCMode )
				{
					SWAP_WORD( product1a );
					SWAP_WORD( product1b );
					SWAP_WORD( product2a );
					SWAP_WORD( product2b );
				}
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
			src_line[3] = src_line[2] + ScummBufferWidth;

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

void OSystem_MorphOS::SuperEagle( uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height )
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

	src = (byte *)ScummBuffer+src_y*ScummBufferWidth+src_x;

	/* Point to the first 3 lines. */
	src_line[0] = src;
	src_line[1] = src;
	src_line[2] = src + ScummBufferWidth;
	src_line[3] = src + ScummBufferWidth * 2;

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
				if( ScummPCMode )
				{
					SWAP_WORD( product1a );
					SWAP_WORD( product1b );
					SWAP_WORD( product2a );
					SWAP_WORD( product2b );
				}
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
			src_line[3] = src_line[2] + ScummBufferWidth;

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

void OSystem_MorphOS::AdvMame2xScaler( uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height )
{
	byte  *dest;
	uint32 dest_bpp;
	uint32 dest_pitch;
	APTR handle;

	if( (handle = LockBitMapTags( ScummRenderTo, LBMI_BYTESPERPIX, &dest_bpp, LBMI_BYTESPERROW, &dest_pitch, LBMI_BASEADDRESS, &dest, TAG_DONE )) == 0 )
		return;

	byte *src = (byte *)ScummBuffer+src_y*ScummBufferWidth+src_x;

	src_line[0] = src;
	src_line[1] = src;
	src_line[2] = src + ScummBufferWidth;

	dst_line[0] = dest+dest_y*2*dest_pitch+dest_x*2*dest_bpp;
	dst_line[1] = dst_line[0]+dest_pitch;

	for( int y = 0; y < height; y++ )
	{
		for( int x = 0; x < width; x++ )
		{
			uint32 B, D, E, F, H;

			if( PixelsPerMask == 2 )
			{
				// short A = *(src + i - nextlineSrc - 1);
				B = ScummColors16[ src_line[ 0 ][ x ] ];
				// short C = *(src + i - nextlineSrc + 1);
				D = ScummColors16[ src_line[ 1 ][ x-1 ] ];
				E = ScummColors16[ src_line[ 1 ][ x ] ];
				F = ScummColors16[ src_line[ 1 ][ x+1 ] ];
				// short G = *(src + i + nextlineSrc - 1);
				H = ScummColors16[ src_line[ 2 ][ x ] ];
				// short I = *(src + i + nextlineSrc + 1);
			}
			else
			{
				// short A = *(src + i - nextlineSrc - 1);
				B = ScummColors[ src_line[ 0 ][ x ] ];
				// short C = *(src + i - nextlineSrc + 1);
				D = ScummColors[ src_line[ 1 ][ x-1 ] ];
				E = ScummColors[ src_line[ 1 ][ x ] ];
				F = ScummColors[ src_line[ 1 ][ x+1 ] ];
				// short G = *(src + i + nextlineSrc - 1);
				H = ScummColors[ src_line[ 2 ][ x ] ];
				// short I = *(src + i + nextlineSrc + 1);
			}


			if (PixelsPerMask == 2)
			{
				if( ScummPCMode )
				{
					SWAP_WORD( B );
					SWAP_WORD( D );
					SWAP_WORD( E );
					SWAP_WORD( F );
					SWAP_WORD( H );
				}
				*((unsigned long *) (&dst_line[0][x * 4])) = ((D == B && B != F && D != H ? D : E) << 16) | (B == F && B != D && F != H ? F : E);
				*((unsigned long *) (&dst_line[1][x * 4])) = ((D == H && D != B && H != F ? D : E) << 16) | (H == F && D != H && B != F ? F : E);
			}
			else
			{
				*((unsigned long *) (&dst_line[0][x * 8])) = D == B && B != F && D != H ? D : E;
				*((unsigned long *) (&dst_line[0][x * 8 + 4])) = B == F && B != D && F != H ? F : E;
				*((unsigned long *) (&dst_line[1][x * 8])) = D == H && D != B && H != F ? D : E;
				*((unsigned long *) (&dst_line[1][x * 8 + 4])) = H == F && D != H && B != F ? F : E;
			}
		}

		src_line[0] = src_line[1];
		src_line[1] = src_line[2];
		if (y + 2 >= height)
			src_line[2] = src_line[1];
		else
			src_line[2] = src_line[1] + ScummBufferWidth;

		if( y < height - 1 )
		{
			dst_line[0] = dst_line[1]+dest_pitch;
			dst_line[1] = dst_line[0]+dest_pitch;
		}
	}

	UnLockBitMap( handle );
}

void OSystem_MorphOS::PointScaler( uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height )
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

	src = (byte *)ScummBuffer+src_y*ScummBufferWidth+src_x;

	dst_line[0] = dest+dest_y*2*dest_pitch+dest_x*2*dest_bpp;
	dst_line[1] = dst_line[0]+dest_pitch;

	for( y = 0; y < height; y++ )
	{
		for( x = 0; x < width; x++ )
		{
			r = (ScummColors[ *(src+x) ] >> 16) & 0xff;
			g = (ScummColors[ *(src+x) ] >> 8) & 0xff;
			b = ScummColors[ *(src+x) ] & 0xff;
			
			color = make_color( dest_pixfmt, r, g, b );
			if( PixelsPerMask == 2 )
			{
				if( ScummPCMode )
					SWAP_WORD( color );

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

		src += ScummBufferWidth;
		
		if (y < height - 1)
		{
			dst_line[0] = dst_line[1]+dest_pitch;
			dst_line[1] = dst_line[0]+dest_pitch;
		}
	}

	UnLockBitMap( handle );
}

/* Copy part of bitmap */
void OSystem_MorphOS::copy_rect(const byte *src, int pitch, int x, int y, int w, int h)
{
	byte *dst;

	if (x < 0) { w+=x; src-=x; x = 0; }
	if (y < 0) { h+=y; src-=y*pitch; y = 0; }
	if (w >= ScummBufferWidth-x) { w = ScummBufferWidth - x; }
	if (h >= ScummBufferHeight-y) { h = ScummBufferHeight - y; }

	if (w<=0 || h<=0)
		return;

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	if( MouseDrawn )
		undraw_mouse();

	dst = (byte *)ScummBuffer+y*ScummBufferWidth + x;
	do
	{
		memcpy( dst, src, w );
		dst += ScummBufferWidth;
		src += pitch;
	} while( --h );
	ScreenChanged = true;
}

void OSystem_MorphOS::update_screen()
{
	if( !ScreenChanged )
		return;

	draw_mouse();

	if( !ScummScale )
	{
		struct RastPort rp;

		InitRastPort( &rp );
		rp.BitMap = ScummRenderTo;

		if( ScummDepth == 8 )
			WritePixelArray( ScummBuffer, 0, 0, ScummBufferWidth, &rp, 0, ScummShakePos, ScummBufferWidth, ScummBufferHeight, RECTFMT_LUT8 );
		else
			WriteLUTPixelArray( ScummBuffer, 0, 0, ScummBufferWidth, &rp, ScummColors, 0, ScummShakePos, ScummBufferWidth, ScummBufferHeight, CTABFMT_XRGB8 );
	}
	else
	{
		uint32 src_y = 0;
		uint32 dest_y = 0;
		if( ScummShakePos < 0 )
			src_y = -ScummShakePos;
		else
			dest_y = ScummShakePos;
		
		switch( ScummScaler )
		{
			case ST_POINT:
				PointScaler( 0, src_y, 0, dest_y, ScummBufferWidth, ScummBufferHeight-src_y-dest_y );
				break;

			case ST_ADVMAME2X:
				AdvMame2xScaler( 0, src_y, 0, dest_y, ScummBufferWidth, ScummBufferHeight-src_y-dest_y );
				break;

			case ST_SUPEREAGLE:
				SuperEagle( 0, src_y, 0, dest_y, ScummBufferWidth, ScummBufferHeight-src_y-dest_y );
				break;

			case ST_SUPER2XSAI:
				Super2xSaI( 0, src_y, 0, dest_y, ScummBufferWidth, ScummBufferHeight-src_y-dest_y );
				break;
		}
	}

	/* Account for shaking (blacken rest of screen) */
	if( ScummShakePos )
	{
		struct RastPort rp;

		InitRastPort( &rp );
		rp.BitMap = ScummRenderTo;

		if( ScummShakePos < 0 )
			FillPixelArray( &rp, 0, (ScummBufferHeight-1) << ScummScale, ScummScrWidth, -ScummShakePos << ScummScale, 0 );
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
	ScreenChanged = false;
}

void OSystem_MorphOS::draw_mouse()
{
	int x,y;
	byte *dst,*bak;
	byte color;

	if( MouseDrawn || !MouseVisible )
		return;
	MouseDrawn = true;

	const int ydraw = MouseY - MouseHotspotY;
	const int xdraw = MouseX - MouseHotspotX;
	const int w = MouseWidth;
	const int h = MouseHeight;
	bak = MouseBackup;
	byte *buf = MouseImage;

	MouseOldX = xdraw;
	MouseOldY = ydraw;
	MouseOldWidth = w;
	MouseOldHeight = h;

	dst = (byte*)ScummBuffer + ydraw*ScummBufferWidth + xdraw;
	bak = MouseBackup;

	for( y = 0; y < h; y++, dst += ScummBufferWidth, bak += MAX_MOUSE_W, buf += w )
	{
		if( (uint)(ydraw+y) < ScummBufferHeight )
		{
			for( x = 0; x<w; x++ )
			{
				if( (uint)(xdraw+x) < ScummBufferWidth )
				{
					bak[x] = dst[x];
					if( (color=buf[x])!=0xFF )
						dst[ x ] = color;
				}
			}
		}
	}
}

void OSystem_MorphOS::undraw_mouse()
{
	int x,y;
	byte *dst,*bak;

	if( !MouseDrawn )
		return;
	MouseDrawn = false;

	dst = (byte*)ScummBuffer + MouseOldY*ScummBufferWidth + MouseOldX;
	bak = MouseBackup;

	for( y = 0; y < MouseOldHeight; y++, bak += MAX_MOUSE_W, dst += ScummBufferWidth )
	{
		if( (uint)(MouseOldY + y) < ScummBufferHeight )
		{
			for( x = 0; x < MouseOldWidth; x++ )
			{
				if( (uint)(MouseOldX + x) < ScummBufferWidth )
					dst[ x ] = bak[ x ];
			}
		}
	}
}

bool OSystem_MorphOS::show_mouse(bool visible)
{
	if( MouseVisible == visible )
		return visible;

	bool last = MouseVisible;
	MouseVisible = visible;

	if( visible )
		draw_mouse();
	else
		undraw_mouse();

	return last;
}

void OSystem_MorphOS::set_mouse_pos(int x, int y)
{
	if (x != MouseX || y != MouseY)
	{
		MouseX = x;
		MouseY = y;
		undraw_mouse();
		ScreenChanged = true;
	}
}

void OSystem_MorphOS::set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y)
{
	MouseWidth = w;
	MouseHeight	= h;

	MouseHotspotX = hotspot_x;
	MouseHotspotY = hotspot_y;

	MouseImage = (byte*)buf;

	undraw_mouse();
	ScreenChanged = true;
}

bool OSystem_MorphOS::set_sound_proc( void *param, OSystem::SoundProc *proc, byte format )
{
	static EmulFunc MySoundEmulFunc;

	SoundProc = proc;
	SoundParam = param;

	/*
	 * Create Sound Thread
	 */
	MySoundEmulFunc.Trap      = TRAP_FUNC;
	MySoundEmulFunc.Address	  = (ULONG)&morphos_sound_thread;
	MySoundEmulFunc.StackSize = 8192;
	MySoundEmulFunc.Extension = 0;
	MySoundEmulFunc.Arg1	     = (ULONG)this;
	MySoundEmulFunc.Arg2	     = AHIST_M16S;

	soundProcTags[ 0 ].ti_Data = (ULONG)&MySoundEmulFunc;
	ScummSoundThread = CreateNewProc( soundProcTags );

	if( !ScummSoundThread )
	{
		puts( "Failed to create sound thread" );
		exit( 1 );
	}

	return true;
}

void OSystem_MorphOS::fill_sound( byte *stream, int len )
{
	if( SoundProc )
		SoundProc( SoundParam, stream, len );
	else
		memset( stream, 0x0, len );
}

void OSystem_MorphOS::init_size( uint w, uint h )
{
	/*
	 * Allocate image buffer
	 */
	ScummBuffer = AllocVec( w*h, MEMF_ANY | MEMF_CLEAR );

	if( ScummBuffer == NULL )
	{
		puts( "Couldn't allocate image buffer" );
		exit( 1 );
	}

	memset( ScummColors, 0, 256*sizeof( ULONG ) );

	ScummBufferWidth = w;
	ScummBufferHeight = h;
	create_screen( CSDSPTYPE_KEEP );
}

OSystem_MorphOS::SCALERTYPE OSystem_MorphOS::FindScaler( const char *ScalerName )
{
	int scaler = 0;
	
	while( ScummScalers[ scaler ].gs_Name )
	{
		if( !stricmp( ScalerName, ScummScalers[ scaler ].gs_Name ) )
			return ScummScalers[ scaler ].gs_Type;
		scaler++;
	}

	if( ScummScalers[ scaler ].gs_Name == NULL )
	{
		puts( "Invalid scaler name. Please use one of the following:" );
		for( scaler = 0; ScummScalers[ scaler ].gs_Name != NULL; scaler++ )
			printf( "  %s\n", ScummScalers[ scaler ].gs_Name );
	}

	return ST_INVALID;
}

