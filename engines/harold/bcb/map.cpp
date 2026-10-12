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

#include "common/algorithm.h"
#include "common/fs.h"
#include "common/ustr.h"
#include "graphics/font.h"
#include "graphics/sjis.h"
#include "graphics/surface.h"
#include "video/avi_decoder.h"

#include "harold/bcb/bcb.h"
#include "harold/bcb/script.h"
#include "harold/bcb/static_data.h"

#include "common/textconsole.h"

namespace BCB {

static const uint32 kScriptSecondaryStateByteOffset = 0x98;
void BCBEngine::composeMapControlsToSurface(Graphics::Surface &surface, const Graphics::Surface &mapArt) {
	error("BCB: composeMapControlsToSurface is not implemented");
}

} // End of namespace BCB
