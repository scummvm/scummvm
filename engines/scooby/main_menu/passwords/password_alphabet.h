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

#ifndef SCOOBY_PASSWORD_ALPHABET_H
#define SCOOBY_PASSWORD_ALPHABET_H

// Owns the fixed symbol ordering shared by password display, entry, and validation.

namespace Scooby {
// The exact 32-symbol executable lookup used for five-bit password values.
// Original Ghidra global g_abPasswordAlphabet at 0x0000871E-0x0000873D.
static const char kSymbols[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ!+#%*?";
} // namespace Scooby

#endif // SCOOBY_PASSWORD_ALPHABET_H
