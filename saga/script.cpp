/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

// Scripting module: Script resource handling functions
#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/console.h"

#include "saga/script.h"
#include "saga/stream.h"
#include "saga/interface.h"
#include "saga/itedata.h"
#include "saga/scene.h"
#include "saga/events.h"
#include "saga/actor.h"
#include "saga/objectmap.h"
#include "saga/isomap.h"
#include "saga/rscfile.h"

namespace Saga {

// Initializes the scripting module.
// Loads script resource look-up table, initializes script data system
Script::Script(SagaEngine *vm) : _vm(vm) {
	ResourceContext *resourceContext;
	byte *resourcePointer;
	size_t resourceLength;
	int prevTell;
	int i, j;
	byte *stringsPointer;
	size_t stringsLength;

	//initialize member variables
	_abortEnabled = true;
	_skipSpeeches = false;
	_conversingThread = NULL;

	_firstObjectSet = false;
	_secondObjectNeeded = false;
	_pendingVerb = kVerbNone;
	_currentVerb = kVerbNone;
	_stickyVerb = kVerbWalkTo;
	_leftButtonVerb = kVerbNone;
	_rightButtonVerb = kVerbNone;
	_pointerObject = ID_NOTHING;

	_staticSize = 0;
	_commonBufferSize = COMMON_BUFFER_SIZE;
	_commonBuffer = (byte*)malloc(_commonBufferSize);
	memset(_commonBuffer, 0, _commonBufferSize);

	debug(8, "Initializing scripting subsystem");
	// Load script resource file context
	_scriptContext = _vm->_resource->getContext(GAME_SCRIPTFILE);
	if (_scriptContext == NULL) {
		error("Script::Script() script context not found");
	}

	resourceContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
	if (resourceContext == NULL) {
		error("Script::Script() resource context not found");
	}

	debug(3, "Loading module LUT from resource %i", _vm->getResourceDescription()->moduleLUTResourceId);
	_vm->_resource->loadResource(resourceContext, _vm->getResourceDescription()->moduleLUTResourceId, resourcePointer, resourceLength);


	// Create logical script LUT from resource
	if (resourceLength % S_LUT_ENTRYLEN_ITECD == 0) {
		_modulesLUTEntryLen = S_LUT_ENTRYLEN_ITECD;
	} else if (resourceLength % S_LUT_ENTRYLEN_ITEDISK == 0) {
		_modulesLUTEntryLen = S_LUT_ENTRYLEN_ITEDISK;
	} else {
		error("Script::Script() Invalid script lookup table length (%i)", resourceLength);
	}

	// Calculate number of entries
	_modulesCount = resourceLength / _modulesLUTEntryLen;

	debug(3, "LUT has %i entries", _modulesCount);

	// Allocate space for logical LUT
	_modules = (ModuleData *)malloc(_modulesCount * sizeof(*_modules));
	if (_modules == NULL) {
		memoryError("Script::Script()");
	}

	// Convert LUT resource to logical LUT
	MemoryReadStreamEndian scriptS(resourcePointer, resourceLength, resourceContext->isBigEndian);
	for (i = 0; i < _modulesCount; i++) {
		memset(&_modules[i], 0, sizeof(ModuleData));

		prevTell = scriptS.pos();
		_modules[i].scriptResourceId = scriptS.readUint16();
		_modules[i].stringsResourceId = scriptS.readUint16();
		_modules[i].voicesResourceId = scriptS.readUint16();

		// Skip the unused portion of the structure
		for (j = scriptS.pos(); j < prevTell + _modulesLUTEntryLen; j++) {
			if (scriptS.readByte() != 0)
				warning("Unused scriptLUT part isn't really unused for LUT %d (pos: %d)", i, j);
		}
	}

	free(resourcePointer);

	// TODO
	//
	// In ITE, the "main strings" resource contains both the verb strings
	// and the object names.
	//
	// In IHNM, the "main strings" contains the verb strings, but not the
	// object names. At least, I think that's the case.

	_vm->_resource->loadResource(resourceContext, _vm->getResourceDescription()->mainStringsResourceId, stringsPointer, stringsLength);

	_vm->loadStrings(_mainStrings, stringsPointer, stringsLength);
	free(stringsPointer);

	setupScriptFuncList();
}

// Shut down script module gracefully; free all allocated module resources
Script::~Script() {

	debug(8, "Shutting down scripting subsystem.");

	_mainStrings.freeMem();

	freeModules();
	free(_modules);

	free(_commonBuffer);
}

void Script::loadModule(int scriptModuleNumber) {
	byte *resourcePointer;
	size_t resourceLength;

	// Validate script number
	if ((scriptModuleNumber < 0) || (scriptModuleNumber >= _modulesCount)) {
		error("Script::loadScript() Invalid script module number");
	}

	if (_modules[scriptModuleNumber].loaded) {
		return;
	}

	// Initialize script data structure
	debug(3, "Loading script module #%d", scriptModuleNumber);

	_vm->_resource->loadResource(_scriptContext, _modules[scriptModuleNumber].scriptResourceId, resourcePointer, resourceLength);

	loadModuleBase(_modules[scriptModuleNumber], resourcePointer, resourceLength);
	free(resourcePointer);

	_vm->_resource->loadResource(_scriptContext, _modules[scriptModuleNumber].stringsResourceId, resourcePointer, resourceLength);

	_vm->loadStrings(_modules[scriptModuleNumber].strings, resourcePointer, resourceLength);
	free(resourcePointer);

	if (_modules[scriptModuleNumber].voicesResourceId > 0) {
		_vm->_resource->loadResource(_scriptContext, _modules[scriptModuleNumber].voicesResourceId, resourcePointer, resourceLength);

		loadVoiceLUT(_modules[scriptModuleNumber].voiceLUT, resourcePointer, resourceLength);
		free(resourcePointer);
	}

	_modules[scriptModuleNumber].staticOffset = _staticSize;
	_staticSize += _modules[scriptModuleNumber].staticSize;
	if (_staticSize > _commonBufferSize) {
		error("Script::loadModule() _staticSize > _commonBufferSize");
	}
	_modules[scriptModuleNumber].loaded = true;
}

void Script::freeModules() {
	int i;
	for (i = 0; i < _modulesCount; i++) {
		if (_modules[i].loaded) {
			_modules[i].freeMem();
		}
	}
	_staticSize = 0;
}

void Script::loadModuleBase(ModuleData &module, const byte *resourcePointer, size_t resourceLength) {
	int i;

	debug(3, "Loading module base...");

	module.moduleBase = (byte*)malloc(resourceLength);
	module.moduleBaseSize = resourceLength;

	memcpy(module.moduleBase, resourcePointer, resourceLength);

	MemoryReadStreamEndian scriptS(module.moduleBase, module.moduleBaseSize, _scriptContext->isBigEndian);

	module.entryPointsCount = scriptS.readUint16();
	scriptS.readUint16(); //skip
	module.entryPointsTableOffset = scriptS.readUint16();
	scriptS.readUint16(); //skip

	if ((module.moduleBaseSize - module.entryPointsTableOffset) < (module.entryPointsCount * SCRIPT_TBLENTRY_LEN)) {
		error("Script::loadModuleBase() Invalid table offset");
	}

	if (module.entryPointsCount > SCRIPT_MAX) {
		error("Script::loadModuleBase()Script limit exceeded");
	}

	module.entryPoints = (EntryPoint *)malloc(module.entryPointsCount * sizeof(*module.entryPoints));
	if (module.entryPoints == NULL) {
		memoryError("Script::loadModuleBase");
	}

	// Read in the entrypoint table

	module.staticSize = scriptS.readUint16();
	while (scriptS.pos() < module.entryPointsTableOffset)
		scriptS.readByte();

	for (i = 0; i < module.entryPointsCount; i++) {
		// First uint16 is the offset of the entrypoint name from the start
		// of the bytecode resource, second uint16 is the offset of the
		// bytecode itself for said entrypoint
		module.entryPoints[i].nameOffset = scriptS.readUint16();
		module.entryPoints[i].offset = scriptS.readUint16();

		// Perform a simple range check on offset values
		if ((module.entryPoints[i].nameOffset >= module.moduleBaseSize) || (module.entryPoints[i].offset >= module.moduleBaseSize)) {
			error("Script::loadModuleBase() Invalid offset encountered in script entrypoint table");
		}
	}
}

void Script::loadVoiceLUT(VoiceLUT &voiceLUT, const byte *resourcePointer, size_t resourceLength) {
	uint16 i;

	voiceLUT.voicesCount = resourceLength / 2;

	voiceLUT.voices = (uint16 *)malloc(voiceLUT.voicesCount * sizeof(*voiceLUT.voices));
	if (voiceLUT.voices == NULL) {
		error("Script::loadVoiceLUT() not enough memory");
	}

	MemoryReadStreamEndian scriptS(resourcePointer, resourceLength, _scriptContext->isBigEndian);

	for (i = 0; i < voiceLUT.voicesCount; i++) {
		voiceLUT.voices[i] = scriptS.readUint16();
	}
}

// verb
void Script::showVerb(int statusColor) {
	const char *verbName;
	const char *object1Name;
	const char *object2Name;
	char statusString[STATUS_TEXT_LEN];

	if (_leftButtonVerb == kVerbNone) {
		_vm->_interface->setStatusText("");
		return;
	}

	verbName = _mainStrings.getString(_leftButtonVerb - 1);

	if (objectTypeId(_currentObject[0]) == kGameObjectNone) {
		_vm->_interface->setStatusText(verbName, statusColor);
		return;
	}

	object1Name = _vm->getObjectName(_currentObject[0]);

	if (!_secondObjectNeeded) {
		snprintf(statusString, STATUS_TEXT_LEN, "%s %s", verbName, object1Name);
		_vm->_interface->setStatusText(statusString, statusColor);
		return;
	}


	if (objectTypeId(_currentObject[1]) != kGameObjectNone) {
		object2Name = _vm->getObjectName(_currentObject[1]);
	} else {
		object2Name = "";
	}

	if (_leftButtonVerb == kVerbGive) {
		snprintf(statusString, STATUS_TEXT_LEN, _vm->getTextString(kTextGiveTo), object1Name, object2Name);
		_vm->_interface->setStatusText(statusString, statusColor);
	} else {
		if (_leftButtonVerb == kVerbUse) {
			snprintf(statusString, STATUS_TEXT_LEN, _vm->getTextString(kTextUseWidth), object1Name, object2Name);
			_vm->_interface->setStatusText(statusString, statusColor);
		} else {
			snprintf(statusString, STATUS_TEXT_LEN, "%s %s", verbName, object1Name);
			_vm->_interface->setStatusText(statusString, statusColor);
		}
	}
}

void Script::setVerb(int verb) {
	_pendingObject[0] = ID_NOTHING;
	_currentObject[0] = ID_NOTHING;
	_pendingObject[1] = ID_NOTHING;
	_currentObject[1] = ID_NOTHING;
	_firstObjectSet = false;
	_secondObjectNeeded = false;

	// The pointer object will be updated again immediately. This way the
	// new verb will be applied to it. It's not exactly how the original
	// engine did it, but it appears to work.
	_pointerObject = ID_NOTHING;

	setLeftButtonVerb( verb );
	showVerb();
}

void Script::setLeftButtonVerb(int verb) {
	int		oldVerb = _currentVerb;

	_currentVerb = _leftButtonVerb = verb;

	if ((_currentVerb != oldVerb) && (_vm->_interface->getMode() == kPanelMain)){
			if (oldVerb > kVerbNone)
				_vm->_interface->setVerbState(oldVerb, 2);

			if (_currentVerb > kVerbNone)
				_vm->_interface->setVerbState(_currentVerb, 2);
	}
}

void Script::setRightButtonVerb(int verb) {
	int		oldVerb = _rightButtonVerb;

	_rightButtonVerb = verb;

	if ((_rightButtonVerb != oldVerb) && (_vm->_interface->getMode() == kPanelMain)){
		if (oldVerb > kVerbNone)
			_vm->_interface->setVerbState(oldVerb, 2);

		if (_rightButtonVerb > kVerbNone)
			_vm->_interface->setVerbState(_rightButtonVerb, 2);
	}
}

void Script::doVerb() {
	int scriptEntrypointNumber = 0;
	int scriptModuleNumber = 0;
	int objectType;
	Event event;
	const char *excuseText;
	int excuseSampleResourceId;
	const HitZone *hitZone;

	objectType = objectTypeId(_pendingObject[0]);

	if (_pendingVerb == kVerbGive) {
		scriptEntrypointNumber = _vm->_actor->getObjectScriptEntrypointNumber(_pendingObject[1]);
		if (_vm->_actor->getObjectFlags(_pendingObject[1]) & (kFollower|kProtagonist|kExtended)) {
			scriptModuleNumber = 0;
		} else {
			scriptModuleNumber = _vm->_scene->getScriptModuleNumber();
		}
	} else {
		if (_pendingVerb == kVerbUse) {
			if ((objectTypeId(_pendingObject[1]) > kGameObjectNone) && (objectType < objectTypeId(_pendingObject[1]))) {
				SWAP(_pendingObject[0], _pendingObject[1]);
				objectType = objectTypeId(_pendingObject[0]);
			}
		}

		if (objectType == kGameObjectHitZone) {
			scriptModuleNumber = _vm->_scene->getScriptModuleNumber();
			hitZone = _vm->_scene->_objectMap->getHitZone(objectIdToIndex(_pendingObject[0]));
			if ((hitZone->getFlags() & kHitZoneExit) == 0) {
				scriptEntrypointNumber = hitZone->getScriptNumber();
			}
		} else {
			if (objectType & (kGameObjectActor | kGameObjectObject)) {
				scriptEntrypointNumber = _vm->_actor->getObjectScriptEntrypointNumber(_pendingObject[0]);

				if ((objectType == kGameObjectActor) && !(_vm->_actor->getObjectFlags(_pendingObject[0]) & (kFollower|kProtagonist|kExtended))) {
					scriptModuleNumber = _vm->_scene->getScriptModuleNumber();
				} else {
					scriptModuleNumber = 0;
				}
			}
		}
	}

	if (scriptEntrypointNumber > 0) {

		event.type = kEvTOneshot;
		event.code = kScriptEvent;
		event.op = kEventExecNonBlocking;
		event.time = 0;
		event.param = scriptModuleNumber;
		event.param2 = scriptEntrypointNumber;
		event.param3 = _pendingVerb;		// Action
		event.param4 = _pendingObject[0];	// Object
		event.param5 = _pendingObject[1];	// With Object
		event.param6 = (objectType == kGameObjectActor) ? _pendingObject[0] : ID_PROTAG;		// Actor

		_vm->_events->queue(&event);

	} else {
		_vm->getExcuseInfo(_pendingVerb, excuseText, excuseSampleResourceId);
		if (excuseText) {
			// In Floppy versions we don't have excuse texts
			if (!(_vm->getFeatures() & GF_CD_FX))
				excuseSampleResourceId = -1;

			_vm->_actor->actorSpeech(ID_PROTAG, &excuseText, 1, excuseSampleResourceId, 0);
		}
	}

	if ((_currentVerb == kVerbWalkTo) || (_currentVerb == kVerbLookAt)) {
		_stickyVerb = _currentVerb;
	}

	_pendingVerb = kVerbNone;
	_currentObject[0] = _currentObject[1] = ID_NOTHING;
	setLeftButtonVerb(_stickyVerb);

	setPointerVerb();
}

void Script::setPointerVerb() {
	if (_vm->_interface->isActive()) {
		_pointerObject = ID_PROTAG;
		whichObject(_vm->mousePos());
	}
}

void Script::hitObject(bool leftButton) {
	int verb;
	verb = leftButton ? _leftButtonVerb : _rightButtonVerb;

	if (verb > kVerbNone) {
		if (_firstObjectSet) {
			if (_secondObjectNeeded) {
				_pendingObject[0] = _currentObject[0];
				_pendingObject[1] = _currentObject[1];
				_pendingVerb = verb;

				_leftButtonVerb = verb;
				if (_pendingVerb > kVerbNone)
					showVerb(kITEColorBrightWhite);
				else
					showVerb();

				_secondObjectNeeded = false;
				_firstObjectSet = false;
				return;
			}
		} else {
			if (verb == kVerbGive) {
				_secondObjectNeeded = true;
			} else {
				if (verb == kVerbUse) {

					if (_currentObjectFlags[0] & kObjUseWith) {
						_secondObjectNeeded = true;
					}
				}
			}

			if (!_secondObjectNeeded) {
				_pendingObject[0] = _currentObject[0];
				_pendingObject[1] = ID_NOTHING;
				_pendingVerb = verb;

				_secondObjectNeeded = false;
				_firstObjectSet = false;
			} else {
				_firstObjectSet = true;
			}
		}

		_leftButtonVerb = verb;
		if (_pendingVerb > kVerbNone)
			showVerb(kITEColorBrightWhite);
		else
			showVerb();
	}

}

void Script::playfieldClick(const Point& mousePoint, bool leftButton) {
	Location pickLocation;
	const HitZone *hitZone;
	Point specialPoint;

	_vm->_actor->abortSpeech();

	if ((_vm->_actor->_protagonist->_currentAction != kActionWait) &&
		(_vm->_actor->_protagonist->_currentAction != kActionFreeze) &&
		(_vm->_actor->_protagonist->_currentAction != kActionWalkToLink) &&
		(_vm->_actor->_protagonist->_currentAction != kActionWalkToPoint)) {
		return;
	}
	if (_pendingVerb > kVerbNone) {
		setLeftButtonVerb(kVerbWalkTo);
	}

	if (_pointerObject != ID_NOTHING) {
		hitObject( leftButton );
	} else {
		_pendingObject[0] = ID_NOTHING;
		_pendingObject[1] = ID_NOTHING;
		_pendingVerb = kVerbWalkTo;
	}


	// tiled stuff
	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		_vm->_isoMap->screenPointToTileCoords(mousePoint, pickLocation);
	} else {
		pickLocation.fromScreenPoint(mousePoint);
	}


	hitZone = NULL;

	if (objectTypeId(_pendingObject[0]) == kGameObjectHitZone) {
		 hitZone = _vm->_scene->_objectMap->getHitZone(objectIdToIndex(_pendingObject[0]));
	} else {
		if ((_pendingVerb == kVerbUse) && (objectTypeId(_pendingObject[1]) == kGameObjectHitZone)) {
			hitZone = _vm->_scene->_objectMap->getHitZone(objectIdToIndex(_pendingObject[1]));
		}
	}

	if (hitZone != NULL) {
		if (hitZone->getFlags() & kHitZoneNoWalk) {
			_vm->_actor->actorFaceTowardsPoint(ID_PROTAG, pickLocation);
			doVerb();
			return;
		}

		if (hitZone->getFlags() & kHitZoneProject) {
			if (!hitZone->getSpecialPoint(specialPoint)) {
				// Original behaved this way and this prevents from crash
				// at ruins. See bug #1257459
				specialPoint.x = specialPoint.y = 0;
			}

			// tiled stuff
			if (_vm->_scene->getFlags() & kSceneFlagISO) {
				pickLocation.u() = specialPoint.x;
				pickLocation.v() = specialPoint.y;
				pickLocation.z = _vm->_actor->_protagonist->_location.z;
			} else {
				pickLocation.fromScreenPoint(specialPoint);
			}
		}
	}

	switch (_pendingVerb) {
	case kVerbWalkTo:
	case kVerbPickUp:
	case kVerbOpen:
	case kVerbClose:
	case kVerbUse:
		_vm->_actor->actorWalkTo(ID_PROTAG, pickLocation);
		break;

	case kVerbLookAt:
		if (objectTypeId(_pendingObject[0]) != kGameObjectActor ) {
			_vm->_actor->actorWalkTo(ID_PROTAG, pickLocation);
		} else {
			doVerb();
		}
		break;

	case kVerbTalkTo:
	case kVerbGive:
		doVerb();
		break;
	}
}

void Script::whichObject(const Point& mousePoint) {
	uint16 objectId;
	int16 objectFlags;
	int newRightButtonVerb;
	uint16 newObjectId;
	ActorData *actor;
	ObjectData *obj;
	Point pickPoint;
	Location pickLocation;
	int hitZoneIndex;
	const HitZone * hitZone;
	PanelButton * panelButton;

	objectId = ID_NOTHING;
	objectFlags = 0;
	_leftButtonVerb = _currentVerb;
	newRightButtonVerb = kVerbNone;

	if (_vm->_actor->_protagonist->_currentAction != kActionWalkDir) {
		if (_vm->_scene->getHeight() >= mousePoint.y) {
			newObjectId = _vm->_actor->hitTest(mousePoint, true);

			if (newObjectId != ID_NOTHING) {
				if (objectTypeId(newObjectId) == kGameObjectObject) {
					objectId = newObjectId;
					objectFlags = 0;
					newRightButtonVerb = kVerbLookAt;

					if ((_currentVerb == kVerbTalkTo) || ((_currentVerb == kVerbGive) && _firstObjectSet)) {
						objectId = ID_NOTHING;
						newObjectId = ID_NOTHING;
					}
				} else {
					actor = _vm->_actor->getActor(newObjectId);
					objectId = newObjectId;
					objectFlags = kObjUseWith;
					newRightButtonVerb = kVerbTalkTo;

					if ((_currentVerb == kVerbPickUp) ||
						(_currentVerb == kVerbOpen) ||
						(_currentVerb == kVerbClose) ||
						((_currentVerb == kVerbGive) && !_firstObjectSet) ||
						((_currentVerb == kVerbUse) && !(actor->_flags & kFollower))) {
							objectId = ID_NOTHING;
							newObjectId = ID_NOTHING;
						}
				}
			}

			if (newObjectId == ID_NOTHING) {

				pickPoint = mousePoint;

				if (_vm->_scene->getFlags() & kSceneFlagISO) {
					pickPoint.y -= _vm->_actor->_protagonist->_location.z;
					_vm->_isoMap->screenPointToTileCoords(pickPoint, pickLocation);
					pickLocation.toScreenPointUV(pickPoint);
				}

				hitZoneIndex = _vm->_scene->_objectMap->hitTest(pickPoint);

				if ((hitZoneIndex != -1)) {
					hitZone = _vm->_scene->_objectMap->getHitZone(hitZoneIndex);
					objectId = hitZone->getHitZoneId();
					objectFlags = 0;
					newRightButtonVerb = hitZone->getRightButtonVerb() & 0x7f;

					if (newRightButtonVerb == kVerbWalkOnly) {
						if (_firstObjectSet) {
							objectId = ID_NOTHING;
						} else {
							newRightButtonVerb = _leftButtonVerb = kVerbWalkTo;
						}
					} else {
						if (newRightButtonVerb == kVerbLookOnly) {
							if (_firstObjectSet) {
								objectId = ID_NOTHING;
							} else {
								newRightButtonVerb = _leftButtonVerb = kVerbLookAt;
							}
						}
					}

					if (newRightButtonVerb >= kVerbOptions) {
						newRightButtonVerb = kVerbNone;
					}

					if ((_currentVerb == kVerbTalkTo) || ((_currentVerb == kVerbGive) && _firstObjectSet)) {
						objectId = ID_NOTHING;
						newObjectId = ID_NOTHING;
					}

					if ((_leftButtonVerb == kVerbUse) && (hitZone->getRightButtonVerb() & 0x80)) {
						objectFlags = kObjUseWith;
					}
				}
			}
		} else {
			if ((_currentVerb == kVerbTalkTo) || ((_currentVerb == kVerbGive) && _firstObjectSet)) {
				// no way
			} else {
				panelButton = _vm->_interface->inventoryHitTest(mousePoint);
				if (panelButton) {
					objectId = _vm->_interface->getInventoryContentByPanelButton(panelButton);
					if (objectId != 0) {
						obj = _vm->_actor->getObj(objectId);
						newRightButtonVerb = kVerbLookAt;
						if (obj->_interactBits & kObjUseWith) {
							objectFlags = kObjUseWith;
						}
					}
				}
			}

			if ((_currentVerb == kVerbPickUp) || (_currentVerb == kVerbTalkTo) || (_currentVerb == kVerbWalkTo)) {
				_leftButtonVerb = kVerbLookAt;
			}
		}
	}

	if (objectId != _pointerObject) {
		_pointerObject = objectId;
		_currentObject[_firstObjectSet ? 1 : 0] = objectId;
		_currentObjectFlags[_firstObjectSet ? 1 : 0] = objectFlags;
		if (_pendingVerb == kVerbNone) {
			showVerb();
		}
	}

	if (newRightButtonVerb != _rightButtonVerb) {
		setRightButtonVerb(newRightButtonVerb);
	}
}

} // End of namespace Saga
