#pragma once

#include "common_header.h"
#include "trigger.h"

namespace pyrodactyl {
namespace event {
class TriggerSet {
	// The conditions needed to unlock whatever we want
	// Places where this is used - events, levels, fighting moves
	std::vector<Trigger> statement;

	// Is the thing unlocked or not?
	bool result;

public:
	TriggerSet() { result = true; }

	void Load(rapidxml::xml_node<char> *node);
	bool Evaluate(pyrodactyl::event::Info &info);
	void Add(const Trigger &t);

	bool Result() { return result; }
	void Result(bool val) { result = val; }

	void Clear(const bool &val = true) {
		statement.clear();
		result = val;
	}
	bool Empty() { return statement.empty(); }
};
} // End of namespace event
} // End of namespace pyrodactyl