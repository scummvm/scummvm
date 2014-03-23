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

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/compression.h"
#include "mads/user_interface.h"

namespace MADS {

UISlot::UISlot() {
	_slotType = ST_NONE;
	_field2 = 0;
	_spritesIndex = 0;
	_frameNumber = 0;
}

/*------------------------------------------------------------------------*/

void UISlots::fullRefresh() {
	UISlot slot;
	slot._slotType = ST_FULL_SCREEN_REFRESH;
	slot._field2 = -1;

	push_back(slot);
}

void UISlots::add(const Common::Point &pt, int frameNumber, int spritesIndex) {
	assert(size() < 50);

	UISlot ie;
	ie._slotType = -3;
	ie._field2 = 201;
	ie._position = pt;
	ie._frameNumber = frameNumber;
	ie._spritesIndex = spritesIndex;

	push_back(ie);
}

void UISlots::draw(bool updateFlag, bool delFlag) {
	Scene &scene = _vm->_game->_scene;
	UserInterface &userInterface = scene._userInterface;
	DirtyArea *dirtyAreaPtr = nullptr;

	// Loop through setting up the dirty areas
	for (uint idx = 0; idx < size(); ++idx) {
		DirtyArea &dirtyArea = userInterface._dirtyAreas[idx];
		UISlot &slot = (*this)[idx];

		if (slot._slotType >= ST_NONE) {
			dirtyArea._active = false;
		} else {
			dirtyArea.setUISlot(&slot);
			dirtyArea._textActive = true;
			if (slot._field2 == 200 && slot._slotType == ST_MINUS5) {
				dirtyArea._active = false;
				dirtyAreaPtr = &dirtyArea;
			}
		}
	}

	userInterface._dirtyAreas.merge(1, userInterface._uiSlots.size());
	if (dirtyAreaPtr)
		dirtyAreaPtr->_active = true;

	// Copy parts of the user interface background that are going to have sprites drawn
	for (uint idx = 0; idx < size(); ++idx) {
		DirtyArea &dirtyArea = userInterface._dirtyAreas[idx];
		UISlot &slot = (*this)[idx];

		if (dirtyArea._active && dirtyArea._bounds.width() > 0
				&& dirtyArea._bounds.height() > 0 && slot._slotType >= -20) {

			// TODO: Figure out the difference between two copy methods used
			if (slot._slotType >= ST_EXPIRED) {
				userInterface._surface.copyTo(&userInterface, dirtyArea._bounds,
					Common::Point(dirtyArea._bounds.left, dirtyArea._bounds.top));
			} else {
				userInterface._surface.copyTo(&userInterface, dirtyArea._bounds,
					Common::Point(dirtyArea._bounds.left, dirtyArea._bounds.top));
			}
		}
	}

	for (uint idx = 0; idx < size(); ++idx) {
		DirtyArea &dirtyArea = userInterface._dirtyAreas[idx];
		UISlot &slot = (*this)[idx];

		int slotType = slot._slotType;
		if (slotType >= ST_NONE) {
			dirtyArea.setUISlot(&slot);
			if (!updateFlag)
				slotType &= ~0x40;

			dirtyArea._textActive = slotType > 0;
			slot._slotType &= 0x40;
		}
	}

	userInterface._dirtyAreas.merge(1, userInterface._uiSlots.size());

	for (uint idx = 0; idx < size(); ++idx) {
		DirtyArea &dirtyArea = userInterface._dirtyAreas[idx];
		UISlot &slot = (*this)[idx];

		if (slot._slotType >= ST_NONE && !(slot._slotType & 0x40)) {
			if (!dirtyArea._active) {
				error("Should never reach this point, even in original");
			}

			if (dirtyArea._textActive) {
				SpriteAsset *asset = scene._sprites[slot._spritesIndex];
				
				if (slot._field2 == 200) {
					MSprite *sprite = asset->getFrame(slot._frameNumber & 0x7F);
					sprite->copyTo(&userInterface, slot._position, 
						sprite->getTransparencyIndex());
				} else {
					MSprite *sprite = asset->getFrame(slot._frameNumber - 1);
					sprite->copyTo(&userInterface, slot._position,
						sprite->getTransparencyIndex());
				}
			}
		}
	}

	// Mark areas of the screen surface for updating
	if (updateFlag) {
		_vm->_screen.setPointer(&userInterface);
		userInterface.setBounds(Common::Rect(0, scene._interfaceY,
			MADS_SCREEN_WIDTH - 1, userInterface.h + scene._interfaceY - 1));
		warning("TODO: sub_111C8 / sub_1146C");

		for (uint idx = 0; idx < size(); ++idx) {
			DirtyArea &dirtyArea = userInterface._dirtyAreas[idx];

			if (dirtyArea._active && dirtyArea._textActive &&
				dirtyArea._bounds.width() > 0 && dirtyArea._bounds.height() > 0) {
				// Flag area of screen as needing update
				Common::Rect r = dirtyArea._bounds;
				r.translate(0, scene._interfaceY);
				_vm->_screen.copyRectToScreen(r);
			}
		}

		warning("TODO: sub 115A2 / sub111D3");
	}

	// Post-processing to remove slots no longer needed
	for (int idx = (int)size() - 1; idx >= 0; --idx) {
		UISlot &slot = (*this)[idx];

		if (slot._slotType < ST_NONE) {
			if (delFlag || updateFlag)
				remove_at(idx);
			else if (slot._slotType >= -20)
				slot._slotType -= 20;
		} else {
			if (updateFlag)
				slot._slotType &= ~0x40;
			else
				slot._slotType |= 0x40;
		}
	}
}

/*------------------------------------------------------------------------*/

UserInterface::UserInterface(MADSEngine *vm) : _vm(vm), _dirtyAreas(vm), 
		_uiSlots(vm) {
	_invSpritesIndex = -1;
	_invFrameNumber = 1;
	_scrollMilli = 0;
	_scrollFlag = false;
	_category = CAT_NONE;
	_inventoryTopIndex = 0;
	_objectY = 0;
	_selectedInvIndex = -1;
	_selectedActionIndex = -1;
	_selectedItemVocabIdx = -1;
	_scrollerY = 0;
	_v1A = -1;
	_v1C = -1;
	_v1E = -1;
	_dirtyAreas.resize(50);
	_inventoryChanged = false;
	Common::fill(&_categoryIndexes[0], &_categoryIndexes[7], 0);

	// Map the user interface to the bottom of the game's screen surface
	byte *pData = _vm->_screen.getBasePtr(0, MADS_SCENE_HEIGHT);
	setPixels(pData, MADS_SCREEN_WIDTH, MADS_INTERFACE_HEIGHT);

	_surface.setSize(MADS_SCREEN_WIDTH, MADS_INTERFACE_HEIGHT);
}

void UserInterface::load(const Common::String &resName) {
	File f(resName);
	MadsPack madsPack(&f);

	// Load in the palette
	Common::SeekableReadStream *palStream = madsPack.getItemStream(0);

	uint32 *gamePalP = &_vm->_palette->_palFlags[0];
	byte *palP = &_vm->_palette->_mainPalette[0];

	for (int i = 0; i < 16; ++i, gamePalP++, palP += 3) {
		palP[0] = VGA_COLOR_TRANS(palStream->readByte());
		palP[1] = VGA_COLOR_TRANS(palStream->readByte());
		palP[2] = VGA_COLOR_TRANS(palStream->readByte());
		*gamePalP |= 1;
		palStream->skip(3);
	}
	delete palStream;

	// Read in the surface data
	Common::SeekableReadStream *pixelsStream = madsPack.getItemStream(1);
	pixelsStream->read(_surface.getData(), MADS_SCREEN_WIDTH * MADS_INTERFACE_HEIGHT);
	delete pixelsStream;
}

void UserInterface::setup(int id) {
	Scene &scene = _vm->_game->_scene;

	if (_vm->_game->_screenObjects._v832EC != id) {
		Common::String resName = _vm->_game->_aaName;

		// Strip off any extension
		const char *p = strchr(resName.c_str(), '.');
		if (p) {
			resName = Common::String(resName.c_str(), p);
		}

		// Add on suffix if necessary
		if (id)
			resName += "A";

		resName += ".INT";

		load(resName);
		_surface.copyTo(this);
	}
	_vm->_game->_screenObjects._v832EC = id;

	scene._userInterface._uiSlots.clear();
	scene._userInterface._uiSlots.fullRefresh();
	_vm->_game->_ticksExpiry = _vm->_events->getFrameCounter();
	_v1A = -1;
	_v1E = -1;
	_v1C = -1;

	if (_vm->_game->_v1 == 5)
		scene._userInterface._uiSlots.draw(false, false);

	scene._action.clear();
	drawTextElements();
	loadElements();
	scene._dynamicHotspots.refresh();
}

void UserInterface::drawTextElements() {
	if (_vm->_game->_screenObjects._v832EC) {
		drawTalkList();
	} else {
		// Draw the actions
		drawActions();
		drawInventoryList();
		drawItemVocabList();
	}
}

void UserInterface::drawActions() {
	for (int idx = 0; idx < 10; ++idx) {
		writeVocab(CAT_ACTION, idx);
	}
}

void UserInterface::drawInventoryList() {
	int endIndex = MIN((int)_vm->_game->_objects._inventoryList.size(), _inventoryTopIndex + 5);
	for (int idx = _inventoryTopIndex; idx < endIndex; ++idx) {
		writeVocab(CAT_INV_LIST, idx);
	}
}

void UserInterface::drawItemVocabList() {
	if (_selectedInvIndex >= 0) {
		InventoryObject &io = _vm->_game->_objects[
			_vm->_game->_objects._inventoryList[_selectedInvIndex]];
		for (int idx = 0; idx < io._vocabCount; ++idx) {
			writeVocab(CAT_INV_VOCAB, idx);
		}
	}
}

void UserInterface::drawScrolller() {
	if (_scrollerY > 0)
		writeVocab(CAT_INV_SCROLLER, _scrollerY);
	writeVocab(CAT_INV_SCROLLER, 4);
}

void UserInterface::writeVocab(ScrCategory category, int id) {
	Common::Rect bounds;
	if (!getBounds(category, id, bounds))
		return;

	Scene &scene = _vm->_game->_scene;
	Font *font = nullptr;

	int vocabId;
	Common::String vocabStr;
	switch (category) {
	case CAT_ACTION:
		font = _vm->_font->getFont(FONT_INTERFACE);
		vocabId = scene._verbList[id]._id;
		if (_v1A) {
			_vm->_font->setColorMode(1);
		} else {
			_vm->_font->setColorMode(id == _selectedActionIndex ? 2 : 0);
		}
		vocabStr = scene.getVocab(vocabId);
		vocabStr.setChar(toupper(vocabStr[0]), 0);
		font->writeString(this, vocabStr, Common::Point(bounds.left, bounds.top));
		break;

	case CAT_INV_LIST:
		font = _vm->_font->getFont(FONT_INTERFACE);
		vocabId = _vm->_game->_objects.getItem(id)._descId;
		if (_v1C == id) {
			_vm->_font->setColorMode(1);
		} else {
			_vm->_font->setColorMode(id == _selectedInvIndex ? 2 : 0);
			vocabStr = scene.getVocab(vocabId);
			vocabStr.setChar(toupper(vocabStr[0]), 0);
			font->writeString(this, vocabStr, Common::Point(bounds.left, bounds.top));
			break;
		}
		break;

	case CAT_TALK_ENTRY:
		error("TODO: CAT_TALK_ENTRY");

	case CAT_INV_SCROLLER:
		font = _vm->_font->getFont(FONT_MISC);

		switch (id) {
		case 1:
			vocabStr = "a";
			break;
		case 2:
			vocabStr = "b";
			break;
		case 3:
			vocabStr = "d";
			break;
		case 4:
			vocabStr = "c";
			break;
		default:
			break;
		}

		font->setColorMode((id == 4) || (_scrollerY == 3) ? 1 : 0);
		font->writeString(this, vocabStr, Common::Point(bounds.left, bounds.top));
		break;
	default:
		font = _vm->_font->getFont(FONT_INTERFACE);
		vocabId = _vm->_game->_objects.getItem(_selectedInvIndex)._vocabList[id]._vocabId;
		if (_v1E == id) {
			_vm->_font->setColorMode(1);
		} else {
			_vm->_font->setColorMode(id == _selectedInvIndex ? 2 : 0);
			vocabStr = scene.getVocab(vocabId);
			vocabStr.setChar(toupper(vocabStr[0]), 0);
			font->writeString(this, vocabStr, Common::Point(bounds.left, bounds.top));
			break;
		}
		break;
	}
}

void UserInterface::setBounds(const Common::Rect &r) {
	_drawBounds = r;
}

void UserInterface::loadElements() {
	Scene &scene = _vm->_game->_scene;
	Common::Rect bounds;
	_vm->_game->_screenObjects.clear();

	if (!_vm->_game->_screenObjects._v832EC) {
		// Set up screen objects for the inventory scroller
		for (int idx = 1; idx <= 3; ++idx) {
			getBounds(CAT_INV_SCROLLER, idx, bounds);
			moveRect(bounds);

			_vm->_game->_screenObjects.add(bounds, LAYER_GUI, CAT_INV_SCROLLER, idx);
		}

		// Set up actions
		_categoryIndexes[CAT_ACTION - 1] = _vm->_game->_screenObjects.size() + 1;
		for (int idx = 0; idx < 10; ++idx) {
			getBounds(CAT_ACTION, idx, bounds);
			moveRect(bounds);

			_vm->_game->_screenObjects.add(bounds, LAYER_GUI, CAT_ACTION, idx);
		}

		// Set up inventory list
		_categoryIndexes[CAT_INV_LIST - 1] = _vm->_game->_screenObjects.size() + 1;
		for (int idx = 0; idx < 5; ++idx) {
			getBounds(CAT_INV_LIST, idx, bounds);
			moveRect(bounds);

			_vm->_game->_screenObjects.add(bounds, LAYER_GUI, CAT_INV_LIST, idx);
		}

		// Set up the inventory vocab list
		_categoryIndexes[CAT_INV_VOCAB - 1] = _vm->_game->_screenObjects.size() + 1;
		for (int idx = 0; idx < 5; ++idx) {
			getBounds(CAT_INV_VOCAB, idx, bounds);
			moveRect(bounds);

			_vm->_game->_screenObjects.add(bounds, LAYER_GUI, CAT_INV_VOCAB, idx);
		}

		// Set up the inventory item picture
		_categoryIndexes[CAT_INV_ANIM - 1] = _vm->_game->_screenObjects.size() + 1;
		_vm->_game->_screenObjects.add(Common::Rect(160, 159, 231, 194), LAYER_GUI,
			CAT_INV_ANIM, 0);
	}

	if (!_vm->_game->_screenObjects._v832EC || _vm->_game->_screenObjects._v832EC == 2) {
		_categoryIndexes[CAT_HOTSPOT - 1] = _vm->_game->_screenObjects.size() + 1;
		for (int hotspotIdx = scene._hotspots.size() - 1; hotspotIdx >= 0; --hotspotIdx) {
			Hotspot &hs = scene._hotspots[hotspotIdx];
			_vm->_game->_screenObjects.add(hs._bounds, LAYER_GUI, CAT_HOTSPOT, hotspotIdx);
		}
	}

	if (_vm->_game->_screenObjects._v832EC == 1) {
		// setup areas for talk entries
		_categoryIndexes[CAT_TALK_ENTRY - 1] = _vm->_game->_screenObjects.size() + 1;
		for (int idx = 0; idx < 5; ++idx) {
			getBounds(CAT_TALK_ENTRY, idx, bounds);
			moveRect(bounds);

			_vm->_game->_screenObjects.add(bounds, LAYER_GUI, CAT_TALK_ENTRY, idx);
		}
	}

	// Store the number of UI elements loaded for easy nuking/refreshing hotspots added later
	_vm->_game->_screenObjects._uiCount = _vm->_game->_screenObjects.size();
}

bool UserInterface::getBounds(ScrCategory category, int v, Common::Rect &bounds) {
	int heightMultiplier, widthMultiplier;
	int leftStart, yOffset, widthAmt;

	switch (category) {
	case CAT_ACTION:
		heightMultiplier = v % 5;
		widthMultiplier = v / 5;
		leftStart = 2;
		yOffset = 3;
		widthAmt = 32;
		break;

	case CAT_INV_LIST:
		if (v < _inventoryTopIndex || v > (_inventoryTopIndex + 5))
			return false;

		heightMultiplier = v - _inventoryTopIndex;
		widthMultiplier = 0;
		leftStart = 90;
		yOffset = 3;
		widthAmt = 69;
		break;

	case CAT_TALK_ENTRY:
		heightMultiplier = v;
		widthMultiplier = 0;
		leftStart = 2;
		yOffset = 3;
		widthAmt = 310;
		break;

	case CAT_INV_SCROLLER:
		heightMultiplier = 0;
		widthMultiplier = 73;
		yOffset = 0;
		widthAmt = 9;
		leftStart = (v != 73) ? 73 : 75;
		break;

	default:
		heightMultiplier = v;
		widthMultiplier = 0;
		leftStart = 240;
		yOffset = 3;
		widthAmt = 80;
		break;
	}

	bounds.left = (widthMultiplier > 0) ? widthMultiplier * widthAmt + leftStart : leftStart;
	bounds.setWidth(widthAmt);
	bounds.top = heightMultiplier * 8 + yOffset;
	bounds.setHeight(8);

	if (category == CAT_INV_SCROLLER) {
		switch (v) {
		case 1:
			// Arrow up
			bounds.top = 4;
			bounds.setHeight(7);
			break;
		case 2:
			// Arrow down
			bounds.top = 35;
			bounds.setHeight(7);
			break;
		case 3:
			// Scroller
			bounds.top = 12;
			bounds.setHeight(22);
			break;
		case 4:
			// Thumb
			bounds.top = _objectY + 14;
			bounds.setHeight(1);
			break;
		default:
			break;
		}
	}

	return true;
}

void UserInterface::moveRect(Common::Rect &bounds) {
	bounds.translate(0, MADS_SCENE_HEIGHT);
}

void UserInterface::drawTalkList() {
	warning("TODO: drawTalkList");
}

void UserInterface::loadInventoryAnim(int objectId) {
	Scene &scene = _vm->_game->_scene;
	noInventoryAnim();
	bool flag = true;

	if (_vm->_invObjectsAnimated) {
		Common::String resName = Common::String::format("*OB%.3dI", objectId);
		SpriteAsset *asset = new SpriteAsset(_vm, resName, 8);
		_invSpritesIndex = scene._sprites.add(asset, 1);
		if (_invSpritesIndex >= 0) {
			_invFrameNumber = 1;
			flag = false;
		}
	}

	if (flag) {
		// TODO: Use of inv_object_data?
	}
}

void UserInterface::noInventoryAnim() {
	Scene &scene = _vm->_game->_scene;

	if (_invSpritesIndex >= 0) {
		scene._sprites.remove(_invSpritesIndex);
		_vm->_game->_ticksExpiry = _vm->_events->getFrameCounter();
		_invSpritesIndex = -1;
	}

	if (!_vm->_game->_screenObjects._v832EC)
		refresh();
}

void UserInterface::refresh() {
	_uiSlots.clear();
	_uiSlots.fullRefresh();
	_uiSlots.draw(false, false);

	drawTextElements();
}

void UserInterface::inventoryAnim() {
	Scene &scene = _vm->_game->_scene;
	if (_vm->_game->_screenObjects._v832EC == 1 || _vm->_game->_screenObjects._v832EC == 2
			|| _invSpritesIndex < 0)
		return;

	// Move to the next frame number in the sequence, resetting if at the end
	SpriteAsset *asset = scene._sprites[_invSpritesIndex];
	if (++_invFrameNumber > asset->getCount())
		_invFrameNumber = 1;

	// Loop through the slots list for inventory animation entry
	for (uint i = 0; i < _uiSlots.size(); ++i) {
		if (_uiSlots[i]._field2 == 200)
			_uiSlots[i]._slotType = -5;
	}

	// Add a new slot entry for the inventory animation
	UISlot slot;
	slot._slotType = ST_FOREGROUND;
	slot._field2 = 200;
	slot._frameNumber = _invFrameNumber;
	slot._spritesIndex = _invSpritesIndex;
	slot._position = Common::Point(160, 3);

	_uiSlots.push_back(slot);
}

void UserInterface::categoryChanged() {
	_v1C = -1;
	_vm->_events->initVars();
	_category = CAT_NONE;
}

void UserInterface::selectObject(int invIndex) {
	warning("TODO: selectObject");
}

void UserInterface::drawInventory(int v1, int v2, int *v3) {
	warning("TODO: drawInventory");
}

void UserInterface::scrollerChanged() {
	warning("TODO: scrollerChanged");
}

void UserInterface::scrollInventory() {
	Common::Array<int> &invList = _vm->_game->_objects._inventoryList;

	if (_vm->_events->_mouseButtons) {
		int yp = _vm->_events->currentPos().y;
		if (yp < MADS_SCENE_HEIGHT || yp == (MADS_SCREEN_HEIGHT - 1)) {
			uint32 timeDiff = _scrollFlag ? 100 : 380;
			uint32 currentMilli = g_system->getMillis();
			_vm->_game->_screenObjects._v8332A = -1;

			if (currentMilli >= (_scrollMilli + timeDiff)) {
				_scrollMilli = currentMilli;
				_scrollFlag = true;

				if (yp == (MADS_SCREEN_HEIGHT - 1)) {
					if (_inventoryTopIndex < (invList.size() - 1)) {
						++_inventoryTopIndex;
						_inventoryChanged = true;
					}
				} else {
					if (_inventoryTopIndex > 0) {
						--_inventoryTopIndex;
						_inventoryChanged = true;
					}
				}
			}
		}
	}

	_vm->_game->_screenObjects._v8332A = 0;
}

} // End of namespace MADS
