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
	current_ = 0;

}

void ObjectContainer::release() {
	for (auto &it : objects_)
		runtime->release(it);

	objects_.clear();
	current_ = 0;
}

void ObjectContainer::pushObject(QDObject& obj) {
	assert(Common::find(objects_.begin(), objects_.end(), obj) == objects_.end());
	objects_.push_back(obj);
}

const char *ObjectContainer::name() const {
	#ifdef _DEBUG
	return name_.c_str();
	#else
	return "";
	#endif

}

bool ObjectContainer::load(const char* base_name, bool hide) {
	if (!runtime->testObject(base_name)) {
		warning("ObjectContainer::load(): Object '%s' not found", transCyrillic(base_name));
		return false;
	}

	#ifdef _DEBUG
	name_ = base_name;
	#endif

	QDObject obj = runtime->getObject(base_name);
	coord_ = runtime->world2game(obj);
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

void ObjectContainer::hideAll() {
	for (auto &it : objects_)
		runtime->hide(it);
}

QDObject ObjectContainer::getObject() {
	if (current_ < objects_.size())
		return objects_[current_++];

	return objects_[0]; // bad, but better than crashing

}

void ObjectContainer::releaseObject(QDObject& obj) {
	QDObjects::iterator it = Common::find(objects_.begin(), objects_.end(), obj);
	if (it != objects_.end()) {
		if ((int)Common::distance(objects_.begin(), it) >= current_)
			error("ObjectContainer::releaseObject(): Object released more than once in to the pool: %s", transCyrillic(name()));

		runtime->hide(obj);
		if (current_ > 0)
			swap(*it, objects_[--current_]);
		obj = 0;
	}
}

} // namespace QDEngine
