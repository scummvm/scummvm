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

#ifndef HODJNPODJ_CRYPT_GLOBALS_H
#define HODJNPODJ_CRYPT_GLOBALS_H

namespace Bagel {
namespace HodjNPodj {
namespace Crypt {

// Main Window positioning constants
#define GAME_WIDTH          640
#define GAME_HEIGHT         480

// Cryptogram info
#define ALPHABET            26  // chars in English alphabet
#define SYMBOLS             45  // chars + 9 symbols + 10 digits
#define USED_SYMBOLS        26
#define REVEAL_SYMBOLS      26

// Scroll button
#define IDC_SCROLL          800

#define SCROLL_BUTTON_X     250
#define SCROLL_BUTTON_Y     0
#define SCROLL_BUTTON_DX    140
#define SCROLL_BUTTON_DY    23

} // namespace Crypt
} // namespace HodjNPodj
} // namespace Bagel

#endif
