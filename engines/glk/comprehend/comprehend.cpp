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

#include "glk/comprehend/comprehend.h"
#include "glk/quetzal.h"
#include "common/config-manager.h"
#include "common/translation.h"

namespace Glk {
namespace Comprehend {

Comprehend::Comprehend(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		_saveSlot(-1) {
}

void Comprehend::runGame() {
	initialize();
	#ifdef TODO
	_bottomWindow = glk_window_open(0, 0, 0, wintype_TextBuffer, 1);
	if (_bottomWindow == nullptr) {
		glk_exit();
		return;
	}
	glk_set_window(_bottomWindow);
#endif
}

void Comprehend::initialize() {
}

} // End of namespace Comprehend
} // End of namespace Glk
