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

#include "common/events.h"
#include "common/stream.h"
#include "graphics/cursorman.h"

#include "startrek/graphics.h"

namespace StarTrek {

/**
 * Returns the index of the button at the given position, or -1 if none.
 */
int StarTrekEngine::getMenuButtonAt(Sprite *sprites, int numSprites, int x, int y) {
	for (int i = 0; i < numSprites; i++) {
		const Sprite &spr = sprites[i];

		if (spr.drawMode != 2)
			continue;

		int left = spr.pos.x - spr.bitmap->xoffset;
		int top = spr.pos.y - spr.bitmap->yoffset;

		// Oddly, this doesn't account for x/yoffset...
		int right = spr.pos.x + spr.bitmap->width - 1;
		int bottom = spr.pos.y + spr.bitmap->height - 1;

		if (x >= left && x <= right && y >= top && y <= bottom)
			return i;
	}

	return -1;
}

/**
 * This chooses a sprite from the list to place the mouse cursor at. The sprite it chooses
 * may be, for example, the top-leftmost one in the list. Exact behaviour is determined by
 * the "mode" parameter.
 *
 * If "containMouseSprite" is a valid index, it's ensured that the mouse is contained
 * within it. "mode" should be -1 in this case.
 */
void StarTrekEngine::chooseMousePositionFromSprites(Sprite *sprites, int numSprites, int containMouseSprite, int mode) {
	uint16 mouseX1 = 0x7fff; // Candidate positions to warp mouse to
	uint16 mouseY1 = 0x7fff;
	uint16 mouseX2 = 0x7fff;
	uint16 mouseY2 = 0x7fff;

	Common::Point mousePos = _gfx->getMousePos();

	// Ensure the cursor is contained within one of the sprites
	if (containMouseSprite >= 0 && containMouseSprite < numSprites) {
		Common::Rect rect = sprites[containMouseSprite].getRect();

		if (mousePos.x < rect.left || mousePos.x >= rect.right
				|| mousePos.y < rect.top || mousePos.y >= rect.bottom) {
			mousePos.x = (rect.left + rect.right) / 2;
			mousePos.y = (rect.top + rect.bottom) / 2;
		}
	}

	// Choose a sprite to warp the cursor to
	for (int i = 0; i < numSprites; i++) {
		Sprite *sprite = &sprites[i];
		if (sprite->drawMode != 2) // Skip hidden buttons
			continue;

		Common::Rect rect = sprite->getRect();

		int hCenter = (rect.left + rect.right) / 2;
		int vCenter = (rect.top + rect.bottom) / 2;

		// Choose which sprite is closest based on certain criteria?
		switch(mode) {
		case 0: // Choose topmost, leftmost sprite that's below the cursor
			if (((vCenter == mousePos.y && hCenter > mousePos.x) || vCenter > mousePos.y)
					&& (vCenter < mouseY1 || (vCenter == mouseY1 && hCenter < mouseX1))) {
				mouseX1 = hCenter;
				mouseY1 = vCenter;
			}
			// fall through

		case 4: // Choose topmost, leftmost sprite
			if (vCenter < mouseY2 || (vCenter == mouseY2 && hCenter < mouseX2)) {
				mouseX2 = hCenter;
				mouseY2 = vCenter;
			}
			break;

		case 1: // Choose bottommost, rightmost sprite that's above the cursor
			if (((vCenter == mousePos.y && hCenter < mousePos.x) || vCenter < mousePos.y)
					&& (mouseY1 == 0x7fff || vCenter > mouseY1
						|| (vCenter == mouseY1 && hCenter > mouseX1))) {
				mouseX1 = hCenter;
				mouseY1 = vCenter;
			}
			// fall through

		case 5: // Choose bottommost, rightmost sprite
			if (mouseY2 == 0x7fff || vCenter > mouseY2
					|| (vCenter == mouseY2 && hCenter > mouseX2)) {
				mouseX2 = hCenter;
				mouseY2 = vCenter;
			}
			break;

		case 2:
			// This seems broken... OR condition on first line has no affect on the logic...
			if ((vCenter < mousePos.y || (vCenter == mouseY1 && hCenter == mousePos.x))
					&& (mouseX1 == 0x7fff || vCenter >= mouseY1)) {
				mouseX1 = hCenter;
				mouseY1 = vCenter;
			}
			if (mouseX2 == 0x7fff || vCenter > mouseY2
					|| (hCenter == mouseX2 && vCenter == mouseY2)) {
				mouseX2 = hCenter;
				mouseY2 = vCenter;
			}
			break;

		case 3:
			// Similar to above...
			if ((vCenter > mousePos.y || (vCenter == mouseY1 && hCenter == mousePos.x))
					&& (mouseX1 == 0x7fff || vCenter <= mouseY1)) {
				mouseX1 = hCenter;
				mouseY1 = vCenter;
			}
			if (mouseX2 == 0x7fff || vCenter < mouseY2
					|| (hCenter == mouseX2 && vCenter == mouseY2)) {
				mouseX2 = hCenter;
				mouseY2 = vCenter;
			}
			break;
		}
	}

	// Warp mouse to one of the coordinates, if one is valid
	if (mouseX1 != 0x7fff) {
		mousePos.x = mouseX1;
		mousePos.y = mouseY1;
	} else if (mouseX2 != 0x7fff) {
		mousePos.x = mouseX2;
		mousePos.y = mouseY2;
	}

	_gfx->warpMouse(mousePos.x, mousePos.y);

}

/**
 * Draws or removes the outline on menu buttons when the cursor hovers on them, or leaves
 * them.
 */
void StarTrekEngine::drawMenuButtonOutline(SharedPtr<Bitmap> bitmap, byte color) {
	int lineWidth = bitmap->width-2;
	int offsetToBottom = (bitmap->height-3)*bitmap->width;

	byte *dest = bitmap->pixels + bitmap->width + 1;

	while (lineWidth--) {
		*dest = color;
		*(dest+offsetToBottom) = color;
		dest++;
	}

	int lineHeight = bitmap->height - 2;
	int offsetToRight = bitmap->width - 3;

	dest = bitmap->pixels + bitmap->width + 1;

	while (lineHeight--) {
		*dest = color;
		*(dest+offsetToRight) = color;
		dest += bitmap->width;
	}
}

void StarTrekEngine::showOptionsMenu(int x, int y) {
	bool tmpMouseControllingShip = _mouseControllingShip;
	_mouseControllingShip = false;

	Common::Point oldMousePos = _gfx->getMousePos();
	SharedPtr<Bitmap> oldMouseBitmap = _gfx->getMouseBitmap();

	_gfx->setMouseBitmap(_gfx->loadBitmap("options"));
	loadMenuButtons("options", x, y);

	uint32 disabledButtons = 0;
	if (_musicWorking) {
		if (_musicEnabled)
			disabledButtons |= (1 << OPTIONBUTTON_ENABLEMUSIC);
		else
			disabledButtons |= (1 << OPTIONBUTTON_DISABLEMUSIC);
	} else
		disabledButtons |= (1 << OPTIONBUTTON_ENABLEMUSIC) | (1 << OPTIONBUTTON_DISABLEMUSIC);

	if (_sfxWorking) {
		if (_sfxEnabled)
			disabledButtons |= (1 << OPTIONBUTTON_ENABLESFX);
		else
			disabledButtons |= (1 << OPTIONBUTTON_DISABLESFX);
	} else
		disabledButtons |= (1 << OPTIONBUTTON_ENABLESFX) | (1 << OPTIONBUTTON_DISABLESFX);

	disableMenuButtons(disabledButtons);
	chooseMousePositionFromSprites(_activeMenu->sprites, _activeMenu->numButtons, -1, 4);
	int event = handleMenuEvents(0, false);

	unloadMenuButtons();
	_mouseControllingShip = tmpMouseControllingShip;
	_gfx->setMouseBitmap(oldMouseBitmap);

	if (event != MENUEVENT_LCLICK_OFFBUTTON && event != MENUEVENT_RCLICK_OFFBUTTON)
		_gfx->warpMouse(oldMousePos.x, oldMousePos.y);


	// Can't use OPTIONBUTTON constants since the button retvals differ from the button
	// indices...
	switch(event) {
	case 0: // Save
		showSaveMenu();
		break;
	case 1: // Load
		showLoadMenu();
		break;
	case 2: // Enable music
		_sound->setMusicEnabled(true);
		break;
	case 3: // Disable music
		_sound->setMusicEnabled(false);
		break;
	case 4: // Enable sfx
		_sound->setSfxEnabled(true);
		break;
	case 5: // Disable sfx
		_sound->setSfxEnabled(false);
		break;
	case 6: // Quit
		showQuitGamePrompt(20, 20);
		break;
	case 7: // Text
		showTextConfigurationMenu(true);
		break;
	default:
		break;
	}
}

/**
 * Show the "action selection" menu, ie. look, talk, etc.
 */
int StarTrekEngine::showActionMenu() {
	const int actionMappingUp[] = { // Actions to jump to when up is pressed
		ACTION_TALK,    // <- ACTION_WALK
		ACTION_TALK,    // <- ACTION_USE
		ACTION_OPTIONS, // <- ACTION_GET
		ACTION_LOOK,    // <- ACTION_LOOK
		ACTION_LOOK,    // <- ACTION_TALK
		ACTION_OPTIONS  // <- ACTION_OPTIONS
	};
	const int actionMappingRight[] = { // Actions to jump to when right is pressed
		ACTION_GET,     // <- ACTION_WALK
		ACTION_WALK,    // <- ACTION_USE
		ACTION_GET,     // <- ACTION_GET
		ACTION_OPTIONS, // <- ACTION_LOOK
		ACTION_OPTIONS, // <- ACTION_TALK
		ACTION_OPTIONS  // <- ACTION_OPTIONS
	};
	const int actionMappingDown[] = { // Actions to jump to when down is pressed
		ACTION_GET,  // <- ACTION_WALK
		ACTION_WALK, // <- ACTION_USE
		ACTION_GET,  // <- ACTION_GET
		ACTION_TALK, // <- ACTION_LOOK
		ACTION_WALK, // <- ACTION_TALK
		ACTION_GET   // <- ACTION_OPTIONS
	};
	const int actionMappingLeft[] = { // Actions to jump to when left is pressed
		ACTION_USE,  // <- ACTION_WALK
		ACTION_USE,  // <- ACTION_USE
		ACTION_WALK, // <- ACTION_GET
		ACTION_USE,  // <- ACTION_LOOK
		ACTION_USE,  // <- ACTION_TALK
		ACTION_LOOK  // <- ACTION_OPTIONS
	};

	const Common::Point pos(50, 50); // Top-left position to put action menu at

	// Positions to put mouse cursor at to select actions (when using arrow keys)
	const Common::Point actionPositions[] = {
		Common::Point(7, 21),  // ACTION_USE
		Common::Point(48, 38), // ACTION_GET
		Common::Point(28, 5),  // ACTION_LOOK
		Common::Point(28, 14), // ACTION_TALK
		Common::Point(45, 9)   // ACTION_OPTIONS
	};

	TrekEvent event;
	Sprite menuSprite;

	bool keyboardControlledMouse = _keyboardControlsMouse;
	Common::Point oldMousePos = _gfx->getMousePos();

	bool addEventBack = false;
	int action = ACTION_WALK;

	menuSprite.bitmap = _gfx->loadBitmap("action");
	int menuWidth = menuSprite.bitmap->width;
	int menuHeight = menuSprite.bitmap->height;

	_gfx->warpMouse(pos.x + menuWidth / 2, pos.y + menuHeight / 2);

	_gfx->addSprite(&menuSprite);
	menuSprite.pos = pos;
	menuSprite.drawPriority = 15;

	chooseMouseBitmapForAction(action, false);

	_gfx->drawAllSprites();

	menuSprite.drawPriority2 = 8;
	bool displayMenu = true;

	while (displayMenu) {
		_sound->checkLoopMusic();

		if (!popNextEvent(&event))
			continue;

		switch (event.type) {

		case TREKEVENT_TICK:
			_gfx->incPaletteFadeLevel();
			_gfx->drawAllSprites();
			break;

		case TREKEVENT_LBUTTONDOWN:
selectAndExit:
			displayMenu = false;
			addEventBack = true;
			break;

		case TREKEVENT_MOUSEMOVE:
mousePosChanged:
			{
				Common::Point mouse = _gfx->getMousePos();
				Common::Point relMouse(mouse.x - pos.x, mouse.y - pos.y);

				Common::String bitmapName;
				Common::Point lockMousePoint(-1, -1);

				// Check if the mouse is hovering over one of the selectable actions
				if (relMouse.x >= 39 && relMouse.x <= 50 && relMouse.y >= 2 && relMouse.y <= 17) {
					action = ACTION_OPTIONS;
					bitmapName = "options";
					lockMousePoint = Common::Point(pos.x + 44, pos.y + 2);
				} else if (relMouse.x >= 18 && relMouse.x <= 38 && relMouse.y >= 2 && relMouse.y <= 9) {
					action = ACTION_LOOK;
					bitmapName = "look";
					lockMousePoint = Common::Point(pos.x + 28, pos.y + 6);
				} else if (relMouse.x >= 18 && relMouse.x <= 38 && relMouse.y >= 11 && relMouse.y <= 17) {
					action = ACTION_TALK;
					bitmapName = "talk";
					lockMousePoint = Common::Point(pos.x + 27, pos.y + 14);
				} else if (relMouse.x >= 2 && relMouse.x <= 13 && relMouse.y >= 16 && relMouse.y <= 26) {
					action = ACTION_USE;
					bitmapName = "use";
					lockMousePoint = Common::Point(pos.x + 7, pos.y + 19);
				} else if (relMouse.x >= 40 && relMouse.x <= 53 && relMouse.y >= 34 && relMouse.y <= 43) {
					action = ACTION_GET;
					bitmapName = "get";
					lockMousePoint = Common::Point(pos.x + 44, pos.y + 38);
				} else {
					action = ACTION_WALK;
					bitmapName = "walk";
				}

				_gfx->setMouseBitmap(_gfx->loadBitmap(bitmapName));

				if (lockMousePoint.x != -1)
					_gfx->lockMousePosition(lockMousePoint.x, lockMousePoint.y);
				else
					_gfx->unlockMousePosition();
			}
			break;

		case TREKEVENT_RBUTTONDOWN:
exitMenu:
			displayMenu = false;
			action = ACTION_WALK;
			break;

		case TREKEVENT_KEYDOWN: {
			int nextAction = action;
			const int *lookupArray;

			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_SPACE:
			case Common::KEYCODE_F2: // Exit menu without selecting anything
				goto exitMenu;
				goto exitMenu;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
			case Common::KEYCODE_F1: // Exit menu with whatever is selected
				goto selectAndExit;

			case Common::KEYCODE_PAGEUP:
			case Common::KEYCODE_KP9:
				nextAction = ACTION_OPTIONS;
				break;

			case Common::KEYCODE_PAGEDOWN:
			case Common::KEYCODE_KP3:
				nextAction = ACTION_GET;
				break;

			// Direction buttons
			case Common::KEYCODE_UP:
			case Common::KEYCODE_KP8:
				lookupArray = actionMappingUp;
				goto lookupNextAction;

			case Common::KEYCODE_RIGHT:
			case Common::KEYCODE_KP6:
				lookupArray = actionMappingRight;
				goto lookupNextAction;

			case Common::KEYCODE_DOWN:
			case Common::KEYCODE_KP2:
				lookupArray = actionMappingDown;
				goto lookupNextAction;

			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_KP4:
				lookupArray = actionMappingLeft;
				goto lookupNextAction;

lookupNextAction:
				// Use a lookup table to decide which action is next after a direction
				// button is pressed.
				assert((action >= ACTION_WALK && action <= ACTION_TALK) || action == ACTION_OPTIONS);
				nextAction = lookupArray[action == ACTION_OPTIONS ? 5 : action - 1];
				break;

			default:
				break;
			}

			if (nextAction == action)
				break;

			action = nextAction;

			// Warp mouse to the position of the selected action
			if (nextAction == ACTION_WALK)
				_gfx->warpMouse(pos.x + menuWidth / 2, pos.y + menuHeight / 2);
			else {
				assert((action >= ACTION_WALK && action <= ACTION_TALK) || action == ACTION_OPTIONS);
				const Common::Point &p = actionPositions[action == ACTION_OPTIONS ? 4 : action - 2];
				_gfx->warpMouse(pos.x + p.x, pos.y + p.y);
			}

			goto mousePosChanged;
		}

		default:
			break;
		}
	}

	playSoundEffectIndex(SND_SELECTION);

	menuSprite.dontDrawNextFrame();
	_gfx->drawAllSprites();
	_gfx->delSprite(&menuSprite);

	_gfx->unlockMousePosition();

	if (action == ACTION_OPTIONS) {
		showOptionsMenu(50, 50);
		action = ACTION_WALK;
	}

	Common::Point mouse = _gfx->getMousePos();
	if (mouse.x < pos.x || mouse.x >= pos.x + menuWidth || mouse.y < pos.y || mouse.y >= pos.y + menuHeight) {
		if (action == ACTION_WALK && addEventBack)
			addEventToQueue(event); // Add left-click event back to queue so Kirk can walk there
	} else
		_gfx->warpMouse(oldMousePos.x, oldMousePos.y);

	chooseMouseBitmapForAction(action, false);
	_keyboardControlsMouse = keyboardControlledMouse;
	return action;
}

/**
 * Loads a .MNU file, which is a list of buttons to display.
 */
void StarTrekEngine::loadMenuButtons(String mnuFilename, int xpos, int ypos) {
	if (_activeMenu == nullptr)
		_keyboardControlsMouseOutsideMenu = _keyboardControlsMouse;

	SharedPtr<Menu> oldMenu = _activeMenu;
	_activeMenu = SharedPtr<Menu>(new Menu());
	_activeMenu->nextMenu = oldMenu;

	SharedPtr<FileStream> stream = loadFile(mnuFilename + ".MNU");

	_activeMenu->menuFile = stream;
	_activeMenu->numButtons = _activeMenu->menuFile->size() / 16;

	for (int i = 0; i < _activeMenu->numButtons; i++) {
		_activeMenu->sprites[i] = Sprite();
		_gfx->addSprite(&_activeMenu->sprites[i]);
		_activeMenu->sprites[i].drawMode = 2;

		char bitmapBasename[11];
		stream->seek(i * 16, SEEK_SET);
		stream->read(bitmapBasename, 10);
		for (int j = 0; j < 10; j++) {
			if (bitmapBasename[j] == ' ')
				bitmapBasename[j] = '\0';
		}
		bitmapBasename[10] = '\0';

		_activeMenu->sprites[i].bitmap = _gfx->loadBitmap(bitmapBasename);
		_activeMenu->sprites[i].pos.x = stream->readUint16() + xpos;
		_activeMenu->sprites[i].pos.y = stream->readUint16() + ypos;
		_activeMenu->retvals[i] = stream->readUint16();

		_activeMenu->sprites[i].drawPriority = 15;
		_activeMenu->sprites[i].drawPriority2 = 8;
	}

	if (_activeMenu->retvals[_activeMenu->numButtons - 1] == 0) {
		// Set default retvals for buttons
		for (int i = 0; i < _activeMenu->numButtons; i++)
			_activeMenu->retvals[i] = i;
	}

	_activeMenu->selectedButton = -1;
	_activeMenu->disabledButtons = 0;
	_keyboardControlsMouse = false;
}

/**
 * Sets which buttons are visible based on the given bitmask.
 */
void StarTrekEngine::setVisibleMenuButtons(uint32 bits) {
	for (int i = 0; i < _activeMenu->numButtons; i++) {
		Sprite *sprite = &_activeMenu->sprites[i];
		uint32 spriteBitmask = (1 << i);
		if (spriteBitmask == 0)
			break;

		if ((bits & spriteBitmask) == 0 || sprite->drawMode != 0) {
			if ((bits & spriteBitmask) == 0 && sprite->drawMode == 2) {
				if (i == _activeMenu->selectedButton) {
					drawMenuButtonOutline(sprite->bitmap, 0x00);
					_activeMenu->selectedButton = -1;
				}

				sprite->field16 = true;
				sprite->bitmapChanged = true;
			}
		} else {
			_gfx->addSprite(sprite);
			sprite->drawMode = 2;
			sprite->bitmapChanged = true;
		}
	}

	_gfx->drawAllSprites();

	for (int i = 0; i < _activeMenu->numButtons; i++) {
		Sprite *sprite = &_activeMenu->sprites[i];
		uint32 spriteBitmask = (1 << i);
		if (spriteBitmask == 0)
			break;

		if ((bits & spriteBitmask) == 0 && sprite->drawMode == 2) {
			_gfx->delSprite(sprite);

			// Setting drawMode to 0 is the game's way of saying that the menu button is
			// hidden (since it would normally be 2).
			sprite->drawMode = 0;
		}
	}
}

/**
 * Disables the given bitmask of buttons.
 */
void StarTrekEngine::disableMenuButtons(uint32 bits) {
	_activeMenu->disabledButtons |= bits;
	if (_activeMenu->selectedButton != -1
			&& (_activeMenu->disabledButtons & (1 << _activeMenu->selectedButton))) {
		Sprite *sprite = &_activeMenu->sprites[_activeMenu->selectedButton];
		drawMenuButtonOutline(sprite->bitmap, 0x00);

		sprite->bitmapChanged = true;
		_activeMenu->selectedButton = -1;
	}
}

void StarTrekEngine::enableMenuButtons(uint32 bits) {
	_activeMenu->disabledButtons &= ~bits;
}

/**
 * This returns either a special menu event (negative number) or the retval of the button
 * clicked (usually an index, always positive).
 */
int StarTrekEngine::handleMenuEvents(uint32 ticksUntilClickingEnabled, bool inTextbox) {
	uint32 tickWhenClickingEnabled = _clockTicks + ticksUntilClickingEnabled;

	while (true) {
		TrekEvent event;
		while (popNextEvent(&event)) {
			switch(event.type) {

			case TREKEVENT_TICK: {
				Common::Point mousePos = _gfx->getMousePos();
				int buttonIndex = getMenuButtonAt(_activeMenu->sprites, _activeMenu->numButtons, mousePos.x, mousePos.y);
				if (buttonIndex != -1) {
					if (_activeMenu->disabledButtons & (1<<buttonIndex))
						buttonIndex = -1;
				}

				if (buttonIndex != _activeMenu->selectedButton) {
					if (_activeMenu->selectedButton != -1) {
						Sprite &spr = _activeMenu->sprites[_activeMenu->selectedButton];
						drawMenuButtonOutline(spr.bitmap, 0x00);
						spr.bitmapChanged = true;
					}
					if (buttonIndex != -1) {
						Sprite &spr = _activeMenu->sprites[buttonIndex];
						drawMenuButtonOutline(spr.bitmap, 0xda);
						spr.bitmapChanged = true;
					}
					_activeMenu->selectedButton = buttonIndex;
				}
				// Not added: updating mouse position (scummvm handles that)

				// sub_10492(); // TODO
				updateActorAnimations();
				_gfx->drawAllSprites();
				// sub_10BE7(); // TODO
				_sound->checkLoopMusic();

				if (_finishedPlayingSpeech != 0) {
					_finishedPlayingSpeech = 0;
					if (_textDisplayMode != TEXTDISPLAY_WAIT) {
						return TEXTBUTTON_SPEECH_DONE;
					}
				}
				_gfx->incPaletteFadeLevel();
				_frameIndex++;

				if (ticksUntilClickingEnabled != 0 && _clockTicks >= tickWhenClickingEnabled)
					return MENUEVENT_ENABLEINPUT;
				break;
			}

			case TREKEVENT_LBUTTONDOWN:
lclick:
				if (_activeMenu->selectedButton != -1) {
					playSoundEffectIndex(SND_SELECTION);
					return _activeMenu->retvals[_activeMenu->selectedButton];
				} else {
					Common::Point mouse = _gfx->getMousePos();
					if (getMenuButtonAt(_activeMenu->sprites, _activeMenu->numButtons, mouse.x, mouse.y) == -1) {
						playSoundEffectIndex(SND_SELECTION);
						return MENUEVENT_LCLICK_OFFBUTTON;
					}
				}
				break;

			case TREKEVENT_RBUTTONDOWN:
rclick:
				playSoundEffectIndex(SND_SELECTION);
				if (_activeMenu->selectedButton == -1)
					return MENUEVENT_RCLICK_OFFBUTTON;
				else
					return MENUEVENT_RCLICK_ONBUTTON;
				break;

			case TREKEVENT_KEYDOWN:
				if (inTextbox) {
					switch (event.kbd.keycode) {
					case Common::KEYCODE_ESCAPE:
					case Common::KEYCODE_F2:
						goto rclick;

					case Common::KEYCODE_RETURN:
					case Common::KEYCODE_KP_ENTER:
					case Common::KEYCODE_F1:
						playSoundEffectIndex(SND_SELECTION);
						return TEXTBUTTON_CONFIRM;

					case Common::KEYCODE_SPACE:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_NEXTCHOICE))
								&& _activeMenu->sprites[TEXTBUTTON_NEXTCHOICE].drawMode == 2) {
							playSoundEffectIndex(SND_SELECTION);
							return TEXTBUTTON_NEXTCHOICE;
						}
						break;

					case Common::KEYCODE_HOME:
					case Common::KEYCODE_KP7:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_SCROLLUP))
								&& _activeMenu->sprites[TEXTBUTTON_SCROLLUP].drawMode == 2) {
							playSoundEffectIndex(SND_SELECTION);
							return TEXTBUTTON_GOTO_TOP;
						}
						break;

					case Common::KEYCODE_UP:
					case Common::KEYCODE_KP8:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_SCROLLUP))
								&& _activeMenu->sprites[TEXTBUTTON_SCROLLUP].drawMode == 2) {
							playSoundEffectIndex(SND_SELECTION);
							return TEXTBUTTON_SCROLLUP_ONELINE;
						}
						break;

					case Common::KEYCODE_PAGEUP:
					case Common::KEYCODE_KP9:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_SCROLLUP))
								&& _activeMenu->sprites[TEXTBUTTON_SCROLLUP].drawMode == 2) {
							playSoundEffectIndex(SND_SELECTION);
							return TEXTBUTTON_SCROLLUP;
						}
						break;

					case Common::KEYCODE_LEFT:
					case Common::KEYCODE_KP4:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_PREVCHOICE))
								&& _activeMenu->sprites[TEXTBUTTON_PREVCHOICE].drawMode == 2) {
							playSoundEffectIndex(SND_SELECTION);
							return TEXTBUTTON_PREVCHOICE;
						}
						break;

					case Common::KEYCODE_RIGHT:
					case Common::KEYCODE_KP6:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_NEXTCHOICE))
								&& _activeMenu->sprites[TEXTBUTTON_NEXTCHOICE].drawMode == 2) {
							playSoundEffectIndex(SND_SELECTION);
							return TEXTBUTTON_NEXTCHOICE;
						}
						break;

					case Common::KEYCODE_END:
					case Common::KEYCODE_KP1:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_SCROLLDOWN))
								&& _activeMenu->sprites[TEXTBUTTON_SCROLLDOWN].drawMode == 2) {
							playSoundEffectIndex(SND_SELECTION);
							return TEXTBUTTON_GOTO_BOTTOM;
						}
						break;

					case Common::KEYCODE_DOWN:
					case Common::KEYCODE_KP2:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_SCROLLDOWN))
								&& _activeMenu->sprites[TEXTBUTTON_SCROLLDOWN].drawMode == 2) {
							playSoundEffectIndex(SND_SELECTION);
							return TEXTBUTTON_SCROLLDOWN_ONELINE;
						}
						break;

					case Common::KEYCODE_PAGEDOWN:
					case Common::KEYCODE_KP3:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_SCROLLDOWN))
								&& _activeMenu->sprites[TEXTBUTTON_SCROLLDOWN].drawMode == 2) {
							playSoundEffectIndex(SND_SELECTION);
							return TEXTBUTTON_SCROLLDOWN;
						}
						break;

					default:
						break;
					}
				} else { // Not in textbox
					switch (event.kbd.keycode) {
					case Common::KEYCODE_ESCAPE:
					case Common::KEYCODE_F2:
						goto rclick;

					case Common::KEYCODE_RETURN:
					case Common::KEYCODE_KP_ENTER:
					case Common::KEYCODE_F1:
						goto lclick;

					case Common::KEYCODE_HOME:
					case Common::KEYCODE_KP7:
						chooseMousePositionFromSprites(_activeMenu->sprites, _activeMenu->numButtons, _activeMenu->selectedButton, 4);
						break;

					case Common::KEYCODE_UP:
					case Common::KEYCODE_KP8:
					case Common::KEYCODE_PAGEUP:
					case Common::KEYCODE_KP9:
						chooseMousePositionFromSprites(_activeMenu->sprites, _activeMenu->numButtons, _activeMenu->selectedButton, 2);
						break;

					case Common::KEYCODE_LEFT:
					case Common::KEYCODE_KP4:
						chooseMousePositionFromSprites(_activeMenu->sprites, _activeMenu->numButtons, _activeMenu->selectedButton, 1);
						break;

					case Common::KEYCODE_RIGHT:
					case Common::KEYCODE_KP6:
						chooseMousePositionFromSprites(_activeMenu->sprites, _activeMenu->numButtons, _activeMenu->selectedButton, 0);
						break;

					case Common::KEYCODE_END:
					case Common::KEYCODE_KP1:
						chooseMousePositionFromSprites(_activeMenu->sprites, _activeMenu->numButtons, _activeMenu->selectedButton, 5);
						break;

					case Common::KEYCODE_DOWN:
					case Common::KEYCODE_KP2:
					case Common::KEYCODE_PAGEDOWN:
					case Common::KEYCODE_KP3:
						chooseMousePositionFromSprites(_activeMenu->sprites, _activeMenu->numButtons, _activeMenu->selectedButton, 3);
						break;

					default:
						break;
					}
					break;

				default:
					break;
				}
			}
		}
	}
}

void StarTrekEngine::unloadMenuButtons() {
	if (_activeMenu->selectedButton != -1)
		drawMenuButtonOutline(_activeMenu->sprites[_activeMenu->selectedButton].bitmap, 0x00);

	for (int i = 0; i < _activeMenu->numButtons; i++) {
		Sprite *sprite = &_activeMenu->sprites[i];
		if (sprite->drawMode == 2) {
			sprite->field16 = true;
			sprite->bitmapChanged = true;
		}
	}

	_gfx->drawAllSprites();

	for (int i = 0; i < _activeMenu->numButtons; i++) {
		Sprite *sprite = &_activeMenu->sprites[i];
		sprite->bitmap.reset();
		if (sprite->drawMode == 2)
			_gfx->delSprite(sprite);
	}

	_activeMenu = _activeMenu->nextMenu;

	if (_activeMenu == nullptr)
		_keyboardControlsMouse = _keyboardControlsMouseOutsideMenu;
}

/**
 * Sets the mouse bitmap based on which action is selected.
 */
void StarTrekEngine::chooseMouseBitmapForAction(int action, bool withRedOutline) {
	const char *lookActionBitmaps[] = {
		"lookh0", // The "look" action randomly animates with these images
		"lookh0",
		"lookh0",
		"lookh0",
		"lookh0",
		"lookh1",
		"lookh2",
		"lookh3"
	};

	Common::String bitmapName;

	switch (action) {

	case ACTION_USE:
		if (withRedOutline)
			bitmapName = "useh";
		else
			bitmapName = "usen";
		break;

	case ACTION_GET:
		if (withRedOutline)
			bitmapName = "geth";
		else
			bitmapName = "getn";
		break;

	case ACTION_LOOK:
		if (withRedOutline) {
			if ((getRandomWord() & 7) == 0)
				_lookActionBitmapIndex = getRandomWord() & 7; // Choose an image randomly
			bitmapName = lookActionBitmaps[_lookActionBitmapIndex];
		} else
			bitmapName = "lookn";
		break;

	case ACTION_TALK:
		if (withRedOutline) {
			if (getRandomWord() & 3)
				bitmapName = "talkh0";
			else
				bitmapName = "talkh1";
		} else
			bitmapName = "talkn";
		break;

	case ACTION_OPTIONS:
		bitmapName = "options";
		break;

	case ACTION_WALK:
	default:
		bitmapName = "walk";
		break;
	}

	_gfx->setMouseBitmap(_gfx->loadBitmap(bitmapName));
}

void StarTrekEngine::showQuitGamePrompt(int x, int y) {
	const char *options[] = {
		"Quit Game",
		"#GENE\\GENER028#Yes, quit the game.",
		"#GENE\\GENER008#No, do not quit the game.",
		""
	};

	if (_inQuitGameMenu)
		return;

	_inQuitGameMenu = true;
	int val = showText(&StarTrekEngine::readTextFromArray, (uintptr)options, x, y, TEXTCOLOR_YELLOW, true, 0, true);
	_inQuitGameMenu = false;

	if (val == 0) {
		// sub_1e70d();
		_system->quit();
	}
}

void StarTrekEngine::showGameOverMenu() {
	const char *options[] = {
		"Game Over",
		"#GENE\\GENER006#Load a previously saved game.",
		"#GENE\\GENER020#Restart the game.",
		"#GENE\\GENER018#Quit the game.",
		""
	};

	while (true) {
		_inQuitGameMenu = true;
		int selected = showText(&StarTrekEngine::readTextFromArray, (uintptr)options, 20, 20, TEXTCOLOR_YELLOW, true, false, true);
		_inQuitGameMenu = false;

		switch (selected) {
		case 0: // Load game
			_gfx->fadeoutScreen();
			showLoadMenu(); // TODO: this probably manipulates the stack to jump out of this function...
			break;
		case 1: // Restart
			_gfx->fadeoutScreen();
			// TODO
			break;
		case 2: // Quit
			_gfx->fadeoutScreen();
			_system->quit();
			break;
		default:
			break;
		}
	}
}

/**
 * This can be called from startup or from the options menu.
 * On startup, this tries to load the setting without user input.
 */
void StarTrekEngine::showTextConfigurationMenu(bool fromOptionMenu) {
	const char *options[] = { // TODO: languages...
		"Text display",
		"Text subtitles.",
		"Display text until you press enter.",
		"No text displayed.",
		""
	};

	int val;
	if (fromOptionMenu || (val = loadTextDisplayMode()) == -1) {
		val = showText(&StarTrekEngine::readTextFromArray, (uintptr)options, 20, 30, TEXTCOLOR_YELLOW, true, 0, true);
		saveTextDisplayMode(val);
	}

	switch(val) {
	case 0:
		_textDisplayMode = TEXTDISPLAY_SUBTITLES;
		break;
	case 1:
		_textDisplayMode = TEXTDISPLAY_WAIT;
		break;
	case 2:
		_textDisplayMode = TEXTDISPLAY_NONE;
		break;
	}
}

int StarTrekEngine::loadTextDisplayMode() {
	return -1; // TODO
}
void StarTrekEngine::saveTextDisplayMode(int value) {
	// TODO;
}

}
