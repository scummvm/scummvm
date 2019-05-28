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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/bbdou/illusions_bbdou.h"
#include "illusions/bbdou/bbdou_triggerfunctions.h"

namespace Illusions {

// TriggerFunction

TriggerFunction::TriggerFunction(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, TriggerFunctionCallback *callback)
	: _sceneId(sceneId), _verbId(verbId), _objectId2(objectId2), _objectId(objectId), _callback(callback) {
}

TriggerFunction::~TriggerFunction() {
	delete _callback;
}

void TriggerFunction::run(uint32 callingThreadId) {
	(*_callback)(this, callingThreadId);
}

// TriggerFunctions

void TriggerFunctions::add(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, TriggerFunctionCallback *callback) {
	ItemsIterator it = findInternal(sceneId, verbId, objectId2, objectId);
	if (it != _triggerFunctions.end()) {
		delete *it;
		_triggerFunctions.erase(it);
	}
	_triggerFunctions.push_back(new TriggerFunction(sceneId, verbId, objectId2, objectId, callback));
}

TriggerFunction *TriggerFunctions::find(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId) {
	ItemsIterator it = findInternal(sceneId, verbId, objectId2, objectId);
	if (it != _triggerFunctions.end())
		return (*it);
	return 0;
}

void TriggerFunctions::removeBySceneId(uint32 sceneId) {
	ItemsIterator it = _triggerFunctions.begin();
	while (it != _triggerFunctions.end()) {
		if ((*it)->_sceneId == sceneId) {
			delete *it;
			it = _triggerFunctions.erase(it);
		} else
			++it;
	}
}

TriggerFunctions::ItemsIterator TriggerFunctions::findInternal(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId) {
	ItemsIterator it = _triggerFunctions.begin();
	for (; it != _triggerFunctions.end(); ++it) {
		TriggerFunction *triggerFunction = *it;
		if (triggerFunction->_sceneId == sceneId && triggerFunction->_verbId == verbId &&
			triggerFunction->_objectId2 == objectId2 && triggerFunction->_objectId == objectId)
			break;
	}
	return it;
}

} // End of namespace Illusions
