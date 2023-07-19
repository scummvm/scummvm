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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#include "crab/event/trigger.h"

namespace Crab {

using namespace pyrodactyl::event;

void Trigger::load(rapidxml::xml_node<char> *node) {
	Common::String ty;
	loadStr(ty, "type", node);

	// Should we throw a warning about missing fields? Depends on the type of trigger
	bool echoOp = true, echoTar = false, echoSub = true;

	if (ty == "obj")
		_type = TRIG_OBJ;
	else if (ty == "opinion") {
		_type = TRIG_OPINION;
		echoTar = true;
	} else if (ty == "loc") {
		_type = TRIG_LOC;
		echoOp = false;
	} else if (ty == "item") {
		_type = TRIG_ITEM;
		echoOp = false;
	} else if (ty == "rect") {
		_type = TRIG_RECT;
		echoOp = false;
	} else if (ty == "stat") {
		_type = TRIG_STAT;
		echoTar = true;
	} else if (ty == "diff") {
		_type = TRIG_DIFF;
		echoSub = false;
	} else if (ty == "trait") {
		_type = TRIG_TRAIT;
	} else
		_type = TRIG_VAR;

	loadStr(_target, "target", node, echoTar);
	loadStr(_subject, "subject", node, echoSub);
	loadStr(_operation, "operation", node, echoOp);
	loadStr(_val, "val", node);

	Common::String str;
	loadStr(str, "rel", node, false);
	if (str == "or")
		_rel = OP_OR;
	else
		_rel = OP_AND;

	loadStr(str, "prefix", node, false);
	if (str == "!")
		_negate = true;
	else
		_negate = false;
}

bool Trigger::evaluate(int lhs, int rhs) {
	if (_operation == ">" && lhs > rhs)
		return true;
	else if (_operation == "=" && lhs == rhs)
		return true;
	else if (_operation == "<" && lhs < rhs)
		return true;
	else if (_operation == "!=" && lhs != rhs)
		return true;
	else if (_operation == "<=" && lhs <= rhs)
		return true;
	else if (_operation == ">=" && lhs >= rhs)
		return true;

	return false;
}

bool Trigger::evaluate(pyrodactyl::event::Info &info) {
	using namespace pyrodactyl::people;
	using namespace pyrodactyl::stat;

	switch (_type) {
	case TRIG_OBJ:
		if (_operation == "p") {
			if (info._talkKeyDown && info.lastPerson() == _val)
				return true;
			else
				return false;
		} else if (_operation == "status") {
			PersonType ty = stringToPersonType(_val);
			if (info.type(_subject) == ty)
				return true;
			else
				return false;
		} else if (_operation == "state") {
			PersonState st = stringToPersonState(_val);
			if (info.state(_subject) == st)
				return true;
			else
				return false;
		}
		break;

	case TRIG_OPINION: {
		Person p;
		if (info.personGet(_subject, p)) {
			if (_target == "like")
				return evaluate(p._opinion._val[OPI_LIKE], StringToNumber<int>(_val));
			else if (_target == "fear")
				return evaluate(p._opinion._val[OPI_FEAR], StringToNumber<int>(_val));
			else if (_target == "respect")
				return evaluate(p._opinion._val[OPI_RESPECT], StringToNumber<int>(_val));
		}
	}
	break;

	case TRIG_LOC:
		return (info.curLocID() == _val);
		break;

	case TRIG_ITEM:
		return info._inv.HasItem(_target, _subject, _val);
		break;

	case TRIG_RECT:
		return info.collideWithTrigger(_subject, StringToNumber<int>(_val));

	case TRIG_STAT: {
		StatType ty = stringToStatType(_target);
		int sub = 0, value = 0;
		bool compareToVar = Common::find_if(_val.begin(), _val.end(), IsChar) != _val.end();

		info.statGet(_subject, ty, sub);
		if (compareToVar)
			info.statGet(_val, ty, value);
		else
			value = StringToNumber<int>(_val);

		return evaluate(sub, value);
	}
		break;

	case TRIG_DIFF:
		return evaluate(info.ironMan(), StringToNumber<int>(_val));

	case TRIG_TRAIT:
		if (info.personValid(_target)) {
			Person *p = &info.personGet(_target);

			for (auto &i : p->_trait)
				if (i._name == _val)
					return true;
		}
		break;

	case TRIG_VAR: {
		int var_sub = 0, var_val = 0;
		bool compare_to_var = Common::find_if(_val.begin(), _val.end(), IsChar) != _val.end();

		info.varGet(_subject, var_sub);
		if (compare_to_var)
			info.varGet(_val, var_val);
		else
			var_val = StringToNumber<int>(_val);

		return evaluate(var_sub, var_val);
	}
		break;

	default:
		break;
	}

	return false;
}

} // End of namespace Crab
