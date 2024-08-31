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


#ifndef QDENGINE_QDCORE_QD_CONDITION_GROUP_H
#define QDENGINE_QDCORE_QD_CONDITION_GROUP_H

#include "qdengine/parser/xml_fwd.h"


namespace QDEngine {

//! Группа условий.
class qdConditionGroup {
public:
	//! Режим проверки условий.
	enum conditions_mode_t {
		//! "И" - должны выполниться все условия.
		CONDITIONS_AND,
		//! "ИЛИ" - достаточно выполнения одного из условий.
		CONDITIONS_OR
	};

	explicit qdConditionGroup(conditions_mode_t md = CONDITIONS_AND);
	qdConditionGroup(const qdConditionGroup &cg);

	qdConditionGroup &operator = (const qdConditionGroup &cg);

	~qdConditionGroup();

	typedef Std::vector<int> conditions_container_t;
	typedef conditions_container_t::const_iterator conditions_iterator_t;

	conditions_iterator_t conditions_begin() const {
		return _conditions.begin();
	}
	conditions_iterator_t conditions_end() const {
		return _conditions.end();
	}

	int conditions_size() const {
		return _conditions.size();
	}

	conditions_mode_t conditions_mode() const {
		return _conditions_mode;
	}
	void set_conditions_mode(conditions_mode_t mode) {
		_conditions_mode = mode;
	}

	bool add_condition(int condition_id);
	bool remove_condition(int condition_id);

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

private:

	conditions_mode_t _conditions_mode;
	conditions_container_t _conditions;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_CONDITION_GROUP_H
