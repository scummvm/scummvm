/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gargoyle/clipboard.h"
#include "gargoyle/gargoyle.h"
#include "gargoyle/windows.h"
#include "common/system.h"

namespace Gargoyle {

void Clipboard::store(const uint32 *text, size_t len) {
	// TODO
}

void Clipboard::send(ClipSource source) {
	// TODO
}

void Clipboard::receive(ClipSource source) {
	Windows &windows = *g_vm->_windows;

	if (g_system->hasTextInClipboard()) {
		Common::String text = g_system->getTextFromClipboard();
		for (uint idx = 0; idx < text.size(); ++idx) {
			uint c = text[idx];
			if (c != '\r' && c != '\n' && c != '\b' && c != '\t')
				windows.inputHandleKey(c);
		}
	}
}

} // End of namespace Gargoyle
