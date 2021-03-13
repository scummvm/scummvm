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

#ifndef AGDS_SCREEN_H
#define AGDS_SCREEN_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/ptr.h"
#include "common/str.h"
#include "common/rect.h"
#include "agds/screenLoadingType.h"

namespace Graphics {
	struct Surface;
}

namespace AGDS {

class AGDSEngine;
class Object;
class Animation;
typedef Common::SharedPtr<Object> ObjectPtr;
struct Region;
typedef Common::SharedPtr<Region> RegionPtr;
struct Patch;
typedef Common::SharedPtr<Patch> PatchPtr;

class Screen {
	static int ObjectZCompare(const ObjectPtr & a, const ObjectPtr & b);
	static int AnimationZCompare(const Animation *a, const Animation *b);

	typedef Common::SortedArray<Animation *, const Animation *> AnimationsType;
	typedef Common::SortedArray<ObjectPtr, const ObjectPtr &> ChildrenType;

	AGDSEngine *	_engine;
	ObjectPtr		_object;
	Common::String	_name;
	ScreenLoadingType _loadingType;
	Common::String	_previousScreen;
	ChildrenType	_children;
	AnimationsType	_animations;
	RegionPtr		_region;
	bool			_applyingPatch;
	int				_characterNear, _characterFar;

public:
	struct KeyHandler {
		ObjectPtr	object;
		uint		ip;

		KeyHandler(): object(), ip() { }
		KeyHandler(Object *o, uint i): object(o), ip(i) { }
	};

	Screen(AGDSEngine *engine, ObjectPtr object, ScreenLoadingType loadingType, const Common::String &prevScreen);
	~Screen();

	void setCharacterNearFar(int near, int far) {
		_characterNear = near;
		_characterFar = far;
	}

	float getZScale(int y) const;

	bool applyingPatch() const {
		return _applyingPatch;
	}

	ObjectPtr getObject() {
		return _object;
	}

	const Common::String &getName() const {
		return _name;
	}

	const Common::String &getPreviousScreenName() const {
		return _previousScreen;
	}

	ScreenLoadingType loadingType() const {
		return _loadingType;
	}

	RegionPtr region() const {
		return _region;
	}

	void region(RegionPtr region) {
		_region = region;
	}

	const ChildrenType & children() const {
		return _children;
	}

	const AnimationsType & animations() const {
		return _animations;
	}

	bool add(ObjectPtr object);
	void add(Animation * animation);
	bool remove(Animation * animation);

	void update(const ObjectPtr &object) {
		bool found = remove(object);
		if (found)
			add(object);
	}

	bool remove(const Common::String & name);
	bool remove(const ObjectPtr & object);
	void paint(Graphics::Surface & backbuffer);
	Common::Array<ObjectPtr> find(Common::Point pos) const;
	ObjectPtr find(const Common::String &name);
	KeyHandler findKeyHandler(const Common::String &keyName);
	Animation * findAnimationByPhaseVar(const Common::String &phaseVar);

	void load(const PatchPtr &patch);
	void save(const PatchPtr &patch);
};


} // End of namespace AGDS

#endif /* AGDS_SCREEN_H */
