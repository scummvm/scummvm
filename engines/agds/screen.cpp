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

#include "agds/screen.h"
#include "agds/agds.h"
#include "agds/animation.h"
#include "agds/character.h"
#include "agds/object.h"
#include "agds/patch.h"
#include "agds/region.h"
#include "common/system.h"

namespace AGDS {

int Screen::ObjectZCompare(const ObjectPtr &a, const ObjectPtr &b) {
	return b->z() - a->z();
}

int Screen::AnimationZCompare(const Animation *a, const Animation *b) {
	return b->z() - a->z();
}

Screen::Screen(AGDSEngine * engine, ObjectPtr object) : _engine(engine), _object(object), _name(object->getName()),
	_children(&ObjectZCompare), _animations(&AnimationZCompare), _applyingPatch(false),
	_characterNear(g_system->getHeight()), _characterFar(g_system->getHeight()) {
	add(object);
}

Screen::~Screen() {
	_children.clear();
}

float Screen::getZScale(int y) const
{
	int dy = g_system->getHeight() - y;
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
	for (ChildrenType::iterator i = _children.begin(); i != _children.end(); ) {
		if (*i == object) {
			if ((*i)->inScene()) {
				debug("double adding object %s", (*i)->getName().c_str());
				return false;
			} else {
				i = _children.erase(i);
			}
		} else
			++i;
	}
	object->inScene(true);
	_children.insert(object);
	return true;
}

bool Screen::remove(Animation * animation) {
	bool removed = false;
	for(auto i = _animations.begin(); i != _animations.end(); ) {
		if (*i == animation) {
			_animations.erase(i++);
			removed = true;
		} else
			++i;
	}
	return removed;
}

ObjectPtr Screen::find(const Common::String &name) {
	for (ChildrenType::iterator i = _children.begin(); i != _children.end(); ++i) {
		ObjectPtr &object = *i;
		if (object->getName() == name)
			return *i;
	}
	return ObjectPtr();
}

bool Screen::remove(const ObjectPtr &object) {
	for (ChildrenType::iterator i = _children.begin(); i != _children.end(); ++i) {
		if (*i == object && object->inScene()) {
			object->inScene(false);
			return true;
		}
	}
	return false;
}

bool Screen::remove(const Common::String &name) {
	for (ChildrenType::iterator i = _children.begin(); i != _children.end(); ++i) {
		const ObjectPtr & object = *i;
		if (object->getName() == name && object->inScene()) {
			object->inScene(false);
			return true;
		}
	}
	return false;
}

Animation *Screen::findAnimationByPhaseVar(const Common::String &phaseVar) {
	for (auto i = _animations.begin(); i != _animations.end(); ++i) {
		Animation *animation = *i;
		if (animation->phaseVar() == phaseVar)
			return animation;
	}
	return NULL;
}

void Screen::paint(Graphics::Surface &backbuffer) {
	auto & currentInventoryObject = _engine->currentInventoryObject();
	for(uint i = 0; i < _animations.size(); ) {
		Animation *animation = _animations.data()[i];
		if (animation->tick(*_engine))
			++i;
		else {
			debug("removing animation %s:%s", animation->process().c_str(), animation->phaseVar().c_str());
			_animations.erase(_animations.begin() + i);
		}
	}

	Character * character = _engine->currentCharacter();
	ChildrenType::iterator child = _children.begin();
	AnimationsType::iterator animation = _animations.begin();
	while(child != _children.end() || animation != _animations.end() || character) {
		bool child_valid = child != _children.end();
		bool animation_valid = animation != _animations.end();

		bool z_valid = false;
		int z = 0;
		int render_type = -1;
		if (child_valid) {
			if (!z_valid || (*child)->z() > z) {
				z = (*child)->z();
				z_valid = true;
				render_type = 0;
			}
		}
		if (animation_valid) {
			if (!z_valid || (*animation)->z() > z) {
				z = (*animation)->z();
				z_valid = true;
				render_type = 1;
			}
		}
		if (character) {
			if (!z_valid || character->z() > z) {
				z = character->z();
				z_valid = true;
				render_type = 2;
			}
		}

		switch (render_type) {
			case 0:
				//debug("object z: %d", (*child)->z());
				if ((*child) != currentInventoryObject && (*child)->inScene())
					(*child)->paint(*_engine, backbuffer);
				++child;
				break;
			case 1:
				//debug("animation z: %d", (*animation)->z());
				(*animation)->paint(*_engine, backbuffer, Common::Point());
				++animation;
				break;
			case 2:
				//debug("character z: %d", character->z());
				character->paint(backbuffer);
				character = nullptr;
				break;
			default:
				error("invalid logic in z-sort");
		}
	}
}

ObjectPtr Screen::find(Common::Point pos) const {
	for (ChildrenType::const_iterator i = _children.begin(); i != _children.end(); ++i) {
		ObjectPtr object = *i;
		if (object->pointIn(pos) && object->inScene())
			return object;
	}
	return ObjectPtr();
}

Screen::KeyHandler Screen::findKeyHandler(const Common::String &keyName) {
	KeyHandler keyHandler;
	for (ChildrenType::const_iterator i = _children.begin(); i != _children.end(); ++i) {
		ObjectPtr object = *i;
		if (!object->inScene())
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
	for(uint i = 0; i < patch->objects.size(); ++i) {
		const Patch::Object &object = patch->objects[i];
		debug("patch object %s %d", object.name.c_str(), object.flag);
		if (object.flag <= 0)
			remove(object.name);
		else {
			auto instance = find(object.name); //find by name finds removed objects too
			if (!instance || !instance->inScene()) {
				_engine->runObject(object.name);
			}
		}
	}
	_applyingPatch = false;
}

void Screen::save(const PatchPtr &patch) {
	patch->objects.clear();
	for (ChildrenType::const_iterator i = _children.begin(); i != _children.end(); ++i) {
		ObjectPtr object = *i;
		if (!object->persistent())
			continue;
		debug("saving patch object %s %d", object->getName().c_str(), object->inScene());
		patch->objects.push_back(Patch::Object(object->getName(), object->inScene()));
	}
}


} // namespace AGDS
