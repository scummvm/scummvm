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
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/environ/scene_base.h"
#include "buried/environ/scene_common.h"

#include "common/system.h"
#include "graphics/surface.h"

namespace Buried {

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
	case 12:
		return new NormalTransporter(_vm, viewWindow, sceneStaticData, priorLocation);
	case 13:
		return new EntryWithoutLensFilter(_vm, viewWindow, sceneStaticData, priorLocation);
	case 30:
		return new PlayPodAudio(_vm, viewWindow, sceneStaticData, priorLocation, 9, 10);
	case 31:
		return new PlayPodAudio(_vm, viewWindow, sceneStaticData, priorLocation, 11, 12);
	case 50:
		return new PlayStingers(_vm, viewWindow, sceneStaticData, priorLocation, 127, offsetof(GlobalFlags, asRBLastStingerID), offsetof(GlobalFlags, asRBStingerID), 10, 14);
	}

	warning("TODO: Alien scene object %d", sceneStaticData.classID);
	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
