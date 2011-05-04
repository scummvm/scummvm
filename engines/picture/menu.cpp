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

#include "common/savefile.h"

#include "picture/picture.h"
#include "picture/menu.h"
#include "picture/palette.h"
#include "picture/render.h"
#include "picture/resource.h"

namespace Picture {

MenuSystem::MenuSystem(PictureEngine *vm) : _vm(vm) {
}

MenuSystem::~MenuSystem() {
}

int MenuSystem::run() {

	debug("MenuSystem::run()");

	_background = new Graphics::Surface();
	_background->create(640, 400, Graphics::PixelFormat::createFormatCLUT8());

	_currMenuID = kMenuIdNone;
	_newMenuID = kMenuIdMain;
	_currItemID = kItemIdNone;
	_editingDescription = false;
	_needRedraw = false;
	_cfgText = true;
	_cfgVoices = true;
	_cfgMasterVolume = 10;
	_cfgVoicesVolume = 10;
	_cfgMusicVolume = 10;
	_cfgSoundFXVolume = 10;
	_cfgBackgroundVolume = 10;
	_running = true;    	
	_top = 30 - _vm->_guiHeight / 2;
	_needRedraw = false;

	// TODO: buildColorTransTable2
	_vm->_palette->buildColorTransTable(0, 16, 7);

	_vm->_screen->_renderQueue->clear();
	_vm->_screen->blastSprite(0x140 + _vm->_cameraX, 0x175 + _vm->_cameraY, 0, 1, 0x4000);

	memcpy(_background->pixels, _vm->_screen->_frontScreen, 640 * 400);

	shadeRect(60, 39, 520, 246, 30, 94);

	while (_running) {
		update();
		_vm->_system->updateScreen();
	}
	
	delete _background;

	return 0;	
}

void MenuSystem::update() {

	if (_currMenuID != _newMenuID) {
		_currMenuID = _newMenuID;
		debug("_currMenuID = %d", _currMenuID);
		initMenu(_currMenuID);
	}

	handleEvents();

	if (_needRedraw) {
		//_vm->_system->copyRectToScreen((const byte *)_vm->_screen->_frontScreen + 39 * 640 + 60, 640, 60, 39, 520, 247);
		_vm->_system->copyRectToScreen((const byte *)_vm->_screen->_frontScreen, 640, 0, 0, 640, 400);
		debug("redraw");
		_needRedraw = false;
	}

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
			_running = false;
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

void MenuSystem::addClickTextItem(ItemID id, int x, int y, int w, uint fontNum, const char *caption, byte defaultColor, byte activeColor) {
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
		drawString(item->rect.left, item->y, 0, item->fontNum, color, item->caption.c_str());
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
			setItemCaption(_editingDescriptionItem, _editingDescriptionItem->caption.c_str());
			drawItem(_editingDescriptionID, true);
		} else if (kbd.keycode == Common::KEYCODE_BACKSPACE) {
			_editingDescriptionItem->caption.deleteLastChar();
			restoreRect(_editingDescriptionItem->rect.left, _editingDescriptionItem->rect.top,
				_editingDescriptionItem->rect.width() + 1, _editingDescriptionItem->rect.height() - 2);
			setItemCaption(_editingDescriptionItem, _editingDescriptionItem->caption.c_str());
			drawItem(_editingDescriptionID, true);
		} else if (kbd.keycode == Common::KEYCODE_RETURN) {
			SavegameItem *savegameItem = getSavegameItemByID(_editingDescriptionID);
			_editingDescription = false;
			_vm->requestSavegame(savegameItem->_slotNum, _editingDescriptionItem->caption);
			_running = false;
		} else if (kbd.keycode == Common::KEYCODE_ESCAPE) {
			_editingDescription = false;
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

void MenuSystem::setItemCaption(Item *item, const char *caption) {
	Font font(_vm->_res->load(_vm->_screen->getFontResIndex(item->fontNum))->data);
	int width = font.getTextWidth((const byte*)caption);
	int height = font.getHeight();
	item->rect = Common::Rect(item->x, item->y - height, item->x + width, item->y);
	if (item->w) {
		item->rect.translate(item->w - width / 2, 0);
	}
	item->caption = caption;
}

void MenuSystem::initMenu(MenuID menuID) {

	int newSlotNum;

	_items.clear();

	memcpy(_vm->_screen->_frontScreen, _background->pixels, 640 * 400);

	switch (menuID) {
	case kMenuIdMain:
		drawString(0, 74, 320, 1, 229, _vm->getSysString(kStrWhatCanIDoForYou));
		addClickTextItem(kItemIdLoad, 0, 115, 320, 0, _vm->getSysString(kStrLoad), 229, 255);
		addClickTextItem(kItemIdSave, 0, 135, 320, 0, _vm->getSysString(kStrSave), 229, 255);
		addClickTextItem(kItemIdToggleText, 0, 165, 320, 0, _vm->getSysString(kStrTextOn), 229, 255);
		addClickTextItem(kItemIdToggleVoices, 0, 185, 320, 0, _vm->getSysString(kStrVoicesOn), 229, 255);
		addClickTextItem(kItemIdVolumesMenu, 0, 215, 320, 0, _vm->getSysString(kStrVolume), 229, 255);
		addClickTextItem(kItemIdPlay, 0, 245, 320, 0, _vm->getSysString(kStrPlay), 229, 255);
		addClickTextItem(kItemIdQuit, 0, 275, 320, 0, _vm->getSysString(kStrQuit), 229, 255);
		break;
	case kMenuIdLoad:
		drawString(0, 74, 320, 1, 229, _vm->getSysString(kStrLoadGame));
		addClickTextItem(kItemIdSavegameUp, 0, 155, 545, 1, "^", 255, 253);
		addClickTextItem(kItemIdSavegameDown, 0, 195, 545, 1, "\\", 255, 253);
		addClickTextItem(kItemIdCancel, 0, 275, 320, 0, _vm->getSysString(kStrCancel), 255, 253);
		addClickTextItem(kItemIdSavegame1, 0, 115 + 20 * 0, 300, 0, "SAVEGAME 1", 231, 234);
		addClickTextItem(kItemIdSavegame2, 0, 115 + 20 * 1, 300, 0, "SAVEGAME 2", 231, 234);
		addClickTextItem(kItemIdSavegame3, 0, 115 + 20 * 2, 300, 0, "SAVEGAME 3", 231, 234);
		addClickTextItem(kItemIdSavegame4, 0, 115 + 20 * 3, 300, 0, "SAVEGAME 4", 231, 234);
		addClickTextItem(kItemIdSavegame5, 0, 115 + 20 * 4, 300, 0, "SAVEGAME 5", 231, 234);
		addClickTextItem(kItemIdSavegame6, 0, 115 + 20 * 5, 300, 0, "SAVEGAME 6", 231, 234);
		addClickTextItem(kItemIdSavegame7, 0, 115 + 20 * 6, 300, 0, "SAVEGAME 7", 231, 234);
		loadSavegamesList();
		setSavegameCaptions();
		break;
	case kMenuIdSave:
		drawString(0, 74, 320, 1, 229, _vm->getSysString(kStrSaveGame));
		addClickTextItem(kItemIdSavegameUp, 0, 155, 545, 1, "^", 255, 253);
		addClickTextItem(kItemIdSavegameDown, 0, 195, 545, 1, "\\", 255, 253);
		addClickTextItem(kItemIdCancel, 0, 275, 320, 0, _vm->getSysString(kStrCancel), 255, 253);
		addClickTextItem(kItemIdSavegame1, 0, 115 + 20 * 0, 300, 0, "SAVEGAME 1", 231, 234);
		addClickTextItem(kItemIdSavegame2, 0, 115 + 20 * 1, 300, 0, "SAVEGAME 2", 231, 234);
		addClickTextItem(kItemIdSavegame3, 0, 115 + 20 * 2, 300, 0, "SAVEGAME 3", 231, 234);
		addClickTextItem(kItemIdSavegame4, 0, 115 + 20 * 3, 300, 0, "SAVEGAME 4", 231, 234);
		addClickTextItem(kItemIdSavegame5, 0, 115 + 20 * 4, 300, 0, "SAVEGAME 5", 231, 234);
		addClickTextItem(kItemIdSavegame6, 0, 115 + 20 * 5, 300, 0, "SAVEGAME 6", 231, 234);
		addClickTextItem(kItemIdSavegame7, 0, 115 + 20 * 6, 300, 0, "SAVEGAME 7", 231, 234);
		newSlotNum = loadSavegamesList() + 1;
		_savegames.push_back(SavegameItem(newSlotNum, Common::String::format("GAME %03d", _savegames.size() + 1)));
		setSavegameCaptions();
		break;
	case kMenuIdVolumes:
		drawString(0, 74, 320, 1, 229, _vm->getSysString(kStrAdjustVolume));
		drawString(0, 130, 200, 0, 246, _vm->getSysString(kStrMaster));
		drawString(0, 155, 200, 0, 244, _vm->getSysString(kStrVoices));
		drawString(0, 180, 200, 0, 244, _vm->getSysString(kStrMusic));
		drawString(0, 205, 200, 0, 244, _vm->getSysString(kStrSoundFx));
		drawString(0, 230, 200, 0, 244, _vm->getSysString(kStrBackground));
		addClickTextItem(kItemIdDone, 0, 275, 200, 0, _vm->getSysString(kStrDone), 229, 253);
		addClickTextItem(kItemIdCancel, 0, 275, 440, 0, _vm->getSysString(kStrCancel), 229, 253);
		addClickTextItem(kItemIdMasterDown, 0, 130 + 25 * 0, 348, 1, "[", 229, 253);
		addClickTextItem(kItemIdVoicesDown, 0, 130 + 25 * 1, 348, 1, "[", 229, 253);
		addClickTextItem(kItemIdMusicDown, 0, 130 + 25 * 2, 348, 1, "[", 229, 253);
		addClickTextItem(kItemIdSoundFXDown, 0, 130 + 25 * 3, 348, 1, "[", 229, 253);
		addClickTextItem(kItemIdBackgroundDown, 0, 130 + 25 * 4, 348, 1, "[", 229, 253);
		addClickTextItem(kItemIdMasterUp, 0, 130 + 25 * 0, 372, 1, "]", 229, 253);
		addClickTextItem(kItemIdVoicesUp, 0, 130 + 25 * 1, 372, 1, "]", 229, 253);
		addClickTextItem(kItemIdMusicUp, 0, 130 + 25 * 2, 372, 1, "]", 229, 253);
		addClickTextItem(kItemIdSoundFXUp, 0, 130 + 25 * 3, 372, 1, "]", 229, 253);
		addClickTextItem(kItemIdBackgroundUp, 0, 130 + 25 * 4, 372, 1, "]", 229, 253);
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
	//Item *item = getItem(id);
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
		_running = false;
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
	// FIXME: Why is the following block disabled?
#if 0
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
#endif	
	byte *src = (byte*)_background->getBasePtr(x, y);
	for (int xc = 0; xc < w; xc++) {
		src[xc] = color2;
		src[xc + h * 640] = color1;
	}
	src += 640;
	w -= 1;
	h -= 1;
	while (h--) {
		src[0] = color2;
		src[w] = color1;
		src += 640;
	}
}

void MenuSystem::drawString(int16 x, int16 y, int w, uint fontNum, byte color, const char *text) {
	fontNum = _vm->_screen->getFontResIndex(fontNum);
	Font font(_vm->_res->load(fontNum)->data);
	if (w) {
		x = x + w - font.getTextWidth((const byte*)text) / 2;
	}
	_vm->_screen->drawString(x, y - font.getHeight(), color, fontNum, (const byte*)text, -1, NULL, true);
	_needRedraw = true;
}

int MenuSystem::loadSavegamesList() {

	int maxSlotNum = -1;

	_savegameListTopIndex = 0;
	_savegames.clear();

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Picture::PictureEngine::SaveHeader header;
	Common::String pattern = _vm->getTargetName();
	pattern += ".???";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); file++) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);
		if (slotNum > maxSlotNum)
			maxSlotNum = slotNum;

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Picture::PictureEngine::readSaveHeader(in, false, header) == Picture::PictureEngine::kRSHENoError) {
					_savegames.push_back(SavegameItem(slotNum, header.description));
					debug("%s -> %s", file->c_str(), header.description.c_str());
				}
				delete in;
			}
		}
	}

	return maxSlotNum;
}

MenuSystem::SavegameItem *MenuSystem::getSavegameItemByID(ItemID id) {
	switch (id) {
	case kItemIdSavegame1:
		return &_savegames[_savegameListTopIndex + 0];
	case kItemIdSavegame2:
		return &_savegames[_savegameListTopIndex + 1];
	case kItemIdSavegame3:
		return &_savegames[_savegameListTopIndex + 2];
	case kItemIdSavegame4:
		return &_savegames[_savegameListTopIndex + 3];
	case kItemIdSavegame5:
		return &_savegames[_savegameListTopIndex + 4];
	case kItemIdSavegame6:
		return &_savegames[_savegameListTopIndex + 5];
	case kItemIdSavegame7:
		return &_savegames[_savegameListTopIndex + 6];
	default:
		return NULL;
	}
}

void MenuSystem::setSavegameCaptions() {
	uint index = _savegameListTopIndex;
	setItemCaption(getItem(kItemIdSavegame1), index < _savegames.size() ? _savegames[index++]._description.c_str() : "");
	setItemCaption(getItem(kItemIdSavegame2), index < _savegames.size() ? _savegames[index++]._description.c_str() : "");
	setItemCaption(getItem(kItemIdSavegame3), index < _savegames.size() ? _savegames[index++]._description.c_str() : "");
	setItemCaption(getItem(kItemIdSavegame4), index < _savegames.size() ? _savegames[index++]._description.c_str() : "");
	setItemCaption(getItem(kItemIdSavegame5), index < _savegames.size() ? _savegames[index++]._description.c_str() : "");
	setItemCaption(getItem(kItemIdSavegame6), index < _savegames.size() ? _savegames[index++]._description.c_str() : "");
	setItemCaption(getItem(kItemIdSavegame7), index < _savegames.size() ? _savegames[index++]._description.c_str() : "");
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
		SavegameItem *savegameItem = getSavegameItemByID(id);
		debug("slotNum = [%d]; description = [%s]", savegameItem->_slotNum, savegameItem->_description.c_str());
		//_vm->loadgame(savegameItem->_filename.c_str());
		_vm->requestLoadgame(savegameItem->_slotNum);
		_running = false;
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
		setItemCaption(item, _vm->getSysString(_cfgText ? kStrTextOn : kStrTextOff));
		drawItem(kItemIdToggleText, true);
	}
}

void MenuSystem::setCfgVoices(bool value, bool active) {
	if (_cfgVoices != value) {
		Item *item = getItem(kItemIdToggleVoices);
		_cfgVoices = value;
		restoreRect(item->rect.left, item->rect.top, item->rect.width() + 1, item->rect.height() - 2);
		setItemCaption(item, _vm->getSysString(_cfgVoices ? kStrVoicesOn : kStrVoicesOff));
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
	
	drawString(0, y, w, 0, 246, text);
	
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
