/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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
#include "common/file.h"
#include "common/textconsole.h"
#include "engines/util.h"

#include "pegasus/elements.h"
#include "pegasus/graphics.h"

namespace Pegasus {

GraphicsManager::GraphicsManager(PegasusEngine *vm) : _vm(vm) {
	initGraphics(640, 480, true, NULL);

	if (_vm->_system->getScreenFormat().bytesPerPixel == 1)
		error("No true color mode available");

	_backLayer = kMinAvailableOrder;
	_frontLayer = kMaxAvailableOrder;
	_firstDisplayElement = _lastDisplayElement = 0;
	_workArea.create(640, 480, _vm->_system->getScreenFormat());
	_lastMousePosition = Common::Point(-1, -1);
}
	
GraphicsManager::~GraphicsManager() {
	_workArea.free();
}

void GraphicsManager::invalRect(const Common::Rect &rect) {
	// We're using a simpler algorithm for dirty rect handling than the original
	// The original was way too overcomplicated for what we need here now.

	if (_dirtyRect.width() == 0 || _dirtyRect.height() == 0) {
		// We have no dirty rect, so this is now our dirty rect
		_dirtyRect = rect;
	} else {
		// Expand our dirty rect to include rect
		_dirtyRect.extend(rect);
	}

	// Sanity check: clip our rect to the screen
	_dirtyRect.right = MIN<int>(640, _dirtyRect.right);
	_dirtyRect.bottom = MIN<int>(480, _dirtyRect.bottom);
}

void GraphicsManager::addDisplayElement(DisplayElement *newElement) {
	newElement->_elementOrder = CLIP<int>(newElement->_elementOrder, kMinAvailableOrder, kMaxAvailableOrder);

	if (_firstDisplayElement) {
		DisplayElement *runner = _firstDisplayElement;
		DisplayElement *lastRunner = 0;

		// Search for first element whose display order is greater than
		// the new element's and add the new element just before it.
		while (runner) {
			if (newElement->_elementOrder < runner->_elementOrder) {
				if (lastRunner) {
					lastRunner->_nextElement = newElement;
					newElement->_nextElement = runner;
				} else {
					newElement->_nextElement = _firstDisplayElement;
					_firstDisplayElement = newElement;
				}
				break;
			}
			lastRunner = runner;
			runner = runner->_nextElement;
		}

		// If got here and runner == NULL, we ran through the whole list without
		// inserting, so add at the end.
		if (!runner) {
			_lastDisplayElement->_nextElement = newElement;
			_lastDisplayElement = newElement;
		}
	} else {
		_firstDisplayElement = newElement;
		_lastDisplayElement = newElement;
	}

	newElement->_elementIsDisplaying = true;
}

void GraphicsManager::removeDisplayElement(DisplayElement *oldElement) {
	if (!_firstDisplayElement)
		return;

	if (oldElement == _firstDisplayElement) {
		if (oldElement == _lastDisplayElement) {
			_firstDisplayElement = 0;
			_lastDisplayElement = 0;
		} else {
			_firstDisplayElement = oldElement->_nextElement;
		}

		invalRect(oldElement->_bounds);
	} else {
		// Scan list for element.
		// If we get here, we know that the list has at least one item, and it
		// is not the first item, so we can skip it.
		DisplayElement *runner = _firstDisplayElement->_nextElement;
		DisplayElement *lastRunner = _firstDisplayElement;

		while (runner) {
			if (runner == oldElement) {
				lastRunner->_nextElement = runner->_nextElement;

				if (oldElement == _lastDisplayElement)
					_lastDisplayElement = lastRunner;

				invalRect(oldElement->_bounds);
				break;
			}

			lastRunner = runner;
			runner = runner->_nextElement;
		}
	}

	oldElement->_nextElement = 0;
	oldElement->_elementIsDisplaying = false;
}

void GraphicsManager::updateDisplay() {
	bool screenDirty = false;

	// TODO: Check for cursor change
	Common::Point mousePos = g_system->getEventManager()->getMousePos();
	if (_lastMousePosition != mousePos) {
		screenDirty = true;
		_lastMousePosition = mousePos;
	}

	if (!_dirtyRect.isEmpty()) {
		for (DisplayElement *runner = _firstDisplayElement; runner != 0; runner = runner->_nextElement) {
			Common::Rect bounds;
			runner->getBounds(bounds);

			// TODO: Better logic; it does a bit more work than it probably needs to
			// but it should work fine for now.
			if (bounds.intersects(_dirtyRect) && runner->validToDraw(_backLayer, _frontLayer))
				runner->draw(bounds);
		}

		// Copy only the dirty rect to the screen
		g_system->copyRectToScreen((byte *)_workArea.getBasePtr(_dirtyRect.left, _dirtyRect.top), _workArea.pitch, _dirtyRect.left, _dirtyRect.top, _dirtyRect.width(), _dirtyRect.height());

		// Mark the screen as dirty
		screenDirty = true;

		// Clear the dirty rect
		_dirtyRect = Common::Rect();
	}

	if (screenDirty)
		g_system->updateScreen();
}
	
} // End of namespace Pegasus
