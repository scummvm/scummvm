#include "stdafx.h"

#include "triggerset.h"

using namespace pyrodactyl::event;

void TriggerSet::Load(rapidxml::xml_node<char> *node) {
	for (rapidxml::xml_node<char> *n = node->first_node("trigger"); n != NULL; n = n->next_sibling("trigger")) {
		Trigger t(n);
		statement.push_back(t);
	}
}

void TriggerSet::Add(const Trigger &t) {
	statement.push_back(t);
}

bool TriggerSet::Evaluate(pyrodactyl::event::Info &info) {
	result = true;

	if (statement.empty() == false) {
		RelOp next_op = OP_AND;
		result = true;
		bool cur_result = false;

		for (auto i = statement.begin(); i != statement.end(); ++i) {
			if (i->negate)
				cur_result = !i->Evaluate(info);
			else
				cur_result = i->Evaluate(info);

			if (next_op == OP_AND)
				result = cur_result && result;
			else
				result = cur_result || result;

			next_op = i->rel;
		}
	} else
		result = true;

	return result;
}