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

/**
 * @brief GPL2 bytecode disassembler
 * @param gplcode A pointer to the bytecode
 *        len Length of the bytecode
 *
 * GPL2 is short for Game Programming Language 2 which is the script language
 * used by Draci Historie. This is a simple disassembler for the language.
 *
 * A compiled GPL2 program consists of a stream of bytes representing commands
 * and their parameters. The syntax is as follows:
 *
 * Syntax of a command:
 *	<name of the command> <number> <sub-number> <list of parameters...>
 *
 *	Syntax of a parameter:
 *	- 1: integer number literally passed to the program
 *	- 2-1: string stored in the reservouir of game strings (i.e. something to be
 *	  displayed) and stored as an index in this list
 *	- 2-2: string resolved by the compiler (i.e., a path to another file) and
 *	  replaced by an integer index of this entity in the appropriate namespace
 *	  (e.g., the index of the palette, location, ...)
 *	- 3-0: relative jump to a label defined in this code.  Each label must be
 *	  first declared in the beginning of the program.
 *	- 3-1 .. 3-9: index of an entity in several namespaces, defined in file ident
 *	- 4: mathematical expression compiled into a postfix format
 *
 * 	In the compiled program, parameters of type 1..3 are represented by a single
 *	16-bit integer.  The called command knows by its definition what namespace the
 *	value comes from.
 */

#include "common/debug.h"
#include "common/stream.h"

#include "draci/gpldisasm.h"
#include "draci/barchive.h"
#include "draci/draci.h"

#define skipParams(x) for(unsigned int i = 0; i < (x); ++i) reader.readUint16LELE()
#define CMDPAIR(x, y) (((x) << 8) | y)

namespace Draci {

// FIXME: Handle math expressions properly instead of just skipping them
void handleMathExpression(Common::MemoryReadStream &reader) {
	uint16 temp;
	while (1) {
		temp = reader.readUint16LE();
		if (temp == 0) {
			break;
		}
		temp = reader.readUint16LE();
	}
	return;
}

int gpldisasm(byte *gplcode, uint16 len) {
	Common::MemoryReadStream reader(gplcode, len);

	while (!reader.eos()) {
		// read in command pair
		uint16 cmdpair = reader.readUint16BE();

		uint16 param1, param2, param3;

		switch (cmdpair) {
		case CMDPAIR(5,1):
			param1 = reader.readUint16LE();
			param2 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_Load %hu %hu", param1, param2);
			break;
		case CMDPAIR(4,1):
			param1 = reader.readUint16LE();
			param2 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_Start %hu %hu", param1, param2);
			break;
		case CMDPAIR(5,2):
			param1 = reader.readUint16LE();
			param2 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_StartPlay %hu %hu",
				param1, param2);
			break;
		case CMDPAIR(5,3):
			debugC(2, kDraciBytecodeDebugLevel, "C_JustTalk");
			break;
		case CMDPAIR(5,4):
			debugC(2, kDraciBytecodeDebugLevel, "C_JustStay");
			break;
		case CMDPAIR(10,2):
			param1 = reader.readUint16LE();
			param2 = reader.readUint16LE();
			param3 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_StayOn %hu %hu %hu",
				param1, param2, param3);
			break;
		case CMDPAIR(10,1):
			param1 = reader.readUint16LE();
			param2 = reader.readUint16LE();
			param3 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_WalkOn %hu %hu %hu",
				param1, param2, param3);
			break;
		case CMDPAIR(10,3):
			param1 = reader.readUint16LE();
			param2 = reader.readUint16LE();
			param3 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_WalkOnPlay %hu %hu %hu",
				param1, param2, param3);
			break;
		case CMDPAIR(7,1):
			param1 = reader.readUint16LE();
			param2 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_ObjStat %hu %hu",
				param1, param2);
			break;
		case CMDPAIR(7,2):
			param1 = reader.readUint16LE();
			param2 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_ObjStat_On %hu %hu",
				param1, param2);
			break;
		case CMDPAIR(8,1):
			param1 = reader.readUint16LE();
			param2 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_IcoStat %hu %hu",
				param1, param2);
			break;
		case CMDPAIR(14,1):
			param1 = reader.readUint16LE();
			param2 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_NewRoom %hu %hu",
				param1, param2);
			break;
		case CMDPAIR(6,1):
			param1 = reader.readUint16LE();
			param2 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_Talk %hu %hu",
				param1, param2);
			break;
		case CMDPAIR(2,1):
			param1 = reader.readUint16LE();
			handleMathExpression(reader);
			debugC(2, kDraciBytecodeDebugLevel, "C_Let <MATHEXPR> %hu", param1);
			break;
		case CMDPAIR(15,1):
			param1 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_ExecInit %hu", param1);
			break;
		case CMDPAIR(15,2):
			param1 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_ExecLook %hu", param1);
			break;
		case CMDPAIR(15,3):
			param1 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_ExecUse %hu", param1);
			break;
		case CMDPAIR(16,1):
			debugC(2, kDraciBytecodeDebugLevel, "C_RepaintInventory");
			break;
		case CMDPAIR(16,2):
			debugC(2, kDraciBytecodeDebugLevel, "C_ExitInventory");
			break;
		case CMDPAIR(1,1):
			param1 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_goto %hu", param1);
			break;
		case CMDPAIR(3,1):
			handleMathExpression(reader);
			param1 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_if <MATHEXPR> %hu", param1);
			break;
		case CMDPAIR(9,1):
			param1 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_Dialogue %hu", param1);
			break;
		case CMDPAIR(9,2):
			debugC(2, kDraciBytecodeDebugLevel, "C_ExitDialogue");
			break;
		case CMDPAIR(9,3):
			debugC(2, kDraciBytecodeDebugLevel, "C_ResetDialogue");
			break;
		case CMDPAIR(9,4):
			debugC(2, kDraciBytecodeDebugLevel, "C_ResetDialogueFrom");
			break;
		case CMDPAIR(9,5):
			param1 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_ResetBlock %hu", param1);
			break;
		case CMDPAIR(11,1):
			param1 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_LoadPalette %hu", param1);
			break;
		case CMDPAIR(12,1):
			debugC(2, kDraciBytecodeDebugLevel, "C_SetPalette");
			break;
		case CMDPAIR(12,2):
			debugC(2, kDraciBytecodeDebugLevel, "C_BlackPalette");
			break;
		case CMDPAIR(13,1):
			param1 = reader.readUint16LE();
			param2 = reader.readUint16LE();
			param3 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_FadePalette %hu %hu %hu",
				param1, param2, param3);
			break;
		case CMDPAIR(13,2):
			param1 = reader.readUint16LE();
			param2 = reader.readUint16LE();
			param3 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_FadePalettePlay %hu %hu %hu",
				param1, param2, param3);
			break;
		case CMDPAIR(17,1):
			debugC(2, kDraciBytecodeDebugLevel, "C_ExitMap");
			break;
		case CMDPAIR(18,1):
			param1 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_LoadMusic %hu", param1);
			break;
		case CMDPAIR(18,2):
			debugC(2, kDraciBytecodeDebugLevel, "C_StartMusic");
			break;
		case CMDPAIR(18,3):
			debugC(2, kDraciBytecodeDebugLevel, "C_StopMusic");
			break;
		case CMDPAIR(18,4):
			param1 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_FadeOutMusic %hu", param1);
			break;
		case CMDPAIR(18,5):
			param1 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_FadeInMusic %hu", param1);
			break;
		case CMDPAIR(19,1):
			debugC(2, kDraciBytecodeDebugLevel, "C_Mark");
			break;
		case CMDPAIR(19,2):
			debugC(2, kDraciBytecodeDebugLevel, "C_Release");
			break;
		case CMDPAIR(20,1):
			debugC(2, kDraciBytecodeDebugLevel, "C_Play");
			break;
		case CMDPAIR(21,1):
			param1 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_LoadMap %hu", param1);
			break;
		case CMDPAIR(21,2):
			debugC(2, kDraciBytecodeDebugLevel, "C_RoomMap");
			break;
		case CMDPAIR(22,1):
			debugC(2, kDraciBytecodeDebugLevel, "C_DisableQuickHero");
			break;
		case CMDPAIR(22,2):
			debugC(2, kDraciBytecodeDebugLevel, "C_EnableQuickHero");
			break;
		case CMDPAIR(23,1):
			debugC(2, kDraciBytecodeDebugLevel, "C_DisableSpeedText");
			break;
		case CMDPAIR(23,2):
			debugC(2, kDraciBytecodeDebugLevel, "C_EnableSpeedText");
			break;
		case CMDPAIR(24,1):
			debugC(2, kDraciBytecodeDebugLevel, "C_QuitGame");
			break;
		case CMDPAIR(25,1):
			debugC(2, kDraciBytecodeDebugLevel, "C_PushNewRoom");
			break;
		case CMDPAIR(25,2):
			debugC(2, kDraciBytecodeDebugLevel, "C_PopNewRoom");
			break;
		case CMDPAIR(26,1):
			debugC(2, kDraciBytecodeDebugLevel, "C_ShowCheat");
			break;
		case CMDPAIR(26,2):
			debugC(2, kDraciBytecodeDebugLevel, "C_HideCheat");
			break;
		case CMDPAIR(26,3):
			param1 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_ClearCheat %hu", param1);
			break;
		case CMDPAIR(27,1):
			param1 = reader.readUint16LE();
			param2 = reader.readUint16LE();
			param3 = reader.readUint16LE();
			debugC(2, kDraciBytecodeDebugLevel, "C_FeedPassword %hu %hu %hu",
				param1, param2, param3);
			break;
		case CMDPAIR(0, 0):
			debugC(2, kDraciBytecodeDebugLevel, "gplend");
			break;
		case CMDPAIR(0, 1):
			debugC(2, kDraciBytecodeDebugLevel, "exit");
			break;
		default:
			debugC(2, kDraciBytecodeDebugLevel, "Unknown opcode %hu, %hu",
			(cmdpair >> 8) & 0xFF, cmdpair & 0xFF);
		}
	}

	return 0;
}

}

