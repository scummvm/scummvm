/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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

#ifndef AGI_PICTURE_H
#define AGI_PICTURE_H

#include "agi/agi.h"

namespace Agi {

/**
 * AGI picture resource.
 */
struct AgiPicture {
	uint32 flen;			/**< size of raw data */
	uint8 *rdata;			/**< raw vector image data */
};

class AgiEngine;
class GfxMgr;

class PictureMgr {
	AgiEngine *_vm;
	GfxMgr *_gfx;

private:

	void drawLine(int x1, int y1, int x2, int y2);
	void putVirtPixel(int x, int y);
	void dynamicDrawLine();
	void absoluteDrawLine();
	INLINE int isOkFillHere(int x, int y);
	void fillScanline(int x, int y);
	void agiFill(unsigned int x, unsigned int y);
	void xCorner();
	void yCorner();
	void fill();
	int plotPatternPoint(int x, int y, int bitpos);
	void plotPattern(int x, int y);
	void plotBrush();
	void drawPicture();

public:
	PictureMgr(AgiEngine *agi, GfxMgr *gfx) {
		_vm = agi;
		_gfx = gfx;
	}

	int decodePicture(int, int);
	int unloadPicture(int);
	void showPic();
	uint8 *convertV3Pic(uint8 *src, uint32 len);
};

} // End of namespace Agi

#endif				/* AGI_PICTURE_H */
