/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project 
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

#ifndef GFX_H
#define GFX_H


enum VideoMode {		/* Video scalers */
	VIDEO_SCALE = 0,
	VIDEO_2XSAI = 1,
	VIDEO_SUPERSAI = 2,
	VIDEO_SUPEREAGLE = 3
};

enum {					/* Camera modes */
	CM_NORMAL = 1,
	CM_FOLLOW_ACTOR = 2,
	CM_PANNING = 3
};

struct CameraData {		/* Camera state data */
	ScummPoint _cur;
	ScummPoint _dest;
	ScummPoint _accel;
	ScummPoint _last;
	int _leftTrigger, _rightTrigger;
	byte _follows, _mode;
	bool _movingToActor;
};


struct VirtScreen {		/* Virtual screen areas */
	int number;
	uint16 unk1;
	uint16 topline;
	uint16 width,height;
	uint16 size;
	byte alloctwobuffers;
	byte scrollable;
	uint16 xstart;
	uint16 tdirty[40];
	uint16 bdirty[40];
	byte *screenPtr;
	byte *backBuf;
};

struct MouseCursor {	/* Mouse cursor */
	int8 hotspot_x, hotspot_y;
	byte colors[4];
	byte data[32];
};

struct ColorCycle {		/* Palette cycles */
	uint16 delay;
	uint16 counter;
	uint16 flags;
	byte start;
	byte end;
};

struct BlastObject {		/* BlastObjects to draw */
	uint16 number,areaX,areaY,areaWidth,areaHeight;
	int16 posX,posY;
	uint16 width,height;
	uint16 unk3,unk4,image;

	uint16 mode;
};

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct BompHeader {			/* Bomp header */
	uint16 unk;
	uint16 width,height;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

struct BompDrawData {		/* Bomp graphics data */
	byte *out;
	int outwidth, outheight;
	int x,y;
	byte scale_x, scale_y;
	byte *dataptr;
	int srcwidth, srcheight;
};

struct Gdi {
	Scumm *_vm;

	byte *_readPtr;
	uint _readOffs;

	int _numZBuffer;
	int _imgBufOffs[4];
	byte _disable_zbuffer;

	bool _useOrDecompress;
	int _numLinesToProcess;
	int _tempNumLines;
	byte _currentX;
	byte _hotspot_x;
	byte _hotspot_y;
	byte _cursorActive;

	int16 _drawMouseX;
	int16 _drawMouseY;
	int16 _mask_top, _mask_bottom, _mask_right, _mask_left;
	byte _currentCursor;
	byte _mouseColors[4];
	byte _mouseColor;
	byte _mouseClipMask1, _mouseClipMask2, _mouseClipMask3;
	byte _mouseColorIndex;
	byte *_mouseMaskPtr;
	byte *_smap_ptr;
	byte *_backbuff_ptr;
	byte *_bgbak_ptr;
	byte *_mask_ptr;
	byte *_mask_ptr_dest;
	byte *_z_plane_ptr;

	byte _palette_mod;
	byte _decomp_shr, _decomp_mask;
	byte _transparency;
	uint32 _vertStripNextInc;
	byte *_backupIsWhere;

	/* Bitmap decompressors */
	void decompressBitmap();
	void unkDecode1();
	void unkDecode2();
	void unkDecode3();
	void unkDecode4();
	void unkDecode5();
	void unkDecode6();
	void unkDecode7();
	void unkDecode8();
	void unkDecode9();
	void unkDecode10();
	void unkDecode11();

	void drawBitmap(byte *ptr, VirtScreen *vs, int x, int y, int h, int stripnr, int numstrip, byte flag);
	void clearUpperMask();

	void disableZBuffer() { _disable_zbuffer++; }
	void enableZBuffer() { _disable_zbuffer--; }

	void draw8ColWithMasking();
	void clear8ColWithMasking();
	void clear8Col();
	void decompressMaskImgOr();
	void decompressMaskImg();

	void resetBackground(int top, int bottom, int strip);
	void drawStripToScreen(VirtScreen *vs, int x, int w, int t, int b);
	void updateDirtyScreen(VirtScreen *vs);

	enum DrawBitmapFlags {
		dbAllowMaskOr = 1,
		dbDrawMaskOnBoth = 2,
		dbClear = 4
	};
};

void blit(byte *dst, byte *src, int w, int h);

#endif
