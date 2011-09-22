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

	// Old
	_pictDecoder = new Graphics::PictDecoder(_vm->_system->getScreenFormat());

	for (int i = 0; i < kImageCacheSize; i++)
		_cache[i].surface = 0;

	// New
	_backLayer = kMinAvailableOrder;
	_frontLayer = kMaxAvailableOrder;
	_firstDisplayElement = _lastDisplayElement = 0;
	_workArea.create(640, 480, _vm->_system->getScreenFormat());
}
	
GraphicsManager::~GraphicsManager() {
	// Old
	delete _pictDecoder;

	for (int i = 0; i < kImageCacheSize; i++) {
		if (_cache[i].surface) {
			_cache[i].surface->free();
			delete _cache[i].surface;
		}
	}

	// New
	_workArea.free();
}

Graphics::Surface *GraphicsManager::decodeImage(const Common::String &filename) {
	int imageSlot = getImageSlot(filename);

	if (_cache[imageSlot].surface)
		return _cache[imageSlot].surface;

	Common::File file;
	if (!file.open(filename))
		error("Could not open \'%s\'", filename.c_str());

	byte palette[256 * 3];
	Graphics::Surface *image = _pictDecoder->decodeImage(&file, palette);

	// For <= 8bpp, we need to convert
	if (image->format.bytesPerPixel == 1) {
		Graphics::PixelFormat format = _vm->_system->getScreenFormat();
		Graphics::Surface *output = new Graphics::Surface();
		output->create(image->w, image->h, format);

		for (uint16 y = 0; y < image->h; y++) {
			for (uint16 x = 0; x < image->w; x++) {
				byte c = *((byte *)image->getBasePtr(x, y));
				byte r = palette[c * 3];
				byte g = palette[c * 3 + 1];
				byte b = palette[c * 3 + 2];

				if (format.bytesPerPixel == 2) {
					uint16 color = format.RGBToColor(r, g, b);
					memcpy(output->getBasePtr(x, y), &color, 2);
				} else if (format.bytesPerPixel == 4) {
					uint32 color = format.RGBToColor(r, g, b);
					memcpy(output->getBasePtr(x, y), &color, 4);
				}
			}
		}

		image->free();
		delete image;
		image = output;
	}

	_cache[imageSlot].surface = image;
	return image;
}

void GraphicsManager::drawPict(Common::String filename, int x, int y, bool updateScreen) {
	Graphics::Surface *surface = decodeImage(filename);
	
	_vm->_system->copyRectToScreen((byte *)surface->pixels, surface->pitch, x, y, surface->w, surface->h);

	if (updateScreen)
		_vm->_system->updateScreen();
}

void GraphicsManager::drawPictTransparent(Common::String filename, int x, int y, uint32 transparency, bool updateScreen) {
	if (_vm->_system->getScreenFormat().bytesPerPixel == 2)
		transparency &= 0xffff;

	Graphics::Surface *surface = decodeImage(filename);
	Graphics::Surface *screen = _vm->_system->lockScreen();

	for (uint16 i = 0; i < surface->h; i++) {
		for (uint16 j = 0; j < surface->w; j++) {
			if (_vm->_system->getScreenFormat().bytesPerPixel == 2) {
				uint16 color = *((uint16 *)surface->getBasePtr(j, i));
				if (color != transparency)
					memcpy(screen->getBasePtr(j + x, i + y), &color, 2);
			} else if (_vm->_system->getScreenFormat().bytesPerPixel == 4) {
				uint32 color = *((uint32 *)surface->getBasePtr(j, i));
				if (color != transparency)
					memcpy(screen->getBasePtr(j + x, i + y), &color, 4);
			}
		}
	}

	_vm->_system->unlockScreen();

	if (updateScreen)
		_vm->_system->updateScreen();
}

uint32 GraphicsManager::getColor(byte r, byte g, byte b) {
	return _vm->_system->getScreenFormat().RGBToColor(r, g, b);
}

int GraphicsManager::getImageSlot(const Common::String &filename) {
	// Let's find a match, an open slot, or the oldest image slot
	uint32 oldestAge = 0xffffffff;
	int slot = 0;

	for (int i = 0; i < kImageCacheSize; i++) {
		if (_cache[i].filename.equalsIgnoreCase(filename)) {
			//warning("Found image %s at slot %d", filename.c_str(), i);
			_cache[i].lastUsed = _vm->_system->getMillis();
			return i;
		}

		if (!_cache[i].surface) {
			//warning("Putting image %s in empty slot %d", filename.c_str(), i);
			_cache[i].filename = filename;
			_cache[i].lastUsed = _vm->_system->getMillis();
			return i;
		}

		if (_cache[i].lastUsed < oldestAge) {
			oldestAge = _cache[i].lastUsed;
			slot = i;
		}
	}

	// Let's make sure that's cleaned out
	//warning("Replacing old image %s with %s in slot %d", _cache[slot].filename.c_str(), filename.c_str(), slot);
	_cache[slot].filename = filename;
	_cache[slot].surface->free();
	delete _cache[slot].surface;
	_cache[slot].surface = 0;
	_cache[slot].lastUsed = _vm->_system->getMillis();
	return slot;
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
	// TODO: Check for cursor move/change

	bool screenDirty = false;

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
