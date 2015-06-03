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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/scalpel/scalpel_scene.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/events.h"
#include "sherlock/people.h"
#include "sherlock/screen.h"

namespace Sherlock {

namespace Scalpel {

void ScalpelScene::drawAllShapes() {
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;

	// Restrict drawing window
	screen.setDisplayBounds(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCENE_HEIGHT));

	// Draw all active shapes which are behind the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		if (_bgShapes[idx]._type == ACTIVE_BG_SHAPE && _bgShapes[idx]._misc == BEHIND)
			screen._backBuffer->transBlitFrom(*_bgShapes[idx]._imageFrame, _bgShapes[idx]._position, _bgShapes[idx]._flags & OBJ_FLIPPED);
	}

	// Draw all canimations which are behind the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		if (_canimShapes[idx]._type == ACTIVE_BG_SHAPE && _canimShapes[idx]._misc == BEHIND)
			screen._backBuffer->transBlitFrom(*_canimShapes[idx]._imageFrame,
			_canimShapes[idx]._position, _canimShapes[idx]._flags & OBJ_FLIPPED);
	}

	// Draw all active shapes which are normal and behind the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		if (_bgShapes[idx]._type == ACTIVE_BG_SHAPE && _bgShapes[idx]._misc == NORMAL_BEHIND)
			screen._backBuffer->transBlitFrom(*_bgShapes[idx]._imageFrame, _bgShapes[idx]._position, _bgShapes[idx]._flags & OBJ_FLIPPED);
	}

	// Draw all canimations which are normal and behind the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		if (_canimShapes[idx]._type == ACTIVE_BG_SHAPE && _canimShapes[idx]._misc == NORMAL_BEHIND)
			screen._backBuffer->transBlitFrom(*_canimShapes[idx]._imageFrame, _canimShapes[idx]._position,
			_canimShapes[idx]._flags & OBJ_FLIPPED);
	}

	// Draw any active characters
	for (int idx = 0; idx < MAX_CHARACTERS; ++idx) {
		Person &p = people[idx];
		if (p._type == CHARACTER && p._walkLoaded) {
			bool flipped = IS_SERRATED_SCALPEL && (
				p._sequenceNumber == WALK_LEFT || p._sequenceNumber == STOP_LEFT ||
				p._sequenceNumber == WALK_UPLEFT || p._sequenceNumber == STOP_UPLEFT ||
				p._sequenceNumber == WALK_DOWNRIGHT || p._sequenceNumber == STOP_DOWNRIGHT);

			screen._backBuffer->transBlitFrom(*p._imageFrame, Common::Point(p._position.x / FIXED_INT_MULTIPLIER,
				p._position.y / FIXED_INT_MULTIPLIER - p.frameHeight()), flipped);
		}
	}

	// Draw all static and active shapes that are NORMAL and are in front of the player
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		if ((_bgShapes[idx]._type == ACTIVE_BG_SHAPE || _bgShapes[idx]._type == STATIC_BG_SHAPE) &&
			_bgShapes[idx]._misc == NORMAL_FORWARD)
			screen._backBuffer->transBlitFrom(*_bgShapes[idx]._imageFrame, _bgShapes[idx]._position,
			_bgShapes[idx]._flags & OBJ_FLIPPED);
	}

	// Draw all static and active canimations that are NORMAL and are in front of the player
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		if ((_canimShapes[idx]._type == ACTIVE_BG_SHAPE || _canimShapes[idx]._type == STATIC_BG_SHAPE) &&
			_canimShapes[idx]._misc == NORMAL_FORWARD)
			screen._backBuffer->transBlitFrom(*_canimShapes[idx]._imageFrame, _canimShapes[idx]._position,
			_canimShapes[idx]._flags & OBJ_FLIPPED);
	}

	// Draw all static and active shapes that are FORWARD
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		_bgShapes[idx]._oldPosition = _bgShapes[idx]._position;
		_bgShapes[idx]._oldSize = Common::Point(_bgShapes[idx].frameWidth(),
			_bgShapes[idx].frameHeight());

		if ((_bgShapes[idx]._type == ACTIVE_BG_SHAPE || _bgShapes[idx]._type == STATIC_BG_SHAPE) &&
			_bgShapes[idx]._misc == FORWARD)
			screen._backBuffer->transBlitFrom(*_bgShapes[idx]._imageFrame, _bgShapes[idx]._position,
			_bgShapes[idx]._flags & OBJ_FLIPPED);
	}

	// Draw all static and active canimations that are forward
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		if ((_canimShapes[idx]._type == ACTIVE_BG_SHAPE || _canimShapes[idx]._type == STATIC_BG_SHAPE) &&
			_canimShapes[idx]._misc == FORWARD)
			screen._backBuffer->transBlitFrom(*_canimShapes[idx]._imageFrame, _canimShapes[idx]._position,
			_canimShapes[idx]._flags & OBJ_FLIPPED);
	}

	screen.resetDisplayBounds();
}


void ScalpelScene::checkBgShapes() {
	People &people = *_vm->_people;
	Person &holmes = people._player;
	Common::Point pt(holmes._position.x / FIXED_INT_MULTIPLIER, holmes._position.y / FIXED_INT_MULTIPLIER);

	// Call the base scene method to handle bg shapes
	Scene::checkBgShapes();

	// Iterate through the canim list
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		Object &obj = _canimShapes[idx];
		if (obj._type == STATIC_BG_SHAPE || obj._type == ACTIVE_BG_SHAPE) {
			if ((obj._flags & 5) == 1) {
				obj._misc = (pt.y < (obj._position.y + obj._imageFrame->_frame.h - 1)) ?
				NORMAL_FORWARD : NORMAL_BEHIND;
			} else if (!(obj._flags & 1)) {
				obj._misc = BEHIND;
			} else if (obj._flags & 4) {
				obj._misc = FORWARD;
			}
		}
	}
}

void ScalpelScene::doBgAnimCheckCursor() {
	Inventory &inv = *_vm->_inventory;
	Events &events = *_vm->_events;
	Sound &sound = *_vm->_sound;
	UserInterface &ui = *_vm->_ui;
	Common::Point mousePos = events.mousePos();
	events.animateCursorIfNeeded();

	if (ui._menuMode == LOOK_MODE) {
		if (mousePos.y > CONTROLS_Y1)
			events.setCursor(ARROW);
		else if (mousePos.y < CONTROLS_Y)
			events.setCursor(MAGNIFY);
	}

	// Check for setting magnifying glass cursor
	if (ui._menuMode == INV_MODE || ui._menuMode == USE_MODE || ui._menuMode == GIVE_MODE) {
		if (inv._invMode == INVMODE_LOOK) {
			// Only show Magnifying glass cursor if it's not on the inventory command line
			if (mousePos.y < CONTROLS_Y || mousePos.y >(CONTROLS_Y1 + 13))
				events.setCursor(MAGNIFY);
			else
				events.setCursor(ARROW);
		} else {
			events.setCursor(ARROW);
		}
	}

	if (sound._diskSoundPlaying && !*sound._soundIsOn) {
		// Loaded sound just finished playing
		sound.freeDigiSound();
	}
}

void ScalpelScene::doBgAnim() {
	ScalpelEngine &vm = *((ScalpelEngine *)_vm);
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;

	doBgAnimCheckCursor();

	screen.setDisplayBounds(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCENE_HEIGHT));
	talk._talkToAbort = false;

	if (_restoreFlag) {
		for (int idx = 0; idx < MAX_CHARACTERS; ++idx) {
			if (people[idx]._type == CHARACTER)
				people[idx].checkSprite();
		}

		for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
			if (_bgShapes[idx]._type == ACTIVE_BG_SHAPE)
				_bgShapes[idx].checkObject();
		}

		if (people._portraitLoaded && people._portrait._type == ACTIVE_BG_SHAPE)
			people._portrait.checkObject();

		for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
			if (_canimShapes[idx]._type != INVALID && _canimShapes[idx]._type != REMOVE)
				_canimShapes[idx].checkObject();
		}

		if (_currentScene == 12)
			vm.eraseMirror12();

		// Restore the back buffer from the back buffer 2 in the changed area
		Common::Rect bounds(people[AL]._oldPosition.x, people[AL]._oldPosition.y,
			people[AL]._oldPosition.x + people[AL]._oldSize.x,
			people[AL]._oldPosition.y + people[AL]._oldSize.y);
		Common::Point pt(bounds.left, bounds.top);

		if (people[AL]._type == CHARACTER)
			screen.restoreBackground(bounds);
		else if (people[AL]._type == REMOVE)
			screen._backBuffer->blitFrom(screen._backBuffer2, pt, bounds);

		for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
			Object &o = _bgShapes[idx];
			if (o._type == ACTIVE_BG_SHAPE || o._type == HIDE_SHAPE || o._type == REMOVE)
				screen.restoreBackground(o.getOldBounds());
		}

		if (people._portraitLoaded)
			screen.restoreBackground(Common::Rect(
				people._portrait._oldPosition.x, people._portrait._oldPosition.y,
				people._portrait._oldPosition.x + people._portrait._oldSize.x,
				people._portrait._oldPosition.y + people._portrait._oldSize.y
			));

		for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
			Object &o = _bgShapes[idx];
			if (o._type == NO_SHAPE && ((o._flags & OBJ_BEHIND) == 0)) {
				// Restore screen area
				screen._backBuffer->blitFrom(screen._backBuffer2, o._position,
					Common::Rect(o._position.x, o._position.y,
					o._position.x + o._noShapeSize.x, o._position.y + o._noShapeSize.y));

				o._oldPosition = o._position;
				o._oldSize = o._noShapeSize;
			}
		}

		for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
			Object &o = _canimShapes[idx];
			if (o._type == ACTIVE_BG_SHAPE || o._type == HIDE_SHAPE || o._type == REMOVE)
				screen.restoreBackground(Common::Rect(o._oldPosition.x, o._oldPosition.y,
					o._oldPosition.x + o._oldSize.x, o._oldPosition.y + o._oldSize.y));
		}
	}

	//
	// Update the background objects and canimations
	//

	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if (o._type == ACTIVE_BG_SHAPE || o._type == NO_SHAPE)
			o.adjustObject();
	}

	if (people._portraitLoaded && people._portrait._type == ACTIVE_BG_SHAPE)
		people._portrait.adjustObject();

	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		if (_canimShapes[idx]._type != INVALID)
			_canimShapes[idx].adjustObject();
	}

	if (people[AL]._type == CHARACTER && people._holmesOn)
		people[AL].adjustSprite();

	// Flag the bg shapes which need to be redrawn
	checkBgShapes();

	if (_currentScene == 12)
		vm.doMirror12();

	// Draw all active shapes which are behind the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if (o._type == ACTIVE_BG_SHAPE && o._misc == BEHIND)
			screen._backBuffer->transBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
	}

	// Draw all canimations which are behind the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		Object &o = _canimShapes[idx];
		if (o._type == ACTIVE_BG_SHAPE && o._misc == BEHIND) {
			screen._backBuffer->transBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
		}
	}

	// Draw all active shapes which are HAPPEN and behind the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if (o._type == ACTIVE_BG_SHAPE && o._misc == NORMAL_BEHIND)
			screen._backBuffer->transBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
	}

	// Draw all canimations which are NORMAL and behind the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		Object &o = _canimShapes[idx];
		if (o._type == ACTIVE_BG_SHAPE && o._misc == NORMAL_BEHIND) {
			screen._backBuffer->transBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
		}
	}

	// Draw the person if not animating
	if (people[AL]._type == CHARACTER && people[AL]._walkLoaded) {
		// If Holmes is too far to the right, move him back so he's on-screen
		int xRight = SHERLOCK_SCREEN_WIDTH - 2 - people[AL]._imageFrame->_frame.w;
		int tempX = MIN(people[AL]._position.x / FIXED_INT_MULTIPLIER, xRight);

		bool flipped = people[AL]._sequenceNumber == WALK_LEFT || people[AL]._sequenceNumber == STOP_LEFT ||
			people[AL]._sequenceNumber == WALK_UPLEFT || people[AL]._sequenceNumber == STOP_UPLEFT ||
			people[AL]._sequenceNumber == WALK_DOWNRIGHT || people[AL]._sequenceNumber == STOP_DOWNRIGHT;
		screen._backBuffer->transBlitFrom(*people[AL]._imageFrame,
			Common::Point(tempX, people[AL]._position.y / FIXED_INT_MULTIPLIER - people[AL]._imageFrame->_frame.h), flipped);
	}

	// Draw all static and active shapes are NORMAL and are in front of the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if ((o._type == ACTIVE_BG_SHAPE || o._type == STATIC_BG_SHAPE) && o._misc == NORMAL_FORWARD)
			screen._backBuffer->transBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
	}

	// Draw all static and active canimations that are NORMAL and are in front of the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		Object &o = _canimShapes[idx];
		if ((o._type == ACTIVE_BG_SHAPE || o._type == STATIC_BG_SHAPE) && o._misc == NORMAL_FORWARD) {
			screen._backBuffer->transBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
		}
	}

	// Draw all static and active shapes that are in front of the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if ((o._type == ACTIVE_BG_SHAPE || o._type == STATIC_BG_SHAPE) && o._misc == FORWARD)
			screen._backBuffer->transBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
	}

	// Draw any active portrait
	if (people._portraitLoaded && people._portrait._type == ACTIVE_BG_SHAPE)
		screen._backBuffer->transBlitFrom(*people._portrait._imageFrame,
			people._portrait._position, people._portrait._flags & OBJ_FLIPPED);

	// Draw all static and active canimations that are in front of the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		Object &o = _canimShapes[idx];
		if ((o._type == ACTIVE_BG_SHAPE || o._type == STATIC_BG_SHAPE) && o._misc == FORWARD) {
			screen._backBuffer->transBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
		}
	}

	// Draw all NO_SHAPE shapes which have flag bit 0 clear
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if (o._type == NO_SHAPE && (o._flags & OBJ_BEHIND) == 0)
			screen._backBuffer->transBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
	}

	// Bring the newly built picture to the screen
	if (_animating == 2) {
		_animating = 0;
		screen.slamRect(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCENE_HEIGHT));
	} else {
		if (people[AL]._type != INVALID && ((_goToScene == -1 || _canimShapes.empty()))) {
			if (people[AL]._type == REMOVE) {
				screen.slamRect(Common::Rect(
					people[AL]._oldPosition.x, people[AL]._oldPosition.y,
					people[AL]._oldPosition.x + people[AL]._oldSize.x,
					people[AL]._oldPosition.y + people[AL]._oldSize.y
				));
				people[AL]._type = INVALID;
			} else {
				screen.flushImage(people[AL]._imageFrame,
					Common::Point(people[AL]._position.x / FIXED_INT_MULTIPLIER,
						people[AL]._position.y / FIXED_INT_MULTIPLIER - people[AL].frameHeight()),
					&people[AL]._oldPosition.x, &people[AL]._oldPosition.y,
					&people[AL]._oldSize.x, &people[AL]._oldSize.y);
			}
		}

		if (_currentScene == 12)
			vm.flushMirror12();

		for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
			Object &o = _bgShapes[idx];
			if ((o._type == ACTIVE_BG_SHAPE || o._type == REMOVE) && _goToScene == -1) {
				screen.flushImage(o._imageFrame, o._position,
					&o._oldPosition.x, &o._oldPosition.y, &o._oldSize.x, &o._oldSize.y);
			}
		}

		if (people._portraitLoaded) {
			if (people._portrait._type == REMOVE)
				screen.slamRect(Common::Rect(
					people._portrait._position.x, people._portrait._position.y,
					people._portrait._position.x + people._portrait._delta.x,
					people._portrait._position.y + people._portrait._delta.y
				));
			else
				screen.flushImage(people._portrait._imageFrame, people._portrait._position,
					&people._portrait._oldPosition.x, &people._portrait._oldPosition.y,
					&people._portrait._oldSize.x, &people._portrait._oldSize.y);

			if (people._portrait._type == REMOVE)
				people._portrait._type = INVALID;
		}

		if (_goToScene == -1) {
			for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
				Object &o = _bgShapes[idx];
				if (o._type == NO_SHAPE && (o._flags & OBJ_BEHIND) == 0) {
					screen.slamArea(o._position.x, o._position.y, o._oldSize.x, o._oldSize.y);
					screen.slamArea(o._oldPosition.x, o._oldPosition.y, o._oldSize.x, o._oldSize.y);
				} else if (o._type == HIDE_SHAPE) {
					// Hiding shape, so flush it out and mark it as hidden
					screen.flushImage(o._imageFrame, o._position,
						&o._oldPosition.x, &o._oldPosition.y, &o._oldSize.x, &o._oldSize.y);
					o._type = HIDDEN;
				}
			}
		}

		for (int idx = _canimShapes.size() - 1; idx >= 0; --idx) {
			Object &o = _canimShapes[idx];

			if (o._type == INVALID) {
				// Anim shape was invalidated by checkEndOfSequence, so at this point we can remove it
				_canimShapes.remove_at(idx);
			} else  if (o._type == REMOVE) {
				if (_goToScene == -1)
					screen.slamArea(o._position.x, o._position.y, o._delta.x, o._delta.y);

				// Shape for an animation is no longer needed, so remove it completely
				_canimShapes.remove_at(idx);
			} else if (o._type == ACTIVE_BG_SHAPE) {
				screen.flushImage(o._imageFrame, o._position,
					&o._oldPosition.x, &o._oldPosition.y, &o._oldSize.x, &o._oldSize.y);
			}
		}
	}

	_restoreFlag = true;
	_doBgAnimDone = true;

	events.wait(3);
	screen.resetDisplayBounds();

	// Check if the method was called for calling a portrait, and a talk was
	// interrupting it. This talk file would not have been executed at the time,
	// since we needed to finish the 'doBgAnim' to finish clearing the portrait
	if (people._clearingThePortrait && talk._scriptMoreFlag == 3) {
		// Reset the flags and call to talk
		people._clearingThePortrait = false;
		talk._scriptMoreFlag = 0;
		talk.talkTo(talk._scriptName);
	}
}

} // End of namespace Scalpel

} // End of namespace Sherlock
