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

#include "agds/screen.h"
#include "agds/agds.h"
#include "agds/animation.h"
#include "agds/character.h"
#include "agds/object.h"
#include "agds/patch.h"
#include "agds/region.h"
#include "common/system.h"
#include "graphics/managed_surface.h"

namespace AGDS {

int Screen::ObjectZCompare(const ObjectPtr &a, const ObjectPtr &b) {
	return b->z() - a->z();
}

int Screen::AnimationZCompare(const ScreenAnimationDesc &a, const ScreenAnimationDesc &b) {
	return b.animation->z() - a.animation->z();
}

Screen::Screen(AGDSEngine *engine, ObjectPtr object, ScreenLoadingType loadingType, const Common::String &prevScreen) : _engine(engine), _object(object), _background(nullptr),
																														_name(object->getName()), _loadingType(loadingType), _previousScreen(prevScreen),
																														_children(&ObjectZCompare), _animations(&AnimationZCompare), _applyingPatch(false),
																														_characterNear(g_system->getHeight()), _characterFar(g_system->getHeight()), _fade(0) {
	add(object);
}

Screen::~Screen() {
	_children.clear();
}

void Screen::scrollTo(Common::Point scroll) {
	int windowW = g_system->getWidth();
	int windowH = g_system->getHeight();

	if (!_background) {
		_scroll.x = _scroll.y = 0;
		return;
	}

	auto rect = _background->getRect();
	int w = rect.width(), h = rect.height();
	debug("picture size %dx%d", w, h);
	if (scroll.x + windowW > w)
		scroll.x = w - windowW;
	if (scroll.y + windowH > h)
		scroll.y = h - windowH;
	if (scroll.x < 0)
		scroll.x = 0;
	if (scroll.y < 0)
		scroll.y = 0;

	debug("setting scroll to %d,%d", scroll.x, scroll.y);
	_scroll = scroll;
}

float Screen::getZScale(int y) const {
	int h = g_system->getHeight();
	int dy = h - y;
	if (dy > _characterNear) {
		if (dy < _characterFar)
			return 1.0f * (_characterFar - dy) / (_characterFar - _characterNear);
		else
			return 0.0f;
	} else
		return 1.0f;
}

bool Screen::add(ObjectPtr object) {
	if (object == NULL) {
		warning("refusing to add null to scene");
		return false;
	}
	for (auto i = _children.begin(); i != _children.end();) {
		if (*i == object) {
			if (object->alive()) {
				debug("double adding object %s", object->getName().c_str());
				return false;
			} else {
				debug("recovering object %s", object->getName().c_str());
				object->alive(true);
				object->allowInitialise(false);
				return true;
			}
		} else
			++i;
	}
	_children.insert(object);
	return true;
}

void Screen::add(AnimationPtr animation) {
	if (animation)
		_animations.insert({animation});
	else
		warning("Screen: skipping null animation");
}

bool Screen::remove(const AnimationPtr &animation) {
	if (!animation)
		return false;

	bool removed = false;
	for (auto i = _animations.begin(); i != _animations.end(); ++i) {
		if (i->animation == animation) {
			debug("removing animation %s:%s", animation->process().c_str(), animation->phaseVar().c_str());
			i->removed = true;
			removed = true;
		}
	}
	return removed;
}

ObjectPtr Screen::find(const Common::String &name) {
	for (auto i = _children.begin(); i != _children.end(); ++i) {
		ObjectPtr &object = *i;
		if (object->getName() == name)
			return *i;
	}
	return ObjectPtr();
}

bool Screen::remove(const ObjectPtr &object) {
	for (auto i = _children.begin(); i != _children.end(); ++i) {
		if (*i == object) {
			if (object->locked())
				object->alive(false);
			else
				_children.erase(i);
			return true;
		}
	}
	return false;
}

bool Screen::remove(const Common::String &name) {
	for (auto i = _children.begin(); i != _children.end(); ++i) {
		const ObjectPtr &object = *i;
		if (object->getName() == name) {
			if (object->locked())
				object->alive(false);
			else
				_children.erase(i);
			return true;
		}
	}
	return false;
}

AnimationPtr Screen::findAnimationByPhaseVar(const Common::String &phaseVar) {
	for (auto i = _animations.begin(); i != _animations.end(); ++i) {
		const auto &desc = *i;
		const auto &animation = desc.animation;
		if (!desc.removed && animation->phaseVar() == phaseVar)
			return animation;
	}
	return NULL;
}

void Screen::tick() {
	for (uint i = 0; i < _animations.size();) {
		const auto &desc = _animations.data()[i];
		const auto &animation = desc.animation;
		if (!desc.removed && animation->tick())
			++i;
		else {
			debug("removing animation %s:%s", animation->process().c_str(), animation->phaseVar().c_str());
			_animations.erase(_animations.begin() + i);
		}
	}
}

void Screen::paint(Graphics::Surface &backbuffer) const {
	auto &currentInventoryObject = _engine->currentInventoryObject();
	Character *character = _engine->currentCharacter();

	auto child = _children.begin();
	auto desc = _animations.begin();
	while (child != _children.end() || desc != _animations.end() || character) {
		bool child_valid = child != _children.end();
		bool animation_valid = desc != _animations.end();
		if (animation_valid && desc->removed) {
			++desc;
			continue;
		}

		bool z_valid = false;
		int z = 0;
		int render_type = -1;
		if (animation_valid) {
			const auto &animation = desc->animation;
			if (!z_valid || animation->z() > z) {
				z = animation->z();
				z_valid = true;
				render_type = 1;
			}
		}
		if (child_valid) {
			if (!z_valid || (*child)->z() > z) {
				z = (*child)->z();
				z_valid = true;
				render_type = 0;
			}
		}
		if (character) {
			if (!z_valid || character->z() > z) {
				z = character->z();
				z_valid = true;
				render_type = 2;
			}
		}

		auto basePos = Common::Point() - _scroll;
		switch (render_type) {
		case 0:
			// debug("object z: %d", (*child)->z());
			if ((*child) != currentInventoryObject && (*child)->alive()) {
				if ((*child)->scale() < 0)
					basePos = Common::Point();
				(*child)->paint(*_engine, backbuffer, basePos);
			}
			++child;
			break;
		case 1:
			// debug("animation z: %d", (*animation)->z());
			if ((*desc).animation->scale() < 0)
				basePos = Common::Point();
			(*desc).animation->paint(backbuffer, basePos);
			++desc;
			break;
		case 2:
			// debug("character z: %d", character->z());
			character->paint(backbuffer, basePos);
			character = nullptr;
			break;
		default:
			error("invalid logic in z-sort");
		}
	}
	if (_fade != 0) {
		auto alpha = (100 - _fade) * 256 / 100;
		auto &format = backbuffer.format;
		for (int y = 0; y < backbuffer.h; ++y) {
			uint32 *line = (uint32 *)backbuffer.getBasePtr(0, y);
			int w = backbuffer.w;
			while (w--) {
				uint8 r, g, b;
				format.colorToRGB(*line, r, g, b);
				r = (r * alpha) >> 8;
				g = (g * alpha) >> 8;
				b = (b * alpha) >> 8;
				*line++ = format.RGBToColor(r, g, b);
			}
		}
	}
}

Common::Array<ObjectPtr> Screen::find(Common::Point pos) const {
	Common::Array<ObjectPtr> objects;
	objects.reserve(_children.size());
	for (auto i = _children.begin(); i != _children.end(); ++i) {
		const auto &object = *i;
		auto visiblePos = (object->scale() >= 0) ? pos + _scroll : pos;
		if (object->pointIn(visiblePos) && object->alive()) {
			objects.insert_at(0, object);
		}
	}
	Character *character = _engine->currentCharacter();
	if (character) {
		if (character->pointIn(pos - _scroll)) {
			objects.insert_at(0, character->object());
		}
	}
	return objects;
}

Screen::KeyHandler Screen::findKeyHandler(const Common::String &keyName) {
	KeyHandler keyHandler;
	for (auto i = _children.begin(); i != _children.end(); ++i) {
		const auto &object = *i;
		if (!object->alive())
			continue;

		uint ip = object->getKeyHandler(keyName);
		if (ip) {
			keyHandler.ip = ip;
			keyHandler.object = object;
			break;
		}
	}
	return keyHandler;
}

void Screen::load(const PatchPtr &patch) {
	debug("applying patch with %u objects", patch->objects.size());
	_applyingPatch = true;
	for (uint i = 0; i < patch->objects.size(); ++i) {
		const Patch::Object &object = patch->objects[i];
		debug("patch object %s %d", object.name.c_str(), object.flag);
		if (object.name == _name)
			continue;
		else if (object.flag <= 0)
			remove(object.name);
		else
			_engine->runObject(object.name, Common::String(), false);
	}
	_loadingType = patch->loadingType;
	if (!patch->prevScreenName.empty())
		_previousScreen = patch->prevScreenName;
	_applyingPatch = false;
}

void Screen::save(const PatchPtr &patch) {
	patch->screenSaved = 1;
	patch->prevScreenName = _previousScreen;
	patch->loadingType = _loadingType;
	patch->objects.clear();
	for (auto i = _children.begin(); i != _children.end(); ++i) {
		const auto &object = *i;
		if (!object->persistent() || !object->alive() || object->getName() == _name)
			continue;
		debug("saving patch object %s %d", object->getName().c_str(), object->alive());
		patch->objects.push_back(Patch::Object(object->getName(), 1));
	}
}

} // namespace AGDS
