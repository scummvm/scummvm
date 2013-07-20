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
 */

#ifndef FULLPIPE_GFX_H
#define FULLPIPE_GFX_H

class Common::ReadStream;

namespace Fullpipe {

class ShadowsItemArray : public CObArray {
	// empty
};

struct Bitmap {
	int _x;
	int _y;
	int _width;
	int _height;
	byte *_pixels;
	int _type;
	int _field_18;
	int _flags;

	void load(Common::ReadStream *s);
	void putDib(int x, int y, int32 *palette);
	void putDibRB(int32 *palette);
	void putDibCB(int32 *palette);

	void colorFill(uint16 *dest, int len, int color);
	void paletteFill(uint16 *dest, byte *src, int len, int32 *palette);
	void copierKeyColor(uint16 *dest, byte *src, int len, int keyColor, int32 *palette, bool cb05_format);
	void copier(uint16 *dest, byte *src, int len, int32 *palette, bool cb05_format);
};

class Picture : public MemoryObject {
	friend class Movement;

	Common::Rect _rect;
	int _convertedBitmap;
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
	virtual bool load(MfcArchive &file);
	void setAOIDs();
	void init();
	void getDibInfo();
	Bitmap *getPixelData();
	void draw(int x, int y, int style, int angle);
	void drawRotated(int x, int y, int angle);

	byte getAlpha() { return (byte)_alpha; }
	void setAlpha(byte alpha) { _alpha = alpha; }

	Common::Point *getDimensions(Common::Point *p);
};

class BigPicture : public Picture {
  public:
	BigPicture();
	virtual bool load(MfcArchive &file);
};

class GameObject : public CObject {
  public:
	int16 _field_4;
	int16 _field_6;
	int _field_8;
	int16 _flags;
	int16 _id;
	char *_objectName;
	int _ox;
	int _oy;
	int _priority;
	int _field_20;

  public:
	GameObject();
	virtual bool load(MfcArchive &file);
	void setOXY(int x, int y);
};

class PictureObject : public GameObject {
	Picture *_picture;
	CPtrList *_pictureObject2List;
	int _ox2;
	int _oy2;

  public:
	PictureObject();
	bool load(MfcArchive &file, bool bigPicture);
	Common::Point *getDimensions(Common::Point *p);
};

class Background : public CObject {
	friend class FullpipeEngine;

  public:
	CPtrList _picObjList;

  protected:
	char *_bgname;
	int _x;
	int _y;
	int16 _messageQueueId;
	MemoryObject *_palette;
	int _bigPictureArray1Count;
	int _bigPictureArray2Count;
	BigPicture ***_bigPictureArray;

  public:
	Background();
	virtual bool load(MfcArchive &file);
	void addPictureObject(PictureObject *pct);
	void renumPictures(PictureObject *pct);
};

class Shadows : public CObject {
	int _sceneId;
	int _staticAniObjectId;
	int _movementId;
	ShadowsItemArray _items;

  public:
	Shadows();
	virtual bool load(MfcArchive &file);
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_GFX_H */
