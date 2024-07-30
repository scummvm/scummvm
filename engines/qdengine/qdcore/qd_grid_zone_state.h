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

#ifndef QDENGINE_QDCORE_QD_GRID_ZONE_STATE_H
#define QDENGINE_QDCORE_QD_GRID_ZONE_STATE_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_conditional_object.h"

namespace QDEngine {

//! Состояние зоны на сетке сцены.
class qdGridZoneState : public qdConditionalObject {
public:
	explicit qdGridZoneState(bool st);
	qdGridZoneState(const qdGridZoneState &st);
	~qdGridZoneState();

	qdGridZoneState &operator = (const qdGridZoneState &st);

	int named_object_type() const {
		return QD_NAMED_OBJECT_GRID_ZONE_STATE;
	}

	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag *p);
	//! Запись данных в скрипт.
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	//! Включить или выключить зону.
	bool state() const {
		return _state;
	}

	qdConditionalObject::trigger_start_mode trigger_start();

	static const char *const ZONE_STATE_ON_NAME;
	static const char *const ZONE_STATE_OFF_NAME;

private:

	//! Включить или выключить зону.
	bool _state;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_GRID_ZONE_STATE_H
