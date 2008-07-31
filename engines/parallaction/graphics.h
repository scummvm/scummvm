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

#include "common/list.h"
#include "common/rect.h"
#include "common/hash-str.h"
#include "common/stream.h"

#include "graphics/surface.h"


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
	virtual uint	getRawSize(uint16 index) = 0;
	virtual uint	getSize(uint16 index) = 0;

	virtual ~Frames() { }

};


struct SurfaceToFrames : public Frames {

	Graphics::Surface	*_surf;

public:
	SurfaceToFrames(Graphics::Surface *surf) : _surf(surf) {
	}

	~SurfaceToFrames() {
		_surf->free();
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
	uint	getRawSize(uint16 index) {
		assert(index == 0);
		return getSize(index);
	}
	uint	getSize(uint16 index) {
		assert(index == 0);
		return _surf->w * _surf->h;
	}

};

/*
	this adapter can handle Surfaces containing multiple frames,
	provided all these frames are the same width and height
*/
struct SurfaceToMultiFrames : public Frames {

	uint _num;
	uint _width, _height;
	Graphics::Surface *_surf;

	SurfaceToMultiFrames(uint num, uint w, uint h, Graphics::Surface *surf) : _num(num), _width(w), _height(h), _surf(surf) {

	}

	~SurfaceToMultiFrames() {
		delete _surf;
	}

	uint16	getNum() {
		return _num;
	}
	byte*	getData(uint16 index) {
		assert(index < _num);
		return (byte*)_surf->getBasePtr(0, _height * index);
	}
	void	getRect(uint16 index, Common::Rect &r) {
		assert(index < _num);
		r.left = 0;
		r.top = 0;
		r.setWidth(_width);
		r.setHeight(_height);
	}
	uint	getRawSize(uint16 index) {
		assert(index < _num);
		return getSize(index);
	}
	uint	getSize(uint16 index) {
		assert(index < _num);
		return _width * _height;
	}

};

struct MaskBuffer {
	// handles a 2-bit depth buffer used for z-buffering

	uint16	w;
	uint16  internalWidth;
	uint16	h;
	uint	size;
	byte	*data;
	bool	bigEndian;

public:
	MaskBuffer() : w(0), internalWidth(0), h(0), size(0), data(0), bigEndian(true) {
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
		uint n;
		if (bigEndian) {
			n = (x & 3) << 1;
		} else {
			n = (3 - (x & 3)) << 1;
		}
		return (m >> n) & 3;
	}

};


struct PathBuffer {
	// handles a 1-bit depth buffer used for masking non-walkable areas

	uint16	w;
	uint16  internalWidth;
	uint16	h;
	uint	size;
	byte	*data;

public:
	PathBuffer() : w(0), internalWidth(0), h(0), size(0), data(0) {
	}

	void create(uint16 width, uint16 height) {
		w = width;
		internalWidth = w >> 3;
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

	inline byte getValue(uint16 x, uint16 y);
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
	void getEntry(uint index, int &red, int &green, int &blue);
	void setEntry(uint index, int red, int green, int blue);
	void makeGrayscale();
	void fadeTo(const Palette& target, uint step);
	uint fillRGBA(byte *rgba);

	void rotate(uint first, uint last, bool forward);
};


struct Cnv : public Frames {
	uint16	_count;		// # of frames
	uint16	_width;		//
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
	uint	getRawSize(uint16 index) {
		assert(index < _count);
		return getSize(index);
	}
	uint	getSize(uint16 index) {
		assert(index < _count);
		return _width * _height;
	}

};


#define CENTER_LABEL_HORIZONTAL	-1
#define CENTER_LABEL_VERTICAL	-1



#define MAX_BALLOON_WIDTH 130

class Parallaction;

struct DoorData;
struct GetData;
struct Label;
class Disk;

enum {
	kGfxObjVisible = 1,

	kGfxObjNormal = 2,
	kGfxObjCharacter = 4,

	kGfxObjTypeDoor = 0,
	kGfxObjTypeGet = 1,
	kGfxObjTypeAnim = 2,
	kGfxObjTypeLabel = 3,
	kGfxObjTypeBalloon = 4,
	kGfxObjTypeCharacter = 8
};

enum {
	kGfxObjDoorZ = -200,
	kGfxObjGetZ = -100
};

class GfxObj {
	char *_name;
	Frames *_frames;

	bool _keep;

public:
	int16 x, y;

	int32 z;

	uint32 _flags;

	uint type;
	uint frame;
	uint layer;
	uint transparentKey;

	GfxObj(uint type, Frames *frames, const char *name = NULL);
	virtual ~GfxObj();

	const char *getName() const;

	uint getNum();
	void getRect(uint frame, Common::Rect &r);
	byte *getData(uint frame);
	uint getRawSize(uint frame);
	uint getSize(uint frame);


	void setFlags(uint32 flags);
	void clearFlags(uint32 flags);
	bool isVisible() {
		return (_flags & kGfxObjVisible) == kGfxObjVisible;
	}

	void release();
};

typedef Common::List<GfxObj*> GfxObjList;

#define LAYER_FOREGROUND   3

/*
	BackgroundInfo keeps information about the background bitmap that can be seen in the game.
	These bitmaps can be of any size, smaller or larger than the visible screen, the latter
	being the most common options.
*/
struct BackgroundInfo {
	int x, y;		// used to display bitmaps smaller than the screen
	int width;
	int height;

	Graphics::Surface	bg;
	MaskBuffer			mask;
	PathBuffer			path;

	Palette				palette;

	int 				layers[4];
	PaletteFxRange		ranges[6];

	BackgroundInfo() : x(0), y(0), width(0), height(0) {
		layers[0] = layers[1] = layers[2] = layers[3] = 0;
		memset(ranges, 0, sizeof(ranges));
	}

	void setPaletteRange(int index, const PaletteFxRange& range) {
		assert(index < 6);
		memcpy(&ranges[index], &range, sizeof(PaletteFxRange));
	}

	uint16 getLayer(uint16 z) {
		for (uint16 i = 0; i < 3; i++) {
			if (layers[i+1] > z) return i;
		}
		return LAYER_FOREGROUND;
	}

	~BackgroundInfo() {
		bg.free();
		mask.free();
		path.free();
		x = 0;
		y = 0;
		width = 0;
		height = 0;
	}

};


enum {
	kBackgroundLocation = 1,
	kBackgroundSlide = 2
};


class BalloonManager {
public:
	virtual ~BalloonManager() { }

	virtual void freeBalloons() = 0;
	virtual int setLocationBalloon(char *text, bool endGame) = 0;
	virtual int setDialogueBalloon(char *text, uint16 winding, byte textColor) = 0;
	virtual int setSingleBalloon(char *text, uint16 x, uint16 y, uint16 winding, byte textColor) = 0;
	virtual void setBalloonText(uint id, char *text, byte textColor) = 0;
	virtual int hitTestDialogueBalloon(int x, int y) = 0;
};


typedef Common::HashMap<Common::String, int32, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> VarMap;

class Gfx {

protected:
	Parallaction*		_vm;

public:
	Disk *_disk;
	VarMap _vars;

	GfxObjList _gfxobjList;
	GfxObj* loadAnim(const char *name);
	GfxObj* loadGet(const char *name);
	GfxObj* loadDoor(const char *name);
	void drawGfxObjects(Graphics::Surface &surf);
	void showGfxObj(GfxObj* obj, bool visible);
	void clearGfxObjects(uint filter);
	void sortAnimations();


	// labels
	void showFloatingLabel(uint label);
	void hideFloatingLabel();

	uint renderFloatingLabel(Font *font, char *text);
	uint createLabel(Font *font, const char *text, byte color);
	void showLabel(uint id, int16 x, int16 y);
	void hideLabel(uint id);
	void freeLabels();

	// dialogue balloons
	GfxObj* registerBalloon(Frames *frames, const char *text);
	void destroyBalloons();

	// other items
	int setItem(GfxObj* obj, uint16 x, uint16 y, byte transparentColor = 0);
	void setItemFrame(uint item, uint16 f);
	void hideDialogueStuff();
	void freeBalloons();
	void freeItems();

	// background surface
	BackgroundInfo	*_backgroundInfo;
	void setBackground(uint type, const char* name, const char* mask, const char* path);
	void patchBackground(Graphics::Surface &surf, int16 x, int16 y, bool mask = false);
	void grabBackground(const Common::Rect& r, Graphics::Surface &dst);
	void fillBackground(const Common::Rect& r, byte color);
	void invertBackground(const Common::Rect& r);
	void freeBackground();

	// palette
	void setPalette(Palette palette);
	void setBlackPalette();
	void animatePalette();

	// amiga specific
	void setHalfbriteMode(bool enable);
	void setProjectorPos(int x, int y);
	void setProjectorProgram(int16 *data);
	int16 *_nextProjectorPos;


	// init
	Gfx(Parallaction* vm);
	virtual ~Gfx();

	void beginFrame();

	void registerVar(const Common::String &name, int32 initialValue);
	void setVar(const Common::String &name, int32 value);
	int32 getVar(const Common::String &name);

	void clearScreen();
	void updateScreen();

public:
	Palette				_palette;

	uint				_screenX;		// scrolling position
	uint				_screenY;

	byte				*_unpackedBitmap;

protected:
	bool				_halfbrite;

	bool 				_skipBackground;

	Common::Point		_hbCirclePos;
	int				_hbCircleRadius;

	// BRA specific
	Palette				_backupPal;

	// frame data stored in programmable variables
	int32				_varBackgroundMode;	// 1 = normal, 2 = only mask
	int32				_varScrollX;
	int32				_varAnimRenderMode;	// 1 = normal, 2 = flat
	int32				_varMiscRenderMode;	// 1 = normal, 2 = flat
	int32				_varRenderMode;
	int32				_varDrawPathZones;	// 0 = don't draw, 1 = draw
	Graphics::Surface 	_bitmapMask;
	int32 				getRenderMode(const char *type);

public:

	struct Item {
		GfxObj *data;
	} _items[14];

	uint	_numItems;

	#define MAX_NUM_LABELS	20
	#define NO_FLOATING_LABEL	1000

	typedef Common::Array<GfxObj*> GfxObjArray;
	GfxObjArray	_labels;
	GfxObjArray _balloons;

	uint _floatingLabel;

	void drawInventory();
	void updateFloatingLabel();
	void drawLabels();
	void drawItems();
	void drawBalloons();

	void copyRect(const Common::Rect &r, Graphics::Surface &src, Graphics::Surface &dst);

	// low level text and patches
	void drawText(Font *font, Graphics::Surface* surf, uint16 x, uint16 y, const char *text, byte color);

	void drawGfxObject(GfxObj *obj, Graphics::Surface &surf, bool scene);
    void blt(const Common::Rect& r, byte *data, Graphics::Surface *surf, uint16 z, byte transparentColor);
	void unpackBlt(const Common::Rect& r, byte *data, uint size, Graphics::Surface *surf, uint16 z, byte transparentColor);
};


} // Parallaction


#endif









