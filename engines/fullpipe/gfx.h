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

namespace Fullpipe {

class ShadowsItemArray : public Common::Array<CObject>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

class Picture : public MemoryObject {
	Common::Rect _rect;
	int _convertedBitmap;
	int _x;
	int _y;
	int _field_44;
	int _width;
	int _height;
	int _bitmap;
	int _field_54;
	MemoryObject2 *_memoryObject2;
	int _alpha;
	byte *_paletteData;

  public:
	Picture();
	virtual bool load(MfcArchive &file);
	void setAOIDs();
};

class BigPicture : public Picture {
  public:
	BigPicture();
	virtual bool load(MfcArchive &file);
};

class GameObject : public CObject {
	friend class PictureObject;

	int16 _field_4;
	int16 _field_6;
	int _field_8;
	int16 _flags;
	int16 _id;
	char *_stringObj;
	int _ox;
	int _oy;
	int _priority;
	int _field_20;

  public:
	GameObject();
	virtual bool load(MfcArchive &file);
};

class PictureObject : public GameObject {
	Picture *_picture;
	CPtrList *_pictureObject2List;
	int _ox2;
	int _oy2;

  public:
	PictureObject();
	bool load(MfcArchive &file, bool bigPicture);
};

class Background : public CObject {
	CPtrList _list;
	char *_stringObj;
	int _x;
	int _y;
	int16 _messageQueueId;
	int _colorMemoryObj;
	int _bigPictureArray1Count;
	int _bigPictureArray2Count;
	BigPicture ***_bigPictureArray;

  public:
	Background();
	virtual bool load(MfcArchive &file);
	void addPictureObject(PictureObject *pct);
};

class Shadows {
	//CObject obj;
	int sceneId;
	int staticAniObjectId;
	int movementId;
	ShadowsItemArray items;
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_GFX_H */
