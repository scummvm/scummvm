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

#ifndef TESORO_CORE_SCRIPTBLOCK_H
#define TESORO_CORE_SCRIPTBLOCK_H

#include "common/array.h"
#include "common/endian.h"
#include "common/scummsys.h"

namespace Tesoro {

class SpeechTables;

class StdioText;

enum ScriptOp {
	kScriptReturn = 0,		// ends the script
	kScriptSay = 1,			// a spoken line: phrase number, second and third number
	kScriptSleep = 2,		// wait this many milliseconds
	kScriptBreak = 3,		// back to the menu being shown, or the end
	kScriptJump = 4,	// jump to label
	kScriptMenuBegin = 5,
	kScriptMenuItem = 6,
	kScriptIfEquals = 7,	// jump when a variable has a value
	kScriptRun = 8,			// start another block at a label
	kScriptSendMessage = 9,	// an order for an object: animation, show, activate, look, walk
	kScriptEnd = 10,		// the closing line of the block
	kScriptSetVariable = 11,
	kScriptDo = 12,			// call the block type's own routine with the ordinal of this line
	kScriptPlayVideo = 13,
	kScriptPlaySound = 14,
	kScriptPlayTrack = 15
};

enum ScriptMessage {
	kMessagePlay = 0,	// bytes: first frame, last frame, mode (0 stop, 1 play, 2 loop, 3 wait)
	kMessageWalk = 1,	// shorts: x, y
	kMessageLook = 2,	// byte: 1 up, 2 down, 3 left, 4 right
	kMessageActivate = 3,	// byte: on/off
	kMessageShow = 4		// byte: on/off
};

/** One command, kept in the original 24 byte layout; which fields are used depends on the command. */
struct ScriptEntry {
	static const uint kSize = 24;
	byte data[kSize];

	ScriptEntry() { memset(data, 0, kSize); }
	uint32 op() const { return READ_LE_UINT32(data); }
	uint32 word(uint offset) const { return READ_LE_UINT32(data + offset); }
	uint16 half(uint offset) const { return READ_LE_UINT16(data + offset); }
	byte byteAt(uint offset) const { return data[offset]; }
	uint64 key(uint offset) const { return (uint64(word(offset + 4)) << 32) | word(offset); }
	void setWord(uint offset, uint32 v) { WRITE_LE_UINT32(data + offset, v); }
	void setHalf(uint offset, uint16 v) { WRITE_LE_UINT16(data + offset, v); }
	void setKey(uint offset, uint64 k) { setWord(offset, uint32(k)); setWord(offset + 4, uint32(k >> 32)); }
};

/** The reference a script makes to the inventory level; the other levels are 1 + their index. */
enum { kInventoryRef = 1000 };


/** A named script: the characters that speak in it, its commands and its labels.
 * Example script
 *
 * .FANMTRONO 20 // name of the block of type 20
 * 3	// 3 characters, names bellow
 * JIM
 * ORZOGUEI
 * JIM2
 * _ir // label ir
 * DO ir // run native routine with type 20, ordinal 0
 * return
 * _volver // label volver
 * DO volver // run native routine with type 20, ordinal 1
 * return
 * .END
 */
class ScriptBlock {
public:
	struct Label {
		uint64 key;
		uint32 entry;	// index of the first command after the label
	};

	uint64 key() const { return _key; } // e.g. FANMTRONO
	uint32 type() const { return _type; } // type in scenes routines e.g. 20
	uint32 characterCount() const { return _characters.size(); } // number of characters involved e.g. 3 in the sample script
	uint64 character(uint32 i) const { return _characters[i]; } // names of the characters.
	uint32 entryCount() const { return _entries.size(); }
	uint32 labelCount() const { return _labels.size(); }
	const ScriptEntry &entry(uint32 i) const { return _entries[i]; } // the command lines (DO ir, return, play avi etc.)
	const Label &label(uint32 i) const { return _labels[i]; } // labels defined in the script. Labels are places the script can be started from.
	/** Index of the command a label points at, or 0 if the label does not exist. */
	uint32 findLabel(uint64 key) const;

private:
	friend class ScriptLibrary;
	/* Parse a given script, starting with a .*/
	bool parse(StdioText &in, const SpeechTables &speech);

	uint64 _key = 0;
	uint32 _type = 0;
	Common::Array<uint64> _characters;
	Common::Array<ScriptEntry> _entries;
	Common::Array<Label> _labels;
};

/** All the blocks of the game's script files. */
class ScriptLibrary {
public:
	~ScriptLibrary();

	/** Compiles the blocks of one script file image. Returns false if reading stopped on an error. */
	bool load(const byte *text, uint32 size, const SpeechTables &speech);

	/** Turns names into positions once every file is loaded: jumps, menu chains and run targets. */
	void resolve();
	void clear();

	uint32 blockCount() const { return _blocks.size(); }
	const ScriptBlock &block(uint32 i) const { return *_blocks[i]; }
	/* Find a script with the given name */
	const ScriptBlock *find(uint64 key) const;

private:
	Common::Array<ScriptBlock *> _blocks;
};

} // End of namespace Tesoro

#endif
