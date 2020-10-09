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

#include "ngi/ngi.h"

#include "ngi/objects.h"
#include "ngi/gfx.h"
#include "ngi/statics.h"
#include "ngi/scene.h"
#include "ngi/interaction.h"
#include "ngi/gameloader.h"

#include "common/memstream.h"
#include "graphics/transparent_surface.h"

namespace NGI {

Background::Background() {
	_x = 0;
	_y = 0;
	_messageQueueId = 0;
}

Background::~Background() {
	Common::for_each(_picObjList.begin(), _picObjList.end(), Common::DefaultDeleter<PictureObject>());
	Common::for_each(_bigPictureArray.begin(), _bigPictureArray.end(), Common::DefaultDeleter<BigPicture>());
}

bool Background::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "Background::load()");
	_bgname = file.readPascalString();

	int count = file.readUint16LE();

	for (int i = 0; i < count; i++) {
		PictureObject *pct = new PictureObject();

		pct->load(file, i == 0);
		addPictureObject(pct);
	}

	assert(g_nmi->_gameProjectVersion >= 5);

	_bigPictureXDim = file.readUint32LE();
	_bigPictureYDim = file.readUint32LE();

	debugC(6, kDebugLoading, "bigPictureArray[%d][%d]", _bigPictureXDim, _bigPictureYDim);

	for (uint i = 0; i < _bigPictureXDim * _bigPictureYDim; ++i) {
		_bigPictureArray.push_back(new BigPicture());
		_bigPictureArray[i]->load(file);
	}

	return true;
}

void Background::addPictureObject(PictureObject *pct) {
	if (pct->_odelay)
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
	_ox2 = 0;
	_oy2 = 0;
	_objtype = kObjTypePictureObject;
	_picture = nullptr;
}

PictureObject::PictureObject(PictureObject *src) : GameObject(src) {
	_picture = src->_picture;
	_ox2 = _ox;
	_oy2 = _oy;
	_pictureObject2List = src->_pictureObject2List;
	_objtype = kObjTypePictureObject;
}

bool PictureObject::load(MfcArchive &file, bool bigPicture) {
	debugC(5, kDebugLoading, "PictureObject::load()");
	GameObject::load(file);

	if (bigPicture)
		_picture = new BigPicture();
	else
		_picture = new Picture();

	_picture->load(file);

	int count = file.readUint16LE();

	if (count > 0) {
		_pictureObject2List.push_back(GameObject());
		_pictureObject2List.back().load(file);
	}

	_ox2 = _ox;
	_oy2 = _oy;

#if 0
	_picture->displayPicture();
#endif

	return true;
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

bool PictureObject::setPicAniInfo(const PicAniInfo &picAniInfo) {
	if (!(picAniInfo.type & 2) || (picAniInfo.type & 1)) {
		error("PictureObject::setPicAniInfo(): Wrong type: %d", picAniInfo.type);
	}

	if (picAniInfo.type & 2) {
		setOXY(picAniInfo.ox, picAniInfo.oy);
		_priority = picAniInfo.priority;
		_odelay = picAniInfo.field_8;
		setFlags(picAniInfo.flags);
		_field_8 = picAniInfo.field_24;

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
	_odelay = 0;
	_flags = 0;
	_id = 0;
	_ox = 0;
	_oy = 0;
	_priority = 0;
	_field_20 = 0;
	_field_8 = 0;
}

GameObject::GameObject(GameObject *src) {
	_odelay = 1;
	_flags = 0;
	_id = src->_id;

	_objectName = src->_objectName;

	_ox = src->_ox;
	_oy = src->_oy;
	_priority = src->_priority;
	_field_20 = 1;
	_field_8 = src->_field_8;
}

Common::String GameObject::toXML() {
	return Common::String::format("id=\"%s\" name=\"%s\" x=%d y=%d priority=%d f8=%d",
		g_nmi->gameIdToStr(_id).c_str(), transCyrillic(_objectName), _ox, _oy, _priority, _field_8);
}

bool GameObject::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "GameObject::load()");
	_odelay = 0;
	_flags = 0;
	_field_20 = 0;

	_id = file.readUint16LE();

	_objectName = file.readPascalString();
	_ox = file.readSint32LE();
	_oy = file.readSint32LE();
	_priority = file.readUint16LE();

	if (g_nmi->_gameProjectVersion >= 11) {
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
		if (_id == (*lst)[i]->_id)
			buf[(*lst)[i]->_odelay] = 1;
	}

	if (buf[_odelay]) {
		uint count;
		for (count = 1; buf[count] && count < lst->size() + 2; count++)
			;
		_odelay = count;
	}

	free(buf);
}

void GameObject::renumPictures(Common::Array<PictureObject *> *lst) {
	int *buf = (int *)calloc(lst->size() + 2, sizeof(int));

	for (uint i = 0; i < lst->size(); i++) {
		if (_id == (*lst)[i]->_id)
			buf[(*lst)[i]->_odelay] = 1;
	}

	if (buf[_odelay]) {
		uint count;
		for (count = 1; buf[count] && count < lst->size() + 2; count++)
			;
		_odelay = count;
	}

	free(buf);
}

bool GameObject::getPicAniInfo(PicAniInfo &info) {
	if (_objtype == kObjTypePictureObject) {
		info.type = 2;
		info.objectId = _id;
		info.sceneId = 0;
		info.field_8 = _odelay;
		info.flags = _flags;
		info.field_24 = _field_8;
		info.ox = _ox;
		info.oy = _oy;
		info.priority = _priority;

		return true;
	}

	if (_objtype == kObjTypeStaticANIObject) {
		StaticANIObject *ani = static_cast<StaticANIObject *>(this);

		info.type = (ani->_messageQueueId << 16) | 1;
		info.objectId = ani->_id;
		info.field_8 = ani->_odelay;
		info.sceneId = ani->_sceneId;
		info.flags = ani->_flags;
		info.field_24 = ani->_field_8;
		if (ani->_movement) {
			info.ox = ani->_movement->_ox;
			info.oy = ani->_movement->_oy;
		} else {
			info.ox = ani->_ox;
			info.oy = ani->_oy;
		}
		info.priority = ani->_priority;

		if (ani->_statics)
			info.staticsId = ani->_statics->_staticsId;

		if (ani->_movement) {
			info.movementId = ani->_movement->_id;
			info.dynamicPhaseIndex = ani->_movement->_currDynamicPhaseIndex;
		}

		info.someDynamicPhaseIndex = ani->_someDynamicPhaseIndex;

		return true;
	}

	return false;
}

bool GameObject::setPicAniInfo(const PicAniInfo &picAniInfo) {
	if (!(picAniInfo.type & 3)) {
		warning("StaticANIObject::setPicAniInfo(): Wrong type: %d", picAniInfo.type);

		return false;
	}

	if (picAniInfo.type & 2) {
		setOXY(picAniInfo.ox, picAniInfo.oy);
		_priority = picAniInfo.priority;
		_odelay = picAniInfo.field_8;
		setFlags(picAniInfo.flags);
		_field_8 = picAniInfo.field_24;

		return true;
	}

	if (picAniInfo.type & 1 && _objtype == kObjTypeStaticANIObject) {
		StaticANIObject *ani = static_cast<StaticANIObject *>(this);

		ani->_messageQueueId = (picAniInfo.type >> 16) & 0xffff;
		ani->_odelay = picAniInfo.field_8;
		ani->setFlags(picAniInfo.flags);
		ani->_field_8 = picAniInfo.field_24;

		if (picAniInfo.staticsId) {
			ani->_statics = ani->getStaticsById(picAniInfo.staticsId);
		} else {
			ani->_statics = 0;
		}

		if (picAniInfo.movementId) {
			ani->_movement = ani->getMovementById(picAniInfo.movementId);
			if (ani->_movement)
				ani->_movement->setDynamicPhaseIndex(picAniInfo.dynamicPhaseIndex);
		} else {
			ani->_movement = 0;
		}

		ani->setOXY(picAniInfo.ox, picAniInfo.oy);
		ani->_priority = picAniInfo.priority;

		ani->setSomeDynamicPhaseIndex(picAniInfo.someDynamicPhaseIndex);

		return true;
	}

	return false;
}

Picture::Picture() :
	_x(0),
	_y(0),
	_field_44(0),
	_field_54(0),
	_alpha(-1),
	_width(0),
	_height(0) {}

Picture::~Picture() {
	freePicture();
}

void Picture::freePicture() {
	debugC(5, kDebugMemory, "Picture::freePicture(): file: %s", _memfilename.c_str());

	if (_bitmap) {
		if (testFlags() && !_field_54) {
			freeData();
		}
		_bitmap.reset();
		_data = nullptr;
	}

	_convertedBitmap.reset();
}

void Picture::freePixelData() {
	freePicture();
	freeData();
}

bool Picture::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "Picture::load()");
	MemoryObject::load(file);

	_x = file.readSint32LE();
	_y = file.readSint32LE();
	_field_44 = file.readUint16LE();

	assert(g_nmi->_gameProjectVersion >= 2);

	_width = file.readUint32LE();
	_height = file.readUint32LE();

	debug(7, "Picture::load: x: %d y: %d, w: %d, h: %d", _x, _y, _width, _height);

	_mflags |= 1;

	_memoryObject2.reset(new MemoryObject2);
	_memoryObject2->load(file);

	if (_memoryObject2->_data) {
		setAOIDs();
	}

	assert(g_nmi->_gameProjectVersion >= 12);

	_alpha = file.readUint32LE() & 0xff;

	int havePal = file.readUint32LE();

	if (havePal > 0) {
		for (int i = 0; i < 256; ++i) {
			_paletteData.pal[i] = file.readUint32LE();
		}
		_paletteData.size = 256;
	}

	getData();

	debugC(5, kDebugLoading, "Picture::load: loaded memobject=\"%s\" x=%d y=%d f44=%d width=%d height=%d alpha=%d memobject2=\"%s\"", _memfilename.c_str(),
				_x, _y, _field_44, _width, _height, _alpha, _memoryObject2->_memfilename.c_str());

	return true;
}

void Picture::setAOIDs() {
	int w = (g_nmi->_pictureScale + _width - 1) / g_nmi->_pictureScale;
	int h = (g_nmi->_pictureScale + _height - 1) / g_nmi->_pictureScale;

	_memoryObject2->_rows = (byte **)malloc(w * sizeof(int *));

	int pitch = 2 * h;
	byte *ptr = _memoryObject2->getData();
	for (int i = 0; i < w; i++) {
		_memoryObject2->_rows[i] = ptr;
		ptr += pitch;
	}
}

void Picture::init() {
	debugC(5, kDebugLoading, "Picture::init(), %s", _memfilename.c_str());

	MemoryObject::getData();

	_bitmap.reset(new Bitmap());

	getDibInfo();

	_bitmap->_flags |= 0x1000000;
}

void Picture::getDibInfo() {
	int off = _dataSize & ~0xf;

	debugC(9, kDebugLoading, "Picture::getDibInfo: _dataSize: %d", _dataSize);

	if (!_dataSize) {
		warning("Picture::getDibInfo(): Empty data size");
		return;
	}

	if (_dataSize != off) {
		warning("Uneven data size: 0x%x", _dataSize);
	}

	if (!_data) {
		warning("Picture::getDibInfo: data is empty <%s>", _memfilename.c_str());

		MemoryObject::load();

		if (!_data)
			error("Picture::getDibInfo: error loading object #%d", _cnum);
	}

	Common::MemoryReadStream *s = new Common::MemoryReadStream(_data + off - 32, 32);
	_bitmap->load(s);
	delete s;

	_bitmap->decode(_data, _paletteData.size ? _paletteData : *g_nmi->_globalPalette);
}

const Bitmap *Picture::getPixelData() {
	if (!_bitmap)
		init();

	return _bitmap.get();
}

void Picture::draw(int x, int y, int style, int angle) {
	int x1 = x;
	int y1 = y;

	debugC(7, kDebugDrawing, "Picture::draw(%d, %d, %d, %d) (%s)", x, y, style, angle, _memfilename.c_str());

	if (x != -1)
		x1 = x;

	if (y != -1)
		y1 = y;

	if (!_bitmap)
		init();

	if (!_bitmap)
		return;

	if ((_alpha & 0xff) < 0xff) {
		debugC(7, kDebugDrawing, "Picture:draw: alpha = %0x", _alpha);
	}

	const Palette *pal;
	if (_paletteData.size) {
		pal = &_paletteData;
	} else {
		//warning("Picture:draw: using global palette");
		pal = g_nmi->_globalPalette;
	}

	switch (style) {
	case 1: {
		//flip
		const Dims dims = getDimensions();
		_bitmap->flipVertical()->drawShaded(1, x1, y1 + 30 + dims.y, *pal, _alpha);
		break;
	}
	case 2:
		//vrtSetFadeRatio(g_vrtDrawHandle, 0.34999999);
		//vrtSetFadeTable(g_vrtDrawHandle, &unk_477F88, 1.0, 1000.0, 0, 0);
		_bitmap->drawShaded(2, x1, y1, *pal, _alpha);
		//vrtSetFadeRatio(g_vrtDrawHandle, 0.0);
		//vrtSetFadeTable(g_vrtDrawHandle, &unk_477F90, 1.0, 1000.0, 0, 0);
		break;
	default:
		if (angle)
			drawRotated(x1, y1, angle);
		else {
			_bitmap->putDib(x1, y1, *pal, _alpha);
		}
	}
}

void Picture::drawRotated(int x, int y, int angle) {
	warning("STUB: Picture::drawRotated(%d, %d, %d)", x, y, angle);
}

void Picture::displayPicture() {
	if (g_nmi->shouldQuit())
		return;

	getData();
	init();

	if (!_dataSize)
		return;

	g_nmi->_backgroundSurface.fillRect(Common::Rect(0, 0, 800, 600), 0);
	g_nmi->_system->copyRectToScreen(g_nmi->_backgroundSurface.getBasePtr(0, 0), g_nmi->_backgroundSurface.pitch, 0, 0, 800, 600);

	draw(0, 0, 0, 0);

	g_nmi->updateEvents();
	g_nmi->_system->delayMillis(10);
	g_nmi->_system->updateScreen();

	while (!g_nmi->shouldQuit()) {
		g_nmi->updateEvents();
		g_nmi->_system->delayMillis(10);
		g_nmi->_system->updateScreen();

		if (g_nmi->_keyState == ' ') {
			g_nmi->_keyState = Common::KEYCODE_INVALID;
			break;
		}
	}
}

void Picture::setPaletteData(const Palette &pal) {
	if (pal.size) {
		_paletteData.copy(pal);
	} else {
		_paletteData.size = 0;
	}
}

void Picture::copyMemoryObject2(Picture &src) {
	if (_width == src._width && _height == src._height) {
		if (src._memoryObject2 && src._memoryObject2->_rows && _memoryObject2) {
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
	return getPixelAtPosEx(x / g_nmi->_pictureScale, y / g_nmi->_pictureScale);

	return false;
}

int Picture::getPixelAtPosEx(int x, int y) {
	if (x < 0 || y < 0)
		return 0;

	warning("STUB: Picture::getPixelAtPosEx(%d, %d)", x, y);

	// TODO: It looks like this doesn't really work.
	if (x < (g_nmi->_pictureScale + _width - 1) / g_nmi->_pictureScale &&
			y < (g_nmi->_pictureScale + _height - 1) / g_nmi->_pictureScale &&
			_memoryObject2 != 0 && _memoryObject2->_rows != 0)
		return _memoryObject2->_rows[x][2 * y];

	return 0;
}

Bitmap::Bitmap() {
	_x = 0;
	_y = 0;
	_width = 0;
	_height = 0;
	_type = 0;
	_dataSize = 0;
	_flags = 0;
	_flipping = Graphics::FLIP_NONE;
	_surface = nullptr;
}

Bitmap::Bitmap(const Bitmap &src) {
	_x = src._x;
	_y = src._y;
	_flags = src._flags;
	_dataSize = src._dataSize;
	_type = src._type;
	_width = src._width;
	_height = src._height;
	_surface = src._surface;
	_flipping = src._flipping;
	_surface = src._surface;
}

Bitmap::~Bitmap() {
	// TODO: This is a hack because Graphics::Surface has terrible resource
	// management
	//_surface->free();
}

void Bitmap::load(Common::ReadStream *s) {
	debugC(5, kDebugLoading, "Bitmap::load()");

	_x = s->readSint32LE();
	_y = s->readSint32LE();
	_width = s->readUint32LE();
	_height = s->readUint32LE();
	s->readUint32LE(); // pixels
	_type = s->readUint32LE();
	_dataSize = s->readUint32LE();
	_flags = s->readUint32LE();

	debugC(8, kDebugLoading, "Bitmap: x: %d y: %d w: %d h: %d dataSize: 0x%x", _x, _y, _width, _height, _dataSize);
	debugC(8, kDebugLoading, "Bitmap: type: %s (0x%04x) flags: 0x%x", Common::tag2string(_type).c_str(), _type, _flags);
}

bool Bitmap::isPixelHitAtPos(int x, int y) {
	if (x < _x || x >= _width + _x || y < _y || y >= _y + _height)
		return false;

	if (!_surface)
		return false;

	return ((*((int32 *)_surface->getBasePtr(x - _x, y - _y)) & 0xff) != 0);
}

void Bitmap::decode(byte *pixels, const Palette &palette) {
	_surface = new Graphics::TransparentSurface, Graphics::SurfaceDeleter();
	_surface->create(_width, _height, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));

	if (_type == MKTAG('R', 'B', '\0', '\0'))
		putDibRB(pixels, palette);
	else
		putDibCB(pixels, palette);
}

void Bitmap::putDib(int x, int y, const Palette &palette, byte alpha) {
	debugC(7, kDebugDrawing, "Bitmap::putDib(%d, %d)", x, y);

	int x1 = x - g_nmi->_sceneRect.left;
	int y1 = y - g_nmi->_sceneRect.top;

	if (!_width || !_height || !_surface)
		return;

	Common::Rect sub(0, 0, _width, _height);
	sub.translate(x, y);
	sub.clip(g_nmi->_sceneRect);
	sub.translate(-x, -y);

	if (sub.isEmpty())
		return;

	if (x1 < 0)
		x1 = 0;

	if (y1 < 0)
		y1 = 0;

	int alphac = TS_ARGB(0xff, alpha, 0xff, 0xff);

	_surface->blit(g_nmi->_backgroundSurface, x1, y1, _flipping, &sub, alphac);
	g_nmi->_system->copyRectToScreen(g_nmi->_backgroundSurface.getBasePtr(x1, y1), g_nmi->_backgroundSurface.pitch, x1, y1, sub.width(), sub.height());
}

bool Bitmap::putDibRB(byte *pixels, const Palette &palette) {
	uint32 *curDestPtr;
	int endy;
	int x;
	int start1;
	int fillLen;
	uint16 pixel;
	int y;
	uint16 *srcPtr2;
	uint16 *srcPtr;

	if (!palette.size) {
		debugC(2, kDebugDrawing, "Bitmap::putDibRB(): Both global and local palettes are empty");
		return false;
	}

	debugC(8, kDebugDrawing, "Bitmap::putDibRB()");

	endy = _height - 1;

	int startx = 0;
	int starty = 0;

	y = endy;

	srcPtr = (uint16 *)pixels;

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
					if (x <= _width + 1 || (fillLen += _width - x + 1, fillLen > 0)) {
						if (y <= endy) {
							int bgcolor = palette.pal[(pixel >> 8) & 0xff];
							curDestPtr = (uint32 *)_surface->getBasePtr(start1, y);
							fillLen = MIN(_width - start1, fillLen);
							colorFill(curDestPtr, fillLen, bgcolor);
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

				if (x > _width) {
					fillLen += _width - x;
					if (fillLen <= 0)
						continue;
				}

				if (y <= endy) {
					curDestPtr = (uint32 *)_surface->getBasePtr(start1, y);
					fillLen = MIN(_width - start1, fillLen);
					paletteFill(curDestPtr, (byte *)srcPtr2, fillLen, palette);
				}
			}
		}
	}

	return false;
}

void Bitmap::putDibCB(byte *pixels, const Palette &palette) {
	uint32 *curDestPtr;
	int endx;
	int endy;
	int bpp;
	uint pitch;
	bool cb05_format;

	endx = _width - 1;
	endy = _height - 1;

	cb05_format = (_type == MKTAG('C', 'B', '\05', 'e'));

	if (!palette.size && !cb05_format)
		error("Bitmap::putDibCB(): Both global and local palettes are empty");

	bpp = cb05_format ? 2 : 1;
	pitch = (bpp * _width + 3) & 0xFFFFFFFC;

	byte *srcPtr = &pixels[pitch * endy];

	if (endy < _height)
		srcPtr = &pixels[pitch * (_height - 1)];

	int starty = 0;
	int startx = 0;

	if (_flags & 0x1000000) {
		for (int y = starty; y <= endy; srcPtr -= pitch, y++) {
			curDestPtr = (uint32 *)_surface->getBasePtr(startx, y);
			copierKeyColor(curDestPtr, srcPtr, endx - startx + 1, _flags & 0xff, palette, cb05_format);
		}
	} else {
		for (int y = starty; y <= endy; srcPtr -= pitch, y++) {
			curDestPtr = (uint32 *)_surface->getBasePtr(startx, y);
			copier(curDestPtr, srcPtr, endx - startx + 1, palette, cb05_format);
		}
	}
}

void Bitmap::colorFill(uint32 *dest, int len, int32 color) {
#if 0
	if (blendMode) {
		if (blendMode != 1)
			error("vrtPutDib : RLE Fill : Invalid alpha blend mode");

		colorFill = ptralphaFillColor16bit;
	} else {
		colorFill = ptrfillColor16bit;
	}
#endif
	byte r, g, b;

	g_nmi->_origFormat.colorToRGB(color, r, g, b);

	uint32 c = TS_ARGB(0xff, r, g, b);

	for (int i = 0; i < len; i++)
		*dest++ = c;
}

void Bitmap::paletteFill(uint32 *dest, byte *src, int len, const Palette &palette) {
#if 0
	if (blendMode) {
		if (blendMode != 1)
			error("vrtPutDib : RLE Fill : Invalid alpha blend mode");

		paletteFill = ptrcopierWithPaletteAlpha;
	} else {
		paletteFill = ptrcopierWithPalette;
	}
#endif

	byte r, g, b;

	for (int i = 0; i < len; i++) {
		g_nmi->_origFormat.colorToRGB(palette.pal[*src++] & 0xffff, r, g, b);

		*dest++ = TS_ARGB(0xff, r, g, b);
	}
}

void Bitmap::copierKeyColor(uint32 *dest, byte *src, int len, int keyColor, const Palette &palette, bool cb05_format) {
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

	byte r, g, b;

	if (!cb05_format) {
		for (int i = 0; i < len; i++) {
			if (*src != keyColor) {
				g_nmi->_origFormat.colorToRGB(palette.pal[*src] & 0xffff, r, g, b);
				*dest = TS_ARGB(0xff, r, g, b);
			}

			dest++;
			src++;
		}
	} else {
		int16 *src16 = (int16 *)src;

		for (int i = 0; i < len; i++) {
			if (*src16 != 0) {
				g_nmi->_origFormat.colorToRGB(READ_LE_UINT16(src16), r, g, b);
				*dest = TS_ARGB(0xff, r, g, b);
			}

			dest++;
			src16++;
		}
	}
}

void Bitmap::copier(uint32 *dest, byte *src, int len, const Palette &palette, bool cb05_format) {
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

	byte r, g, b;

	if (!cb05_format) {
		for (int i = 0; i < len; i++) {
			g_nmi->_origFormat.colorToRGB(palette.pal[*src++] & 0xffff, r, g, b);

			*dest++ = TS_ARGB(0xff, r, g, b);
		}
	} else {
		int16 *src16 = (int16 *)src;

		for (int i = 0; i < len; i++) {
			g_nmi->_origFormat.colorToRGB(READ_LE_UINT16(src16++), r, g, b);
			*dest++ = TS_ARGB(0xff, r, g, b);
		}
	}
}

Bitmap *Bitmap::reverseImage(bool flip) const {
	Bitmap *b = new Bitmap(*this);

	if (flip)
		b->_flipping ^= Graphics::FLIP_H;

	return b;
}

Bitmap *Bitmap::flipVertical() const {
	Bitmap *b = new Bitmap(*this);

	b->_flipping ^= Graphics::FLIP_V;

	return b;
}

void Bitmap::drawShaded(int type, int x, int y, const Palette &palette, int alpha) {
	if (alpha != 255)
		warning("STUB: Bitmap::drawShaded(%d, %d, %d, %d)", type, x, y, alpha);

	putDib(x, y, palette, alpha);
}

void Bitmap::drawRotated(int x, int y, int angle, const Palette &palette, int alpha) {
	warning("STUB: Bitmap::drawRotated(%d, %d, %d, %d)", x, y, angle, alpha);

	putDib(x, y, palette, alpha);
}

bool BigPicture::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "BigPicture::load()");
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

		_bitmap->putDib(nx, ny, Palette(), _alpha);
	}
}

Shadows::Shadows() {
	_staticAniObjectId = 0;
	_movementId = 0;
	_sceneId = 0;
}

bool Shadows::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "Shadows::load()");
	_sceneId = file.readUint32LE();
	_staticAniObjectId = file.readUint32LE();
	_movementId = file.readUint32LE();

	return true;
}

void Shadows::init() {
	Scene *scene = g_nmi->accessScene(_sceneId);

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

	_items[0].dynPhase = mov->_staticsObj1;
	Dims dims = _items[0].dynPhase->getDimensions();
	_items[0].width = dims.x;
	_items[0].height = dims.y;

	for (uint i = 1; i < num; i++) {
		_items[i].dynPhase = mov->getDynamicPhaseByIndex(i - 1);
		dims = _items[i].dynPhase->getDimensions();
		_items[i].width = dims.x;
		_items[i].height = dims.y;
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

void NGIEngine::drawAlphaRectangle(int x1, int y1, int x2, int y2, int alpha) {
	for (int y = y1; y < y2; y++) {
		uint32 *ptr = (uint32 *)g_nmi->_backgroundSurface.getBasePtr(x1, y);

		for (int x = x1; x < x2; x++) {
			uint32 color = *ptr;
			color = (((color >> 24) & 0xff) * alpha / 0xff) << 24 |
					(((color >> 16) & 0xff) * alpha / 0xff) << 16 |
					(((color >>  8) & 0xff) * alpha / 0xff) <<  8 |
					(color & 0xff);
			*ptr = color;
			ptr++;
		}
	}
}

void NGIEngine::sceneFade(Scene *sc, bool direction) {
	for (int dim = 0; dim < 255; dim += 20) {
		int ticks = g_nmi->_system->getMillis();
		sc->draw();

		drawAlphaRectangle(0, 0, g_nmi->_backgroundSurface.w, g_nmi->_backgroundSurface.h, direction ? dim : 255 - dim);
		g_nmi->_system->copyRectToScreen(g_nmi->_backgroundSurface.getBasePtr(0, 0), g_nmi->_backgroundSurface.pitch, 0, 0, 800, 600);

		g_nmi->_system->updateScreen();
		ticks = g_nmi->_system->getMillis() - ticks;

		if (ticks < 42)
			g_nmi->_system->delayMillis(42 - ticks);
	}
}

} // End of namespace NGI
