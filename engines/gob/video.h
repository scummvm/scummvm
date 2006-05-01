/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */
#ifndef GOB_VIDEO_H
#define GOB_VIDEO_H

#include "common/stdafx.h"
#include "common/util.h"
#include "gob/gob.h"

namespace Gob {

#define VID_SET_CURSOR(val) { _AH = 1; _CX = (val); geninterrupt(0x10); }
#define VID_RESTORE_MODE { _AX = 3; geninterrupt(0x10); }

#define TEXT_VID_SEG 	0xB800
#define TEXT_VID_OFF 	0
#define TEXT_COL_COUNT	80
#define TEXT_ROW_COUNT	25

extern int16 setAllPalette;

class Video {
public:
	struct SurfaceDesc {
		int16 width;
		int16 height;
		int8 reserved1;
		int8 flag;
		int16 vidMode;
		byte *vidPtr;
		int16 reserved2;
		SurfaceDesc() : width(0), height(0), reserved1(0), flag(0),
						  vidMode(0), vidPtr(0), reserved2(0) {}
	};

	struct FontDesc {
		char *dataPtr;
		int8 itemWidth;
		int8 itemHeight;
		int8 startItem;
		int8 endItem;
		int8 itemSize;
		int8 bitWidth;
		void *extraData;
		FontDesc() : dataPtr(0), itemWidth(0), itemHeight(0), startItem(0),
			               endItem(0), itemSize(0), bitWidth(0) {}
	};

#define GDR_VERSION	4

#define PRIMARY_SURFACE		0x80
#define RETURN_PRIMARY		0x01
#define DISABLE_SPR_ALLOC	0x20

#pragma START_PACK_STRUCTS

	struct Color {
		byte red;
		byte green;
		byte blue;
	} GCC_PACK;

#pragma END_PACK_STRUCTS

	struct PalDesc {
		Color *vgaPal;
		int16 *unused1;
		int16 *unused2;
		PalDesc() : vgaPal(0), unused1(0), unused2(0) {}
	};

	bool _extraMode;

	Video(class GobEngine *vm);
	virtual ~Video() {};
	int32 getRectSize(int16 width, int16 height, int16 flag, int16 mode);
	void freeSurfDesc(SurfaceDesc * surfDesc);
	int16 clampValue(int16 val, int16 max);
	void drawSprite(SurfaceDesc * source, SurfaceDesc * dest, int16 left,
					int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp);
	void fillRect(SurfaceDesc * dest, int16 left, int16 top, int16 right, int16 bottom,
				  int16 color);
	void drawLine(SurfaceDesc * dest, int16 x0, int16 y0, int16 x1, int16 y1,
				  int16 color);
	void putPixel(int16 x, int16 y, int16 color, SurfaceDesc * dest);
	void drawCircle(Video::SurfaceDesc *dest, int16 x, int16 y, int16 radius, int16 color);
	void clearSurf(SurfaceDesc * dest);
	void drawPackedSprite(byte *sprBuf, int16 width, int16 height, int16 x, int16 y,
						  int16 transp, SurfaceDesc * dest);
	void setPalElem(int16 index, char red, char green, char blue, int16 unused,
					int16 vidMode);
	void setPalette(PalDesc * palDesc);
	void setFullPalette(PalDesc * palDesc);
	void initPrimary(int16 mode);
	char spriteUncompressor(byte *sprBuf, int16 srcWidth, int16 srcHeight, int16 x,
							int16 y, int16 transp, SurfaceDesc * destDesc);
	void freeDriver(void);
	void setHandlers();

	virtual void drawLetter(int16 item, int16 x, int16 y, FontDesc * fontDesc,
			int16 color1, int16 color2, int16 transp, SurfaceDesc * dest) = 0;
	virtual SurfaceDesc *initSurfDesc(int16 vidMode, int16 width, int16 height, int16 flags) = 0;
	virtual void waitRetrace(int16) = 0;

protected:
	class VideoDriver *_videoDriver;
	GobEngine *_vm;

	char initDriver(int16 vidMode);
};

class Video_v1 : public Video {
public:
	virtual void drawLetter(int16 item, int16 x, int16 y, FontDesc * fontDesc,
			int16 color1, int16 color2, int16 transp, SurfaceDesc * dest);
	virtual SurfaceDesc *initSurfDesc(int16 vidMode, int16 width, int16 height, int16 flags);
	virtual void waitRetrace(int16);

	Video_v1(GobEngine *vm);
	virtual ~Video_v1() {};
};

class Video_v2 : public Video_v1 {
public:
	virtual void drawLetter(int16 item, int16 x, int16 y, FontDesc * fontDesc,
			int16 color1, int16 color2, int16 transp, SurfaceDesc * dest);
	virtual SurfaceDesc *initSurfDesc(int16 vidMode, int16 width, int16 height, int16 flags);
	virtual void waitRetrace(int16);

	Video_v2(GobEngine *vm);
	virtual ~Video_v2() {};
};

class VideoDriver {
public:
	VideoDriver() {}
	virtual ~VideoDriver() {}
	virtual void drawSprite(Video::SurfaceDesc *source, Video::SurfaceDesc *dest, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp) = 0;
	virtual void fillRect(Video::SurfaceDesc *dest, int16 left, int16 top, int16 right, int16 bottom, byte color) = 0;
	virtual void putPixel(int16 x, int16 y, byte color, Video::SurfaceDesc *dest) = 0;
	virtual void drawLetter(unsigned char item, int16 x, int16 y, Video::FontDesc *fontDesc, byte color1, byte color2, byte transp, Video::SurfaceDesc *dest) = 0;
	virtual void drawLine(Video::SurfaceDesc *dest, int16 x0, int16 y0, int16 x1, int16 y1, byte color) = 0;
	virtual void drawPackedSprite(byte *sprBuf, int16 width, int16 height, int16 x, int16 y, byte transp, Video::SurfaceDesc *dest) = 0;
};

}				// End of namespace Gob

#endif
