/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/selector.h"
#include "sci/engine/state.h"
#include "sci/graphics/gfxdrivers.h"
#include "sci/graphics/maciconbar.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"

#include "common/memstream.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "image/pict.h"

namespace Sci {

GfxMacIconBar::GfxMacIconBar(ResourceManager *resMan, EventManager *eventMan, SegManager *segMan, GfxScreen *screen, GfxPalette *palette) :
	_resMan(resMan), _eventMan(eventMan), _segMan(segMan), _screen(screen), _palette(palette)  {
	if (g_sci->getGameId() == GID_FREDDYPHARKAS)
		_inventoryIndex = 5;
	else
		_inventoryIndex = 4;

	_inventoryIcon = nullptr;
	_allDisabled = true;
	
	_isUpscaled = (_screen->getUpscaledHires() == GFX_SCREEN_UPSCALED_640x400);
}

GfxMacIconBar::~GfxMacIconBar() {
	freeIcons();
}

void GfxMacIconBar::initIcons(uint16 count, reg_t *objs) {
	// free icons and reset state in case game is restarting
	freeIcons();
	_iconBarItems.clear();
	_inventoryIcon = nullptr;
	_allDisabled = true;

	for (uint16 i = 0; i < count; i++) {
		addIcon(objs[i]);
	}
}

void GfxMacIconBar::freeIcons() {
	if (_inventoryIcon) {
		_inventoryIcon->free();
		delete _inventoryIcon;
	}

	for (uint32 i = 0; i < _iconBarItems.size(); i++) {
		if (_iconBarItems[i].nonSelectedImage) {
			_iconBarItems[i].nonSelectedImage->free();
			delete _iconBarItems[i].nonSelectedImage;
		}

		if (_iconBarItems[i].selectedImage) {
			_iconBarItems[i].selectedImage->free();
			delete _iconBarItems[i].selectedImage;
		}
	}
}

void GfxMacIconBar::addIcon(reg_t obj) {
	IconBarItem item;
	uint32 iconIndex = readSelectorValue(_segMan, obj, SELECTOR(iconIndex));

	item.object = obj;
	item.nonSelectedImage = createImage(iconIndex, false);

	if (iconIndex != _inventoryIndex)
		item.selectedImage = createImage(iconIndex, true);
	else
		item.selectedImage = nullptr;

	item.enabled = true;

	// Start after last icon
	uint16 x = _iconBarItems.empty() ? 0 : _iconBarItems.back().rect.right;

	// Start below the main viewing window and add a two pixel buffer
	uint16 y = _screen->getHeight() + 2;

	if (item.nonSelectedImage)
		item.rect = Common::Rect(x, y, MIN<uint32>(x + item.nonSelectedImage->w, 320), y + item.nonSelectedImage->h);
	else
		error("Could not find a non-selected image for icon %d", iconIndex);

	_iconBarItems.push_back(item);
}

void GfxMacIconBar::drawIcons() {
	// Draw the icons to the bottom of the screen

	for (uint32 i = 0; i < _iconBarItems.size(); i++)
		drawIcon(i, false);
}

void GfxMacIconBar::drawIcon(uint16 iconIndex, bool selected) {
	if (iconIndex >= _iconBarItems.size())
		return;

	Common::Rect rect = _iconBarItems[iconIndex].rect;

	if (isIconEnabled(iconIndex)) {
		if (selected)
			drawImage(_iconBarItems[iconIndex].selectedImage, rect, true);
		else
			drawImage(_iconBarItems[iconIndex].nonSelectedImage, rect, true);
	} else
		drawImage(_iconBarItems[iconIndex].nonSelectedImage, rect, false);

	if ((iconIndex == _inventoryIndex) && _inventoryIcon) {
		Common::Rect invRect = Common::Rect(0, 0, _inventoryIcon->w, _inventoryIcon->h);
		invRect.moveTo(rect.left, rect.top);
		invRect.translate((rect.width() - invRect.width()) / 2, (rect.height() - invRect.height()) / 2);

		drawImage(_inventoryIcon, invRect, isIconEnabled(iconIndex));
	}
}

// Add a black checkerboard pattern to an image before copying it to the screen.
// The pattern is to be applied to the image after any upscaling occurs, so rect
// must be the final screen coordinates.
void GfxMacIconBar::drawDisabledPattern(Graphics::Surface &surface, const Common::Rect &rect) {
	for (int y = 0; y < surface.h; y++) {
		// Start at the next four byte boundary
		int startX = 3 - ((rect.left + 3) & 3);

		// Start odd rows at two bytes past that (also properly aligned)
		if ((y + rect.top) & 1) {
			startX = (startX + 2) & 3;
		}

		// Set every fourth pixel to black
		for (int x = startX; x < surface.w; x += 4) {
			surface.setPixel(x, y, 0);
		}
	}
}

void GfxMacIconBar::drawImage(Graphics::Surface *surface, const Common::Rect &rect, bool enable) {
	if (surface == nullptr) {
		return;
	}

	if (_isUpscaled) {
		Common::Rect dstRect(rect.left * 2, rect.top * 2, rect.right * 2, rect.bottom * 2);

		// increase _upscaleBuffer if needed
		const uint32 upscaleSize = dstRect.width() * dstRect.height();
		if (upscaleSize > _upscaleBuffer->size()) {
			_upscaleBuffer.clear();
			_upscaleBuffer->allocate(upscaleSize);
		}

		// scale2x
		const int srcWidth = rect.width();
		const int srcHeight = rect.height();
		const int srcPitch = surface->pitch;
		const byte *srcPtr = (byte *)surface->getPixels();
		byte *dstPtr = _upscaleBuffer->getUnsafeDataAt(0, upscaleSize);
		for (int y = 0; y < srcHeight; y++) {
			for (int x = 0; x < srcWidth; x++) {
				const byte color = *srcPtr++;
				dstPtr[0] = color;
				dstPtr[1] = color;
				dstPtr[dstRect.width() + 0] = color;
				dstPtr[dstRect.width() + 1] = color;
				dstPtr += 2;
			}
			srcPtr += (srcPitch - srcWidth);
			dstPtr += dstRect.width();
		}

		if (!enable) {
			Graphics::Surface upscaleSurface;
			upscaleSurface.init(dstRect.width(), dstRect.height(), dstRect.width(), _upscaleBuffer->getUnsafeDataAt(0, upscaleSize), surface->format);
			drawDisabledPattern(upscaleSurface, dstRect);
		}
		_screen->gfxDriver()->copyRectToScreen(_upscaleBuffer->getUnsafeDataAt(0, upscaleSize), 0, 0, dstRect.width(), dstRect.left, dstRect.top, dstRect.width(), dstRect.height(), nullptr, nullptr);
	} else {
		if (!enable) {
			Graphics::Surface disableSurface;
			disableSurface.copyFrom(*surface);
			drawDisabledPattern(disableSurface, rect);
			_screen->gfxDriver()->copyRectToScreen((const byte*)disableSurface.getPixels(), 0, 0, disableSurface.pitch, rect.left, rect.top, rect.width(), rect.height(), nullptr, nullptr);
		} else {
			_screen->gfxDriver()->copyRectToScreen((const byte*)surface->getPixels(), 0, 0, surface->pitch, rect.left, rect.top, rect.width(), rect.height(), nullptr, nullptr);
		}
	}
}

bool GfxMacIconBar::isIconEnabled(uint16 iconIndex) const {
	if (iconIndex >= _iconBarItems.size())
		return false;

	return !_allDisabled && _iconBarItems[iconIndex].enabled;
}

void GfxMacIconBar::setIconEnabled(int16 iconIndex, bool enabled) {
	if (iconIndex < 0)
		_allDisabled = !enabled;
	else if (iconIndex < (int)_iconBarItems.size()) {
		_iconBarItems[iconIndex].enabled = enabled;
	}
}

void GfxMacIconBar::setInventoryIcon(int16 icon) {
	Graphics::Surface *surface = nullptr;

	if (icon >= 0)
		surface = loadPict(ResourceId(kResourceTypeMacPict, icon));

	if (_inventoryIcon) {
		// Free old inventory icon if we're removing the inventory icon
		// or setting a new one.
		if ((icon < 0) || surface) {
			_inventoryIcon->free();
			delete _inventoryIcon;
			_inventoryIcon = nullptr;
		}
	}

	if (surface)
		_inventoryIcon = surface;

	drawIcon(_inventoryIndex, false);
}

Graphics::Surface *GfxMacIconBar::loadPict(ResourceId id) {
	Resource *res = _resMan->findResource(id, false);

	if (!res || res->size() == 0)
		return nullptr;

	Image::PICTDecoder pictDecoder;
	Common::MemoryReadStream stream(res->toStream());
	if (!pictDecoder.loadStream(stream))
		return nullptr;

	Graphics::Surface *surface = new Graphics::Surface();
	surface->copyFrom(*pictDecoder.getSurface());
	remapColors(surface, pictDecoder.getPalette());

	return surface;
}

Graphics::Surface *GfxMacIconBar::createImage(uint32 iconIndex, bool isSelected) {
	ResourceType type = isSelected ? kResourceTypeMacIconBarPictS : kResourceTypeMacIconBarPictN;
	return loadPict(ResourceId(type, iconIndex + 1));
}

void GfxMacIconBar::remapColors(Graphics::Surface *surf, const byte *palette) {
	byte *pixels = (byte *)surf->getPixels();

	// Remap to the screen palette
	for (uint16 i = 0; i < surf->w * surf->h; i++) {
		byte color = *pixels;

		byte r = palette[color * 3];
		byte g = palette[color * 3 + 1];
		byte b = palette[color * 3 + 2];

		*pixels++ = _palette->findMacIconBarColor(r, g, b);
	}
}

bool GfxMacIconBar::pointOnIcon(uint32 iconIndex, Common::Point point) {
	return _iconBarItems[iconIndex].rect.contains(point);
}

bool GfxMacIconBar::handleEvents(SciEvent evt, reg_t &iconObj) {
	iconObj = NULL_REG;

	// Not a mouse press
	if (evt.type != kSciEventMousePress)
		return false;

	// If the mouse is not over the icon bar, return
	if (evt.mousePos.y < _screen->getHeight())
		return false;

	// Mouse press on the icon bar, check the icon rectangles
	uint iconNr;
	for (iconNr = 0; iconNr < _iconBarItems.size(); iconNr++) {
		if (pointOnIcon(iconNr, evt.mousePos) && isIconEnabled(iconNr))
			break;
	}

	// Mouse press on the icon bar but not on an enabled icon,
	// return true to indicate that this mouse press was handled
	if (iconNr == _iconBarItems.size())
		return true;

	drawIcon(iconNr, true);
	bool isSelected = true;

	// Wait for mouse release
	while (evt.type != kSciEventMouseRelease) {
		// Mimic behavior of SSCI when moving mouse with button held down
		if (isSelected != pointOnIcon(iconNr, evt.mousePos)) {
			isSelected = !isSelected;
			drawIcon(iconNr, isSelected);
		}

		evt = _eventMan->getSciEvent(kSciEventMouseRelease);
		g_system->delayMillis(10);
	}

	drawIcon(iconNr, false);

	// If user moved away from the icon, we do nothing
	if (pointOnIcon(iconNr, evt.mousePos))
		iconObj = _iconBarItems[iconNr].object;

	// The mouse press was handled
	return true;
}

} // End of namespace Sci
