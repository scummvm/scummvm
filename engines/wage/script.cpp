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

Script::Script(Common::SeekableReadStream *data) : _data(data) {
	convertToText();
}

Script::~Script() {
	for (int i = 0; i < _scriptText.size(); i++) {
		delete _scriptText[i];
	}
}

void Script::print() {
	for (int i = 0; i < _scriptText.size(); i++) {
		debug(4, "%d [%04x]: %s", i, _scriptText[i]->offset, _scriptText[i]->line.c_str());
	}
}

void Script::printLine(int offset) {
	for (int i = 0; i < _scriptText.size(); i++)
		if (_scriptText[i]->offset >= offset) {
			debug(4, "%d [%04x]: %s", i, _scriptText[i]->offset, _scriptText[i]->line.c_str());
			break;
		}
}

bool Script::execute(World *world, int loopCount, String *inputText, Designed *inputClick, WageEngine *callbacks) {
	_world = world;
	_loopCount = loopCount;
	_inputText = inputText;
	_inputClick = inputClick;
	_callbacks = callbacks;
	_handled = false;

	_data->seek(12);
	while (_data->pos() < _data->size()) {
		printLine(_data->pos());

		byte command = _data->readByte();

		switch(command) {
		case 0x80: // IF
			processIf();
			break;
		case 0x87: // EXIT
			debug(6, "exit at offset %d", _data->pos() - 1);

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
				appendText(op->toString());
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
				callbacks->playSound(op->toString());
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
				_callbacks->setMenu(op->toString());
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
		debug(1, "Executing global script...");
		bool globalHandled = _world->_globalScript->execute(_world, _loopCount, _inputText, _inputClick, _callbacks);
		if (globalHandled)
			_handled = true;
	} else if (inputText != NULL) {
		Common::String input(*inputText);
		input.toLowercase();
		if (input.equals("n") || input.contains("north")) {
			handleMoveCommand(Scene::NORTH, "north");
		} else if (input.equals("e") || input.contains("east")) {
			handleMoveCommand(Scene::EAST, "east");
		} else if (input.equals("s") || input.contains("south")) {
			handleMoveCommand(Scene::SOUTH, "south");
		} else if (input.equals("w") || input.contains("west")) {
			handleMoveCommand(Scene::WEST, "west");
		} else if (input.hasPrefix("take ")) {
			handleTakeCommand(&input.c_str()[5]);
		} else if (input.hasPrefix("get ")) {
			handleTakeCommand(&input.c_str()[4]);
		} else if (input.hasPrefix("pick up ")) {
			handleTakeCommand(&input.c_str()[8]);
		} else if (input.hasPrefix("drop ")) {
			handleDropCommand(&input.c_str()[5]);
		} else if (input.hasPrefix("aim ")) {
			handleAimCommand(&input.c_str()[4]);
		} else if (input.hasPrefix("wear ")) {
			handleWearCommand(&input.c_str()[5]);
		} else if (input.hasPrefix("put on ")) {
			handleWearCommand(&input.c_str()[7]);
		} else if (input.hasPrefix("offer ")) {
			handleOfferCommand(&input.c_str()[6]);
		} else if (input.contains("look")) {
			handleLookCommand();
		} else if (input.contains("inventory")) {
			handleInventoryCommand();
		} else if (input.contains("status")) {
			handleStatusCommand();
		} else if (input.contains("rest") || input.equals("wait")) {
			handleRestCommand();
		} else if (callbacks->getOffer() != NULL && input.contains("accept")) {
			handleAcceptCommand();
		} else {
			Chr *player = _world->_player;
			WeaponArray *weapons = player->getWeapons();
			for (WeaponArray::const_iterator weapon = weapons->begin(); weapon != weapons->end(); ++weapon) {
				if (tryAttack(*weapon, input)) {
					handleAttack(*weapon);
					break;
				}
			}

			delete weapons;
		}
	// TODO: weapons, offer, etc...
	} else if (_inputClick->_classType == OBJ) {
		Obj *obj = (Obj *)_inputClick;
		if (obj->_type != Obj::IMMOBILE_OBJECT) {
			takeObj(obj);
		} else {
			appendText(obj->_clickMessage);
		}
	}

	return _handled;
}

Script::Operand *Script::readOperand() {
	byte operandType = _data->readByte();

	debug(7, "%x: readOperand: 0x%x", _data->pos(), operandType);

	Context *cont = &_world->_player->_context;
	switch (operandType) {
	case 0xA0: // TEXT$
		return new Operand(_inputText, TEXT_INPUT);
	case 0xA1:
		return new Operand(_inputClick, CLICK_INPUT);
	case 0xC0: // STORAGE@
		return new Operand(_world->_storageScene, SCENE);
	case 0xC1: // SCENE@
		return new Operand(_world->_player->_currentScene, SCENE);
	case 0xC2: // PLAYER@
		return new Operand(_world->_player, CHR);
	case 0xC3: // MONSTER@
		return new Operand(_callbacks->getMonster(), CHR);
	case 0xC4: // RANDOMSCN@
		return new Operand(_world->_orderedScenes[_callbacks->_rnd->getRandomNumber(_world->_orderedScenes.size())], SCENE);
	case 0xC5: // RANDOMCHR@
		return new Operand(_world->_orderedChrs[_callbacks->_rnd->getRandomNumber(_world->_orderedChrs.size())], CHR);
	case 0xC6: // RANDOMOBJ@
		return new Operand(_world->_orderedObjs[_callbacks->_rnd->getRandomNumber(_world->_orderedObjs.size())], OBJ);
	case 0xB0: // VISITS#
		return new Operand(cont->_visits, NUMBER);
	case 0xB1: // RANDOM# for Star Trek, but VISITS# for some other games?
		return new Operand(1 + _callbacks->_rnd->getRandomNumber(100), NUMBER);
	case 0xB5: // RANDOM# // A random number between 1 and 100.
		return new Operand(1 + _callbacks->_rnd->getRandomNumber(100), NUMBER);
	case 0xB2: // LOOP#
		return new Operand(_loopCount, NUMBER);
	case 0xB3: // VICTORY#
		return new Operand(cont->_kills, NUMBER);
	case 0xB4: // BADCOPY#
		return new Operand(0, NUMBER); // \?\?\??
	case 0xFF:
		{
			// user variable
			int value = _data->readByte();

			// TODO: Verify that we're using the right index.
			return new Operand(cont->_userVariables[value - 1], NUMBER);
		}
	case 0xD0:
		return new Operand(cont->_statVariables[PHYS_STR_BAS], NUMBER);
	case 0xD1:
		return new Operand(cont->_statVariables[PHYS_HIT_BAS], NUMBER);
	case 0xD2:
		return new Operand(cont->_statVariables[PHYS_ARM_BAS], NUMBER);
	case 0xD3:
		return new Operand(cont->_statVariables[PHYS_ACC_BAS], NUMBER);
	case 0xD4:
		return new Operand(cont->_statVariables[SPIR_STR_BAS], NUMBER);
	case 0xD5:
		return new Operand(cont->_statVariables[SPIR_HIT_BAS], NUMBER);
	case 0xD6:
		return new Operand(cont->_statVariables[SPIR_ARM_BAS], NUMBER);
	case 0xD7:
		return new Operand(cont->_statVariables[SPIR_ACC_BAS], NUMBER);
	case 0xD8:
		return new Operand(cont->_statVariables[PHYS_SPE_BAS], NUMBER);
	case 0xE0:
		return new Operand(cont->_statVariables[PHYS_STR_CUR], NUMBER);
	case 0xE1:
		return new Operand(cont->_statVariables[PHYS_HIT_CUR], NUMBER);
	case 0xE2:
		return new Operand(cont->_statVariables[PHYS_ARM_CUR], NUMBER);
	case 0xE3:
		return new Operand(cont->_statVariables[PHYS_ACC_CUR], NUMBER);
	case 0xE4:
		return new Operand(cont->_statVariables[SPIR_STR_CUR], NUMBER);
	case 0xE5:
		return new Operand(cont->_statVariables[SPIR_HIT_CUR], NUMBER);
	case 0xE6:
		return new Operand(cont->_statVariables[SPIR_ARM_CUR], NUMBER);
	case 0xE7:
		return new Operand(cont->_statVariables[SPIR_ACC_CUR], NUMBER);
	case 0xE8:
		return new Operand(cont->_statVariables[PHYS_SPE_CUR], NUMBER);
	default:
		if (operandType >= 0x20 && operandType < 0x80) {
			_data->seek(-1, SEEK_CUR);
			return readStringOperand();
		} else {
			debug("Dunno what %x is (index=%d)!\n", operandType, _data->pos()-1);
		}
		return NULL;
	}
}

void Script::assign(byte operandType, int uservar, uint16 value) {
	Context *cont = &_world->_player->_context;

	switch (operandType) {
	case 0xFF:
		cont->_userVariables[uservar - 1] = value;
		break;
	case 0xD0:
		cont->_statVariables[PHYS_STR_BAS] = value;
		break;
	case 0xD1:
		cont->_statVariables[PHYS_HIT_BAS] = value;
		break;
	case 0xD2:
		cont->_statVariables[PHYS_ARM_BAS] = value;
		break;
	case 0xD3:
		cont->_statVariables[PHYS_ACC_BAS] = value;
		break;
	case 0xD4:
		cont->_statVariables[SPIR_STR_BAS] = value;
		break;
	case 0xD5:
		cont->_statVariables[SPIR_HIT_BAS] = value;
		break;
	case 0xD6:
		cont->_statVariables[SPIR_ARM_BAS] = value;
		break;
	case 0xD7:
		cont->_statVariables[SPIR_ACC_BAS] = value;
		break;
	case 0xD8:
		cont->_statVariables[PHYS_SPE_BAS] = value;
		break;
	case 0xE0:
		cont->_statVariables[PHYS_STR_CUR] = value;
		break;
	case 0xE1:
		cont->_statVariables[PHYS_HIT_CUR] = value;
		break;
	case 0xE2:
		cont->_statVariables[PHYS_ARM_CUR] = value;
		break;
	case 0xE3:
		cont->_statVariables[PHYS_ACC_CUR] = value;
		break;
	case 0xE4:
		cont->_statVariables[SPIR_STR_CUR] = value;
		break;
	case 0xE5:
		cont->_statVariables[SPIR_HIT_CUR] = value;
		break;
	case 0xE6:
		cont->_statVariables[SPIR_ARM_CUR] = value;
		break;
	case 0xE7:
		cont->_statVariables[SPIR_ACC_CUR] = value;
		break;
	case 0xE8:
		cont->_statVariables[PHYS_SPE_CUR] = value;
		break;
	default:
		debug("No idea what I'm supposed to assign! (%x at %d)!\n", operandType, _data->pos()-1);
	}
}

Script::Operand *Script::readStringOperand() {
	String *sb;
	bool allDigits = true;

	sb = new String();

	while (true) {
		byte c = _data->readByte();
		if (c >= 0x20 && c < 0x80)
			*sb += c;
		else
			break;
		if (c < '0' || c > '9')
			allDigits = false;
	}
	_data->seek(-1, SEEK_CUR);

	if (allDigits && sb->size() > 0) {
		int r = atol(sb->c_str());
		delete sb;

		return new Operand(r, NUMBER);
	} else {
		// TODO: This string could be a room name or something like that.
		return new Operand(sb, STRING);
	}
}

const char *Script::readOperator() {
	byte cmd = _data->readByte();

	debug(7, "readOperator: 0x%x", cmd);
	switch (cmd) {
	case 0x81:
		return "=";
	case 0x82:
		return "<";
	case 0x83:
		return ">";
	case 0x8f:
		return "+";
	case 0x90:
		return "-";
	case 0x91:
		return "*";
	case 0x92:
		return "/";
	case 0x93:
		return "==";
	case 0x94:
		return ">>";
	case 0xfd:
		return ";";
	default:
		warning("UNKNOWN OP %x", cmd);
	}
	return NULL;
}

void Script::processIf() {
	int logicalOp = 0; // 0 => initial, 1 => and, 2 => or
	bool result = true;
	bool done = false;

	do {
		Operand *lhs = readOperand();
		const char *op = readOperator();
		Operand *rhs = readOperand();

		bool condResult = eval(lhs, op, rhs);

		if (logicalOp == 1) {
			result = (result && condResult);
		} else if (logicalOp == 2) {
			result = (result || condResult);
		} else { // logicalOp == 0
			result = condResult;
		}

		byte logical = _data->readByte();

		if (logical == 0x84) {
			logicalOp = 1; // and
		} else if (logical == 0x85) {
			logicalOp = 2; // or
		} else if (logical == 0xFE) {
			done = true; // then
		}
	} while (!done);

	if (result == false) {
		skipBlock();
	}
}

void Script::skipIf() {
	do {
		readOperand();
		readOperator();
		readOperand();
	} while (_data->readByte() != 0xFE);
}

void Script::skipBlock() {
	int nesting = 1;

	while (true) {
		byte op = _data->readByte();

		if (_data->eos())
			return;

		if (op == 0x80) { // IF
			nesting++;
			skipIf();
		} else if (op == 0x88 || op == 0x87) { // END or EXIT
			nesting--;
			if (nesting == 0) {
				return;
			}
		} else switch (op) {
			case 0x8B: // PRINT
			case 0x8C: // SOUND
			case 0x8E: // LET
			case 0x95: // MENU
				while (_data->readByte() != 0xFD)
					;
		}
	}
}

enum {
	kCompEqNumNum,
	kCompEqObjScene,
	kCompEqChrScene,
	kCompEqObjChr,
	kCompEqChrChr,
	kCompEqSceneScene,
	kCompEqStringTextInput,
	kCompEqTextInputString,
	kCompEqNumberTextInput,
	kCompEqTextInputNumber,
	kCompLtNumNum,
	kCompLtStringTextInput,
	kCompLtTextInputString,
	kCompLtObjChr,
	kCompLtChrObj,
	kCompLtObjScene,
	kCompGtNumNum,
	kCompGtStringString,
	kCompGtChrScene,
	kMoveObjChr,
	kMoveObjScene,
	kMoveChrScene
};

struct Comparator {
	char op;
	OperandType o1;
	OperandType o2;
	int cmp;
} static comparators[] = {
	{ '=', NUMBER, NUMBER, kCompEqNumNum },
	{ '=', OBJ, SCENE, kCompEqObjScene },
	{ '=', CHR, SCENE, kCompEqChrScene },
	{ '=', OBJ, CHR, kCompEqObjChr },
	{ '=', CHR, CHR, kCompEqChrChr },
	{ '=', SCENE, SCENE, kCompEqSceneScene },
	{ '=', STRING, TEXT_INPUT, kCompEqStringTextInput },
	{ '=', TEXT_INPUT, STRING, kCompEqTextInputString },
	{ '=', NUMBER, TEXT_INPUT, kCompEqNumberTextInput },
	{ '=', TEXT_INPUT, NUMBER, kCompEqTextInputNumber },

	{ '<', NUMBER, NUMBER, kCompLtNumNum },
	{ '<', STRING, TEXT_INPUT, kCompLtStringTextInput },
	{ '<', TEXT_INPUT, STRING, kCompLtTextInputString },
	{ '<', OBJ, CHR, kCompLtObjChr },
	{ '<', CHR, OBJ, kCompLtChrObj },
	{ '<', OBJ, SCENE, kCompLtObjScene },
	{ '<', CHR, CHR, kCompEqChrChr }, // Same logic as =
	{ '<', SCENE, SCENE, kCompEqSceneScene },

	{ '>', NUMBER, NUMBER, kCompGtNumNum },
	{ '>', TEXT_INPUT, STRING, kCompLtTextInputString }, // Same logic as <
	//FIXME: this prevents the below cases from working due to exact
	//matches taking precedence over conversions...
	//{ '>', STRING, STRING, kCompGtStringString }, // Same logic as <
	{ '>', OBJ, CHR, kCompLtObjChr }, // Same logic as <
	{ '>', OBJ, SCENE, kCompLtObjScene }, // Same logic as <
	{ '>', CHR, SCENE, kCompGtChrScene },

	{ 'M', OBJ, CHR, kMoveObjChr },
	{ 'M', OBJ, SCENE, kMoveObjScene },
	{ 'M', CHR, SCENE, kMoveChrScene },
	{ 0, OBJ, OBJ, 0 }
};

bool Script::compare(Operand *o1, Operand *o2, int comparator) {
	switch(comparator) {
	case kCompEqNumNum:
		return o1->_value.number == o2->_value.number;
	case kCompEqObjScene:
		for (Common::List<Obj *>::const_iterator it = o2->_value.scene->_objs.begin(); it != o2->_value.scene->_objs.end(); ++it)
			if (*it == o1->_value.obj)
				return true;
		return false;
	case kCompEqChrScene:
		for (Common::List<Chr *>::const_iterator it = o2->_value.scene->_chrs.begin(); it != o2->_value.scene->_chrs.end(); ++it)
			if (*it == o1->_value.chr)
				return true;
		return false;
	case kCompEqObjChr:
		for (Common::Array<Obj *>::const_iterator it = o2->_value.chr->_inventory.begin(); it != o2->_value.chr->_inventory.end(); ++it)
			if (*it == o1->_value.obj)
				return true;
		return false;
	case kCompEqChrChr:
		return o1->_value.chr == o2->_value.chr;
	case kCompEqSceneScene:
		return o1->_value.scene == o2->_value.scene;
	case kCompEqStringTextInput:
		if (_inputText == NULL) {
			return false;
		} else {
			Common::String s1(*_inputText), s2(*o1->_value.string);
			s1.toLowercase();
			s2.toLowercase();

			return s1.contains(s2);
		}
	case kCompEqTextInputString:
		return compare(o2, o1, kCompEqStringTextInput);
	case kCompEqNumberTextInput:
		if (_inputText == NULL) {
			return false;
		} else {
			Common::String s1(*_inputText), s2(o1->toString());
			s1.toLowercase();
			s2.toLowercase();

			return s1.contains(s2);
		}
	case kCompEqTextInputNumber:
		if (_inputText == NULL) {
			return false;
		} else {
			Common::String s1(*_inputText), s2(o2->toString());
			s1.toLowercase();
			s2.toLowercase();

			return s1.contains(s2);
		}
	case kCompLtNumNum:
		return o1->_value.number < o2->_value.number;
	case kCompLtStringTextInput:
		return !compare(o1, o2, kCompEqStringTextInput);
	case kCompLtTextInputString:
		return !compare(o2, o1, kCompEqStringTextInput);
	case kCompLtObjChr:
		return o1->_value.obj->_currentOwner != o2->_value.chr;
	case kCompLtChrObj:
		return compare(o2, o1, kCompLtObjChr);
	case kCompLtObjScene:
		return o1->_value.obj->_currentScene != o2->_value.scene;
	case kCompGtNumNum:
		return o1->_value.number > o2->_value.number;
	case kCompGtStringString:
		return o1->_value.string == o2->_value.string;
	case kCompGtChrScene:
		return (o1->_value.chr != NULL && o1->_value.chr->_currentScene != o2->_value.scene);
	case kMoveObjChr:
		if (o1->_value.obj->_currentOwner != o2->_value.chr) {
			_world->move(o1->_value.obj, o2->_value.chr);
			_handled = true;  // TODO: Is this correct?
		}
		break;
	case kMoveObjScene:
		if (o1->_value.obj->_currentScene != o2->_value.scene) {
			_world->move(o1->_value.obj, o2->_value.scene);
			// Note: This shouldn't call setHandled() - see
			// Sultan's Palace 'Food and Drink' scene.
		}
		break;
	case kMoveChrScene:
		_world->move(o1->_value.chr, o2->_value.scene);
		_handled = true;  // TODO: Is this correct?
		break;
	}

	return false;
}

bool Script::evaluatePair(Operand *lhs, const char *op, Operand *rhs) {
	for (int cmp = 0; comparators[cmp].op != 0; cmp++) {
		if (comparators[cmp].op != op[0])
			continue;

		if (comparators[cmp].o1 == lhs->_type && comparators[cmp].o2 == rhs->_type)
			return compare(lhs, rhs, comparators[cmp].cmp);
	}

	// Now, try partial matches.
	Operand *c1, *c2;
	for (int cmp = 0; comparators[cmp].op != 0; cmp++) {
		if (comparators[cmp].op != op[0])
			continue;

		if (comparators[cmp].o1 == lhs->_type &&
				(c2 = convertOperand(rhs, comparators[cmp].o2)) != NULL) {
			bool res = compare(lhs, c2, comparators[cmp].cmp);
			delete c2;
			return res;
		} else if (comparators[cmp].o2 == rhs->_type &&
				(c1 = convertOperand(lhs, comparators[cmp].o1)) != NULL) {
			bool res = compare(c1, rhs, comparators[cmp].cmp);
			delete c1;
			return res;
		}
	}

	// Now, try double conversion.
	for (int cmp = 0; comparators[cmp].op != 0; cmp++) {
		if (comparators[cmp].op != op[0])
			continue;

		if (comparators[cmp].o1 == lhs->_type || comparators[cmp].o2 == rhs->_type)
			continue;

		if ((c1 = convertOperand(lhs, comparators[cmp].o1)) != NULL) {
			if ((c2 = convertOperand(rhs, comparators[cmp].o2)) != NULL) {
				bool res = compare(c1, c2, comparators[cmp].cmp);
				delete c1;
				delete c2;
				return res;
			}
			delete c1;
		}
	}

	warning("UNHANDLED CASE: [lhs=%d/%s, op=%s rhs=%d/%s]",
		lhs->_type, lhs->toString().c_str(), op, rhs->_type, rhs->toString().c_str());


	return false;
}

bool Script::eval(Operand *lhs, const char *op, Operand *rhs) {
	bool result = false;

	if (lhs->_type == CLICK_INPUT || rhs->_type == CLICK_INPUT) {
		return evalClickCondition(lhs, op, rhs);
	} else if (!strcmp(op, "==") || !strcmp(op, ">>")) {
		// TODO: check if >> can be used for click inputs and if == can be used for other things
		// exact string match
		if (lhs->_type == TEXT_INPUT) {
			if ((rhs->_type != STRING && rhs->_type != NUMBER) || _inputText == NULL) {
				result = false;
			} else {
				result = _inputText->equalsIgnoreCase(rhs->toString());
			}
		} else if (rhs->_type == TEXT_INPUT) {
			if ((lhs->_type != STRING && lhs->_type != NUMBER) || _inputText == NULL) {
				result = false;
			} else {
				result = _inputText->equalsIgnoreCase(lhs->toString());
			}
		} else {
			error("UNHANDLED CASE: [lhs=%d/%s, rhs=%d/%s]",
				lhs->_type, lhs->toString().c_str(), rhs->_type, rhs->toString().c_str());
		}
		if (!strcmp(op, ">>")) {
			result = !result;
		}

		return result;
	} else {
		return evaluatePair(lhs, op, rhs);
	}

	return false;
}

Script::Operand *Script::convertOperand(Operand *operand, int type) {
	if (operand->_type == type)
		error("Incorrect conversion to type %d", type);

	if (type == SCENE) {
		if (operand->_type == STRING || operand->_type == NUMBER) {
			Common::String key(operand->toString());
			key.toLowercase();
			if (_world->_scenes.contains(key))
				return new Operand(_world->_scenes[key], SCENE);
		}
	} else if (type == OBJ) {
		if (operand->_type == STRING || operand->_type == NUMBER) {
			Common::String key = operand->toString();
			key.toLowercase();
			if (_world->_objs.contains(key))
				return new Operand(_world->_objs[key], OBJ);
		} else if (operand->_type == CLICK_INPUT) {
			if (_inputClick->_classType == OBJ)
				return new Operand(_inputClick, OBJ);
		}
	} else if (type == CHR) {
		if (operand->_type == STRING || operand->_type == NUMBER) {
			Common::String key = operand->toString();
			key.toLowercase();
			if (_world->_chrs.contains(key))
				return new Operand(_world->_chrs[key], CHR);
		} else if (operand->_type == CLICK_INPUT) {
			if (_inputClick->_classType == CHR)
				return new Operand(_inputClick, CHR);
		}
	}

	return NULL;
}

bool Script::evalClickEquality(Operand *lhs, Operand *rhs, bool partialMatch) {
	bool result = false;
	if (lhs->_value.obj == NULL || rhs->_value.obj == NULL) {
		result = false;
	} else if (lhs->_value.obj == rhs->_value.obj) {
		result = true;
	} else if (rhs->_type == STRING) {
		Common::String str = rhs->toString();
		str.toLowercase();

		debug(9, "evalClickEquality(%s, %s, %d)", lhs->_value.designed->_name.c_str(), rhs->_value.designed->_name.c_str(), partialMatch);
		debug(9, "l: %d r: %d (ch: %d ob: %d)", lhs->_type, rhs->_type, CHR, OBJ);
		debug(9, "class: %d", lhs->_value.inputClick->_classType);

		if (lhs->_value.inputClick->_classType == CHR || lhs->_value.inputClick->_classType == OBJ) {
			Common::String name = lhs->_value.designed->_name;
			name.toLowercase();

			warning("%s <> %s", name.c_str(), str.c_str());
			if (partialMatch)
				result = name.contains(str);
			else
				result = name.equals(str);
		}

		debug(9, "result: %d", result);
	}
	return result;
}

bool Script::evalClickCondition(Operand *lhs, const char *op, Operand *rhs) {
	// TODO: check if >> can be used for click inputs
	if (strcmp(op, "==") && strcmp(op, "=") && strcmp(op, "<") && strcmp(op, ">")) {
		error("Unknown operation '%s' for Script::evalClickCondition", op);
	}

	bool partialMatch = strcmp(op, "==");
	bool result;
	if (lhs->_type == CLICK_INPUT) {
		result = evalClickEquality(lhs, rhs, partialMatch);
	} else {
		result = evalClickEquality(rhs, lhs, partialMatch);
	}
	if (!strcmp(op, "<") || !strcmp(op, ">")) {
		// CLICK$<FOO only matches if there was a click
		if (_inputClick == NULL) {
			result = false;
		} else {
			result = !result;
		}
	}
	return result;
}

void Script::takeObj(Obj *obj) {
	if (_world->_player->_inventory.size() >= _world->_player->_maximumCarriedObjects) {
		appendText("Your pack is full, you must drop something.");
	} else {
		_world->move(obj, _world->_player);
		int type = _world->_player->wearObjIfPossible(obj);
		if (type == Chr::HEAD_ARMOR) {
			appendText(String("You are now wearing the ") + obj->_name + ".");
		} else if (type == Chr::BODY_ARMOR) {
			appendText(String("You are now wearing the ") + obj->_name + ".");
		} else if (type == Chr::SHIELD_ARMOR) {
			appendText(String("You are now wearing the ") + obj->_name + ".");
		} else if (type == Chr::MAGIC_ARMOR) {
			appendText(String("You are now wearing the ") + obj->_name + ".");
		} else {
			appendText(String("You now have the ") + obj->_name + ".");
		}
		appendText(obj->_clickMessage);
	}
}

void Script::processMove() {
	Operand *what = readOperand();
	byte skip = _data->readByte();
	if (skip != 0x8a)
		error("Incorrect operator for MOVE: %02x", skip);

	Operand *to = readOperand();

	skip = _data->readByte();
	if (skip != 0xfd)
		error("No end for MOVE: %02x", skip);

	debug(6, "MOVE: [what=%d/%s, to=%d/%s]",
		what->_type, what->toString().c_str(), to->_type, to->toString().c_str());

	evaluatePair(what, "M", to);
}

void Script::processLet() {
	const char *lastOp = NULL;
	int16 result = 0;
	int operandType = _data->readByte();
	int uservar = 0;

	if (operandType == 0xff) {
		uservar = _data->readByte();
	}

	byte eq = _data->readByte(); // skip "=" operator

	debug(7, "processLet: 0x%x, uservar: 0x%x, eq: 0x%x", operandType, uservar, eq);

	do {
		Operand *operand = readOperand();
		// TODO assert that value is NUMBER
		int16 value = operand->_value.number;
		if (lastOp != NULL) {
			if (lastOp[0] == '+')
				result += value;
			else if (lastOp[0] == '-')
				result -= value;
			else if (lastOp[0] == '/')
				result = (int16)(value == 0 ? 0 : result / value);
			else if (lastOp[0] == '*')
				result *= value;
		} else {
			result = value;
		}
		lastOp = readOperator();

		if (lastOp[0] == ';')
			break;
	} while (true);
	//System.out.println("processLet " + buildStringFromOffset(oldIndex - 1, index - oldIndex + 1) + "}");

	assign(operandType, uservar, result);
}

void Script::appendText(String str) {
	_handled = true;
	_callbacks->appendText(str);
}

static const int directionsX[] = { 0, 0, 1, -1 };
static const int directionsY[] = { -1, 1, 0, 0 };

void Script::handleMoveCommand(Scene::Directions dir, const char *dirName) {
	Scene *playerScene = _world->_player->_currentScene;
	Common::String msg(playerScene->_messages[dir]);

	warning("Dir: %s  msg: %s", dirName, msg.c_str());

	if (!playerScene->_blocked[dir]) {
		int destX = playerScene->_worldX + directionsX[dir];
		int destY = playerScene->_worldY + directionsY[dir];

		Scene *scene = _world->getSceneAt(destX, destY);

		if (scene != NULL) {
			if (msg.size() > 0) {
				appendText(msg);
			}
			_world->move(_world->_player, scene);
			return;
		}
	}
	if (msg != NULL && msg.size() > 0) {
		appendText(msg);
	} else {
		Common::String txt("You can't go ");
		txt += dirName;
		txt += ".";
		appendText(txt);
	}
}

void Script::handleLookCommand() {
	appendText(_world->_player->_currentScene->_text);

	Common::String *items = getGroundItemsList(_world->_player->_currentScene);
	if (items != NULL) {
		appendText(*items);

		delete items;
	}
}

Common::String *Script::getGroundItemsList(Scene *scene) {
	Common::Array<Obj *> objs;

	for (Common::List<Obj *>::const_iterator it = scene->_objs.begin(); it != scene->_objs.end(); ++it)
		if ((*it)->_type != Obj::IMMOBILE_OBJECT)
			objs.push_back(*it);

	if (objs.size()) {
		Common::String *res = new Common::String("On the ground you see ");
		appendObjNames(*res, objs);
		return res;
	}
	return NULL;
}

void Script::appendObjNames(Common::String &str, Common::Array<Obj *> &objs) {
	for (int i = 0; i < objs.size(); i++) {
		Obj *obj = objs[i];

		if (!obj->_namePlural)
			str += getIndefiniteArticle(obj->_name);
		else
			str += "some ";

		str += obj->_name;

		if (i == objs.size() - 1) {
			str += ".";
		} else if (i == objs.size() - 2) {
			if (objs.size() > 2)
				str += ",";
			str += " and ";
		} else {
			str += ", ";
		}
	}
}

void Script::handleInventoryCommand() {
	warning("STUB: handleInventoryCommand");
}

void Script::handleStatusCommand() {
	warning("STUB: handleStatusCommand");
}

void Script::handleRestCommand() {
	warning("STUB: handleRestCommand");
}

void Script::handleAcceptCommand() {
	warning("STUB: handleAcceptCommand");
}

void Script::handleTakeCommand(const char *target) {
	warning("STUB: handleTakeCommand");
}

void Script::handleDropCommand(const char *target) {
	warning("STUB: handleDropCommand");
}

void Script::handleAimCommand(const char *target) {
	warning("STUB: handleAimCommand");
}

void Script::handleWearCommand(const char *target) {
	warning("STUB: handleWearCommand");
}

void Script::handleOfferCommand(const char *target) {
	warning("STUB: handleOfferCommand");
}

bool Script::tryAttack(Weapon *weapon, Common::String &input) {
	warning("STUB: tryAttack");

	return false;
}

void Script::handleAttack(Weapon *weapon) {
	warning("STUB: handleAttack");
}

enum {
	BLOCK_START,
	BLOCK_END,
	STATEMENT,
	OPERATOR,
	OPCODE
};

struct {
	const char *cmd;
	int type;
} mapping[] = {
	{ "IF{", STATEMENT }, // 0x80
	{ "=", OPERATOR },
	{ "<", OPERATOR },
	{ ">", OPERATOR },
	{ "}AND{", OPCODE },
	{ "}OR{", OPCODE },
	{ "\?\?\?(0x86)", OPCODE },
	{ "EXIT\n", BLOCK_END },
	{ "END\n", BLOCK_END }, // 0x88
	{ "MOVE{", STATEMENT },
	{ "}TO{", OPCODE },
	{ "PRINT{", STATEMENT },
	{ "SOUND{", STATEMENT },
	{ "\?\?\?(0x8d)", OPCODE },
	{ "LET{", STATEMENT },
	{ "+", OPERATOR },
	{ "-", OPERATOR }, // 0x90
	{ "*", OPERATOR },
	{ "/", OPERATOR },
	{ "==", OPERATOR },
	{ ">>", OPERATOR },
	{ "MENU{", STATEMENT },
	{ "\?\?\?(0x96)", OPCODE },
	{ "\?\?\?(0x97)", OPCODE },
	{ "\?\?\?(0x98)", OPCODE }, // 0x98
	{ "\?\?\?(0x99)", OPCODE },
	{ "\?\?\?(0x9a)", OPCODE },
	{ "\?\?\?(0x9b)", OPCODE },
	{ "\?\?\?(0x9c)", OPCODE },
	{ "\?\?\?(0x9d)", OPCODE },
	{ "\?\?\?(0x9e)", OPCODE },
	{ "\?\?\?(0x9f)", OPCODE },
	{ "TEXT$", OPCODE }, // 0xa0
	{ "CLICK$", OPCODE },
	{ "\?\?\?(0xa2)", OPCODE },
	{ "\?\?\?(0xa3)", OPCODE },
	{ "\?\?\?(0xa4)", OPCODE },
	{ "\?\?\?(0xa5)", OPCODE },
	{ "\?\?\?(0xa6)", OPCODE },
	{ "\?\?\?(0xa7)", OPCODE },
	{ "\?\?\?(0xa8)", OPCODE }, // 0xa8
	{ "\?\?\?(0xa9)", OPCODE },
	{ "\?\?\?(0xaa)", OPCODE },
	{ "\?\?\?(0xab)", OPCODE },
	{ "\?\?\?(0xac)", OPCODE },
	{ "\?\?\?(0xad)", OPCODE },
	{ "\?\?\?(0xae)", OPCODE },
	{ "\?\?\?(0xaf)", OPCODE },
	{ "VISITS#", OPCODE }, // 0xb0 // The number of scenes the player has visited, including repeated visits.
	{ "RANDOM#", OPCODE }, // RANDOM# for Star Trek, but VISITS# for some other games?
	{ "LOOP#", OPCODE },   // The number of commands the player has given in the current scene.
	{ "VICTORY#", OPCODE }, // The number of characters killed.
	{ "BADCOPY#", OPCODE },
	{ "RANDOM#", OPCODE }, // A random number between 1 and 100.
	{ "\?\?\?(0xb6)", OPCODE },
	{ "\?\?\?(0xb7)", OPCODE },
	{ "\?\?\?(0xb8)", OPCODE }, // 0xb8
	{ "\?\?\?(0xb9)", OPCODE },
	{ "\?\?\?(0xba)", OPCODE },
	{ "\?\?\?(0xbb)", OPCODE },
	{ "\?\?\?(0xbc)", OPCODE },
	{ "\?\?\?(0xbd)", OPCODE },
	{ "\?\?\?(0xbe)", OPCODE },
	{ "\?\?\?(0xbf)", OPCODE },
	{ "STORAGE@", OPCODE }, // 0xc0
	{ "SCENE@", OPCODE },
	{ "PLAYER@", OPCODE },
	{ "MONSTER@", OPCODE },
	{ "RANDOMSCN@", OPCODE },
	{ "RANDOMCHR@", OPCODE },
	{ "RANDOMOBJ@", OPCODE },
	{ "\?\?\?(0xc7)", OPCODE },
	{ "\?\?\?(0xc8)", OPCODE }, // 0xc8
	{ "\?\?\?(0xc9)", OPCODE },
	{ "\?\?\?(0xca)", OPCODE },
	{ "\?\?\?(0xcb)", OPCODE },
	{ "\?\?\?(0xcc)", OPCODE },
	{ "\?\?\?(0xcd)", OPCODE },
	{ "\?\?\?(0xce)", OPCODE },
	{ "\?\?\?(0xcf)", OPCODE },
	{ "PHYS.STR.BAS#", OPCODE }, // 0xd0
	{ "PHYS.HIT.BAS#", OPCODE },
	{ "PHYS.ARM.BAS#", OPCODE },
	{ "PHYS.ACC.BAS#", OPCODE },
	{ "SPIR.STR.BAS#", OPCODE },
	{ "SPIR.HIT.BAS#", OPCODE },
	{ "SPIR.ARM.BAS#", OPCODE },
	{ "SPIR.ACC.BAS#", OPCODE },
	{ "PHYS.SPE.BAS#", OPCODE }, // 0xd8
	{ "\?\?\?(0xd9)", OPCODE },
	{ "\?\?\?(0xda)", OPCODE },
	{ "\?\?\?(0xdb)", OPCODE },
	{ "\?\?\?(0xdc)", OPCODE },
	{ "\?\?\?(0xdd)", OPCODE },
	{ "\?\?\?(0xde)", OPCODE },
	{ "\?\?\?(0xdf)", OPCODE },
	{ "PHYS.STR.CUR#", OPCODE }, // 0xe0
	{ "PHYS.HIT.CUR#", OPCODE },
	{ "PHYS.ARM.CUR#", OPCODE },
	{ "PHYS.ACC.CUR#", OPCODE },
	{ "SPIR.STR.CUR#", OPCODE },
	{ "SPIR.HIT.CUR#", OPCODE },
	{ "SPIR.ARM.CUR#", OPCODE },
	{ "SPIR.ACC.CUR#", OPCODE },
	{ "PHYS.SPE.CUR#", OPCODE }, // 0xe8
	{ "\?\?\?(0xe9)", OPCODE },
	{ "\?\?\?(0xea)", OPCODE },
	{ "\?\?\?(0xeb)", OPCODE },
	{ "\?\?\?(0xec)", OPCODE },
	{ "\?\?\?(0xed)", OPCODE },
	{ "\?\?\?(0xee)", OPCODE },
	{ "\?\?\?(0xef)", OPCODE },
	{ "\?\?\?(0xf0)", OPCODE },
	{ "\?\?\?(0xf1)", OPCODE },
	{ "\?\?\?(0xf2)", OPCODE },
	{ "\?\?\?(0xf3)", OPCODE },
	{ "\?\?\?(0xf4)", OPCODE },
	{ "\?\?\?(0xf5)", OPCODE },
	{ "\?\?\?(0xf6)", OPCODE },
	{ "\?\?\?(0xf7)", OPCODE },
	{ "\?\?\?(0xf8)", OPCODE }, // 0xa8
	{ "\?\?\?(0xf9)", OPCODE },
	{ "\?\?\?(0xfa)", OPCODE },
	{ "\?\?\?(0xfb)", OPCODE },
	{ "\?\?\?(0xfc)", OPCODE },
	{ "}\n", OPCODE },
	{ "}THEN\n", BLOCK_START },
	{ "\?\?\?(0xff)", OPCODE } // Uservar
};

void Script::convertToText() {
	_data->seek(12);

	int indentLevel = 0;
	ScriptText *scr = new ScriptText;
	scr->offset = _data->pos();

	while(true) {
		int c = _data->readByte();

		if (_data->eos())
			break;

		if (c < 0x80) {
			if (c < 0x20)
				error("Unknown code 0x%02x at %d", c, _data->pos());

			do {
				scr->line += c;
				c = _data->readByte();
			} while (c < 0x80);

			_data->seek(-1, SEEK_CUR);
		} else if (c == 0xff) {
			int value = _data->readByte();
			value -= 1;
			scr->line += (char)('A' + (value / 9));
			scr->line += (char)('0' + (value % 9) + 1);
			scr->line += '#';
		} else {
			const char *cmd = mapping[c - 0x80].cmd;
			int type = mapping[c - 0x80].type;

			if (type == STATEMENT) {
				for (int i = 0; i < indentLevel; i++)
					scr->line += ' ';
			} else if (type == BLOCK_START) {
				indentLevel += 2;
			} else if (type == BLOCK_END) {
				indentLevel -= 2;
				for (int i = 0; i < indentLevel; i++)
					scr->line += ' ';
			}

			scr->line += cmd;

			if (strchr(cmd, '\n')) {
				scr->line.deleteLastChar();

				_scriptText.push_back(scr);

				scr = new ScriptText;
				scr->offset = _data->pos();
			}
		}
	}
}

} // End of namespace Wage
