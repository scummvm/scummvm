/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "queen/resource.h"

namespace Queen {

//English Floppy Demo v1
const GameVersion QueenResource::_gameVersionPE100v1 = {
	"PE100",
	155,
	true,
	true,
	_resourceTablePE100v1
};

//English Floppy Demo v2
const GameVersion QueenResource::_gameVersionPE100v2 = {
	"PE100",
	155,
	true,
	true,
	_resourceTablePE100v2
};

//English Floppy
const GameVersion QueenResource::_gameVersionPEM10 = {
	"PEM10",
	1076,
	true,
	false,
	_resourceTablePEM10
};

//English CD Talkie
const GameVersion QueenResource::_gameVersionCEM10 = {
	"CEM10",
	7671,
	false,
	false,
	_resourceTableCEM10
};

} // End of namespace Queen
