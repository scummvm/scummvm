#include "stdafx.h"

#include "Stat.h"

namespace pyrodactyl {
namespace stat {
StatType StringToStatType(const std::string &val) {
	if (val == STATNAME_HEALTH)
		return STAT_HEALTH;
	else if (val == STATNAME_ATTACK)
		return STAT_ATTACK;
	else if (val == STATNAME_DEFENSE)
		return STAT_DEFENSE;
	else if (val == STATNAME_SPEED)
		return STAT_SPEED;
	/*else if(val == STATNAME_CHARISMA) return STAT_CHARISMA;

	return STAT_INTELLIGENCE;*/
	return STAT_HEALTH;
}

const char *StatTypeToString(const StatType &val) {
	if (val == STAT_HEALTH)
		return STATNAME_HEALTH;
	else if (val == STAT_ATTACK)
		return STATNAME_ATTACK;
	else if (val == STAT_DEFENSE)
		return STATNAME_DEFENSE;
	else if (val == STAT_SPEED)
		return STATNAME_SPEED;
	/*else if(val == STAT_CHARISMA) return STATNAME_CHARISMA;

	return STATNAME_INTELLIGENCE;*/
	return STATNAME_HEALTH;
}
} // End of namespace stat
} // End of namespace pyrodactyl

using namespace pyrodactyl::stat;

void Stat::Load(rapidxml::xml_node<char> *node) {
	LoadNum(cur, "cur", node);
	LoadNum(def, "def", node);
	LoadNum(min, "min", node);
	LoadNum(max, "max", node);
}

void StatGroup::Load(rapidxml::xml_node<char> *node) {
	val[STAT_HEALTH].Load(node->first_node(STATNAME_HEALTH));
	val[STAT_ATTACK].Load(node->first_node(STATNAME_ATTACK));
	val[STAT_DEFENSE].Load(node->first_node(STATNAME_DEFENSE));
	val[STAT_SPEED].Load(node->first_node(STATNAME_SPEED));
	/*val[STAT_CHARISMA].Load(node->first_node(STATNAME_CHARISMA));
	val[STAT_INTELLIGENCE].Load(node->first_node(STATNAME_INTELLIGENCE));*/
}

void StatGroup::Change(const StatType &type, const int &change) {
	val[type].cur += change;
	val[type].Validate();
}

void StatGroup::Set(const StatType &type, const int &num) {
	val[type].cur = num;
	val[type].Validate();
}

void Stat::Validate() {
	if (cur < min)
		cur = min;
	else if (cur > max)
		cur = max;
}

void Stat::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char *name) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, name);
	child->append_attribute(doc.allocate_attribute("cur", gStrPool.Get(cur)));
	child->append_attribute(doc.allocate_attribute("def", gStrPool.Get(def)));
	child->append_attribute(doc.allocate_attribute("min", gStrPool.Get(min)));
	child->append_attribute(doc.allocate_attribute("max", gStrPool.Get(max)));
	root->append_node(child);
}