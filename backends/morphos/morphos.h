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
 * MorphOS-specific header file
 *
 * $Header$
 *
 */

#ifndef MORPHOS_MORPHOS_H
#define MORPHOS_MORPHOS_H

#include <graphics/regions.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <libraries/cdda.h>

#include "morphos_scaler.h"

class OSystem_MorphOS : public OSystem
{
	public:
					OSystem_MorphOS(int game_id, SCALERTYPE gfx_mode, bool full_screen);
		virtual ~OSystem_MorphOS();

		// Set colors of the palette
		virtual void set_palette(const byte *colors, uint start, uint num);

		// Set the size of the video bitmap.
		// Typically, 320x200
		virtual void init_size(uint w, uint h);

		// Draw a bitmap to screen.
		// The screen will not be updated to reflect the new bitmap
		virtual void copy_rect(const byte *buf, int pitch, int x, int y, int w, int h);

		// Update the dirty areas of the screen
		virtual void update_screen();

		// Either show or hide the mouse cursor
		virtual bool show_mouse(bool visible);

		// Set the position of the mouse cursor
		virtual void set_mouse_pos(int x, int y);

		// Set the bitmap that's used when drawing the cursor.
		virtual void set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y);

		// Shaking is used in SCUMM. Set current shake position.
		virtual void set_shake_pos(int shake_pos);

		// Get the number of milliseconds since the program was started.
		virtual uint32 get_msecs();

		// Delay for a specified amount of milliseconds
		virtual void delay_msecs(uint msecs);

		// Add a new callback timer
		virtual void set_timer(int timer, int (*callback)(int));

		// Mutex handling
		virtual void *create_mutex(void);
		virtual void lock_mutex(void *mutex);
		virtual void unlock_mutex(void *mutex);
		virtual void delete_mutex(void *mutex);

		// Create a thread
		virtual void *create_thread(ThreadProc *proc, void *param);

		// Get the next event.
		// Returns true if an event was retrieved.
		virtual bool poll_event(Event *event);

		// Set the function to be invoked whenever samples need to be generated
		virtual bool set_sound_proc(void *param, SoundProc *proc, byte format);
				  void fill_sound    (byte * stream, int len);

		virtual uint32 property(int param, Property *value);

		// Poll cdrom status
		// Returns true if cd audio is playing
		virtual bool poll_cdrom();

		// Play cdrom audio track
		virtual void play_cdrom(int track, int num_loops, int start_frame, int length);

		// Stop cdrom audio track
		virtual void stop_cdrom();

		// Update cdrom audio status
		virtual void update_cdrom();

		// Quit
		virtual void quit();

		static OSystem_MorphOS *create(int game_id, SCALERTYPE gfx_scaler, bool full_screen);

		static bool OpenATimer(MsgPort **port, IORequest **req, ULONG unit, bool required = true);

	private:
		typedef enum { CSDSPTYPE_WINDOWED, CSDSPTYPE_FULLSCREEN, CSDSPTYPE_TOGGLE, CSDSPTYPE_KEEP } CS_DSPTYPE;

		static const int MAX_MOUSE_W = 40;
		static const int MAX_MOUSE_H = 40;

		void   CreateScreen(CS_DSPTYPE dspType);
		void 	 SwitchScalerTo(SCALERTYPE newScaler);
		bool   AddUpdateRect(WORD x, WORD y, WORD w, WORD h);

		void   DrawMouse();
		void   UndrawMouse();

		/* Display-related attributes */
		Screen  	    *ScummScreen;
		Window  	    *ScummWindow;
		char 			  ScummWndTitle[125];
		APTR          ScummBuffer;
		int			  ScummBufferWidth;
		int			  ScummBufferHeight;
		ScreenBuffer *ScummScreenBuffer[2];
		BitMap  	    *ScummRenderTo;
		ULONG			  ScummPaintBuffer;
		int			  ScummScrWidth;
		int 			  ScummScrHeight;
		int			  ScummDepth;
		bool 			  Scumm16ColFmt16;
		UWORD 		 *ScummNoCursor;
		ULONG 		  ScummColors[256];
		USHORT 		  ScummColors16[256];
		WORD			  ScummWinX;
		WORD			  ScummWinY;
		bool			  ScummDefaultMouse;
		bool			  ScummOrigMouse;
		int 			  ScummShakePos;
		bool			  FullScreenMode;
		bool 			  ScreenChanged;
		UWORD			**BlockColors;
		bool			 *DirtyBlocks;
		Region 		 *UpdateRegion;
		Region 		 *NewUpdateRegion;

		/* Sound-related attributes */
		Process   *ScummMusicThread;
		Process 	 *ScummSoundThread;
		SoundProc *SoundProc;
		void      *SoundParam;

		/* CD-ROM related attributes */
		CDRIVEPTR CDrive;
		ULONG 	 CDDATrackOffset;

		/* Scaling-related attributes */
		SCALERTYPE ScummScaler;
		int  		  ScummScale;
		MorphOSScaler *Scaler;

		/* Mouse cursor-related attributes */
		bool  MouseVisible, MouseDrawn;
		int   MouseX, MouseY;
		int   MouseWidth, MouseHeight;
		int   MouseOldX, MouseOldY;
		int   MouseOldWidth, MouseOldHeight;
		int   MouseHotspotX, MouseHotspotY;
		byte *MouseImage, MouseBackup[MAX_MOUSE_W*MAX_MOUSE_H];

		/* Timer-related attributes */
		MsgPort 	   *TimerMsgPort;
		timerequest *TimerIORequest;

		/* Game-related attributes */
		int   GameID;
};

int morphos_main(int argc, char *argv[]);

extern OSystem_MorphOS *TheSystem;

#endif

