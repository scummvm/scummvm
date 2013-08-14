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

namespace Fullpipe {

int handleObjectInteraction(GameObject *subject, GameObject *object, int invId) {
	return getGameLoaderInteractionController()->handleInteraction(subject, object, invId);
}

bool CInteractionController::load(MfcArchive &file) {
	debug(5, "CInteractionController::load()");

	return _interactions.load(file);
}

void CInteractionController::sortInteractions(int sceneId) {
	warning("STUB: CInteractionController::sortInteractions(%d)", sceneId);
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
