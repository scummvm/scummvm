/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
 * Copyright (C) 2002-2005 Chris Apers - PalmOS Backend
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#ifndef BE_OS5_H
#define BE_OS5_H

#include "be_base.h"

#if !defined(SYSTEM_CALLBACK) || defined(PALMOS_68K)
#	define SYSTEM_CALLBACK
#	ifdef PALMOS_ARM
#		define CALLBACK_PROLOGUE \
			__asm { \
				stmfd  r13!,{r9,r10}; \
				ldr	   r9,[r0]; \
				ldr	   r10,[r0,#4]; \
			}
#		define CALLBACK_EPILOGUE __asm { ldmfd  r13!,{r9,r10} }
#		define CALLBACK_INIT(regs) \
			__asm { \
				ldr  r0, = regs; \
				add	 r0,r0,r10; \
				str	 r9,[r0]; \
				str	 r10,[r0,#4]; \
			}
#	else
#		define CALLBACK_PROLOGUE \
			asm (	\
				movem.l a4-a5, -(sp);	\
				move.l UserDataP, a0;	\
				move.l 0(a0), a4;	\
				move.l 4(a0), a5;	\
			);
#		define CALLBACK_EPILOGUE asm ( movem.l (sp)+, a4-a5 );
#		define CALLBACK_INIT(regs) \
			{	\
				void *ptr = &regs; \
				asm (	\
					move.l	ptr, a0;	\
					move.l	a4, 0(a0);	\
					move.l	a5, 4(a0);	\
				); \
			}
#	endif
#else
#	define CALLBACK_PROLOGUE
#	define CALLBACK_EPILOGUE
#	define CALLBACK_INIT(regs)
#endif

typedef struct {
	UInt32 __reg1;
	UInt32 __reg2;

	void *proc;
	void *param;

	SndStreamRef handle;
	Boolean	active;
} SoundDataType;
extern SoundDataType _sound;

typedef struct {
	UInt32 __r9;
	UInt32 __r10;
	TimerPtr timer;
	UInt32 timerID;
	UInt32 ticks;
} TimerExType, *TimerExPtr;

class OSystem_PalmOS5 : public OSystem_PalmBase {
private:
	byte *_overlayP;
	WinHandle _overlayH;

	virtual void int_initBackend();
	virtual void int_updateScreen();
	virtual void int_initSize(uint w, uint h, int overlayScale);

	virtual void unload_gfx_mode();
	virtual void load_gfx_mode();

	void draw_mouse();
	void undraw_mouse();
	virtual void get_coordinates(EventPtr ev, Coord &x, Coord &y);
	virtual bool check_event(Event &event, EventPtr ev);

#ifdef PALMOS_ARM
	void timer_handler() {};
#endif

protected:
	UInt16 _sysOldCoord, _sysOldOrientation;

public:
	OSystem_PalmOS5();
	static OSystem *create();

	void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);
	void clearScreen();

	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int cursorTargetScale);

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(OverlayColor *buf, int pitch);
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	virtual OverlayColor RGBToColor(uint8 r, uint8 g, uint8 b);
	virtual void colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b);

#ifdef PALMOS_ARM
	void setTimerCallback(TimerProc callback, int interval);
#endif

	bool setSoundCallback(SoundProc proc, void *param);
	void clearSoundCallback();
	
	void int_quit();
	void setWindowCaption(const char *caption) {};

};

#endif
