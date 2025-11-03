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

#ifndef OBJECT_H
#define OBJECT_H

#include "common/array.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/stream.h"

namespace Graphics {
struct Surface;
class ManagedSurface;
} // namespace Graphics

namespace AGDS {

class AGDSEngine;
struct Region;
using RegionPtr = Common::SharedPtr<Region>;
class Animation;
using AnimationPtr = Common::SharedPtr<Animation>;

class Object {
public:
	using CodeType = Common::Array<uint8>;

	struct StringEntry {
		Common::String string;
		uint16 flags;

		StringEntry() : string(), flags() {}
		StringEntry(const Common::String &s, uint16 f) : string(s), flags(f) {}
	};

private:
	using StringTableType = Common::Array<StringEntry>;
	using KeyHandlersType = Common::HashMap<Common::String, uint, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>;
	using UseHandlersType = Common::HashMap<Common::String, uint>;
	using ManagedSurfacePtr = Common::ScopedPtr<Graphics::ManagedSurface>;

	Common::String _name;
	CodeType _code;
	StringTableType _stringTable;
	bool _stringTableLoaded;
	KeyHandlersType _keyHandlers;
	UseHandlersType _useHandlers;
	ManagedSurfacePtr _picture;
	ManagedSurfacePtr _rotatedPicture;
	RegionPtr _region;
	RegionPtr _trapRegion;
	AnimationPtr _animation;
	AnimationPtr _mouseCursor;
	Common::Point _pos, _animationPos, _offset;
	Common::Point _regionOffset;
	Common::Rect _srcRect;
	int _z;
	int _rotation;
	Common::String _text;
	Common::String _title;
	uint _clickHandler;
	uint _examineHandler;
	uint _userUseHandler;
	uint _throwHandler;
	uint _useOnHandler;
	uint _trapHandler;
	uint _handlerBD;
	uint _handlerC1;
	int _alpha;
	int _scale;
	uint _locked;
	bool _alive;
	bool _persistent;
	bool _allowInitialise;
	bool _ignoreRegion;
	bool _v2;

private:
	void freeRotated();
	void freePicture();
	void createRotated();

public:
	Object(const Common::String &name, Common::SeekableReadStream &stream, bool v2);
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
	const StringEntry &getString(uint16 index) const;
	uint getStringTableSize() const { return _stringTable.size(); }

	const Common::String &getName() const { return _name; }

	const CodeType &getCode() const {
		return _code;
	}

	void setAnimation(const AnimationPtr &animation) {
		_animation = animation;
	}

	const AnimationPtr &getAnimation() const {
		return _animation;
	}

	void setAnimationPosition(Common::Point animationPos) {
		_animationPos = animationPos;
	}

	void setMouseCursor(const AnimationPtr &mouseCursor) {
		_mouseCursor = mouseCursor;
		freeRotated();
	}

	const AnimationPtr &getMouseCursor() const {
		return _mouseCursor;
	}

	void setPicture(Graphics::ManagedSurface *);

	Graphics::ManagedSurface *getPicture() const {
		return _rotatedPicture ? _rotatedPicture.get() : _picture.get();
	}

	void rotate(int rot);
	int rotation() const {
		return _rotation;
	}

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

	const RegionPtr &region() const {
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
		return _useHandlers.getValOrDefault(name, 0); // use handler can never be 0
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

	void setHandlerBD(uint ip) {
		_handlerBD = ip;
	}

	uint getHandlerBD() const {
		return _handlerBD;
	}

	void setHandlerC1(uint ip) {
		_handlerC1 = ip;
	}

	uint getHandlerC1() const {
		return _handlerC1;
	}

	void setTrapHandler(uint ip, RegionPtr region) {
		_trapHandler = ip;
		_trapRegion = region;
	}

	RegionPtr getTrapRegion() const { return _trapRegion; }

	uint getTrapHandler() const { return _trapHandler; }

	void paint(AGDSEngine &engine, Graphics::Surface &backbuffer, Common::Point pos) const;

	void moveTo(Common::Point pos);

	void z(int z) {
		_z = z;
	}

	int z() const {
		return _z;
	}

	const Common::String &getText() const {
		return _text;
	}

	void setText(const Common::String &text) {
		_text = text;
	}

	void title(const Common::String &title) {
		_title = title;
	}

	const Common::String &title() const {
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
		return i != _keyHandlers.end() ? i->_value : 0;
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

	bool alive() const { return _alive; }

	void alive(bool value);

	void srcRect(Common::Rect srcRect) { _srcRect = srcRect; }

	bool pointIn(Common::Point pos);
};
using ObjectPtr = Common::SharedPtr<Object>;

} // End of namespace AGDS

#endif /* AGDS_OBJECT_H */
