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

#ifndef LASTEXPRESS_HELPERS_H
#define LASTEXPRESS_HELPERS_H

// Misc
#define rnd(value) _engine->getRandom().getRandomNumber(value - 1)

// Logic
#define getCharacter(c) (_engine->_characters->characters[c])
#define getCharacterCurrentParams(c) (getCharacter(c).callParams[getCharacter(c).currentCall].parameters)
#define getCharacterParams(c, n) (getCharacter(c).callParams[n].parameters)

// Helpers
#define SAFE_DELETE(_p) do { if (_p) delete (_p); (_p) = nullptr; } while (false)
#define SAFE_DELETE_ARR(_p) do { if (_p) delete[] (_p); (_p) = nullptr; } while (false)
#define SAFE_FREE(_p)   do { if (_p) free   (_p); (_p) = nullptr; } while (false)

#endif // LASTEXPRESS_HELPERS_H
