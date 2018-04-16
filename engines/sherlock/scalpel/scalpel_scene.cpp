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
#include "sherlock/scalpel/scalpel_map.h"
#include "sherlock/scalpel/scalpel_people.h"
#include "sherlock/scalpel/scalpel_user_interface.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/events.h"
#include "sherlock/people.h"
#include "sherlock/screen.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

namespace Scalpel {

const int FS_TRANS[8] = {
	STOP_UP, STOP_UPRIGHT, STOP_RIGHT, STOP_DOWNRIGHT, STOP_DOWN, STOP_DOWNLEFT, STOP_LEFT, STOP_UPLEFT
};

/*----------------------------------------------------------------*/

ScalpelScene::~ScalpelScene() {
	for (uint idx = 0; idx < _canimShapes.size(); ++idx)
		delete _canimShapes[idx];
}

bool ScalpelScene::loadScene(const Common::String &filename) {
	ScalpelMap &map = *(ScalpelMap *)_vm->_map;
	bool result = Scene::loadScene(filename);

	if (!_vm->isDemo()) {
		// Reset the previous map location and position on overhead map
		map._oldCharPoint = _currentScene;

		map._overPos.x = (map[_currentScene].x - 6) * FIXED_INT_MULTIPLIER;
		map._overPos.y = (map[_currentScene].y + 9) * FIXED_INT_MULTIPLIER;

	}

	return result;
}

void ScalpelScene::drawAllShapes() {
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;

	// Restrict drawing window
	screen.setDisplayBounds(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCENE_HEIGHT));

	// Draw all active shapes which are behind the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		if (_bgShapes[idx]._type == ACTIVE_BG_SHAPE && _bgShapes[idx]._misc == BEHIND)
			screen.getBackBuffer()->SHtransBlitFrom(*_bgShapes[idx]._imageFrame, _bgShapes[idx]._position, _bgShapes[idx]._flags & OBJ_FLIPPED);
	}

	// Draw all canimations which are behind the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		if (_canimShapes[idx]->_type == ACTIVE_BG_SHAPE && _canimShapes[idx]->_misc == BEHIND)
			screen.getBackBuffer()->SHtransBlitFrom(*_canimShapes[idx]->_imageFrame,
			_canimShapes[idx]->_position, _canimShapes[idx]->_flags & OBJ_FLIPPED);
	}

	// Draw all active shapes which are normal and behind the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		if (_bgShapes[idx]._type == ACTIVE_BG_SHAPE && _bgShapes[idx]._misc == NORMAL_BEHIND)
			screen.getBackBuffer()->SHtransBlitFrom(*_bgShapes[idx]._imageFrame, _bgShapes[idx]._position, _bgShapes[idx]._flags & OBJ_FLIPPED);
	}

	// Draw all canimations which are normal and behind the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		if (_canimShapes[idx]->_type == ACTIVE_BG_SHAPE && _canimShapes[idx]->_misc == NORMAL_BEHIND)
			screen.getBackBuffer()->SHtransBlitFrom(*_canimShapes[idx]->_imageFrame, _canimShapes[idx]->_position,
			_canimShapes[idx]->_flags & OBJ_FLIPPED);
	}

	// Draw any active characters
	for (int idx = 0; idx < MAX_CHARACTERS; ++idx) {
		Person &p = people[idx];
		if (p._type == CHARACTER && p._walkLoaded) {
			bool flipped = IS_SERRATED_SCALPEL && (
				p._sequenceNumber == WALK_LEFT || p._sequenceNumber == STOP_LEFT ||
				p._sequenceNumber == WALK_UPLEFT || p._sequenceNumber == STOP_UPLEFT ||
				p._sequenceNumber == WALK_DOWNRIGHT || p._sequenceNumber == STOP_DOWNRIGHT);

			screen.getBackBuffer()->SHtransBlitFrom(*p._imageFrame, Common::Point(p._position.x / FIXED_INT_MULTIPLIER,
				p._position.y / FIXED_INT_MULTIPLIER - p.frameHeight()), flipped);
		}
	}

	// Draw all static and active shapes that are NORMAL and are in front of the player
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		if ((_bgShapes[idx]._type == ACTIVE_BG_SHAPE || _bgShapes[idx]._type == STATIC_BG_SHAPE) &&
			_bgShapes[idx]._misc == NORMAL_FORWARD)
			screen.getBackBuffer()->SHtransBlitFrom(*_bgShapes[idx]._imageFrame, _bgShapes[idx]._position,
			_bgShapes[idx]._flags & OBJ_FLIPPED);
	}

	// Draw all static and active canimations that are NORMAL and are in front of the player
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		if ((_canimShapes[idx]->_type == ACTIVE_BG_SHAPE || _canimShapes[idx]->_type == STATIC_BG_SHAPE) &&
			_canimShapes[idx]->_misc == NORMAL_FORWARD)
			screen.getBackBuffer()->SHtransBlitFrom(*_canimShapes[idx]->_imageFrame, _canimShapes[idx]->_position,
			_canimShapes[idx]->_flags & OBJ_FLIPPED);
	}

	// Draw all static and active shapes that are FORWARD
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		_bgShapes[idx]._oldPosition = _bgShapes[idx]._position;
		_bgShapes[idx]._oldSize = Common::Point(_bgShapes[idx].frameWidth(),
			_bgShapes[idx].frameHeight());

		if ((_bgShapes[idx]._type == ACTIVE_BG_SHAPE || _bgShapes[idx]._type == STATIC_BG_SHAPE) &&
			_bgShapes[idx]._misc == FORWARD)
			screen.getBackBuffer()->SHtransBlitFrom(*_bgShapes[idx]._imageFrame, _bgShapes[idx]._position,
			_bgShapes[idx]._flags & OBJ_FLIPPED);
	}

	// Draw all static and active canimations that are forward
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		if ((_canimShapes[idx]->_type == ACTIVE_BG_SHAPE || _canimShapes[idx]->_type == STATIC_BG_SHAPE) &&
			_canimShapes[idx]->_misc == FORWARD)
			screen.getBackBuffer()->SHtransBlitFrom(*_canimShapes[idx]->_imageFrame, _canimShapes[idx]->_position,
			_canimShapes[idx]->_flags & OBJ_FLIPPED);
	}

	screen.resetDisplayBounds();
}

void ScalpelScene::checkBgShapes() {
	People &people = *_vm->_people;
	Person &holmes = people[HOLMES];
	Common::Point pt(holmes._position.x / FIXED_INT_MULTIPLIER, holmes._position.y / FIXED_INT_MULTIPLIER);

	// Call the base scene method to handle bg shapes
	Scene::checkBgShapes();

	// Iterate through the canim list
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		Object &obj = *_canimShapes[idx];
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
			if (_canimShapes[idx]->_type != INVALID && _canimShapes[idx]->_type != REMOVE)
				_canimShapes[idx]->checkObject();
		}

		if (_currentScene == DRAWING_ROOM)
			vm.eraseBrumwellMirror();

		// Restore the back buffer from the back buffer 2 in the changed area
		Common::Rect bounds(people[HOLMES]._oldPosition.x, people[HOLMES]._oldPosition.y,
			people[HOLMES]._oldPosition.x + people[HOLMES]._oldSize.x,
			people[HOLMES]._oldPosition.y + people[HOLMES]._oldSize.y);
		Common::Point pt(bounds.left, bounds.top);

		if (people[HOLMES]._type == CHARACTER)
			screen.restoreBackground(bounds);
		else if (people[HOLMES]._type == REMOVE)
			screen.getBackBuffer()->SHblitFrom(screen._backBuffer2, pt, bounds);

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
				screen.getBackBuffer()->SHblitFrom(screen._backBuffer2, o._position,
					Common::Rect(o._position.x, o._position.y,
					o._position.x + o._noShapeSize.x, o._position.y + o._noShapeSize.y));

				o._oldPosition = o._position;
				o._oldSize = o._noShapeSize;
			}
		}

		for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
			Object &o = *_canimShapes[idx];
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
		if (_canimShapes[idx]->_type != INVALID)
			_canimShapes[idx]->adjustObject();
	}

	if (people[HOLMES]._type == CHARACTER && people._holmesOn)
		people[HOLMES].adjustSprite();

	// Flag the bg shapes which need to be redrawn
	checkBgShapes();

	if (_currentScene == DRAWING_ROOM)
		vm.doBrumwellMirror();

	// Draw all active shapes which are behind the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if (o._type == ACTIVE_BG_SHAPE && o._misc == BEHIND)
			screen.getBackBuffer()->SHtransBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
	}

	// Draw all canimations which are behind the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		Object &o = *_canimShapes[idx];
		if (o._type == ACTIVE_BG_SHAPE && o._misc == BEHIND) {
			screen.getBackBuffer()->SHtransBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
		}
	}

	// Draw all active shapes which are HAPPEN and behind the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if (o._type == ACTIVE_BG_SHAPE && o._misc == NORMAL_BEHIND)
			screen.getBackBuffer()->SHtransBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
	}

	// Draw all canimations which are NORMAL and behind the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		Object &o = *_canimShapes[idx];
		if (o._type == ACTIVE_BG_SHAPE && o._misc == NORMAL_BEHIND) {
			screen.getBackBuffer()->SHtransBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
		}
	}

	// Draw the player if he's active and his walk has been loaded into memory
	if (people[HOLMES]._type == CHARACTER && people[HOLMES]._walkLoaded && people._holmesOn) {
		// If Holmes is too far to the right, move him back so he's on-screen
		int xRight = SHERLOCK_SCREEN_WIDTH - 2 - people[HOLMES]._imageFrame->_frame.w;
		int tempX = MIN(people[HOLMES]._position.x / FIXED_INT_MULTIPLIER, xRight);

		bool flipped = people[HOLMES]._sequenceNumber == WALK_LEFT || people[HOLMES]._sequenceNumber == STOP_LEFT ||
			people[HOLMES]._sequenceNumber == WALK_UPLEFT || people[HOLMES]._sequenceNumber == STOP_UPLEFT ||
			people[HOLMES]._sequenceNumber == WALK_DOWNRIGHT || people[HOLMES]._sequenceNumber == STOP_DOWNRIGHT;
		screen.getBackBuffer()->SHtransBlitFrom(*people[HOLMES]._imageFrame,
			Common::Point(tempX, people[HOLMES]._position.y / FIXED_INT_MULTIPLIER - people[HOLMES]._imageFrame->_frame.h), flipped);
	}

	// Draw all static and active shapes are NORMAL and are in front of the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if ((o._type == ACTIVE_BG_SHAPE || o._type == STATIC_BG_SHAPE) && o._misc == NORMAL_FORWARD)
			screen.getBackBuffer()->SHtransBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
	}

	// Draw all static and active canimations that are NORMAL and are in front of the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		Object &o = *_canimShapes[idx];
		if ((o._type == ACTIVE_BG_SHAPE || o._type == STATIC_BG_SHAPE) && o._misc == NORMAL_FORWARD) {
			screen.getBackBuffer()->SHtransBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
		}
	}

	// Draw all static and active shapes that are in front of the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if ((o._type == ACTIVE_BG_SHAPE || o._type == STATIC_BG_SHAPE) && o._misc == FORWARD)
			screen.getBackBuffer()->SHtransBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
	}

	// Draw any active portrait
	if (people._portraitLoaded && people._portrait._type == ACTIVE_BG_SHAPE)
		screen.getBackBuffer()->SHtransBlitFrom(*people._portrait._imageFrame,
			people._portrait._position, people._portrait._flags & OBJ_FLIPPED);

	// Draw all static and active canimations that are in front of the person
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		Object &o = *_canimShapes[idx];
		if ((o._type == ACTIVE_BG_SHAPE || o._type == STATIC_BG_SHAPE) && o._misc == FORWARD) {
			screen.getBackBuffer()->SHtransBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
		}
	}

	// Draw all NO_SHAPE shapes which have flag bit 0 clear
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &o = _bgShapes[idx];
		if (o._type == NO_SHAPE && (o._flags & OBJ_BEHIND) == 0)
			screen.getBackBuffer()->SHtransBlitFrom(*o._imageFrame, o._position, o._flags & OBJ_FLIPPED);
	}

	// Bring the newly built picture to the screen
	if (_animating == 2) {
		_animating = 0;
		screen.slamRect(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCENE_HEIGHT));
	} else {
		if (people[HOLMES]._type != INVALID && ((_goToScene == -1 || _canimShapes.empty()))) {
			if (people[HOLMES]._type == REMOVE) {
				screen.slamRect(Common::Rect(
					people[HOLMES]._oldPosition.x, people[HOLMES]._oldPosition.y,
					people[HOLMES]._oldPosition.x + people[HOLMES]._oldSize.x,
					people[HOLMES]._oldPosition.y + people[HOLMES]._oldSize.y
				));
				people[HOLMES]._type = INVALID;
			} else {
				screen.flushImage(people[HOLMES]._imageFrame,
					Common::Point(people[HOLMES]._position.x / FIXED_INT_MULTIPLIER,
						people[HOLMES]._position.y / FIXED_INT_MULTIPLIER - people[HOLMES].frameHeight()),
					&people[HOLMES]._oldPosition.x, &people[HOLMES]._oldPosition.y,
					&people[HOLMES]._oldSize.x, &people[HOLMES]._oldSize.y);
			}
		}

		if (_currentScene == DRAWING_ROOM)
			vm.flushBrumwellMirror();

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
			Object &o = *_canimShapes[idx];

			if (o._type == INVALID) {
				// Anim shape was invalidated by checkEndOfSequence, so at this point we can remove it
				delete _canimShapes[idx];
				_canimShapes.remove_at(idx);
			} else  if (o._type == REMOVE) {
				if (_goToScene == -1)
					screen.slamArea(o._position.x, o._position.y, o._delta.x, o._delta.y);

				// Shape for an animation is no longer needed, so remove it completely
				delete _canimShapes[idx];
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

int ScalpelScene::startCAnim(int cAnimNum, int playRate) {
	Events &events = *_vm->_events;
	ScalpelMap &map = *(ScalpelMap *)_vm->_map;
	People &people = *_vm->_people;
	Resources &res = *_vm->_res;
	Talk &talk = *_vm->_talk;
	ScalpelUserInterface &ui = *(ScalpelUserInterface *)_vm->_ui;
	Point32 tpPos, walkPos;
	int tpDir, walkDir;
	int tFrames = 0;
	int gotoCode = -1;
	Object *cObj;

	// Validation
	if (cAnimNum >= (int)_cAnim.size())
		// number out of bounds
		return -1;
	if (_canimShapes.size() >= 3 || playRate == 0)
		// Too many active animations, or invalid play rate
		return 0;

	CAnim &cAnim = _cAnim[cAnimNum];
	if (playRate < 0) {
		// Reverse direction
		walkPos = cAnim._teleport[0];
		walkDir = cAnim._teleport[0]._facing;
		tpPos = cAnim._goto[0];
		tpDir = cAnim._goto[0]._facing;
	} else {
		// Forward direction
		walkPos = cAnim._goto[0];
		walkDir = cAnim._goto[0]._facing;
		tpPos = cAnim._teleport[0];
		tpDir = cAnim._teleport[0]._facing;
	}

	CursorId oldCursor = events.getCursor();
	events.setCursor(WAIT);

	if (walkPos.x != -1) {
		// Holmes must walk to the walk point before the cAnimation is started
		if (people[HOLMES]._position != walkPos)
			people[HOLMES].walkToCoords(walkPos, walkDir);
	}

	if (talk._talkToAbort)
		return 1;

	// Add new anim shape entry for displaying the animation
	cObj = new Object();
	_canimShapes.push_back(cObj);

	// Copy the canimation into the bgShapes type canimation structure so it can be played
	cObj->_allow = cAnimNum + 1;				// Keep track of the parent structure
	cObj->_name = _cAnim[cAnimNum]._name;	// Copy name

	// Remove any attempt to draw object frame
	if (cAnim._type == NO_SHAPE && cAnim._sequences[0] < 100)
		cAnim._sequences[0] = 0;

	cObj->_sequences = cAnim._sequences;
	cObj->_images = nullptr;
	cObj->_position = cAnim._position;
	cObj->_delta = Common::Point(0, 0);
	cObj->_type = cAnim._type;
	cObj->_flags = cAnim._flags;

	cObj->_maxFrames = 0;
	cObj->_frameNumber = -1;
	cObj->_sequenceNumber = cAnimNum;
	cObj->_oldPosition = Common::Point(0, 0);
	cObj->_oldSize = Common::Point(0, 0);
	cObj->_goto = Common::Point(0, 0);
	cObj->_status = 0;
	cObj->_misc = 0;
	cObj->_imageFrame = nullptr;

	if (cAnim._name.size() > 0 && cAnim._type != NO_SHAPE) {
		if (tpPos.x != -1)
			people[HOLMES]._type = REMOVE;

		Common::String fname = cAnim._name + ".vgs";
		if (!res.isInCache(fname)) {
			// Set up RRM scene data
			Common::SeekableReadStream *roomStream = res.load(_roomFilename);
			roomStream->seek(cAnim._dataOffset);
			//rrmStream->seek(44 + cAnimNum * 4);
			//rrmStream->seek(rrmStream->readUint32LE());

			// Load the canimation into the cache
			Common::SeekableReadStream *imgStream = !_compressed ? roomStream->readStream(cAnim._dataSize) :
				Resources::decompressLZ(*roomStream, cAnim._dataSize);
			res.addToCache(fname, *imgStream);

			delete imgStream;
			delete roomStream;
		}

		// Now load the resource as an image
		if (!IS_3DO) {
			cObj->_images = new ImageFile(fname);
		} else {
			cObj->_images = new ImageFile3DO(fname, kImageFile3DOType_RoomFormat);
		}
		cObj->_imageFrame = &(*cObj->_images)[0];
		cObj->_maxFrames = cObj->_images->size();

		int frames = 0;
		if (playRate < 0) {
			// Reverse direction
			// Count number of frames
			while (frames < MAX_FRAME && cObj->_sequences[frames])
				++frames;
		} else {
			// Forward direction
			BaseObject::_countCAnimFrames = true;

			while (cObj->_type == ACTIVE_BG_SHAPE) {
				cObj->checkObject();
				++frames;

				if (frames >= 1000)
					error("CAnim has infinite loop sequence");
			}

			if (frames > 1)
				--frames;

			BaseObject::_countCAnimFrames = false;

			cObj->_type = cAnim._type;
			cObj->_frameNumber = -1;
			cObj->_position = cAnim._position;
			cObj->_delta = Common::Point(0, 0);
		}

		// Return if animation has no frames in it
		if (frames == 0)
			return -2;

		++frames;
		int repeat = ABS(playRate);
		int dir;

		if (playRate < 0) {
			// Play in reverse
			dir = -2;
			cObj->_frameNumber = frames - 3;
		} else {
			dir = 0;
		}

		tFrames = frames - 1;
		int pauseFrame = (_cAnimFramePause) ? frames - _cAnimFramePause : -1;

		while (--frames) {
			if (frames == pauseFrame)
				ui.printObjectDesc();

			doBgAnim();

			// Repeat same frame
			int temp = repeat;
			while (--temp > 0) {
				cObj->_frameNumber--;
				doBgAnim();

				if (_vm->shouldQuit())
					return 0;
			}

			cObj->_frameNumber += dir;
		}

		people[HOLMES]._type = CHARACTER;
	}

	// Teleport to ending coordinates if necessary
	if (tpPos.x != -1) {
		people[HOLMES]._position = tpPos;	// Place the player
		people[HOLMES]._sequenceNumber = tpDir;
		people[HOLMES].gotoStand();
	}

	if (playRate < 0)
		// Reverse direction - set to end sequence
		cObj->_frameNumber = tFrames - 1;

	if (cObj->_frameNumber <= 26)
		gotoCode = cObj->_sequences[cObj->_frameNumber + 3];

	// Unless anim shape has already been removed, do a final check to allow it to become REMOVEd
	for (uint idx = 0; idx < _canimShapes.size(); ++idx) {
		if (_canimShapes[idx] == cObj) {
			cObj->checkObject();
			break;
		}
	}

	if (gotoCode > 0 && !talk._talkToAbort) {
		_goToScene = gotoCode;

		if (_goToScene < 97 && map[_goToScene].x) {
			map._overPos = map[_goToScene];
		}
	}

	people.loadWalk();

	if (tpPos.x != -1 && !talk._talkToAbort) {
		// Teleport to ending coordinates
		people[HOLMES]._position = tpPos;
		people[HOLMES]._sequenceNumber = tpDir;

		people[HOLMES].gotoStand();
	}

	events.setCursor(oldCursor);

	return 1;
}

int ScalpelScene::closestZone(const Common::Point &pt) {
	int dist = 1000;
	int zone = -1;

	for (uint idx = 0; idx < _zones.size(); ++idx) {
		Common::Point zc((_zones[idx].left + _zones[idx].right) / 2,
			(_zones[idx].top + _zones[idx].bottom) / 2);
		int d = ABS(zc.x - pt.x) + ABS(zc.y - pt.y);

		if (d < dist) {
			// Found a closer zone
			dist = d;
			zone = idx;
		}
	}

	return zone;
}

int ScalpelScene::findBgShape(const Common::Point &pt) {
	if (!_doBgAnimDone)
		// New frame hasn't been drawn yet
		return -1;

	for (int idx = (int)_bgShapes.size() - 1; idx >= 0; --idx) {
		Object &o = _bgShapes[idx];
		if (o._type != INVALID && o._type != NO_SHAPE && o._type != HIDDEN
				&& o._type != REMOVE && o._aType <= PERSON) {
			if (o.getNewBounds().contains(pt))
				return idx;
		} else if (o._type == NO_SHAPE) {
			if (o.getNoShapeBounds().contains(pt))
				return idx;
		}
	}

	return -1;
}

} // End of namespace Scalpel

} // End of namespace Sherlock
