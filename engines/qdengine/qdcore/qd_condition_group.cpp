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

#include "qdengine/qd_precomp.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/qdcore/qd_condition_group.h"


namespace QDEngine {

qdConditionGroup::qdConditionGroup(conditions_mode_t md) : conditions_mode_(md) {
}

qdConditionGroup::qdConditionGroup(const qdConditionGroup &cg) : conditions_mode_(cg.conditions_mode_), conditions_(cg.conditions_) {
}

qdConditionGroup::~qdConditionGroup() {
}

qdConditionGroup &qdConditionGroup::operator = (const qdConditionGroup &cg) {
	if (this == &cg) return *this;

	conditions_mode_ = cg.conditions_mode_;
	conditions_ = cg.conditions_;

	return *this;
}

bool qdConditionGroup::add_condition(int condition_id) {
	conditions_container_t::iterator it = std::find(conditions_.begin(), conditions_.end(), condition_id);
	if (it != conditions_.end())
		return false;

	conditions_.push_back(condition_id);
	return true;
}

bool qdConditionGroup::remove_condition(int condition_id) {
	for (conditions_container_t::iterator it = conditions_.begin(); it != conditions_.end(); ++it) {
		if (*it > condition_id)
			(*it)--;
	}

	conditions_container_t::iterator it1 = std::find(conditions_.begin(), conditions_.end(), condition_id);
	if (it1 != conditions_.end())
		return false;

	conditions_.erase(it1);
	return true;
}

bool qdConditionGroup::load_script(const xml::tag *p) {
#ifndef _QUEST_EDITOR
	conditions_.reserve(p->data_size());
#endif

	if (const xml::tag * tp = p->search_subtag(QDSCR_TYPE))
		conditions_mode_ = conditions_mode_t(xml::tag_buffer(*tp).get_int());

	xml::tag_buffer buf(*p);
	for (int i = 0; i < p->data_size(); i++)
		conditions_.push_back(buf.get_int());

	return true;
}

bool qdConditionGroup::save_script(Common::SeekableWriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<condition_group");
	fh.writeString(Common::String::format(" type=\"%d\"", (int)conditions_mode_));
	fh.writeString(">");

	fh.writeString(Common::String::format("%lu", conditions_.size()));
	for (auto &it : conditions_) {
		fh.writeString(Common::String::format(" %d", it));
	}
	fh.writeString("</condition_group>\r\n");
	return true;
}

} // namespace QDEngine
