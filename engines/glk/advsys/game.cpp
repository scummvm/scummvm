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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/advsys/game.h"
#include "common/memstream.h"

namespace Glk {
namespace AdvSys {

void Decrypter::decrypt(byte *data, size_t size) {
	for (; --size; ++data)
		*data = ~(*data + 30);
}

/*--------------------------------------------------------------------------*/

#define HEADER_SIZE 62

bool Header::load(Common::ReadStream &s) {
	_valid = false;
	byte data[HEADER_SIZE];

	// Read in the data
	if (s.read(data, HEADER_SIZE) != HEADER_SIZE)
		return false;
	decrypt(data, HEADER_SIZE);
	Common::MemoryReadStream ms(data, HEADER_SIZE, DisposeAfterUse::NO);

	// Validate the header
	_valid = !strncmp((const char*)data + 2, "ADVSYS", 6);
	if (!_valid)
		return false;
	
	_size = ms.readUint16LE();
	ms.skip(6);
	_headerVersion = ms.readUint16LE();
	_name = Common::String((const char *)data + 10, (const char *)data + 28);
	ms.skip(18);
	_version = ms.readUint16LE();
	_wordTable = ms.readUint16LE();
	_wordTypeTable = ms.readUint16LE();
	_objectTable = ms.readUint16LE();
	_actionTable = ms.readUint16LE();
	_variableTable = ms.readUint16LE();
	_dataSpace = ms.readUint16LE();
	_codeSpace = ms.readUint16LE();
	_dataBlock = ms.readUint16LE();
	_messageBlock = ms.readUint16LE();
	_initCode = ms.readUint16LE();
	_updateCode = ms.readUint16LE();
	_before = ms.readUint16LE();
	_after = ms.readUint16LE();
	_errorHandler = ms.readUint16LE();
	_saveArea = ms.readUint16LE();
	_saveSize = ms.readUint16LE();

	return true;
}

/*--------------------------------------------------------------------------*/

#define MAX_VERSION 102

bool Game::load(Common::SeekableReadStream &s) {
	// Load the header
	s.seek(0);
	if (!Header::load(s))
		return false;
	
	if (_headerVersion < 101 || _headerVersion > MAX_VERSION)
		error("Wrong version number");

	// Load the needed game data and decrypt it
	_data.resize(_size);
	if (!s.read(&_data[0], _size))
		return false;
	decrypt(&_data[0], _size);

	return true;
}

} // End of namespace AdvSys
} // End of namespace Glk
