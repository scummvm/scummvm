/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
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

#include "buried/buried.h"
#include "buried/graphics.h"
#include "buried/invdata.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/environ/scene_base.h"
#include "buried/environ/scene_common.h"

#include "common/system.h"
#include "graphics/surface.h"

namespace Buried {

class ArmControls : public SceneBase {
public:
	ArmControls(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _controls[3];
	int _animIDs[3];
};

ArmControls::ArmControls(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_controls[0] = Common::Rect(124, 150, 140, 164);
	_controls[1] = Common::Rect(145, 146, 161, 160);
	_controls[2] = Common::Rect(155, 162, 165, 172);
	_animIDs[0] = 3;
	_animIDs[1] = 4;
	_animIDs[2] = 5;
}

int ArmControls::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	for (int i = 0; i < 3; i++) {
		if (_controls[i].contains(pointLocation)) {
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animIDs[i]);
			return SC_TRUE;
		}
	}

	return SC_FALSE;
}

int ArmControls::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	for (int i = 0; i < 3; i++)
		if (_controls[i].contains(pointLocation))
			return kCursorFinger;

	return kCursorArrow;
}

class OpenAlienDoorA : public SceneBase {
public:
	OpenAlienDoorA(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
};

OpenAlienDoorA::OpenAlienDoorA(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int OpenAlienDoorA::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asDangerDoorASealed != 1 && ((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 0) {
		DestinationScene destData;
		destData.destinationScene = _staticData.location;
		destData.destinationScene.depth = 1;
		destData.transitionType = TRANSITION_VIDEO;
		destData.transitionData = 16;
		destData.transitionStartFrame = -1;
		destData.transitionLength = -1;

		Common::String text = _vm->getString(IDS_AS_RA_BEINGS_DETECTED_20_METERS);
		((SceneViewWindow *)viewWindow)->moveToDestination(destData);
		((SceneViewWindow *)viewWindow)->displayLiveText(text);
		return SC_FALSE;
	}

	return SC_TRUE;
}

class AlienDoorAEncounter : public SceneBase {
public:
	AlienDoorAEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
	int postExitRoom(Window *viewWindow, const Location &newLocation);
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int timerCallback(Window *viewWindow);

private:
	uint32 _timerStart;
	LocationStaticData _originalSceneData;
	bool _cloaked;
	Common::Rect _nerve;
};

AlienDoorAEncounter::AlienDoorAEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_timerStart = 0;
	_cloaked = false;
	_nerve = Common::Rect(184, 160, 268, 189);

	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asTakenEvidenceThisTrip == 1) {
		_originalSceneData = _staticData;
		_staticData.destUp.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destLeft.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destRight.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destDown.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destForward.destinationScene = Location(-1, -1, -1, -1, -1, -1);
	}
}

int AlienDoorAEncounter::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asTakenEvidenceThisTrip == 1)
		_timerStart = g_system->getMillis();

	return SC_TRUE;
}

int AlienDoorAEncounter::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (_staticData.location.depth != newLocation.depth && _staticData.location.timeZone == newLocation.timeZone && _staticData.location.node == newLocation.node)
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 7), 128, false, true);

	return SC_TRUE;
}

int AlienDoorAEncounter::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (itemID == kItemRichardsSword && ((SceneViewWindow *)viewWindow)->getGlobalFlags().asTakenEvidenceThisTrip == 1 && _nerve.contains(pointLocation))
		return 1;

	return 0;
}

int AlienDoorAEncounter::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (itemID == kItemRichardsSword && ((SceneViewWindow *)viewWindow)->getGlobalFlags().asTakenEvidenceThisTrip == 1 && _nerve.contains(pointLocation)) {
		// Reset the nerve flag
		((SceneViewWindow *)viewWindow)->getGlobalFlags().asDangerDoorASealed = 1;

		DestinationScene destData;
		destData.destinationScene = _staticData.location;
		destData.destinationScene.depth = 0;
		destData.transitionType = TRANSITION_VIDEO;
		destData.transitionData = 11;
		destData.transitionStartFrame = -1;
		destData.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(destData);
	}

	return SIC_REJECT;
}

int AlienDoorAEncounter::timerCallback(Window *viewWindow) {
	if (_timerStart != 0) {
		if (_cloaked) {
			if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcCloakingEnabled == 0) {
				_cloaked = false;
				_timerStart = g_system->getMillis();
			}
		} else {
			if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcCloakingEnabled == 1) {
				_cloaked = true;
			} else {
				if (_timerStart + 15000 < g_system->getMillis()) {
					((SceneViewWindow *)viewWindow)->playSynchronousAnimation(12);
					((SceneViewWindow *)viewWindow)->showDeathScene(50);
					return SC_DEATH;
				}
			}
		}
	}

	return SC_TRUE;
}

class AlienDoorAMoveDeath : public SceneBase {
public:
	AlienDoorAMoveDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int timerCallback(Window *viewWindow);
};

AlienDoorAMoveDeath::AlienDoorAMoveDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int AlienDoorAMoveDeath::timerCallback(Window *viewWindow) {
	((SceneViewWindow *)viewWindow)->showDeathScene(50);
	return SC_DEATH;
}

class AlienDoorBOpen : public SceneBase {
public:
	AlienDoorBOpen(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
};

AlienDoorBOpen::AlienDoorBOpen(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int AlienDoorBOpen::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asDangerDoorASealed != 1) {
		DestinationScene destData;
		destData.destinationScene = _staticData.location;
		destData.destinationScene.depth = 1;
		destData.transitionType = TRANSITION_VIDEO;
		destData.transitionData = (((SceneViewWindow *)viewWindow)->getGlobalFlags().asDoorBGuardsSeen == 1) ? 15 : 13;
		destData.transitionStartFrame = -1;
		destData.transitionLength = -1;

		((SceneViewWindow *)viewWindow)->moveToDestination(destData);
		return SC_FALSE;
	}

	return SC_TRUE;
}

class AlienDoorBEncounter : public SceneBase {
public:
	AlienDoorBEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
	int postExitRoom(Window *viewWindow, const Location &newLocation);
	int timerCallback(Window *viewWindow);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	uint32 _timerStart;
	LocationStaticData _originalSceneData;
	Common::Rect _nerve;
	Location _forwardLocation;
};

AlienDoorBEncounter::AlienDoorBEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_timerStart = 0;
	_nerve = Common::Rect(154, 155, 256, 189);
	_forwardLocation = _staticData.destForward.destinationScene;
	_staticData.destForward.destinationScene = Location(-1, -1, -1, -1, -1, -1);

	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asDoorBGuardsSeen == 0) {
		_originalSceneData = _staticData;
		_staticData.destUp.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destLeft.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destRight.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destDown.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destForward.destinationScene = Location(-1, -1, -1, -1, -1, -1);
	} else {
		_staticData.navFrameIndex = 122;
	}
}

int AlienDoorBEncounter::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asDoorBGuardsSeen == 0) {
		_timerStart = g_system->getMillis();
		((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_AS_RA_BEINGS_DETECTED_20_METERS));
	}

	return SC_TRUE;
}

int AlienDoorBEncounter::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (_staticData.location.depth != newLocation.depth && _staticData.location.timeZone == newLocation.timeZone && _staticData.location.node == newLocation.node)
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 7), 128, false, true);

	return SC_TRUE;
}

int AlienDoorBEncounter::timerCallback(Window *viewWindow) {
	if (_timerStart != 0 && (_timerStart + 15000 < g_system->getMillis() || ((SceneViewWindow *)viewWindow)->getGlobalFlags().bcCloakingEnabled == 1)) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcCloakingEnabled == 0) {
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(9);
			((SceneViewWindow *)viewWindow)->showDeathScene(50);
		} else {
			_staticData = _originalSceneData;
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(8);
			_staticData.navFrameIndex = 122;
			((SceneViewWindow *)viewWindow)->getGlobalFlags().asDoorBGuardsSeen = 1;
			_timerStart = 0;
			_staticData.destForward.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		}
	}

	return SC_TRUE;
}

int AlienDoorBEncounter::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asDoorBGuardsSeen == 1 && _nerve.contains(pointLocation)) {
		_staticData.destForward.destinationScene = _forwardLocation;
		((SceneViewWindow *)viewWindow)->moveToDestination(_staticData.destForward);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int AlienDoorBEncounter::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asDoorBGuardsSeen == 1 && _nerve.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class NormalTransporter : public SceneBase {
public:
	NormalTransporter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _clickRegion;
};

NormalTransporter::NormalTransporter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_clickRegion = Common::Rect(118, 93, 153, 125);
}

int NormalTransporter::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation)) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asAmbassadorEncounter == 1) {
			_vm->_sound->setAmbientSound();
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(1);
			_staticData.navFrameIndex = -1;
			((SceneViewWindow *)viewWindow)->showCompletionScene();
		} else {
			DestinationScene destData;
			destData.destinationScene = Location(3, 2, 4, 0, 1, 0);
			destData.transitionType = TRANSITION_VIDEO;
			destData.transitionData = 1;
			destData.transitionStartFrame = -1;
			destData.transitionLength = -1;
			((SceneViewWindow *)viewWindow)->moveToDestination(destData);
			return SC_TRUE;
		}
	}

	return SC_FALSE;
}

int NormalTransporter::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class EntryWithoutLensFilter : public SceneBase {
public:
	EntryWithoutLensFilter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
	int paint(Window *viewWindow, Graphics::Surface *preBuffer);
	int timerCallback(Window *viewWindow);

private:
	bool _transPlayed;
};

EntryWithoutLensFilter::EntryWithoutLensFilter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_transPlayed = false;
}

int EntryWithoutLensFilter::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

	Common::String text;
	if (_vm->getVersion() >= MAKEVERSION(1, 0, 4, 0))
		text = _vm->getString(IDS_AS_VISIBILITY_MESSAGE);
	else
		text = "Alert: No visible light waves detected. Jumpsuit's built-in light source ineffective.";

	((SceneViewWindow *)viewWindow)->displayLiveText(text);
	return SC_TRUE;
}

int EntryWithoutLensFilter::paint(Window *viewWindow, Graphics::Surface *preBuffer) {
	preBuffer->fillRect(Common::Rect(432, 189), _vm->_gfx->getColor(0, 0, 0));
	return SC_REPAINT;
}

int EntryWithoutLensFilter::timerCallback(Window *viewWindow) {
	if (!_transPlayed && ((SceneViewWindow *)viewWindow)->getGlobalFlags().lensFilterActivated == 1) {
		_transPlayed = true;
		DestinationScene destData;
		destData.destinationScene = _staticData.location;
		destData.destinationScene.depth = 0;
		destData.transitionType = TRANSITION_NONE;
		destData.transitionData = -1;
		destData.transitionStartFrame = -1;
		destData.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(destData);
	}

	return SC_TRUE;
}

class PlayPodAudio : public SceneBase {
public:
	PlayPodAudio(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation, int untransAudio, int transSoundID);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);

private:
	int _untransSoundID;
	int _transSoundID;
};

PlayPodAudio::PlayPodAudio(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation, int untransAudio, int transSoundID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_untransSoundID = untransAudio;
	_transSoundID = transSoundID;
}

int PlayPodAudio::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		if (_transSoundID >= 0)
			_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _transSoundID));
	} else {
		if (_untransSoundID >= 0)
			_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _untransSoundID));
	}

	return SC_TRUE;
}

class InorganicPodTransDeath : public SceneBase {
public:
	InorganicPodTransDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left = -1, int top = -1, int right = -1, int bottom = -1, int animID = -1, int deathScene = -1);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _clickRegion;
	int _transportAnimID;
	int _deathScene;
};

InorganicPodTransDeath::InorganicPodTransDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom, int animID, int deathScene) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_clickRegion = Common::Rect(left, top, right, bottom);
	_transportAnimID = animID;
	_deathScene = deathScene;
}

int InorganicPodTransDeath::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation)) {
		_vm->_sound->setAmbientSound();
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_transportAnimID);
		_staticData.navFrameIndex = -1;
		((SceneViewWindow *)viewWindow)->showDeathScene(_deathScene);
	}

	return SC_FALSE;
}

int InorganicPodTransDeath::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

bool SceneViewWindow::initializeAlienTimeZoneAndEnvironment(Window *viewWindow, int environment) {
	if (environment == -1) {
		GlobalFlags &flags = ((SceneViewWindow *)viewWindow)->getGlobalFlags();

		flags.asInitialGuardsPass = 0;
		flags.asRBPodAStatus = 0;
		flags.asRBPodBStatus = 0;
		flags.asRBPodCStatus = 0;
		flags.asRBPodDStatus = 0;
		flags.asRBPodEStatus = 0;
		flags.asRBPodFStatus = 0;
	} else if (environment == 1) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreTransportToKrynn = 1;
	}

	return true;
}

bool SceneViewWindow::startAlienAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade) {
	_vm->_sound->setAmbientSound(_vm->getFilePath(7, environment, SF_AMBIENT), oldTimeZone == 7 && fade, 64);
	return true;
}


SceneBase *SceneViewWindow::constructAlienSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	// TODO

	switch (sceneStaticData.classID) {
	case 1:
		return new ArmControls(_vm, viewWindow, sceneStaticData, priorLocation);
	case 4:
		return new OpenAlienDoorA(_vm, viewWindow, sceneStaticData, priorLocation);
	case 5:
		return new AlienDoorAEncounter(_vm, viewWindow, sceneStaticData, priorLocation);
	case 6:
		return new AlienDoorAMoveDeath(_vm, viewWindow, sceneStaticData, priorLocation);
	case 7:
		return new AlienDoorBOpen(_vm, viewWindow, sceneStaticData, priorLocation);
	case 8:
		return new AlienDoorBEncounter(_vm, viewWindow, sceneStaticData, priorLocation);
	case 12:
		return new NormalTransporter(_vm, viewWindow, sceneStaticData, priorLocation);
	case 13:
		return new EntryWithoutLensFilter(_vm, viewWindow, sceneStaticData, priorLocation);
	case 30:
		return new PlayPodAudio(_vm, viewWindow, sceneStaticData, priorLocation, 9, 10);
	case 31:
		return new PlayPodAudio(_vm, viewWindow, sceneStaticData, priorLocation, 11, 12);
	case 32:
		return new InorganicPodTransDeath(_vm, viewWindow, sceneStaticData, priorLocation, 92, 88, 158, 128, 2, 52);
	case 33:
		return new InorganicPodTransDeath(_vm, viewWindow, sceneStaticData, priorLocation, 92, 88, 158, 128, 26, 53);
	case 50:
		return new PlayStingers(_vm, viewWindow, sceneStaticData, priorLocation, 127, offsetof(GlobalFlags, asRBLastStingerID), offsetof(GlobalFlags, asRBStingerID), 10, 14);
	}

	warning("TODO: Alien scene object %d", sceneStaticData.classID);
	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
