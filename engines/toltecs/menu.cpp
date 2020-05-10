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

#include "audio/mixer.h"

#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/translation.h"

#include "gui/saveload.h"

#include "toltecs/toltecs.h"
#include "toltecs/menu.h"
#include "toltecs/palette.h"
#include "toltecs/render.h"
#include "toltecs/resource.h"

namespace Toltecs {

MenuSystem::MenuSystem(ToltecsEngine *vm) : _vm(vm) {
	_background = nullptr;
	_running = false;
	_currMenuID = _newMenuID = kMenuIdNone;
	_currItemID = kItemIdNone;
	_top = 0;
	_savegameListTopIndex = 0;
	_editingDescription = false;
	_editingDescriptionID = kItemIdNone;
	_editingDescriptionItem = nullptr;
	_needRedraw = false;
}

MenuSystem::~MenuSystem() {
}

int MenuSystem::run(MenuID menuId) {
	_background = new Graphics::Surface();
	_background->create(640, 400, Graphics::PixelFormat::createFormatCLUT8());

	// Save original background
	Graphics::Surface backgroundOrig;
	backgroundOrig.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
	memcpy(backgroundOrig.getBasePtr(0,0), _vm->_screen->_frontScreen, 640 * 400);

	_currMenuID = kMenuIdNone;
	_newMenuID = menuId;
	_currItemID = kItemIdNone;
	_editingDescription = false;

	_running = true;
	_top = 30 - _vm->_guiHeight / 2;

	_needRedraw = false;

	_vm->_palette->buildColorTransTable(0, 16, 7);

	_vm->_screen->_renderQueue->clear();
	// Draw the menu background and frame
	_vm->_screen->blastSprite(0x140 + _vm->_cameraX, 0x175 + _vm->_cameraY, 0, 1, 0x4000);
	shadeRect(60, 39, 520, 247, 225, 229);

	memcpy(_background->getPixels(), _vm->_screen->_frontScreen, 640 * 400);

	while (_running) {
		update();
		_vm->_system->updateScreen();
	}

	// Restore original background
	memcpy(_vm->_screen->_frontScreen, backgroundOrig.getBasePtr(0,0), 640 * 400);
	_vm->_system->copyRectToScreen(_vm->_screen->_frontScreen, 640, 0, 0, 640, 400);
	_vm->_system->updateScreen();

	// Cleanup
	backgroundOrig.free();
	_background->free();
	delete _background;

	return 0;
}

void MenuSystem::update() {
	if (_currMenuID != _newMenuID) {
		_currMenuID = _newMenuID;
		//debug("_currMenuID = %d", _currMenuID);
		initMenu(_currMenuID);
	}

	handleEvents();

	if (_needRedraw) {
		//_vm->_system->copyRectToScreen(_vm->_screen->_frontScreen + 39 * 640 + 60, 640, 60, 39, 520, 247);
		_vm->_system->copyRectToScreen(_vm->_screen->_frontScreen, 640, 0, _top, 640, 400 - _top);
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
		case Common::EVENT_RETURN_TO_LAUNCHER:
		case Common::EVENT_QUIT:
			_running = false;
			break;
		case Common::EVENT_MOUSEMOVE:
			handleMouseMove(event.mouse.x, event.mouse.y);
			break;
		case Common::EVENT_LBUTTONUP:
			handleMouseClick(event.mouse.x, event.mouse.y);
			break;
		default:
			break;
		}
	}
}

void MenuSystem::addClickTextItem(ItemID id, int x, int y, int w, uint fontNum, const char *caption, byte defaultColor, byte activeColor) {
	Item item;
	item.enabled = true;
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
	for (Common::Array<Item>::iterator iter = _items.begin(); iter != _items.end(); ++iter) {
		if ((*iter).enabled && (*iter).rect.contains(x, y - _top))
			return (*iter).id;
	}
	return kItemIdNone;
}

MenuSystem::Item *MenuSystem::getItem(ItemID id) {
	for (Common::Array<Item>::iterator iter = _items.begin(); iter != _items.end(); ++iter) {
		if ((*iter).id == id)
			return &(*iter);
	}
	return NULL;
}

void MenuSystem::setItemCaption(Item *item, const char *caption) {
	Font font(_vm->_res->load(_vm->_screen->getFontResIndex(item->fontNum))->data);
	int width = font.getTextWidth((const byte*)caption);
	int height = font.getHeight();
	if (width & 1)
		width++;
	item->rect = Common::Rect(item->x, item->y - height, item->x + width, item->y);
	if (item->w) {
		item->rect.translate(item->w - width / 2, 0);
	}
	item->caption = caption;
}

void MenuSystem::initMenu(MenuID menuID) {
	_items.clear();

	memcpy(_vm->_screen->_frontScreen, _background->getPixels(), 640 * 400);

	switch (menuID) {
	case kMenuIdMain:
		drawString(0, 75, 320, 1, 229, _vm->getSysString(kStrWhatCanIDoForYou));
		addClickTextItem(kItemIdLoad, 0, 116, 320, 0, _vm->getSysString(kStrLoad), 253, 255);
		addClickTextItem(kItemIdSave, 0, 136, 320, 0, _vm->getSysString(kStrSave), 253, 255);
		addClickTextItem(kItemIdToggleText, 0, 166, 320, 0, _vm->getSysString(_vm->_cfgText ? kStrTextOn : kStrTextOff), 253, 255);
		addClickTextItem(kItemIdToggleVoices, 0, 186, 320, 0, _vm->getSysString(_vm->_cfgVoices ? kStrVoicesOn : kStrVoicesOff), 253, 255);
		addClickTextItem(kItemIdVolumesMenu, 0, 216, 320, 0, _vm->getSysString(kStrVolume), 253, 255);
		addClickTextItem(kItemIdPlay, 0, 246, 320, 0, _vm->getSysString(kStrPlay), 253, 255);
		addClickTextItem(kItemIdQuit, 0, 276, 320, 0, _vm->getSysString(kStrQuit), 253, 255);
		break;
	case kMenuIdLoad:
		if (ConfMan.getBool("originalsaveload")) {
			shadeRect(80, 92, 440, 141, 226, 225);
			drawString(0, 75, 320, 1, 229, _vm->getSysString(kStrLoadGame));
			addClickTextItem(kItemIdSavegameUp, 0, 156, 545, 1, "^", 253, 255);
			addClickTextItem(kItemIdSavegameDown, 0, 196, 545, 1, "\\", 253, 255);
			addClickTextItem(kItemIdCancel, 0, 276, 320, 0, _vm->getSysString(kStrCancel), 253, 255);
			for (int i = 1; i <= 7; i++) {
				Common::String saveDesc = Common::String::format("SAVEGAME %d", i);
				addClickTextItem((ItemID)(kItemIdSavegame1 + i - 1), 0, 116 + 20 * (i - 1), 300, 0, saveDesc.c_str(), 231, 234);
			}
			loadSavegamesList();
			setSavegameCaptions(true);
		} else {
			GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
			int slot = dialog->runModalWithCurrentTarget();
			delete dialog;

			if (slot >= 0)
				_vm->requestLoadgame(slot);

			_running = false;
		}
		break;
	case kMenuIdSave:
		if (ConfMan.getBool("originalsaveload")) {
			shadeRect(80, 92, 440, 141, 226, 225);
			drawString(0, 75, 320, 1, 229, _vm->getSysString(kStrSaveGame));
			addClickTextItem(kItemIdSavegameUp, 0, 156, 545, 1, "^", 253, 255);
			addClickTextItem(kItemIdSavegameDown, 0, 196, 545, 1, "\\", 253, 255);
			addClickTextItem(kItemIdCancel, 0, 276, 320, 0, _vm->getSysString(kStrCancel), 253, 255);
			for (int i = 1; i <= 7; i++) {
				Common::String saveDesc = Common::String::format("SAVEGAME %d", i);
				addClickTextItem((ItemID)(kItemIdSavegame1 + i - 1), 0, 116 + 20 * (i - 1), 300, 0, saveDesc.c_str(), 231, 234);
			}
			int newSlotNum = loadSavegamesList() + 1;
			_savegames.push_back(SavegameItem(newSlotNum, Common::String::format("GAME %04d", _savegames.size())));
			setSavegameCaptions(true);
		} else {
			GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
			int slot = dialog->runModalWithCurrentTarget();
			Common::String desc = dialog->getResultString();
			if (desc.empty()) {
				// Create our own description for the saved game, the user didn't enter one
				desc = dialog->createDefaultSaveDescription(slot);
			}

			if (slot >= 0)
				_vm->requestSavegame(slot, desc);

			_running = false;
		}
		break;
	case kMenuIdVolumes:
		drawString(0, 75, 320, 1, 229, _vm->getSysString(kStrAdjustVolume));
		drawString(0, 131, 200, 0, 246, _vm->getSysString(kStrMaster));
		drawString(0, 156, 200, 0, 244, _vm->getSysString(kStrVoices));
		drawString(0, 181, 200, 0, 244, _vm->getSysString(kStrMusic));
		drawString(0, 206, 200, 0, 244, _vm->getSysString(kStrSoundFx));
		drawString(0, 231, 200, 0, 244, _vm->getSysString(kStrBackground));
		addClickTextItem(kItemIdDone, 0, 276, 200, 0, _vm->getSysString(kStrDone), 253, 255);
		addClickTextItem(kItemIdCancel, 0, 276, 440, 0, _vm->getSysString(kStrCancel), 253, 255);
		addClickTextItem(kItemIdMasterDown, 0, 131 + 25 * 0, 348, 1, "[", 243, 246);
		addClickTextItem(kItemIdVoicesDown, 0, 131 + 25 * 1, 348, 1, "[", 243, 246);
		addClickTextItem(kItemIdMusicDown, 0, 131 + 25 * 2, 348, 1, "[", 243, 246);
		addClickTextItem(kItemIdSoundFXDown, 0, 131 + 25 * 3, 348, 1, "[", 243, 246);
		addClickTextItem(kItemIdBackgroundDown, 0, 131 + 25 * 4, 348, 1, "[", 243, 246);
		addClickTextItem(kItemIdMasterUp, 0, 131 + 25 * 0, 372, 1, "]", 243, 246);
		addClickTextItem(kItemIdVoicesUp, 0, 131 + 25 * 1, 372, 1, "]", 243, 246);
		addClickTextItem(kItemIdMusicUp, 0, 131 + 25 * 2, 372, 1, "]", 243, 246);
		addClickTextItem(kItemIdSoundFXUp, 0, 131 + 25 * 3, 372, 1, "]", 243, 246);
		addClickTextItem(kItemIdBackgroundUp, 0, 131 + 25 * 4, 372, 1, "]", 243, 246);
		drawVolumeBar(kItemIdMaster);
		drawVolumeBar(kItemIdVoices);
		drawVolumeBar(kItemIdMusic);
		drawVolumeBar(kItemIdSoundFX);
		drawVolumeBar(kItemIdBackground);
		break;
	default:
		break;
	}

	for (Common::Array<Item>::iterator iter = _items.begin(); iter != _items.end(); ++iter) {
		if ((*iter).enabled)
			drawItem((*iter).id, false);
	}

	// Check if the mouse is already over an item
	_currItemID = kItemIdNone;
	Common::Point mousePos = _vm->_system->getEventManager()->getMousePos();
	handleMouseMove(mousePos.x, mousePos.y);
}

void MenuSystem::enableItem(ItemID id) {
	Item *item = getItem(id);
	if (item) {
		item->enabled = true;
		drawItem(id, false);
		_currItemID = kItemIdNone;
		Common::Point mousePos = _vm->_system->getEventManager()->getMousePos();
		handleMouseMove(mousePos.x, mousePos.y);
	}
}

void MenuSystem::disableItem(ItemID id) {
	Item *item = getItem(id);
	if (item) {
		item->enabled = false;
		restoreRect(item->rect.left, item->rect.top, item->rect.width(), item->rect.height());
		if (_currItemID == id) {
			_currItemID = kItemIdNone;
		}
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
		setCfgText(!_vm->_cfgText, true);
		if (!_vm->_cfgVoices && !_vm->_cfgText)
			setCfgVoices(true, false);
		break;
	case kItemIdToggleVoices:
		setCfgVoices(!_vm->_cfgVoices, true);
		if (!_vm->_cfgVoices && !_vm->_cfgText)
			setCfgText(true, false);
		break;
	case kItemIdVolumesMenu:
		//debug("kItemIdVolumesMenu");
		_newMenuID = kMenuIdVolumes;
		break;
	case kItemIdPlay:
		//debug("kItemIdPlay");
		_running = false;
		break;
	case kItemIdQuit:
		_running = false;
		_vm->quitGame();
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
	byte *src = (byte *)_background->getBasePtr(x, y);
	byte *dst = _vm->_screen->_frontScreen + x + y * 640;
	while (h--) {
		memcpy(dst, src, w);
		src += 640;
		dst += 640;
	}
}

void MenuSystem::shadeRect(int x, int y, int w, int h, byte color1, byte color2) {
	byte *src = (byte *)_vm->_screen->_frontScreen + x + y * 640;
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
		int width = font.getTextWidth((const byte*)text);
		if (width & 1)
			width++;
		x = x + w - width / 2;
	}
	_vm->_screen->drawString(x, y - font.getHeight(), color, fontNum, (const byte*)text, -1, NULL, true);
	_needRedraw = true;
}

int MenuSystem::loadSavegamesList() {
	int maxSlotNum = -1;

	_savegameListTopIndex = 0;
	_savegames.clear();

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Toltecs::ToltecsEngine::SaveHeader header;
	Common::String pattern = _vm->getTargetName();
	pattern += ".???";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);
		if (slotNum > maxSlotNum)
			maxSlotNum = slotNum;

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Toltecs::ToltecsEngine::readSaveHeader(in, header) == Toltecs::ToltecsEngine::kRSHENoError) {
					_savegames.push_back(SavegameItem(slotNum, header.description));
					//debug("%s -> %s", file->c_str(), header.description.c_str());
				}
				delete in;
			}
		}
	}

	return maxSlotNum;
}

MenuSystem::SavegameItem *MenuSystem::getSavegameItemByID(ItemID id) {
	if (id >= kItemIdSavegame1 && id <= kItemIdSavegame7)
		return &_savegames[_savegameListTopIndex + id - kItemIdSavegame1];
	else
		return NULL;
}

void MenuSystem::setSavegameCaptions(bool scrollToBottom) {
	int size = _savegames.size();
	if (scrollToBottom && size > 0) {
		while (_savegameListTopIndex + 7 <= size)
			_savegameListTopIndex += 6;
	}
	int index = _savegameListTopIndex;
	for (int i = 1; i <= 7; i++)
		setItemCaption(getItem((ItemID)(kItemIdSavegame1 + i - 1)), index < size ? _savegames[index++]._description.c_str() : "");
	if (_savegameListTopIndex == 0) {
		disableItem(kItemIdSavegameUp);
	} else {
		enableItem(kItemIdSavegameUp);
	}
	if (_savegameListTopIndex + 7 > size) {
		disableItem(kItemIdSavegameDown);
	} else {
		enableItem(kItemIdSavegameDown);
	}
}

void MenuSystem::scrollSavegames(int delta) {
	int newPos = CLIP<int>(_savegameListTopIndex + delta, 0, _savegames.size() - 1);
	_savegameListTopIndex = newPos;
	restoreRect(80, 92, 440, 140);
	setSavegameCaptions(false);
	for (int i = 1; i <= 7; i++)
		drawItem((ItemID)(kItemIdSavegame1 + i - 1), false);
}

void MenuSystem::clickSavegameItem(ItemID id) {
	if (_currMenuID == kMenuIdLoad) {
		SavegameItem *savegameItem = getSavegameItemByID(id);
		//debug("slotNum = [%d]; description = [%s]", savegameItem->_slotNum, savegameItem->_description.c_str());
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
	if (_vm->_cfgText != value) {
		Item *item = getItem(kItemIdToggleText);
		_vm->_cfgText = value;
		restoreRect(item->rect.left, item->rect.top, item->rect.width() + 1, item->rect.height() - 2);
		setItemCaption(item, _vm->getSysString(_vm->_cfgText ? kStrTextOn : kStrTextOff));
		drawItem(kItemIdToggleText, true);
		ConfMan.setBool("subtitles", value);
	}
}

void MenuSystem::setCfgVoices(bool value, bool active) {
	if (_vm->_cfgVoices != value) {
		Item *item = getItem(kItemIdToggleVoices);
		_vm->_cfgVoices = value;
		restoreRect(item->rect.left, item->rect.top, item->rect.width() + 1, item->rect.height() - 2);
		setItemCaption(item, _vm->getSysString(_vm->_cfgVoices ? kStrVoicesOn : kStrVoicesOff));
		drawItem(kItemIdToggleVoices, true);
		ConfMan.setBool("speech_mute", !value);
	}
}

void MenuSystem::drawVolumeBar(ItemID itemID) {
	int w = 440, y, volume;
	char text[21];

	switch (itemID) {
	case kItemIdMaster:	// unused in ScummVM, always 20
		y = 130 + 25 * 0;
		volume = 20;
		break;
	case kItemIdVoices:
		y = 130 + 25 * 1;
		volume = _vm->_cfgVoicesVolume;
		break;
	case kItemIdMusic:
		y = 130 + 25 * 2;
		volume = _vm->_cfgMusicVolume;
		break;
	case kItemIdSoundFX:
		y = 130 + 25 * 3;
		volume = _vm->_cfgSoundFXVolume;
		break;
	case kItemIdBackground:	// unused in ScummVM, always 20
		y = 130 + 25 * 4;
		volume = 20;
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
	byte newVolume;

	switch (itemID) {
	case kItemIdVoices:
		_vm->_cfgVoicesVolume = CLIP(_vm->_cfgVoicesVolume + delta, 0, 20);
		// Always round volume up instead of down.
		newVolume = (_vm->_cfgVoicesVolume * Audio::Mixer::kMaxChannelVolume + 19) / 20;
		_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, newVolume);
		ConfMan.setInt("speech_volume", newVolume);
		break;
	case kItemIdMusic:
		_vm->_cfgMusicVolume = CLIP(_vm->_cfgMusicVolume + delta, 0, 20);
		newVolume = (_vm->_cfgMusicVolume * Audio::Mixer::kMaxChannelVolume + 19) / 20;
		_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, newVolume);
		ConfMan.setInt("music_volume", newVolume);
		break;
	case kItemIdSoundFX:
		_vm->_cfgSoundFXVolume = CLIP(_vm->_cfgSoundFXVolume + delta, 0, 20);
		newVolume = (_vm->_cfgSoundFXVolume * Audio::Mixer::kMaxChannelVolume + 19) / 20;
		_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, newVolume);
		ConfMan.setInt("sfx_volume", newVolume);
		break;
	case kItemIdMaster:
	case kItemIdBackground:
		// unused in ScummVM
		break;
	default:
		return;
	}

	_vm->syncSoundSettings();
	drawVolumeBar(itemID);
}

} // End of namespace Toltecs
