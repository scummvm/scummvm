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

#include "fullpipe/fullpipe.h"

#include "fullpipe/objects.h"
#include "fullpipe/gfx.h"
#include "fullpipe/statics.h"
#include "fullpipe/scene.h"
#include "fullpipe/interaction.h"
#include "fullpipe/gameloader.h"

#include "common/memstream.h"

namespace Fullpipe {

Bitmap::Bitmap() {
	_x = 0;
	_y = 0;
	_width = 0;
	_height = 0;
	_pixels = 0;
	_type = 0;
	_dataSize = 0;
	_flags = 0;
}

Bitmap::Bitmap(Bitmap *src) {
	_x = src->_x;
	_y = src->_y;
	_flags = src->_flags;
	_dataSize = src->_dataSize;
	_type = src->_type;
	_width = src->_width;
	_height = src->_height;
	_pixels = src->_pixels;
}

Bitmap::~Bitmap() {
	if (_pixels)
		free(_pixels);

	_pixels = 0;
}

void Bitmap::load(Common::ReadStream *s) {
	debug(5, "Bitmap::load()");

	_x = s->readUint32LE();
	_y = s->readUint32LE();
	_width = s->readUint32LE();
	_height = s->readUint32LE();
	s->readUint32LE(); // pixels
	_type = s->readUint32LE();
	_dataSize = s->readUint32LE();
	_flags = s->readUint32LE();

	debug(8, "Bitmap: x: %d y: %d w: %d h: %d dataSize: 0x%x", _x, _y, _width, _height, _dataSize);
	debug(8, "Bitmap: type: %s (0x%04x) flags: 0x%x", Common::tag2string(_type).c_str(), _type, _flags);
}

Background::Background() {
	_x = 0;
	_y = 0;
	_messageQueueId = 0;
	_bigPictureArray1Count = 0;
	_bigPictureArray2Count = 0;
	_bigPictureArray = 0;
	_bgname = 0;
	_palette = 0;
}

Background::~Background() {
	_picObjList.clear();

	for (int i = 0; i < _bigPictureArray1Count; i++) {
		for (int j = 0; j < _bigPictureArray2Count; j++)
			delete _bigPictureArray[i][j];

		free(_bigPictureArray[i]);
	}

	free(_bigPictureArray);
}

bool Background::load(MfcArchive &file) {
	debug(5, "Background::load()");
	_bgname = file.readPascalString();

	int count = file.readUint16LE();

	for (int i = 0; i < count; i++) {
		PictureObject *pct = new PictureObject();

		pct->load(file, i == 0);
		addPictureObject(pct);
	}

	assert(g_fp->_gameProjectVersion >= 4);

	_bigPictureArray1Count = file.readUint32LE();

	assert(g_fp->_gameProjectVersion >= 5);

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
	if (pct->_okeyCode)
		pct->renumPictures(&_picObjList);

	bool inserted = false;
	for (uint i = 1; i < _picObjList.size(); i++) {
		if (_picObjList[i]->_priority <= pct->_priority) {
			_picObjList.insert_at(i, pct);
			inserted = true;
			break;
		}
	}

	if (!inserted) {
		_picObjList.push_back(pct);
	}
}

PictureObject::PictureObject() {
	_ox = 0;
	_oy = 0;
	_picture = 0;
	_ox2 = 0;
	_oy2 = 0;
	_pictureObject2List = 0;
	_objtype = kObjTypePictureObject;
}

PictureObject::~PictureObject() {
	delete _picture;
	_pictureObject2List->clear();
	delete _pictureObject2List;
}

PictureObject::PictureObject(PictureObject *src) : GameObject(src) {
	_picture = src->_picture;
	_ox2 = _ox;
	_oy2 = _oy;
	_pictureObject2List = src->_pictureObject2List;
	_objtype = kObjTypePictureObject;
}

bool PictureObject::load(MfcArchive &file, bool bigPicture) {
	debug(5, "PictureObject::load()");
	GameObject::load(file);

	if (bigPicture)
		_picture = new BigPicture();
	else
		_picture = new Picture();

	_picture->load(file);

	_pictureObject2List = new Common::Array<GameObject *>;

	int count = file.readUint16LE();

	if (count > 0) {
		GameObject *o = new GameObject();
		
		o->load(file);
		_pictureObject2List->push_back(o);
	}

	_ox2 = _ox;
	_oy2 = _oy;

#if 0
	_picture->displayPicture();
#endif

	return true;
}

Common::Point *PictureObject::getDimensions(Common::Point *p) {
	_picture->getDimensions(p);

	return p;
}

void PictureObject::draw() {
	if (_flags & 1)
		_picture->draw(_ox, _oy, 2, 0);
	else
		_picture->draw(_ox, _oy, 0, 0);
}

void PictureObject::drawAt(int x, int y) {
	if (x == -1)
		x = _ox;
	if (y == -1)
		y = _oy;

	_picture->_x = x;
	_picture->_y = y;

	if (_flags & 1)
		_picture->draw(x, y, 2, 0);
	else
		_picture->draw(x, y, 0, 0);
}

bool PictureObject::setPicAniInfo(PicAniInfo *picAniInfo) {
	if (!(picAniInfo->type & 2) || (picAniInfo->type & 1)) {
		error("PictureObject::setPicAniInfo(): Wrong type: %d", picAniInfo->type);

		return false;
	}

	if (picAniInfo->type & 2) {
		setOXY(picAniInfo->ox, picAniInfo->oy);
		_priority = picAniInfo->priority;
		_okeyCode = picAniInfo->field_8;
		setFlags(picAniInfo->flags);
		_field_8 = picAniInfo->field_24;

		return true;
	}

	return false;
}

bool PictureObject::isPointInside(int x, int y) {
	bool res;
	int oldx = _picture->_x;
	int oldy = _picture->_y;

	_picture->_x = _ox;
	_picture->_y = _oy;

	res = _picture->isPointInside(x, y);

	_picture->_x = oldx;
	_picture->_y = oldy;

	return res;
}

bool PictureObject::isPixelHitAtPos(int x, int y) {
	int oldx = _picture->_x;
	int oldy = _picture->_y;

	_picture->_x = _ox;
	_picture->_y = _oy;
	bool res = _picture->isPixelHitAtPos(x, y);
	_picture->_x = oldx;
	_picture->_y = oldy;

	return res;
}

void PictureObject::setOXY2() {
	_ox2 = _ox;
	_oy2 = _oy;
}

GameObject::GameObject() {
	_okeyCode = 0;
	_flags = 0;
	_id = 0;
	_ox = 0;
	_oy = 0;
	_priority = 0;
	_field_20 = 0;
	_field_8 = 0;
	_objectName = 0;
}

GameObject::GameObject(GameObject *src) {
	_okeyCode = 1;
	_flags = 0;
	_id = src->_id;

	_objectName = (char *)calloc(strlen(src->_objectName) + 1, 1);
	strncpy(_objectName, src->_objectName, strlen(src->_objectName));

	_ox = src->_ox;
	_oy = src->_oy;
	_priority = src->_priority;
	_field_20 = 1;
	_field_8 = src->_field_8;
}

GameObject::~GameObject() {
	free(_objectName);
}

bool GameObject::load(MfcArchive &file) {
	debug(5, "GameObject::load()");
	_okeyCode = 0;
	_flags = 0;
	_field_20 = 0;
	
	_id = file.readUint16LE();
	
	_objectName = file.readPascalString();
	_ox = file.readUint32LE();
	_oy = file.readUint32LE();
	_priority = file.readUint16LE();

	if (g_fp->_gameProjectVersion >= 11) {
		_field_8 = file.readUint32LE();
	}

	return true;
}

void GameObject::setOXY(int x, int y) {
	_ox = x;
	_oy = y;
}

void GameObject::renumPictures(Common::Array<StaticANIObject *> *lst) {
	int *buf = (int *)calloc(lst->size() + 2, sizeof(int));

	for (uint i = 0; i < lst->size(); i++) {
		if (_id == ((GameObject *)((*lst)[i]))->_id)
			buf[((GameObject *)((*lst)[i]))->_okeyCode] = 1;
	}

	if (buf[_okeyCode]) {
		uint count;
		for (count = 1; buf[count] && count < lst->size() + 2; count++)
			;
		_okeyCode = count;
	}

	free(buf);
}

void GameObject::renumPictures(Common::Array<PictureObject *> *lst) {
	int *buf = (int *)calloc(lst->size() + 2, sizeof(int));

	for (uint i = 0; i < lst->size(); i++) {
		if (_id == ((GameObject *)((*lst)[i]))->_id)
			buf[((GameObject *)((*lst)[i]))->_okeyCode] = 1;
	}

	if (buf[_okeyCode]) {
		uint count;
		for (count = 1; buf[count] && count < lst->size() + 2; count++)
			;
		_okeyCode = count;
	}

	free(buf);
}

bool GameObject::getPicAniInfo(PicAniInfo *info) {
	if (_objtype == kObjTypePictureObject) {
		info->type = 2;
		info->objectId = _id;
		info->sceneId = 0;
		info->field_8 = _okeyCode;
		info->flags = _flags;
		info->field_24 = _field_8;
		info->ox = _ox;
		info->oy = _oy;
		info->priority = _priority;
		warning("Yep %d", _id);

		return true;
	}

	if (_objtype == kObjTypeStaticANIObject) {
		StaticANIObject *ani = (StaticANIObject *)this;

		info->type = (ani->_messageQueueId << 16) | 1;
		info->objectId = ani->_id;
		info->field_8 = ani->_okeyCode;
		info->sceneId = ani->_sceneId;
		info->flags = ani->_flags;
		info->field_24 = ani->_field_8;
		if (ani->_movement) {
			info->ox = ani->_movement->_ox;
			info->oy = ani->_movement->_oy;
		} else {
			info->ox = ani->_ox;
			info->oy = ani->_oy;
		}
		info->priority = ani->_priority;

		if (ani->_statics)
			info->staticsId = ani->_statics->_staticsId;

		if (ani->_movement) {
			info->movementId = ani->_movement->_id;
			info->dynamicPhaseIndex = ani->_movement->_currDynamicPhaseIndex;
		}

		info->someDynamicPhaseIndex = ani->_someDynamicPhaseIndex;

		return true;
	}

	return false;
}

bool GameObject::setPicAniInfo(PicAniInfo *picAniInfo) {
	if (!(picAniInfo->type & 3)) {
		warning("StaticANIObject::setPicAniInfo(): Wrong type: %d", picAniInfo->type);

		return false;
	}

	if (picAniInfo->type & 3) {
		setOXY(picAniInfo->ox, picAniInfo->oy);
		_priority = picAniInfo->priority;
		_okeyCode = picAniInfo->field_8;
		setFlags(picAniInfo->flags);
		_field_8 = picAniInfo->field_24;
	}

	if (picAniInfo->type & 1) {
		StaticANIObject *ani = (StaticANIObject *)this;

		ani->_messageQueueId = (picAniInfo->type >> 16) & 0xffff;

		if (picAniInfo->staticsId) {
			ani->_statics = ani->getStaticsById(picAniInfo->staticsId);
		} else {
			ani->_statics = 0;
		}

		if (picAniInfo->movementId) {
			ani->_movement = ani->getMovementById(picAniInfo->movementId);
			if (ani->_movement)
				ani->_movement->setDynamicPhaseIndex(picAniInfo->dynamicPhaseIndex);
		} else {
			ani->_movement = 0;
		}

		ani->setSomeDynamicPhaseIndex(picAniInfo->someDynamicPhaseIndex);
	}

	return true;
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
	_memoryObject2 = 0;
	_width = 0;
	_height = 0;
}

Picture::~Picture() {
	freePicture();

	_bitmap = 0;

	if (_memoryObject2)
		delete _memoryObject2;

	if (_paletteData)
		free(_paletteData);

	if (_convertedBitmap) {
		delete _convertedBitmap;
		_convertedBitmap = 0;
	}
}

void Picture::freePicture() {
	debug(5, "Picture::freePicture(): file: %s", _memfilename);

	if (_bitmap) {
		if (testFlags() && !_field_54) {
			freeData();
			free(_bitmap);
			_bitmap = 0;
		}
	}

	if (_bitmap) {
		_bitmap = 0;
		_data = 0;
	}

	if (_convertedBitmap) {
		free(_convertedBitmap->_pixels);
		delete _convertedBitmap;
		_convertedBitmap = 0;
	}
}

void Picture::freePixelData() {
	freePicture();
	freeData();
}

bool Picture::load(MfcArchive &file) {
	debug(5, "Picture::load()");
	MemoryObject::load(file);

	_x = file.readUint32LE();
	_y = file.readUint32LE();
	_field_44 = file.readUint16LE();
	
	assert(g_fp->_gameProjectVersion >= 2);

	_width = file.readUint32LE();
	_height = file.readUint32LE();

	_mflags |= 1;

	_memoryObject2 = new MemoryObject2;
	_memoryObject2->load(file);

	if (_memoryObject2->_data) {
		setAOIDs();
	}

	assert (g_fp->_gameProjectVersion >= 12);

	_alpha = file.readUint32LE() & 0xff;

	int havePal = file.readUint32LE();

	if (havePal > 0) {
		_paletteData = (byte *)calloc(1024, 1);
		file.read(_paletteData, 1024);
	}

	getData();

	debug(5, "Picture::load: loaded <%s>", _memfilename);

	return true;
}

void Picture::setAOIDs() {
	int w = (g_fp->_pictureScale + _width - 1) / g_fp->_pictureScale;
	int h = (g_fp->_pictureScale + _height - 1) / g_fp->_pictureScale;

	_memoryObject2->_rows = (byte **)malloc(w * sizeof(int *));

	int pitch = 2 * h;
	byte *ptr = _memoryObject2->getData();
	for (int i = 0; i < w; i++) {
		_memoryObject2->_rows[i] = ptr;
		ptr += pitch;
	}
}

void Picture::init() {
	debug(5, "Picture::init(), %s", _memfilename);

	MemoryObject::getData();

	_bitmap = new Bitmap();

	getDibInfo();

	_bitmap->_flags |= 0x1000000;
}

Common::Point *Picture::getDimensions(Common::Point *p) {
	p->x = _width;
	p->y = _height;

	return p;
}

void Picture::getDibInfo() {
	int off = _dataSize & ~0xf;

	debug(9, "Picture::getDibInfo: _dataSize: %d", _dataSize);

	if (!_dataSize) {
		warning("Picture::getDibInfo(): Empty data size");
		return;
	}

	if (_dataSize != off) {
		warning("Uneven data size: 0x%x", _dataSize);
	}

	if (!_data) {
		warning("Picture::getDibInfo: data is empty <%s>", _memfilename);

		MemoryObject::load();
	}

	Common::MemoryReadStream *s = new Common::MemoryReadStream(_data + off - 32, 32);

	_bitmap->load(s);
	_bitmap->_pixels = _data;
}

Bitmap *Picture::getPixelData() {
	if (!_bitmap)
		init();

	return _bitmap;
}

void Picture::draw(int x, int y, int style, int angle) {
	int x1 = x;
	int y1 = y;

	debug(7, "Picture::draw(%d, %d, %d, %d) (%s)", x, y, style, angle, _memfilename);

	if (x != -1)
		x1 = x;

	if (y != -1)
		y1 = y;

	if (!_bitmap)
		init();

	if (!_bitmap)
		return;

	if ((_alpha & 0xff) < 0xff) {
		debug(7, "Picture:draw: alpha = %0x", _alpha);
	}

	byte *pal = _paletteData;

	if (!pal) {
		//warning("Picture:draw: using global palette");
		pal = g_fp->_globalPalette;
	}

	Common::Point point;

	switch (style) {
	case 1:
		//flip
		getDimensions(&point);
		_bitmap->flipVertical()->drawShaded(1, x1, y1 + 30 + point.y, pal);
		break;
	case 2:
		//vrtSetFadeRatio(g_vrtDrawHandle, 0.34999999);
		//vrtSetFadeTable(g_vrtDrawHandle, &unk_477F88, 1.0, 1000.0, 0, 0);
		_bitmap->drawShaded(2, x1, y1, pal);
		//vrtSetFadeRatio(g_vrtDrawHandle, 0.0);
		//vrtSetFadeTable(g_vrtDrawHandle, &unk_477F90, 1.0, 1000.0, 0, 0);
		break;
	default:
		if (angle)
			drawRotated(x1, y1, angle);
		else {
			_bitmap->putDib(x1, y1, (int32 *)pal);
		}
	}
}

void Picture::drawRotated(int x, int y, int angle) {
	warning("STUB: Picture::drawRotated(%d, %d, %d)", x, y, angle);
}

void Picture::displayPicture() {
	if (!g_fp->_gameContinue)
		return;

	getData();
	init();

	if (!_dataSize)
		return;

	g_fp->_backgroundSurface.fillRect(Common::Rect(0, 0, 800, 600), 0);
	g_fp->_system->copyRectToScreen(g_fp->_backgroundSurface.getBasePtr(0, 0), g_fp->_backgroundSurface.pitch, 0, 0, 800, 600);

	draw(0, 0, 0, 0);

	g_fp->updateEvents();
	g_fp->_system->delayMillis(10);
	g_fp->_system->updateScreen();

	while (g_fp->_gameContinue) {
		g_fp->updateEvents();
		g_fp->_system->delayMillis(10);
		g_fp->_system->updateScreen();

		if (g_fp->_keyState == ' ') {
			g_fp->_keyState = Common::KEYCODE_INVALID;
			break;
		}
	}
}

void Picture::setPaletteData(byte *pal) {
	if (_paletteData)
		free(_paletteData);

	if (pal) {
		_paletteData = (byte *)malloc(1024);
		memcpy(_paletteData, pal, 1024);
	}
}

void Picture::copyMemoryObject2(Picture *src) {
	if (_width == src->_width && _height == src->_height) {
		if (src->_memoryObject2 && src->_memoryObject2->_rows && _memoryObject2) {
			byte *data = loadData();
			_memoryObject2->copyData(data, _dataSize);
			setAOIDs();
		}
	}
}

bool Picture::isPointInside(int x, int y) {
	if (x >= _x) {
		if (y >= _y && x < _x + _width && y < _y + _height)
			return true;
	}
	return false;
}

bool Picture::isPixelHitAtPos(int x, int y) {
	if (x < _x || y < _y || x >= _x + _width || y >= _y + _height)
		return false;

	if (!_bitmap)
		init();

	_bitmap->_x = _x;
	_bitmap->_y = _y;

	return _bitmap->isPixelHitAtPos(x, y);
}

int Picture::getPixelAtPos(int x, int y) {
	return getPixelAtPosEx(x / g_fp->_pictureScale, y / g_fp->_pictureScale);

	return false;
}

int Picture::getPixelAtPosEx(int x, int y) {
	if (x < 0 || y < 0)
		return 0;

	if (x < (g_fp->_pictureScale + _width - 1) / g_fp->_pictureScale &&
			y < (g_fp->_pictureScale + _height - 1) / g_fp->_pictureScale &&
			_memoryObject2 != 0 && _memoryObject2->_rows != 0)
		return _memoryObject2->_rows[x][2 * y];

	return 0;
}

bool Bitmap::isPixelHitAtPos(int x, int y) {
	if (x < _x || x >= _width + _x || y < _y || y >= _y + _height)
		return false;

	int off;

	if (_type == 'CB\x05e')
		off = 2 * ((_width + 1) / 2);
	else
		off = 4 * ((_width + 3) / 4);

	off = x + off * (_y + _height - y - 1) - _x;

	if (_flags & 0x1000000) {
		switch (_type) {
		case 'CB\0\0':
			if (_pixels[off] == (_flags & 0xff))
				return false;
			break;
		case 'CB\x05e':
			if (!*(int16 *)&_pixels[2 * off])
				return false;
			break;
		case 'RB\0\0':
			return isPixelAtHitPosRB(x, y);
		}
	}
	return true;
}

bool Bitmap::isPixelAtHitPosRB(int x, int y) {
	int ox = _x;
	int oy = _y;

	_x = _y = 0;

	bool res = putDibRB(0, x, y);
	_x = ox;
	_y = oy;

	return res;
}

void Bitmap::putDib(int x, int y, int32 *palette) {
	debug(7, "Bitmap::putDib(%d, %d)", x, y);

	_x = x - g_fp->_sceneRect.left;
	_y = y - g_fp->_sceneRect.top;

	if (_type == MKTAG('R', 'B', '\0', '\0'))
		putDibRB(palette);
	else
		putDibCB(palette);
}

bool Bitmap::putDibRB(int32 *palette, int pX, int pY) {
	uint16 *curDestPtr;
	int endy;
	int x;
	int start1;
	int fillLen;
	uint16 pixel;
	int endx;
	int y;
	uint16 *srcPtr2;
	uint16 *srcPtr;

	if (!palette && pX == -1) {
		debug(2, "Bitmap::putDibRB(): Both global and local palettes are empty");
		return false;
	}

	debug(8, "Bitmap::putDibRB()");

	endx = _width + _x - 1;
	endy = _height + _y - 1;

	if (_x > 799 || endx < 0 || _y > 599 || endy < 0)
		return false;

	if (pX == -1) {
		if (endy > 599)
			endy = 599;

		if (endx > 799)
			endx = 799;
	}

	int startx = _x;
	if (startx < 0)
		startx = 0;

	int starty = _y;
	if (starty < 0)
		starty = 0;

	y = endy;

	srcPtr = (uint16 *)_pixels;

	bool breakup = false;
	for (y = endy; y >= starty && !breakup; y--) {
		x = startx;

		while ((pixel = *srcPtr++) != 0) {
			if (pixel == 0x100) {
				breakup = true;
				break;
			}

			while (pixel == 0x200 && y >= starty) {
				uint16 value = *srcPtr++;

				x += (byte)(value & 0xff);
				y -= (byte)((value >> 8) & 0xff);

				pixel = *srcPtr++;
			}

			if (y < starty || pixel == 0)
				break;

			start1 = x;
			fillLen = (byte)(pixel & 0xff);

			if (fillLen) {
				x += fillLen;

				if (start1 < 0) {
					fillLen += start1;

					if (fillLen > 0)
						start1 = 0;
				}

				if (fillLen > 0 || start1 >= 0) {
					if (x <= 799 + 1 || (fillLen += 799 - x + 1, fillLen > 0)) {
						if (y <= endy) {
							if (pX == -1) {
								int bgcolor = palette[(pixel >> 8) & 0xff];
								curDestPtr = (uint16 *)g_fp->_backgroundSurface.getBasePtr(start1, y);
								colorFill(curDestPtr, fillLen, bgcolor);
							} else {
								if (y == pY && pX >= start1 && pX < start1 + fillLen)
									return true;
							}
						}
					}
				}
			} else {
				fillLen = (pixel >> 8) & 0xff;
				srcPtr2 = srcPtr;
				x += fillLen;
				srcPtr += (fillLen + 1) >> 1;

				if (start1 < 0) {
					fillLen += start1;
					if (fillLen > 0) {
						srcPtr2 = (uint16 *)((byte *)srcPtr2 - start1);
						start1 = 0;
					}
				}

				if (x > 799 + 1) {
					fillLen += 799 - x + 1;
					if (fillLen <= 0)
						continue;
				}

				if (y <= endy) {
					if (pX == -1) {
						curDestPtr = (uint16 *)g_fp->_backgroundSurface.getBasePtr(start1, y);
						paletteFill(curDestPtr, (byte *)srcPtr2, fillLen, (int32 *)palette);
					} else {
						if (y == pY && pX >= start1 && pX < start1 + fillLen)
							return true;
					}
				}
			}
		}
	}

	if (pX == -1)
		g_fp->_system->copyRectToScreen(g_fp->_backgroundSurface.getBasePtr(startx, starty), g_fp->_backgroundSurface.pitch, startx, starty, endx + 1 - startx, endy + 1 - starty);

	return false;
}

void Bitmap::putDibCB(int32 *palette) {
	uint16 *curDestPtr;
	int endx;
	int endy;
	int bpp;
	uint pitch;
	bool cb05_format;

	endx = _width + _x - 1;
	endy = _height + _y - 1;

	debug(8, "Bitmap::putDibCB(): %d, %d, %d, %d [%d, %d]", _x, _y, endx, endy, _width, _height);

	if (_x > 799 || endx < 0 || _y > 599 || endy < 0)
		return;

	if (endy > 599)
		endy = 599;

	if (endx > 799)
		endx = 799;

	cb05_format = (_type == MKTAG('C', 'B', '\05', 'e'));

	if (!palette && !cb05_format)
		error("Bitmap::putDibCB(): Both global and local palettes are empty");

	bpp = cb05_format ? 2 : 1;
	pitch = (bpp * _width + 3) & 0xFFFFFFFC;

	byte *srcPtr = &_pixels[pitch * (endy - _y)];

	if (endy - _y < _height)
		srcPtr = &_pixels[pitch * (_height - 1)];

	int starty = _y;
	if (starty < 0) {
		starty = 0;
		srcPtr = &_pixels[pitch * (_height + _y)];
	}

	int startx = _x;
	if (startx < 0) {
		srcPtr += bpp * -_x;
		startx = 0;
	}

	if (_flags & 0x1000000) {
		for (int y = starty; y <= endy; srcPtr -= pitch, y++) {
			curDestPtr = (uint16 *)g_fp->_backgroundSurface.getBasePtr(startx, y);
			copierKeyColor(curDestPtr, srcPtr, endx - startx + 1, _flags & 0xff, (int32 *)palette, cb05_format);
		}
	} else {
		for (int y = starty; y <= endy; srcPtr -= pitch, y++) {
			curDestPtr = (uint16 *)g_fp->_backgroundSurface.getBasePtr(startx, y);
			copier(curDestPtr, srcPtr, endx - startx + 1, (int32 *)palette, cb05_format);
		}
	}

	g_fp->_system->copyRectToScreen(g_fp->_backgroundSurface.getBasePtr(startx, starty), g_fp->_backgroundSurface.pitch, startx, starty, endx + 1 - startx, endy + 1 - starty);
}

void Bitmap::colorFill(uint16 *dest, int len, int32 color) {
#if 0
	if (blendMode) {
		if (blendMode != 1)
			error("vrtPutDib : RLE Fill : Invalid alpha blend mode");

		colorFill = ptralphaFillColor16bit;
	} else {
		colorFill = ptrfillColor16bit;
	}
#endif

	for (int i = 0; i < len; i++)
		*dest++ = (int16)(color & 0xffff);
}

void Bitmap::paletteFill(uint16 *dest, byte *src, int len, int32 *palette) {
#if 0
	if (blendMode) {
		if (blendMode != 1)
			error("vrtPutDib : RLE Fill : Invalid alpha blend mode");

		paletteFill = ptrcopierWithPaletteAlpha;
	} else {
		paletteFill = ptrcopierWithPalette;
	}
#endif

	for (int i = 0; i < len; i++)
		*dest++ = READ_LE_UINT32(&palette[*src++]) & 0xffff;
}

void Bitmap::copierKeyColor(uint16 *dest, byte *src, int len, int keyColor, int32 *palette, bool cb05_format) {
#if 0
	if (blendMode) {
		if (blendMode == 1) {
			if (cb05_format)
				copierKeyColor = ptrcopier16bitKeycolorAlpha;
			else
				copierKeyColor = ptrcopierKeycolorAlpha;
		} else {
			copier = 0;
		}
	} else if (cb05_format) {
		copierKeyColor = ptrcopier16bitKeycolor;
	} else {
		copierKeyColor = ptrkeyColor16bit;
	}
#endif

	if (!cb05_format) {
		for (int i = 0; i < len; i++) {
			if (*src != keyColor)
				*dest = READ_LE_UINT32(&palette[*src]) & 0xffff;

			dest++;
			src++;
		}
	} else {
		int16 *src16 = (int16 *)src;

		for (int i = 0; i < len; i++) {
			if (*src16 != 0)
				*dest = *src16;

			dest++;
			src16++;
		}
	}
}

void Bitmap::copier(uint16 *dest, byte *src, int len, int32 *palette, bool cb05_format) {
#if 0
	if (blendMode) {
		if (blendMode == 1) {
			if (cb05_format)
				copier = ptrcopier16bitAlpha;
			else
				copier = ptrcopierWithPaletteAlpha;
		} else {
			copier = 0;
		}
	} else if (cb05_format) {
		copier = ptrcopier16bit;
	} else {
		copier = ptrcopierWithPalette;
	}
#endif

	if (!cb05_format) {
		for (int i = 0; i < len; i++)
			*dest++ = READ_LE_UINT32(&palette[*src++]) & 0xffff;
	} else {
		int16 *src16 = (int16 *)src;

		for (int i = 0; i < len; i++)
			*dest++ = *src16++;
	}
}

Bitmap *Bitmap::reverseImage() {
	switch (_type) {
	case MKTAG('R', 'B', '\0', '\0'):
		return reverseImageRB();
	case MKTAG('C', 'B', '\0', '\0'):
		return reverseImageCB();
	case MKTAG('C', 'B', '\05', 'e'):
		return reverseImageCB05();
	default:
		error("Bitmap::reverseImage: Unknown image type: %x", _type);
	}

	return 0;
}

Bitmap *Bitmap::reverseImageRB() {
	uint16 *newpixels = (uint16 *)calloc(((_dataSize + 15) & 0xfffffff0) + sizeof(Bitmap), 1);
	uint16 *srcPtr = (uint16 *)_pixels;

	int idx = 0;
	while (srcPtr[idx] != 0x100) {
		uint16 *srcPtr2 = &srcPtr[idx];

		int prevIdx = idx;
		int i = idx;

		while (*srcPtr2) {
			++srcPtr2;
			++idx;
		}

		int idx2 = idx;

		newpixels[idx] = srcPtr[idx];

		while (i != idx) {
			int fillLen = 2 - ((srcPtr[prevIdx] & 0xff) != 0 ? 1 : 0);
			idx2 -= fillLen;
			memcpy(&newpixels[idx2], &srcPtr[prevIdx], 2 * fillLen);
			prevIdx = fillLen + i;
			i += fillLen;
		}
		++idx;
	}
	newpixels[idx] = 256;

	int oldBmp = ((_dataSize + 15) >> 1) & 0x7FFFFFF8;
	memcpy(&newpixels[oldBmp], &srcPtr[oldBmp], sizeof(Bitmap));

	Bitmap *res = new Bitmap(this);
	res->_pixels = (byte *)newpixels;

	return res;
}

Bitmap *Bitmap::reverseImageCB() {
	warning("STUB: Bitmap::reverseImageCB()");

	return this;
}

Bitmap *Bitmap::reverseImageCB05() {
	warning("STUB: Bitmap::reverseImageCB05()");

	return this;
}

Bitmap *Bitmap::flipVertical() {
	warning("STUB: Bitmap::flipVertical()");

	return this;
}

void Bitmap::drawShaded(int type, int x, int y, byte *palette) {
	warning("STUB: Bitmap::drawShaded(%d, %d, %d)", type, x, y);

	putDib(x, y, (int32 *)palette);
}

	void Bitmap::drawRotated(int x, int y, int angle, byte *palette) {
	warning("STUB: Bitmap::drawShaded(%d, %d, %d)", x, y, angle);

	putDib(x, y, (int32 *)palette);
}

bool BigPicture::load(MfcArchive &file) {
	debug(5, "BigPicture::load()");
	Picture::load(file);

	return true;
}

void BigPicture::draw(int x, int y, int style, int angle) {
	if (!_bitmap)
		init();

	if (_bitmap) {
		_bitmap->_flags &= 0xFEFFFFFF;

		int nx = _x;
		int ny = _y;

		if (x != -1)
			nx = x;

		if (y != -1)
			ny = y;

		if (_alpha < 0xFF) {
			//vrtSetAlphaBlendMode(g_vrtDrawHandle, 1, v9);
		}

		_bitmap->putDib(nx, ny, 0);

		if (_alpha < 0xFF) {
			//vrtSetAlphaBlendMode(g_vrtDrawHandle, 0, 255);
		}
	}
}

Shadows::Shadows() {
	_staticAniObjectId = 0;
	_movementId = 0;
	_sceneId = 0;
}

bool Shadows::load(MfcArchive &file) {
	debug(5, "Shadows::load()");
	_sceneId = file.readUint32LE();
	_staticAniObjectId = file.readUint32LE();
	_movementId = file.readUint32LE();

	return true;
}

void Shadows::init() {
	Scene *scene = g_fp->accessScene(_sceneId);

	StaticANIObject *st;
	Movement *mov;

	if (scene && (st = scene->getStaticANIObject1ById(_staticAniObjectId, -1)) != 0
		&& ((mov = st->getMovementById(_movementId)) != 0))
		initMovement(mov);
}

void Shadows::initMovement(Movement *mov) {
	uint num;

	if (mov->_currMovement)
		num = mov->_currMovement->_dynamicPhases.size();
	else
		num = mov->_dynamicPhases.size();

	_items.clear();
	_items.resize(num);

	Common::Point point;

	_items[0].dynPhase = (DynamicPhase *)mov->_staticsObj1;
	_items[0].dynPhase->getDimensions(&point);
	_items[0].width = point.x;
	_items[0].height = point.y;

	for (uint i = 1; i < num; i++) {
		_items[i].dynPhase = mov->getDynamicPhaseByIndex(i - 1);
		_items[i].dynPhase->getDimensions(&point);
		_items[i].width = point.x;
		_items[i].height = point.y;
	}
}

DynamicPhase *Shadows::findSize(int width, int height) {
	int idx = 0;
	int min = 1000;

	if (!_items.size())
		return 0;

	for (uint i = 0; i < _items.size(); i++) {
		int w = abs(width - _items[i].width);
		if (w < min) {
			min = w;
			idx = i;
		}
	}
	return _items[idx].dynPhase;
}

void FullpipeEngine::drawAlphaRectangle(int x1, int y1, int x2, int y2, int alpha) {
	warning("STUB: FullpipeEngine::drawAlphaRectangle()");
}

} // End of namespace Fullpipe
