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


#ifndef QDENGINE_QDCORE_QD_CONDITION_OBJECT_REFERENCE_H
#define QDENGINE_QDCORE_QD_CONDITION_OBJECT_REFERENCE_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_named_object_reference.h"


namespace QDEngine {

class qdNamedObject;

class qdConditionObjectReference {
public:
	qdConditionObjectReference();
	qdConditionObjectReference(const qdConditionObjectReference &ref);
	~qdConditionObjectReference();

	qdConditionObjectReference &operator = (const qdConditionObjectReference &ref);

	void set_object(qdNamedObject *p);
	bool find_object();

	const qdNamedObject *object() const {
		return _object;
	}

	bool is_empty() const {
		if (_object) return true;
		else return false;
	}

	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag *p);
	//! Запись данных в скрипт.
	bool save_script(Common::WriteStream &fh, int indent = 0, int id = 0) const;

private:

	qdNamedObject *_object;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_CONDITION_OBJECT_REFERENCE_H
