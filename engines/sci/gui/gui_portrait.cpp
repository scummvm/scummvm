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

#include "common/util.h"
#include "common/stack.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_palette.h"
#include "sci/gui/gui_portrait.h"

namespace Sci {

SciGuiPortrait::SciGuiPortrait(ResourceManager *resMan, SciGuiScreen *screen, SciGuiPalette *palette, Common::String resourceName)
	: _resMan(resMan), _screen(screen), _palette(palette), _resourceName(resourceName) {
	init();
}

SciGuiPortrait::~SciGuiPortrait() {
}

void SciGuiPortrait::init() {
	// .BIN files are loaded from actors directory and from .\ directory
	// seem to have 98 byte header
}

} // End of namespace Sci
