/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project 
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
 
#ifndef WIZ_HE_H
#define WIZ_HE_H

#include "common/rect.h"

namespace Scumm {

struct WizPolygon {
	Common::Point vert[5];
	Common::Rect bound;
	int id;
	int numVerts;
	bool flag;
};

struct WizImage {
	int resNum;
	int x1;
	int y1;
	int flags;
	int state;
	int unk;
	int paletteNum;
};

struct WizParameters {
	byte filename[260];
	Common::Rect box;
	int processFlags;
	int processMode;
	int unk_11C;
	int unk_120;
	int unk_124;
	int unk_128;
	int unk_12C;
	int unk_130;
	int unk_134;
	int unk_138;
	int compType;
	int fileWriteMode;
	int angle;
	int zoom;
	int unk_15C;
	int unk_160;
	int unk_164;
	int resDefImgW;
	int resDefImgH;
	int unk_178;
	uint8 remapColor[256];
	uint8 remapIndex[256];
	int remapNum;
	int dstResNum;
	byte fillColor;
	Common::Rect box2;
	WizImage img;
};

enum WizImageFlags {
	kWIFHasPalette = 0x1,
	kWIFRemapPalette = 0x2,
	kWIFPrint = 0x4,
	kWIFBlitToFrontVideoBuffer = 0x8,
	kWIFMarkBufferDirty = 0x10,
	kWIFBlitToMemBuffer = 0x20,
	kWIFIsPolygon = 0x40,
	kWIFFlipX = 0x400,
	kWIFFlipY = 0x800
};

enum WizProcessFlags {
	kWPFSetPos = 0x1,
	kWPFZoom = 0x8,
	kWPFRotate = 0x10,
	kWPFNewFlags = 0x20,
	kWPFRemapPalette = 0x40,
	kWPFClipBox = 0x200,
	kWPFNewState = 0x400,
	kWPFUseFile = 0x800,
	kWPFUseDefImgWidth = 0x2000,
	kWPFUseDefImgHeight = 0x4000,
	kWPFPaletteNum = 0x8000,
	kWPFDstResNum = 0x10000,
	kWPFFillColor = 0x20000,
	kWPFClipBox2 = 0x40000
};

struct Wiz {
	enum {
		NUM_POLYGONS = 200,
		NUM_IMAGES   = 255
	};

	WizImage _images[NUM_IMAGES];
	uint16 _imagesNum;
	WizPolygon _polygons[NUM_POLYGONS];
	
	Wiz();
	void imageNumClear();
	Common::Rect _rectOverride;
	bool _rectOverrideEnabled;

	void polygonClear();
	void polygonLoad(const uint8 *polData);
	void polygonStore(int id, bool flag, int vert1x, int vert1y, int vert2x, int vert2y, int vert3x, int vert3y, int vert4x, int vert4y);
	void polygonCalcBoundBox(Common::Point *vert, int numVerts, Common::Rect & bound);
	void polygonErase(int fromId, int toId);
	int polygonHit(int id, int x, int y);
	bool polygonDefined(int id);
	bool polygonContains(const WizPolygon &pol, int x, int y);
	void polygonRotatePoints(Common::Point *pts, int num, int alpha);

	static void copyAuxImage(uint8 *dst1, uint8 *dst2, const uint8 *src, int dstw, int dsth, int srcx, int srcy, int srcw, int srch);	
	static void copyWizImage(uint8 *dst, const uint8 *src, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect);
	static void copyRawWizImage(uint8 *dst, const uint8 *src, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, const uint8 *palPtr, int transColor);
	static void decompressWizImage(uint8 *dst, int dstPitch, const Common::Rect &dstRect, const uint8 *src, const Common::Rect &srcRect, const uint8 *imagePal = NULL);
	int isWizPixelNonTransparent(const uint8 *data, int x, int y, int w, int h);
	uint8 getWizPixelColor(const uint8 *data, int x, int y, int w, int h, uint8 color);
	uint8 getRawWizPixelColor(const uint8 *data, int x, int y, int w, int h, uint8 color);
	void computeWizHistogram(uint32 *histogram, const uint8 *data, const Common::Rect *srcRect);
	void computeRawWizHistogram(uint32 *histogram, const uint8 *data, int srcPitch, const Common::Rect *srcRect);
};

} // End of namespace Scumm

#endif
