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

#include <exec/semaphores.h>
#include <libraries/cdda.h>

class OSystem_MorphOS : public OSystem
{
	public:
		typedef enum { ST_INVALID = 0, ST_NONE, ST_POINT, ST_ADVMAME2X, ST_SUPEREAGLE, ST_SUPER2XSAI } SCALERTYPE;

					OSystem_MorphOS( int game_id, SCALERTYPE gfx_mode, bool full_screen );
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

		static OSystem_MorphOS *create  ( int game_id, SCALERTYPE gfx_scaler, bool full_screen );
		static uint32 make_color( int pixfmt, int r, int g, int b );

		static SCALERTYPE  FindScaler		  ( const char *ScalerName );

	private:
		typedef void (*ScalerFunc)( uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height );
		typedef enum { CSDSPTYPE_WINDOWED, CSDSPTYPE_FULLSCREEN, CSDSPTYPE_TOGGLE, CSDSPTYPE_KEEP } CS_DSPTYPE;

		struct GfxScaler
		{
			STRPTR 		gs_Name;
			SCALERTYPE	gs_Type;
		};

		static const int MAX_MOUSE_W = 40;
		static const int MAX_MOUSE_H = 40;

		void   		  create_screen   ( CS_DSPTYPE dspType );
		void 	 		  SwitchScalerTo  ( SCALERTYPE newScaler );
		void   		  Super2xSaI		( uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height );
		void   		  SuperEagle		( uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height );
		void 			  AdvMame2xScaler ( uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height );
		void   		  PointScaler		( uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height );

		void   draw_mouse();
		void   undraw_mouse();

		/* Display-related attributes */
		struct Screen  	  *ScummScreen;
		struct Window  	  *ScummWindow;
		char 						ScummWndTitle[ 125 ];
		APTR            		ScummBuffer;
		int						ScummBufferWidth;
		int						ScummBufferHeight;
		struct ScreenBuffer *ScummScreenBuffer[ 2 ];
		struct BitMap  	  *ScummRenderTo;
		bool			    		ScummPCMode;
		ULONG			   		ScummPaintBuffer;
		int						ScummScrWidth;
		int 						ScummScrHeight;
		int						ScummDepth;
		bool 						Scumm16ColFmt16;
		UWORD 		   	  *ScummNoCursor;
		ULONG 		    		ScummColors[256];
		USHORT 		    		ScummColors16[256];
		WORD			    		ScummWinX;
		WORD			    		ScummWinY;
		bool			    		ScummDefaultMouse;
		bool			    		ScummOrigMouse;
		int 			    		ScummShakePos;
		bool						FullScreenMode;
		bool 						ScreenChanged;

		/* Scaling-related attributes */
		uint32 					colorMask;
		uint32 					lowPixelMask;
		uint32 					qcolorMask;
		uint32 					qlowpixelMask;
		uint32 					redblueMask;
		uint32 					greenMask;
		int 						PixelsPerMask;
		byte 					  *src_line[4];
		byte 					  *dst_line[2];

		/* Sound-related attributes */
		struct Process *ScummMusicThread;
		struct Process *ScummSoundThread;
		SoundProc 		*SoundProc;
		void      		*SoundParam;

		/* CD-ROM related attributes */
		CDRIVEPTR 		 CDrive;
		ULONG 			 CDDATrackOffset;
		int 				 cd_track, cd_num_loops, cd_start_frame, cd_end_frame;
		uint32 			 cd_end_time, cd_stop_time, cd_next_second;

		/* Scaling-related attributes */
		SCALERTYPE ScummScaler;
		int  		  ScummScale;
		static GfxScaler ScummScalers[ 10 ];

		/* Mouse cursor-related attributes */
		bool  MouseVisible, MouseDrawn;
		int   MouseX, MouseY;
		int   MouseWidth, MouseHeight;
		int   MouseOldX, MouseOldY;
		int   MouseOldWidth, MouseOldHeight;
		int   MouseHotspotX, MouseHotspotY;
		byte *MouseImage, MouseBackup[ MAX_MOUSE_W*MAX_MOUSE_H ];

		/* Timer-related attributes */
		struct MsgPort 	 *TimerMsgPort;
		struct timerequest *TimerIORequest;

		/* Game-related attributes */
		int   GameID;
};

int morphos_sound_thread( OSystem_MorphOS *syst, ULONG SampleType );
bool init_morphos_music( ULONG MidiUnit );
void exit_morphos_music();

int morphos_main( int argc, char *argv[] );

extern OSystem_MorphOS *TheSystem;
extern struct SignalSemaphore ScummMusicThreadRunning;
extern struct SignalSemaphore ScummSoundThreadRunning;

extern LONG ScummMidiUnit;
extern struct IOMidiRequest *ScummMidiRequest;
extern struct timerequest   *MusicTimerIORequest;

