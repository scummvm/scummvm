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

#include "common/stream.h"
#include "common/system.h"
#include "graphics/pict.h"
#include "graphics/surface.h"

namespace Sci {

void GfxMacIconBar::addIcon(reg_t obj) {
	_iconBarObjects.push_back(obj);
}

void GfxMacIconBar::drawIcons() {
	// Draw the icons to the bottom of the screen

	byte *pal = new byte[256 * 4];
	Graphics::PictDecoder *pict = new Graphics::PictDecoder(Graphics::PixelFormat::createFormatCLUT8());
	uint32 lastX = 0;

	for (uint32 i = 0; i < _iconBarObjects.size(); i++) {
		uint32 iconIndex = readSelectorValue(g_sci->getEngineState()->_segMan, _iconBarObjects[i], SELECTOR(iconIndex));
		Resource *res = g_sci->getResMan()->findResource(ResourceId(kResourceTypeMacIconBarPictN, iconIndex + 1), false);
		if (!res)
			continue;

		Common::MemoryReadStream *stream = new Common::MemoryReadStream(res->data, res->size);
		Graphics::Surface *surf = pict->decodeImage(stream, pal);
		remapColors(surf, pal);

		g_system->copyRectToScreen((byte *)surf->pixels, surf->pitch, lastX, 200, MIN<uint32>(surf->w, 320 - lastX), surf->h);

		lastX += surf->w;
		surf->free();
		delete surf;
		delete stream;
	}

	delete pict;
	delete[] pal;
}

void GfxMacIconBar::remapColors(Graphics::Surface *surf, byte *palette) {
	byte *pixels = (byte *)surf->pixels;

	// Remap to the screen palette
	for (uint16 i = 0; i < surf->w * surf->h; i++) {
		byte color = *pixels;

		byte r = palette[color * 4];
		byte g = palette[color * 4 + 1];
		byte b = palette[color * 4 + 2];

		// For black, make sure the index is 0
		if (r == 0 && g == 0 && b == 0)
			*pixels++ = 0;
		else
			*pixels++ = g_sci->_gfxPalette->kernelFindColor(r, g, b);
	}
}

} // End of namespace Sci
