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

#include "common/debug.h"
#include "common/stream.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_conditional_object.h"
#include "qdengine/qdcore/qd_game_scene.h"


namespace QDEngine {


qdConditionalObject::qdConditionalObject() : _conditions_mode(CONDITIONS_OR) {
}

qdConditionalObject::qdConditionalObject(const qdConditionalObject &obj) : qdNamedObject(obj),
	_conditions_mode(obj._conditions_mode),
	_conditions(obj._conditions),
	_condition_groups(obj._condition_groups) {
}

qdConditionalObject::~qdConditionalObject() {
}

qdConditionalObject &qdConditionalObject::operator = (const qdConditionalObject &obj) {
	if (this == &obj) return *this;

	*static_cast<qdNamedObject *>(this) = obj;

	_conditions_mode = obj._conditions_mode;
	_conditions = obj._conditions;
	_condition_groups = obj._condition_groups;

	return *this;
}

int qdConditionalObject::add_condition(const qdCondition *p) {
	_conditions.push_back(*p);
	_conditions.back().set_owner(this);

	return _conditions.size() - 1;
}

bool qdConditionalObject::update_condition(int num, const qdCondition &p) {
	assert(num >= 0 && num < (int)_conditions.size());

	qdCondition &cond = _conditions[num];
	cond = p;
	cond.set_owner(this);

	return true;
}

bool qdConditionalObject::check_conditions() {
	qdCondition::clear_successful_clicks();

	if (!_conditions.empty()) {
		switch (conditions_mode()) {
		case CONDITIONS_AND:
			for (conditions_container_t::iterator it = _conditions.begin(); it != _conditions.end(); ++it) {
				if (!it->is_in_group()) {
					if (!it->check())
						return false;
				}
			}
			for (condition_groups_container_t::iterator it = _condition_groups.begin(); it != _condition_groups.end(); ++it) {
				if (!check_group_conditions(*it))
					return false;
			}
			return true;
		case CONDITIONS_OR:
			for (conditions_container_t::iterator it = _conditions.begin(); it != _conditions.end(); ++it) {
				if (!it->is_in_group()) {
					if (it->check())
						return true;
				}
			}
			for (condition_groups_container_t::iterator it = _condition_groups.begin(); it != _condition_groups.end(); ++it) {
				if (check_group_conditions(*it))
					return true;
			}
			return false;
		}
	}

	return true;
}

bool qdConditionalObject::remove_conditon(int idx) {
	assert(idx >= 0 && idx < (int)_conditions.size());

	_conditions.erase(_conditions.begin() + idx);

	for (condition_groups_container_t::iterator it = _condition_groups.begin(); it != _condition_groups.end(); ++it)
		it->remove_condition(idx);

	return true;
}

bool qdConditionalObject::load_conditions_script(const xml::tag *p) {
	int count = 0;
	int gr_count = 0;

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_CONDITION:
			count++;
			break;
		case QDSCR_CONDITION_GROUP:
			gr_count++;
			break;
		}
	}

	if (count) _conditions.resize(count);
	conditions_container_t::iterator ict = _conditions.begin();

	if (gr_count) _condition_groups.resize(gr_count);
	condition_groups_container_t::iterator igt = _condition_groups.begin();

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_CONDITION:
			if (const xml::tag *tp = it->search_subtag(QDSCR_TYPE)) {
				qdCondition *cp = &*ict;
				cp->set_type(qdCondition::ConditionType(xml::tag_buffer(*tp).get_int()));
				cp->load_script(&*it);
				cp->set_owner(this);
			}
			++ict;
			break;
		case QDSCR_CONDITION_GROUP:
			if (const xml::tag *tp = it->search_subtag(QDSCR_TYPE))
				igt->set_conditions_mode(qdConditionGroup::conditions_mode_t(xml::tag_buffer(*tp).get_int()));
			igt->load_script(&*it);
			++igt;
			break;
		case QDSCR_CONDITIONS_MODE:
			set_conditions_mode(ConditionsMode((xml::tag_buffer(*it).get_int())));
			break;
		}
	}

	for (uint i = 0; i < _conditions.size(); i++) {
		if (is_condition_in_group(i))
			_conditions[i].add_group_reference();
	}

	return true;
}

bool qdConditionalObject::save_conditions_script(Common::WriteStream &fh, int indent) const {
	if (_conditions.size()) {
		for (auto &it : _conditions) {
			it.save_script(fh, indent + 1);
		}

		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}

		fh.writeString(Common::String::format("<conditions_mode>%d</conditions_mode>\r\n", _conditions_mode));
	}

	for (auto  &it : _condition_groups) {
		it.save_script(fh, indent);
	}

	return true;
}

void qdConditionalObject::conditions_quant(float dt) {
	for (auto &it : _conditions) {
		it.quant(dt);
	}
}

bool qdConditionalObject::load_data(Common::SeekableReadStream &fh, int save_version) {
	debugC(4, kDebugSave, "    qdConditionalObject::load_data(): before: %d", (int)fh.pos());
	if (!qdNamedObject::load_data(fh, save_version))
		return false;

	for (auto &it : _conditions)
		it.load_data(fh, save_version);

	debugC(4, kDebugSave, "    qdConditionalObject::load_data(): after: %d", (int)fh.pos());
	return true;
}

bool qdConditionalObject::save_data(Common::WriteStream &fh) const {
	debugC(4, kDebugSave, "    qdConditionalObject::save_data(): before: %d", (int)fh.pos());
	if (!qdNamedObject::save_data(fh)) {
		return false;
	}

	for (auto &it : _conditions)
		it.save_data(fh);

	debugC(4, kDebugSave, "    qdConditionalObject::save_data(): after: %d", (int)fh.pos());
	return true;
}

bool qdConditionalObject::check_group_conditions(const qdConditionGroup &gr) {
	switch (gr.conditions_mode()) {
	case qdConditionGroup::CONDITIONS_AND:
		for (qdConditionGroup::conditions_iterator_t it = gr.conditions_begin(); it != gr.conditions_end(); ++it) {
			if (!_conditions[*it].check())
				return false;
		}
		return true;
	case qdConditionGroup::CONDITIONS_OR:
		for (qdConditionGroup::conditions_iterator_t it = gr.conditions_begin(); it != gr.conditions_end(); ++it) {
			if (_conditions[*it].check())
				return true;
		}
		return false;
	}

	return true;
}

bool qdConditionalObject::is_condition_in_group(int condition_idx) const {
	for (condition_groups_container_t::const_iterator it = _condition_groups.begin(); it != _condition_groups.end(); ++it) {
		if (Common::find(it->conditions_begin(), it->conditions_end(), condition_idx) != it->conditions_end())
			return true;
	}

	return false;
}

int qdConditionalObject::add_condition_group(const qdConditionGroup *p) {
	_condition_groups.push_back(*p);
	return _condition_groups.size() - 1;
}

bool qdConditionalObject::update_condition_group(int num, const qdConditionGroup &p) {
	assert(num >= 0 && num < (int)_condition_groups.size());

	qdConditionGroup &gr = _condition_groups[num];
	gr = p;

	for (uint i = 0; i < _conditions.size(); i++) {
		if (is_condition_in_group(i))
			_conditions[i].add_group_reference();
		else
			_conditions[i].remove_group_reference();
	}

	return true;
}

bool qdConditionalObject::remove_conditon_group(int idx) {
	assert(idx >= 0 && idx < (int)_condition_groups.size());

	_condition_groups.erase(_condition_groups.begin() + idx);

	for (uint i = 0; i < _conditions.size(); i++) {
		if (is_condition_in_group(i))
			_conditions[i].add_group_reference();
		else
			_conditions[i].remove_group_reference();
	}

	return true;
}


bool qdConditionalObject::init() {
	bool result = true;

	for (uint i = 0; i < _conditions.size(); i++) {
		if (!_conditions[i].init())
			result = false;
	}

	return result;
}

bool qdConditionalObject::trigger_can_start() const {
	if (const qdGameScene *p = static_cast<const qdGameScene *>(owner(QD_NAMED_OBJECT_SCENE)))
		return p->is_active();

	return true;
}
} // namespace QDEngine
