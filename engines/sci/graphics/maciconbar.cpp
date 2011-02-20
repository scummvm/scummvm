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
 * $URL$
 * $Id$
 *
 */

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/selector.h"
#include "sci/engine/state.h"
#include "sci/graphics/maciconbar.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"

#include "common/memstream.h"
#include "common/system.h"
#include "graphics/pict.h"
#include "graphics/surface.h"

namespace Sci {

GfxMacIconBar::GfxMacIconBar() {
	_lastX = 0;
}

GfxMacIconBar::~GfxMacIconBar() {
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
	uint32 iconIndex = readSelectorValue(g_sci->getEngineState()->_segMan, obj, SELECTOR(iconIndex));

	item.object = obj;
	item.nonSelectedImage = createImage(iconIndex, false);
	item.selectedImage = createImage(iconIndex, true);
	item.enabled = true;

	// Start after the main viewing window and add a two pixel buffer
	uint16 y = g_sci->_gfxScreen->getHeight() + 2;

	if (item.nonSelectedImage)
		item.rect = Common::Rect(_lastX, y, MIN<uint32>(_lastX + item.nonSelectedImage->w, 320), y + item.nonSelectedImage->h);
	else
		error("Could not find a non-selected image for icon %d", iconIndex);

	_lastX += item.rect.width();

	_iconBarItems.push_back(item);
}

void GfxMacIconBar::drawIcons() {
	// Draw the icons to the bottom of the screen

	for (uint32 i = 0; i < _iconBarItems.size(); i++)
		redrawIcon(i);
}

void GfxMacIconBar::redrawIcon(uint16 iconIndex) {
	if (iconIndex >= _iconBarItems.size())
		return;

	if (_iconBarItems[iconIndex].enabled)
		drawEnabledImage(_iconBarItems[iconIndex].nonSelectedImage, _iconBarItems[iconIndex].rect);
	else
		drawDisabledImage(_iconBarItems[iconIndex].nonSelectedImage, _iconBarItems[iconIndex].rect);
}

void GfxMacIconBar::drawEnabledImage(Graphics::Surface *surface, const Common::Rect &rect) {
	if (surface)
		g_system->copyRectToScreen((byte *)surface->pixels, surface->pitch, rect.left, rect.top, rect.width(), rect.height());
}

void GfxMacIconBar::drawDisabledImage(Graphics::Surface *surface, const Common::Rect &rect) {
	if (!surface)
		return;

	// Add a black checkboard pattern to the image before copying it to the screen

	Graphics::Surface newSurf;
	newSurf.copyFrom(*surface);

	for (int i = 0; i < newSurf.h; i++) {
		int startX = rect.left & 3;

		if ((i + rect.top) & 1)
			startX += 2;

		for (int j = startX; j < newSurf.w; j += 4)
			*((byte *)newSurf.getBasePtr(j, i)) = 0;
	}

	g_system->copyRectToScreen((byte *)newSurf.pixels, newSurf.pitch, rect.left, rect.top, rect.width(), rect.height());
	newSurf.free();
}

void GfxMacIconBar::drawSelectedImage(uint16 iconIndex) {
	assert(iconIndex <= _iconBarItems.size());

	// TODO
}

bool GfxMacIconBar::isIconEnabled(uint16 iconIndex) const {
	if (iconIndex >= _iconBarItems.size())
		return false;

	return _iconBarItems[iconIndex].enabled;
}

void GfxMacIconBar::setIconEnabled(uint16 iconIndex, bool enabled) {
	if (iconIndex == 0xffff) {
		for (uint32 i = 0; i < _iconBarItems.size(); i++)
			_iconBarItems[i].enabled = enabled;
	} else if (iconIndex < _iconBarItems.size()) {
		_iconBarItems[iconIndex].enabled = enabled;
	}
}

Graphics::Surface *GfxMacIconBar::createImage(uint32 iconIndex, bool isSelected) {
	Graphics::PictDecoder pictDecoder(Graphics::PixelFormat::createFormatCLUT8());
	ResourceType type = isSelected ? kResourceTypeMacIconBarPictS : kResourceTypeMacIconBarPictN;

	Resource *res = g_sci->getResMan()->findResource(ResourceId(type, iconIndex + 1), false);

	if (!res || res->size == 0)
		return 0;

	byte palette[256 * 3];
	Common::SeekableReadStream *stream = new Common::MemoryReadStream(res->data, res->size);
	Graphics::Surface *surface = pictDecoder.decodeImage(stream, palette);
	remapColors(surface, palette);

	delete stream;
	return surface;
}

void GfxMacIconBar::remapColors(Graphics::Surface *surf, byte *palette) {
	byte *pixels = (byte *)surf->pixels;

	// Remap to the screen palette
	for (uint16 i = 0; i < surf->w * surf->h; i++) {
		byte color = *pixels;

		byte r = palette[color * 3];
		byte g = palette[color * 3 + 1];
		byte b = palette[color * 3 + 2];

		*pixels++ = g_sci->_gfxPalette->findMacIconBarColor(r, g, b);
	}
}

} // End of namespace Sci
