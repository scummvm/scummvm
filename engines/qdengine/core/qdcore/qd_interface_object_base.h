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


#ifndef QDENGINE_CORE_QDCORE_QD_INTERFACE_OBJECT_BASE_H
#define QDENGINE_CORE_QDCORE_QD_INTERFACE_OBJECT_BASE_H

#include "qdengine/core/qdcore/qd_named_object_base.h"


namespace QDEngine {

//! Базовый класс для всех интерфейсных объектов.
class qdInterfaceObjectBase : public qdNamedObjectBase {
public:
	qdInterfaceObjectBase();
	qdInterfaceObjectBase(const qdInterfaceObjectBase &obj);
	~qdInterfaceObjectBase();

	//! Устанавливает владельца объекта.
	void set_owner(qdInterfaceObjectBase *p) {
		owner_ = p;
	}
	//! Возвращает указатель на владельца объекта.
	qdInterfaceObjectBase *owner() const {
		return owner_;
	}

protected:

	qdInterfaceObjectBase &operator = (const qdInterfaceObjectBase &obj);

private:

	//! Владелец объекта.
	mutable qdInterfaceObjectBase *owner_;
};

} // namespace QDEngine

#endif // QDENGINE_CORE_QDCORE_QD_INTERFACE_OBJECT_BASE_H
