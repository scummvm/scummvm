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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef BE_OS5_H
#define BE_OS5_H

#include "be_base.h"

typedef struct {
	UInt32 __a4;
	UInt32 __a5;

	void *proc;
	void *param;

	SndStreamRef handle;
	Boolean	active;
} SoundDataType;

class OSystem_PalmOS5 : public OSystem_PalmBase {
private:
	virtual void int_initBackend();
	virtual void int_updateScreen();
	virtual void int_initSize(uint w, uint h, int overlayScale);

	virtual void unload_gfx_mode();
	virtual void load_gfx_mode();

	void draw_mouse();
	void undraw_mouse();
	virtual void get_coordinates(EventPtr ev, Coord &x, Coord &y);
	virtual bool check_event(Event &event, EventPtr ev) { return false;}

protected:
	UInt16 _sysOldCoord, _sysOldOrientation;
	SoundDataType _sound;

public:
	static OSystem *create();

	void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);
	void clearScreen();

	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int cursorTargetScale);

	virtual void showOverlay() {};
	virtual void hideOverlay() {};
	virtual void clearOverlay() {};
	virtual void grabOverlay(OverlayColor *buf, int pitch) {};
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {};
	virtual OverlayColor RGBToColor(uint8 r, uint8 g, uint8 b);
	virtual void colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b);

	bool setSoundCallback(SoundProc proc, void *param);
	void clearSoundCallback();
	
	void int_quit();
	void setWindowCaption(const char *caption) {};

};

#endif
