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

#ifndef GLK_COMPREHEND_GAME_OPCODES_H
#define GLK_COMPREHEND_GAME_OPCODES_H

#include "glk/comprehend/game.h"

namespace Glk {
namespace Comprehend {

/**
 * Intermediate derived game class that handles script opcodes common to both
 * version 1 and version 2 of the engine
 */
class ComprehendGameOpcodes : public ComprehendGame {
protected:
	ScriptOpcode _opcodeMap[0x100];

	void execute_opcode(const Instruction *instr, const Sentence *sentence, FunctionState *func_state) override;

	Item *getItem(const Instruction *instr);

	virtual void func_set_test_result(FunctionState *func_state, bool value);
	bool isItemPresent(Item *item) const;
public:
	ComprehendGameOpcodes();

	virtual byte getOpcode(const Instruction *instr) {
		return instr->_opcode;
	}
	ScriptOpcode getScriptOpcode(const Instruction *instr) override {
		return _opcodeMap[getOpcode(instr)];
	}
};


/**
 * Version 1 Comprehend game
 */
class ComprehendGameV1 : public ComprehendGameOpcodes {
protected:
	void execute_opcode(const Instruction *instr, const Sentence *sentence, FunctionState *func_state) override;
public:
	ComprehendGameV1();
};

/**
 * Version 2 Comprehend game
 */
class ComprehendGameV2 : public ComprehendGameOpcodes {
protected:
	void execute_opcode(const Instruction *instr, const Sentence *sentence, FunctionState *func_state) override;
public:
	ComprehendGameV2();

	byte getOpcode(const Instruction *instr) override;
	void func_set_test_result(FunctionState *func_state, bool value) override;
};

} // namespace Comprehend
} // namespace Glk

#endif
