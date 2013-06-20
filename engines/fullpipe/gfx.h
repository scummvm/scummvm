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

class Background {
	CPtrList list;
	int stringObj;
	int x;
	int y;
	int16 messageQueueId;
	int colorMemoryObj;
	int bigPictureArray1Count;
	int bigPictureArray2Count;
	int bigPictureArray;
};

class Shadows {
	//CObject obj;
	int sceneId;
	int staticAniObjectId;
	int movementId;
	ShadowsItemArray items;
};

class Picture {
	MemoryObject obj;
	Common::Rect rect;
	int convertedBitmap;
	int x;
	int y;
	int field_44;
	int width;
	int height;
	int bitmap;
	int field_54;
	int memoryObject2;
	int alpha;
	int paletteData;
};

class BigPicture {
	Picture pic;
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_GFX_H */
