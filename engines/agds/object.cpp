/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "agds/object.h"
#include "agds/agds.h"
#include "agds/animation.h"
#include "agds/font.h"
#include "agds/region.h"
#include "agds/systemVariable.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "common/rect.h"
#include "graphics/managed_surface.h"
#include "graphics/surface.h"

namespace AGDS {

Object::Object(const Common::String &name, Common::SeekableReadStream &stream, int version) : _name(name), _stringTableLoaded(false),
																							  _picture(), _rotatedPicture(), _region(),
																							  _animation(), _mouseCursor(),
																							  _pos(), _z(10), _rotation(0),
																							  _clickHandler(0), _examineHandler(0), _userUseHandler(0),
																							  _throwHandler(0), _useOnHandler(0),
																							  _handlerBD(0), _handlerC1(0),
																							  _alpha(255), _scale(100), _locked(0), _alive(true),
																							  _persistent(true), _allowInitialise(true),
																							  _ignoreRegion(false), _version(version) {
	uint16 id = stream.readUint16LE();
	debug("id: 0x%02x %u", id, id);

	uint16 dataSize = stream.readUint16LE();
	if (dataSize != 0)
		error("implement me: object with data (%u/0x%04x)", dataSize, dataSize);
	uint16 codeSize = stream.readUint16LE();
	uint8 flags = stream.readByte();
	if (flags != 1)
		error("implement me: no flags handling yet");

	debug("object code size %u", codeSize);
	_code.resize(codeSize);
	stream.read(_code.data(), _code.size());
}

Object::~Object() {
	freeRotated();
	freePicture();
}

void Object::lock() {
	++_locked;
}

void Object::unlock() {
	if (_locked == 0)
		error("%s: object lock counter underrun", _name.c_str());
	--_locked;
}

void Object::freeRotated() {
	if (_rotatedPicture) {
		_rotatedPicture->free();
		_rotatedPicture.reset();
	}
}

void Object::freePicture() {
	if (_picture) {
		_picture->free();
		_picture.reset();
	}
}

void Object::readStringTable(unsigned resOffset, uint16 resCount) {
	if (_stringTableLoaded)
		return;

	resOffset += 5 /*instruction*/ + (_version >= 2 ? 0x13 : 0x11) /*another header*/;
	if (resOffset >= _code.size())
		error("invalid resource table offset %u/%u", resOffset, _code.size());

	// debug("resource table at %04x", resOffset);
	Common::MemoryReadStream stream(_code.data() + resOffset, _code.size() - resOffset);
	_stringTable.resize(resCount);
	for (uint16 i = 0; i < resCount; ++i) {
		uint16 offset = stream.readUint16LE();
		uint16 flags = stream.readUint16LE();

		unsigned nameOffset = resOffset + offset;
		if (nameOffset > _code.size())
			error("invalid resource name offset %u/%u", nameOffset, _code.size());

		const char *nameBegin = reinterpret_cast<const char *>(_code.data() + nameOffset);
		const char *codeEnd = reinterpret_cast<const char *>(_code.data() + _code.size());
		const char *nameEnd = Common::find(nameBegin, codeEnd, 0);

		Common::String name(nameBegin, nameEnd - nameBegin);

		// debug("resource table 1[%04u]: 0x%04x %s", i, flags, name.c_str());
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

void Object::setPicture(Graphics::ManagedSurface *picture) {
	_pos = Common::Point();
	freePicture();
	_picture.reset(picture);
	freeRotated();
	_rotation = 0;

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
	createRotated();
}

void Object::rotate(int rot) {
	rot %= 4;
	if (rot == _rotation)
		return;

	debug("%s: setting rotation to %d", _name.c_str(), rot);
	_rotation = rot;
	createRotated();
}

void Object::createRotated() {
	freeRotated();
	if (_rotation == 0 || !_picture)
		return;

	Graphics::TransformStruct transform(100, 100, 90 * _rotation, _picture->w / 2, _picture->h / 2);
	_rotatedPicture.reset(getPicture()->rotoscale(transform));
}

void Object::alive(bool value) {
	_alive = value;
	if (!_alive)
		_region.reset();
}

void Object::region(RegionPtr region) {
	_region = region;
}

void Object::moveTo(Common::Point pos) {
	_pos = pos;
}

void Object::generateRegion(Common::Rect rect) {
	_region = RegionPtr(new Region(rect));
	debug("%s: generated region: %s", _name.c_str(), _region->toString().c_str());
}

void Object::generateRegion() {
	if (!getPicture()) {
		warning("generateRegion called on null picture");
		return;
	}

	generateRegion(getRect());
}

Common::Rect Object::getRect() const {
	auto picture = getPicture();
	if (!picture) {
		warning("getRect called on null picture");
		return Common::Rect();
	}
	Common::Rect rect = picture->getBounds();
	rect.moveTo(_pos.x, _pos.y);
	return rect;
}

bool Object::pointIn(Common::Point pos) {
	if (!_alive)
		return false;

	auto picture = getPicture();
	if (picture) {
		auto rect = getRect();
		if (rect.contains(pos)) {
			return true;
		}
	}

	if (_animation) {
		Common::Rect rect(0, 0, _animation->width(), _animation->height());
		rect.moveTo(_pos + _animationPos);
		if (rect.contains(pos)) {
			return true;
		}
	}

	// pos -= _pos;
	pos -= _regionOffset;

	if (_trapRegion && _trapRegion->pointIn(pos))
		return true;

	if (!_ignoreRegion && _region && _region->pointIn(pos))
		return true;

	return false;
}

void Object::paint(AGDSEngine &engine, Graphics::Surface &backbuffer, Common::Point pos) const {
	auto picture = getPicture();
	if (picture) {
		Common::Point dst = pos + getPosition();
		Common::Rect srcRect = picture->getBounds();
		if (!_srcRect.isEmpty()) {
			srcRect = _srcRect;
		}
		uint32 color = _picture->format.ARGBToColor(_alpha, 255, 255, 255);
		if (Common::Rect::getBlitRect(dst, srcRect, backbuffer.getRect())) {
			picture->blendBlitTo(backbuffer, dst.x, dst.y, Graphics::FLIP_NONE, &srcRect, color);
		}
	}

	if (_animation) {
		pos += _pos + _animationPos;
		_animation->tick();
		_animation->paint(backbuffer, pos);
	}

	if (!_text.empty()) {
		pos += _region ? _region->center + _regionOffset : _pos;
		int w = backbuffer.w - pos.x;
		engine.getFont(engine.getSystemVariable("objtext_font")->getInteger())->drawString(&backbuffer, _text, pos.x, pos.y, w, 0);
	}

	if (engine.showHints() && !_title.empty()) {
		pos += _region ? _region->center : _pos;
		int w = backbuffer.w - pos.x;
		auto font = engine.getFont(engine.getSystemVariable("tell_font")->getInteger());
		pos.x -= font->getStringWidth(_title) / 2;
		font->drawString(&backbuffer, _title, pos.x, pos.y, w, 0);
	}
}

} // namespace AGDS
