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

#ifndef GLK_COMPREHEND_DEBUGGER_DUMPER_H
#define GLK_COMPREHEND_DEBUGGER_DUMPER_H

#include "common/hashmap.h"
#include "common/str-array.h"

namespace Glk {
namespace Comprehend {

class ComprehendGame;
struct FunctionState;
struct Instruction;
struct Word;

class DebuggerDumper {
private:
	Common::HashMap<byte, Common::String> _opcodes;
	ComprehendGame *_game;

private:
	void dumpFunctions();
	void dumpFunction(uint functionNum);
	void dumpActionTable();
	static int wordIndexCompare(const Word &a, const Word &b);
	void dumpDictionary();
	void dumpWordMap();
	void dumpRooms();
	void dumpItems();
	void dumpStringTable(Common::StringArray &table);
	void dumpGameDataStrings();
	void dumpExtraStrings();
	void dumpReplaceWords();
	void dumpHeader();
	void dumpState();

protected:
	/**
	 * Prints out dumped text
	 */
	virtual void print(const char *fmt, ...) = 0;

public:
	DebuggerDumper();
	virtual ~DebuggerDumper() {}

	Common::String dumpInstruction(ComprehendGame *game,
		const FunctionState *func_state, const Instruction *instr);

	bool dumpGameData(ComprehendGame *game, const Common::String &type,
		int param = 0);
};

} // namespace Comprehend
} // namespace Glk

#endif
