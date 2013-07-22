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
 */

#include "fullpipe/fullpipe.h"

#include "fullpipe/objects.h"
#include "fullpipe/input.h"

namespace Fullpipe {

CInputController::CInputController() {
	g_fullpipe->_inputController = this;

	_flag = 0;
	_cursorHandle = 0;
	_hCursor = 0;
	_field_14 = 0;
	_cursorId = 0;
	_cursorIndex = -1;
	_flags = 1;

	_cursorBounds.left = 0;
	_cursorBounds.top = 0;
	_cursorBounds.right = 0;
	_cursorBounds.bottom = 0;

	_cursorItemPicture = 0;
}

void CInputController::setInputDisabled(bool state) {
	_flag = state;
	g_fullpipe->_inputDisabled = state;
}

void setInputDisabled(bool state) {
	g_fullpipe->_inputController->setInputDisabled(state);
}

} // End of namespace Fullpipe
