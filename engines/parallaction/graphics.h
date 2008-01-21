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

#ifndef PARALLACTION_GRAPHICS_H
#define PARALLACTION_GRAPHICS_H

#include "common/rect.h"
#include "common/stream.h"

#include "graphics/surface.h"

#include "parallaction/defs.h"


namespace Parallaction {


#include "common/pack-start.h"	// START STRUCT PACKING

struct PaletteFxRange {

	uint16	_timer;
	uint16	_step;
	uint16	_flags;
	byte	_first;
	byte	_last;

} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

class Font {

protected:
	byte _color;


public:
	Font() {}
	virtual ~Font() {}

	virtual void setColor(byte color) {
		_color = color;
	}
	virtual uint32 getStringWidth(const char *s) = 0;
	virtual uint16 height() = 0;

	virtual void drawString(byte* buffer, uint32 pitch, const char *s) = 0;


};


struct Frames {

	virtual uint16	getNum() = 0;
	virtual byte*	getData(uint16 index) = 0;
	virtual void	getRect(uint16 index, Common::Rect &r) = 0;

	virtual ~Frames() { }

};


struct SurfaceToFrames : public Frames {

	Graphics::Surface	*_surf;

public:
	SurfaceToFrames(Graphics::Surface *surf) : _surf(surf) {
	}

	~SurfaceToFrames() {
		delete _surf;
	}

	uint16	getNum() {
		return 1;
	}
	byte*	getData(uint16 index) {
		assert(index == 0);
		return (byte*)_surf->getBasePtr(0,0);
	}
	void	getRect(uint16 index, Common::Rect &r) {
		assert(index == 0);
		r.left = 0;
		r.top = 0;
		r.setWidth(_surf->w);
		r.setHeight(_surf->h);
	}


};

struct MaskBuffer {
	// handles a 2-bit depth buffer used for z-buffering

	uint16	w;
	uint16  internalWidth;
	uint16	h;
	uint	size;
	byte	*data;

public:
	MaskBuffer() : w(0), internalWidth(0), h(0), size(0), data(0) {
	}

	void create(uint16 width, uint16 height) {
		w = width;
		internalWidth = w >> 2;
		h = height;
		size = (internalWidth * h);
		data = (byte*)calloc(size, 1);
	}

	void free() {
		::free(data);
		data = 0;
		w = 0;
		h = 0;
		internalWidth = 0;
		size = 0;
	}

	inline byte getValue(uint16 x, uint16 y) {
		byte m = data[(x >> 2) + y * internalWidth];
		uint n = (x & 3) << 1;
		return ((3 << n) & m) >> n;
	}

};

class Palette {

	byte	_data[768];
	uint	_colors;
	uint	_size;
	bool	_hb;

public:
	Palette();
	Palette(const Palette &pal);

	void clone(const Palette &pal);

	void makeBlack();
	void setEntries(byte* data, uint first, uint num);
	void setEntry(uint index, int red, int green, int blue);
	void makeGrayscale();
	void fadeTo(const Palette& target, uint step);
	uint fillRGBA(byte *rgba);

	void rotate(uint first, uint last, bool forward);
};


struct Cnv : public Frames {
	uint16	_count; 	// # of frames
	uint16	_width; 	//
	uint16	_height;	//
	byte**	field_8;	// unused
	byte*	_data;

public:
	Cnv() {
		_width = _height = _count = 0;
		_data = NULL;
	}

	Cnv(uint16 numFrames, uint16 width, uint16 height, byte* data) : _count(numFrames), _width(width), _height(height), _data(data) {

	}

	~Cnv() {
		free(_data);
	}

	byte* getFramePtr(uint16 index) {
		if (index >= _count)
			return NULL;
		return &_data[index * _width * _height];
	}

	uint16	getNum() {
		return _count;
	}

	byte	*getData(uint16 index) {
		return getFramePtr(index);
	}

	void getRect(uint16 index, Common::Rect &r) {
		r.left = 0;
		r.top = 0;
		r.setWidth(_width);
		r.setHeight(_height);
	}
};


#define CENTER_LABEL_HORIZONTAL	-1
#define CENTER_LABEL_VERTICAL	-1

struct Label {
	Graphics::Surface	_cnv;

	Common::Point		_pos;
	bool				_visible;

	Label();
	~Label();

	void free();
	void resetPosition();
};




#define NUM_BUFFERS 4
#define MAX_BALLOON_WIDTH 130

class Parallaction;

struct DoorData;
struct GetData;
struct Label;


class Gfx {

public:
	enum Buffers {
		// bit buffers
		kBitFront,
		kBitBack,
		kBit2
	};

public:

	// balloons and text
	uint16 getStringWidth(const char *text);
	void getStringExtent(char *text, uint16 maxwidth, int16* width, int16* height);

	// labels
	void setFloatingLabel(Label *label);
	Label *renderFloatingLabel(Font *font, char *text);
	uint createLabel(Font *font, const char *text, byte color);
	void showLabel(uint id, int16 x, int16 y);
	void hideLabel(uint id);
	void freeLabels();

	// cut/paste
	void patchBackground(Graphics::Surface &surf, int16 x, int16 y, bool mask = false);

	void flatBlitCnv(Graphics::Surface *cnv, int16 x, int16 y, Gfx::Buffers buffer);
	void flatBlitCnv(Frames *cnv, uint16 frame, int16 x, int16 y, Gfx::Buffers buffer);
	void blitCnv(Graphics::Surface *cnv, int16 x, int16 y, uint16 z, Gfx::Buffers buffer);
	void restoreBackground(const Common::Rect& r);
	void backupDoorBackground(DoorData *data, int16 x, int16 y);
	void restoreDoorBackground(const Common::Rect& r, byte *data, byte* background);
	void backupGetBackground(GetData *data, int16 x, int16 y);
	void restoreGetBackground(const Common::Rect& r, byte *data);

	int setLocationBalloon(char *text, bool endGame);
	int setDialogueBalloon(char *text, uint16 winding, byte textColor);
	int setSingleBalloon(char *text, uint16 x, uint16 y, uint16 winding, byte textColor);
	void setBalloonText(uint id, char *text, byte textColor);
	int hitTestDialogueBalloon(int x, int y);

	int setItem(Frames* frames, uint16 x, uint16 y);
	void setItemFrame(uint item, uint16 f);
	void hideDialogueStuff();
	void freeBalloons();
	void freeItems();

	// low level surfaces
	void clearScreen(Gfx::Buffers buffer);
	void copyScreen(Gfx::Buffers srcbuffer, Gfx::Buffers dstbuffer);
	void copyRect(Gfx::Buffers dstbuffer, const Common::Rect& r, byte *src, uint16 pitch);
	void grabRect(byte *dst, const Common::Rect& r, Gfx::Buffers srcbuffer, uint16 pitch);
	void floodFill(Gfx::Buffers buffer, const Common::Rect& r, byte color);
	void invertRect(Gfx::Buffers buffer, const Common::Rect& r);

	// palette
	void setPalette(Palette palette);
	void setBlackPalette();
	void animatePalette();

	// amiga specific
	void setHalfbriteMode(bool enable);
	void setProjectorPos(int x, int y);

	// misc
	int16 queryMask(int16 v);
	void swapBuffers();
	void updateScreen();
	void setBackground(Graphics::Surface *surf);
	void setMask(MaskBuffer *buffer);

	// init
	Gfx(Parallaction* vm);
	virtual ~Gfx();


public:
	uint16				_bgLayers[4];
	PaletteFxRange		_palettefx[6];
	Palette				_palette;

	int 				_backgroundWidth;
	int 				_backgroundHeight;

	uint				_screenX;		// scrolling position
	uint				_screenY;

protected:
	Parallaction*		_vm;
	Graphics::Surface	*_buffers[NUM_BUFFERS];
	MaskBuffer			*_depthMask;
	Font				*_font;
	bool				_halfbrite;

	Common::Point		_hbCirclePos;
	int				_hbCircleRadius;


protected:
	static int16 _dialogueBalloonX[5];

	struct Balloon {
		uint16 x;
		uint16 y;
		Common::Rect outerBox;
		Common::Rect innerBox;
		uint16 winding;
		Graphics::Surface surface;
	} _balloons[5];

	uint	_numBalloons;

	struct Item {
		uint16 x;
		uint16 y;
		uint16 frame;
		Frames *data;

		Common::Rect rect;
	} _items[2];

	uint	_numItems;

	#define MAX_NUM_LABELS	5
	Label*	_labels[MAX_NUM_LABELS];
	uint	_numLabels;
	Label	*_floatingLabel;

	void drawInventory();
	void updateFloatingLabel();
	void drawLabels();
	void drawItems();
	void drawBalloons();

	void initBuffers(int w, int h);
	void freeBuffers();

	void copyRect(uint width, uint height, byte *dst, uint dstPitch, byte *src, uint srcPitch);

	int createBalloon(int16 w, int16 h, int16 winding, uint16 borderThickness);
	Balloon *getBalloon(uint id);

	// low level text and patches
	void setFont(Font* font);
	void drawText(Graphics::Surface* surf, uint16 x, uint16 y, const char *text, byte color);
	bool drawWrappedText(Graphics::Surface* surf, char *text, byte color, int16 wrapwidth);

    void blt(const Common::Rect& r, byte *data, Graphics::Surface *surf, uint16 z, byte transparentColor);
};


} // Parallaction


#endif






