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

struct ShapeEntry {
	Object *_shape;
	Person *_person;
	bool _isAnimation;
	int _yp;

	ShapeEntry(Person *person, int yp) : _shape(nullptr), _person(person), _yp(yp), _isAnimation(false) {}
	ShapeEntry(Object *shape, int yp) : _shape(shape), _person(nullptr), _yp(yp), _isAnimation(false) {}
	ShapeEntry(int yp) : _shape(nullptr), _person(nullptr), _yp(yp), _isAnimation(true) {}
	int personNum;
};
typedef Common::List<ShapeEntry> ShapeList;

static bool sortImagesY(const ShapeEntry &s1, const ShapeEntry &s2) {
	return s1._yp <= s2._yp;
}

/*----------------------------------------------------------------*/

TattooScene::TattooScene(SherlockEngine *vm) : Scene(vm) {
	_arrowZone = -1;
	_mask = _mask1 = nullptr;
	_maskCounter = 0;
	_labTableScene = false;
}

bool TattooScene::loadScene(const Common::String &filename) {
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	bool result = Scene::loadScene(filename);

	if (_currentScene != STARTING_INTRO_SCENE) {
		// Set the menu/ui mode and whether we're in a lab table close-up scene
		_labTableScene = _currentScene > 91 && _currentScene < 100;
		ui._menuMode = _labTableScene ? LAB_MODE : STD_MODE;
	}

	return result;
}

void TattooScene::drawAllShapes() {
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	ShapeList shapeList;

	// Draw all objects and animations that are set to behind
	screen.setDisplayBounds(Common::Rect(ui._currentScroll.x, 0, ui._currentScroll.x + SHERLOCK_SCREEN_WIDTH,  SHERLOCK_SCREEN_HEIGHT));

	// Draw all active shapes which are behind the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &obj = _bgShapes[idx];

		if (obj._type == ACTIVE_BG_SHAPE && obj._misc == BEHIND) {
			if (obj._quickDraw && obj._scaleVal == 256)
				screen._backBuffer1.blitFrom(*obj._imageFrame, obj._position);
			else
				screen._backBuffer1.transBlitFrom(*obj._imageFrame, obj._position, obj._flags & OBJ_FLIPPED, 0, obj._scaleVal);
		}
	}

	// Draw the animation if it is behind the person
	if (_activeCAnim._imageFrame != nullptr && _activeCAnim._zPlacement == BEHIND)
		screen._backBuffer1.transBlitFrom(*_activeCAnim._imageFrame, _activeCAnim._position,
			(_activeCAnim._flags & 4) >> 1, 0, _activeCAnim._scaleVal);

	screen.setDisplayBounds(Common::Rect(0, 0, screen._backBuffer1.w(), screen._backBuffer1.h()));

	// Queue drawing of all objects that are set to NORMAL.
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &obj = _bgShapes[idx];

		if (obj._type == ACTIVE_BG_SHAPE && (obj._misc == NORMAL_BEHIND || obj._misc == NORMAL_FORWARD)) {
			if (obj._scaleVal == 256)
				shapeList.push_back(ShapeEntry(&obj, obj._position.y + obj._imageFrame->_offset.y +
					obj._imageFrame->_height));
			else
				shapeList.push_back(ShapeEntry(&obj, obj._position.y + obj._imageFrame->sDrawYOffset(obj._scaleVal) +
					obj._imageFrame->sDrawYSize(obj._scaleVal)));
		}
	}

	// Queue drawing the animation if it is NORMAL and can fall in front of, or behind the people
	if (_activeCAnim._imageFrame != nullptr && (_activeCAnim._zPlacement == NORMAL_BEHIND || _activeCAnim._zPlacement == NORMAL_FORWARD)) {
		if (_activeCAnim._scaleVal == 256)
			shapeList.push_back(ShapeEntry(_activeCAnim._position.y + _activeCAnim._imageFrame->_offset.y +
				_activeCAnim._imageFrame->_height));
		else
			shapeList.push_back(ShapeEntry(_activeCAnim._position.y + _activeCAnim._imageFrame->sDrawYOffset(_activeCAnim._scaleVal) +
				_activeCAnim._imageFrame->sDrawYSize(_activeCAnim._scaleVal)));
	}

	// Queue all active characters for drawing
	for (uint idx = 0; idx < MAX_CHARACTERS; ++idx) {
		if (people[idx]._type == CHARACTER && people[idx]._walkLoaded)
			shapeList.push_back(ShapeEntry(&people[idx], people[idx]._position.y / FIXED_INT_MULTIPLIER));
	}

	// Sort the list
	Common::sort(shapeList.begin(), shapeList.end(), sortImagesY);

	// Draw the list of shapes in order
	for (ShapeList::iterator i = shapeList.begin(); i != shapeList.end(); ++i) {
		ShapeEntry &se = *i;

		if (se._shape) {
			// it's a bg shape
			if (se._shape->_quickDraw && se._shape->_scaleVal == 256)
				screen._backBuffer1.blitFrom(*se._shape->_imageFrame, se._shape->_position);
			else
				screen._backBuffer1.transBlitFrom(*se._shape->_imageFrame, se._shape->_position,
					se._shape->_flags & OBJ_FLIPPED, 0, se._shape->_scaleVal);
		} else if (se._isAnimation) {
			// It's an active animation
			screen._backBuffer1.transBlitFrom(*_activeCAnim._imageFrame, _activeCAnim._position,
				(_activeCAnim._flags & 4) >> 1, 0, _activeCAnim._scaleVal);
		} else {
			// Drawing person
			Person &p = *se._person;

			p._tempX = p._position.x / FIXED_INT_MULTIPLIER;
			p._tempScaleVal = getScaleVal(p._position);
			Common::Point adjust = p._adjust;

			if (p._tempScaleVal == 256) {
				p._tempX += adjust.x;
				screen._backBuffer1.transBlitFrom(*p._imageFrame, Common::Point(p._tempX, p._position.y / FIXED_INT_MULTIPLIER
					- p.frameHeight() - adjust.y), p._walkSequences[p._sequenceNumber]._horizFlip, 0, p._tempScaleVal);
			} else {
				if (adjust.x) {
					if (!p._tempScaleVal)
						++p._tempScaleVal;

					if (p._tempScaleVal >= 256 && adjust.x)
						--adjust.x;

					adjust.x = adjust.x * 256 / p._tempScaleVal;

					if (p._tempScaleVal >= 256)
						++adjust.x;
					p._tempX += adjust.x;
				}

				if (adjust.y)
				{
					if (!p._tempScaleVal)
						p._tempScaleVal++;

					if (p._tempScaleVal >= 256 && adjust.y)
						--adjust.y;

					adjust.y = adjust.y * 256 / p._tempScaleVal;

					if (p._tempScaleVal >= 256)
						++adjust.y;
				}

				screen._backBuffer1.transBlitFrom(*p._imageFrame, Common::Point(p._tempX, p._position.y / FIXED_INT_MULTIPLIER
					- p._imageFrame->sDrawYSize(p._tempScaleVal) - adjust.y), p._walkSequences[p._sequenceNumber]._horizFlip, 0, p._tempScaleVal);
			}
		}
	}

	// Draw all objects & canimations that are set to FORWARD.
	// Draw all static and active shapes that are FORWARD
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &obj = _bgShapes[idx];

		if (obj._type == ACTIVE_BG_SHAPE && obj._misc == FORWARD) {
			if (obj._quickDraw && obj._scaleVal == 256)
				screen._backBuffer1.blitFrom(*obj._imageFrame, obj._position);
			else
				screen._backBuffer1.transBlitFrom(*obj._imageFrame, obj._position, obj._flags & OBJ_FLIPPED, 0, obj._scaleVal);
		}
	}

	// Draw the canimation if it is set as FORWARD
	if (_activeCAnim._imageFrame != nullptr && _activeCAnim._zPlacement == FORWARD)
		screen._backBuffer1.transBlitFrom(*_activeCAnim._imageFrame, _activeCAnim._position, (_activeCAnim._flags & 4) >> 1, 0, _activeCAnim._scaleVal);

	// Draw all NO_SHAPE shapes which have their flag bits clear
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &obj = _bgShapes[idx];
		if (obj._type == NO_SHAPE && (obj._flags & 1) == 0)
			screen._backBuffer1.fillRect(obj.getNoShapeBounds(), 15);
	}
}

void TattooScene::checkBgShapes() {
	People &people = *_vm->_people;
	Person &holmes = people._player;
	Common::Point pt(holmes._position.x / FIXED_INT_MULTIPLIER, holmes._position.y / FIXED_INT_MULTIPLIER);

	// Call the base scene method to handle bg shapes
	Scene::checkBgShapes();

	// Check for any active playing animation
	if (_activeCAnim._imageFrame && _activeCAnim._zPlacement != REMOVE) {
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
			screen.blitFrom(screen._backBuffer1, Common::Point(0, 0), Common::Rect(ui._currentScroll.x, 0,
			ui._currentScroll.x + screen.w(), screen.h()));
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
				screen._backBuffer1.blitFrom(screen._backBuffer2, obj._oldPosition, 
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
	if ((people[people._walkControl]._position.x / FIXED_INT_MULTIPLIER - ui._currentScroll.x) < 
			(SHERLOCK_SCREEN_WIDTH / 8) && people[people._walkControl]._delta.x < 0) {
		
		ui._targetScroll.x = (short)(people[people._walkControl]._position.x / FIXED_INT_MULTIPLIER - 
				SHERLOCK_SCREEN_WIDTH / 8 - 250);
		if (ui._targetScroll.x < 0)
			ui._targetScroll.x = 0;
	}

	if ((people[people._walkControl]._position.x / FIXED_INT_MULTIPLIER - ui._currentScroll.x) > (SHERLOCK_SCREEN_WIDTH / 4 * 3) 
			&& people[people._walkControl]._delta.x > 0)
		ui._targetScroll.x = (short)(people[people._walkControl]._position.x / FIXED_INT_MULTIPLIER - 
			SHERLOCK_SCREEN_WIDTH / 4 * 3 + 250);

	if (ui._targetScroll.x > ui._scrollSize)
		ui._targetScroll.x = ui._scrollSize;

	ui.doScroll();
}

void TattooScene::doBgAnim() {
	TattooEngine &vm = *(TattooEngine *)_vm;
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	TattooUserInterface &ui = *((TattooUserInterface *)_vm->_ui);

	doBgAnimCheckCursor();

	talk._talkToAbort = false;

	// Check the characters and sprites for updates
	for (uint idx = 0; idx < MAX_CHARACTERS; ++idx) {
		if (people[idx]._type == CHARACTER)
			people[idx].checkSprite();
	}

	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		if (_bgShapes[idx]._type == ACTIVE_BG_SHAPE)
			_bgShapes[idx].checkObject();
	}

	// If one of the objects has signalled a call to a talk file, to go to another scene, exit immediately
	if (_goToScene != -1)
		return;

	// Erase any affected background areas
	doBgAnimEraseBackground();

	doBgAnimUpdateBgObjectsAndAnim();

	ui.drawInterface();

	doBgAnimDrawSprites();

	if (vm._creditsActive)
		vm.blitCredits();

	if (!vm._fastMode)
		events.wait(3);

	screen._flushScreen = false;
	_doBgAnimDone = false;
	ui._drawMenu = false;

	for (uint idx = 1; idx < MAX_CHARACTERS; ++idx) {
		if (people[idx]._updateNPCPath)
			people[idx].updateNPC();
	}
}

void TattooScene::doBgAnimUpdateBgObjectsAndAnim() {
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &obj = _bgShapes[idx];
		if (obj._type == ACTIVE_BG_SHAPE || obj._type == NO_SHAPE)
			obj.adjustObject();
	}

	for (uint idx = 0; idx < MAX_CHARACTERS; ++idx) {
		if (people[idx]._type == CHARACTER)
			people[idx].adjustSprite();
	}

	if (_activeCAnim._imageFrame != nullptr && _activeCAnim._zPlacement != REMOVE) {
		_activeCAnim.getNextFrame();
	}

	// Flag the bg shapes which need to be redrawn
	checkBgShapes();
	drawAllShapes();


	if (_mask != nullptr) {
		switch (_currentScene) {
		case 7:
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x - SHERLOCK_SCREEN_WIDTH, 110), ui._currentScroll.x);
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x, 110), ui._currentScroll.x);
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x + SHERLOCK_SCREEN_WIDTH, 110), ui._currentScroll.x);
			break;

		case 8:
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x - SHERLOCK_SCREEN_WIDTH, 180), ui._currentScroll.x);
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x, 180), ui._currentScroll.x);
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x + SHERLOCK_SCREEN_WIDTH, 180), ui._currentScroll.x);
			if (!_vm->readFlags(880))
				screen._backBuffer1.maskArea((*_mask1)[0], Common::Point(940, 300), ui._currentScroll.x);
			break;

		case 18:
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x, 203), ui._currentScroll.x);
			if (!_vm->readFlags(189))
				screen._backBuffer1.maskArea((*_mask1)[0], Common::Point(124 + _maskOffset.x, 239), ui._currentScroll.x);
			break;

		case 53:
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x, 110), ui._currentScroll.x);
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x - SHERLOCK_SCREEN_WIDTH, 110), ui._currentScroll.x);
			break;

		case 68:
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x, 203), ui._currentScroll.x);
			screen._backBuffer1.maskArea((*_mask1)[0], Common::Point(124 + _maskOffset.x, 239), ui._currentScroll.x);
			break;
		}
	}
}

void TattooScene::updateBackground() {
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	Scene::updateBackground();

	if (_mask != nullptr) {
		switch (_currentScene) {
		case 7:
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x - SHERLOCK_SCREEN_WIDTH, 110), ui._currentScroll.x);
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x, 110), ui._currentScroll.x);
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x + SHERLOCK_SCREEN_WIDTH, 110), ui._currentScroll.x);
			break;

		case 8:
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x - SHERLOCK_SCREEN_WIDTH, 180), ui._currentScroll.x);
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x, 180), ui._currentScroll.x);
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x + SHERLOCK_SCREEN_WIDTH, 180), ui._currentScroll.x);
			if (!_vm->readFlags(880))
				screen._backBuffer1.maskArea((*_mask1)[0], Common::Point(940, 300), ui._currentScroll.x);
			break;

		case 18:
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(0, 203), ui._currentScroll.x);
			if (!_vm->readFlags(189))
				screen._backBuffer1.maskArea((*_mask1)[0], Common::Point(124, 239), ui._currentScroll.x);
			break;

		case 53:
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(_maskOffset.x, 110), ui._currentScroll.x);
			break;

		case 68:
			screen._backBuffer1.maskArea((*_mask)[0], Common::Point(0, 203), ui._currentScroll.x);
			screen._backBuffer1.maskArea((*_mask1)[0], Common::Point(124, 239), ui._currentScroll.x);
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

void TattooScene::doBgAnimDrawSprites() {
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	for (uint idx = 0; idx < MAX_CHARACTERS; ++idx) {
		Person &person = people[idx];

		if (person._type != INVALID) {
			if (_goToScene == -1 || _cAnim.size() == 0) {
				if (person._type == REMOVE) {
					screen.slamRect(person.getOldBounds());
					person._type = INVALID;
				} else {
					if (person._tempScaleVal == 256) {
						screen.flushImage(person._imageFrame, Common::Point(person._tempX, person._position.y / FIXED_INT_MULTIPLIER
							- person.frameHeight()), &person._oldPosition.x, &person._oldPosition.y, &person._oldSize.x, &person._oldSize.y);
					}  else {
						int ts = person._imageFrame->sDrawYSize(person._tempScaleVal);
						int ty  = person._position.y / FIXED_INT_MULTIPLIER - ts;
						screen.flushScaleImage(person._imageFrame, Common::Point(person._tempX, ty),
							&person._oldPosition.x, &person._oldPosition.y, &person._oldSize.x, &person._oldSize.y, person._tempScaleVal);
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

	if (_activeCAnim._imageFrame != nullptr || _activeCAnim._zPlacement == REMOVE) {
		if (_activeCAnim._zPlacement != REMOVE) {
			screen.flushImage(_activeCAnim._imageFrame, _activeCAnim._position, _activeCAnim._oldBounds, _activeCAnim._scaleVal);
		} else {
			screen.slamArea(_activeCAnim._removeBounds.left - ui._currentScroll.x, _activeCAnim._removeBounds.top, 
				_activeCAnim._removeBounds.width(), _activeCAnim._removeBounds.height());
			_activeCAnim._removeBounds.left = _activeCAnim._removeBounds.top = 0;
			_activeCAnim._removeBounds.right = _activeCAnim._removeBounds.bottom = 0;
			_activeCAnim._zPlacement = -1;		// Reset _zPlacement so we don't REMOVE again
		}
	}
}

int TattooScene::getScaleVal(const Common::Point &pt) {
	bool found = false;
	int result = 256;
	Common::Point pos(pt.x / FIXED_INT_MULTIPLIER, pt.y / FIXED_INT_MULTIPLIER);

	for (uint idx = 0; idx < _scaleZones.size() && !found; ++idx) {
		ScaleZone &sz = _scaleZones[idx];
		if (sz.contains(pos)) {
			int n = (sz._bottomNumber - sz._topNumber) * 100 / sz.height() * (pos.y - sz.top) / 100 + sz._topNumber;
			result = 25600L / n;
		}
	}

	// If it wasn't found, we may be off screen to the left or right, so find the scale zone 
	// that would apply to the y val passed in disregarding the x
	if (!found) {
		for (uint idx = 0; idx < _scaleZones.size() && !found; ++idx) {
			ScaleZone &sz = _scaleZones[idx];
			if (pos.y >= sz.top && pos.y < sz.bottom) {
				int n = (sz._bottomNumber - sz._topNumber) * 100 / sz.height() * (pos.y - sz.top) / 100 + sz._topNumber;
				result = 25600L / n;
			}
		}
	}

	return result;
}


} // End of namespace Tattoo

} // End of namespace Sherlock
