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
 * $URL$
 * $Id$
 *
 */

#include "common/stream.h"

#include "draci/draci.h"
#include "draci/game.h"
#include "draci/barchive.h"

namespace Draci {

Game::Game() {
	Common::String path("INIT.DFW");
	
	BArchive initArchive(path);
	BAFile *file;
	
	file = initArchive[5];
	Common::MemoryReadStream reader(file->_data, file->_length);
	
	unsigned int numPersons = file->_length / personSize;
	_persons = new Person[numPersons];
	
	for (unsigned int i = 0; i < numPersons; ++i) {
		_persons[i]._x = reader.readByte();
		_persons[i]._y = reader.readByte();
		_persons[i]._fontColour = reader.readUint16LE();
	}
}

} 
