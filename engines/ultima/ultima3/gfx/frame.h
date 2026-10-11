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

#ifndef ULTIMA3_GFX_FRAME_H
#define ULTIMA3_GFX_FRAME_H

#include "graphics/views/gfx_surface.h"

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

/**
 * Draws the frame around the game screen in the VGA renderer, which is made
 * of characters from the font rather than the original's patterned cells
 */
void drawGameFrame(Graphics::Views::GfxSurface &s);

/**
 * Draws the border of the window at the bottom of the menu screens in the
 * VGA renderer
 */
void drawWindowFrame(Graphics::Views::GfxSurface &s);

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima

#endif
