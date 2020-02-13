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

#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo_journal.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

bool WidgetList::contains(const WidgetBase *item) const {
	for (const_iterator i = begin(); i != end(); ++i) {
		if ((*i) == item)
			return true;
	}

	return false;
}

/*-------------------------------------------------------------------------*/

TattooUserInterface::TattooUserInterface(SherlockEngine *vm): UserInterface(vm),
		_inventoryWidget(vm), _messageWidget(vm), _textWidget(vm), _tooltipWidget(vm),
		_verbsWidget(vm), _creditsWidget(vm), _optionsWidget(vm), _quitWidget(vm) {
	Common::fill(&_lookupTable[0], &_lookupTable[PALETTE_COUNT], 0);
	Common::fill(&_lookupTable1[0], &_lookupTable1[PALETTE_COUNT], 0);
	_scrollSize = 0;
	_scrollSpeed = 16;
	_drawMenu = false;
	_bgShape = nullptr;
	_personFound = false;
	_lockoutTimer = 0;
	_exitZone = -1;
	_scriptZone = -1;
	_arrowZone = _oldArrowZone = -1;
	_activeObj = -1;
	_cAnimFramePause = 0;
	_scrollHighlight = SH_NONE;
	_mask = _mask1 = nullptr;
	_maskCounter = 0;

	_interfaceImages = new ImageFile("intrface.vgs");
}

TattooUserInterface::~TattooUserInterface() {
	delete _interfaceImages;
	delete _mask;
	delete _mask1;
}

void TattooUserInterface::initScrollVars() {
	Screen &screen = *_vm->_screen;
	_scrollSize = screen._backBuffer1.width() - SHERLOCK_SCREEN_WIDTH;
	_targetScroll = Common::Point(0, 0);
	screen._currentScroll = Common::Point(0, 0);
}

void TattooUserInterface::lookAtObject() {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Sound &sound = *_vm->_sound;
	Talk &talk = *_vm->_talk;
	Common::Point mousePos = events.mousePos();
	Common::String desc;

	_lookPos = mousePos;
	_menuMode = LOOK_MODE;

	if (_personFound) {
		desc = people[_bgFound - 1000]._examine;
	} else {
		// Check if there is a Look animation
		if (_bgShape->_lookcAnim != 0) {
			int cAnimSpeed = _bgShape->_lookcAnim & 0xe0;
			cAnimSpeed >>= 5;
			++cAnimSpeed;

			_cAnimFramePause = _bgShape->_lookFrames;
			desc = _bgShape->_examine;

			int cNum = (_bgShape->_lookcAnim & 0x1f) - 1;
			scene.startCAnim(cNum);
		} else if (_bgShape->_lookPosition.y != 0) {
			// Need to walk to object before looking at it
			people[HOLMES].walkToCoords(_bgShape->_lookPosition, _bgShape->_lookPosition._facing);
		}

		if (!talk._talkToAbort) {
			desc = _bgShape->_examine;

			if (_bgShape->_lookFlag)
				_vm->setFlags(_bgShape->_lookFlag);

			// Find the Sound File to Play if there is one
			if (!desc.hasPrefix("_")) {
				for (uint idx = 0; idx < scene._objSoundList.size(); ++idx) {
					// Get the object name up to the equals
					const char *p = strchr(scene._objSoundList[idx].c_str(), '=');

					// Form the name and remove any trailing spaces
					Common::String name(scene._objSoundList[idx].c_str(), p);
					while (name.hasSuffix(" "))
						name.deleteLastChar();

					// See if this Object Sound List entry matches the object's name
					if (!_bgShape->_name.compareToIgnoreCase(name)) {
						// Move forward to get the sound filename
						while ((*p == ' ') || (*p == '='))
							++p;

						// If it's not "NONE", play the speech File
						Common::String soundName(p);
						if (soundName.compareToIgnoreCase("NONE")) {
							soundName.toLowercase();
							if (!soundName.contains('.'))
								soundName += ".wav";

							sound.playSound(soundName, WAIT_RETURN_IMMEDIATELY);
						}

						break;
					}
				}
			}
		}
	}

	// Only show the desciption if the object has one, and if no talk file interrupted while walking to it
	if (!talk._talkToAbort && !desc.empty()) {
		if (_cAnimFramePause == 0)
			printObjectDesc(desc, true);
		else
			// The description was already printed by an animation
			_cAnimFramePause = 0;
	} else if (desc.empty()) {
		// There was no description to display, so reset back to STD_MODE
		_menuMode = STD_MODE;
	}
}

void TattooUserInterface::printObjectDesc(const Common::String &str, bool firstTime) {
	Events &events = *_vm->_events;
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	Talk &talk = *_vm->_talk;

	if (str.hasPrefix("_")) {
		// The passed string specifies a talk file
		_lookScriptFlag = true;
		events.setCursor(MAGNIFY);
		int savedSelector = _selector;

		if (!_invLookFlag)
			_windowOpen = false;

		talk.talkTo(str.c_str() + 1);
		_lookScriptFlag = false;

		if (talk._talkToAbort) {
			events.setCursor(ARROW);
			return;
		}

		// See if we're looking at an inventory item
		if (_invLookFlag) {
			_selector = _oldSelector = savedSelector;
			doInventory(0);
			_invLookFlag = false;

		} else {
			// Nope
			events.setCursor(ARROW);
			_key = -1;
			_menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;
			events._pressed = events._released = events._rightReleased = false;
			events._oldButtons = 0;
		}
	} else {
		events._pressed = events._released = events._rightReleased = false;

		// Show text dialog
		_textWidget.load(str);
		_textWidget.summonWindow();

		if (firstTime)
			_selector = _oldSelector = -1;

		_drawMenu = _windowOpen = true;
	}
}

void TattooUserInterface::doJournal() {
	TattooJournal &journal = *(TattooJournal *)_vm->_journal;
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	Screen &screen = *_vm->_screen;
	byte lookupTable[PALETTE_COUNT], lookupTable1[PALETTE_COUNT];

	Common::copy(&_lookupTable[0], &_lookupTable[PALETTE_COUNT], &lookupTable[0]);
	Common::copy(&_lookupTable1[0], &_lookupTable1[PALETTE_COUNT], &lookupTable1[0]);
	_menuMode = JOURNAL_MODE;
	journal.show();

	_menuMode = STD_MODE;
	_windowOpen = false;
	_key = -1;

	// Restore the the old screen palette and greyscale lookup table
	screen.clear();
	screen.setPalette(screen._cMap);
	Common::copy(&lookupTable[0], &lookupTable[PALETTE_COUNT], &_lookupTable[0]);
	Common::copy(&lookupTable1[0], &lookupTable1[PALETTE_COUNT], &_lookupTable1[0]);

	// Restore the scene
	screen._backBuffer1.SHblitFrom(screen._backBuffer2);
	scene.updateBackground();
	screen.slamArea(screen._currentScroll.x, screen._currentScroll.y, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
}

void TattooUserInterface::reset() {
	UserInterface::reset();
	_lookPos = Common::Point(SHERLOCK_SCREEN_WIDTH / 2, SHERLOCK_SCREEN_HEIGHT / 2);
	_tooltipWidget.setText("");
	_widgets.clear();
	_fixedWidgets.clear();
}

void TattooUserInterface::handleInput() {
	TattooEngine &vm = *(TattooEngine *)_vm;
	Events &events = *_vm->_events;
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	Common::Point mousePos = events.mousePos();

	_keyState.keycode = Common::KEYCODE_INVALID;

	// Check for credits starting
	if (_vm->readFlags(3000) && !_creditsWidget.active())
		_creditsWidget.initCredits();

	// Check the mouse positioning
	if (events.isCursorVisible())
		_bgFound = scene.findBgShape(mousePos);
	_personFound = _bgFound >= 1000;
	_bgShape = (_bgFound != -1 && _bgFound < 1000) ? &scene._bgShapes[_bgFound] : nullptr;

	if (_lockoutTimer)
		--_lockoutTimer;

	// Key handling
	if (events.kbHit()) {
		_keyState = events.getKey();

		if (_keyState.keycode == Common::KEYCODE_ESCAPE && vm._runningProlog && !_lockoutTimer) {
			vm.setFlags(-76);
			vm.setFlags(396);
			scene._goToScene = STARTING_GAME_SCENE;
		} else if (_menuMode == STD_MODE) {
			if (_keyState.keycode == Common::KEYCODE_s && vm._allowFastMode) {
				events.toggleSpeed();

			} else if (_keyState.keycode == Common::KEYCODE_l && _bgFound != -1) {
				// Beging used for testing that Look dialogs work
				lookAtObject();
			}
		}
	}

	if (!events.isCursorVisible())
		_keyState.keycode = Common::KEYCODE_INVALID;

	// If there's any active widgets/windows, let the most recently open one do event processing
	if (!_widgets.empty())
		_widgets.back()->handleEvents();
	else if (!_fixedWidgets.empty())
		_fixedWidgets.back()->handleEvents();

	// Handle input depending on what mode we're in
	switch (_menuMode) {
	case STD_MODE:
		doStandardControl();
		break;
	case LOOK_MODE:
		doLookControl();
		break;
	default:
		break;
	}
}

void TattooUserInterface::drawInterface(int bufferNum) {
	Screen &screen = *_vm->_screen;

	// Draw any active on-screen widgets
	for (Common::List<WidgetBase *>::iterator i = _fixedWidgets.begin(); i != _fixedWidgets.end(); ++i)
		(*i)->draw();
	for (Common::List<WidgetBase *>::iterator i = _widgets.begin(); i != _widgets.end(); ++i)
		(*i)->draw();

	// Handle drawing credits
	// TODO: See if credits are only shown on a single screen. If so, _fixedWidgets could be used
	if (_creditsWidget.active())
		_creditsWidget.drawCredits();

	// Bring the widgets to the screen
	if (_mask != nullptr)
		screen._flushScreen = true;
}

void TattooUserInterface::doBgAnimRestoreUI() {
	TattooScene &scene = *((TattooScene *)_vm->_scene);
	Screen &screen = *_vm->_screen;

	// If there are any on-screen widgets, then erase them
	for (Common::List<WidgetBase *>::iterator i = _widgets.begin(); i != _widgets.end(); ++i)
		(*i)->erase();
	for (Common::List<WidgetBase *>::iterator i = _fixedWidgets.begin(); i != _fixedWidgets.end(); ++i)
		(*i)->erase();

	// If there is a Text Tag being display, restore the area underneath it
	_tooltipWidget.erase();

	// If a canimation is active, restore the graphics underneath it
	if (scene._activeCAnim.active())
		screen.restoreBackground(scene._activeCAnim._oldBounds);

	// If a canimation just ended, remove its graphics from the backbuffer
	if (scene._activeCAnim._removeBounds.width() > 0)
		screen.restoreBackground(scene._activeCAnim._removeBounds);
}

void TattooUserInterface::doScroll() {
	Screen &screen = *_vm->_screen;

	// If we're already at the target scroll position, nothing needs to be done
	if (_targetScroll.x == screen._currentScroll.x)
		return;

	screen._flushScreen = true;
	if (_targetScroll.x > screen._currentScroll.x) {
		screen._currentScroll.x += _scrollSpeed;
		if (screen._currentScroll.x > _targetScroll.x)
			screen._currentScroll.x = _targetScroll.x;
	} else if (_targetScroll.x < screen._currentScroll.x) {
		screen._currentScroll.x -= _scrollSpeed;
		if (screen._currentScroll.x < _targetScroll.x)
			screen._currentScroll.x = _targetScroll.x;
	}

	// Reset the default look position to the center of the new screen area
	_lookPos = screen._currentScroll + Common::Point(SHERLOCK_SCREEN_WIDTH / 2, SHERLOCK_SCREEN_HEIGHT / 2);
}

void TattooUserInterface::doStandardControl() {
	TattooEngine &vm = *(TattooEngine *)_vm;
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	SaveManager &saves = *_vm->_saves;
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	Talk &talk = *_vm->_talk;
	Common::Point mousePos = events.mousePos();

	// Don't do any input processing whilst the prolog is running
	if (vm._runningProlog)
		return;

	// When the end credits are active, any press will open the ScummVM global main menu
	if (_creditsWidget.active()) {
		if (_keyState.keycode || events._released || events._rightReleased) {
			vm._canLoadSave = true;
			vm.openMainMenuDialog();
			vm._canLoadSave = false;
		}

		return;
	}

	// Display the names of any Objects the cursor is pointing at
	displayObjectNames();

	switch (_keyState.keycode) {
	case Common::KEYCODE_F5:
		// Save game
		events.warpMouse();
		saveGame();
		return;

	case Common::KEYCODE_F7:
		// Load game
		events.warpMouse();
		loadGame();
		return;

	case Common::KEYCODE_F1:
		// Display journal
		if (vm.readFlags(FLAG_PLAYER_IS_HOLMES)) {
			freeMenu();
			doJournal();

			// See if we're in a Lab Table Room
			_menuMode = (scene._labTableScene) ? LAB_MODE : STD_MODE;
			return;
		}
		break;

	case Common::KEYCODE_TAB:
	case Common::KEYCODE_F3:
		// Display inventory
		freeMenu();
		doInventory(3);
		return;

	case Common::KEYCODE_F4:
		// Display options
		events.warpMouse();
		_optionsWidget.load();
		return;

	case Common::KEYCODE_F10:
		// Quit menu
		freeMenu();
		events.warpMouse();
		doQuitMenu();
		return;

	default:
		break;
	}

	// See if a mouse button was released
	if (events._released || events._rightReleased) {
		// See if the mouse was released in an exit (Arrow) zone. Unless it's also pointing at an object
		// within the zone, in which case the object gets precedence
		_exitZone = -1;
		if (_arrowZone != -1 && events._released)
			_exitZone = _arrowZone;

		// Turn any Text display off
		if (_arrowZone == -1 || events._rightReleased)
			freeMenu();

		bool noDesc = false;
		if (_personFound) {
			if (people[_bgFound - 1000]._description.empty() || people[_bgFound - 1000]._description.hasPrefix(" "))
				noDesc = true;
		} else if (_bgFound != -1) {
			if (_bgShape->_description.empty() || _bgShape->_description.hasPrefix(" "))
				noDesc = true;
		} else {
			noDesc = true;
		}

		if (events._rightReleased) {
			// Show the verbs menu for the highlighted object
			_tooltipWidget.banishWindow();
			saves.createThumbnail();
			_verbsWidget.load(!noDesc);
			_verbsWidget.summonWindow();

			_selector = _oldSelector = -1;
			_activeObj = _bgFound;
			_menuMode = VERB_MODE;
		} else if (_personFound || (_bgFound != -1 && _bgFound < 1000 && _bgShape->_aType == PERSON)) {
			// The object found is a person (the default for people is TALK)
			talk.initTalk(_bgFound);
			_activeObj = -1;
		} else if (!noDesc) {
			// Either call the code to Look at its Examine Field or call the Exit animation
			// if the object is an exit, specified by the first four characters of the name being "EXIT"
			Common::String name = _personFound ? people[_bgFound - 1000]._name : _bgShape->_name;
			if (!name.hasPrefix("EXIT")) {
				lookAtObject();
			} else {
				// Run the Exit animation and set which scene to go to next
				for (int idx = 0; idx < 6; ++idx) {
					if (!_bgShape->_use[idx]._verb.compareToIgnoreCase("Open")) {
						checkAction(_bgShape->_use[idx], _bgFound);
						_activeObj = -1;
					}
				}
			}
		} else {
			// See if there are any Script Zones where they clicked
			if (scene.checkForZones(mousePos, _scriptZone) != 0) {
				// Mouse click in a script zone
				events._pressed = events._released = false;
			} else if (scene.checkForZones(mousePos, NOWALK_ZONE) != 0) {
				events._pressed = events._released = false;
			} else {
				// Walk to where the mouse was clicked
				people[HOLMES]._walkDest = mousePos;
				people[HOLMES].goAllTheWay();
			}
		}
	}
}

void TattooUserInterface::doLookControl() {
	Events &events = *_vm->_events;
	TattooScene &scene = *(TattooScene *)_vm->_scene;

	// See if a mouse button was released or a key pressed to close the active look dialog
	if (events._released || events._rightReleased || _keyState.keycode) {
		// See if we were looking at an inventory object
		if (!_invLookFlag) {
			// See if there is any more text to display
			if (!_textWidget._remainingText.empty()) {
				printObjectDesc(_textWidget._remainingText, false);
			} else {
				// Otherwise restore the background and go back into STD_MODE
				freeMenu();
				_key = -1;
				_menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;

				events.setCursor(ARROW);
				events._pressed = events._released = events._rightReleased = false;
				events._oldButtons = 0;
			}
		} else {
			// We were looking at a Inventory object
			// Erase the text window, and then redraw the inventory window
			_textWidget.banishWindow();
			doInventory(0);

			_invLookFlag = false;
			_key = -1;

			events.setCursor(ARROW);
			events._pressed = events._released = events._rightReleased = false;
			events._oldButtons = 0;
		}
	}
}

void TattooUserInterface::displayObjectNames() {
	Events &events = *_vm->_events;
	Scene &scene = *_vm->_scene;
	Common::Point mousePos = events.mousePos();
	_arrowZone = -1;

	if (_bgFound == -1 || scene._currentScene == OVERHEAD_MAP2) {
		for (uint idx = 0; idx < scene._exits.size() && _arrowZone == -1; ++idx) {
			Exit &exit = scene._exits[idx];
			if (exit.contains(mousePos))
				_arrowZone = idx;
		}
	}

	_tooltipWidget.handleEvents();
	_oldArrowZone = _arrowZone;
}

void TattooUserInterface::doInventory(int mode) {
	People &people = *_vm->_people;
	people[HOLMES].gotoStand();

	_inventoryWidget.load(mode);
	_inventoryWidget.summonWindow();

	_menuMode = INV_MODE;
}

void TattooUserInterface::doControls() {
	_optionsWidget.load();
}

void TattooUserInterface::pickUpObject(int objNum) {
	Inventory &inv = *_vm->_inventory;
	Scene &scene = *_vm->_scene;
	Talk &talk = *_vm->_talk;
	Object &obj = scene._bgShapes[objNum];
	bool printed = false;
	int verbField = -1;

	// Find which Verb field to use for pick up data
	for (int idx = 0; idx < 6; ++idx) {
		if (!scumm_stricmp(obj._use[idx]._target.c_str(), "*PICKUP"))
			verbField = idx;
	}

	if (verbField != -1) {
		if (obj._use[verbField]._cAnimNum)
			scene.startCAnim(obj._use[verbField]._cAnimNum - 1);
	}

	if (!talk._talkToAbort) {
		if (obj._type == NO_SHAPE)
			obj._type = INVALID;
		else
			// Erase shape
			obj._type = REMOVE;
	} else {
		return;
	}

	if (verbField != -1) {
		for (int idx = 0; idx < 4 && !talk._talkToAbort; ++idx) {
			if (obj.checkNameForCodes(obj._use[verbField]._names[idx])) {
				if (!talk._talkToAbort)
					printed = true;
			}
		}
	}

	if (talk._talkToAbort)
		return;

	// Add the item to the player's inventory
	inv.putItemInInventory(obj);

	if (!printed) {
		Common::String desc = obj._description;
		desc.setChar(tolower(desc[0]), 0);

		putMessage("%s %s", FIXED(PickedUp), desc.c_str());
	}

	if (_menuMode != TALK_MODE && _menuMode != MESSAGE_MODE) {
		_menuMode = STD_MODE;
		_keyState.keycode = Common::KEYCODE_INVALID;
	}
}

void TattooUserInterface::doQuitMenu() {
	_quitWidget.show();
}

void TattooUserInterface::putMessage(const char *formatStr, ...) {
	// Create the string to display
	va_list args;
	va_start(args, formatStr);
	Common::String str = Common::String::vformat(formatStr, args);
	va_end(args);

	// Open the message widget
	_menuMode = MESSAGE_MODE;
	_messageWidget.load(str, 25);
	_messageWidget.summonWindow();
}

void TattooUserInterface::setupBGArea(const byte cMap[PALETTE_SIZE]) {
	Scene &scene = *_vm->_scene;

	// This requires that there is a 16 grayscale palette sequence in the palette that goes from lighter
	// to darker as the palette numbers go up. The last palette entry in that run is specified by _bgColor
	byte *p = &_lookupTable[0];
	for (int idx = 0; idx < PALETTE_COUNT; ++idx)
		*p++ = BG_GREYSCALE_RANGE_END - (cMap[idx * 3] * 30 + cMap[idx * 3 + 1] * 59 + cMap[idx * 3 + 2] * 11) / 480;

	// If we're going to a scene with a haze special effect, initialize the translate table to lighten the colors
	if (_mask != nullptr) {
		p = &_lookupTable1[0];

		for (int idx = 0; idx < PALETTE_COUNT; ++idx) {
			int r, g, b;
			switch (scene._currentScene) {
			case 8:
				r = cMap[idx * 3] * 4 / 5;
				g = cMap[idx * 3 + 1] * 3 / 4;
				b = cMap[idx * 3 + 2] * 3 / 4;
				break;

			case 18:
			case 68:
				r = cMap[idx * 3] * 4 / 3;
				g = cMap[idx * 3 + 1] * 4 / 3;
				b = cMap[idx * 3 + 2] * 4 / 3;
				break;

			case 7:
			case 53:
				r = cMap[idx * 3] * 4 / 3;
				g = cMap[idx * 3 + 1] * 4 / 3;
				b = cMap[idx * 3 + 2] * 4 / 3;
				break;

			default:
				r = g = b = 0;
				break;
			}

			byte c = 0xff;
			int cd = 99999;

			for (int pal = 0; pal < PALETTE_COUNT; ++pal) {
				int d = (r - cMap[pal * 3]) * (r - cMap[pal * 3]) + (g - cMap[pal * 3 + 1]) * (g - cMap[pal * 3 + 1]) +
					(b - cMap[pal * 3 + 2]) * (b - cMap[pal * 3 + 2]);

				if (d < cd) {
					c = pal;
					cd = d;
					if (!d)
						break;
				}
			}
			*p++ = c;
		}
	}
}

void TattooUserInterface::doBgAnimEraseBackground() {
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;

	static const int16 OFFSETS[16] = { -1, -2, -3, -3, -2, -1, -1, 0, 1, 2, 3, 3, 2, 1, 0, 0 };

	if (_mask != nullptr) {
		// Since a mask is active, restore the screen from the secondary back buffer prior to applying the mask
		screen._backBuffer1.SHblitFrom(screen._backBuffer2, screen._currentScroll, Common::Rect(screen._currentScroll.x, 0,
			screen._currentScroll.x + SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));

		switch (scene._currentScene) {
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
				if (++_maskOffset.x == screen._backBuffer1.width())
					_maskOffset.x = 0;
			}
			break;

		default:
			break;
		}
	} else {
		// Standard scene without mask, so call user interface to erase any UI elements as necessary
		doBgAnimRestoreUI();

		// Restore background for any areas covered by characters and shapes
		for (int idx = 0; idx < MAX_CHARACTERS; ++idx)
			screen.restoreBackground(Common::Rect(people[idx]._oldPosition.x, people[idx]._oldPosition.y,
				people[idx]._oldPosition.x + people[idx]._oldSize.x, people[idx]._oldPosition.y + people[idx]._oldSize.y));

		for (uint idx = 0; idx < scene._bgShapes.size(); ++idx) {
			Object &obj = scene._bgShapes[idx];

			if ((obj._type == ACTIVE_BG_SHAPE && (obj._maxFrames > 1 || obj._delta.x != 0 || obj._delta.y != 0)) ||
					obj._type == HIDE_SHAPE || obj._type == REMOVE)
				screen._backBuffer1.SHblitFrom(screen._backBuffer2, obj._oldPosition,
					Common::Rect(obj._oldPosition.x, obj._oldPosition.y, obj._oldPosition.x + obj._oldSize.x,
						obj._oldPosition.y + obj._oldSize.y));
		}

		// If credits are active, erase the area they cover
		if (_creditsWidget.active())
			_creditsWidget.eraseCredits();
	}

	for (uint idx = 0; idx < scene._bgShapes.size(); ++idx) {
		Object &obj = scene._bgShapes[idx];

		if (obj._type == NO_SHAPE && (obj._flags & 1) == 0) {
			screen._backBuffer1.SHblitFrom(screen._backBuffer2, obj._position, obj.getNoShapeBounds());

			obj._oldPosition = obj._position;
			obj._oldSize = obj._noShapeSize;
		}
	}

	// Adjust the Target Scroll if needed
	if ((people[people._walkControl]._position.x / FIXED_INT_MULTIPLIER - screen._currentScroll.x) <
			(SHERLOCK_SCREEN_WIDTH / 8) && people[people._walkControl]._delta.x < 0) {

		_targetScroll.x = (short)(people[people._walkControl]._position.x / FIXED_INT_MULTIPLIER -
				SHERLOCK_SCREEN_WIDTH / 8 - 250);
		if (_targetScroll.x < 0)
			_targetScroll.x = 0;
	}

	if ((people[people._walkControl]._position.x / FIXED_INT_MULTIPLIER - screen._currentScroll.x) >
			(SHERLOCK_SCREEN_WIDTH / 4 * 3)	&& people[people._walkControl]._delta.x > 0)
		_targetScroll.x = (short)(people[people._walkControl]._position.x / FIXED_INT_MULTIPLIER -
			SHERLOCK_SCREEN_WIDTH / 4 * 3 + 250);

	if (_targetScroll.x > _scrollSize)
		_targetScroll.x = _scrollSize;

	doScroll();
}

void TattooUserInterface::drawMaskArea(bool mode) {
	Scene &scene = *_vm->_scene;
	int xp = mode ? _maskOffset.x : 0;

	if (_mask != nullptr) {
		switch (scene._currentScene) {
		case 7:
			maskArea(*_mask, Common::Point(_maskOffset.x - SHERLOCK_SCREEN_WIDTH, 110));
			maskArea(*_mask, Common::Point(_maskOffset.x, 110));
			maskArea(*_mask, Common::Point(_maskOffset.x + SHERLOCK_SCREEN_WIDTH, 110));
			break;

		case 8:
			maskArea(*_mask, Common::Point(_maskOffset.x - SHERLOCK_SCREEN_WIDTH, 180));
			maskArea(*_mask, Common::Point(_maskOffset.x, 180));
			maskArea(*_mask, Common::Point(_maskOffset.x + SHERLOCK_SCREEN_WIDTH, 180));
			if (!_vm->readFlags(880))
				maskArea(*_mask1, Common::Point(940, 300));
			break;

		case 18:
			maskArea(*_mask, Common::Point(xp, 203));
			if (!_vm->readFlags(189))
				maskArea(*_mask1, Common::Point(124 + xp, 239));
			break;

		case 53:
			maskArea(*_mask, Common::Point(_maskOffset.x, 110));
			if (mode)
				maskArea(*_mask, Common::Point(_maskOffset.x - SHERLOCK_SCREEN_WIDTH, 110));
			break;

		case 68:
			maskArea(*_mask, Common::Point(xp, 203));
			maskArea(*_mask1, Common::Point(124 + xp, 239));
			break;

		default:
			break;
		}
	}
}

void TattooUserInterface::maskArea(Common::SeekableReadStream &mask, const Common::Point &pt) {
	Screen &screen = *_vm->_screen;
	Surface &bb1 = screen._backBuffer1;
	mask.seek(0);
	int xSize = mask.readUint16LE();
	int ySize = mask.readUint16LE();
	int pixel, len, xp, yp;

	for (yp = 0; yp < ySize; ++yp) {
		byte *ptr = (byte *)bb1.getBasePtr(pt.x, pt.y + yp);

		for (xp = 0; xp < xSize;) {
			// The mask data consists of pairs of pixel/lengths, where all non-zero pixels means that the
			// given pixel on the back buffer is darkened (the mask pixel value isn't otherwise used)
			pixel = mask.readByte();
			len = mask.readByte();

			for (; len > 0; --len, ++xp, ++ptr) {
				if (pixel && (pt.x + xp) >= screen._currentScroll.x && (pt.x + xp) < (screen._currentScroll.x + SHERLOCK_SCREEN_WIDTH)) {
					*ptr = _lookupTable1[*ptr];
				}
			}
		}

		assert(xp == xSize);
	}
}

void TattooUserInterface::makeBGArea(const Common::Rect &r) {
	Screen &screen = *_vm->_screen;

	for (int yp = r.top; yp < r.bottom; ++yp) {
		byte *ptr = (byte *)screen._backBuffer1.getBasePtr(r.left, yp);

		for (int xp = r.left; xp < r.right; ++xp, ++ptr)
			*ptr = _lookupTable[*ptr];
	}

	screen.slamRect(r);
}

void TattooUserInterface::drawDialogRect(Surface &s, const Common::Rect &r, bool raised) {
	if (raised) {
		// Draw Left
		s.vLine(r.left, r.top, r.bottom - 1, INFO_TOP);
		s.vLine(r.left + 1, r.top, r.bottom - 2, INFO_TOP);
		// Draw Top
		s.hLine(r.left + 2, r.top, r.right - 1, INFO_TOP);
		s.hLine(r.left + 2, r.top + 1, r.right - 2, INFO_TOP);
		// Draw Right
		s.vLine(r.right - 1, r.top + 1, r.bottom - 1, INFO_BOTTOM);
		s.vLine(r.right - 2, r.top + 2, r.bottom - 1, INFO_BOTTOM);
		// Draw Bottom
		s.hLine(r.left + 1, r.bottom - 1, r.right - 3, INFO_BOTTOM);
		s.hLine(r.left + 2, r.bottom - 2, r.right - 3, INFO_BOTTOM);

	} else {
		// Draw Left
		s.vLine(r.left, r.top, r.bottom - 1, INFO_BOTTOM);
		s.vLine(r.left + 1, r.top, r.bottom - 2, INFO_BOTTOM);
		// Draw Top
		s.hLine(r.left + 2, r.top, r.right - 1, INFO_BOTTOM);
		s.hLine(r.left + 2, r.top + 1, r.right - 2, INFO_BOTTOM);
		// Draw Right
		s.vLine(r.right - 1, r.top + 1, r.bottom - 1, INFO_TOP);
		s.vLine(r.right - 2, r.top + 2, r.bottom - 1, INFO_TOP);
		// Draw Bottom
		s.hLine(r.left + 1, r.bottom - 1, r.right - 3, INFO_TOP);
		s.hLine(r.left + 2, r.bottom - 2, r.right - 3, INFO_TOP);
	}
}

void TattooUserInterface::banishWindow(bool slideUp) {
	if (!_widgets.empty())
		_widgets.back()->banishWindow();
}

void TattooUserInterface::freeMenu() {
	for (Common::List<WidgetBase *>::iterator i = _widgets.begin(); i != _widgets.end(); ++i)
		(*i)->erase();
	_widgets.clear();
}

void TattooUserInterface::clearWindow() {
	banishWindow();
}

void TattooUserInterface::loadGame() {
	WidgetFiles &files = *(WidgetFiles *)_vm->_saves;
	files.show(SAVEMODE_LOAD);
}

void TattooUserInterface::saveGame() {
	WidgetFiles &files = *(WidgetFiles *)_vm->_saves;
	files.show(SAVEMODE_SAVE);
}

void TattooUserInterface::addFixedWidget(WidgetBase *widget) {
	_fixedWidgets.push_back(widget);
	widget->summonWindow();
}

} // End of namespace Tattoo

} // End of namespace Sherlock
