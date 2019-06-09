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

#define NIL 0

/**
 * Actions
 */
enum Action {
	A_VERBS = 0,
	A_PREPOSITIONS = 2,
	A_FLAG = 4,
	A_MASK = 5,
	A_CODE = 6,
	A_SIZE = 8
};

/**
 * Object fields
 */
enum ObjectField {
	O_CLASS = 0,
	O_NOUNS = 2,
	O_ADJECTIVES = 4,
	O_NPROPERTIES = 6,
	O_PROPERTIES = 8,
	O_SIZE = 8
};

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
private:
	bool _restartFlag;
private:
	/**
	 * Find an object property field
	 */
	int findProperty(int obj, int prop) const;

	/**
	 * Returns true if an object has a given noun
	 */
	bool hasNoun(int obj, int noun) const;

	/**
	 * Returns true if an object has a given adjective
	 */
	bool hasAdjective(int obj, int adjective) const;

	/**
	 * Returns true if an action has a given verb
	 */
	bool hasVerb(int act, int* verbs) const;

	/**
	 * Returns true if an action is in a given list
	 */
	bool hasPreposition(int act, int preposition) const {
		return inList(getActionField(act, A_PREPOSITIONS), preposition);
	}

	/**
	 * Check if a word is in an element of a given list
	 */
	bool inList(int link, int word) const;
public:
	Common::Array<byte> _data;
	int _residentOffset;
	int _wordCount;
	int _objectCount;
	int _actionCount;
	int _variableCount;

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
	Game() : Header(), _restartFlag(false), _residentOffset(0), _wordCount(0), _objectCount(0),
		_actionCount(0), _variableCount(0), _residentBase(nullptr), _wordTable(nullptr),
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
	 * Returns true if the game is restarting, and resets the flag
	 */
	bool shouldRestart();

	/**
	 * Save the game data to a savegame
	 */
	void saveGameData(Common::WriteStream& ws);

	/**
	 * Restore the game data from a savegame
	 */
	void loadGameData(Common::ReadStream& rs);

	/**
	 * Find a word in the dictionary
	 */
	int findWord(const Common::String& word) const;

	/**
	 * Return a word's type
	 */
	int getWordType(int word) const { return _wordTypeTable[word]; }

	/**
	 * Match an object against a name and list of adjectives
	 */
	bool match(int obj, int noun, int* adjectives);

	/**
	 * Check to see if this is a valid verb
	 */
	int checkVerb(int* verbs);

	/**
	 * Find an action matching a given description
	 */
	int findAction(int* verbs, int preposition, int flag);

	/**
	 * Get an object property
	 */
	int getObjectProperty(int obj, int prop);

	/**
	 * Sets an object property
	 */
	void setObjectProperty(int obj, int prop, int val);

	/**
	 * Gets a field from an object
	 */
	int getObjectField(int obj, int offset) const {
		return READ_LE_UINT16(_residentBase + getObjectLocation(obj) + offset);
	}

	/**
	 * Sets a field in an object
	 */
	void setObjectField(int obj, int offset, int val) {
		WRITE_LE_UINT16(_residentBase + getObjectLocation(obj) + offset, val);
	}

	/**
	 * Gets a field from an action
	 */
	int getActionField(int action, int offset) const {
		return READ_LE_UINT16(_residentBase + getActionLocation(action) + offset);
	}

	/**
	 * Gets a byte field from an action
	 */
	int getActionByte(int action, int offset) const {
		return _residentBase[getActionLocation(action) + offset];
	}

	/**
	 * Gets the offset of an object from the object table
	 */
	int getObjectLocation(int obj) const;

	/**
	 * Gets the offset of an action from the action table
	 */
	int getActionLocation(int action) const;

	/**
	 * Get a variable value
	 */
	int getVariable(int variableNum);

	/**
	 * Set a variable value
	 */
	void setVariable(int variableNum, int value);

	/**
	 * Gets a code byte
	 */
	int getCodeByte(int offset) const {
		return _codeSpace[offset];
	}

	/**
	 * Gets a code byte
	 */
	int getCodeWord(int offset) const {
		return READ_LE_UINT16(_codeSpace + offset);
	}

	/**
	 * Read a word
	 */
	int readWord(int offset) const {
		return READ_LE_UINT16(_residentBase + offset);
	}

	/**
	 * Write a word
	 */
	void writeWord(int offset, int val) {
		WRITE_LE_UINT16(_residentBase + offset, val);
	}
};

} // End of namespace AdvSys
} // End of namespace Glk

#endif
