/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "buried/avi_frames.h"
#include "buried/biochip_right.h"
#include "buried/buried.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/inventory_info.h"
#include "buried/inventory_window.h"
#include "buried/message.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/environ/scene_base.h"

#include "common/algorithm.h"
#include "common/stream.h"
#include "graphics/font.h"
#include "graphics/surface.h"

namespace Buried {

InventoryWindow::InventoryWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_background = nullptr;
	_magSelected = false;
	_upSelected = false;
	_downSelected = false;
	_textSelected = -1;
	_itemComesFromInventory = false;
	_draggingObject = false;
	_draggingItemID = -1;
	_draggingItemSpriteData.image = nullptr;
	_draggingIconIndex = 0;
	_draggingItemInInventory = false;

	if (_vm->isDemo()) {
		// Demo gets its own items
		_itemArray.push_back(kItemBioChipInterface);
		_itemArray.push_back(kItemGrapplingHook);
	} else {
		// Default items
		_itemArray.push_back(kItemBioChipBlank);
		_itemArray.push_back(kItemBioChipCloak);
		_itemArray.push_back(kItemBioChipEvidence);
		_itemArray.push_back(kItemBioChipFiles);
		_itemArray.push_back(kItemBioChipInterface);
		_itemArray.push_back(kItemBioChipJump);
	}

	setCurItem(0);

	_infoWindow = nullptr;
	_letterViewWindow = nullptr;

	_scrollTimer = 0;

	rebuildPreBuffer();

	_fontHeight = (_vm->getLanguage() == Common::JA_JPN) ? 10 : 14;
	_textFont = _vm->_gfx->createFont(_fontHeight);

	_rect = Common::Rect(182, 375, 450, 454);
	_curCursor = (int)kCursorNone;

	if (_vm->isDemo()) {
		// The demo uses a video for drag frames
		Common::String dragFramesFileName;
		if (_vm->isTrueColor())
			dragFramesFileName = "COMMON/INVDRAG.BTV";
		else
			dragFramesFileName = "COMMON/INVDRAG8.BTV";

		_dragFrames = new AVIFrames(dragFramesFileName);
	} else {
		// The full version uses bitmaps
		_dragFrames = nullptr;
	}
}

InventoryWindow::~InventoryWindow() {
	destroyInfoWindow();
	destroyBurnedLetterWindow();

	if (_background) {
		_background->free();
		delete _background;
	}

	if (_draggingItemSpriteData.image) {
		_draggingItemSpriteData.image->free();
		delete _draggingItemSpriteData.image;
	}

	if (_scrollTimer != 0)
		killTimer(_scrollTimer);

	delete _textFont;
	delete _dragFrames;
}

bool InventoryWindow::rebuildPreBuffer() {
	if (_background) {
		_background->free();
		delete _background;
	}

	_background = _vm->_gfx->getBitmap(IDB_INVENTORY_BACKGROUND);
	Graphics::Surface *arrows = _vm->_gfx->getBitmap(IDB_INVENTORY_ARROWS);

	int leftOffset = 3;
	if (_magSelected)
		leftOffset += 69;
	if (_upSelected)
		leftOffset += 23;
	if (_downSelected)
		leftOffset += 46;

	_vm->_gfx->crossBlit(_background, 96, 7, 18, 69, arrows, leftOffset, 0);
	arrows->free();
	delete arrows;

	if (!_itemArray.empty()) {
		// Draw the icon for the current item
		const uint16 curItem = getCurItem();
		Graphics::Surface *icon = _vm->_gfx->getBitmap(IDB_PICON_BITMAP_BASE + _itemArray[curItem]);
		_vm->_gfx->crossBlit(_background, 17, 8, icon->w, icon->h, icon, 0, 0);
		icon->free();
		delete icon;
	}

	return true;
}

bool InventoryWindow::addItem(int itemID) {
	_itemArray.push_back(itemID);

	// Sort the array
	Common::sort(_itemArray.begin(), _itemArray.end());

	// Find the new position, and set the current selection to that
	for (int i = 0; i < (int)_itemArray.size(); i++) {
		if (_itemArray[i] == itemID) {
			setCurItem(i);
			break;
		}
	}

	// Redraw
	rebuildPreBuffer();
	invalidateWindow(false);

	// Update scoring flags
	GlobalFlags &globalFlags = ((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags();

	switch (itemID) {
	case kItemBioChipTranslate:
		globalFlags.scoreGotTranslateBioChip = 1;
		break;
	case kItemBioChipAI:
		globalFlags.scoreDownloadedArthur = 1;
		break;
	case kItemCopperKey:
		globalFlags.scoreGotKeyFromSmithy = 1;
		break;
	case kItemSiegeCycle:
		globalFlags.scoreMadeSiegeCycle = 1;
		globalFlags.genHadSiegeCycle = 1;
		break;
	case kItemJadeBlock:
		globalFlags.scoreGotWealthGodPiece = 1;
		break;
	case kItemLimestoneBlock:
		globalFlags.scoreGotRainGodPiece = 1;
		break;
	case kItemObsidianBlock:
		globalFlags.scoreGotWarGodPiece = 1;
		break;
	case kItemDriveAssembly:
		globalFlags.genHadDriveAssembly = 1;
		break;
	case kItemWheelAssembly:
		globalFlags.genHadWheelAssembly = 1;
		break;
	}

	return true;
}

bool InventoryWindow::removeItem(int itemID) {
	bool found = false;

	for (int i = 0; i < (int)_itemArray.size(); i++) {
		if (_itemArray[i] == itemID) {
			found = true;
			_itemArray.remove_at(i);
			break;
		}
	}

	if (!found)
		return false;

	const uint16 curItem = getCurItem();
	if (curItem >= (int)_itemArray.size()) {
		setCurItem(curItem - 1);
	}

	rebuildPreBuffer();
	invalidateWindow(false);

	return true;
}

bool InventoryWindow::startDraggingNewItem(int itemID, const Common::Point &pointStart) {
	_draggingItemInInventory = false;
	_itemComesFromInventory = false;
	_draggingObject = true;
	_draggingItemID = itemID;
	_draggingIconIndex = 0;

	InventoryElement staticItemData = getItemStaticData(_draggingItemID);

	if (_vm->isDemo())
		_draggingItemSpriteData.image = _dragFrames->getFrameCopy(staticItemData.firstDragID + _draggingIconIndex);
	else
		_draggingItemSpriteData.image = _vm->_gfx->getBitmap(IDB_DRAG_BITMAP_BASE + staticItemData.firstDragID - 1);
	_draggingItemSpriteData.xPos = 0;
	_draggingItemSpriteData.yPos = 0;
	_draggingItemSpriteData.width = _draggingItemSpriteData.image->w;
	_draggingItemSpriteData.height = _draggingItemSpriteData.image->h;

	if (_vm->isTrueColor()) {
		_draggingItemSpriteData.transColor = _vm->_gfx->getColor(255, 255, 255);
	} else {
		_draggingItemSpriteData.transColor = _draggingItemSpriteData.image->getPixel(0, 0);
	}

	setCapture();
	onSetCursor(kMessageTypeLButtonDown);
	onMouseMove(pointStart, 0);

	return true;
}

bool InventoryWindow::displayBurnedLetterWindow() {
	if (_letterViewWindow)
		return true;

	Location currentLocation;
	LocationStaticData currentSceneStaticData;
	((GameUIWindow *)_parent)->_sceneViewWindow->getCurrentSceneLocation(currentLocation);
	((GameUIWindow *)_parent)->_sceneViewWindow->getSceneStaticData(currentLocation, currentSceneStaticData);

	_letterViewWindow = new BurnedLetterViewWindow(_vm, ((GameUIWindow *)_parent)->_sceneViewWindow, currentSceneStaticData);
	((GameUIWindow *)_parent)->_sceneViewWindow->burnedLetterWindowDisplayed(true);
	_letterViewWindow->setWindowPos(kWindowPosTop, 0, 0, 0, 0, kWindowPosShowWindow | kWindowPosNoMove | kWindowPosNoSize);

	return true;
}

bool InventoryWindow::destroyBurnedLetterWindow() {
	if (!_letterViewWindow)
		return false;

	delete _letterViewWindow;
	_letterViewWindow = nullptr;

	((GameUIWindow *)_parent)->_sceneViewWindow->burnedLetterWindowDisplayed(false);

	return true;
}

void InventoryWindow::onPaint() {
	// Draw the prebuffer
	Common::Rect absoluteRect = getAbsoluteRect();
	_vm->_gfx->blit(_background, absoluteRect.left, absoluteRect.top);

	// Draw inventory item names
	uint32 textColor = _vm->_gfx->getColor(212, 109, 0);
	const uint16 curItem = getCurItem();
	for (int i = -2; i < 3; i++) {
		if ((i + curItem) >= 0 && (i + curItem) < (int)_itemArray.size()) {
			Common::Rect textRect = Common::Rect(120, (i + 2) * 13 + 8, 254, (i + 3) * 13 + 8);

			if (_vm->getLanguage() == Common::JA_JPN) {
				// Japanese version is shifted by one pixel
				textRect.top++;
				textRect.bottom++;
			}

			textRect.translate(absoluteRect.left, absoluteRect.top);
			Common::String text = _vm->getString(IDES_ITEM_TITLE_BASE + _itemArray[curItem + i]);
			_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFont, text, textRect.left, textRect.top, textRect.width(), textRect.height(), textColor, _fontHeight);
		}
	}
}

void InventoryWindow::onLButtonDown(const Common::Point &point, uint flags) {
	if (!isWindowEnabled())
		return;

	Common::Rect up(95, 8, 114, 29);
	Common::Rect down(95, 54, 114, 75);
	Common::Rect magnify(95, 30, 114, 52);
	Common::Rect picon(15, 8, 93, 72);

	Common::Rect inventoryText[5];
	for (int i = 0; i < 5; i++)
		inventoryText[i] = Common::Rect(120, i * 13 + 8, 254, (i + 1) * 13 + 8);

	bool redraw = false;
	if (up.contains(point)) {
		if (getCurItem() > 0) {
			_upSelected = true;
			redraw = true;
			_scrollTimer = setTimer(250);
		}
	}

	if (down.contains(point)) {
		if (getCurItem() < ((int)_itemArray.size() - 1)) {
			_downSelected = true;
			redraw = true;
			_scrollTimer = setTimer(250);
		}
	}

	if (magnify.contains(point)) {
		_magSelected = true;
		redraw = true;
	}

	for (int i = 0; i < 5; i++) {
		if (inventoryText[i].contains(point)) {
			_textSelected = i;
			break;
		}
	}

	if (picon.contains(point) && !_itemArray.empty() && !_infoWindow) {
		const uint16 curItem = getCurItem();
		int itemID = _itemArray[curItem];

		switch (itemID) {
		case kItemBioChipAI:
		case kItemBioChipBlank:
		case kItemBioChipCloak:
		case kItemBioChipEvidence:
		case kItemBioChipFiles:
		case kItemBioChipInterface:
		case kItemBioChipJump:
		case kItemBioChipTranslate:
			((GameUIWindow *)_parent)->_bioChipRightWindow->changeCurrentBioChip(itemID);
			return;
		}

		if (!((GameUIWindow *)_parent)->_sceneViewWindow->isAuxWindowDisplayed()) {
			if (itemID == kItemBurnedLetter) {
				displayBurnedLetterWindow();
				return;
			}

			if (itemID == kItemLensFilter) {
				if (((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags().generalWalkthroughMode == 1) {
					((GameUIWindow *)_parent)->_sceneViewWindow->displayLiveText(_vm->getString(IDS_LENS_FILTER_ATTACHED));
					((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags().lensFilterActivated = 1;
					return;
				}

				if (((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags().lensFilterActivated == 0) {
					((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags().lensFilterActivated = 1;
					((GameUIWindow *)_parent)->_sceneViewWindow->displayLiveText(_vm->getString(IDS_LENS_FILTER_ATTACHED));
				} else {
					// Deny removing the filter in the alien space ship
					Location currentLocation;
					((GameUIWindow *)_parent)->_sceneViewWindow->getCurrentSceneLocation(currentLocation);

					if (currentLocation.timeZone == 7) {
						((GameUIWindow *)_parent)->_sceneViewWindow->displayLiveText(_vm->getString(IDS_LENS_FILTER_DENY_REMOVAL));
					} else {
						((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags().lensFilterActivated = 0;
						((GameUIWindow *)_parent)->_sceneViewWindow->displayLiveText(_vm->getString(IDS_LENS_FILTER_REMOVED));
					}
				}

				return;
			}

			InventoryElement staticItemData = getItemStaticData(_itemArray[curItem]);

			if (staticItemData.firstDragID < 0)
				return;

			if (_vm->isDemo())
				_draggingItemSpriteData.image = _dragFrames->getFrameCopy(staticItemData.firstDragID);
			else
				_draggingItemSpriteData.image = _vm->_gfx->getBitmap(IDB_DRAG_BITMAP_BASE + staticItemData.firstDragID - 1);
			_draggingItemInInventory = true;
			_itemComesFromInventory = true;
			_draggingObject = true;
			_draggingItemID = itemID;
			_draggingIconIndex = 0;

			_draggingItemSpriteData.xPos = 0;
			_draggingItemSpriteData.yPos = 0;
			_draggingItemSpriteData.width = _draggingItemSpriteData.image->w;
			_draggingItemSpriteData.height = _draggingItemSpriteData.image->h;

			if (_vm->isTrueColor()) {
				_draggingItemSpriteData.transColor = _vm->_gfx->getColor(255, 255, 255);
			} else {
				_draggingItemSpriteData.transColor = _draggingItemSpriteData.image->getPixel(0, 0);
			}

			setCapture();

			onSetCursor(kMessageTypeLButtonDown);
			((GameUIWindow *)_parent)->_sceneViewWindow->changeSpriteStatus(true);
			onMouseMove(point, 0);
		}
	}

	if (redraw) {
		rebuildPreBuffer();
		invalidateWindow(false);
	}
}

void InventoryWindow::onLButtonUp(const Common::Point &point, uint flags) {
	if (!isWindowEnabled())
		return;

	Common::Rect up(95, 8, 114, 29);
	Common::Rect down(95, 54, 114, 75);
	Common::Rect magnify(95, 30, 114, 52);
	Common::Rect picon(15, 8, 93, 72);

	Common::Rect inventoryText[5];
	for (int i = 0; i < 5; i++)
		inventoryText[i] = Common::Rect(120, i * 13 + 8, 254, (i + 1) * 13 + 8);

	bool redraw = _upSelected || _downSelected || _magSelected;
	uint16 curItem = getCurItem();

	if (up.contains(point) && _upSelected) {
		if (curItem > 0) {
			curItem--;
			setCurItem(curItem);
		}

		if (_infoWindow)
			_infoWindow->changeCurrentItem(_itemArray[curItem]);

		if (_scrollTimer != 0) {
			killTimer(_scrollTimer);
			_scrollTimer = 0;
		}
	}

	if (down.contains(point) && _downSelected) {
		if (curItem < ((int)_itemArray.size() - 1)) {
			curItem++;
			setCurItem(curItem);
		}

		if (_infoWindow)
			_infoWindow->changeCurrentItem(_itemArray[curItem]);

		if (_scrollTimer != 0) {
			killTimer(_scrollTimer);
			_scrollTimer = 0;
		}
	}

	if (magnify.contains(point)) {
		destroyBurnedLetterWindow();

		if (_infoWindow) {
			destroyInfoWindow();
		} else {
			_infoWindow = new InventoryInfoWindow(_vm, ((GameUIWindow *)_parent)->_sceneViewWindow, _itemArray[curItem]);
			((GameUIWindow *)_parent)->_sceneViewWindow->infoWindowDisplayed(true);
			_infoWindow->setWindowPos(kWindowPosTop, 0, 0, 0, 0, kWindowPosShowWindow | kWindowPosNoMove | kWindowPosNoSize);
			_magSelected = true;
			redraw = true;
		}
	}

	if (_textSelected >= 0) {
		curItem = getCurItem();

		for (int i = 0; i < 5; i++) {
			if (inventoryText[i].contains(point) && (curItem + i - 2) >= 0 && (curItem + i - 2) < (int)_itemArray.size() && i == _textSelected) {
				setCurItem(curItem + i - 2);
				redraw = true;

				if (_infoWindow)
					_infoWindow->changeCurrentItem(_itemArray[curItem]);
			}
		}
	}

	_upSelected = false;
	_downSelected = false;
	_textSelected = -1;

	if (_draggingObject) {
		_draggingObject = false;

		_vm->releaseCapture();

		// Reset the cursor
		((GameUIWindow *)getParent())->_sceneViewWindow->resetCursor();

		// Did we drop on the scene or on the inventory window?
		Common::Point ptScene = convertPointToWindow(point, ((GameUIWindow *)getParent())->_sceneViewWindow);
		Common::Point ptGameUI = convertPointToWindow(point, getParent());
		Window *droppedChild = getParent()->childWindowAtPoint(ptGameUI);
		((GameUIWindow *)getParent())->_sceneViewWindow->changeSpriteStatus(false);

		bool returnToInventory = true;
		if (droppedChild == ((GameUIWindow *)getParent())->_sceneViewWindow) {
			if (((GameUIWindow *)getParent())->_sceneViewWindow->droppedItem(_draggingItemID, ptScene, 0) == SIC_ACCEPT)
				returnToInventory = false;

			if (_draggingItemInInventory)
				removeItem(_draggingItemID);
		}

		((GameUIWindow *)getParent())->_sceneViewWindow->changeSpriteStatus(true);

		if (droppedChild == this) {
			returnToInventory = false;

			if (!_draggingItemInInventory) {
				if (_draggingItemID == kItemCheeseGirl || _draggingItemID == kItemBioChipTranslate || _draggingItemID == kItemGenoSingleCart) {
					if (!isItemInInventory(_draggingItemID))
						addItem(_draggingItemID);
				} else {
					addItem(_draggingItemID);
				}
			}
		}

		if (!_itemComesFromInventory)
			((GameUIWindow *)getParent())->_sceneViewWindow->droppedItem(_draggingItemID, Common::Point(-1, -1), 0);

		if (returnToInventory) {
			// If we dropped within the scene, then perform a graduated fall to the inventory window
			if (ptScene.y < 190) {
				// Loop and move the object into the inventory
				int xDelta = 180 - ptScene.x;
				int yDelta = 210 - ptScene.y;

				for (int i = 0; i < 8; i++) {
					xDelta /= 2;
					yDelta /= 2;

					ptScene.x += xDelta;
					ptScene.y += yDelta;
					_draggingItemSpriteData.xPos = ptScene.x - _draggingItemSpriteData.width / 2;
					_draggingItemSpriteData.yPos = ptScene.y - _draggingItemSpriteData.height / 2;

					((GameUIWindow *)getParent())->_sceneViewWindow->updatePrebufferWithSprite(_draggingItemSpriteData);

					// TODO: Update screen? This seems to be missing *something*
				}
			}

			if (!_draggingItemInInventory)
				addItem(_draggingItemID);
		}

		_draggingItemSpriteData.image = nullptr;
		((GameUIWindow *)getParent())->_sceneViewWindow->updatePrebufferWithSprite(_draggingItemSpriteData);
		_itemComesFromInventory = false;

		((GameUIWindow *)getParent())->_bioChipRightWindow->sceneChanged();
	}

	if (redraw) {
		rebuildPreBuffer();
		invalidateWindow(false);
	}
}

void InventoryWindow::onMouseMove(const Common::Point &point, uint flags) {
	_curMousePos = point;

	if (_draggingObject) {
		Common::Point ptScene = convertPointToWindow(point, ((GameUIWindow *)getParent())->_sceneViewWindow);
		Common::Point ptView(ptScene);

		if (ptScene.y > 189) {
			if (!_draggingItemInInventory) {
				if (_draggingItemID == kItemCheeseGirl || _draggingItemID == kItemBioChipTranslate || _draggingItemID == kItemGenoSingleCart) {
					if (!isItemInInventory(_draggingItemID))
						addItem(_draggingItemID);
				} else {
					addItem(_draggingItemID);
				}

				_draggingItemInInventory = true;
				rebuildPreBuffer();
				invalidateWindow();
			}
		} else {
			if (_draggingItemInInventory) {
				if (_draggingItemID == kItemCheeseGirl || _draggingItemID == kItemBioChipTranslate || _draggingItemID == kItemGenoSingleCart) {
					if (isItemInInventory(_draggingItemID))
						removeItem(_draggingItemID);
				} else {
					removeItem(_draggingItemID);
				}

				_draggingItemInInventory = false;
				rebuildPreBuffer();
				invalidateWindow();
			}
		}

		ptScene.x = CLIP<int>(ptScene.x, 0, 431) - _draggingItemSpriteData.width / 2;
		ptScene.y = MAX<int>(ptScene.y, 0) - _draggingItemSpriteData.height / 2;

		int newIcon = ((GameUIWindow *)getParent())->_sceneViewWindow->draggingItem(_draggingItemID, ptView, 0);

		if (_draggingIconIndex != newIcon) {
			InventoryElement staticItemData = getItemStaticData(_draggingItemID);

			if (newIcon > staticItemData.dragIDCount - 1)
				newIcon = staticItemData.dragIDCount - 1;

			if (_draggingIconIndex != newIcon) {
				if (_vm->isDemo())
					_draggingItemSpriteData.image = _dragFrames->getFrameCopy(staticItemData.firstDragID);
				else
					_draggingItemSpriteData.image = _vm->_gfx->getBitmap(IDB_DRAG_BITMAP_BASE + staticItemData.firstDragID + newIcon - 1);
				_draggingItemSpriteData.xPos = 0;
				_draggingItemSpriteData.yPos = 0;
				_draggingItemSpriteData.width = _draggingItemSpriteData.image->w;
				_draggingItemSpriteData.height = _draggingItemSpriteData.image->h;
				_draggingIconIndex = newIcon;

				if (_vm->isTrueColor()) {
					_draggingItemSpriteData.transColor = _vm->_gfx->getColor(255, 255, 255);
				} else {
					_draggingItemSpriteData.transColor = _draggingItemSpriteData.image->getPixel(0, 0);
				}
			}
		}

		_draggingItemSpriteData.xPos = ptScene.x;
		_draggingItemSpriteData.yPos = ptScene.y;
		((GameUIWindow *)getParent())->_sceneViewWindow->updatePrebufferWithSprite(_draggingItemSpriteData);
	} else {
		Common::Rect up(95, 8, 114, 29);
		Common::Rect down(95, 54, 114, 75);
		Common::Rect magnify(95, 30, 114, 52);

		if (_upSelected && !up.contains(point)) {
			_upSelected = false;
			rebuildPreBuffer();
			invalidateWindow(false);
		}

		if (_downSelected && !down.contains(point)) {
			_downSelected = false;
			rebuildPreBuffer();
			invalidateWindow(false);
		}

		if (_magSelected && !magnify.contains(point)) {
			_magSelected = false;
			rebuildPreBuffer();
			invalidateWindow(false);
		}
	}
}

bool InventoryWindow::onSetCursor(uint message) {
	if (!isWindowEnabled())
		return false;

	if (_draggingObject) {
		_vm->_gfx->setCursor(kCursorClosedHand);
	} else if (Common::Rect(15, 8, 93, 72).contains(_curMousePos)) {
		_vm->_gfx->setCursor(kCursorOpenHand);
	} else {
		_vm->_gfx->setCursor(kCursorArrow);
	}

	return true;
}

void InventoryWindow::onTimer(uint timer) {
	const uint16 curItem = getCurItem();
	if (_upSelected) {
		if (curItem > 0) {
			setCurItem(curItem - 1);
			rebuildPreBuffer();
			invalidateWindow(false);
		}
	} else if (_downSelected) {
		if (curItem < (int)_itemArray.size() - 1) {
			setCurItem(curItem + 1);
			rebuildPreBuffer();
			invalidateWindow(false);
		}
	}
}

bool InventoryWindow::isItemInInventory(int itemID) {
	for (uint i = 0; i < _itemArray.size(); i++)
		if (_itemArray[i] == itemID)
			return true;

	return false;
}

InventoryElement InventoryWindow::getItemStaticData(int itemID) {
	Common::SeekableReadStream *resource = _vm->getItemData(IDER_ITEM_DB);
	resource->seek(itemID * (2 + 4 + 4) + 2);

	InventoryElement element;
	element.itemID = resource->readSint16LE();
	element.firstDragID = resource->readSint32LE();
	element.dragIDCount = resource->readSint32LE();

	delete resource;

	return element;
}

bool InventoryWindow::destroyInfoWindow() {
	if (!_infoWindow)
		return false;

	delete _infoWindow;
	_infoWindow = nullptr;

	((GameUIWindow *)_parent)->_sceneViewWindow->infoWindowDisplayed(false);

	_magSelected = false;
	rebuildPreBuffer();
	invalidateWindow(false);

	return true;
}

void InventoryWindow::setItemArray(const Common::Array<int> &array) {
	_itemArray = array;
	Common::sort(_itemArray.begin(), _itemArray.end());

	// Sanity check
	uint16 curItem = getCurItem();
	if (curItem >= _itemArray.size()) {
		warning("Invalid current item, resetting it to the first one");
		setCurItem(0);
	}
}

void InventoryWindow::setCurItem(uint16 itemId) {
	GlobalFlags &globalFlags = ((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags();
	globalFlags.curItem = itemId;
}

uint16 InventoryWindow::getCurItem() const {
	GlobalFlags &globalFlags = ((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags();
	return globalFlags.curItem;
}
} // End of namespace Buried
