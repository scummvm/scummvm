/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * $URL$
 * $Id$
 *
 */

#ifndef BE_ZODIAC_H
#define BE_ZODIAC_H

#include <tapwave.h>
#include "be_os5ex.h"

#define MIN_OFFSET	20

class OSystem_PalmZodiac : public OSystem_PalmOS5Ex {
private:

	TwGfxType *_gfxH;
	TwGfxSurfaceType *_palmScreenP, *_tmpScreenP;
	TwGfxSurfaceType *_overlayP;
	Boolean _fullscreen;

	TwGfxPointType _srcPos;
	TwGfxRectType _srcRect, _dstRect;
	TwGfxBitmapType _srcBmp;

	void int_initBackend();
	void int_setShakePos(int shakeOffset);

	void draw_mouse();
	void undraw_mouse();

	void unload_gfx_mode();
	void load_gfx_mode();
	void hotswap_gfx_mode(int mode);

	void calc_rect(Boolean fullscreen);
	bool check_event(Common::Event &event, EventPtr ev);
	void draw_osd(UInt16 id, Int32 x, Int32 y, Boolean show, UInt8 color = 0);

public:
	OSystem_PalmZodiac();

	static OSystem *create();

	void setFeatureState(Feature f, bool enable);

	int getDefaultGraphicsMode() const;

	void updateScreen();

	void clearOverlay();
	void grabOverlay(OverlayColor *buf, int pitch);
	void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
};

#endif
