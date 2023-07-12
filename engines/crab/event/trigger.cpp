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
	bool echo_op = true, echo_tar = false, echo_sub = true;

	if (ty == "obj")
		type = TRIG_OBJ;
	else if (ty == "opinion") {
		type = TRIG_OPINION;
		echo_tar = true;
	} else if (ty == "loc") {
		type = TRIG_LOC;
		echo_op = false;
	} else if (ty == "item") {
		type = TRIG_ITEM;
		echo_op = false;
	} else if (ty == "rect") {
		type = TRIG_RECT;
		echo_op = false;
	} else if (ty == "stat") {
		type = TRIG_STAT;
		echo_tar = true;
	} else if (ty == "diff") {
		type = TRIG_DIFF;
		echo_sub = false;
	} else if (ty == "trait") {
		type = TRIG_TRAIT;
	} else
		type = TRIG_VAR;

	loadStr(target, "target", node, echo_tar);
	loadStr(subject, "subject", node, echo_sub);
	loadStr(operation, "operation", node, echo_op);
	loadStr(val, "val", node);

	Common::String str;
	loadStr(str, "rel", node, false);
	if (str == "or")
		rel = OP_OR;
	else
		rel = OP_AND;

	loadStr(str, "prefix", node, false);
	if (str == "!")
		negate = true;
	else
		negate = false;
}

bool Trigger::Evaluate(int lhs, int rhs) {
	if (operation == ">" && lhs > rhs)
		return true;
	else if (operation == "=" && lhs == rhs)
		return true;
	else if (operation == "<" && lhs < rhs)
		return true;
	else if (operation == "!=" && lhs != rhs)
		return true;
	else if (operation == "<=" && lhs <= rhs)
		return true;
	else if (operation == ">=" && lhs >= rhs)
		return true;

	return false;
}

bool Trigger::Evaluate(pyrodactyl::event::Info &info) {
	using namespace pyrodactyl::people;
	using namespace pyrodactyl::stat;

	switch (type) {
	case TRIG_OBJ:
		if (operation == "p") {
			if (info._talkKeyDown && info.lastPerson() == val)
				return true;
			else
				return false;
		} else if (operation == "status") {
			PersonType ty = StringToPersonType(val);
			if (info.type(subject) == ty)
				return true;
			else
				return false;
		} else if (operation == "state") {
			PersonState st = StringToPersonState(val);
			if (info.state(subject) == st)
				return true;
			else
				return false;
		}
		break;

	case TRIG_OPINION: {
		Person p;
		if (info.personGet(subject, p)) {
			if (target == "like")
				return Evaluate(p.opinion.val[OPI_LIKE], StringToNumber<int>(val));
			else if (target == "fear")
				return Evaluate(p.opinion.val[OPI_FEAR], StringToNumber<int>(val));
			else if (target == "respect")
				return Evaluate(p.opinion.val[OPI_RESPECT], StringToNumber<int>(val));
		}
	} break;

	case TRIG_LOC:
		return (info.curLocID() == val);
		break;

	case TRIG_ITEM:
		return info._inv.HasItem(target, subject, val);
		break;

	case TRIG_RECT:
		return info.collideWithTrigger(subject, StringToNumber<int>(val));

	case TRIG_STAT: {
		StatType ty = StringToStatType(target);
		int sub = 0, value = 0;
		bool compare_to_var = Common::find_if(val.begin(), val.end(), IsChar) != val.end();

		info.statGet(subject, ty, sub);
		if (compare_to_var)
			info.statGet(val, ty, value);
		else
			value = StringToNumber<int>(val);

		return Evaluate(sub, value);
	} break;

	case TRIG_DIFF:
		return Evaluate(info.ironMan(), StringToNumber<int>(val));

	case TRIG_TRAIT:
		if (info.personValid(target)) {
			Person *p = &info.personGet(target);

			for (auto &i : p->trait)
				if (i.name == val)
					return true;
		}
		break;

	case TRIG_VAR: {
		int var_sub = 0, var_val = 0;
		bool compare_to_var = Common::find_if(val.begin(), val.end(), IsChar) != val.end();

		info.varGet(subject, var_sub);
		if (compare_to_var)
			info.varGet(val, var_val);
		else
			var_val = StringToNumber<int>(val);

		return Evaluate(var_sub, var_val);
	} break;

	default:
		break;
	}

	return false;
}

} // End of namespace Crab
