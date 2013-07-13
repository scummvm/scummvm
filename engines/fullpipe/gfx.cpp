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

#include "fullpipe/fullpipe.h"

#include "fullpipe/objects.h"

namespace Fullpipe {

Background::Background() {
	_x = 0;
	_y = 0;
	_messageQueueId = 0;
	_bigPictureArray1Count = 0;
	_bigPictureArray2Count = 0;
	_bigPictureArray = 0;
	_stringObj = 0;
	_colorMemoryObj = 0;
}

bool Background::load(MfcArchive &file) {
	_stringObj = file.readPascalString();

	int count = file.readUint16LE();

	for (int i = 0; i < count; i++) {
		PictureObject *pct = new PictureObject();

		pct->load(file, i == 0);
		addPictureObject(pct);
	}

	assert(g_fullpipe->_gameProjectVersion >= 4);

	_bigPictureArray1Count = file.readUint32LE();

	assert(g_fullpipe->_gameProjectVersion >= 5);

	_bigPictureArray2Count = file.readUint32LE();

	_bigPictureArray = (BigPicture ***)calloc(_bigPictureArray1Count, sizeof(BigPicture **));

	debug(6, "bigPictureArray[%d][%d]", _bigPictureArray1Count, _bigPictureArray2Count);

	for (int i = 0; i < _bigPictureArray1Count; i++) {
		_bigPictureArray[i] = (BigPicture **)calloc(_bigPictureArray2Count, sizeof(BigPicture *));
		for (int j = 0; j < _bigPictureArray2Count; j++) {
		  _bigPictureArray[i][j] = new BigPicture();

		  _bigPictureArray[i][j]->load(file);
		}
	}

	return true;
}

void Background::addPictureObject(PictureObject *pct) {
	if (pct->_field_4)
		renumPictures(pct);

	bool inserted = false;
	for (uint i = 0; i < _picObjList.size(); i++) {
		if (((PictureObject *)_picObjList[i])->_priority == pct->_priority) {
	  		_picObjList.insert_at(i, pct);
			inserted = true;
			break;
		}
	}

	if (!inserted) {
		_picObjList.push_back(pct);
	}
}

void Background::renumPictures(PictureObject *pct) {
  int *buf = (int *)calloc(_picObjList.size() + 2, sizeof(int));

	for (uint i = 0; i < _picObjList.size(); i++) {
		if (pct->_id == ((PictureObject *)_picObjList[i])->_id)
			buf[((PictureObject *)_picObjList[i])->_field_4] = 1;
	}
	
	if (buf[pct->_field_4]) {
		uint count;
		for (count = 1; buf[count] && count < _picObjList.size() + 2; count++)
			;
		pct->_field_4 = count;
	}

	free(buf);
}

PictureObject::PictureObject() {
	_ox = 0;
	_oy = 0;
	_picture = 0;
}

bool PictureObject::load(MfcArchive &file, bool bigPicture) {
	GameObject::load(file);

	if (bigPicture)
		_picture = new BigPicture();
	else
		_picture = new Picture();

	_picture->load(file);

	_pictureObject2List = new CPtrList();

	int count = file.readUint16LE();

	if (count > 0) {
		GameObject *o = new GameObject();
		
		o->load(file);
		_pictureObject2List->push_back(o);
	}

	_ox2 = _ox;
	_oy2 = _oy;

	return true;
}

GameObject::GameObject() {
	_field_4 = 0;
	_flags = 0;
	_id = 0;
	_ox = 0;
	_oy = 0;
	_priority = 0;
	_field_20 = 0;
	_field_8 = 0;
}

bool GameObject::load(MfcArchive &file) {
	_field_4 = 0;
	_flags = 0;
	_field_20 = 0;
	
	_id = file.readUint16LE();
	
	_stringObj = file.readPascalString();
	_ox = file.readUint32LE();
	_oy = file.readUint32LE();
	_priority = file.readUint16LE();

	if (g_fullpipe->_gameProjectVersion >= 11) {
	  _field_8 = file.readUint32LE();
	}

	return true;
}

void GameObject::setOXY(int x, int y) {
	_ox = x;
	_oy = y;
}

Picture::Picture() {
	_x = 0;
	_y = 0;
	_field_44 = 0;
	_field_54 = 0;
	_bitmap = 0;
	_alpha = -1;
	_paletteData = 0;
	_convertedBitmap = 0;
}

bool Picture::load(MfcArchive &file) {
	MemoryObject::load(file);

	_x = file.readUint32LE();
	_y = file.readUint32LE();
	_field_44 = file.readUint16LE();
	
	assert(g_fullpipe->_gameProjectVersion >= 2);

	_width = file.readUint32LE();
	_height = file.readUint32LE();

	_flags |= 1;

	_memoryObject2 = new MemoryObject2;
	_memoryObject2->load(file);

	if (_memoryObject2->_data) {
		setAOIDs();
	}

	assert (g_fullpipe->_gameProjectVersion >= 12);

	_alpha = file.readUint32LE();

	int havePal = file.readUint32LE();

	if (havePal > 0) {
		_paletteData = (byte *)calloc(1024, 1);
		file.read(_paletteData, 1024);
	}

	return true;
}

void Picture::setAOIDs() {
	warning("STUB: Picture::setAOIDs()");
}

BigPicture::BigPicture() {
}

bool BigPicture::load(MfcArchive &file) {
	Picture::load(file);

	return true;
}

Shadows::Shadows() {
	_staticAniObjectId = 0;
	_movementId = 0;
	_sceneId = 0;
}

bool Shadows::load(MfcArchive &file) {
	_sceneId = file.readUint32LE();
	_staticAniObjectId = file.readUint32LE();
	_movementId = file.readUint32LE();

	return true;
}


} // End of namespace Fullpipe
