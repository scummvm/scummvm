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
#include "bladerunner/mouse.h"

#include "bladerunner/actor.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/combat.h"
#include "bladerunner/debugger.h"
#include "bladerunner/dialogue_menu.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/items.h"
#include "bladerunner/regions.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/settings.h"
#include "bladerunner/shape.h"
#include "bladerunner/time.h"
#include "bladerunner/view.h"
#include "bladerunner/zbuffer.h"

#include "graphics/surface.h"

namespace BladeRunner {

Mouse::Mouse(BladeRunnerEngine *vm) {
	_vm = vm;
	_cursor = 0;
	_frame = 3;
	_hotspotX = 0;
	_hotspotY = 0;
	_x = 0;
	_y = 0;
	_disabledCounter = 0;
	_lastFrameTime = 0u;
	_animCounter = 0;

	_randomCountdownX = 0;
	_randomCountdownY = 0;
	_randomX = 0;
	_randomY = 0;

	_drawModeBitFlags = 0;
}

Mouse::~Mouse() {
}

void Mouse::setCursor(int cursor) {
	assert(cursor >= 0 && cursor <= 17);
	if (cursor == _cursor) {
		return;
	}

	_cursor = cursor;
	_drawModeBitFlags = 0;

	switch (_cursor) {
	case 0:
		// normal cursor (white)
		// (also the default init value for mouse cursor)
		_frame = 3;
		_hotspotX = 0;
		_hotspotY = 0;
		break;

	case 1:
		// normal cursor over hotspot (not exit) (green rotating)
		// animating: 8 frames (4-11)
		_frame = 4;
		_hotspotX = 0;
		_hotspotY = 0;
		if (_vm->_debugger->_useAdditiveDrawModeForMouseCursorMode0
		    || _vm->_debugger->_useAdditiveDrawModeForMouseCursorMode1) {
			_drawModeBitFlags |= MouseDrawFlags::SPECIAL;
			if (_vm->_debugger->_useAdditiveDrawModeForMouseCursorMode0) {
				_drawModeBitFlags |= MouseDrawFlags::ADDITIVE_MODE0;
			} else {
				_drawModeBitFlags |= MouseDrawFlags::ADDITIVE_MODE1;
			}
		}
		break;

	case 2:
		// static exit cursor (upwards/North)
		_frame = 12;
		_hotspotX = 12;
		_hotspotY = 0;
		if (_vm->_debugger->_useBetaCrosshairsCursor) {
			_drawModeBitFlags |= (MouseDrawFlags::CUSTOM | MouseDrawFlags::ESPER_UP);
		} else {
			_drawModeBitFlags &= ~(MouseDrawFlags::CUSTOM | MouseDrawFlags::ESPER_UP);
		}
		break;

	case 3:
		// static exit cursor (right/East)
		_frame = 15;
		_hotspotX = 23;
		_hotspotY = 12;
		if (_vm->_debugger->_useBetaCrosshairsCursor) {
			_drawModeBitFlags |= (MouseDrawFlags::CUSTOM | MouseDrawFlags::ESPER_RIGHT);
		} else {
			_drawModeBitFlags &= ~(MouseDrawFlags::CUSTOM | MouseDrawFlags::ESPER_RIGHT);
		}
		break;

	case 4:
		// static exit cursor (downwards/South)
		_frame = 13;
		_hotspotX = 12;
		_hotspotY = 23;
		if (_vm->_debugger->_useBetaCrosshairsCursor) {
			_drawModeBitFlags |= (MouseDrawFlags::CUSTOM | MouseDrawFlags::ESPER_DOWN);
		}
		break;

	case 5:
		// static exit cursor (left/West)
		_frame = 14;
		_hotspotX = 0;
		_hotspotY = 12;
		if (_vm->_debugger->_useBetaCrosshairsCursor) {
			_drawModeBitFlags |= (MouseDrawFlags::CUSTOM | MouseDrawFlags::ESPER_LEFT);
		}
		break;

	case 6:
		// combat cursor, simple bullets (normal / no target)
		_frame = 16;
		_hotspotX = 19;
		_hotspotY = 19;
		break;

	case 7:
		// combat cursor, simple bullets (hot target)
		// animating: 8 frames (17-24)
		_frame = 17;
		_hotspotX = 19;
		_hotspotY = 19;
		break;

	case 8:
		// combat cursor, advanced bullets (normal / no target)
		_frame = 25;
		_hotspotX = 19;
		_hotspotY = 19;
		break;

	case 9:
		// combat cursor, advanced bullets (hot target)
		// animating: 8 frames (26-33)
		_frame = 26;
		_hotspotX = 19;
		_hotspotY = 19;
		break;

	case 10:
		// combat cursor, best bullets (normal / no target)
		_frame = 34;
		_hotspotX = 19;
		_hotspotY = 19;
		break;

	case 11:
		// combat cursor, best bullets (hot target)
		// animating: 8 frames (35-42)
		_frame = 35;
		_hotspotX = 19;
		_hotspotY = 19;
		break;

	case 12:
		// exit cursor (upwards/North)
		// resets animCounter too (as opposed to _cursor == 2)
		// bouncy animation (handled in updateCursorFrame())
		_frame = 12;
		_hotspotX = 12;
		_hotspotY = 0;
		_animCounter = 0;
		if (_vm->_debugger->_useBetaCrosshairsCursor) {
			_drawModeBitFlags |= (MouseDrawFlags::CUSTOM | MouseDrawFlags::EXIT_UP);
		}
		break;

	case 13:
		// exit cursor (right/East)
		// resets animCounter too (as opposed to _cursor == 3)
		// bouncy animation (handled in updateCursorFrame())
		_frame = 15;
		_hotspotX = 23;
		_hotspotY = 12;
		_animCounter = 0;
		if (_vm->_debugger->_useBetaCrosshairsCursor) {
			_drawModeBitFlags |= (MouseDrawFlags::CUSTOM | MouseDrawFlags::EXIT_RIGHT);
		}
		break;

	case 14:
		// exit cursor (downwards/South)
		// resets animCounter too (as opposed to _cursor == 4)
		// bouncy animation (handled in updateCursorFrame())
		_frame = 13;
		_hotspotX = 12;
		_hotspotY = 23;
		_animCounter = 0;
		if (_vm->_debugger->_useBetaCrosshairsCursor) {
			_drawModeBitFlags |= (MouseDrawFlags::CUSTOM | MouseDrawFlags::EXIT_DOWN);
		}
		break;

	case 15:
		// exit cursor (left/West)
		// resets animCounter too (as opposed to _cursor == 5)
		// bouncy animation (handled in updateCursorFrame())
		_frame = 14;
		_hotspotX = 0;
		_hotspotY = 12;
		_animCounter = 0;
		if (_vm->_debugger->_useBetaCrosshairsCursor) {
			_drawModeBitFlags |= (MouseDrawFlags::CUSTOM | MouseDrawFlags::EXIT_LEFT);
		}
		break;

	case 16:
		// (beta version) combat cursor (inactive)
		_drawModeBitFlags &= ~(0x01 << _vm->_settings->getAmmoType());
		_drawModeBitFlags &= ~(MouseDrawFlags::SPECIAL);
#if !BLADERUNNER_ORIGINAL_BUGS
		_frame = 0;
		_hotspotX = 11;
		_hotspotY = 11;
		break;

	case 17:
#endif
		// (beta version) combat cursor (white or flashing white/blue)
		// Original behavior in the code remnandts seems to have been
		// that this cursor (id: 16) would animate (3 frames (0-2)),
		// essentially continuously flashing fast.
		// In the preview trailers, the cursor is white while not on a target
		// and red when on target (shown on Zuben) -- perhaps other solid colors were used,
		// (it's hard to tell the color at the shooting grounds shown in the preview trailer).
		// We introduce an extra case (id 17) to differentiate the beta crosshairs
		// depending on whether they're hovering over a hot target or not.
		// TODO Maybe replace the solid colored frames with a color based on McCoy's bullet type
		// So:
		// id 16: inactive (beta) combat crosshairs
		// id 17: active (beta) combat crosshairs
		_drawModeBitFlags |= (0x01 << _vm->_settings->getAmmoType());
		_drawModeBitFlags |=  MouseDrawFlags::SPECIAL;
		_frame = 1;
		_hotspotX = 11;
		_hotspotY = 11;
		break;

	default:
		break;
	}
}

void Mouse::getXY(int *x, int *y) const {
	*x = _x;
	*y = _y;
}

void Mouse::setMouseJitterUp() {
	switch (_vm->_settings->getDifficulty()) {
	default:
		// fall through intended
	case kGameDifficultyEasy:
		_randomCountdownX = 2;
		_randomX = _vm->_rnd.getRandomNumberRng(0, 6) - 3;
		_randomY = _vm->_rnd.getRandomNumberRng(0, 10) - 20;
		break;

	case kGameDifficultyMedium:
		_randomCountdownX = 3;
		_randomX = _vm->_rnd.getRandomNumberRng(0, 8) - 4;
		_randomY = _vm->_rnd.getRandomNumberRng(0, 10) - 25;
		break;

	case kGameDifficultyHard:
		_randomCountdownX = 4;
		_randomX = _vm->_rnd.getRandomNumberRng(0, 10) - 5;
		_randomY = _vm->_rnd.getRandomNumberRng(0, 10) - 30;
		break;
	}
}

void Mouse::setMouseJitterDown() {
	switch (_vm->_settings->getDifficulty()) {
	default:
		// fall through intended
	case kGameDifficultyEasy:
		_randomCountdownY = 2;
		_randomX = _vm->_rnd.getRandomNumberRng(0, 6) - 3;
		_randomY = _vm->_rnd.getRandomNumberRng(10, 20);
		break;

	case kGameDifficultyMedium:
		_randomCountdownY = 3;
		_randomX = _vm->_rnd.getRandomNumberRng(0, 8) - 4;
		_randomY = _vm->_rnd.getRandomNumberRng(15, 25);
		break;

	case kGameDifficultyHard:
		_randomCountdownY = 4;
		_randomX = _vm->_rnd.getRandomNumberRng(0, 10) - 5;
		_randomY = _vm->_rnd.getRandomNumberRng(20, 30);
		break;
	}
}

void Mouse::disable() {
	++_disabledCounter;

	_randomCountdownX = 0;
	_randomCountdownY = 0;
}

void Mouse::enable(bool force) {
	if (force || --_disabledCounter <= 0) {
		_disabledCounter = 0;
	}
}

bool Mouse::isDisabled() const {
	return _disabledCounter > 0;
}

void Mouse::draw(Graphics::Surface &surface, int x, int y) {
	if (_disabledCounter) {
		_randomCountdownX = 0;
		_randomCountdownY = 0;
		return;
	}

	if (_randomCountdownX > 0) {
		--_randomCountdownX;
		x += _randomX;
		y += _randomY;

		if (!_randomCountdownX)
			setMouseJitterDown();
	} else if (_randomCountdownY > 0) {
		--_randomCountdownY;
		x += _randomX;
		y += _randomY;
	}

	_x = CLIP(x, 0, surface.w - 1);
	_y = CLIP(y, 0, surface.h - 1);

	_vm->_shapes->get(_frame)->draw(surface, _x - _hotspotX, _y - _hotspotY, _drawModeBitFlags);

	updateCursorFrame();
}

void Mouse::updateCursorFrame() {
	uint32 now = _vm->_time->current();
	const int offset[4] = { 0, 6, 12, 6 };

	if (now - _lastFrameTime < 66) {
		return;
	}
	_lastFrameTime = now;

	switch (_cursor) {
	case 0:
		break;

	case 1:
		if (++_frame > 11)
			_frame = 4;
		break;

	case 2:
		// fall through
	case 3:
		// fall through
	case 4:
		// fall through
	case 5:
		// fall through
		// 2,3,4,5 are case for "static" exit arrows, used in ESPER
	case 6:
		// 6 is combat cursor, simple bullets (normal / no target)
		break;

	case 7:
		if (++_frame > 24)
			_frame = 17;
		break;

	case 8:
		break;

	case 9:
		if (++_frame > 33)
			_frame = 26;
		break;

	case 10:
		break;

	case 11:
		if (++_frame > 42)
			_frame = 35;
		break;

	case 12:
		if ((_drawModeBitFlags & Mouse::MouseDrawFlags::CUSTOM)
			&& (_drawModeBitFlags & Mouse::MouseDrawFlags::EXIT_UP)) {
			// use the 3 least significant bits in place of "frame" index
			_drawModeBitFlags +=1;
			if ((_drawModeBitFlags & 0x7) == 0x7) {
				_drawModeBitFlags &= ~0x7;
			}
		} else {
			if (++_animCounter >= 4) {
				_animCounter = 0;
			}
			_hotspotY = -offset[_animCounter];
		}
		break;

	case 13:
		if ((_drawModeBitFlags & Mouse::MouseDrawFlags::CUSTOM)
		    && (_drawModeBitFlags & Mouse::MouseDrawFlags::EXIT_RIGHT)) {
			// use the 3 least significant bits in place of "frame" index
			_drawModeBitFlags +=1;
			if ((_drawModeBitFlags & 0x7) == 0x7) {
				_drawModeBitFlags &= ~0x7;
			}
		} else {
			if (++_animCounter >= 4) {
				_animCounter = 0;
			}
			_hotspotX = 23 + offset[_animCounter];
		}
		break;

	case 14:
		if ((_drawModeBitFlags & Mouse::MouseDrawFlags::CUSTOM)
		    && (_drawModeBitFlags & Mouse::MouseDrawFlags::EXIT_DOWN)) {
			// use the 3 least significant bits in place of "frame" index
			_drawModeBitFlags +=1;
			if ((_drawModeBitFlags & 0x7) == 0x7) {
				_drawModeBitFlags &= ~0x7;
			}
		} else {
			if (++_animCounter >= 4) {
				_animCounter = 0;
			}
			_hotspotY = 23 + offset[_animCounter];
		}
		break;

	case 15:
		if ((_drawModeBitFlags & Mouse::MouseDrawFlags::CUSTOM)
		    && (_drawModeBitFlags & Mouse::MouseDrawFlags::EXIT_LEFT)) {
			// use the 3 least significant bits in place of "frame" index
			_drawModeBitFlags +=1;
			if ((_drawModeBitFlags & 0x7) == 0x7) {
				_drawModeBitFlags &= ~0x7;
			}
		} else {
			if (++_animCounter >= 4) {
				_animCounter = 0;
			}
			_hotspotX = -offset[_animCounter];
		}
		break;

	case 16:
#if !BLADERUNNER_ORIGINAL_BUGS
		break;

	case 17:
#endif
		if (++_frame > 2)
#if BLADERUNNER_ORIGINAL_BUGS
			_frame = 0;
#else
			// Better not to flash the white frame (frame 0),
			// while quickly animating the beta cursor.
			// It's less annoying to the eyes this way.
			_frame = 1;
#endif
		break;

	default:
		break;
	}
}

void Mouse::tick(int x, int y) {
	if (!_vm->playerHasControl() || isDisabled()) {
		return;
	}

	if (_vm->_dialogueMenu->isVisible()) {
		setCursor(0);
		return;
	}

	Vector3 scenePosition = getXYZ(x, y);
	int cursorId = 0;

	bool isClickable = false;
	bool isObstacle  = false;
	bool isTarget    = false;

	int sceneObjectId = _vm->_sceneObjects->findByXYZ(&isClickable, &isObstacle, &isTarget, scenePosition, true, false, true);
	int exitType = _vm->_scene->_exits->getTypeAtXY(x, y);

	if (sceneObjectId >= kSceneObjectOffsetActors && sceneObjectId < kSceneObjectOffsetItems) {
		exitType = -1;
	}

	if (exitType != -1) {
		switch (exitType) {
		case 0:
			cursorId = 12;
			break;

		case 1:
			cursorId = 13;
			break;

		case 2:
			cursorId = 14;
			break;

		case 3:
			cursorId = 15;
			break;

		default:
			break;
		}
		setCursor(cursorId);
		return;
	}

	if (!_vm->_combat->isActive()) {
		if (sceneObjectId == kActorMcCoy + kSceneObjectOffsetActors
		|| (sceneObjectId > 0 && isClickable)
		|| _vm->_scene->_regions->getRegionAtXY(x, y) >= 0) {
			cursorId = 1;
		}
		setCursor(cursorId);
		return;
	}

	int animationMode = _vm->_playerActor->getAnimationMode();
	int actorId = Actor::findTargetUnderMouse(_vm, x, y);
	int itemId = _vm->_items->findTargetUnderMouse(x, y);

	bool isObject = isTarget && sceneObjectId >= kSceneObjectOffsetObjects && sceneObjectId <= (95 + kSceneObjectOffsetObjects);

	if (!_vm->_playerActor->isMoving()) {
		if (actorId > 0) {
			_vm->_playerActor->faceActor(actorId, false);
		} else if (itemId >= 0) {
			_vm->_playerActor->faceItem(itemId, false);
		} else if (isObject) {
			_vm->_playerActor->faceXYZ(scenePosition, false);
		}
	}

	if (actorId >= 0 || itemId >= 0 || isObject) {
		if (_vm->_debugger->_useBetaCrosshairsCursor) {
			cursorId = 17;
		} else {
			switch (_vm->_settings->getAmmoType()) {
			case 0:
				cursorId = 7;
				break;

			case 1:
				cursorId = 9;
				break;

			case 2:
				cursorId = 11;
				break;

			default:
				break;
			}
		}
		if (!_vm->_playerActor->isMoving() && animationMode != kAnimationModeCombatAim && animationMode != kAnimationModeCombatHit && animationMode != kAnimationModeCombatDie) {
			_vm->_playerActor->changeAnimationMode(kAnimationModeCombatAim, false);
		}
	} else {
		if (_vm->_debugger->_useBetaCrosshairsCursor) {
			cursorId = 16;
		} else {
			switch (_vm->_settings->getAmmoType()) {
			case 0:
				cursorId = 6;
				break;

			case 1:
				cursorId = 8;
				break;

			case 2:
				cursorId = 10;
				break;

			default:
				break;
			}
		}
		if (!_vm->_playerActor->isMoving() && animationMode != kAnimationModeCombatIdle && animationMode != kAnimationModeCombatHit && animationMode != kAnimationModeCombatDie) {
			_vm->_playerActor->changeAnimationMode(kAnimationModeCombatIdle, false);
		}
	}
	setCursor(cursorId);
}

// This method checks if jitter (due to gun recoil) is currently ongoing
bool Mouse::isRandomized() const {
	return _randomCountdownX > 0 || _randomCountdownY > 0;
}

bool Mouse::isInactive() const {
	// Note: This only refers to "inactive" cursor in combat mode!
	return _cursor == 6 || _cursor == 8 || _cursor == 10 || _cursor == 16;
}

// TEST: RC01 after intro: [290, 216] -> [-204.589249 51.450668 7.659241]
Vector3 Mouse::getXYZ(int x, int y) const {
	if (_vm->_scene->getSetId() == -1)
		return Vector3();

	int screenRight = BladeRunnerEngine::kOriginalGameWidth  - x;
	int screenDown  = BladeRunnerEngine::kOriginalGameHeight - y;

	float zcoef = 1.0f / tan(_vm->_view->_fovX / 2.0f);

	// Division of float by int is float, so no precision is lost here
	float x3d = (2.0f / BladeRunnerEngine::kOriginalGameWidth  * screenRight - 1.0f);
	float y3d = (2.0f / BladeRunnerEngine::kOriginalGameHeight * screenDown  - 1.0f) * 0.75f;

	uint16 zbufval = _vm->_zbuffer->getZValue(x, y);

	Vector3 pos;
	pos.z = zbufval / 25.5f;
	pos.x = pos.z / zcoef * x3d;
	pos.y = pos.z / zcoef * y3d;

	Matrix4x3 matrix = _vm->_view->_frameViewMatrix;

	matrix.unknown();

	return matrix * pos;
}

} // End of namespace BladeRunner
