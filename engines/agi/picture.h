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

#ifndef AGI_PICTURE_H
#define AGI_PICTURE_H

#include "agi/agi.h"

namespace Agi {

#define _DEFAULT_WIDTH		160
#define _DEFAULT_HEIGHT		168

/**
 * AGI picture resource.
 */
struct AgiPicture {
	uint32 flen;			/**< size of raw data */
	uint8 *rdata;			/**< raw vector image data */
};

// AGI picture version
enum AgiPictureVersion {
	AGIPIC_C64,
	AGIPIC_V1,
	AGIPIC_V15,
	AGIPIC_V2
};

class AgiBase;
class GfxMgr;

class PictureMgr {
	AgiBase *_vm;
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
	void plotBrush();
	void drawPicture();

	// TODO: this is hardcoded for V2 pictures for now
	static const int pictureType = AGIPIC_V2;
	int width, height;

public:
	PictureMgr(AgiBase *agi, GfxMgr *gfx) {
		_vm = agi;
		_gfx = gfx;
	}

	int decodePicture(int n, int clear, bool agi256 = false, int pic_width = _DEFAULT_WIDTH, int pic_height = _DEFAULT_HEIGHT);
	int unloadPicture(int);
	void showPic(int x = 0, int y = 0, int pic_width = _DEFAULT_WIDTH, int pic_height = _DEFAULT_HEIGHT);
	uint8 *convertV3Pic(uint8 *src, uint32 len);

	void plotPattern(int x, int y);		// public because it's used directly by preagi

	// preagi needed functions (for plotPattern)
	void setPattern(uint8 code, uint8 num);
	void setColor(uint8 color);
};

} // End of namespace Agi

#endif /* AGI_PICTURE_H */
