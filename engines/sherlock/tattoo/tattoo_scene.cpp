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

#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/events.h"
#include "sherlock/people.h"

namespace Sherlock {

namespace Tattoo {

TattooScene::TattooScene(SherlockEngine *vm) : Scene(vm) {
	_arrowZone = -1;
	_mask = _mask1 = nullptr;
	_maskCounter = 0;
}

void TattooScene::checkBgShapes() {
	People &people = *_vm->_people;
	Person &holmes = people._player;
	Common::Point pt(holmes._position.x / FIXED_INT_MULTIPLIER, holmes._position.y / FIXED_INT_MULTIPLIER);

	// Call the base scene method to handle bg shapes
	Scene::checkBgShapes();

	// Check for any active playing animation
	if (_activeCAnim._images && _activeCAnim._zPlacement != REMOVE) {
		switch (_activeCAnim._flags & 3) {
		case 0:
			_activeCAnim._zPlacement = BEHIND;
			break;
		case 1:
			_activeCAnim._zPlacement = ((_activeCAnim._position.y + _activeCAnim._imageFrame->_frame.h - 1)) ?
				NORMAL_FORWARD : NORMAL_BEHIND;
			break;
		case 2:
			_activeCAnim._zPlacement = FORWARD;
			break;
		default:
			break;
		}
	}
}

void TattooScene::doBgAnimCheckCursor() {
	Events &events = *_vm->_events;
	UserInterface &ui = *_vm->_ui;
	Common::Point mousePos = events.mousePos();

	// If we're in Look Mode, make sure the cursor is the magnifying glass
	if (ui._menuMode == LOOK_MODE && events.getCursor() != MAGNIFY)
		events.setCursor(MAGNIFY);

	// See if the mouse is over any of the arrow zones, and if so, change the cursor to the correct
	// arrow cursor indicating the direcetion of the exit
	if (events.getCursor() == ARROW || events.getCursor() >= EXIT_ZONES_START) {
		CursorId cursorId = ARROW;

		if (ui._menuMode == STD_MODE && _arrowZone != -1 && _currentScene != 90) {
			for (uint idx = 0; idx < _exits.size(); ++idx) {
				Exit &exit = _exits[idx];
				if (exit.contains(mousePos))
					cursorId = (CursorId)(exit._image + EXIT_ZONES_START);
			}
		}

		events.setCursor(cursorId);
	}
}

void TattooScene::doBgAnimEraseBackground() {
	TattooEngine &vm = *((TattooEngine *)_vm);
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *((TattooUserInterface *)_vm->_ui);
	
	static const int16 OFFSETS[16] = { -1, -2, -3, -3, -2, -1, -1, 0, 1, 2, 3, 3, 2, 1, 0, 0 };

	if (_mask != nullptr) {
		if (screen._backBuffer1.w() > screen.w())
			screen.blitFrom(screen._backBuffer1, Common::Point(0, 0), Common::Rect(screen._currentScroll, 0,
			screen._currentScroll + screen.w(), screen.h()));
		else
			screen.blitFrom(screen._backBuffer1);

		switch (_currentScene) {
		case 7:
			if (++_maskCounter == 2) {
				_maskCounter = 0;
				if (--_maskOffset.x < 0)
					_maskOffset.x = SHERLOCK_SCREEN_WIDTH - 1;
			}
			break;

		case 8:
			_maskOffset.x += 2;
			if (_maskOffset.x >= SHERLOCK_SCREEN_WIDTH)
				_maskOffset.x = 0;
			break;

		case 18:
		case 68:
			++_maskCounter;
			if (_maskCounter / 4 >= 16)
				_maskCounter = 0;

			_maskOffset.x = OFFSETS[_maskCounter / 4];
			break;

		case 53:
			if (++_maskCounter == 2) {
				_maskCounter = 0;
				if (++_maskOffset.x == screen._backBuffer1.w())
					_maskOffset.x = 0;
			}
			break;

		default:
			break;
		}
	} else {
		// Standard scene without mask, so call user interface to erase any UI elements as necessary
		ui.doBgAnimRestoreUI();
		
		// Restore background for any areas covered by characters and shapes
		for (uint idx = 0; idx < MAX_CHARACTERS; ++idx)
			screen.restoreBackground(Common::Rect(people[idx]._oldPosition.x, people[idx]._oldPosition.y,
				people[idx]._oldPosition.x + people[idx]._oldSize.x, people[idx]._oldPosition.y + people[idx]._oldSize.y));

		for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
			Object &obj = _bgShapes[idx];
						
			if ((obj._type == ACTIVE_BG_SHAPE && (obj._maxFrames > 1 || obj._delta.x != 0 || obj._delta.y != 0)) || 
					obj._type == HIDE_SHAPE || obj._type == REMOVE)
				screen._backBuffer1.blitFrom(*obj._imageFrame, obj._oldPosition, 
					Common::Rect(obj._oldPosition.x, obj._oldPosition.y, obj._oldPosition.x + obj._oldSize.x,
						obj._oldPosition.y + obj._oldSize.y));
		}

		// If credits are active, erase the area they cover
		if (vm._creditsActive)
			vm.eraseCredits();
	}

	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &obj = _bgShapes[idx];

		if (obj._type == NO_SHAPE && (obj._flags & 1) == 0) {
			screen._backBuffer1.blitFrom(screen._backBuffer2, obj._position, obj.getNoShapeBounds());

			obj._oldPosition = obj._position;
			obj._oldSize = obj._noShapeSize;
		}
	}

	// Adjust the Target Scroll if needed
	if ((people[people._walkControl]._position.x / FIXED_INT_MULTIPLIER - screen._currentScroll) < 
			(SHERLOCK_SCREEN_WIDTH / 8) && people[people._walkControl]._delta.x < 0) {
		
		screen._targetScroll = (short)(people[people._walkControl]._position.x / FIXED_INT_MULTIPLIER - 
				SHERLOCK_SCREEN_WIDTH / 8 - 250);
		if (screen._targetScroll < 0)
			screen._targetScroll = 0;
	}

	if ((people[people._walkControl]._position.x / FIXED_INT_MULTIPLIER - screen._currentScroll) > (SHERLOCK_SCREEN_WIDTH / 4 * 3) 
			&& people[people._walkControl]._delta.x > 0)
		screen._targetScroll = (short)(people[people._walkControl]._position.x / FIXED_INT_MULTIPLIER - 
			SHERLOCK_SCREEN_WIDTH / 4 * 3 + 250);

	if (screen._targetScroll > screen._scrollSize)
		screen._targetScroll = screen._scrollSize;

	ui.doScroll();
}

void TattooScene::doBgAnim() {
	doBgAnimCheckCursor();

//	Events &events = *_vm->_events;
	People &people = *_vm->_people;
//	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;

	screen.setDisplayBounds(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCENE_HEIGHT));
	talk._talkToAbort = false;

	// Check the characters and sprites for updates
	for (int idx = 0; idx < MAX_CHARACTERS; ++idx) {
		if (people[idx]._type == CHARACTER)
			people[idx].checkSprite();
	}

	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		if (_bgShapes[idx]._type == ACTIVE_BG_SHAPE)
			_bgShapes[idx].checkObject();
	}

	// Erase any affected background areas
	doBgAnimEraseBackground();
}

void TattooScene::updateBackground() {
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;

	Scene::updateBackground();

	if (_mask != nullptr) {
		switch (_currentScene) {
		case 7:
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x - SHERLOCK_SCREEN_WIDTH, 110), screen._currentScroll);
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x, 110), screen._currentScroll);
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x + SHERLOCK_SCREEN_WIDTH, 110), screen._currentScroll);
			break;

		case 8:
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x - SHERLOCK_SCREEN_WIDTH, 180), screen._currentScroll);
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x, 180), screen._currentScroll);
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x + SHERLOCK_SCREEN_WIDTH, 180), screen._currentScroll);
			if (!_vm->readFlags(880))
				screen._backBuffer1.maskArea((*_mask1)[0], Common::Point(940, 300), screen._currentScroll);
			break;

		case 18:
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(0, 203), screen._currentScroll);
			if (!_vm->readFlags(189))
				screen._backBuffer1.maskArea((*_mask1)[0], Common::Point(124, 239), screen._currentScroll);
			break;

		case 53:
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x, 110), screen._currentScroll);
			break;

		case 68:
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(0, 203), screen._currentScroll);
			screen._backBuffer1.maskArea((*_mask1)[0], Common::Point(124, 239), screen._currentScroll);
			break;

		default:
			break;
		}
	}

	screen._flushScreen = true;

	for (int idx = 0; idx < MAX_CHARACTERS; ++idx) {
		Person &p = people[idx];

		if (p._type != INVALID) {
			if (_goToScene == -1 || _cAnim.size() == 0) {
				if (p._type == REMOVE) {
					screen.slamArea(p._oldPosition.x, p._oldPosition.y, p._oldSize.x, p._oldSize.y);
					p._type = INVALID;
				} else {
					if (p._tempScaleVal == 256) {
						screen.flushImage(p._imageFrame, Common::Point(p._tempX, p._position.y / FIXED_INT_MULTIPLIER
							- p._imageFrame->_width), &p._oldPosition.x, &p._oldPosition.y, &p._oldSize.x, &p._oldSize.y);
					}  else {
						int ts = p._imageFrame->sDrawYSize(p._tempScaleVal);
						int ty = p._position.y / FIXED_INT_MULTIPLIER - ts;
						screen.flushScaleImage(p._imageFrame, Common::Point(p._tempX, ty),
							&p._oldPosition.x, &p._oldPosition.y, &p._oldSize.x, &p._oldSize.y, p._tempScaleVal);
					}
				}
			}
		}
	}

	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &obj = _bgShapes[idx];

		if (obj._type == ACTIVE_BG_SHAPE || obj._type == REMOVE) {
			if (_goToScene == -1) {
				if (obj._scaleVal == 256)
					screen.flushImage(obj._imageFrame, obj._position, &obj._oldPosition.x, &obj._oldPosition.y,
						&obj._oldSize.x, &obj._oldSize.y);
				else
					screen.flushScaleImage(obj._imageFrame, obj._position, &obj._oldPosition.x, &obj._oldPosition.y,
						&obj._oldSize.x, &obj._oldSize.y, obj._scaleVal);

				if (obj._type == REMOVE)
					obj._type = INVALID;
			}
		}
	}

	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &obj = _bgShapes[idx];

		if (_goToScene == -1) {
			if (obj._type == NO_SHAPE && (obj._flags & 1) == 0) {
				screen.slamRect(obj.getNoShapeBounds());
				screen.slamRect(obj.getOldBounds());
			} else if (obj._type == HIDE_SHAPE) {
				if (obj._scaleVal == 256)
					screen.flushImage(obj._imageFrame, obj._position, &obj._oldPosition.x, &obj._oldPosition.y,
						&obj._oldSize.x, &obj._oldSize.y);
				else
					screen.flushScaleImage(obj._imageFrame, obj._position, &obj._oldPosition.x, &obj._oldPosition.y,
						&obj._oldSize.x, &obj._oldSize.y, obj._scaleVal);
				obj._type = HIDDEN;
			}
		}
	}

	screen._flushScreen = false;
}


} // End of namespace Tattoo

} // End of namespace Sherlock
