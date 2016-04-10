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

#include "gnap/gnap.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"

namespace Gnap {

enum {
	kHSPlatypus		= 0,
	kHSExit			= 1,
	kHSCoin			= 2,
	kHSToilet		= 3,
	kHSDevice		= 4
};

int GnapEngine::scene14_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 1);
	return 0x27;
}

void GnapEngine::scene14_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0);
	setHotspot(kHSExit, 0, 590, 799, 599, SF_EXIT_D_CURSOR);
	setHotspot(kHSCoin, 330, 390, 375, 440, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSToilet, 225, 250, 510, 500, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (isFlag(kGFNeedleTaken))
		_hotspots[kHSCoin]._flags = SF_DISABLED;
	_hotspotsCount = 5;
}

void GnapEngine::scene14_run() {
	_largeSprite = nullptr;

	queueInsertDeviceIcon();
	
	if (!isFlag(kGFNeedleTaken))
		_gameSys->insertSequence(0x23, 10, 0, 0, kSeqNone, 0, 0, 0);
	
	endSceneInit();
	
	if (!isFlag(kGFNeedleTaken) && invHas(kItemTongs))
		_largeSprite = _gameSys->createSurface(1);

	if (!isFlag(kGFNeedleTaken)) {
		_gameSys->insertSequence(0x24, 10, 0x23, 10, kSeqSyncWait, 0, 0, 0);
		_gnapSequenceId = 0x24;
		_timers[2] = getRandom(40) + 50;
	}
	
	while (!_sceneDone) {
	
		updateMouseCursor();
		updateCursorByHotspot();
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene14_updateHotspots();
			}
			break;

		case kHSExit:
			_sceneDone = true;
			_newSceneNum = 13;
			break;

		case kHSCoin:
			if (_grabCursorSpriteIndex == kItemTongs) {
				invAdd(kItemQuarter);
				setFlag(kGFNeedleTaken);
				setGrabCursorSprite(-1);
				hideCursor();
				_gameSys->setAnimation(0x26, 10, 0);
				_gameSys->insertSequence(0x26, 10, _gnapSequenceId, 10, kSeqSyncWait, 0, 0, 0);
			} else if (_grabCursorSpriteIndex >= 0) {
				playSound(0x108E9, 0);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					playSound(0x108E9, 0);
					break;
				case GRAB_CURSOR:
					_gameSys->insertSequence(0x25, 10, _gnapSequenceId, 10, kSeqSyncWait, 0, 0, 0);
					_gameSys->insertSequence(0x23, 10, 0x25, 10, kSeqSyncWait, 0, 0, 0);
					_gnapSequenceId = 0x23;
					break;
				case TALK_CURSOR:
					playSound((getRandom(5) + 0x8D5) | 0x10000, 0);
					break;
				case PLAT_CURSOR:
					_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _gnapGridX, 576 - _gnapGridY);
					break;
				}
			}
			break;

		case kHSToilet:
			if (_grabCursorSpriteIndex >= 0) {
				_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _gnapGridX, 576 - _gnapGridY);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
				case GRAB_CURSOR:
					playSound(0x108B1, 0);
					break;
				case TALK_CURSOR:
					playSound((getRandom(5) + 0x8D5) | 0x10000, 0);
					break;
				case PLAT_CURSOR:
					_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _gnapGridX, 576 - _gnapGridY);
					break;
				}
			}
			break;

		default:
			_mouseClickState._left = false;
			break;

		}
	
		scene14_updateAnimations();
	
		checkGameKeys();
	
		if (!isFlag(kGFNeedleTaken) && !_timers[2]) {
			_gameSys->insertSequence(0x24, 10, _gnapSequenceId, 10, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0x24;
			_timers[2] = getRandom(40) + 50;
		}
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene14_updateHotspots();
		}
		
		gameUpdateTick();

	}
	
	if (_largeSprite)
		deleteSurface(&_largeSprite);

}

void GnapEngine::scene14_updateAnimations() {
	
	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		_gameSys->insertSpriteDrawItem(_largeSprite, 0, 0, 300);
		_gameSys->setAnimation(0x10843, 301, 1);
		_gameSys->insertSequence(0x10843, 301, 0x26, 10, kSeqSyncWait, 0, 0, 0);
	}
	
	if (_gameSys->getAnimationStatus(1) == 2) {
		_gameSys->setAnimation(0, 0, 1);
		_sceneDone = true;
		_newSceneNum = 13;
		_grabCursorSpriteIndex = kItemQuarter;
	}
	
}

} // End of namespace Gnap
