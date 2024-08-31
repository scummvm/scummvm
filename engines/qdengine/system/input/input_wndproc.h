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

#ifndef QDENGINE_SYSTEM_INPUT_INPUT_WNDPROC_H
#define QDENGINE_SYSTEM_INPUT_INPUT_WNDPROC_H

namespace Common {
struct Event;
}

namespace QDEngine {

class mouseDispatcher;
class keyboardDispatcher;
namespace input {

//! Обработка сообщений ввода с клавиатуры.
/**
Возвращает true, если сообщение обработано.

Обрабатываемые сообщения:
WM_KEYDOWN
WM_KEYUP
WM_SYSKEYDOWN
WM_SYSKEYUP
*/
bool keyboard_wndproc(const Common::Event &event, keyboardDispatcher *dsp);
//! Обработка сообщений мыши.
/**
Возвращает true, если сообщение обработано.

Обрабатываемые сообщения:
WM_MOUSEMOVE
WM_LBUTTONDOWN
WM_RBUTTONDOWN
WM_LBUTTONUP
WM_RBUTTONUP
*/
bool mouse_wndproc(const Common::Event &event, mouseDispatcher *dsp);
}

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_INPUT_INPUT_WNDPROC_H
