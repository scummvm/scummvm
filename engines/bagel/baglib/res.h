
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

#ifndef BAGEL_BAGLIB_RES_H
#define BAGEL_BAGLIB_RES_H

namespace Bagel {

// This section should not change, the first cursor is always invalid
#define BOFINVALID 0

// OBJ
// Lists object types
#define BOFBMPOBJ 1
#define BOFSPRITEOBJ 2
#define BOFBUTTONOBJ 3
#define BOFSOUNDOBJ 4
#define BOFLINKOBJ 5
#define BOFTEXTOBJ 6
#define BOFCHAROBJ 7
#define BOFVAROBJ 8
#define BOFAREAOBJ 9
#define BOFMOVIEOBJ 11
#define BOFCOMMOBJ 12
#define BOFEXPRESSOBJ 13
#define BOFTHINGOBJ 14
#define BOFRESPRNTOBJ 15
#define BOFDOSSIEROBJ 16

// PAN
// These CANNOT change

#define BOFLTCURSOR 14
#define BOFUPCURSOR 15
#define BOFRTCURSOR 16
#define BOFDNCURSOR 17
#define BOFTAKEHAND 18
#define BOFHAND 19

} // namespace Bagel

#endif
