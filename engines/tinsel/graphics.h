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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * Low level graphics interface.
 */

#ifndef TINSEL_GRAPHICS_H		// prevent multiple includes
#define TINSEL_GRAPHICS_H

#include "tinsel/dw.h"

#include "common/rect.h"
#include "common/system.h"
#include "graphics/surface.h"

namespace Tinsel {

struct PALQ;


#define	SCREEN_WIDTH	320			// PC screen dimensions
#define	SCREEN_HEIGHT	200
#define	SCRN_CENTRE_X	((SCREEN_WIDTH  - 1) / 2)	// screen centre x
#define	SCRN_CENTRE_Y	((SCREEN_HEIGHT - 1) / 2)	// screen centre y

/** Class representing either a buffered surface or the physical screen. */
class Surface : public Graphics::Surface {
private:
	bool _isScreen;
public:
	Surface(bool isScreen = false) { _isScreen = isScreen; }
	Surface(int Width, int Height) { create(Width, Height, 1); _isScreen = false; }

	// Surface methods
	byte *getData() { return (byte *)pixels; }
	byte *getBasePtr(int x, int y) { return (byte *)Graphics::Surface::getBasePtr(x, y); }

	void update() { 
		if (_isScreen) {
			g_system->copyRectToScreen((const byte *)pixels, pitch, 0, 0, w, h);
			g_system->updateScreen(); 
		}
	}
	void updateRect(const Common::Rect &r) {
		g_system->copyRectToScreen(getBasePtr(r.left, r.top), pitch, r.left, r.top, r.width(), r.height());
		g_system->updateScreen(); 
	}

};

/** draw object structure - only used when drawing objects */
struct DRAWOBJECT {
	char *charBase;		// character set base address
	int transOffset;	// transparent character offset
	int flags;		// object flags - see above for list
	PALQ *pPal;		// objects palette Q position
	int constant;		// which colour in palette for monochrome objects
	int width;		// width of object
	int height;		// height of object
	SCNHANDLE hBits;	// image bitmap handle
	int lineoffset;		// offset to next line
	int leftClip;		// amount to clip off object left
	int rightClip;		// amount to clip off object right
	int topClip;		// amount to clip off object top
	int botClip;		// amount to clip off object bottom
	short xPos;		// x position of object
	short yPos;		// y position of object
};


/*----------------------------------------------------------------------*\
|*			    Function Prototypes				*|
\*----------------------------------------------------------------------*/

void ClearScreen(uint32 val);
void DrawObject(DRAWOBJECT *pObj);

// called to update a rectangle on the video screen from a video page
void UpdateScreenRect(const Common::Rect &pClip);

} // end of namespace Tinsel

#endif
