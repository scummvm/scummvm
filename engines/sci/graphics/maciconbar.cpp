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

	for (uint32 i = 0; i < _iconBarItems.size(); i++) {
		Graphics::Surface *surface = _iconBarItems[i].nonSelectedImage;

		if (surface) {
			g_system->copyRectToScreen((byte *)surface->pixels, surface->pitch, _iconBarItems[i].rect.left,
				_iconBarItems[i].rect.top, _iconBarItems[i].rect.width(), _iconBarItems[i].rect.height());
		}
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
