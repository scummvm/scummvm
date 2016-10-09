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
#include "sherlock/tattoo/tattoo_people.h"
#include "sherlock/tattoo/tattoo_talk.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/events.h"
#include "sherlock/people.h"

namespace Sherlock {

namespace Tattoo {

const int FS_TRANS[8] = {
	STOP_UP, STOP_UPRIGHT, STOP_RIGHT, STOP_DOWNRIGHT, STOP_DOWN, STOP_DOWNLEFT, STOP_LEFT, STOP_UPLEFT
};

/*----------------------------------------------------------------*/

struct ShapeEntry {
	Object *_shape;
	TattooPerson *_person;
	bool _isAnimation;
	int _yp;
	int _ordering;

	ShapeEntry(TattooPerson *person, int yp, int ordering) :
		_shape(nullptr), _person(person), _yp(yp), _isAnimation(false), _ordering(ordering) {}
	ShapeEntry(Object *shape, int yp, int ordering) :
		_shape(shape), _person(nullptr), _yp(yp), _isAnimation(false), _ordering(ordering) {}
	ShapeEntry(int yp, int ordering) :
		_shape(nullptr), _person(nullptr), _yp(yp), _isAnimation(true), _ordering(ordering) {}
};
typedef Common::List<ShapeEntry> ShapeList;

static bool sortImagesY(const ShapeEntry &s1, const ShapeEntry &s2) {
	// Objects are order by the calculated Y position first and then, when both are equal,
	// by the order in which the entries were added
	return s1._yp < s2._yp || (s1._yp == s2._yp && s1._ordering < s2._ordering);
}

/*----------------------------------------------------------------*/

TattooScene::TattooScene(SherlockEngine *vm) : Scene(vm), _labWidget(vm) {
	_labTableScene = false;
}

bool TattooScene::loadScene(const Common::String &filename) {
	TattooEngine &vm = *(TattooEngine *)_vm;
	Events &events = *_vm->_events;
	Music &music = *_vm->_music;
	Talk &talk = *_vm->_talk;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	// If we're going to the first game scene after the intro sequence, flag it as finished
	if (vm._runningProlog && _currentScene == STARTING_GAME_SCENE) {
		vm._runningProlog = false;
		events.showCursor();
		talk._talkToAbort = false;
	}

	// Check if it's a scene we need to keep trakc track of how many times we've visited
	for (int idx = (int)_sceneTripCounters.size() - 1; idx >= 0; --idx) {
		if (_sceneTripCounters[idx]._sceneNumber == _currentScene) {
			if (--_sceneTripCounters[idx]._numTimes == 0) {
				_vm->setFlags(_sceneTripCounters[idx]._flag);
				_sceneTripCounters.remove_at(idx);
			}
		}
	}

	// Handle loading music for the scene
	if (talk._scriptMoreFlag != 1 && talk._scriptMoreFlag != 3)
		music._nextSongName = Common::String::format("res%02d", _currentScene);

	// Set the NPC paths for the scene
	setNPCPath(WATSON);

	// If it's a new song, then start it up
	if (music._currentSongName.compareToIgnoreCase(music._nextSongName)) {
		// WORKAROUND: Stop playing music after Diogenes fire scene in the intro,
		// since it overlaps slightly into the next scene
		if (talk._scriptName == "prol80p" && _currentScene == 80) {
			music.stopMusic();
			events.wait(5);
		}

		if (music.loadSong(music._nextSongName)) {
			if (music._musicOn)
				music.startSong();
		}
	}

	bool result = Scene::loadScene(filename);

	if (_currentScene != STARTING_INTRO_SCENE) {
		// Set the menu/ui mode and whether we're in a lab table close-up scene
		_labTableScene = _currentScene > 91 && _currentScene < 100;
		ui._menuMode = _labTableScene ? LAB_MODE : STD_MODE;

		if (_labTableScene)
			ui.addFixedWidget(&_labWidget);
	}

	return result;
}

void TattooScene::drawAllShapes() {
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	Screen &screen = *_vm->_screen;
	ShapeList shapeList;
	int ordering = 0;

	// Draw all objects and animations that are set to behind
	screen.setDisplayBounds(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));

	// Draw all active shapes which are behind the person
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &obj = _bgShapes[idx];

		if (obj._type == ACTIVE_BG_SHAPE && obj._misc == BEHIND) {
			if (obj._quickDraw && obj._scaleVal == SCALE_THRESHOLD)
				screen._backBuffer1.SHblitFrom(*obj._imageFrame, obj._position);
			else
				screen._backBuffer1.SHtransBlitFrom(*obj._imageFrame, obj._position, obj._flags & OBJ_FLIPPED, 0, obj._scaleVal);
		}
	}

	// Draw the animation if it is behind the person
	if (_activeCAnim.active() && _activeCAnim._zPlacement == BEHIND)
		screen._backBuffer1.SHtransBlitFrom(_activeCAnim._imageFrame, _activeCAnim._position,
			(_activeCAnim._flags & 4) >> 1, 0, _activeCAnim._scaleVal);

	screen.resetDisplayBounds();

	// Queue drawing of all objects that are set to NORMAL.
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &obj = _bgShapes[idx];

		if (obj._type == ACTIVE_BG_SHAPE && (obj._misc == NORMAL_BEHIND || obj._misc == NORMAL_FORWARD)) {
			if (obj._scaleVal == SCALE_THRESHOLD)
				shapeList.push_back(ShapeEntry(&obj, obj._position.y + obj._imageFrame->_offset.y +
					obj._imageFrame->_height, ordering++));
			else
				shapeList.push_back(ShapeEntry(&obj, obj._position.y + obj._imageFrame->sDrawYOffset(obj._scaleVal) +
					obj._imageFrame->sDrawYSize(obj._scaleVal), ordering++));
		}
	}

	// Queue drawing the animation if it is NORMAL and can fall in front of, or behind the people
	if (_activeCAnim.active() && (_activeCAnim._zPlacement == NORMAL_BEHIND || _activeCAnim._zPlacement == NORMAL_FORWARD)) {
		if (_activeCAnim._scaleVal == SCALE_THRESHOLD)
			shapeList.push_back(ShapeEntry(_activeCAnim._position.y + _activeCAnim._imageFrame._offset.y +
				_activeCAnim._imageFrame._height, ordering++));
		else
			shapeList.push_back(ShapeEntry(_activeCAnim._position.y + _activeCAnim._imageFrame.sDrawYOffset(_activeCAnim._scaleVal) +
				_activeCAnim._imageFrame.sDrawYSize(_activeCAnim._scaleVal), ordering++));
	}

	// Queue all active characters for drawing
	for (int idx = 0; idx < MAX_CHARACTERS; ++idx) {
		if (people[idx]._type == CHARACTER && people[idx]._walkLoaded)
			shapeList.push_back(ShapeEntry(&people[idx], people[idx]._position.y / FIXED_INT_MULTIPLIER, ordering++));
	}

	// Sort the list
	Common::sort(shapeList.begin(), shapeList.end(), sortImagesY);

	// Draw the list of shapes in order
	for (ShapeList::iterator i = shapeList.begin(); i != shapeList.end(); ++i) {
		ShapeEntry &se = *i;

		if (se._shape) {
			// it's a bg shape
			if (se._shape->_quickDraw && se._shape->_scaleVal == SCALE_THRESHOLD)
				screen._backBuffer1.SHblitFrom(*se._shape->_imageFrame, se._shape->_position);
			else
				screen._backBuffer1.SHtransBlitFrom(*se._shape->_imageFrame, se._shape->_position,
					se._shape->_flags & OBJ_FLIPPED, 0, se._shape->_scaleVal);
		} else if (se._isAnimation) {
			// It's an active animation
			screen._backBuffer1.SHtransBlitFrom(_activeCAnim._imageFrame, _activeCAnim._position,
				(_activeCAnim._flags & 4) >> 1, 0, _activeCAnim._scaleVal);
		} else {
			// Drawing person
			TattooPerson &p = *se._person;

			p._tempX = p._position.x / FIXED_INT_MULTIPLIER;
			p._tempScaleVal = getScaleVal(p._position);
			Common::Point adjust = p._adjust;

			if (p._tempScaleVal == SCALE_THRESHOLD) {
				p._tempX += adjust.x;
				screen._backBuffer1.SHtransBlitFrom(*p._imageFrame, Common::Point(p._tempX, p._position.y / FIXED_INT_MULTIPLIER
					- p.frameHeight() - adjust.y), p._walkSequences[p._sequenceNumber]._horizFlip, 0, p._tempScaleVal);
			} else {
				if (adjust.x) {
					if (!p._tempScaleVal)
						++p._tempScaleVal;

					if (p._tempScaleVal >= SCALE_THRESHOLD && adjust.x)
						--adjust.x;

					adjust.x = adjust.x * SCALE_THRESHOLD / p._tempScaleVal;

					if (p._tempScaleVal >= SCALE_THRESHOLD)
						++adjust.x;
					p._tempX += adjust.x;
				}

				if (adjust.y) {
					if (!p._tempScaleVal)
						p._tempScaleVal++;

					if (p._tempScaleVal >= SCALE_THRESHOLD && adjust.y)
						--adjust.y;

					adjust.y = adjust.y * SCALE_THRESHOLD / p._tempScaleVal;

					if (p._tempScaleVal >= SCALE_THRESHOLD)
						++adjust.y;
				}

				screen._backBuffer1.SHtransBlitFrom(*p._imageFrame, Common::Point(p._tempX, p._position.y / FIXED_INT_MULTIPLIER
					- p._imageFrame->sDrawYSize(p._tempScaleVal) - adjust.y), p._walkSequences[p._sequenceNumber]._horizFlip, 0, p._tempScaleVal);
			}
		}
	}

	// Draw all objects & canimations that are set to FORWARD.
	// Draw all static and active shapes that are FORWARD
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &obj = _bgShapes[idx];

		if (obj._type == ACTIVE_BG_SHAPE && obj._misc == FORWARD) {
			if (obj._quickDraw && obj._scaleVal == SCALE_THRESHOLD)
				screen._backBuffer1.SHblitFrom(*obj._imageFrame, obj._position);
			else
				screen._backBuffer1.SHtransBlitFrom(*obj._imageFrame, obj._position, obj._flags & OBJ_FLIPPED, 0, obj._scaleVal);
		}
	}

	// Draw the canimation if it is set as FORWARD
	if (_activeCAnim.active() && _activeCAnim._zPlacement == FORWARD)
		screen._backBuffer1.SHtransBlitFrom(_activeCAnim._imageFrame, _activeCAnim._position, (_activeCAnim._flags & 4) >> 1, 0, _activeCAnim._scaleVal);

	// Draw all NO_SHAPE shapes which have their flag bits clear
	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &obj = _bgShapes[idx];
		if (obj._type == NO_SHAPE && (obj._flags & 1) == 0)
			screen._backBuffer1.fillRect(obj.getNoShapeBounds(), 15);
	}
}

void TattooScene::paletteLoaded() {
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	ui.setupBGArea(screen._cMap);
	ui.initScrollVars();
}

void TattooScene::checkBgShapes() {
	// Call the base scene method to handle bg shapes
	Scene::checkBgShapes();

	// Check for any active playing animation
	if (_activeCAnim.active() && _activeCAnim._zPlacement != REMOVE) {
		switch (_activeCAnim._flags & 3) {
		case 0:
			_activeCAnim._zPlacement = BEHIND;
			break;
		case 1:
			_activeCAnim._zPlacement = ((_activeCAnim._position.y + _activeCAnim._imageFrame._frame.h - 1)) ?
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

void TattooScene::freeScene() {
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Scene::freeScene();

	// Delete any scene overlays that were used by the scene
	delete ui._mask;
	delete ui._mask1;
	ui._mask = ui._mask1 = nullptr;
}

void TattooScene::doBgAnimCheckCursor() {
	Events &events = *_vm->_events;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();

	// If we're in Look Mode, make sure the cursor is the magnifying glass
	if (ui._menuMode == LOOK_MODE && events.getCursor() != MAGNIFY)
		events.setCursor(MAGNIFY);

	// See if the mouse is over any of the arrow zones, and if so, change the cursor to the correct
	// arrow cursor indicating the direcetion of the exit
	if (events.getCursor() == ARROW || events.getCursor() >= EXIT_ZONES_START) {
		CursorId cursorId = ARROW;

		if (ui._menuMode == STD_MODE && ui._arrowZone != -1 && _currentScene != 90) {
			for (uint idx = 0; idx < _exits.size(); ++idx) {
				Exit &exit = _exits[idx];
				if (exit.contains(mousePos))
					cursorId = (CursorId)(exit._image + EXIT_ZONES_START);
			}
		}

		events.setCursor(cursorId);
	} else {
		events.animateCursorIfNeeded();
	}
}

void TattooScene::doBgAnim() {
	TattooEngine &vm = *(TattooEngine *)_vm;
	Events &events = *_vm->_events;
	Music &music = *_vm->_music;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	TattooUserInterface &ui = *((TattooUserInterface *)_vm->_ui);

	doBgAnimCheckCursor();
	music.checkSongProgress();

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

	// If one of the objects has signalled a call to a talk file, to go to another scene, exit immediately
	if (_goToScene != -1)
		return;

	// Erase any affected background areas
	ui.doBgAnimEraseBackground();

	doBgAnimUpdateBgObjectsAndAnim();

	doBgAnimDrawSprites();

	ui.drawInterface();

	if (ui._creditsWidget.active())
		ui._creditsWidget.blitCredits();

	if (screen._flushScreen) {
		screen.slamArea(screen._currentScroll.x, screen._currentScroll.y, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
		screen._flushScreen = false;
	}

	screen._flushScreen = false;
	_doBgAnimDone = true;
	ui._drawMenu = false;

	// Handle drawing tooltips
	if (ui._menuMode == STD_MODE || ui._menuMode == LAB_MODE)
		ui._tooltipWidget.draw();
	if (!ui._postRenderWidgets.empty()) {
		for (WidgetList::iterator i = ui._postRenderWidgets.begin(); i != ui._postRenderWidgets.end(); ++i)
			(*i)->draw();
		ui._postRenderWidgets.clear();
	}

	if (!vm._fastMode)
		events.wait(3);

	for (int idx = 1; idx < MAX_CHARACTERS; ++idx) {
		if (people[idx]._updateNPCPath)
			people[idx].updateNPC();
	}
}

void TattooScene::doBgAnimUpdateBgObjectsAndAnim() {
	People &people = *_vm->_people;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	for (uint idx = 0; idx < _bgShapes.size(); ++idx) {
		Object &obj = _bgShapes[idx];
		if (obj._type == ACTIVE_BG_SHAPE || obj._type == NO_SHAPE)
			obj.adjustObject();
	}

	for (int idx = 0; idx < MAX_CHARACTERS; ++idx) {
		if (people[idx]._type == CHARACTER)
			people[idx].adjustSprite();
	}

	// Flag the bg shapes which need to be redrawn
	checkBgShapes();
	drawAllShapes();

	ui.drawMaskArea(true);
}

void TattooScene::updateBackground() {
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	Scene::updateBackground();

	ui.drawMaskArea(false);

	screen._flushScreen = true;

	for (int idx = 0; idx < MAX_CHARACTERS; ++idx) {
		TattooPerson &p = people[idx];

		if (p._type != INVALID) {
			if (_goToScene == -1 || _cAnim.size() == 0) {
				if (p._type == REMOVE) {
					screen.slamArea(p._oldPosition.x, p._oldPosition.y, p._oldSize.x, p._oldSize.y);
					p._type = INVALID;
				} else {
					if (p._tempScaleVal == SCALE_THRESHOLD) {
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
				if (obj._scaleVal == SCALE_THRESHOLD)
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
				if (obj._scaleVal == SCALE_THRESHOLD)
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
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	Screen &screen = *_vm->_screen;

	for (int idx = 0; idx < MAX_CHARACTERS; ++idx) {
		TattooPerson &person = people[idx];

		if (person._type != INVALID) {
			if (_goToScene == -1 || _cAnim.size() == 0) {
				if (person._type == REMOVE) {
					screen.slamRect(person.getOldBounds());
					person._type = INVALID;
				} else {
					if (person._tempScaleVal == SCALE_THRESHOLD) {
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
				if (obj._scaleVal == SCALE_THRESHOLD)
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
				if (obj._scaleVal == SCALE_THRESHOLD)
					screen.flushImage(obj._imageFrame, obj._position, &obj._oldPosition.x, &obj._oldPosition.y,
						&obj._oldSize.x, &obj._oldSize.y);
				else
					screen.flushScaleImage(obj._imageFrame, obj._position, &obj._oldPosition.x, &obj._oldPosition.y,
						&obj._oldSize.x, &obj._oldSize.y, obj._scaleVal);
				obj._type = HIDDEN;
			}
		}
	}

	if (_activeCAnim.active() || _activeCAnim._zPlacement == REMOVE) {
		if (_activeCAnim._zPlacement != REMOVE) {
			screen.flushImage(&_activeCAnim._imageFrame, _activeCAnim._position, _activeCAnim._oldBounds, _activeCAnim._scaleVal);
		} else {
			screen.slamRect(_activeCAnim._removeBounds);
			_activeCAnim._removeBounds = Common::Rect(0, 0, 0, 0);
			_activeCAnim._zPlacement = -1;		// Reset _zPlacement so we don't REMOVE again
		}
	}
}

int TattooScene::getScaleVal(const Point32 &pt) {
	bool found = false;
	int result = SCALE_THRESHOLD;
	Common::Point pos(pt.x / FIXED_INT_MULTIPLIER, pt.y / FIXED_INT_MULTIPLIER);

	for (uint idx = 0; idx < _scaleZones.size() && !found; ++idx) {
		ScaleZone &sz = _scaleZones[idx];
		if (sz.contains(pos)) {
			int n = (sz._bottomNumber - sz._topNumber) * 100 / sz.height() * (pos.y - sz.top) / 100 + sz._topNumber;
			result = 25600L / n;
			// CHECKME: Shouldn't we set 'found' at this place?
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

int TattooScene::startCAnim(int cAnimNum, int playRate) {
	TattooEngine &vm = *(TattooEngine *)_vm;
	Events &events = *_vm->_events;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	Resources &res = *_vm->_res;
	Talk &talk = *_vm->_talk;
	UserInterface &ui = *_vm->_ui;

	// Exit immediately if the anim number is out of range, or the anim doesn't have a position specified
	if (cAnimNum < 0 || cAnimNum >= (int)_cAnim.size() || _cAnim[cAnimNum]._position.x == -1)
		// Return out of range error
		return -1;

	// Get the co-ordinates that the Player & NPC #1 must walk to and end on
	CAnim &cAnim = _cAnim[cAnimNum];
	PositionFacing goto1 = cAnim._goto[0];
	PositionFacing goto2 = cAnim._goto[1];
	PositionFacing teleport1 = cAnim._teleport[0];
	PositionFacing teleport2 = cAnim._teleport[1];

	// See if the Player must walk to a position before the animation starts
	SpriteType savedPlayerType = people[HOLMES]._type;
	if (goto1.x != -1 && people[HOLMES]._type == CHARACTER) {
		if (people[HOLMES]._position != goto1)
			people[HOLMES].walkToCoords(goto1, goto1._facing);
	}

	if (talk._talkToAbort)
		return 1;

	// See if NPC #1 must walk to a position before the animation starts
	SpriteType savedNPCType = people[WATSON]._type;
	if (goto2.x != -1 && people[WATSON]._type == CHARACTER) {
		if (people[WATSON]._position != goto2)
			people[WATSON].walkToCoords(goto2, goto2._facing);
	}

	if (talk._talkToAbort)
		return 1;

	// Turn the player (and NPC #1 if neccessary) off before running the canimation
	if (teleport1.x != -1 && savedPlayerType == CHARACTER)
		people[HOLMES]._type = REMOVE;

	if (teleport2.x != -1 && savedNPCType == CHARACTER)
		people[WATSON]._type = REMOVE;

	if (ui._windowOpen)
		ui.banishWindow();

	// Open up the room resource file and get the data for the animation
	Common::SeekableReadStream *stream = res.load(_roomFilename);
	stream->seek(44 + cAnimNum * 4);
	stream->seek(stream->readUint32LE());
	Common::SeekableReadStream *animStream = stream->readStream(cAnim._dataSize);
	delete stream;

	// Set up the active animation
	_activeCAnim._position = cAnim._position;
	_activeCAnim._oldBounds = Common::Rect(0, 0, 0, 0);
	_activeCAnim._flags = cAnim._flags;
	_activeCAnim._scaleVal = cAnim._scaleVal;
	_activeCAnim._zPlacement = 0;

	_activeCAnim.load(animStream, _compressed);

	while (!_vm->shouldQuit()) {
		// Get the next frame
		if (!_activeCAnim.getNextFrame())
			break;

		// Draw the frame
		doBgAnim();

		// Check for Escape key being pressed to abort animation
		events.pollEvents();
		if (events.kbHit()) {
			Common::KeyState keyState = events.getKey();

			if (keyState.keycode == Common::KEYCODE_ESCAPE && vm._runningProlog) {
				_vm->setFlags(-76);
				_vm->setFlags(396);
				_goToScene = STARTING_GAME_SCENE;
				talk._talkToAbort = true;
				_activeCAnim.close();
				break;
			}
		}
	}

	// Turn the people back on
	people[HOLMES]._type = savedPlayerType;
	if (teleport2.x != -1)
		people[WATSON]._type = savedNPCType;

	// Teleport the Player to the ending coordinates if necessary
	if (teleport1.x != -1 && savedPlayerType == CHARACTER) {
		people[HOLMES]._position = teleport1;
		people[HOLMES]._sequenceNumber = teleport1._facing;
		people[HOLMES].gotoStand();
	}

	// Teleport Watson to the ending coordinates if necessary
	if (teleport2.x != -1 && savedNPCType == CHARACTER) {
		people[WATSON]._position = teleport2;
		people[WATSON]._sequenceNumber = teleport2._facing;
		people[WATSON].gotoStand();
	}

	// Flag the Canimation to be cleared
	_activeCAnim._zPlacement = REMOVE;
	_activeCAnim._removeBounds = _activeCAnim._oldBounds;
	_vm->_ui->_bgFound = -1;

	// Free up the animation
	_activeCAnim.close();

	return 1;
}

void TattooScene::setNPCPath(int npc) {
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	SaveManager &saves = *_vm->_saves;
	Talk &talk = *_vm->_talk;

	// Don't do initial scene setup if a savegame has just been loaded
	if (saves._justLoaded)
		return;

	people[npc].clearNPC();
	people[npc]._npcName = Common::String::format("WATS%.2dA", _currentScene);

	// If we're in the middle of a script that will continue once the scene is loaded,
	// return without calling the path script
	if (talk._scriptMoreFlag == 1 || talk._scriptMoreFlag == 3)
		return;

	// Turn off all the NPCs, since the talk script will turn them back on as needed
	for (int idx = 1; idx < MAX_CHARACTERS; ++idx)
		people[idx]._type = INVALID;

	// Call the path script for the scene
	Common::String pathFile = Common::String::format("PATH%.2dA", _currentScene);
	talk.talkTo(pathFile);
}

int TattooScene::findBgShape(const Common::Point &pt) {
	People &people = *_vm->_people;
	UserInterface &ui = *_vm->_ui;

	if (!_doBgAnimDone)
		// New frame hasn't been drawn yet
		return -1;

	int result = -1;
	for (int idx = (int)_bgShapes.size() - 1; idx >= 0 && result == -1; --idx) {
		Object &o = _bgShapes[idx];

		if (o._type != INVALID && o._type != NO_SHAPE && o._type != HIDDEN &&
				(o._aType <= PERSON || (ui._menuMode == LAB_MODE && o._aType == SOLID))) {
			if (o.getNewBounds().contains(pt))
				result = idx;
		} else if (o._type == NO_SHAPE) {
			if (o.getNoShapeBounds().contains(pt))
				result = idx;
		}
	}

	// Now check for the mouse being over an NPC. If so, it overrides any found bg object
	for (int idx = 1; idx < MAX_CHARACTERS; ++idx) {
		Person &person = people[idx];

		if (person._type == CHARACTER) {
			int scaleVal = getScaleVal(person._position);
			Common::Rect charRect;

			if (scaleVal == SCALE_THRESHOLD)
				charRect = Common::Rect(person.frameWidth(), person.frameHeight());
			else
				charRect = Common::Rect(person._imageFrame->sDrawXSize(scaleVal), person._imageFrame->sDrawYSize(scaleVal));
			charRect.moveTo(person._position.x / FIXED_INT_MULTIPLIER, person._position.y / FIXED_INT_MULTIPLIER
				- charRect.height());

			if (charRect.contains(pt))
				result = 1000 + idx;
		}
	}

	return result;
}

void TattooScene::synchronize(Serializer &s) {
	TattooEngine &vm = *(TattooEngine *)_vm;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Scene::synchronize(s);

	if (s.isLoading()) {
		// In case we were showing the intro prologue or the ending credits, stop them
		vm._runningProlog = false;
		ui._creditsWidget.close();
	}
}

int TattooScene::closestZone(const Common::Point &pt) {
	int zone = -1;
	int dist = 9999;

	for (uint idx = 0; idx < _zones.size(); ++idx) {
		Common::Rect &r = _zones[idx];

		// Check the distance from the point to the center of the zone
		int d = ABS(r.left + (r.width() / 2) - pt.x) + ABS(r.top + (r.height() / 2) - pt.y);
		if (d < dist) {
			dist = d;
			zone = idx;
		}

		// Check the distance from the point to the upper left of the zone
		d = ABS((int)(r.left - pt.x)) + ABS((int)(r.top - pt.y));
		if (d < dist)
		{
			dist = d;
			zone = idx;
		}

		// Check the distance from the point to the upper right of the zone
		d = ABS(r.left + r.width() - pt.x) + ABS(r.top - pt.y);
		if (d < dist) {
			dist = d;
			zone = idx;
		}

		// Check the distance from the point to the lower left of the zone
		d = ABS(r.left - pt.x) + ABS(r.top + r.height() - pt.y);
		if (d < dist) {
			dist = d;
			zone = idx;
		}

		// Check the distance from the point to the lower right of the zone
		d = ABS(r.left + r.width() - pt.x) + ABS(r.top + r.height() - pt.y);
		if (d < dist) {
			dist = d;
			zone = idx;
		}
	}

	return zone;
}

} // End of namespace Tattoo

} // End of namespace Sherlock
