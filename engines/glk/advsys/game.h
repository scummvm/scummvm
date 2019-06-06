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

#include "common/array.h"
#include "common/stream.h"

namespace Glk {
namespace AdvSys {

/**
 * Built-in variables
 */
enum Variable {
	V_ACTOR = 1,		///< Actor noun phrase number
	V_ACTION = 2,		///< Action from phrase
	V_DOBJECT = 3,		///< First direct object noun phrase number
	V_NDOBJECTS = 4,	///< Number of direct object noun phrases
	V_IOBJECT = 5,		///< Indirect object noun phrase number
	V_OCOUNT = 6		///< Total object count
};

/**
 * Data decryption
 */
class Decrypter {
public:
	/**
	 * Decrypt a data block
	 */
	static void decrypt(byte* data, size_t size);
};

/**
 * AdvSys game header
 */
class Header : public Decrypter {
public:
	bool _valid;				///< Signals whether header is valid
	size_t _size;				///< Resident size in bytes
	uint _headerVersion;		///< Header structure version
	Common::String _name;		///< Adventure name
	uint _version;				///< Adventure version			
	uint _wordTableOffset;		///< Word table offset
	uint _wordTypeTableOffset;	///< Word type table offset
	uint _objectTableOffset;	///< Object table offset
	uint _actionTableOffset;	///< Action table offset
	uint _variableTableOffset;	///< Variable table offset
	uint _dataSpaceOffset;		///< Data space offset
	uint _codeSpaceOffset;		///< Code space offset
	uint _dataBlockOffset;		///< First data block offset
	uint _messageBlockOffset;	///< First message block offset
	uint _initCodeOffset;		///< Initialization code offset
	uint _updateCodeOffset;		///< Update code offset
	uint _beforeOffset;			///< Code offset before verb handler
	uint _afterOffset;			///< Code offset after verb handler
	uint _errorHandlerOffset;	///< Error handler code offset
	uint _saveAreaOffset;		///< Save area offset
	uint _saveSize;				///< Save area size
public:
	/**
	 * Constructor
	 */
	Header() : _valid(false), _size(0), _headerVersion(0), _version(0), _wordTableOffset(0),
		_wordTypeTableOffset(0), _objectTableOffset(0), _actionTableOffset(0), _variableTableOffset(0),
		_dataSpaceOffset(0), _codeSpaceOffset(0), _dataBlockOffset(0), _messageBlockOffset(0),
		_initCodeOffset(0), _updateCodeOffset(0), _beforeOffset(0), _afterOffset(0),
		_errorHandlerOffset(0), _saveAreaOffset(0), _saveSize(0) {
	}

	/**
	 * Constructor
	 */
	Header(Common::ReadStream &s) {
		init(s);
	}

	/**
	 * init the header
	 */
	bool init(Common::ReadStream &s);
};

/**
 * Game abstraction class
 */
class Game : public Header {
public:
	Common::Array<byte> _data;
	uint _residentOffset;
	uint _wordCount;
	uint _objectCount;
	uint _actionCount;
	uint _variableCount;

	byte* _residentBase;
	byte* _wordTable;
	byte* _wordTypeTable;
	byte* _objectTable;
	byte* _actionTable;
	byte* _variableTable;
	byte* _saveArea;
	byte* _dataSpace;
	byte* _codeSpace;
public:
	/**
	 * Constructor
	 */
	Game() : Header(), _residentOffset(0), _wordCount(0), _objectCount(0), _actionCount(0),
		_variableCount(0), _residentBase(nullptr), _wordTable(nullptr),
		_wordTypeTable(nullptr), _objectTable(nullptr), _actionTable(nullptr),
		_variableTable(nullptr), _saveArea(nullptr) {}

	/**
	 * init data for the game
	 */
	bool init(Common::SeekableReadStream &s);

	/**
	 * Restore savegame data from the game to it's initial state
	 */
	void restart(Common::SeekableReadStream& s);

	/**
	 * Save the game data to a savegame
	 */
	void saveGameData(Common::WriteStream& ws);

	/**
	 * Restore the game data from a savegame
	 */
	void loadGameData(Common::ReadStream& rs);

	/**
	 * Set a variable value
	 */
	void setVariable(uint variableNum, int value);

	/**
	 * Get a variable value
	 */
	int getVariable(uint variableNum);
};

} // End of namespace AdvSys
} // End of namespace Glk

#endif
