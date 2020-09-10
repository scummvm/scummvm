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

#include "agds/object.h"
#include "agds/agds.h"
#include "agds/animation.h"
#include "agds/font.h"
#include "agds/region.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "graphics/transparent_surface.h"

namespace AGDS {

Object::Object(const Common::String &name, Common::SeekableReadStream *stream) : _name(name), _stringTableLoaded(false),
                                                                                 _picture(), _region(),
                                                                                 _animation(), _mouseCursor(),
                                                                                 _pos(), _z(10),
                                                                                 _clickHandler(0), _examineHandler(0),
                                                                                 _alpha(255), _inScene(false) {
	byte id = stream->readUint16LE();
	debug("id: 0x%02x %u", id, id);

	uint16 dataSize = stream->readUint16LE();
	if (dataSize != 0)
		error("implement me: object with data (%u/0x%04x)", dataSize, dataSize);
	uint16 codeSize = stream->readUint16LE();
	uint8 flags = stream->readByte();
	if (flags != 1)
		error("implement me: no flags handling yet");

	debug("object code size %u", codeSize);
	_code.resize(codeSize);
	stream->read(_code.data(), _code.size());
}

Object::~Object() {
	if (_picture) {
		_picture->free();
		delete _picture;
	}
}

void Object::readStringTable(unsigned resOffset, uint16 resCount) {
	if (_stringTableLoaded)
		return;

	resOffset += 5 /*instruction*/ + 0x11 /*another header*/;
	if (resOffset >= _code.size())
		error("invalid resource table offset");

	//debug("resource table at %08x", resOffset);
	Common::MemoryReadStream stream(_code.data() + resOffset, _code.size() - resOffset);
	_stringTable.resize(resCount);
	for (uint16 i = 0; i < resCount; ++i) {
		uint16 offset = stream.readUint16LE();
		uint16 flags = stream.readUint16LE();

		unsigned nameOffset = resOffset + offset;
		if (nameOffset > _code.size())
			error("invalid resource name offset");

		const char *nameBegin = reinterpret_cast<const char *>(_code.data() + nameOffset);
		const char *codeEnd = reinterpret_cast<const char *>(_code.data() + _code.size());
		const char *nameEnd = Common::find(nameBegin, codeEnd, 0);

		Common::String name(nameBegin, nameEnd - nameBegin);

		//debug("resource table 1[%04u]: 0x%04x %s", i, flags, name.c_str());
		_stringTable[i] = StringEntry(name, flags);
	}
	debug("loaded %u strings", resCount);
	_stringTableLoaded = true;
}

const Object::StringEntry &Object::getString(uint16 index) const {
	if (!_stringTableLoaded)
		error("no string table loaded");

	if (index >= _stringTable.size()) {
		static StringEntry empty;
		warning("no resource name with id %u", index);
		return empty;
	}

	return _stringTable[index];
}

void Object::setPicture(Graphics::TransparentSurface *picture) {
	if (_picture) {
		_picture->free();
		delete _picture;
	}
	_picture = picture;

	if (!picture) {
		_offset = Common::Point();
		return;
	}

	assert(picture->format.bytesPerPixel == 4);
	const byte *pixels = static_cast<const byte *>(picture->getPixels());
	uint pitch = picture->pitch;

	uint16 w = picture->w, h = picture->h;
	_offset.x = w;
	_offset.y = h;

	for (uint16 y = 0; y < h; ++y) {
		for (uint16 x = 0; x < w; ++x) {
			uint32 color = *reinterpret_cast<const uint32 *>(pixels + (y * pitch + x * picture->format.bytesPerPixel));
			uint8 r, g, b, a;
			picture->format.colorToARGB(color, a, r, g, b);
			if (a != 0) {
				if (y < _offset.y)
					_offset.y = y;
				if (x < _offset.x)
					_offset.x = x;
				break;
			}
		}
	}
	debug("OFFSET %d, %d", _offset.x, _offset.y);
}

void Object::region(RegionPtr region) {
	_region = region;
	_pos = region->topLeft();
}

void Object::moveTo(Common::Point pos)
{
	Common::Point delta = pos - _pos;
	debug("moving object %+d,%+d", delta.x, delta.y);
	if (_region)
		_region->move(delta);
	_pos = pos;
}

void Object::generateRegion() {
	if (!_picture) {
		warning("generateRegion called on null picture");
		return;
	}

	Common::Rect rect = _picture->getRect();
	rect.moveTo(_pos.x, _pos.y);
	_region = RegionPtr(new Region(rect));
	debug("%s: generated region: %s", _name.c_str(), _region->toString().c_str());
}

void Object::paint(AGDSEngine &engine, Graphics::Surface &backbuffer) {
	if (_picture) {
		Common::Point dst = _pos - _offset;
		Common::Rect srcRect = _picture->getRect();
		uint32 color = (_alpha << 24) | 0xffffff; //fixme: _picture->format.ARGBToColor(_alpha, 255, 255, 255); is not working
		if (Common::Rect::getBlitRect(dst, srcRect, backbuffer.getRect())) {
			_picture->blit(backbuffer, dst.x, dst.y, Graphics::FLIP_NONE, &srcRect, color);
		}
	}
	if (_animation) {
		_animation->paint(engine, backbuffer, _animationPos);
	}
	if (!_text.empty()) {
		Common::Point pos = _region ? _region->center : _pos;
		int w = backbuffer.w - pos.x;
		engine.getFont(1)->drawString(&backbuffer, _text, pos.x, pos.y, w, 0);
	}
}

} // namespace AGDS
