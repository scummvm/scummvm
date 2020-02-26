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

#include "common/system.h"

#include "cryomni3d/cryomni3d.h"

#include "cryomni3d/versailles/toolbar.h"

namespace CryOmni3D {
namespace Versailles {

void Toolbar::init(const Sprites *sprites, FontManager *fontManager,
                   const Common::Array<Common::String> *messages, Inventory *inventory,
                   CryOmni3DEngine *engine) {
	_sprites = sprites;
	_fontManager = fontManager;
	_messages = messages;
	_inventory = inventory;
	_engine = engine;

	_bgSurface.create(640, 60, Graphics::PixelFormat::createFormatCLUT8());
	_destSurface.create(640, 60, Graphics::PixelFormat::createFormatCLUT8());

	// Inventory
	addZone(51, 56, Common::Point(211, 8), &Toolbar::callbackInventory<0>);
	addZone(51, 56, Common::Point(258, 8), &Toolbar::callbackInventory<1>);
	addZone(51, 56, Common::Point(305, 8), &Toolbar::callbackInventory<2>);
	addZone(51, 56, Common::Point(352, 8), &Toolbar::callbackInventory<3>);
	addZone(51, 56, Common::Point(399, 8), &Toolbar::callbackInventory<4>);
	addZone(51, 56, Common::Point(446, 8), &Toolbar::callbackInventory<5>);
	addZone(51, 56, Common::Point(493, 8), &Toolbar::callbackInventory<6>);
	addZone(51, 56, Common::Point(540, 8), &Toolbar::callbackInventory<7>);

	// Documentation
	const Graphics::Cursor &cursorDoc = _sprites->getCursor(133);
	Common::Point docPos(627 - cursorDoc.getWidth(), 42 - cursorDoc.getHeight());
	addZone(133, 137, docPos, &Toolbar::callbackDocumentation);

	// Options
	const Graphics::Cursor &cursorOpt = _sprites->getCursor(225);
	Common::Point optPos(0, 60 - cursorOpt.getHeight());
	addZone(225, 225, optPos, &Toolbar::callbackOptions);

	// Previous or next
	addZone(183, uint16(-1), Common::Point(190, 18), &Toolbar::callbackInventoryPrev);
	addZone(240, uint16(-1), Common::Point(574, 18), &Toolbar::callbackInventoryNext);
	// View
	addZone(142, uint16(-1), Common::Point(158, 12), &Toolbar::callbackViewObject);
}

Toolbar::~Toolbar() {
	_bgSurface.free();
	_destSurface.free();
}

void Toolbar::inventoryChanged(uint newPosition) {
	if (newPosition != uint(-1) && newPosition > _inventoryOffset) {
		_inventoryOffset = newPosition - 7;
	}
	// Refresh
	updateZones();
}

void Toolbar::addZone(uint16 cursorMainId, uint16 cursorSecondaryId, Common::Point position,
                      ZoneCallback callback) {
	const Graphics::Cursor &cursorMain = _sprites->getCursor(cursorMainId);
	Common::Rect rct(cursorMain.getWidth(), cursorMain.getHeight());
	rct.moveTo(position);

	// By default it's the secondary image
	Zone zone = { rct, cursorMainId, cursorSecondaryId, callback, true, false };
	_zones.push_back(zone);
}

Common::Array<Toolbar::Zone>::const_iterator Toolbar::hitTestZones(const Common::Point &mousePos)
const {
	Common::Array<Zone>::const_iterator it;
	for (it = _zones.begin(); it != _zones.end(); it++) {
		if (!it->hidden && it->rect.contains(mousePos) && it->callback) {
			break;
		}
	}
	return it;
}

uint Toolbar::captureEvent(const Common::Point &mousePos, uint dragStatus) {
	uint result = 0;
	Common::Array<Zone>::const_iterator it = hitTestZones(mousePos);
	if (it != _zones.end()) {
		result = (this->*(it->callback))(dragStatus);
	}
	return result;
}

void Toolbar::updateZones() {
	_zones[8].secondary = !_engine->hasPlaceDocumentation();

	Inventory::const_iterator inventoryIt, inventorySelectedIt;
	if (!_inventoryEnabled) {
		_inventoryMaxOffset = 0;
		_inventoryOffset = 0;
		_zones[10].secondary = true;
		_zones[11].secondary = true;
		inventoryIt = _inventory->end();
		inventorySelectedIt = _inventory->end();
	} else {
		_inventoryMaxOffset = 0;
		// Find an object in inventory after the 8 first
		for (inventoryIt = _inventory->begin() + 8; inventoryIt != _inventory->end(); inventoryIt++) {
			if (*inventoryIt != nullptr) {
				_inventoryMaxOffset = (inventoryIt - _inventory->begin()) - 7;
			}
		}
		_zones[10].secondary = !_inventoryMaxOffset;
		_zones[11].secondary = !_inventoryMaxOffset;
		if (_inventoryOffset > _inventoryMaxOffset) {
			// Clamp inventory offset to its max
			_inventoryOffset = _inventoryMaxOffset;
		}
		inventoryIt = _inventory->begin() + _inventoryOffset;
		inventorySelectedIt = _inventory->begin() + _inventorySelected;
	}
	// Inventory zones are from 0 to 7
	for (Common::Array<Zone>::iterator zoneIt = _zones.begin(); zoneIt != _zones.begin() + 8;
	        zoneIt++, inventoryIt++) {
		if (!_inventoryEnabled) {
			zoneIt->hidden = true;
			zoneIt->imageMain = 0;
			zoneIt->imageSecondary = 0;
			zoneIt->secondary = false;
		} else if (inventoryIt >= _inventory->end() || *inventoryIt == nullptr) {
			// Nothing in inventory at this position
			zoneIt->hidden = false;
			zoneIt->imageMain = 51;
			zoneIt->imageSecondary = 56;
			zoneIt->secondary = true;
		} else {
			// Setup inventory icon
			zoneIt->hidden = false;
			zoneIt->imageMain = (*inventoryIt)->idCA();
			zoneIt->imageSecondary = (*inventoryIt)->idCl();
			zoneIt->secondary = (inventorySelectedIt != inventoryIt);
		}
	}
}

uint Toolbar::callbackInventory(uint invId, uint dragStatus) {
	if (!_inventoryEnabled) {
		return 0;
	}

	invId += _inventoryOffset;
	Object *obj = nullptr;
	if (invId < _inventory->size()) {
		obj = (*_inventory)[invId];
	}
	if (obj == nullptr) {
		return 0;
	}

	if (!obj->valid()) {
		return 0;
	}

	switch (dragStatus) {
	case kDragStatus_Pressed:
		_inventorySelected = invId;
		_engine->setCursor(181);
		_zones[12].secondary = (obj->viewCallback() == nullptr);
		_inventoryButtonDragging = true;
		return 1;
	case kDragStatus_Dragging:
		if (_inventorySelected == invId) {
			return 0;
		}
		_inventorySelected = invId;
		_zones[12].secondary = (obj->viewCallback() == nullptr);
		_inventoryButtonDragging = true;
		return 1;
	case kDragStatus_Finished:
		_engine->setCursor(obj->idSl());
		_inventory->setSelectedObject(obj);
		_inventorySelected = invId;
		return 1;
	default:
		return 0;
	}

}

uint Toolbar::callbackInventoryPrev(uint dragStatus) {
	if (!_inventoryEnabled) {
		return 0;
	}

	if (dragStatus == kDragStatus_Pressed && _inventoryOffset > 0) {
		// Restart auto repeat only if there could be something
		_engine->setAutoRepeatClick(150);
		_inventoryOffset--;
		return 1;
	}
	// In any other case we didn't do anything
	return 0;
}

uint Toolbar::callbackInventoryNext(uint dragStatus) {
	if (!_inventoryEnabled) {
		return 0;
	}

	if (dragStatus == kDragStatus_Pressed && _inventoryOffset < _inventoryMaxOffset) {
		_engine->setAutoRepeatClick(150);
		_inventoryOffset++;
		return 1;
	}
	// In any other case we didn't do anything
	return 0;
}

uint Toolbar::callbackViewObject(uint dragStatus) {
	if (!_inventoryEnabled) {
		return 0;
	}

	_mouseInViewObject = true;

	if (_inventorySelected == uint(-1)) {
		// Nothing selected in toolbar
		return 0;
	}
	Inventory::const_iterator inventorySelectedIt = _inventory->begin() + _inventorySelected;
	Object *selectedObject = *inventorySelectedIt;
	if (selectedObject == nullptr || selectedObject->viewCallback() == nullptr) {
		// Nothing to view, the sprite isn't even displayed
		return 0;
	}

	switch (dragStatus) {
	case kDragStatus_NoDrag:
		_backupSelectedObject = selectedObject;
		_engine->setCursor(181);
		return 0;
	case kDragStatus_Pressed:
	case kDragStatus_Dragging:
		return 1;
	case kDragStatus_Finished:
		// Just clicked
		_engine->showMouse(false);
		(*selectedObject->viewCallback())();
		_engine->showMouse(true);
		_parentMustRedraw = true;
		_shortExit = true;
		return 1;
	default:
		return 0;
	}
}

uint Toolbar::callbackOptions(uint dragStatus) {
	_mouseInOptions = true;

	switch (dragStatus) {
	case kDragStatus_NoDrag:
		_backupSelectedObject = _inventory->selectedObject();
		_engine->setCursor(181);
		return 0;
	case kDragStatus_Pressed:
	case kDragStatus_Dragging:
		// Nothing to do, we wait release
		return 0;
	case kDragStatus_Finished:
		// Just clicked
		_engine->displayOptions();
		_parentMustRedraw = true;
		_shortExit = true;
		_engine->setMousePos(Common::Point(320, 240)); // Center of screen
		// Displaying options hides the mouse
		_engine->showMouse(true);
		return 0;
	default:
		return 0;
	}
}

uint Toolbar::callbackDocumentation(uint dragStatus) {
	_mouseInOptions = true;

	switch (dragStatus) {
	case kDragStatus_NoDrag:
	case kDragStatus_Pressed:
	case kDragStatus_Dragging:
		// Nothing to do, we wait release
		return 0;
	case kDragStatus_Finished:
		// Just clicked
		if (_engine->displayPlaceDocumentation()) {
			_parentMustRedraw = true;
			_shortExit = true;
			_engine->setMousePos(Common::Point(320, 240)); // Center of screen
		}
		return 0;
	default:
		return 0;
	}
}

void Toolbar::drawToolbar(const Graphics::Surface *original) {
	if (_position > 60) {
		_position = 60;
	}

	if (_position != 0) {
		// Not entirely drawn, we must copy a part of the original image
		Common::Rect rct(0, 420, 640, 420 + _position);
		_destSurface.copyRectToSurface(*original, 0, 0, rct);
	}

	if (_position == 60) {
		// Entirely hidden, just stop there, we have nothing to draw
		return;
	}

	// Not entirely hidden, we must display the transparent background prepared for us
	Common::Rect rct(0, _position, 640, 60);
	_destSurface.copyRectToSurface(_bgSurface, 0, _position, rct);

	// Now draw the various zones on the surface
	for (Common::Array<Zone>::const_iterator it = _zones.begin(); it != _zones.end(); it++) {
		if (it->hidden) {
			continue;
		}

		uint16 spriteId = it->secondary ? it->imageSecondary : it->imageMain;
		if (spriteId == uint16(-1)) {
			continue;
		}

		Common::Rect dst = it->rect;
		dst.translate(0, _position);

		// Clip the rectangle to fit inside the surface
		dst.clip(Common::Rect(_destSurface.w, _destSurface.h));

		if (dst.isEmpty()) {
			continue;
		}

		const Graphics::Surface &sprite = _sprites->getSurface(spriteId);
		_destSurface.transBlitFrom(sprite, Common::Rect(dst.width(), dst.height()), dst,
		                           _sprites->getKeyColor(spriteId));
	}

	// And now draw the object description if needed
	if (_inventoryEnabled && _inventoryHovered != uint(-1)) {
		Object *obj = (*_inventory)[_inventoryHovered];

		uint zoneId = _inventoryHovered - _inventoryOffset;
		if (zoneId >= 8) {
			// The object is hidden: huh?
			return;
		}

		_fontManager->setSurface(&_destSurface);
		_fontManager->setForeColor(243);
		_fontManager->setCurrentFont(5);
		_fontManager->setTransparentBackground(true);
		const Common::String &objName = (*_messages)[obj->idOBJ()];
		uint x = 195 - _fontManager->getStrWidth(objName);
		uint startX = _zones[zoneId].rect.left + kTextOffset;
		_fontManager->displayStr(x, 38 + _position, objName);
		_destSurface.hLine(x, 54 + _position, startX - 1, 243); // minus 1 because hLine draws inclusive
		_destSurface.vLine(startX, 42 + _position, 54 + _position, 243);
	}
}

bool Toolbar::displayToolbar(const Graphics::Surface *original) {
	/**
	 * In game there are 2 functions to handle toolbar: one in warp and one in fixed images
	 * This one is the warp one and fixed images have a more asynchronous one during pop-up/down phases
	 * Let's make it simple for now
	 */

	// WORKAROUND: Set cursor here to be more consistent: it's thumb cursor just before showing until just after showed
	_engine->setCursor(181);

	_parentMustRedraw = false;
	_shortExit = false;

	// Prepare the background of the toolbar by making it translucent
	// Get the lowest part of the image
	const Graphics::Surface subset = original->getSubArea(Common::Rect(0, original->h - _bgSurface.h,
	                                 _bgSurface.w, original->h));
	_engine->makeTranslucent(_bgSurface, subset);

	// WORKAROUND: Reset the inventory status at init to let sprites highlighted until toolbar is hidden
	_inventorySelected = uint(-1);
	_inventoryHovered = uint(-1);
	_zones[12].secondary = true;

	updateZones();

	for (_position = 60; _position > 0; _position--) {
		// Make the toolbar go up
		drawToolbar(original);
		g_system->copyRectToScreen(_destSurface.getPixels(), _destSurface.pitch, 0,
		                           original->h - _destSurface.h, _destSurface.w, _destSurface.h);
		g_system->updateScreen();

		// Slow down animation
		g_system->delayMillis(10);

		_engine->pollEvents();
		if (_engine->shouldAbort()) {
			return false;
		}
	}

	// Flush events
	_engine->clearKeys();
	_engine->waitMouseRelease();

	handleToolbarEvents(original);
	if (_engine->shouldAbort()) {
		return false;
	}

	if (_shortExit) {
		return _parentMustRedraw;
	}

	for (_position = 0; _position <= 60; _position++) {
		// Make the toolbar go up
		drawToolbar(original);
		g_system->copyRectToScreen(_destSurface.getPixels(), _destSurface.pitch, 0,
		                           original->h - _destSurface.h, _destSurface.w, _destSurface.h);
		g_system->updateScreen();

		// Slow down animation
		g_system->delayMillis(10);

		_engine->pollEvents();
		if (_engine->shouldAbort()) {
			return false;
		}
	}

	return _parentMustRedraw;
}

void Toolbar::handleToolbarEvents(const Graphics::Surface *original) {
	bool mouseInsideToolbar;
	bool exitToolbar = false;
	bool redrawToolbar;

	// Don't have anything hovered for now
	_inventoryHovered = uint(-1);
	_inventorySelected = uint(-1);
	_inventory->setSelectedObject(nullptr);
	_backupSelectedObject = nullptr;

	// Refresh zones because we erased selected object
	updateZones();

	// No need of original surface because the toolbar is fully displayed
	drawToolbar(original);

	g_system->copyRectToScreen(_destSurface.getPixels(), _destSurface.pitch, 0,
	                           original->h - _destSurface.h, _destSurface.w, _destSurface.h);
	g_system->updateScreen();

	_engine->setCursor(181);

	mouseInsideToolbar = (_engine->getMousePos().y > 388);

	while (!exitToolbar) {
		_mouseInOptions = false;
		_mouseInViewObject = false;

		_engine->pollEvents();
		if (_engine->shouldAbort()) {
			exitToolbar = true;
			break;
		}

		redrawToolbar = false;
		if (_engine->checkKeysPressed(2, Common::KEYCODE_ESCAPE, Common::KEYCODE_SPACE) ||
		        _engine->getCurrentMouseButton() == 2) {
			_engine->waitMouseRelease();
			exitToolbar = true;
			break;
		}

		Common::Point mousePosInToolbar = _engine->getMousePos();
		mousePosInToolbar -= Common::Point(0, 420);

		if (captureEvent(mousePosInToolbar, _engine->getDragStatus())) {
			// Something has changed with the zones handling, update zones
			updateZones();
			redrawToolbar = true;
		} else if (_engine->getDragStatus() == kDragStatus_Pressed) {
			// A click happened and wasn't handled, deselect object
			_inventorySelected = uint(-1);
			_inventory->setSelectedObject(nullptr);
			_engine->setCursor(181);
			// Reset view object
			_zones[12].secondary = true;
			updateZones();
			redrawToolbar = true;
		}

		if (!mouseInsideToolbar) {
			mouseInsideToolbar = (_engine->getMousePos().y > 388);
		} else if (_engine->getMousePos().y <= 388) {
			// mouseInsideToolbar is true and the mouse is outside the toolbar
			exitToolbar = true;
			break;
		}

		if (_engine->getCurrentMouseButton() == 1) {
			// When the mouse button is down, nothing is selected
			// It's selected on release
			_inventory->setSelectedObject(nullptr);
		}

		if (_backupSelectedObject != nullptr && !(_mouseInOptions || _mouseInViewObject) &&
		        !_engine->getCurrentMouseButton()) {
			_inventory->setSelectedObject(_backupSelectedObject);
			_engine->setCursor(_backupSelectedObject->idSl());
			_backupSelectedObject = nullptr;
		}

		// Hover the inventory objects
		if (_inventory->selectedObject() == nullptr /* || _inventoryButtonDragging */) {
			// The 2nd above condition is maybe useless because when the mouse button is down the selected object is always null
			bool shouldHover = false;
			Common::Array<Zone>::const_iterator zoneIt = hitTestZones(mousePosInToolbar);
			uint zoneId = zoneIt - _zones.begin();
			uint inventoryId = zoneId + _inventoryOffset;
			if (zoneId < 8 && inventoryId < _inventory->size() && (*_inventory)[inventoryId] != nullptr) {
				// It's the inventory
				shouldHover = true;
				if (_inventoryHovered != inventoryId && (*_inventory)[inventoryId]->valid()) {
					// It's not the one currently hovered and it's a valid object
					_inventoryHovered = inventoryId;
					redrawToolbar = true;
				}
			}
			if (!shouldHover && _inventoryHovered != uint(-1) && !_mouseInViewObject)  {
				// Remove hovering
				_inventoryHovered = uint(-1);
				_inventorySelected = uint(-1);
				updateZones();
				if (!_inventory->selectedObject()) {
					// Reset back the cursor if nothing is selected
					_engine->setCursor(181);
				}
				// Remove view
				_zones[12].secondary = true;
				redrawToolbar = true;
			}
			_inventoryButtonDragging = false;
		}

		if (_parentMustRedraw) {
			break;
		}

		if (redrawToolbar) {
			drawToolbar(original);
			g_system->copyRectToScreen(_destSurface.getPixels(), _destSurface.pitch, 0,
			                           original->h - _destSurface.h, _destSurface.w, _destSurface.h);
		}

		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	// Hide description when finished and selected object
	// WORKAROUND: moved to the start to keep the selected object hilighted until the toolbar disappearance
}

} // End of namespace Versailles
} // End of namespace CryOmni3D
