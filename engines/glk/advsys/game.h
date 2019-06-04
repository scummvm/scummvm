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

#ifndef GLK_ADVSYS_GAME
#define GLK_ADVSYS_GAME

#include "common/stream.h"

namespace Glk {
namespace AdvSys {

/**
 * Decompressor
 */
struct Compression {
	/**
	 * Decompress a data block
	 */
	static void decompress(byte* data, size_t size);
};

/**
 * AdvSys game header
 */
struct Header {
	bool _valid;			///< Signals whether header is valid
	size_t _size;			///< Header size in bytes
	uint _headerVersion;	///< Header structure version
	Common::String _name;	///< Adventure name
	uint _version;			///< Adventure version			
	uint _wordTable;		///< Word table offset
	uint _wordTypeTable;	///< Word type table offset
	uint _objectTable;		///< Object table offset
	uint _actionTable;		///< Action table offset
	uint _variableTable;	///< Variable table offset
	uint _dataSpace;		///< Data space offset
	uint _codeSpace;		///< Code space offset
	uint _dataBlock;		///< First data block offset
	uint _messageBlock;		///< First message block offset
	uint _initCode;			///< Initialization code offset
	uint _updateCode;		///< Update code offset
	uint _before;			///< Code offset before verb handler
	uint _after;			///< Code offset after verb handler
	uint _errorHandler;		///< Error handler code offset
	uint _saveArea;			///< Save area offset
	uint _saveSize;			///< Save area size

	/**
	 * Constructor
	 */
	Header() : _valid(false), _size(0), _headerVersion(0), _version(0), _wordTable(0),
		_wordTypeTable(0), _objectTable(0), _actionTable(0), _variableTable(0),
		_dataSpace(0), _codeSpace(0), _dataBlock(0), _messageBlock(0), _initCode(0),
		_updateCode(0), _before(0), _after(0), _errorHandler(0), _saveArea(0), _saveSize(0) {
	}

	/**
	 * Constructor
	 */
	Header(Common::ReadStream* s) {
		load(s);
	}

	/**
	 * Load the header
	 */
	void load(Common::ReadStream *s);
};

} // End of namespace AdvSys
} // End of namespace Glk

#endif
