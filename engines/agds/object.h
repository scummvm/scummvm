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

#ifndef AGDS_OBJECT_H
#define AGDS_OBJECT_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "common/stream.h"

namespace Graphics { struct Surface; struct TransparentSurface; }

namespace AGDS {

class AGDSEngine;
struct Region;
typedef Common::SharedPtr<Region> RegionPtr;
class Animation;

class Object {
public:
	typedef Common::Array<uint8> CodeType;

	struct StringEntry
	{
		Common::String	string;
		uint16			flags;

		StringEntry(): string(), flags() { }
		StringEntry(const Common::String &s, uint16 f): string(s), flags(f) { }
	};

private:
	typedef Common::Array<StringEntry> StringTableType;
	typedef Common::HashMap<Common::String, uint, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> KeyHandlersType;
	typedef Common::HashMap<Common::String, uint> UseHandlersType;

	Common::String					_name;
	CodeType						_code;
	StringTableType					_stringTable;
	bool							_stringTableLoaded;
	KeyHandlersType					_keyHandlers;
	UseHandlersType					_useHandlers;
	Graphics::TransparentSurface *	_picture;
	Graphics::TransparentSurface *	_rotatedPicture;
	RegionPtr						_region;
	RegionPtr						_trapRegion;
	Animation *						_animation;
	Animation *						_mouseCursor;
	Common::Point					_pos, _animationPos, _offset;
	Common::Point					_regionOffset;
	Common::Rect					_srcRect;
	int								_z;
	Common::String					_text;
	Common::String					_title;
	uint							_clickHandler;
	uint							_examineHandler;
	uint							_userUseHandler;
	uint							_throwHandler;
	uint							_useOnHandler;
	uint							_trapHandler;
	int								_alpha;
	int								_scale;
	uint							_locked;
	bool							_alive;
	bool							_persistent;
	bool							_allowInitialise;
	bool							_ignoreRegion;

private:
	void freeRotated();

public:
	Object(const Common::String &name, Common::SeekableReadStream * stream);
	~Object();

	bool allowInitialise() const {
		return _allowInitialise;
	}
	void allowInitialise(bool allow) {
		_allowInitialise = allow;
	}

	void persistent(bool persistent) {
		_persistent = persistent;
	}
	bool persistent() const {
		return _persistent;
	}

	void ignoreRegion(bool ignoreRegion) {
		_ignoreRegion = ignoreRegion;
	}
	bool ignoreRegion() const {
		return _ignoreRegion;
	}

	void readStringTable(unsigned resOffset, uint16 resCount);
	const StringEntry & getString(uint16 index) const;
	uint getStringTableSize() const
	{ return _stringTable.size(); }

	const Common::String & getName() const
	{ return _name; }

	const CodeType & getCode() const {
		return _code;
	}

	void setAnimation(Animation *animation) {
		_animation = animation;
	}

	Animation *getAnimation() const {
		return _animation;
	}

	void setAnimationPosition(Common::Point animationPos) {
		_animationPos = animationPos;
	}

	void setMouseCursor(Animation *mouseCursor) {
		_mouseCursor = mouseCursor;
		freeRotated();
	}
	Animation *getMouseCursor() const {
		return _mouseCursor;
	}

	void setPicture(Graphics::TransparentSurface *);

	Graphics::TransparentSurface *getPicture() const {
		return _rotatedPicture? _rotatedPicture: _picture;
	}

	void rotate(int rot);

	void generateRegion();
	void generateRegion(Common::Rect rect);

	void regionOffset(Common::Point offset) {
		_regionOffset = offset;
	}

	void setAlpha(int alpha) {
		if (alpha < 0)
			alpha = 0;
		if (alpha > 100)
			alpha = 100;
		_alpha = (100 - alpha) * 255 / 100;
	}

	int alpha() const {
		return _alpha;
	}

	void scale(int scale) {
		_scale = scale;
	}

	int scale() const {
		return _scale;
	}

	void region(RegionPtr region);

	const RegionPtr& region() const {
		return _region;
	}

	void setClickHandler(uint ip) {
		_clickHandler = ip;
	}

	uint getClickHandler() const {
		return _clickHandler;
	}

	void setExamineHandler(uint ip) {
		_examineHandler = ip;
	}

	uint getExamineHandler() const {
		return _examineHandler;
	}

	void setUseHandler(const Common::String &name, uint ip) {
		_useHandlers[name] = ip;
	}

	uint getUseHandler(const Common::String &name) const {
		return _useHandlers.getValOrDefault(name, 0); //use handler can never be 0
	}

	void setUserUseHandler(uint ip) {
		_userUseHandler = ip;
	}

	void setThrowHandler(uint ip) {
		_throwHandler = ip;
	}

	void setUseOnHandler(uint ip) {
		_useOnHandler = ip;
	}

	uint getUserUseHandler() const {
		return _userUseHandler;
	}

	void setTrapHandler(uint ip, RegionPtr region) {
		_trapHandler = ip;
		_trapRegion = region;
	}

	RegionPtr getTrapRegion() const
	{ return _trapRegion; }

	uint getTrapHandler() const
	{ return _trapHandler; }

	void paint(AGDSEngine &engine, Graphics::Surface &backbuffer, Common::Point pos) const;

	void moveTo(Common::Point pos);

	void z(int z) {
		_z = z;
	}

	int z() const {
		return _z;
	}

	const Common::String & getText() const {
		return _text;
	}

	void setText(const Common::String &text) {
		_text = text;
	}

	void title(const Common::String &title) {
		_title = title;
	}

	const Common::String& title() const {
		return _title;
	}

	Common::Point getPosition() const {
		return _pos - _offset;
	}

	Common::Point getOffset() const {
		return _offset;
	}

	Common::Rect getRect() const;

	void setKeyHandler(const Common::String &name, uint ip) {
		_keyHandlers[name] = ip;
	}

	uint getKeyHandler(const Common::String &name) const {
		KeyHandlersType::const_iterator i = _keyHandlers.find(name);
		return i != _keyHandlers.end()? i->_value: 0;
	}

	uint throwHandler() const {
		return _throwHandler;
	}

	uint useOnHandler() const {
		return _useOnHandler;
	}

	bool locked() const {
		return _locked != 0;
	}
	void lock();
	void unlock();

	bool alive() const
	{ return _alive; }

	void alive(bool value);

	void srcRect(Common::Rect srcRect) { _srcRect = srcRect; }

	bool pointIn(Common::Point pos);
};
typedef Common::SharedPtr<Object> ObjectPtr;


} // End of namespace AGDS

#endif /* AGDS_OBJECT_H */
