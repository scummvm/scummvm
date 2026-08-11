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

#include "qdengine/qdengine.h"
#include "qdengine/minigames/adv/common.h"
#include "qdengine/minigames/adv/ObjectContainer.h"
#include "qdengine/minigames/adv/RunTime.h"

namespace QDEngine {

ObjectContainer::ObjectContainer() {
	_current = 0;

}

void ObjectContainer::release(MinigameManager *runtime) {
	for (auto &it : _objects)
		runtime->release(it);

	_objects.clear();
	_current = 0;
}

void ObjectContainer::pushObject(QDObject& obj) {
	assert(Common::find(_objects.begin(), _objects.end(), obj) == _objects.end());
	_objects.push_back(obj);
}

const char *ObjectContainer::name() const {
#ifdef _DEBUG
	return _name.c_str();
#else
	return "";
#endif
}

bool ObjectContainer::load(const char* base_name, MinigameManager *runtime, bool hide) {
	if (!runtime->testObject(base_name)) {
		warning("ObjectContainer::load(): Object '%s' not found", transCyrillic(base_name));
		return false;
	}

#ifdef _DEBUG
	_name = base_name;
#endif

	QDObject obj = runtime->getObject(base_name);
	_coord = runtime->world2game(obj);
	pushObject(obj);
	if (hide)
		runtime->hide(obj);

	char name[128];
	name[127] = 0;
	for (int dubl = 0; ; ++dubl) {
		snprintf(name, 127, "%s%04d", base_name, dubl);
		if (runtime->testObject(name)) {
			obj = runtime->getObject(name);
			pushObject(obj);
			if (hide)
				runtime->hide(obj);
		} else
			break;
	}

	return true;
}

void ObjectContainer::hideAll(MinigameManager *runtime) {
	for (auto &it : _objects)
		runtime->hide(it);
}

QDObject ObjectContainer::getObject() {
	if (_current < (int)_objects.size())
		return _objects[_current++];

	return _objects[0]; // bad, but better than crashing

}

void ObjectContainer::releaseObject(QDObject& obj, MinigameManager *runtime) {
	QDObjects::iterator it = Common::find(_objects.begin(), _objects.end(), obj);
	if (it != _objects.end()) {
		if ((int)Common::distance(_objects.begin(), it) >= _current)
			error("ObjectContainer::releaseObject(): Object released more than once in to the pool: %s", transCyrillic(name()));

		runtime->hide(obj);
		if (_current > 0)
			SWAP(*it, _objects[--_current]);
		obj = 0;
	}
}

} // namespace QDEngine
