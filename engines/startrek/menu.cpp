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
#include "startrek/resource.h"

namespace StarTrek {

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
		switch (mode) {
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

		default:
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

void StarTrekEngine::drawMenuButtonOutline(Bitmap *bitmap, byte color) {
	int lineWidth = bitmap->width - 2;
	int offsetToBottom = (bitmap->height - 3) * bitmap->width;

	byte *dest = bitmap->pixels + bitmap->width + 1;

	while (lineWidth--) {
		*dest = color;
		*(dest + offsetToBottom) = color;
		dest++;
	}

	int lineHeight = bitmap->height - 2;
	int offsetToRight = bitmap->width - 3;

	dest = bitmap->pixels + bitmap->width + 1;

	while (lineHeight--) {
		*dest = color;
		*(dest + offsetToRight) = color;
		dest += bitmap->width;
	}
}

void StarTrekEngine::showOptionsMenu(int x, int y) {
	bool tmpMouseControllingShip = _mouseControllingShip;
	_mouseControllingShip = false;

	Common::Point oldMousePos = _gfx->getMousePos();
	_gfx->setMouseBitmap("options");
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
	_gfx->popMouseBitmap();

	if (event != MENUEVENT_LCLICK_OFFBUTTON && event != MENUEVENT_RCLICK_OFFBUTTON)
		_gfx->warpMouse(oldMousePos.x, oldMousePos.y);


	// Can't use OPTIONBUTTON constants since the button retvals differ from the button
	// indices...
	switch (event) {
	case 0: // Save
		showSaveMenu();
		break;
	case 1: // Load
		showLoadMenu();
		_resetGameMode = true;
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

void StarTrekEngine::showBridgeMenu(Common::String menu, int x, int y) {
	bool tmpMouseControllingShip = _mouseControllingShip;
	_mouseControllingShip = false;

	Common::Point oldMousePos = _gfx->getMousePos();
	loadMenuButtons(menu, x, y);

	chooseMousePositionFromSprites(_activeMenu->sprites, _activeMenu->numButtons, -1, 4);
	int event = handleMenuEvents(0, false);

	unloadMenuButtons();
	_mouseControllingShip = tmpMouseControllingShip;

	if (event != MENUEVENT_LCLICK_OFFBUTTON && event != MENUEVENT_RCLICK_OFFBUTTON)
		_gfx->warpMouse(oldMousePos.x, oldMousePos.y);

	handleBridgeMenu(event);
}

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

	menuSprite.setBitmap(_resource->loadBitmapFile("action"));
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
			displayMenu = false;
			addEventBack = true;
			break;

		case TREKEVENT_MOUSEMOVE:
			action = mouseMoveEvent();
			break;

		case TREKEVENT_RBUTTONDOWN:
			displayMenu = false;
			action = ACTION_WALK;
			break;

		case TREKEVENT_KEYDOWN: {
			int nextAction = action;

			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_SPACE:
			case Common::KEYCODE_F2: // Exit menu without selecting anything
				displayMenu = false;
				action = ACTION_WALK;
				break;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
			case Common::KEYCODE_F1: // Exit menu with whatever is selected
				displayMenu = false;
				addEventBack = true;
				break;

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
				nextAction = lookupNextAction(actionMappingUp, action);
				break;

			case Common::KEYCODE_RIGHT:
			case Common::KEYCODE_KP6:
				nextAction = lookupNextAction(actionMappingRight, action);
				break;

			case Common::KEYCODE_DOWN:
			case Common::KEYCODE_KP2:
				nextAction = lookupNextAction(actionMappingDown, action);
				break;

			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_KP4:
				nextAction = lookupNextAction(actionMappingLeft, action);
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

			action = mouseMoveEvent();
		}

		default:
			break;
		}
	}

	_sound->playSoundEffectIndex(kSfxSelection);

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

int StarTrekEngine::lookupNextAction(const int *lookupArray, int action) {
	// Use a lookup table to decide which action is next after a direction
	// button is pressed.
	assert((action >= ACTION_WALK && action <= ACTION_TALK) || action == ACTION_OPTIONS);
	return lookupArray[action == ACTION_OPTIONS ? 5 : action - 1];
}

void StarTrekEngine::loadMenuButtons(String mnuFilename, int xpos, int ypos) {
	if (_activeMenu == nullptr)
		_keyboardControlsMouseOutsideMenu = _keyboardControlsMouse;

	Menu *oldMenu = _activeMenu;
	_activeMenu = new Menu();
	_activeMenu->nextMenu = oldMenu;

	Common::MemoryReadStreamEndian *stream = _resource->loadFile(mnuFilename + ".MNU");

	_activeMenu->numButtons = stream->size() / 16;

	for (int i = 0; i < _activeMenu->numButtons; i++) {
		_activeMenu->sprites[i] = Sprite();
		_gfx->addSprite(&_activeMenu->sprites[i]);
		_activeMenu->sprites[i].drawMode = 2;

		char bitmapBasename[11];
		stream->seek(i * 16, SEEK_SET);
		stream->read(bitmapBasename, 10);
		bitmapBasename[10] = '\0';
		Common::String bitmapName = bitmapBasename;
		bitmapName.trim();

		_activeMenu->sprites[i].setBitmap(_resource->loadBitmapFile(bitmapName));
		_activeMenu->sprites[i].pos.x = stream->readUint16() + xpos;
		_activeMenu->sprites[i].pos.y = stream->readUint16() + ypos;
		_activeMenu->retvals[i] = stream->readUint16();

		_activeMenu->sprites[i].drawPriority = 15;
		_activeMenu->sprites[i].drawPriority2 = 8;
	}

	delete stream;

	if (_activeMenu->retvals[_activeMenu->numButtons - 1] == 0) {
		// Set default retvals for buttons
		for (int i = 0; i < _activeMenu->numButtons; i++)
			_activeMenu->retvals[i] = i;
	}

	_activeMenu->selectedButton = -1;
	_activeMenu->disabledButtons = 0;
	_keyboardControlsMouse = false;
}

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

int StarTrekEngine::leftClickEvent() {
	if (_activeMenu->selectedButton != -1) {
		_sound->playSoundEffectIndex(kSfxSelection);
		return _activeMenu->retvals[_activeMenu->selectedButton];
	} else {
		Common::Point mouse = _gfx->getMousePos();
		if (getMenuButtonAt(_activeMenu->sprites, _activeMenu->numButtons, mouse.x, mouse.y) == -1) {
			_sound->playSoundEffectIndex(kSfxSelection);
			return MENUEVENT_LCLICK_OFFBUTTON;
		}
	}

	return MENUEVENT_LCLICK_OFFBUTTON;
}

int StarTrekEngine::rightClickEvent() {
	_sound->playSoundEffectIndex(kSfxSelection);
	if (_activeMenu->selectedButton == -1)
		return MENUEVENT_RCLICK_OFFBUTTON;
	else
		return MENUEVENT_RCLICK_ONBUTTON;
}

int StarTrekEngine::mouseMoveEvent() {
	const Common::Point pos(50, 50); // Top-left position to put action menu at
	int action = ACTION_WALK;

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

	_gfx->setMouseBitmap(bitmapName);

	if (lockMousePoint.x != -1)
		_gfx->lockMousePosition(lockMousePoint.x, lockMousePoint.y);
	else
		_gfx->unlockMousePosition();

	return action;
}

int StarTrekEngine::handleMenuEvents(uint32 ticksUntilClickingEnabled, bool inTextbox) {
	uint32 tickWhenClickingEnabled = _clockTicks + ticksUntilClickingEnabled;

	while (true) {
		TrekEvent event;
		while (popNextEvent(&event)) {
			switch (event.type) {

			case TREKEVENT_TICK: {
				Common::Point mousePos = _gfx->getMousePos();
				int buttonIndex = getMenuButtonAt(_activeMenu->sprites, _activeMenu->numButtons, mousePos.x, mousePos.y);
				if (buttonIndex != -1) {
					if (_activeMenu->disabledButtons & (1 << buttonIndex))
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

				updateActorAnimations();
				renderBanBelowSprites();
				_gfx->drawAllSprites(false);
				renderBanAboveSprites();
				_gfx->updateScreen();

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
				return leftClickEvent();

			case TREKEVENT_RBUTTONDOWN:
				return rightClickEvent();

			case TREKEVENT_KEYDOWN:
				if (inTextbox) {
					switch (event.kbd.keycode) {
					case Common::KEYCODE_ESCAPE:
					case Common::KEYCODE_F2:
						return rightClickEvent();

					case Common::KEYCODE_RETURN:
					case Common::KEYCODE_KP_ENTER:
					case Common::KEYCODE_F1:
						_sound->playSoundEffectIndex(kSfxSelection);
						return TEXTBUTTON_CONFIRM;

					case Common::KEYCODE_SPACE:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_NEXTCHOICE))
						        && _activeMenu->sprites[TEXTBUTTON_NEXTCHOICE].drawMode == 2) {
							_sound->playSoundEffectIndex(kSfxSelection);
							return TEXTBUTTON_NEXTCHOICE;
						}
						break;

					case Common::KEYCODE_HOME:
					case Common::KEYCODE_KP7:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_SCROLLUP))
						        && _activeMenu->sprites[TEXTBUTTON_SCROLLUP].drawMode == 2) {
							_sound->playSoundEffectIndex(kSfxSelection);
							return TEXTBUTTON_GOTO_TOP;
						}
						break;

					case Common::KEYCODE_UP:
					case Common::KEYCODE_KP8:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_SCROLLUP))
						        && _activeMenu->sprites[TEXTBUTTON_SCROLLUP].drawMode == 2) {
							_sound->playSoundEffectIndex(kSfxSelection);
							return TEXTBUTTON_SCROLLUP_ONELINE;
						}
						break;

					case Common::KEYCODE_PAGEUP:
					case Common::KEYCODE_KP9:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_SCROLLUP))
						        && _activeMenu->sprites[TEXTBUTTON_SCROLLUP].drawMode == 2) {
							_sound->playSoundEffectIndex(kSfxSelection);
							return TEXTBUTTON_SCROLLUP;
						}
						break;

					case Common::KEYCODE_LEFT:
					case Common::KEYCODE_KP4:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_PREVCHOICE))
						        && _activeMenu->sprites[TEXTBUTTON_PREVCHOICE].drawMode == 2) {
							_sound->playSoundEffectIndex(kSfxSelection);
							return TEXTBUTTON_PREVCHOICE;
						}
						break;

					case Common::KEYCODE_RIGHT:
					case Common::KEYCODE_KP6:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_NEXTCHOICE))
						        && _activeMenu->sprites[TEXTBUTTON_NEXTCHOICE].drawMode == 2) {
							_sound->playSoundEffectIndex(kSfxSelection);
							return TEXTBUTTON_NEXTCHOICE;
						}
						break;

					case Common::KEYCODE_END:
					case Common::KEYCODE_KP1:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_SCROLLDOWN))
						        && _activeMenu->sprites[TEXTBUTTON_SCROLLDOWN].drawMode == 2) {
							_sound->playSoundEffectIndex(kSfxSelection);
							return TEXTBUTTON_GOTO_BOTTOM;
						}
						break;

					case Common::KEYCODE_DOWN:
					case Common::KEYCODE_KP2:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_SCROLLDOWN))
						        && _activeMenu->sprites[TEXTBUTTON_SCROLLDOWN].drawMode == 2) {
							_sound->playSoundEffectIndex(kSfxSelection);
							return TEXTBUTTON_SCROLLDOWN_ONELINE;
						}
						break;

					case Common::KEYCODE_PAGEDOWN:
					case Common::KEYCODE_KP3:
						if (!(_activeMenu->disabledButtons & (1 << TEXTBUTTON_SCROLLDOWN))
						        && _activeMenu->sprites[TEXTBUTTON_SCROLLDOWN].drawMode == 2) {
							_sound->playSoundEffectIndex(kSfxSelection);
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
						return rightClickEvent();

					case Common::KEYCODE_RETURN:
					case Common::KEYCODE_KP_ENTER:
					case Common::KEYCODE_F1:
						return leftClickEvent();

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
		delete sprite->bitmap;
		sprite->bitmap = nullptr;
		if (sprite->drawMode == 2)
			_gfx->delSprite(sprite);
	}

	Menu *prevMenu = _activeMenu;
	_activeMenu = _activeMenu->nextMenu;
	delete prevMenu;

	if (_activeMenu == nullptr)
		_keyboardControlsMouse = _keyboardControlsMouseOutsideMenu;
}

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

	if (getFeatures() & GF_DEMO)
		return;

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

	_gfx->setMouseBitmap(bitmapName);
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
			_resetGameMode = true;
			return;
		case 1: // Restart
			_gfx->fadeoutScreen();
			// TODO
			_resetGameMode = true;
			return;
		case 2: // Quit
			_gfx->fadeoutScreen();
			_system->quit();
			return;
		default:
			break;
		}
	}
}

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

	switch (val) {
	case 0:
		_textDisplayMode = TEXTDISPLAY_SUBTITLES;
		break;
	case 1:
		_textDisplayMode = TEXTDISPLAY_WAIT;
		break;
	case 2:
		_textDisplayMode = TEXTDISPLAY_NONE;
		break;
	default:
		break;
	}
}

int StarTrekEngine::loadTextDisplayMode() {
	return -1; // TODO
}
void StarTrekEngine::saveTextDisplayMode(int value) {
	// TODO;
}

void StarTrekEngine::showRepublicMap(int16 arg0, int16 turbolift) {
	_gfx->fadeoutScreen();
	_sound->stopAllVocSounds();

	bool spriteLoaded = false;
	int16 clickedArea = 0;

	removeDrawnActorsFromScreen();
	_gfx->pushSprites();

	if (!_awayMission.veng.showedRepublicMapFirstTime) {
		_gfx->setBackgroundImage("veng9b");
		_gfx->copyBackgroundScreen();
		_system->updateScreen();
		_system->delayMillis(10);
		_gfx->setPri(15);
		_gfx->fadeinScreen();

		// TODO: hide mouse sprite?

		bool exitLoop = 0;
		int16 var54 = 0x2d;

		while (!exitLoop) {
			TrekEvent event;
			if (!popNextEvent(&event))
				continue;

			switch (event.type) {
			case TREKEVENT_TICK:
				if (--var54 == 0)
					exitLoop = true;
				break;

			case TREKEVENT_LBUTTONDOWN:
			case TREKEVENT_RBUTTONDOWN:
				exitLoop = true;
				break;

			case TREKEVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
				case Common::KEYCODE_RETURN:
				case Common::KEYCODE_KP_ENTER:
				case Common::KEYCODE_SPACE:
					exitLoop = true;
					break;

				default:
					break;
				}
				break;

			default:
				break;
			}
		}

		// BUGFIX: Original game used variable "scannedComputerBank" (0x32) instead of
		// "showedRepublicMapFirstTime" (0x33), which is used elsewhere. Byte 0x33 is
		// otherwise unused, so maybe this is a weird off-by-1 error.
		// The effective result is that scanning the computer bank would cause the preview
		// of the map screen to not appear.
		_awayMission.veng.showedRepublicMapFirstTime = true;

		_gfx->fadeoutScreen();
	}

	_gfx->setBackgroundImage("veng9");
	_gfx->copyBackgroundScreen();
	_system->updateScreen();
	_system->delayMillis(10);
	_gfx->setPri(15);

	Sprite someSprite;
	_gfx->drawAllSprites();
	_gfx->warpMouse(_gfx->getMousePos().x, 96);
	_gfx->fadeinScreen();

	bool exitLoop = false;

	while (!exitLoop) {
		TrekEvent event;
		if (!popNextEvent(&event))
			continue;

		switch (event.type) {
		case TREKEVENT_TICK:
			_frameIndex++;
			// sub_12fff(); // TODO
			_gfx->drawAllSprites();
			break;

		case TREKEVENT_LBUTTONDOWN:
			clickedArea = getRepublicMapAreaOrFailure(turbolift);
			if (clickedArea == 0) {
			} else if (clickedArea == 6) {
				Common::String text = "#GENE\\GENE_F14#Turbolift access is blocked by an extremely high radiation level.";
				showTextbox("Mr. Spock", text, 50, 50, TEXTCOLOR_YELLOW, 0); // ENHANCEMENT: Speaker is Spock
			} else if (clickedArea == 7) {
				Common::String text = "#GENE\\GENE_F15#This turbolift cannot reach that area of the ship.";
				showTextbox("Mr. Spock", text, 50, 50, TEXTCOLOR_YELLOW, 0); // ENHANCEMENT: Speaker is Spock
			} else
				exitLoop = true;
			break;

		case TREKEVENT_MOUSEMOVE: {
			if (_gfx->getMousePos().y < 96) // TODO: more elegant solution
				_gfx->warpMouse(_gfx->getMousePos().x, 96);

			clickedArea = getRepublicMapAreaAtMouse();
			if (clickedArea != 0) {
				if (!spriteLoaded) {
					_gfx->addSprite(&someSprite);
					someSprite.setXYAndPriority(3, 168, 15);
					someSprite.setBitmap(_resource->loadBitmapFile(Common::String::format("turbo%d", clickedArea)));
					spriteLoaded = true;
				}
			} else {
				if (spriteLoaded) {
					someSprite.dontDrawNextFrame();
					_gfx->drawAllSprites();
					_gfx->delSprite(&someSprite);
					delete someSprite.bitmap;
					someSprite.bitmap = nullptr;
					spriteLoaded = false;
				}
			}
			break;
		}

		case TREKEVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
			case Common::KEYCODE_F1:
				// Same as TREKEVENT_LBUTTONDOWN
				clickedArea = getRepublicMapAreaOrFailure(turbolift);
				if (clickedArea == 0) {
				} else if (clickedArea == 6) {
					Common::String text = "#GENE\\GENE_F14#Turbolift access is blocked by an extremely high radiation level.";
					showTextbox("Mr. Spock", text, 50, 50, TEXTCOLOR_YELLOW, 0); // ENHANCEMENT: Speaker is Spock
				} else if (clickedArea == 7) {
					Common::String text = "#GENE\\GENE_F15#This turbolift cannot reach that area of the ship.";
					showTextbox("Mr. Spock", text, 50, 50, TEXTCOLOR_YELLOW, 0); // ENHANCEMENT: Speaker is Spock
				} else
					exitLoop = true;
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
	}

	_gfx->fadeoutScreen();
	delete someSprite.bitmap;
	someSprite.bitmap = nullptr;
	_gfx->popSprites();

	_gfx->loadPri(getScreenName());
	_gfx->setBackgroundImage(getScreenName());
	_gfx->copyBackgroundScreen();
	_system->updateScreen();
	_system->delayMillis(10);

	_gfx->drawAllSprites();

	int16 roomIndex, spawnIndex;
	if (clickedArea == 1) {
		roomIndex = 0;
		spawnIndex = 1;
	} else if (clickedArea == 2) {
		roomIndex = 1;
		spawnIndex = 1;
	} else if (clickedArea == 3 && turbolift == 0) {
		roomIndex = 3;
		spawnIndex = 1;
	} else if (clickedArea == 3 && turbolift == 1) {
		roomIndex = 3;
		spawnIndex = 0;
	} else if (clickedArea == 4) {
		roomIndex = 5;
		spawnIndex = 1;
	} else if (clickedArea == 5) {
		roomIndex = 7;
		spawnIndex = 1;
	} else {
		warning("Unknown room selected");
		roomIndex = 0;
		spawnIndex = 1;
	}

	_roomIndexToLoad = roomIndex;
	_spawnIndexToLoad = spawnIndex;
}

int StarTrekEngine::getRepublicMapAreaAtMouse() {
	Common::Point mouse = _gfx->getMousePos();

	if (mouse.x >= 0x7f && mouse.x <= 0x91 && mouse.y >= 0x78 && mouse.y <= 0x7b)
		return 1;
	else if (mouse.x >= 0x6e && mouse.x <= 0x7e && mouse.y >= 0x83 && mouse.y <= 0x87)
		return 2;
	else if (mouse.x >= 0x95 && mouse.x <= 0xad && mouse.y >= 0x8f && mouse.y <= 0x93)
		return 3;
	else if (mouse.x >= 0xef && mouse.x <= 0xfd && mouse.y >= 0x98 && mouse.y <= 0xa0)
		return 4;
	else if (mouse.x >= 0x6b && mouse.x <= 0x80 && mouse.y >= 0xa3 && mouse.y <= 0xa7)
		return 5;
	else if (mouse.x >= 0x6e && mouse.x <= 0x88 && mouse.y >= 0xab && mouse.y <= 0xaf)
		return 6;
	else
		return 0;
}

int StarTrekEngine::getRepublicMapAreaOrFailure(int16 turbolift) {
	Common::Point mouse = _gfx->getMousePos();

	if (mouse.x >= 0x7f && mouse.x <= 0x91 && mouse.y >= 0x78 && mouse.y <= 0x7b)
		return turbolift == 0 ? 1 : 7;
	else if (mouse.x >= 0x6e && mouse.x <= 0x7e && mouse.y >= 0x83 && mouse.y <= 0x87)
		return turbolift == 0 ? 2 : 7;
	else if (mouse.x >= 0x95 && mouse.x <= 0xad && mouse.y >= 0x8f && mouse.y <= 0x93)
		return 3;
	else if (mouse.x >= 0xef && mouse.x <= 0xfd && mouse.y >= 0x98 && mouse.y <= 0xa0)
		return turbolift == 1 ? 4 : 7;
	else if (mouse.x >= 0x6b && mouse.x <= 0x80 && mouse.y >= 0xa3 && mouse.y <= 0xa7)
		return turbolift == 1 ? 5 : 7;
	else if (mouse.x >= 0x6e && mouse.x <= 0x88 && mouse.y >= 0xab && mouse.y <= 0xaf)
		return 6;
	return 0;
}

} // End of namespace StarTrek
