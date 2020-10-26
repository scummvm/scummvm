/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/ui/world/gamewindow.h"

#include "engines/stark/scene.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/knowledgeset.h"
#include "engines/stark/resources/image.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/layer.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/gameinterface.h"
#include "engines/stark/services/userinterface.h"

#include "engines/stark/ui/cursor.h"
#include "engines/stark/ui/world/actionmenu.h"
#include "engines/stark/ui/world/inventorywindow.h"

#include "engines/stark/visual/text.h"
#include "engines/stark/visual/image.h"

namespace Stark {

GameWindow::GameWindow(Gfx::Driver *gfx, Cursor *cursor, ActionMenu *actionMenu, InventoryWindow *inventory) :
		Window(gfx, cursor),
	_actionMenu(actionMenu),
	_inventory(inventory),
	_objectUnderCursor(nullptr),
	_displayExit(false) {
	_position = Common::Rect(Gfx::Driver::kGameViewportWidth, Gfx::Driver::kGameViewportHeight);
	_position.translate(0, Gfx::Driver::kTopBorderHeight);
	_visible = true;

	_fadeRenderer = _gfx->createFadeRenderer();

	_exitArrow = StarkStaticProvider->getUIElement(StaticProvider::kExitArrow);
	_exitArrowLeft = StarkStaticProvider->getUIElement(StaticProvider::kExitArrowLeft);
	_exitArrowRight = StarkStaticProvider->getUIElement(StaticProvider::kExitArrowRight);

	_exitLeftBoundary = 5;
	_exitRightBoundary = Gfx::Driver::kGameViewportWidth - _exitArrowRight->getWidth() - 5;
}

GameWindow::~GameWindow() {
	delete _fadeRenderer;
}

void GameWindow::onRender() {
	// List the items to render
	Resources::Location *location = StarkGlobal->getCurrent()->getLocation();
	_renderEntries = location->listRenderEntries();
	Gfx::LightEntryArray lightEntries = location->listLightEntries();

	// Render all the scene items
	Gfx::RenderEntryArray::iterator element = _renderEntries.begin();
	while (element != _renderEntries.end()) {
		// Draw the current element
		(*element)->render(lightEntries);

		// Go for the next one
		element++;
	}

	if (_displayExit) {
		Common::Array<Common::Point> exitPositions = StarkGameInterface->listExitPositions();

		for (uint i = 0; i < exitPositions.size(); ++i) {
			Common::Point pos = exitPositions[i];
			VisualImageXMG *exitImage = nullptr;

			if (pos.x < _exitLeftBoundary) {
				pos.x = _exitLeftBoundary;
				exitImage = _exitArrowLeft;
			} else if (pos.x > _exitRightBoundary) {
				pos.x = _exitRightBoundary;
				exitImage = _exitArrowRight;
			} else {
				exitImage = _exitArrow;
			}

			exitImage->render(pos, false);
		}
	}

	float fadeLevel = StarkScene->getFadeLevel();
	if ((1.0f - fadeLevel) > 0.00001f) {
		_fadeRenderer->render(fadeLevel);
	}
}

void GameWindow::onMouseMove(const Common::Point &pos) {
	_renderEntries = StarkGlobal->getCurrent()->getLocation()->listRenderEntries();

	if (!StarkUserInterface->isInteractive()) {
		_objectUnderCursor = nullptr;
		_cursor->setCursorType(Cursor::kPassive);
		_cursor->setMouseHint("");
		return;
	}

	int16 selectedInventoryItem = _inventory->getSelectedInventoryItem();
	int16 singlePossibleAction = -1;
	bool defaultAction = false;
	bool itemActive = false;

	checkObjectAtPos(pos, selectedInventoryItem, singlePossibleAction, defaultAction);

	if (selectedInventoryItem != -1 && !defaultAction) {
		VisualImageXMG *cursorImage = StarkGameInterface->getCursorImage(selectedInventoryItem);
		_cursor->setCursorImage(cursorImage);
		itemActive = singlePossibleAction == selectedInventoryItem;
	} else if (_objectUnderCursor) {
		switch (singlePossibleAction) {
			case -1:
				_cursor->setCursorType(Cursor::kActive);
				break;
			case Resources::PATTable::kActionLook:
				_cursor->setCursorType(Cursor::kEye);
				break;
			case Resources::PATTable::kActionTalk:
				_cursor->setCursorType(Cursor::kMouth);
				break;
			case Resources::PATTable::kActionUse:
				_cursor->setCursorType(Cursor::kHand);
				break;
			default:
				VisualImageXMG *cursorImage = StarkGameInterface->getCursorImage(singlePossibleAction);
				_cursor->setCursorImage(cursorImage);
				break;
		}
	} else {
		// Not an object
		_cursor->setCursorType(Cursor::kDefault);
	}
	_cursor->setItemActive(itemActive);

	Common::String mouseHint;
	if (_objectUnderCursor) {
		mouseHint = StarkGameInterface->getItemTitleAt(_objectUnderCursor, _objectRelativePosition);
	}
	_cursor->setMouseHint(mouseHint);
}

void GameWindow::onClick(const Common::Point &pos) {
	if (!StarkGlobal->getCurrent()) {
		return; // No level is loaded yet, interaction is impossible
	}

	if (!StarkUserInterface->isInteractive()) {
		StarkUserInterface->markInteractionDenied();
		return;
	}

	_actionMenu->close();

	int16 selectedInventoryItem = _inventory->getSelectedInventoryItem();
	int16 singlePossibleAction = -1;
	bool defaultAction;

	checkObjectAtPos(pos, selectedInventoryItem, singlePossibleAction, defaultAction);

	if (_objectUnderCursor) {
		if (singlePossibleAction != -1) {
			StarkGameInterface->itemDoActionAt(_objectUnderCursor, singlePossibleAction, _objectRelativePosition);
		} else if (selectedInventoryItem == -1) {
			_actionMenu->open(_objectUnderCursor, _objectRelativePosition);
		}
	} else {
		// The walk code expects unscaled absolute mouse coordinates
		StarkGameInterface->walkTo(_cursor->getMousePosition(true));
	}
}

void GameWindow::onRightClick(const Common::Point &pos) {
	if (!StarkUserInterface->isInteractive()) {
		return;
	}

	int16 selectedInventoryItem = _inventory->getSelectedInventoryItem();

	if (selectedInventoryItem == -1) {
		_inventory->open();
	} else {
		_inventory->setSelectedInventoryItem(-1);
	}
}

void GameWindow::onDoubleClick(const Common::Point &pos) {
	if (!StarkUserInterface->isInteractive()) {
		StarkUserInterface->markInteractionDenied();
		return;
	}

	if (StarkGameInterface->isAprilWalking()) {
		StarkGameInterface->setAprilRunning();
	}
}

void GameWindow::checkObjectAtPos(const Common::Point &pos, int16 selectedInventoryItem, int16 &singlePossibleAction, bool &isDefaultAction) {
	_objectUnderCursor = nullptr;
	singlePossibleAction = -1;
	isDefaultAction = false;

	Math::Ray ray = StarkScene->makeRayFromMouse(_cursor->getMousePosition(true));

	Common::Rect cursorRect;
	if (selectedInventoryItem != -1) {
		cursorRect = _cursor->getHotRectangle();
		cursorRect.translate(pos.x, pos.y);
	}

	// Render entries are sorted from the farthest to the camera to the nearest
	// Loop in reverse order
	for (int i = _renderEntries.size() - 1; i >= 0; i--) {
		if (_renderEntries[i]->containsPoint(pos, _objectRelativePosition, cursorRect)
		    || _renderEntries[i]->intersectRay(ray)) {
			_objectUnderCursor = _renderEntries[i]->getOwner();
			break;
		}
	}

	if (!_objectUnderCursor || !StarkGameInterface->itemHasActionAt(_objectUnderCursor, _objectRelativePosition, -1)) {
		// Only consider items with runnable scripts
		_objectUnderCursor = nullptr;
		return;
	}

	int32 defaultAction = StarkGameInterface->itemGetDefaultActionAt(_objectUnderCursor, _objectRelativePosition);
	if (defaultAction != -1) {
		// Use the default action if there is one
		singlePossibleAction = defaultAction;
		isDefaultAction = true;
	} else if (selectedInventoryItem != -1) {
		// Use the selected inventory item if there is one
		if (StarkGameInterface->itemHasActionAt(_objectUnderCursor, _objectRelativePosition, selectedInventoryItem)) {
			singlePossibleAction = selectedInventoryItem;
		}
	} else {
		// Otherwise, use stock actions
		Resources::ActionArray actionsPossible = StarkGameInterface->listStockActionsPossibleForObjectAt(
				_objectUnderCursor, _objectRelativePosition);

		if (actionsPossible.size() == 1) {
			singlePossibleAction = actionsPossible[0];
		}
	}
}

void GameWindow::reset() {
	_renderEntries.clear();
	_objectUnderCursor = nullptr;
	_objectRelativePosition.x = 0;
	_objectRelativePosition.y = 0;
}

void GameWindow::onScreenChanged() {
	// May be called when resources have not been loaded
	if (!StarkGlobal->getCurrent()) {
		return;
	}

	Resources::Location *location = StarkGlobal->getCurrent()->getLocation();
	Common::Array<Resources::ImageText *> images = location->listChildrenRecursive<Resources::ImageText>(Resources::Image::kImageText);

	for (uint i = 0; i < images.size(); i++) {
		images[i]->resetVisual();
	}
}

} // End of namespace Stark
