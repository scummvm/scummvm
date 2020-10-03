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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef NGI_GFX_H
#define NGI_GFX_H

#include "common/ptr.h"

namespace Graphics {
	struct Surface;
	struct TransparentSurface;
}

namespace NGI {

class DynamicPhase;
class Movement;
struct PicAniInfo;

typedef Common::Point Dims;

struct Bitmap {
	int _x;
	int _y;
	int _width;
	int _height;
	int _type;
	int _dataSize;
	int _flags;
	int _flipping;
	Graphics::TransparentSurface *_surface;

	Bitmap();
	Bitmap(const Bitmap &src);
	~Bitmap();

	void load(Common::ReadStream *s);
	void decode(byte *pixels, const Palette &palette);
	void putDib(int x, int y, const Palette &palette, byte alpha);
	bool putDibRB(byte *pixels, const Palette &palette);
	void putDibCB(byte *pixels, const Palette &palette);

	void colorFill(uint32 *dest, int len, int32 color);
	void paletteFill(uint32 *dest, byte *src, int len, const Palette &palette);
	void copierKeyColor(uint32 *dest, byte *src, int len, int keyColor, const Palette &palette, bool cb05_format);
	void copier(uint32 *dest, byte *src, int len, const Palette &palette, bool cb05_format);

	/** ownership of returned object is transferred to caller */
	Bitmap *reverseImage(bool flip = true) const;
	/** ownership of returned object is transferred to caller */
	Bitmap *flipVertical() const;

	void drawShaded(int type, int x, int y, const Palette &palette, int alpha);
	void drawRotated(int x, int y, int angle, const Palette &palette, int alpha);

	bool isPixelHitAtPos(int x, int y);

private:
	Bitmap operator=(const Bitmap &);
};

class Picture : public MemoryObject {
public:
	Picture();
	~Picture() override;

	void freePicture();
	void freePixelData();

	bool load(MfcArchive &file) override;
	void setAOIDs();
	virtual void init();
	void getDibInfo();
	const Bitmap *getPixelData();
	virtual void draw(int x, int y, int style, int angle);
	void drawRotated(int x, int y, int angle);

	byte getAlpha() { return (byte)_alpha; }
	void setAlpha(byte alpha) { _alpha = alpha; }

	Dims getDimensions() const { return Dims(_width, _height); }
	bool isPointInside(int x, int y);
	bool isPixelHitAtPos(int x, int y);
	int getPixelAtPos(int x, int y);
	int getPixelAtPosEx(int x, int y);

	const Bitmap *getConvertedBitmap() const { return _convertedBitmap.get(); }
	const Palette &getPaletteData() const { return _paletteData; }
	void setPaletteData(const Palette &pal);

	void copyMemoryObject2(Picture &src);

	int _x, _y;

protected:
	Common::Rect _rect;
	Common::ScopedPtr<Bitmap> _convertedBitmap;
	int _field_44;
	int _width;
	int _height;
	Common::ScopedPtr<Bitmap> _bitmap;
	int _field_54;
	Common::ScopedPtr<MemoryObject2> _memoryObject2;
	int _alpha;
	Palette _paletteData;

	void displayPicture();
};

class BigPicture : public Picture {
  public:
	BigPicture() {}
	~BigPicture() override {}

	bool load(MfcArchive &file) override;
	void draw(int x, int y, int style, int angle) override;
};

class GameObject : public CObject {
  public:
	int16 _odelay;
	int _field_8;
	int16 _flags;
	int16 _id;
	Common::String _objectName;
	int _ox;
	int _oy;
	int _priority;
	int _field_20;

  public:
	GameObject();
	GameObject(GameObject *src);

	virtual Common::String toXML();
	bool load(MfcArchive &file) override;
	void setOXY(int x, int y);
	void renumPictures(Common::Array<StaticANIObject *> *lst);
	void renumPictures(Common::Array<PictureObject *> *lst);
	void setFlags(int16 flags) { _flags = flags; }
	void clearFlags() { _flags = 0; }
	Common::String getName() { return _objectName; }

	bool getPicAniInfo(PicAniInfo &info);
	bool setPicAniInfo(const PicAniInfo &info);
};

class PictureObject : public GameObject {
public:
	PictureObject();

	PictureObject(PictureObject *src);

	virtual bool load(MfcArchive &file, bool bigPicture);
	bool load(MfcArchive &file) override { assert(0); return false; } // Disable base class

	Dims getDimensions() const { return _picture->getDimensions(); }
	void draw();
	void drawAt(int x, int y);

	bool setPicAniInfo(const PicAniInfo &picAniInfo);
	bool isPointInside(int x, int y);
	bool isPixelHitAtPos(int x, int y);
	void setOXY2();

	Picture *_picture;

private:
	Common::Array<GameObject> _pictureObject2List;
	int _ox2;
	int _oy2;
};

class Background : public CObject {
public:
	/** list items are owned */
	Common::Array<PictureObject *> _picObjList;

	Common::String _bgname;
	int _x;
	int _y;
	int16 _messageQueueId;
	Palette _palette;
	/** list items are owned */
	Common::Array<BigPicture *> _bigPictureArray;
	uint _bigPictureXDim;
	uint _bigPictureYDim;

public:
	Background();
	~Background() override;

	bool load(MfcArchive &file) override;
	void addPictureObject(PictureObject *pct);

	BigPicture *getBigPicture(int x, int y) { return _bigPictureArray[y * _bigPictureXDim + x]; }
};

struct ShadowsItem {
	int width;
	int height;
	DynamicPhase *dynPhase;
};

typedef Common::Array<ShadowsItem> ShadowsItemArray;

class Shadows : public CObject {
	int _sceneId;
	int _staticAniObjectId;
	int _movementId;
	ShadowsItemArray _items;

  public:
	Shadows();
	bool load(MfcArchive &file) override;
	void init();

	void initMovement(Movement *mov);
	DynamicPhase *findSize(int width, int height);
};

} // End of namespace NGI

#endif /* NGI_GFX_H */
