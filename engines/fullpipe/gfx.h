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

#ifndef FULLPIPE_GFX_H
#define FULLPIPE_GFX_H

namespace Graphics {
	struct Surface;
	struct TransparentSurface;
}

namespace Fullpipe {

class DynamicPhase;
class Movement;
struct PicAniInfo;

struct Bitmap {
	int _x;
	int _y;
	int _width;
	int _height;
	int _type;
	int _dataSize;
	int _flags;
	Graphics::TransparentSurface *_surface;
	int _flipping;
	bool _copied_surface;

	Bitmap();
	Bitmap(Bitmap *src);
	~Bitmap();

	void load(Common::ReadStream *s);
	void decode(byte *pixels, int32 *palette);
	void putDib(int x, int y, int32 *palette, byte alpha);
	bool putDibRB(byte *pixels, int32 *palette);
	void putDibCB(byte *pixels, int32 *palette);

	void colorFill(uint32 *dest, int len, int32 color);
	void paletteFill(uint32 *dest, byte *src, int len, int32 *palette);
	void copierKeyColor(uint32 *dest, byte *src, int len, int keyColor, int32 *palette, bool cb05_format);
	void copier(uint32 *dest, byte *src, int len, int32 *palette, bool cb05_format);

	Bitmap *reverseImage(bool flip = true);
	Bitmap *flipVertical();

	void drawShaded(int type, int x, int y, byte *palette, int alpha);
	void drawRotated(int x, int y, int angle, byte *palette, int alpha);

	bool isPixelHitAtPos(int x, int y);
};

class Picture : public MemoryObject {
 public:
	Common::Rect _rect;
	Bitmap *_convertedBitmap;
	int _x;
	int _y;
	int _field_44;
	int _width;
	int _height;
	Bitmap *_bitmap;
	int _field_54;
	MemoryObject2 *_memoryObject2;
	int _alpha;
	byte *_paletteData;

	void displayPicture();

  public:
	Picture();
	virtual ~Picture();

	void freePicture();
	void freePixelData();

	virtual bool load(MfcArchive &file);
	void setAOIDs();
	virtual void init();
	void getDibInfo();
	Bitmap *getPixelData();
	virtual void draw(int x, int y, int style, int angle);
	void drawRotated(int x, int y, int angle);

	byte getAlpha() { return (byte)_alpha; }
	void setAlpha(byte alpha) { _alpha = alpha; }

	Common::Point *getDimensions(Common::Point *p);
	bool isPointInside(int x, int y);
	bool isPixelHitAtPos(int x, int y);
	int getPixelAtPos(int x, int y);
	int getPixelAtPosEx(int x, int y);

	byte *getPaletteData() { return _paletteData; }
	void setPaletteData(byte *pal);

	void copyMemoryObject2(Picture *src);
};

class BigPicture : public Picture {
  public:
	BigPicture() {}
	virtual ~BigPicture() {}

	virtual bool load(MfcArchive &file);
	virtual void draw(int x, int y, int style, int angle);
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
	~GameObject();

	virtual bool load(MfcArchive &file);
	void setOXY(int x, int y);
	void renumPictures(Common::Array<StaticANIObject *> *lst);
	void renumPictures(Common::Array<PictureObject *> *lst);
	void setFlags(int16 flags) { _flags = flags; }
	void clearFlags() { _flags = 0; }
	Common::String getName() { return _objectName; }

	bool getPicAniInfo(PicAniInfo *info);
	bool setPicAniInfo(PicAniInfo *info);
};

class PictureObject : public GameObject {
  public:
	Picture *_picture;
	Common::Array<GameObject *> *_pictureObject2List;
	int _ox2;
	int _oy2;

  public:
	PictureObject();

	PictureObject(PictureObject *src);
	virtual ~PictureObject();

	virtual bool load(MfcArchive &file, bool bigPicture);
	virtual bool load(MfcArchive &file) { assert(0); return false; } // Disable base class

	Common::Point *getDimensions(Common::Point *p);
	void draw();
	void drawAt(int x, int y);

	bool setPicAniInfo(PicAniInfo *picAniInfo);
	bool isPointInside(int x, int y);
	bool isPixelHitAtPos(int x, int y);
	void setOXY2();
};

class Background : public CObject {
  public:
	Common::Array<PictureObject *> _picObjList;

	Common::String _bgname;
	int _x;
	int _y;
	int16 _messageQueueId;
	MemoryObject *_palette;
	int _bigPictureArray1Count;
	int _bigPictureArray2Count;
	BigPicture ***_bigPictureArray;

  public:
	Background();
	virtual ~Background();

	virtual bool load(MfcArchive &file);
	void addPictureObject(PictureObject *pct);

	BigPicture *getBigPicture(int x, int y) { return _bigPictureArray[x][y]; }
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
	virtual bool load(MfcArchive &file);
	void init();

	void initMovement(Movement *mov);
	DynamicPhase *findSize(int width, int height);
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_GFX_H */
