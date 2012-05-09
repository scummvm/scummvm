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
 * $URL$
 * $Id$
 *
 */

#include "prisoner/prisoner.h"
#include "prisoner/resource.h"
#include "prisoner/screen.h"

namespace Prisoner {

/* Background */

void PrisonerEngine::loadBackground(Common::String &pakName, int16 pakSlot) {

	_backgroundResourceCacheSlot = _res->load<PictureResource>(pakName, pakSlot, 1);
	_backgroundResource = _res->get<PictureResource>(_backgroundResourceCacheSlot);

	_backgroundWidth = _backgroundResource->getSurface()->w;
	_backgroundHeight = _backgroundResource->getSurface()->h;

	if (_backgroundWidth >= 640) {
		_backgroundDrawX = 0;
		_backgroundNoScrollFlag1 = false;
	} else {
		_backgroundDrawX = 160;
		_backgroundNoScrollFlag1 = true;
	}

	if (_backgroundHeight >= 316) {
		_backgroundDrawY = 82;
		_backgroundNoScrollFlag1 = false;
	} else {
		_backgroundDrawY = 141;
		_backgroundNoScrollFlag1 = true;
	}

	_backgroundNoScrollFlag2 = (_backgroundWidth != 640 || _backgroundHeight != 316) && !_backgroundNoScrollFlag1;

	initBackgroundDrawDimensions();

}

void PrisonerEngine::unloadBackground() {
	if (_backgroundResourceCacheSlot != -1) {
		_res->unload(_backgroundResourceCacheSlot);
		_backgroundResourceCacheSlot = -1;
	}
}

void PrisonerEngine::setBackground(Common::String &pakName, int16 pakSlot, int16 backgroundFlag) {
	unloadBackground();
	resetBackgroundValues();
	loadBackground(pakName, pakSlot);
	updateBackground(true);
	_backgroundFlag = backgroundFlag;
}

void PrisonerEngine::clearBackground(int16 backgroundFlag) {
	_backgroundFlag = backgroundFlag;
	unloadBackground();
	resetBackgroundValues();
	_screen->clear();
	_clearBackgroundFlag = true;
}

void PrisonerEngine::initBackgroundDrawDimensions() {
	if (_backgroundWidth >= 640)
		_backgroundDrawWidth = 640;
	else
		_backgroundDrawWidth = _backgroundWidth;
	if (_backgroundHeight >= 316)
		_backgroundDrawHeight = 316;
	else
		_backgroundDrawHeight = _backgroundHeight;
	// CHECKME g_backgroundAlways01 = 0;
	// CHECKME g_backgroundAlways02 = 0;
}

void PrisonerEngine::updateCameraPosition(int16 x, int16 y) {
	if (_backgroundNoScrollFlag1) {
		_cameraY = 0;
		_cameraX = 0;
	} else {
		_cameraX = x - 320;
		if (_cameraX + _backgroundDrawWidth > _backgroundWidth)
			_cameraX = _backgroundWidth - _backgroundDrawWidth;
		if (_cameraX < 0)
			_cameraX = 0;
		_cameraY = y - 240;
		if (_cameraY + _backgroundDrawHeight > _backgroundHeight)
			_cameraY = _backgroundHeight - _backgroundDrawHeight;
		if ( _cameraY < 0)
			_cameraY = 0;
	}
}

void PrisonerEngine::setBackgroundCameraLocked(bool value) {
	_backgroundCameraLocked = value;
}

void PrisonerEngine::updateBackground(bool fullRedraw) {

	if ((!_updateScreenValue && _backgroundNoScrollFlag2) || _backgroundNoScrollFlag1)
		fullRedraw = true;

	if (_backgroundResourceCacheSlot == -1) {
		if (fullRedraw) {
			addDirtyRect(_backgroundDrawX, _backgroundDrawY, _backgroundDrawWidth, _backgroundDrawHeight, 1);
			_screen->clear();
		}
	} else {
		if (_cameraFocusActor) {
			ActorSprite *actorSprite = _actors[_cameraFollowsActorIndex].actorSprite;
			_cameraFocusActor = false;
			updateCameraPosition(actorSprite->x, actorSprite->y);
			fullRedraw = true;
		}
		if (fullRedraw) {
			addDirtyRect(_backgroundDrawX, _backgroundDrawY, _backgroundDrawWidth, _backgroundDrawHeight, 1);
			_screen->copyRectFrom(_backgroundResource->getSurface(), _cameraX, _cameraY,
				_backgroundDrawX, _backgroundDrawY, _backgroundDrawWidth, _backgroundDrawHeight);
		}
	}

}

void PrisonerEngine::resetBackgroundValues() {
	_backgroundResourceCacheSlot = -1;
	_backgroundWidth = 0;
	_backgroundHeight = 0;
	_cameraX = 0;
	_cameraY = 0;
	_backgroundCameraLocked = false;
}

void PrisonerEngine::updateCameraFollowing() {

	if (_cameraFollowsActorIndex == -1 || _backgroundResourceCacheSlot == -1 ||
		_backgroundCameraLocked || !_backgroundNoScrollFlag2)
		return;

	int16 scrollX = _backgroundWidth / 10;
	int16 scrollY = _backgroundHeight / 10;
	int16 absActorX = _actors[_cameraFollowsActorIndex].actorSprite->x - _cameraX + _cameraDeltaX - _backgroundDrawX;
	int16 absActorY = _actors[_cameraFollowsActorIndex].actorSprite->y - _cameraY + _cameraDeltaY - _backgroundDrawY;
	int16 origCameraX = _cameraX;
	int16 origCameraY = _cameraY;

	if (absActorX > _backgroundDrawWidth - scrollX) {
		_cameraX += _backgroundDrawWidth / 2;
		if (_cameraX > _backgroundWidth - _backgroundDrawWidth)
			_cameraX = _backgroundWidth - _backgroundDrawWidth;
	} else {
		if (absActorX < scrollX)
			_cameraX -= _backgroundDrawWidth / 2;
		if (_cameraX < 0)
			_cameraX = 0;
	}

	if (absActorY > _backgroundDrawHeight - scrollY) {
		_cameraY += _backgroundDrawHeight / 2;
		if (_cameraY > _backgroundHeight - _backgroundDrawHeight)
			_cameraY = _backgroundHeight - _backgroundDrawHeight;
	} else {
		if (absActorY < scrollY)
			_cameraY -= _backgroundDrawHeight / 2;
		if (_cameraY < 0)
			_cameraY = 0;
	}

	if (_cameraX != origCameraX || _cameraY != origCameraY)
		updateBackground(true);

}

void PrisonerEngine::setCameraFollowsActor(int16 actorIndex) {
	_cameraFollowsActorIndex = actorIndex;
	_cameraFocusActor = true;
}

void PrisonerEngine::updateScreen(bool fullRedraw, int16 x, int16 y) {
	updateCameraFollowing();
	updateBackground(fullRedraw);
	_screen->setClipRect(0, 82, 639, 397);
	drawSprites(_cameraX, _cameraY);
	_screen->setClipRect(0, 0, 639, 479);
	updatePaletteTasks();
	updateActorFrameSounds();
	updateScreenTexts();
	if (x != -1 && y != -1)
		updateCurrZone(x, y);
}

void PrisonerEngine::updateCurrZone(int16 x, int16 y) {

	switch (_currMouseCursor) {

	case kCursorDefault:
	case kCursorMove:
	case kCursorDoor:
	case kCursorAct:
	case kCursorAutomatic:
	case kCursorItem:
		updateZones(x, y);
		break;

	case kCursorLook:
		if (_mainActorIndex != -1 && _exitZoneActionFlag) {
			Actor *actor = &_actors[_mainActorIndex];
			actor->walkDestX = x;
			actor->walkDestY = y;
			setActorLookInDirection(_mainActorIndex);
		}
		updateZones(x, y);
		break;

	case kCursorDialog:
		updateDialog(x, y);
		break;

	default:
		break;
	}

}

} // End of namespace Prisoner
