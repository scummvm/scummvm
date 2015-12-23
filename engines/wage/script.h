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

#ifndef WAGE_SCRIPT_H
#define WAGE_SCRIPT_H

namespace Wage {

enum OperandTypes {
	OBJ = 0,
	CHR = 1,
	SCENE = 2,
	NUMBER = 3,
	STRING = 4,
	CLICK_INPUT = 5,
	TEXT_INPUT = 6
};

class Script {
public:
	Script(Common::SeekableReadStream *data) : _data(data) {}
	~Script();

private:
	Common::SeekableReadStream *_data;

	WageEngine *_callbacks;
	World *_world;
	int _loopCount;
	String *_inputText;
	Designed *_inputClick;
	bool _evalResult;
	bool _handled;

	class Operand {
	public:
		union {
			Obj *obj;
			Chr *chr;
			Scene *scene;
			int16 number;
			String *string;
			Designed *inputClick;
		} _value;
		OperandTypes _type;
		Common::String _str;

		Operand(Obj *value, OperandTypes type) {
			_value.obj = value;
			_type = type;
		}

		Operand(Chr *value, OperandTypes type) {
			_value.chr = value;
			_type = type;
		}

		Operand(Scene *value, OperandTypes type) {
			_value.scene = value;
			_type = type;
		}

		Operand(int value, OperandTypes type) {
			_value.number = value;
			_type = type;
		}

		Operand(String *value, OperandTypes type) {
			_value.string = value;
			_type = type;
		}

		Operand(Designed *value, OperandTypes type) {
			_value.inputClick = value;
			_type = type;
		}

		Common::String toString() {
			char buf[128];

			if (_value.obj == NULL)
				_str = "";

			switch(_type) {
			case NUMBER:
				_str = snprintf(buf, 128, "%d", _value.number);
				return _str;
			case STRING:
			case TEXT_INPUT:
				return *_value.string;
			case OBJ:
				return _value.obj->toString();
			case CHR:
				return _value.chr->toString();
			case SCENE:
				return _value.scene->toString();
			case CLICK_INPUT:
				return _value.inputClick->toString();
			default:
				error("Unhandled operand type: _type");
			}
		}
	};

public:
	bool execute(World *world, int loopCount, String *inputText, Designed *inputClick, WageEngine *callbacks);

private:
	Operand *readOperand();
	Operand *readStringOperand();
	const char *readOperator();
	void processIf();
	void skipBlock();
	void skipIf();
	bool compare(Operand *o1, Operand *o2, int comparator);
	bool eval(Operand *lhs, const char *op, Operand *rhs);
	bool evalClickCondition(Operand *lhs, const char *op, Operand *rhs);
	void processMove();
	void processLet();

	void assign(byte operandType, int uservar, uint16 value);

	void appendText(String str);
	void handleMoveCommand(Scene::Directions dir, const char *dirName);
	void handleLookCommand();
	void handleInventoryCommand();
	void handleStatusCommand();
	void handleRestCommand();
	void handleAcceptCommand();

	void handleTakeCommand(const char *target);
	void handleDropCommand(const char *target);
	void handleAimCommand(const char *target);
	void handleWearCommand(const char *target);
	void handleOfferCommand(const char *target);

	bool tryAttack(Weapon *weapon, Common::String &input);
	void handleAttack(Weapon *weapon);

};

} // End of namespace Wage

#endif
