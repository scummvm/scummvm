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
int StarTrekEngine::getMenuButtonAt(const Menu &menu, int x, int y) {
	for (int i = 0; i < menu.numButtons; i++) {
		const Sprite &spr = menu.sprites[i];

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
				debug("Try %d %d", mouseX1, mouseY1);
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
	}
	else if (mouseX2 != 0x7fff) {
		mousePos.x = mouseX2;
		mousePos.y = mouseY2;
	}

	_system->warpMouse(mousePos.x, mousePos.y);

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
	SharedPtr<Bitmap> oldMouseBitmap = _gfx->_mouseBitmap;

	_gfx->setMouseCursor(_gfx->loadBitmap("options"));
	loadMenuButtons("options", x, y);

	uint32 disabledButtons = 0;
	if (_musicWorking) {
		if (_musicEnabled)
			disabledButtons |= (1 << OPTIONBUTTON_ENABLEMUSIC);
		else
			disabledButtons |= (1 << OPTIONBUTTON_DISABLEMUSIC);
	}
	else
		disabledButtons |= (1 << OPTIONBUTTON_ENABLEMUSIC) | (1 << OPTIONBUTTON_DISABLEMUSIC);

	if (_sfxWorking) {
		if (_sfxEnabled)
			disabledButtons |= (1 << OPTIONBUTTON_ENABLESFX);
		else
			disabledButtons |= (1 << OPTIONBUTTON_DISABLESFX);
	}
	else
		disabledButtons |= (1 << OPTIONBUTTON_ENABLESFX) | (1 << OPTIONBUTTON_DISABLESFX);

	disableMenuButtons(disabledButtons);
	chooseMousePositionFromSprites(_activeMenu->sprites, _activeMenu->numButtons, -1, 4);
	int event = handleMenuEvents(0, false);

	unloadMenuButtons();
	_mouseControllingShip = tmpMouseControllingShip;
	_gfx->setMouseCursor(oldMouseBitmap);

	if (event != MENUEVENT_LCLICK_OFFBUTTON && event != MENUEVENT_RCLICK_OFFBUTTON)
		_system->warpMouse(oldMousePos.x, oldMousePos.y);


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
		memset(&_activeMenu->sprites[i], 0, sizeof(Sprite));
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

		_activeMenu->sprites[i].field6 = 8;
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
		}
		else {
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
int StarTrekEngine::handleMenuEvents(uint32 ticksUntilClickingEnabled, bool arg4) {
	// TODO: finish

	uint32 tickWhenClickingEnabled = _clockTicks + ticksUntilClickingEnabled;

	while (true) {
		TrekEvent event;
		while (popNextEvent(&event)) {
			switch(event.type) {

			case TREKEVENT_TICK: {
			case TREKEVENT_MOUSEMOVE: // FIXME: actual game only uses TICK event here
				Common::Point mousePos = _gfx->getMousePos();
				int buttonIndex = getMenuButtonAt(*_activeMenu, mousePos.x, mousePos.y);
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

				// sub_10492();
				// sub_10A91();
				_gfx->drawAllSprites();
				// sub_10BE7();
				// sub_2A4B1();

				if (_finishedPlayingSpeech != 0) {
					_finishedPlayingSpeech = 0;
					if (_textDisplayMode != TEXTDISPLAY_WAIT) {
						return TEXTBUTTON_SPEECH_DONE;
					}
				}
				// sub_1E88C();
				_frameIndex++;

				if (ticksUntilClickingEnabled != 0 && _clockTicks >= tickWhenClickingEnabled)
					return MENUEVENT_ENABLEINPUT;
				break;
			}

			case TREKEVENT_LBUTTONDOWN:
				if (_activeMenu->selectedButton != -1) {
					playSoundEffectIndex(0x10);
					return _activeMenu->retvals[_activeMenu->selectedButton];
				}
				else {
					Common::Point mouse = _gfx->getMousePos();
					if (getMenuButtonAt(*_activeMenu, mouse.x, mouse.y) == -1) {
						playSoundEffectIndex(0x10);
						return MENUEVENT_LCLICK_OFFBUTTON;
					}
				}
				break;

			case TREKEVENT_RBUTTONDOWN:
				// TODO
				break;

			case TREKEVENT_KEYDOWN:
				// TODO
				break;

			default:
				break;
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

void StarTrekEngine::showSaveMenu() {
	// TODO
}

void StarTrekEngine::showLoadMenu() {
	// TODO
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
	int val = showText(&StarTrekEngine::readTextFromArray, (uintptr)options, x, y, 0xb0, true, 0, 1);
	_inQuitGameMenu = false;

	if (val == 0) {
		// sub_1e70d();
		_system->quit();
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
		val = showText(&StarTrekEngine::readTextFromArray, (uintptr)options, 20, 30, 0xb0, true, 0, 1);
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
