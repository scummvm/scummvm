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

#include "common/memstream.h"

namespace Fullpipe {

void Bitmap::load(Common::ReadStream *s) {
	debug(5, "Bitmap::load()");

	_x = s->readUint32LE();
	_y = s->readUint32LE();
	_width = s->readUint32LE();
	_height = s->readUint32LE();
	s->readUint32LE(); // pixels
	_type = s->readUint32LE();
	_field_18 = s->readUint32LE();
	_flags = s->readUint32LE();

	debug(8, "Bitmap: x: %d y: %d w: %d h: %d field_18: 0x%x", _x, _y, _width, _height, _field_18);
	debug(8, "Bitmap: type: %s (0x%04x) flags: 0x%x", Common::tag2string(_type).c_str(), _type, _flags);
}

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
	debug(5, "Background::load()");
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
	debug(5, "PictureObject::load()");
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
	debug(5, "GameObject::load()");
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
	debug(5, "Picture::load()");
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

	debug(5, "Picture::load: <%s>", _memfilename);

	displayPicture();

	return true;
}

void Picture::setAOIDs() {
	int w = (g_fullpipe->_pictureScale + _width - 1) / g_fullpipe->_pictureScale;
	int h = (g_fullpipe->_pictureScale + _height - 1) / g_fullpipe->_pictureScale;

	_memoryObject2->_rows = (byte **)malloc(w * sizeof(int *));

	int pitch = 2 * h;
	byte *ptr = _memoryObject2->getData();
	for (int i = 0; i < w; i++) {
		_memoryObject2->_rows[i] = ptr;
		ptr += pitch;
	}
}

void Picture::init() {
	_bitmap = new Bitmap();

	getDibInfo();

	_bitmap->_flags |= 0x1000000;
}

void Picture::getDibInfo() {
	int off = _dataSize & ~0xf;

	debug(0, "Picture::getDibInfo: _dataSize: %d", _dataSize);

	if (!_dataSize) {
		warning("Picture::getDibInfo(): Empty data size");
		return;
	}

	if (_dataSize != off) {
		warning("Uneven data size: 0x%x", _dataSize);
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

	if (x != -1)
		x1 = x;

	if (y != -1)
		y1 = y;

	if (!_bitmap)
		init();

	if (!_bitmap)
		return;

	if (_alpha < 0xff) {
		warning("Picture:draw: alpha = %0x", _alpha);
	}

	if (_bitmap->_type == MKTAG('C', 'B', '\0', '\0') || _bitmap->_type == MKTAG('R', 'B', '\0', '\0')) {
		if (_paletteData) {
			warning("Picture:draw: have palette");
		}
	}

	switch (style) {
	case 1:
		//flip
		warning("Picture::draw: style 1");
		break;
	case 2:
		error("Picture::draw: style 2");
		break;
	default:
		if (angle) {
			warning("Picture:draw: angle = %d", angle);
			drawRotated(x1, y1, angle);
		} else {
			_bitmap->putDib(x1, y1, _paletteData);
		}
	}
}

void Picture::drawRotated(int x, int y, int angle) {
}

void Picture::displayPicture() {
	getData();
	init();

	g_fullpipe->_backgroundSurface.fillRect(Common::Rect(0, 0, 799, 599), 0);
	g_fullpipe->_system->copyRectToScreen(g_fullpipe->_backgroundSurface.getBasePtr(0, 0), g_fullpipe->_backgroundSurface.pitch, 0, 0, 799, 599);

	draw(0, 0, 0, 0);

	while (1) {
		g_fullpipe->updateEvents();
		g_fullpipe->_system->delayMillis(10);
		g_fullpipe->_system->updateScreen();

		if (g_fullpipe->_keyState == ' ') {
			g_fullpipe->_keyState = Common::KEYCODE_INVALID;
			break;
		}
	}
}

void Bitmap::putDib(int x, int y, byte *palette) {
	int16 *curDestPtr;
	int endy;
	int pos;
	byte *srcPtr;
	uint pixel;
	int start1;
	int fillValue;
	int pixoffset;
	int end2;
	int pixelHigh;
	int pixoffset1;
	int leftx;
	uint pixel1;
	uint pixel1High;
	int bpp;
	uint pitch;
	byte *srcPtr1;
	int end;
	int endx;
	int endy1;
	bool cb05_format;
	byte *pixPtr;
	byte *srcPtr2;
	int start;

	endx = _width + _x - 1;
	endy = _height + _y - 1;

	if (_x > 799 || _width + _x - 1 < 0 || _y > 599 || endy < 0)
		return;

	if (endy > 599)
		endy = 599;

	if (_type == MKTAG('R', 'B', '\0', '\0')) {
		endy1 = endy;
		pixPtr = _pixels;
		pos = _x;

	LABEL_17:
		srcPtr = pixPtr;
		while (1) {
			while (1) {
                while (1) {
					while (1) {
						while (1) {
							pixel = *(int16 *)srcPtr;
							srcPtr += 2;
							pixPtr = srcPtr;
							if (pixel)
								break;
							--endy1;
							if (endy1 < _y)
								return;
							pos = _x;
						}

						if (pixel == 0x100)
							return;

						if (pixel != 0x200)
							break;

						pixel1 = *(int16 *)srcPtr;
						srcPtr += 2;
						pos += (byte)pixel1;
						pixel1High = pixel1 >> 8;

						if (pixel1High) {
							endy1 -= pixel1High;

							if (endy1 < _y)
								return;
						}
					}
					start1 = pos;
					fillValue = (byte)pixel;

					if (!(byte)pixel)
						break;

					pos += (byte)pixel;
					pixoffset = -start1;

					if (pixoffset <= 0)
						goto LABEL_25;

					fillValue = (byte)pixel - pixoffset;

					if (fillValue > 0) {
						start1 = 0;

					LABEL_25:
						end2 = 799;
						if (pos <= end2 + 1 || (fillValue += end2 - pos + 1, fillValue > 0)) {
							if (endy1 <= endy) {
								curDestPtr = (int16 *)g_fullpipe->_backgroundSurface.getBasePtr(endy1, start1);
								int bgcolor = *(int32 *)(palette + 4 * ((pixel >> 8) & 0xff));

								colorFill(curDestPtr, fillValue, bgcolor);
							}
							goto LABEL_17;
						}
					}
				}
				pixelHigh = pixel >> 8;
				srcPtr2 = srcPtr;
				pos += pixelHigh;
				srcPtr += 2 * ((pixelHigh + 1) >> 1);
				pixoffset1 = -start1;

				if (pixoffset1 > 0)
					break;

			LABEL_37:
				leftx = 799;
				if (pos > leftx + 1) {
					pixelHigh += leftx - pos + 1;
					if (pixelHigh <= 0)
						continue;
				}
				if (endy1 <= endy) {
					curDestPtr = (int16 *)g_fullpipe->_backgroundSurface.getBasePtr(endy1, start1);
					paletteFill(curDestPtr, srcPtr2, pixelHigh, (int32 *)palette);
				}
			}
			pixelHigh -= pixoffset1;
			if (pixelHigh > 0) {
				start1 = 0;
				srcPtr2 += pixoffset1;
				goto LABEL_37;
			}
		}
	}

	cb05_format = (_type == MKTAG('C', 'B', '\05', 'e'));
	bpp = cb05_format ? 2 : 1;
	end = _width + _x - 1;
	pitch = (bpp * (endx - _x + 1) + 3) & 0xFFFFFFFC;
	start = _x;
	srcPtr1 = &_pixels[pitch * (endy - _y)];
	if (_x < 0) {
		srcPtr1 += bpp * -_x;
		start = 0;
	}

	if (endx > 799)
		end = 799;

	if (_flags & 0x1000000) {
		for (int n = _y; n < endy; srcPtr1 -= pitch) {
			curDestPtr = (int16 *)g_fullpipe->_backgroundSurface.getBasePtr(start, n);
			copierKeyColor(curDestPtr, srcPtr1, end - start + 1, _flags & 0xff, (int32 *)palette, cb05_format);
			++n;
		}
	} else {
		for (int n = _y; n <= endy; srcPtr1 -= pitch) {
			curDestPtr = (int16 *)g_fullpipe->_backgroundSurface.getBasePtr(start, n);
			copier(curDestPtr, srcPtr1, end - start + 1, (int32 *)palette, cb05_format);
			++n;
		}
	}

	g_fullpipe->_system->copyRectToScreen(g_fullpipe->_backgroundSurface.getBasePtr(start, _y), g_fullpipe->_backgroundSurface.pitch, start, _y, end, endy);
}

void Bitmap::colorFill(int16 *dest, int len, int color) {
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
		*dest++ = (int16)color;
}

void Bitmap::paletteFill(int16 *dest, byte *src, int len, int32 *palette) {
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
		*dest++ = palette[*src++];
}

void Bitmap::copierKeyColor(int16 *dest, byte *src, int len, int keyColor, int32 *palette, bool cb05_format) {
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
				*dest = palette[*src];

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

void Bitmap::copier(int16 *dest, byte *src, int len, int32 *palette, bool cb05_format) {
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
			*dest++ = palette[*src++];
	} else {
		int16 *src16 = (int16 *)src;

		for (int i = 0; i < len; i++)
			*dest++ = *src16++;
	}
}

BigPicture::BigPicture() {
}

bool BigPicture::load(MfcArchive &file) {
	debug(5, "BigPicture::load()");
	Picture::load(file);

	return true;
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


} // End of namespace Fullpipe
