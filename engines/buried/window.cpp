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

#include "common/textconsole.h"

#include "buried/message.h"
#include "buried/window.h"

namespace Buried {

void Window::dispatchMessage() {
	if (_queue.empty())
		return;

	Message *message = _queue.pop();

	switch (message->getMessageType()) {
	case kMessageTypeEraseBackground:
		onEraseBackground();
		break;
	case kMessageTypeKeyUp:
		onKeyUp(((KeyUpMessage *)message)->getKeyState(), ((KeyUpMessage *)message)->getFlags());
		break;
	case kMessageTypeKeyDown:
		onKeyDown(((KeyDownMessage *)message)->getKeyState(), ((KeyDownMessage *)message)->getFlags());
		break;
	case kMessageTypeTimer:
		onTimer(((TimerMessage *)message)->getTimer());
		break;
	case kMessageTypeSetFocus:
		onSetFocus(((SetFocusMessage *)message)->getWindow());
		break;
	case kMessageTypeKillFocus:
		onKillFocus(((KillFocusMessage *)message)->getWindow());
		break;
	case kMessageTypeQueryNewPalette:
		onQueryNewPalette();
		break;
	case kMessageTypePaint:
		onPaint();
		break;
	case kMessageTypeLButtonUp:
		onLButtonUp(((LButtonUpMessage *)message)->getPoint(), ((LButtonUpMessage *)message)->getFlags());
		break;
	case kMessageTypeLButtonDown:
		onLButtonDown(((LButtonDownMessage *)message)->getPoint(), ((LButtonDownMessage *)message)->getFlags());
		break;
	case kMessageTypeLButtonDoubleClick:
		onLButtonDoubleClick(((LButtonDoubleClickMessage *)message)->getPoint(), ((LButtonDoubleClickMessage *)message)->getFlags());
		break;
	case kMessageTypeMButtonUp:
		onMButtonUp(((MButtonUpMessage *)message)->getPoint(), ((MButtonUpMessage *)message)->getFlags());
		break;
	case kMessageTypeRButtonUp:
		onRButtonUp(((RButtonUpMessage *)message)->getPoint(), ((RButtonUpMessage *)message)->getFlags());
		break;
	case kMessageTypeRButtonDown:
		onRButtonDown(((RButtonDownMessage *)message)->getPoint(), ((RButtonDownMessage *)message)->getFlags());
		break;
	case kMessageTypeSetCursor:
		onSetCursor(((SetCursorMessage *)message)->getWindow(), ((SetCursorMessage *)message)->getCursor());
		break;
	case kMessageTypeEnable:
		onEnable(((EnableMessage *)message)->getEnable());
		break;
	default:
		error("Unknown message type %d", message->getMessageType());
	}

	delete message;
}

} // End of namespace Buried
