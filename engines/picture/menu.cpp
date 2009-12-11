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
#include "picture/palette.h"
#include "picture/resource.h"
#include "picture/screen.h"

namespace Picture {

MenuSystem::MenuSystem(PictureEngine *vm) : _vm(vm) {
	_currMenuID = kMenuIdNone;
	_newMenuID = kMenuIdMain;
	_newMenuID = kMenuIdSave;
	_newMenuID = kMenuIdLoad;
	_currItemID = kItemIdNone;
	_editingDescription = false;
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

	_top = 30 - _vm->_guiHeight / 2;

	memset(_vm->_screen->_frontScreen, 250, 640 * 400);

	memcpy(_background->pixels, _vm->_screen->_frontScreen, 640 * 400);

	_vm->_palette->buildColorTransTable(0, 16, 7);

	shadeRect(60, 39, 520, 246, 30, 94);
	
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
			handleKeyDown(event.kbd);
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
		drawString(item->rect.left, item->y, 0, item->fontNum, color, (byte*)item->caption.c_str());
	}
}

void MenuSystem::handleMouseMove(int x, int y) {
	if (!_editingDescription) {
		ItemID newItemID = findItemAt(x, y);
		if (_currItemID != newItemID) {
			leaveItem(_currItemID);
			_currItemID = newItemID;
			enterItem(newItemID);
		}
	}
}

void MenuSystem::handleMouseClick(int x, int y) {
	if (!_editingDescription) {
		ItemID id = findItemAt(x, y);
		clickItem(id);
	}
}

void MenuSystem::handleKeyDown(const Common::KeyState& kbd) {
	if (_editingDescription) {
		if (kbd.keycode >= Common::KEYCODE_SPACE && kbd.keycode <= Common::KEYCODE_z) {
			_editingDescriptionItem->caption += kbd.ascii;
			restoreRect(_editingDescriptionItem->rect.left, _editingDescriptionItem->rect.top,
				_editingDescriptionItem->rect.width() + 1, _editingDescriptionItem->rect.height() - 2);
			setItemCaption(_editingDescriptionItem, (const byte*)_editingDescriptionItem->caption.c_str());
			drawItem(_editingDescriptionID, true);
		} else if (kbd.keycode == Common::KEYCODE_BACKSPACE) {
			_editingDescriptionItem->caption.deleteLastChar();
			restoreRect(_editingDescriptionItem->rect.left, _editingDescriptionItem->rect.top,
				_editingDescriptionItem->rect.width() + 1, _editingDescriptionItem->rect.height() - 2);
			setItemCaption(_editingDescriptionItem, (const byte*)_editingDescriptionItem->caption.c_str());
			drawItem(_editingDescriptionID, true);
		} else if (kbd.keycode == Common::KEYCODE_RETURN) {
			_editingDescription = false;
			_newMenuID = kMenuIdMain;
		}
	}
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
	item->rect = Common::Rect(item->x, item->y - height, item->x + width, item->y);
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
		addClickTextItem(kItemIdLoad, 0, 115, 320, 0, (const byte*)"LOAD", 229, 255);
		addClickTextItem(kItemIdSave, 0, 135, 320, 0, (const byte*)"SAVE", 229, 255);
		addClickTextItem(kItemIdToggleText, 0, 165, 320, 0, (const byte*)"TEXT ON", 229, 255);
		addClickTextItem(kItemIdToggleVoices, 0, 185, 320, 0, (const byte*)"VOICES ON", 229, 255);
		addClickTextItem(kItemIdVolumesMenu, 0, 215, 320, 0, (const byte*)"VOLUME", 229, 255);
		addClickTextItem(kItemIdPlay, 0, 245, 320, 0, (const byte*)"PLAY", 229, 255);
		addClickTextItem(kItemIdQuit, 0, 275, 320, 0, (const byte*)"QUIT GAME", 229, 255);
		break;
	case kMenuIdLoad:
		drawString(0, 74, 320, 1, 229, (byte*)"Load game");
		addClickTextItem(kItemIdSavegameUp, 0, 155, 545, 1, (const byte*)"^", 255, 253);
		addClickTextItem(kItemIdSavegameDown, 0, 195, 545, 1, (const byte*)"\\", 255, 253);
		addClickTextItem(kItemIdCancel, 0, 275, 320, 0, (const byte*)"CANCEL", 255, 253);
		addClickTextItem(kItemIdSavegame1, 0, 115 + 20 * 0, 300, 0, (const byte*)"SAVEGAME 1", 231, 234);
		addClickTextItem(kItemIdSavegame2, 0, 115 + 20 * 1, 300, 0, (const byte*)"SAVEGAME 2", 231, 234);
		addClickTextItem(kItemIdSavegame3, 0, 115 + 20 * 2, 300, 0, (const byte*)"SAVEGAME 3", 231, 234);
		addClickTextItem(kItemIdSavegame4, 0, 115 + 20 * 3, 300, 0, (const byte*)"SAVEGAME 4", 231, 234);
		addClickTextItem(kItemIdSavegame5, 0, 115 + 20 * 4, 300, 0, (const byte*)"SAVEGAME 5", 231, 234);
		addClickTextItem(kItemIdSavegame6, 0, 115 + 20 * 5, 300, 0, (const byte*)"SAVEGAME 6", 231, 234);
		addClickTextItem(kItemIdSavegame7, 0, 115 + 20 * 6, 300, 0, (const byte*)"SAVEGAME 7", 231, 234);
		initSavegames();
		setSavegameCaptions();
		break;
	case kMenuIdSave:
		drawString(0, 74, 320, 1, 229, (byte*)"Save game");
		addClickTextItem(kItemIdSavegameUp, 0, 155, 545, 1, (const byte*)"^", 255, 253);
		addClickTextItem(kItemIdSavegameDown, 0, 195, 545, 1, (const byte*)"\\", 255, 253);
		addClickTextItem(kItemIdCancel, 0, 275, 320, 0, (const byte*)"CANCEL", 255, 253);
		addClickTextItem(kItemIdSavegame1, 0, 115 + 20 * 0, 300, 0, (const byte*)"SAVEGAME 1", 231, 234);
		addClickTextItem(kItemIdSavegame2, 0, 115 + 20 * 1, 300, 0, (const byte*)"SAVEGAME 2", 231, 234);
		addClickTextItem(kItemIdSavegame3, 0, 115 + 20 * 2, 300, 0, (const byte*)"SAVEGAME 3", 231, 234);
		addClickTextItem(kItemIdSavegame4, 0, 115 + 20 * 3, 300, 0, (const byte*)"SAVEGAME 4", 231, 234);
		addClickTextItem(kItemIdSavegame5, 0, 115 + 20 * 4, 300, 0, (const byte*)"SAVEGAME 5", 231, 234);
		addClickTextItem(kItemIdSavegame6, 0, 115 + 20 * 5, 300, 0, (const byte*)"SAVEGAME 6", 231, 234);
		addClickTextItem(kItemIdSavegame7, 0, 115 + 20 * 6, 300, 0, (const byte*)"SAVEGAME 7", 231, 234);
		initSavegames();
		_savegames.push_back(SavegameItem("", Common::String::printf("GAME %03d", _savegames.size() + 1)));
		setSavegameCaptions();
		break;
	case kMenuIdVolumes:
		drawString(0, 74, 320, 1, 229, (byte*)"Adjust volume");
		drawString(0, 130, 200, 0, 246, (byte*)"Master");
		drawString(0, 155, 200, 0, 244, (byte*)"Voices");
		drawString(0, 180, 200, 0, 244, (byte*)"Music");
		drawString(0, 205, 200, 0, 244, (byte*)"Sound FX");
		drawString(0, 230, 200, 0, 244, (byte*)"Background");
		addClickTextItem(kItemIdDone, 0, 275, 200, 0, (const byte*)"DONE", 229, 253);
		addClickTextItem(kItemIdCancel, 0, 275, 440, 0, (const byte*)"CANCEL", 229, 253);
		addClickTextItem(kItemIdMasterDown, 0, 130 + 25 * 0, 348, 1, (const byte*)"[", 229, 253);
		addClickTextItem(kItemIdVoicesDown, 0, 130 + 25 * 1, 348, 1, (const byte*)"[", 229, 253);
		addClickTextItem(kItemIdMusicDown, 0, 130 + 25 * 2, 348, 1, (const byte*)"[", 229, 253);
		addClickTextItem(kItemIdSoundFXDown, 0, 130 + 25 * 3, 348, 1, (const byte*)"[", 229, 253);
		addClickTextItem(kItemIdBackgroundDown, 0, 130 + 25 * 4, 348, 1, (const byte*)"[", 229, 253);
		addClickTextItem(kItemIdMasterUp, 0, 130 + 25 * 0, 372, 1, (const byte*)"]", 229, 253);
		addClickTextItem(kItemIdVoicesUp, 0, 130 + 25 * 1, 372, 1, (const byte*)"]", 229, 253);
		addClickTextItem(kItemIdMusicUp, 0, 130 + 25 * 2, 372, 1, (const byte*)"]", 229, 253);
		addClickTextItem(kItemIdSoundFXUp, 0, 130 + 25 * 3, 372, 1, (const byte*)"]", 229, 253);
		addClickTextItem(kItemIdBackgroundUp, 0, 130 + 25 * 4, 372, 1, (const byte*)"]", 229, 253);
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
		_newMenuID = kMenuIdSave;
		break;
	case kItemIdLoad:
		_newMenuID = kMenuIdLoad;
		break;
	case kItemIdToggleText:
		setCfgText(!_cfgText, true);
		if (!_cfgVoices && !_cfgText)
			setCfgVoices(true, false);
		break;
	case kItemIdToggleVoices:
		setCfgVoices(!_cfgVoices, true);
		if (!_cfgVoices && !_cfgText)
			setCfgText(true, false);
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
	case kItemIdSavegame1:
	case kItemIdSavegame2:
	case kItemIdSavegame3:
	case kItemIdSavegame4:
	case kItemIdSavegame5:
	case kItemIdSavegame6:
	case kItemIdSavegame7:
		clickSavegameItem(id);
		break;
	case kItemIdDone:
		_newMenuID = kMenuIdMain;
		break;
	case kItemIdSavegameUp:
		scrollSavegames(-6);
		break;
	case kItemIdSavegameDown:
		scrollSavegames(+6);
		break;
	default:
		break;
	}
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

void MenuSystem::shadeRect(int x, int y, int w, int h, byte color1, byte color2) {
	byte *src = (byte*)_background->getBasePtr(x, y);
	for (int xc = 0; xc < w; xc++) {
		src[xc] = color1;
		src[xc + h * 640] = 46;
	}
	src += 640;
	w -= 1;
	h -= 1;
	while (h--) {
		src[0] = color2;
		src[w] = color2;
		for (int xc = 1; xc < w; xc++) {
			src[xc] = _vm->_palette->getColorTransPixel(src[xc]);
		}
		src += 640;
	}
}

void MenuSystem::drawString(int16 x, int16 y, int w, uint fontNum, byte color, byte *text) {
	fontNum = _vm->_screen->getFontResIndex(fontNum);
	Font font(_vm->_res->load(fontNum)->data);
	if (w) {
		x = x + w - font.getTextWidth(text) / 2;
	}
	_vm->_screen->drawString(x, y - font.getHeight(), color, fontNum, text, -1, NULL, true);
}

void MenuSystem::initSavegames() {

	_savegameListTopIndex = 0;
	_savegames.clear();
	
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Picture::PictureEngine::SaveHeader header;
	Common::String pattern = _vm->getTargetName();
	pattern += ".???";

	Common::StringList filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	for (Common::StringList::const_iterator file = filenames.begin(); file != filenames.end(); file++) {
		Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
		if (in) {
			if (_vm->readSaveHeader(in, false, header) == Picture::PictureEngine::kRSHENoError) {
				_savegames.push_back(SavegameItem(*file, header.description));
				debug("%s -> %s", file->c_str(), header.description.c_str());
			}
			delete in;
		}
	}

	// DEBUG: Add some more items
	_savegames.push_back(SavegameItem("abc", "Test 1"));
	_savegames.push_back(SavegameItem("abc", "Test 2"));
	_savegames.push_back(SavegameItem("abc", "Test 3"));
	_savegames.push_back(SavegameItem("abc", "Test 4"));
	_savegames.push_back(SavegameItem("abc", "Test 5"));
	_savegames.push_back(SavegameItem("abc", "Test 6"));
	_savegames.push_back(SavegameItem("abc", "Test 7"));
	_savegames.push_back(SavegameItem("abc", "Test 8"));
	_savegames.push_back(SavegameItem("abc", "Test 9"));
	_savegames.push_back(SavegameItem("abc", "Test 10"));
	_savegames.push_back(SavegameItem("abc", "Test 11"));
	_savegames.push_back(SavegameItem("abc", "Test 12"));

}

void MenuSystem::setSavegameCaptions() {
	uint index = _savegameListTopIndex;
	setItemCaption(getItem(kItemIdSavegame1), index < _savegames.size() ? (const byte*)_savegames[index++]._description.c_str() : (const byte*)"");
	setItemCaption(getItem(kItemIdSavegame2), index < _savegames.size() ? (const byte*)_savegames[index++]._description.c_str() : (const byte*)"");
	setItemCaption(getItem(kItemIdSavegame3), index < _savegames.size() ? (const byte*)_savegames[index++]._description.c_str() : (const byte*)"");
	setItemCaption(getItem(kItemIdSavegame4), index < _savegames.size() ? (const byte*)_savegames[index++]._description.c_str() : (const byte*)"");
	setItemCaption(getItem(kItemIdSavegame5), index < _savegames.size() ? (const byte*)_savegames[index++]._description.c_str() : (const byte*)"");
	setItemCaption(getItem(kItemIdSavegame6), index < _savegames.size() ? (const byte*)_savegames[index++]._description.c_str() : (const byte*)"");
	setItemCaption(getItem(kItemIdSavegame7), index < _savegames.size() ? (const byte*)_savegames[index++]._description.c_str() : (const byte*)"");
}

void MenuSystem::scrollSavegames(int delta) {
	int newPos = CLIP<int>(_savegameListTopIndex + delta, 0, _savegames.size() - 1);
	_savegameListTopIndex = newPos;
	restoreRect(80, 92, 440, 140);
	setSavegameCaptions();
	drawItem(kItemIdSavegame1, false);
	drawItem(kItemIdSavegame2, false);
	drawItem(kItemIdSavegame3, false);
	drawItem(kItemIdSavegame4, false);
	drawItem(kItemIdSavegame5, false);
	drawItem(kItemIdSavegame6, false);
	drawItem(kItemIdSavegame7, false);
}

void MenuSystem::clickSavegameItem(ItemID id) {
	if (_currMenuID == kMenuIdLoad) {
		SavegameItem *savegameItem;
		switch (id) {
		case kItemIdSavegame1:
			savegameItem = &_savegames[_savegameListTopIndex + 0];
			break;
		case kItemIdSavegame2:
			savegameItem = &_savegames[_savegameListTopIndex + 1];
			break;
		case kItemIdSavegame3:
			savegameItem = &_savegames[_savegameListTopIndex + 2];
			break;
		case kItemIdSavegame4:
			savegameItem = &_savegames[_savegameListTopIndex + 3];
			break;
		case kItemIdSavegame5:
			savegameItem = &_savegames[_savegameListTopIndex + 4];
			break;
		case kItemIdSavegame6:
			savegameItem = &_savegames[_savegameListTopIndex + 5];
			break;
		case kItemIdSavegame7:
			savegameItem = &_savegames[_savegameListTopIndex + 6];
			break;
		default:
			return;
		}
		debug("filename = [%s]; description = [%s]", savegameItem->_filename.c_str(), savegameItem->_description.c_str());
	} else {
		_editingDescription = true;
		_editingDescriptionItem = getItem(id);
		_editingDescriptionID = id;
		_editingDescriptionItem->activeColor = 249;
		_editingDescriptionItem->defaultColor = 249;
		drawItem(_editingDescriptionID, true);
	}
}

void MenuSystem::setCfgText(bool value, bool active) {
	if (_cfgText != value) {
		Item *item = getItem(kItemIdToggleText);
		_cfgText = value;
  		restoreRect(item->rect.left, item->rect.top, item->rect.width() + 1, item->rect.height() - 2);
		setItemCaption(item, _cfgText ? (const byte*)"TEXT ON" : (const byte*)"TEXT OFF");
		drawItem(kItemIdToggleText, true);
	}
}

void MenuSystem::setCfgVoices(bool value, bool active) {
	if (_cfgVoices != value) {
		Item *item = getItem(kItemIdToggleVoices);
		_cfgVoices = value;
		restoreRect(item->rect.left, item->rect.top, item->rect.width() + 1, item->rect.height() - 2);
		setItemCaption(item, _cfgVoices ? (const byte*)"VOICES ON" : (const byte*)"VOICES OFF");
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

	Font font(_vm->_res->load(_vm->_screen->getFontResIndex(1))->data);
	restoreRect(390, y - font.getHeight(), 100, 25);
	
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
