/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#ifndef GOB_VIDEO_H
#define GOB_VIDEO_H

#include "common/list.h"
#include "common/rect.h"
#include "common/ptr.h"

#include "gob/gob.h"
#include "gob/surface.h"

namespace Gob {

class Font {
public:
	Font(const byte *data);
	~Font();

	uint8  getCharWidth (uint8 c) const;
	uint8  getCharWidth ()        const;
	uint8  getCharHeight()        const;

	bool hasChar(uint8 c) const;

	bool isMonospaced() const;

	void drawLetterRaw(Surface &surf, uint8 c, uint16 x, uint16 y,
					   uint32 color1, uint32 color2, bool transp) const;
	void drawLetter(Surface &surf, uint8 c, uint16 x, uint16 y,
					uint8 colorIndex1, uint8 colorIndex2, bool transp) const;

	void drawString(const Common::String &str, int16 x, int16 y, uint8 colorIndex1, uint8 colorIndex2,
					bool transp, Surface &dest) const;

private:
	const byte *_dataPtr;
	const byte  *_data;
	const uint8 *_charWidths;

	int8   _itemWidth;
	int8   _itemHeight;
	uint8  _startItem;
	uint8  _endItem;
	uint8  _itemSize;
	int8   _bitWidth;

	uint16 getCharCount() const;
	const byte *getCharData(uint8 c) const;
};

class Video {
public:
#define GDR_VERSION 4

#define PRIMARY_SURFACE   0x80
#define RETURN_PRIMARY    0x01
#define DISABLE_SPR_ALLOC 0x20
#define SCUMMVM_CURSOR    0x100

#include "common/pack-start.h"	// START STRUCT PACKING

	struct Color {
		byte red;
		byte green;
		byte blue;
	} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

	struct PalDesc {
		Color *vgaPal;
		uint32 highColorMap[256];
		int16 *unused1;
		int16 *unused2;
		PalDesc() : vgaPal(nullptr), unused1(nullptr), unused2(nullptr) {
			memset(highColorMap, 0, sizeof(highColorMap));
		}
	};

	bool _doRangeClamp;

	int16 _surfWidth;
	int16 _surfHeight;

	int16 _scrollOffsetX;
	int16 _scrollOffsetY;

	SurfacePtr _splitSurf;
	int16 _splitHeight1;
	int16 _splitHeight2;
	int16 _splitStart;

	int16 _screenDeltaX;
	int16 _screenDeltaY;

	void initPrimary(int16 mode);
	SurfacePtr initSurfDesc(int16 width, int16 height, int16 flags = 0, byte bpp = 0);

	void setSize(Graphics::PixelFormat *highColorFormat = nullptr);

	void clearScreen();
	void retrace(bool mouse = true);
	void waitRetrace(bool mouse = true);
	void sparseRetrace(int max);

	void drawPackedSprite(byte *sprBuf, int16 width, int16 height,
			int16 x, int16 y, int16 transp, Surface &dest);
	void drawPackedSprite(const char *path, Surface &dest, int width = 320);

	void setPalColor(byte *pal, byte red, byte green, byte blue) {
		pal[0] = red << 2;
		pal[1] = green << 2;
		pal[2] = blue << 2;
	}
	void setPalColor(byte *pal, Color &color) {
		setPalColor(pal, color.red, color.green, color.blue);
	}
	void setPalElem(int16 index, char red, char green, char blue,
			int16 unused, int16 vidMode);
	void setPalette(PalDesc *palDesc);
	void setFullPalette(PalDesc *palDesc);
	void setPalette(Color *palette);

	void dirtyRectsClear();
	void dirtyRectsAll();
	void dirtyRectsAdd(int16 left, int16 top, int16 right, int16 bottom);
	void dirtyRectsApply(int left, int top, int width, int height, int x, int y);

	virtual char spriteUncompressor(byte *sprBuf, int16 srcWidth,
			int16 srcHeight, int16 x, int16 y, int16 transp,
			Surface &destDesc) = 0;

	Video(class GobEngine *vm);
	virtual ~Video();

protected:
	bool _dirtyAll;
	Common::List<Common::Rect> _dirtyRects;

	int _curSparse;
	uint32 _lastSparse;

	GobEngine *_vm;

	void drawPacked(byte *sprBuf, int16 width, int16 height, int16 x, int16 y, byte transp, Surface &dest);
};

class Video_v1 : public Video {
public:
	char spriteUncompressor(byte *sprBuf, int16 srcWidth, int16 srcHeight,
			int16 x, int16 y, int16 transp, Surface &destDesc) override;

	Video_v1(GobEngine *vm);
	~Video_v1() override {}
};

class Video_v2 : public Video_v1 {
public:
	char spriteUncompressor(byte *sprBuf, int16 srcWidth, int16 srcHeight,
			int16 x, int16 y, int16 transp, Surface &destDesc) override;

	Video_v2(GobEngine *vm);
	~Video_v2() override {}
};

class Video_v6 : public Video_v2 {
public:
	char spriteUncompressor(byte *sprBuf, int16 srcWidth, int16 srcHeight,
			int16 x, int16 y, int16 transp, Surface &destDesc) override;

	Video_v6(GobEngine *vm);
	~Video_v6() override {}

private:
	void drawPacked(const byte *sprBuf, int16 x, int16 y, Surface &surfDesc);
	void drawYUVData(const byte *srcData, Surface &destDesc,
			int16 width, int16 height, int16 x, int16 y);
	void drawYUV(Surface &destDesc, int16 x, int16 y,
			int16 dataWidth, int16 dataHeight, int16 width, int16 height,
			const byte *dataY, const byte *dataU, const byte *dataV);
};

} // End of namespace Gob

#endif // GOB_VIDEO_H
