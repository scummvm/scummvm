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
#include "graphics/palette.h"

#include "chewy/graphics.h"
#include "chewy/resource.h"

namespace Chewy {

void Graphics::drawImage(Common::String filename, int imageNum) {
	Resource *res = new Resource("comic.tgp");
	TBFChunk *cur = res->getChunk(0);
	byte *buf = res->getChunkData(0);

	g_system->getPaletteManager()->setPalette(cur->palette, 0, 256);
	g_system->copyRectToScreen(buf, cur->width, 0, 0, cur->width, cur->height);
	g_system->updateScreen();
	delete res;
}

} // End of namespace Chewy
