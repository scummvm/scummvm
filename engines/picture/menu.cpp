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
 * $URL: https://www.switchlink.se/svn/picture/resource.cpp $
 * $Id: resource.cpp 2 2008-08-04 12:18:15Z johndoe $
 *
 */

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/mixer.h"

#include "picture/picture.h"
#include "picture/menu.h"
#include "picture/resource.h"
#include "picture/screen.h"

namespace Picture {

MenuSystem::MenuSystem(PictureEngine *vm) : _vm(vm) {
	_currMenuID = kMenuIdNone;
	_newMenuID = kMenuIdMain;
	_currItemID = kItemIdNone;
	_cfgText = true;
	_cfgVoices = true;
	_cfgMasterVolume = 10;
	_cfgVoicesVolume = 10;
	_cfgMusicVolume = 10;
	_cfgSoundFXVolume = 10;
	_cfgBackgroundVolume = 10;
}

MenuSystem::~MenuSystem() {
}

int MenuSystem::run() {

	_background = new Graphics::Surface();
	_background->create(640, 400, 1);

	memcpy(_background->pixels, _vm->_screen->_frontScreen, 640 * 400);
	
	while (1) {
		update();
		_vm->updateScreen();
	}
	
	delete _background;
	
}

void MenuSystem::update() {

	if (_currMenuID != _newMenuID) {
		_currMenuID = _newMenuID;
		debug("_currMenuID = %d", _currMenuID);
		initMenu(_currMenuID);
	}

	handleEvents();

	_vm->_system->copyRectToScreen((const byte *)_vm->_screen->_frontScreen,
		640, 0, 0, 640, 400);

	_vm->_system->delayMillis(5);

}

void MenuSystem::handleEvents() {

	Common::Event event;
	Common::EventManager *eventMan = _vm->_system->getEventManager();
	while (eventMan->pollEvent(event)) {
	switch (event.type) {
		case Common::EVENT_KEYDOWN:
			break;
		case Common::EVENT_QUIT:
			// TODO: quitGame();
			break;
		case Common::EVENT_MOUSEMOVE:
			handleMouseMove(event.mouse.x, event.mouse.y);
			break;
		case Common::EVENT_LBUTTONDOWN:
			handleMouseClick(event.mouse.x, event.mouse.y);
			break;
		default:
			break;
		}
	}

}

void MenuSystem::addClickTextItem(ItemID id, int x, int y, int w, uint fontNum, const byte *caption, byte defaultColor, byte activeColor) {
	Item item;
	item.id = id;
	item.defaultColor = defaultColor;
	item.activeColor = activeColor;
	item.x = x;
	item.y = y;
	item.w = w;
	item.fontNum = fontNum;
	setItemCaption(&item, caption);
	_items.push_back(item);
}

void MenuSystem::drawItem(ItemID itemID, bool active) {
	Item *item = getItem(itemID);
	if (item) {
		byte color = active ? item->activeColor : item->defaultColor;
		drawString(item->rect.left, item->rect.top, 0, item->fontNum, color, (byte*)item->caption.c_str());
	}
}

void MenuSystem::handleMouseMove(int x, int y) {
	ItemID newItemID = findItemAt(x, y);
	if (_currItemID != newItemID) {
		leaveItem(_currItemID);
		_currItemID = newItemID;
		enterItem(newItemID);
	}
}

void MenuSystem::handleMouseClick(int x, int y) {
	ItemID id = findItemAt(x, y);
	clickItem(id);
}

ItemID MenuSystem::findItemAt(int x, int y) {
	for (Common::Array<Item>::iterator iter = _items.begin(); iter != _items.end(); iter++) {
		if ((*iter).rect.contains(x, y))
			return (*iter).id;
	}
	return kItemIdNone;
}

MenuSystem::Item *MenuSystem::getItem(ItemID id) {
	for (Common::Array<Item>::iterator iter = _items.begin(); iter != _items.end(); iter++) {
		if ((*iter).id == id)
			return &(*iter);
	}
	return NULL;
}

void MenuSystem::setItemCaption(Item *item, const byte *caption) {
	Font font(_vm->_res->load(_vm->_screen->getFontResIndex(item->fontNum))->data);
	int width = font.getTextWidth((byte*)caption);
	int height = font.getHeight();
	item->rect = Common::Rect(item->x, item->y, item->x + width, item->y + height);
	if (item->w) {
		item->rect.translate(item->w - width / 2, 0);
	}
	item->caption = (const char*)caption;
}

void MenuSystem::initMenu(MenuID menuID) {

	_items.clear();

	memcpy(_vm->_screen->_frontScreen, _background->pixels, 640 * 400);

	switch (menuID) {
	case kMenuIdMain:
		drawString(0, 74, 320, 1, 229, (byte*)"What can I do for you?");
		addClickTextItem(kItemIdLoad, 0, 115, 320, 0, (const byte*)"LOAD", kFontColorMenuDefault, kFontColorMenuActive);
		addClickTextItem(kItemIdSave, 0, 135, 320, 0, (const byte*)"SAVE", kFontColorMenuDefault, kFontColorMenuActive);
		addClickTextItem(kItemIdToggleText, 0, 165, 320, 0, (const byte*)"TEXT ON", kFontColorMenuDefault, kFontColorMenuActive);
		addClickTextItem(kItemIdToggleVoices, 0, 185, 320, 0, (const byte*)"VOICES ON", kFontColorMenuDefault, kFontColorMenuActive);
		addClickTextItem(kItemIdVolumesMenu, 0, 215, 320, 0, (const byte*)"VOLUME", kFontColorMenuDefault, kFontColorMenuActive);
		addClickTextItem(kItemIdPlay, 0, 245, 320, 0, (const byte*)"PLAY", kFontColorMenuDefault, kFontColorMenuActive);
		addClickTextItem(kItemIdQuit, 0, 275, 320, 0, (const byte*)"QUIT GAME", kFontColorMenuDefault, kFontColorMenuActive);
		break;
	case kMenuIdVolumes:
		drawString(0, 74, 320, 1, 229, (byte*)"Adjust volume");
		drawString(0, 130, 200, 0, 244, (byte*)"Master");
		drawString(0, 155, 200, 0, 244, (byte*)"Voices");
		drawString(0, 180, 200, 0, 244, (byte*)"Music");
		drawString(0, 205, 200, 0, 244, (byte*)"Sound FX");
		drawString(0, 230, 200, 0, 244, (byte*)"Background");
		addClickTextItem(kItemIdDone, 0, 275, 200, 0, (const byte*)"DONE", kFontColorMenuDefault, 253);
		addClickTextItem(kItemIdCancel, 0, 275, 440, 0, (const byte*)"CANCEL", kFontColorMenuDefault, 253);
		addClickTextItem(kItemIdMasterDown, 0, 130 + 25 * 0, 348, 1, (const byte*)"[", kFontColorMenuDefault, 253);
		addClickTextItem(kItemIdVoicesDown, 0, 130 + 25 * 1, 348, 1, (const byte*)"[", kFontColorMenuDefault, 253);
		addClickTextItem(kItemIdMusicDown, 0, 130 + 25 * 2, 348, 1, (const byte*)"[", kFontColorMenuDefault, 253);
		addClickTextItem(kItemIdSoundFXDown, 0, 130 + 25 * 3, 348, 1, (const byte*)"[", kFontColorMenuDefault, 253);
		addClickTextItem(kItemIdBackgroundDown, 0, 130 + 25 * 4, 348, 1, (const byte*)"[", kFontColorMenuDefault, 253);
		addClickTextItem(kItemIdMasterUp, 0, 130 + 25 * 0, 372, 1, (const byte*)"]", kFontColorMenuDefault, 253);
		addClickTextItem(kItemIdVoicesUp, 0, 130 + 25 * 1, 372, 1, (const byte*)"]", kFontColorMenuDefault, 253);
		addClickTextItem(kItemIdMusicUp, 0, 130 + 25 * 2, 372, 1, (const byte*)"]", kFontColorMenuDefault, 253);
		addClickTextItem(kItemIdSoundFXUp, 0, 130 + 25 * 3, 372, 1, (const byte*)"]", kFontColorMenuDefault, 253);
		addClickTextItem(kItemIdBackgroundUp, 0, 130 + 25 * 4, 372, 1, (const byte*)"]", kFontColorMenuDefault, 253);
		drawVolumeBar(kItemIdMaster);
		drawVolumeBar(kItemIdVoices);
		drawVolumeBar(kItemIdMusic);
		drawVolumeBar(kItemIdSoundFX);
		drawVolumeBar(kItemIdBackground);
		break;
	default:
		break;
	}

	for (Common::Array<Item>::iterator iter = _items.begin(); iter != _items.end(); iter++) {
		drawItem((*iter).id, false);
	}

}

void MenuSystem::enterItem(ItemID id) {
	drawItem(id, true);
}

void MenuSystem::leaveItem(ItemID id) {
	drawItem(id, false);
}

void MenuSystem::clickItem(ItemID id) {
	Item *item = getItem(id);
	switch (id) {
	// Main menu
	case kItemIdSave:
		debug("kItemIdSave");
		break;
	case kItemIdLoad:
		debug("kItemIdLoad");
		break;
	case kItemIdToggleText:
		setCfgText(!_cfgText, true);
		break;
	case kItemIdToggleVoices:
		setCfgVoices(!_cfgVoices, true);
		break;
	case kItemIdVolumesMenu:
		debug("kItemIdVolumesMenu");
		_newMenuID = kMenuIdVolumes;
		break;
	case kItemIdPlay:
		debug("kItemIdPlay");
		break;
	case kItemIdQuit:
		debug("kItemIdQuit");
		break;
	// Volumes menu
	case kItemIdMasterUp:
		changeVolumeBar(kItemIdMaster, +1);
		break;
	case kItemIdVoicesUp:
		changeVolumeBar(kItemIdVoices, +1);
		break;
	case kItemIdMusicUp:
		changeVolumeBar(kItemIdMusic, +1);
		break;
	case kItemIdSoundFXUp:
		changeVolumeBar(kItemIdSoundFX, +1);
		break;
	case kItemIdBackgroundUp:
		changeVolumeBar(kItemIdBackground, +1);
		break;
	case kItemIdMasterDown:
		changeVolumeBar(kItemIdMaster, -1);
		break;
	case kItemIdVoicesDown:
		changeVolumeBar(kItemIdVoices, -1);
		break;
	case kItemIdMusicDown:
		changeVolumeBar(kItemIdMusic, -1);
		break;
	case kItemIdSoundFXDown:
		changeVolumeBar(kItemIdSoundFX, -1);
		break;
	case kItemIdBackgroundDown:
		changeVolumeBar(kItemIdBackground, -1);
		break;
	case kItemIdCancel:
		_newMenuID = kMenuIdMain;
		break;
	// Save/Load menu
	default:
		break;
	}
}

void MenuSystem::saveBackground() {
}

void MenuSystem::restoreBackground() {
}

void MenuSystem::restoreRect(int x, int y, int w, int h) {
	byte *src = (byte*)_background->getBasePtr(x, y);
	byte *dst = _vm->_screen->_frontScreen + x + y * 640;
	while (h--) {
		memcpy(dst, src, w);
		src += 640;
		dst += 640;
	}
}

void MenuSystem::drawString(int16 x, int16 y, int w, uint fontNum, byte color, byte *text) {
	fontNum = _vm->_screen->getFontResIndex(fontNum);
	if (w) {
		Font font(_vm->_res->load(fontNum)->data);
		x = x + w - font.getTextWidth(text) / 2;
	}
	_vm->_screen->drawString(x, y, color, fontNum, text, -1, NULL, true);
}

void MenuSystem::setCfgText(bool value, bool active) {
	if (_cfgText != value) {
		Item *item = getItem(kItemIdToggleText);
		_cfgText = value;
		if (_cfgText)
			setItemCaption(item, (const byte*)"TEXT ON");
		else
			setItemCaption(item, (const byte*)"TEXT OFF");
		drawItem(kItemIdToggleText, true);
	}
}

void MenuSystem::setCfgVoices(bool value, bool active) {
	if (_cfgVoices != value) {
		Item *item = getItem(kItemIdToggleVoices);
		_cfgVoices = value;
		if (_cfgVoices)
			setItemCaption(item, (const byte*)"VOICES ON");
		else
			setItemCaption(item, (const byte*)"VOICES OFF");
		drawItem(kItemIdToggleVoices, true);
	}
}

void MenuSystem::drawVolumeBar(ItemID itemID) {
	int w = 440, y, volume;
	char text[21];
	
	switch (itemID) {
	case kItemIdMaster:
		y = 130 + 25 * 0;
		volume = _cfgMasterVolume;
		break;
	case kItemIdVoices:
		y = 130 + 25 * 1;
		volume = _cfgVoicesVolume;
		break;
	case kItemIdMusic:
		y = 130 + 25 * 2;
		volume = _cfgMusicVolume;
		break;
	case kItemIdSoundFX:
		y = 130 + 25 * 3;
		volume = _cfgSoundFXVolume;
		break;
	case kItemIdBackground:
		y = 130 + 25 * 4;
		volume = _cfgBackgroundVolume;
		break;
	default:
		return;
	}

	restoreRect(390, y, 100, 25);
	
	for (int i = 0; i < volume; i++)
		text[i] = '|';
	text[volume] = 0;
	
	drawString(0, y, w, 0, 246, (byte*)text);
	
}

void MenuSystem::changeVolumeBar(ItemID itemID, int delta) {

	int *volume, newVolume;

	switch (itemID) {
	case kItemIdMaster:
		volume = &_cfgMasterVolume;
		break;
	case kItemIdVoices:
		volume = &_cfgVoicesVolume;
		break;
	case kItemIdMusic:
		volume = &_cfgMusicVolume;
		break;
	case kItemIdSoundFX:
		volume = &_cfgSoundFXVolume;
		break;
	case kItemIdBackground:
		volume = &_cfgBackgroundVolume;
		break;
	default:
		return;
	}

	newVolume = CLIP(*volume + delta, 0, 20);

	if (newVolume != *volume) {
		*volume = newVolume;
		drawVolumeBar(itemID);
	}

}

} // End of namespace Picture
