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

#ifndef SCREEN_H
#define SCREEN_H

#include "agds/screenLoadingType.h"
#include "common/array.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Graphics {
struct Surface;
}

namespace AGDS {

class AGDSEngine;
class Animation;
using AnimationPtr = Common::SharedPtr<Animation>;
class Object;
using ObjectPtr = Common::SharedPtr<Object>;
struct Region;
using RegionPtr = Common::SharedPtr<Region>;
struct Patch;
using PatchPtr = Common::SharedPtr<Patch>;

struct ScreenAnimationDesc {
	AnimationPtr animation;
	bool removed = false;
	ScreenAnimationDesc(const AnimationPtr &a) : animation(a) {
	}
};

class Screen {
	static int ObjectZCompare(const ObjectPtr &a, const ObjectPtr &b);
	static int AnimationZCompare(const ScreenAnimationDesc &a, const ScreenAnimationDesc &b);

	using Animations = Common::SortedArray<ScreenAnimationDesc, const ScreenAnimationDesc &>;
	using Children = Common::SortedArray<ObjectPtr, const ObjectPtr &>;

	AGDSEngine *_engine;
	ObjectPtr _object;
	Object *_background;
	Common::Point _scroll;
	Common::String _name;
	ScreenLoadingType _loadingType;
	Common::String _previousScreen;
	Children _children;
	Animations _animations;
	RegionPtr _region;
	bool _applyingPatch;
	int _characterNear, _characterFar;
	int _fade;

public:
	struct KeyHandler {
		ObjectPtr object;
		uint ip;

		KeyHandler() : object(), ip() {}
		KeyHandler(Object *o, uint i) : object(o), ip(i) {}
	};

	Screen(AGDSEngine *engine, ObjectPtr object, ScreenLoadingType loadingType, const Common::String &prevScreen);
	~Screen();

	int fade() const {
		return _fade;
	}

	void fade(int fade) {
		_fade = fade;
	}

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

	const RegionPtr &region() const {
		return _region;
	}

	void region(RegionPtr region) {
		_region = region;
	}

	const Children &children() const {
		return _children;
	}

	const Animations &animations() const {
		return _animations;
	}
	void scrollTo(Common::Point scroll);
	Common::Point scrollPosition() const {
		return _scroll;
	}

	bool add(ObjectPtr object);
	void add(AnimationPtr animation);
	bool remove(const AnimationPtr &animation);

	void update(const ObjectPtr &object) {
		bool found = remove(object);
		if (found)
			add(object);
	}

	void setBackground(Object *object) {
		_background = object;
	}

	bool remove(const Common::String &name);
	bool remove(const ObjectPtr &object);

	void tick();
	void paint(Graphics::Surface &backbuffer) const;
	Common::Array<ObjectPtr> find(Common::Point pos) const;
	ObjectPtr find(const Common::String &name);
	KeyHandler findKeyHandler(const Common::String &keyName);
	AnimationPtr findAnimationByPhaseVar(const Common::String &phaseVar);

	void load(const PatchPtr &patch);
	void save(const PatchPtr &patch);
};

} // End of namespace AGDS

#endif /* AGDS_SCREEN_H */
