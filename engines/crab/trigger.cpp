#include "stdafx.h"
#include "trigger.h"

using namespace pyrodactyl::event;

void Trigger::Load(rapidxml::xml_node<char> *node)
{
	std::string ty;
	LoadStr(ty, "type", node);

	//Should we throw a warning about missing fields? Depends on the type of trigger
	bool echo_op = true, echo_tar = false, echo_sub = true;

	if (ty == "obj") type = TRIG_OBJ;
	else if (ty == "opinion") { type = TRIG_OPINION; echo_tar = true; }
	else if (ty == "loc") { type = TRIG_LOC; echo_op = false; }
	else if (ty == "item") { type = TRIG_ITEM; echo_op = false; }
	else if (ty == "rect") { type = TRIG_RECT; echo_op = false; }
	else if (ty == "stat") { type = TRIG_STAT; echo_tar = true; }
	else if (ty == "diff") { type = TRIG_DIFF; echo_sub = false; }
	else if (ty == "trait") { type = TRIG_TRAIT; }
	else type = TRIG_VAR;

	LoadStr(target, "target", node, echo_tar);
	LoadStr(subject, "subject", node, echo_sub);
	LoadStr(operation, "operation", node, echo_op);
	LoadStr(val, "val", node);

	std::string str;
	LoadStr(str, "rel", node, false);
	if (str == "or")
		rel = OP_OR;
	else
		rel = OP_AND;

	LoadStr(str, "prefix", node, false);
	if (str == "!")
		negate = true;
	else
		negate = false;
}

bool Trigger::Evaluate(int lhs, int rhs)
{
	if (operation == ">" && lhs > rhs)        return true;
	else if (operation == "=" && lhs == rhs)  return true;
	else if (operation == "<" && lhs < rhs)   return true;
	else if (operation == "!=" && lhs != rhs) return true;
	else if (operation == "<=" && lhs <= rhs) return true;
	else if (operation == ">=" && lhs >= rhs) return true;

	return false;
}

bool Trigger::Evaluate(pyrodactyl::event::Info &info)
{
	using namespace pyrodactyl::people;
	using namespace pyrodactyl::stat;

	switch (type)
	{
	case TRIG_OBJ:
		if (operation == "p")
		{
			if (info.TalkKeyDown && info.LastPerson() == val)
				return true;
			else
				return false;
		}
		else if (operation == "status")
		{
			PersonType ty = StringToPersonType(val);
			if (info.Type(subject) == ty)
				return true;
			else
				return false;
		}
		else if (operation == "state")
		{
			PersonState st = StringToPersonState(val);
			if (info.State(subject) == st)
				return true;
			else
				return false;
		}
		break;

	case TRIG_OPINION:
	{
		Person p;
		if (info.PersonGet(subject, p))
		{
			if (target == "like")
				return Evaluate(p.opinion.val[OPI_LIKE], StringToNumber<int>(val));
			else if (target == "fear")
				return Evaluate(p.opinion.val[OPI_FEAR], StringToNumber<int>(val));
			else if (target == "respect")
				return Evaluate(p.opinion.val[OPI_RESPECT], StringToNumber<int>(val));
		}
	}
		break;

	case TRIG_LOC:
		return (info.CurLocID() == val);
		break;

	case TRIG_ITEM:
		return info.inv.HasItem(target, subject, val);
		break;

	case TRIG_RECT:
		return info.CollideWithTrigger(subject, StringToNumber<int>(val));

	case TRIG_STAT:
	{
		StatType ty = StringToStatType(target);
		int sub = 0, value = 0;
		bool compare_to_var = std::find_if(val.begin(), val.end(), IsChar) != val.end();

		info.StatGet(subject, ty, sub);
		if (compare_to_var)
			info.StatGet(val, ty, value);
		else
			value = StringToNumber<int>(val);

		return Evaluate(sub, value);
	}
		break;

	case TRIG_DIFF:
		return Evaluate(info.IronMan(), StringToNumber<int>(val));

	case TRIG_TRAIT:
		if (info.PersonValid(target))
		{
			Person *p = &info.PersonGet(target);

			for (auto &i : p->trait)
				if (i.name == val)
					return true;
		}
		break;

	case TRIG_VAR:
	{
		int var_sub = 0, var_val = 0;
		bool compare_to_var = std::find_if(val.begin(), val.end(), IsChar) != val.end();

		info.VarGet(subject, var_sub);
		if (compare_to_var)
			info.VarGet(val, var_val);
		else
			var_val = StringToNumber<int>(val);

		return Evaluate(var_sub, var_val);
	}
		break;

	default:break;
	}

	return false;
}