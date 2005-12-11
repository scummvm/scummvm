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

#ifndef BE_ZODIAC_H
#define BE_ZODIAC_H

#include <tapwave.h>
#include "be_os5.h"

#define MIN_OFFSET	20

class OSystem_PalmZodiac : public OSystem_PalmOS5 {
private:
	enum {
		kRatioNone = 0,
		kRatioHeight,
		kRatioWidth
	};
	struct {
		UInt8 adjustAspect;
		Coord width;	// (width x 320)
		Coord height;	// (480 x height)
	} _ratio;

	TwGfxType *_gfxH;
	TwGfxSurfaceType *_palmScreenP, *_tmpScreenP;
	TwGfxSurfaceType *_overlayP;
	UInt16 _nativePal[256];
	Boolean _fullscreen;
	
	TwGfxPointType _srcPos;
	TwGfxRectType _srcRect, _dstRect;
	TwGfxBitmapType _srcBmp;
	Boolean _stretched;

	void int_initBackend();
	void int_setShakePos(int shakeOffset);

	void draw_mouse();
	void undraw_mouse();

	void unload_gfx_mode();
	void load_gfx_mode();
	void hotswap_gfx_mode(int mode);

	void extras_palette(uint8 index, uint8 r, uint8 g, uint8 b);
	void calc_rect(Boolean fullscreen);
	void get_coordinates(EventPtr ev, Coord &x, Coord &y);
	bool check_event(Event &event, EventPtr ev);
	void draw_osd(UInt16 id, Int32 x, Int32 y, Boolean show, UInt8 color = 0);

public:
	OSystem_PalmZodiac();

	static OSystem *create();
	
	void setFeatureState(Feature f, bool enable);
	int getDefaultGraphicsMode() const;

	void updateScreen();
	bool grabRawScreen(Graphics::Surface *surf);
	
	void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int cursorTargetScale);

	void showOverlay();
	void hideOverlay();
	void clearOverlay();
	void grabOverlay(OverlayColor *buf, int pitch);
	void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	OverlayColor RGBToColor(uint8 r, uint8 g, uint8 b);
	void colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b);
};

#endif
