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

#include "fullpipe/fullpipe.h"

#include "fullpipe/interaction.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/statics.h"

namespace Fullpipe {

int handleObjectInteraction(GameObject *subject, GameObject *object, int invId) {
	return getGameLoaderInteractionController()->handleInteraction(subject, object, invId);
}

bool CInteractionController::load(MfcArchive &file) {
	debug(5, "CInteractionController::load()");

	return _interactions.load(file);
}

int static_compSceneId = 0;

bool CInteractionController::compareInteractions(const void *p1, const void *p2) {
	const CInteraction *i1 = (const CInteraction *)p1;
	const CInteraction *i2 = (const CInteraction *)p2;

	if ( i2->_sceneId < i1->_sceneId) {
		if ( i1->_sceneId != static_compSceneId)
			return false;
	}
	if (i2->_sceneId != i1->_sceneId) {
		if (i1->_sceneId > 0 && i2->_sceneId == static_compSceneId)
			return false;
		if (i2->_sceneId != i1->_sceneId)
			return true;
	}
	if (i2->_objectId3 == -1)
		goto LABEL_17;
	if (i2->_objectId3 == -2)
		goto LABEL_18;
	if (i1->_objectId3 != -1 && i1->_objectId3 != -2) {
LABEL_17:
		if (i2->_objectId3 != -2 )
			return true;
LABEL_18:
		if (i1->_objectId3 != -1)
			return true;
	}

	return false;
}

void CInteractionController::sortInteractions(int sceneId) {
	static_compSceneId = sceneId;

	Common::sort(_interactions.begin(), _interactions.end(), CInteractionController::compareInteractions);
}

int CInteractionController::handleInteraction(GameObject *subject, GameObject *object, int invId) {
	warning("STUB: CInteractionController::handleInteraction()");

	return 0;
}

CInteraction::CInteraction() {
	_objectId1 = 0;
	_objectId2 = 0;
	_staticsId1 = 0;
	_objectId3 = 0;
	_objectState2 = 0;
	_objectState1 = 0;
	_messageQueue = 0;
	_flags = 0;
	_yOffs = 0;
	_xOffs = 0;
	_staticsId2 = 0;
	_field_28 = 0;
	_sceneId = -1;
	_actionName = 0;
}

bool CInteraction::load(MfcArchive &file) {
	debug(5, "CInteraction::load()");

	_objectId1 = file.readUint16LE();
	_objectId2 = file.readUint16LE();
	_staticsId1 = file.readUint16LE();
	_staticsId2 = file.readUint16LE();
	_objectId3 = file.readUint16LE();
	_objectState2 = file.readUint32LE();
	_objectState1 = file.readUint32LE();
	_xOffs = file.readUint32LE();
	_yOffs = file.readUint32LE();
	_sceneId = file.readUint32LE();
	_flags = file.readUint32LE();
	_actionName = file.readPascalString();

	_messageQueue = (MessageQueue *)file.readClass();

	return true;
}

bool CInteraction::canInteract(GameObject *obj1, GameObject *obj2, int invId) {
	if (_sceneId > 0 && g_fullpipe->_currentScene && g_fullpipe->_currentScene->_sceneId != _sceneId)
		return false;

	if (_flags & 0x20000)
		return false;

	if (!obj2)
		return false;
	if (obj2->_id != _objectId1)
		return false;

	if ((_flags & 8) && (_flags & 1)) {
		if (!obj2->_objtype != kObjTypeStaticANIObject)
			return false;

		StaticANIObject *st = (StaticANIObject *)obj2;

		if (!st->_statics)
			return false;

		if (st->_statics->_staticsId != _staticsId1) {
			if (_staticsId1)
				return false;
		}
	}

	if ((_objectId3 != invId && _objectId3 != -1 && _objectId3 != -2) || (!invId && _objectId3 == -2))
		return false;

	if (_objectState1) {
		if (_flags & 0x10) {
			if ((g_fullpipe->getObjectState(obj1->getName()) & _objectState1) == 0)
				return false;
		} else {
			if (g_fullpipe->getObjectState(obj1->getName()) != _objectState1)
				return false;
		}
	}

	if (_objectState2) {
		if (_flags & 0x10) {
			if ((g_fullpipe->getObjectState(obj2->getName()) & _objectState2) == 0)
				return false;
		} else {
			if (g_fullpipe->getObjectState(obj2->getName()) != _objectState2)
				return false;
		}
	}

	if (_objectId2 && (!obj1 || _objectId2 != obj1->_id))
		return false;

	return true;
}

bool EntranceInfo::load(MfcArchive &file) {
	debug(5, "EntranceInfo::load()");

	_sceneId = file.readUint32LE();
	_field_4 = file.readUint32LE();
	_messageQueueId = file.readUint32LE();
	file.read(_gap_C, 292); // FIXME, Ugh
	_field_130 = file.readUint32LE();

	return true;
}

} // End of namespace Fullpipe
