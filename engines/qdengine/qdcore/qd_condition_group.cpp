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

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/qdcore/qd_condition_group.h"


namespace QDEngine {

qdConditionGroup::qdConditionGroup(conditions_mode_t md) : _conditions_mode(md) {
}

qdConditionGroup::qdConditionGroup(const qdConditionGroup &cg) : _conditions_mode(cg._conditions_mode), _conditions(cg._conditions) {
}

qdConditionGroup::~qdConditionGroup() {
}

qdConditionGroup &qdConditionGroup::operator = (const qdConditionGroup &cg) {
	if (this == &cg) return *this;

	_conditions_mode = cg._conditions_mode;
	_conditions = cg._conditions;

	return *this;
}

bool qdConditionGroup::add_condition(int condition_id) {
	conditions_container_t::iterator it = Common::find(_conditions.begin(), _conditions.end(), condition_id);
	if (it != _conditions.end())
		return false;

	_conditions.push_back(condition_id);
	return true;
}

bool qdConditionGroup::remove_condition(int condition_id) {
	for (conditions_container_t::iterator it = _conditions.begin(); it != _conditions.end(); ++it) {
		if (*it > condition_id)
			(*it)--;
	}

	conditions_container_t::iterator it1 = Common::find(_conditions.begin(), _conditions.end(), condition_id);
	if (it1 != _conditions.end())
		return false;

	_conditions.erase(it1);
	return true;
}

bool qdConditionGroup::load_script(const xml::tag *p) {
	_conditions.reserve(p->data_size());

	if (const xml::tag *tp = p->search_subtag(QDSCR_TYPE))
		_conditions_mode = conditions_mode_t(xml::tag_buffer(*tp).get_int());

	xml::tag_buffer buf(*p);
	for (int i = 0; i < p->data_size(); i++)
		_conditions.push_back(buf.get_int());

	return true;
}

bool qdConditionGroup::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<condition_group");
	if (debugChannelSet(-1, kDebugLog))
		fh.writeString(Common::String::format(" type=\"%s\"", _conditions_mode == CONDITIONS_AND ? "CONDITIONS_AND" : "CONDITIONS_OR"));
	else
		fh.writeString(Common::String::format(" type=\"%d\"", (int)_conditions_mode));

	fh.writeString(">");

	fh.writeString(Common::String::format("%u", _conditions.size()));
	for (auto &it : _conditions) {
		fh.writeString(Common::String::format(" %d", it));
	}
	fh.writeString("</condition_group>\r\n");
	return true;
}

} // namespace QDEngine
