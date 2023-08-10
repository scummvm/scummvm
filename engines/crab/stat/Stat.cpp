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

#include "crab/crab.h"
#include "crab/GameParam.h"
#include "crab/stat/Stat.h"

namespace Crab {

namespace pyrodactyl {
namespace stat {
StatType stringToStatType(const Common::String &val) {
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

const char *statTypeToString(const StatType &val) {
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

void Stat::load(rapidxml::xml_node<char> *node) {
	loadNum(_cur, "cur", node);
	loadNum(_def, "def", node);
	loadNum(_min, "min", node);
	loadNum(_max, "max", node);
}

void StatGroup::load(rapidxml::xml_node<char> *node) {
	_val[STAT_HEALTH].load(node->first_node(STATNAME_HEALTH));
	_val[STAT_ATTACK].load(node->first_node(STATNAME_ATTACK));
	_val[STAT_DEFENSE].load(node->first_node(STATNAME_DEFENSE));
	_val[STAT_SPEED].load(node->first_node(STATNAME_SPEED));
	/*val[STAT_CHARISMA].load(node->first_node(STATNAME_CHARISMA));
	val[STAT_INTELLIGENCE].load(node->first_node(STATNAME_INTELLIGENCE));*/
}

void StatGroup::change(const pyrodactyl::stat::StatType &type, const int &change) {
	_val[type]._cur += change;
	_val[type].validate();
}

void StatGroup::set(const pyrodactyl::stat::StatType &type, const int &num) {
	_val[type]._cur = num;
	_val[type].validate();
}

void Stat::validate() {
	if (_cur < _min)
		_cur = _min;
	else if (_cur > _max)
		_cur = _max;
}

void Stat::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char *name) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, name);
	child->append_attribute(doc.allocate_attribute("cur", g_engine->_stringPool->get(_cur)));
	child->append_attribute(doc.allocate_attribute("def", g_engine->_stringPool->get(_def)));
	child->append_attribute(doc.allocate_attribute("min", g_engine->_stringPool->get(_min)));
	child->append_attribute(doc.allocate_attribute("max", g_engine->_stringPool->get(_max)));
	root->append_node(child);
}

} // End of namespace Crab
