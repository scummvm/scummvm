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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "wage/wage.h"
#include "wage/entities.h"
#include "wage/script.h"
#include "wage/world.h"

#include "common/stream.h"

namespace Wage {

bool Script::execute(World *world, int loopCount, String *inputText, Designed *inputClick, WageEngine *callbacks) {
	_world = world;
	_loopCount = loopCount;
	_inputText = inputText;
	_inputClick = inputClick;
	_callbacks = callbacks;
	_handled = false;

	_data->skip(12);
	while (_data->pos() < _data->size()) {
		switch(_data->readByte()) {
		case 0x80: // IF
			processIf();
			break;
		case 0x87: // EXIT
			debug(0, "exit at offset %d", _data->pos() - 1);

			return true;
		case 0x89: // MOVE
			{
				Scene *currentScene = _world->_player->_currentScene;
				processMove();
				if (_world->_player->_currentScene != currentScene)
					return true;
				break;
			}
		case 0x8B: // PRINT
			{
				Operand *op = readOperand();
				// TODO check op type is string or number, or something good...
				appendText(op->_str);
				byte d = _data->readByte();
				if (d != 0xFD)
					warning("Operand 0x8B (PRINT) End Byte != 0xFD");
				break;
			}
		case 0x8C: // SOUND
			{
				Operand *op = readOperand();
				// TODO check op type is string.
				_handled = true;
				callbacks->playSound(op->_str);
				byte d = _data->readByte();
				if (d != 0xFD)
					warning("Operand 0x8B (PRINT) End Byte != 0xFD");
				break;
			}
		case 0x8E: // LET
			processLet();
			break;
		case 0x95: // MENU
			{
				Operand *op = readStringOperand(); // allows empty menu
				// TODO check op type is string.
				_callbacks->setMenu(op->_str);
				byte d = _data->readByte();
				if (d != 0xFD)
					warning("Operand 0x8B (PRINT) End Byte != 0xFD");
			}
		case 0x88: // END
			break;
		default:
			debug(0, "Unknown opcode: %d", _data->pos());
		}
	}


	if (_world->_globalScript != this) {
		debug(0, "Executing global script...");
		bool globalHandled = _world->_globalScript->execute(_world, _loopCount, _inputText, _inputClick, _callbacks);
		if (globalHandled)
			_handled = true;
	}

#if 0
	else if (inputText != null) {
		String input = inputText.toLowerCase();
		if (input.equals("n") || input.contains("north")) {
			handleMoveCommand(Scene.NORTH, "north");
		} else if (input.equals("e") || input.contains("east")) {
			handleMoveCommand(Scene.EAST, "east");
		} else if (input.equals("s") || input.contains("south")) {
			handleMoveCommand(Scene.SOUTH, "south");
		} else if (input.equals("w") || input.contains("west")) {
			handleMoveCommand(Scene.WEST, "west");
		} else if (input.startsWith("take ")) {
			handleTakeCommand(input.substring(5));
		} else if (input.startsWith("get ")) {
			handleTakeCommand(input.substring(4));
		} else if (input.startsWith("pick up ")) {
			handleTakeCommand(input.substring(8));
		} else if (input.startsWith("drop ")) {
			handleDropCommand(input.substring(5));
		} else if (input.startsWith("aim ")) {
			handleAimCommand(input.substring(4));
		} else if (input.startsWith("wear ")) {
			handleWearCommand(input.substring(5));
		} else if (input.startsWith("put on ")) {
			handleWearCommand(input.substring(7));
		} else if (input.startsWith("offer ")) {
			handleOfferCommand(input.substring(6));
		} else if (input.contains("look")) {
			handleLookCommand();
		} else if (input.contains("inventory")) {
			handleInventoryCommand();
		} else if (input.contains("status")) {
			handleStatusCommand();
		} else if (input.contains("rest") || input.equals("wait")) {
			handleRestCommand();
		} else if (callbacks.getOffer() != null && input.contains("accept")) {
			handleAcceptCommand();
		} else {
			Chr player = world.getPlayer();
			for (Weapon weapon : player.getWeapons()) {
				if (tryAttack(weapon, input)) {
					handleAttack(weapon);
					break;
				}
			}
		}
	// TODO: weapons, offer, etc...
	} else if (inputClick instanceof Obj) {
		Obj obj = (Obj) inputClick;
		if (obj.getType() != Obj.IMMOBILE_OBJECT) {
			takeObj(obj);
		} else {
			appendText(obj.getClickMessage());
		}
	}
#endif

	return _handled;
}
	
Script::Operand *Script::readOperand() {
	byte operandType = _data->readByte();
	switch (operandType) {
	case 0xA0: // TEXT$
		return new Operand(_inputText, Operand::TEXT_INPUT);
	case 0xA1:
		return new Operand(_inputClick, Operand::CLICK_INPUT);
	case 0xC0: // STORAGE@
		return new Operand(&_world->_storageScene, Operand::SCENE);
	case 0xC1: // SCENE@
		return new Operand(_world->_player->_currentScene, Operand::SCENE);
	case 0xC2: // PLAYER@
		return new Operand(_world->_player, Operand::CHR);
	case 0xC3: // MONSTER@
		return new Operand(_callbacks->_monster, Operand::CHR);
	case 0xC4: // RANDOMSCN@
		return new Operand(_world->_orderedScenes[_callbacks->_rnd->getRandomNumber(_world->_orderedScenes.size())], Operand::SCENE);
	case 0xC5: // RANDOMCHR@
		return new Operand(_world->_orderedChrs[_callbacks->_rnd->getRandomNumber(_world->_orderedChrs.size())], Operand::CHR);
	case 0xC6: // RANDOMOBJ@
		return new Operand(_world->_orderedObjs[_callbacks->_rnd->getRandomNumber(_world->_orderedObjs.size())], Operand::OBJ);
	case 0xB0: // VISITS#
		return new Operand(_world->_player->_context._visits, Operand::NUMBER);
	case 0xB1: // RANDOM# for Star Trek, but VISITS# for some other games?
		return new Operand(1 + _callbacks->_rnd->getRandomNumber(100), Operand::NUMBER);
	case 0xB5: // RANDOM# // A random number between 1 and 100.
		return new Operand(1 + _callbacks->_rnd->getRandomNumber(100), Operand::NUMBER);
	case 0xB2: // LOOP#
		return new Operand(_loopCount, Operand::NUMBER);
	case 0xB3: // VICTORY#
		return new Operand(_world->_player->_context._kills, Operand::NUMBER);
	case 0xB4: // BADCOPY#
		return new Operand(0, Operand::NUMBER); // ????
	case 0xFF:
		{
			// user variable
			int value = _data->readByte();
			if (value < 0)
				value += 256;

			// TODO: Verify that we're using the right index.
			return new Operand(_world->_player->_context._userVariables[value], Operand::NUMBER);
		}
	case 0xD0:
		return new Operand(_world->_player->_context._statVariables[Context::PHYS_STR_BAS], Operand::NUMBER);
	case 0xD1:
		return new Operand(_world->_player->_context._statVariables[Context::PHYS_HIT_BAS], Operand::NUMBER);
	case 0xD2:
		return new Operand(_world->_player->_context._statVariables[Context::PHYS_ARM_BAS], Operand::NUMBER);
	case 0xD3:
		return new Operand(_world->_player->_context._statVariables[Context::PHYS_ACC_BAS], Operand::NUMBER);
	case 0xD4:
		return new Operand(_world->_player->_context._statVariables[Context::SPIR_STR_BAS], Operand::NUMBER);
	case 0xD5:
		return new Operand(_world->_player->_context._statVariables[Context::SPIR_HIT_BAS], Operand::NUMBER);
	case 0xD6:
		return new Operand(_world->_player->_context._statVariables[Context::SPIR_ARM_BAS], Operand::NUMBER);
	case 0xD7:
		return new Operand(_world->_player->_context._statVariables[Context::SPIR_ACC_BAS], Operand::NUMBER);
	case 0xD8:
		return new Operand(_world->_player->_context._statVariables[Context::PHYS_SPE_BAS], Operand::NUMBER);
	case 0xE0:
		return new Operand(_world->_player->_context._statVariables[Context::PHYS_STR_CUR], Operand::NUMBER);
	case 0xE1:
		return new Operand(_world->_player->_context._statVariables[Context::PHYS_HIT_CUR], Operand::NUMBER);
	case 0xE2:
		return new Operand(_world->_player->_context._statVariables[Context::PHYS_ARM_CUR], Operand::NUMBER);
	case 0xE3:
		return new Operand(_world->_player->_context._statVariables[Context::PHYS_ACC_CUR], Operand::NUMBER);
	case 0xE4:
		return new Operand(_world->_player->_context._statVariables[Context::SPIR_STR_CUR], Operand::NUMBER);
	case 0xE5:
		return new Operand(_world->_player->_context._statVariables[Context::SPIR_HIT_CUR], Operand::NUMBER);
	case 0xE6:
		return new Operand(_world->_player->_context._statVariables[Context::SPIR_ARM_CUR], Operand::NUMBER);
	case 0xE7:
		return new Operand(_world->_player->_context._statVariables[Context::SPIR_ACC_CUR], Operand::NUMBER);
	case 0xE8:
		return new Operand(_world->_player->_context._statVariables[Context::PHYS_SPE_CUR], Operand::NUMBER);
	default:
		if (operandType >= 0x20 && operandType < 0x80) {
			return readStringOperand();
		} else {
			debug("Dunno what %x is (index=%d)!\n", operandType, _data->pos()-1);
		}
		return NULL;
	}
}

Script::Operand *Script::readStringOperand() {
	String *sb;
	bool allDigits = true;

	sb = new String();

	byte c = 0x20;
	while (c >= 0x20 && c < 0x80) {
		c = _data->readByte();
		if (c < '0' || c > '9')
			allDigits = false;
		*sb += c;
	}

	if (allDigits && sb->size() > 0) {
		debug(0, "Read number %s", sb->c_str());
		int r = atol(sb->c_str());
		delete sb;

		return new Operand(r, Operand::NUMBER);
	} else {
		// TODO: This string could be a room name or something like that.
		debug(0, "Read string %s", sb->c_str());
		return new Operand(sb, Operand::STRING);
	}
}

void Script::processIf() {
}

void Script::processMove() {
}

void Script::processLet() {
}

void Script::appendText(String str) {
	_handled = true;
	_callbacks->appendText(str);
}

} // End of namespace Wage
