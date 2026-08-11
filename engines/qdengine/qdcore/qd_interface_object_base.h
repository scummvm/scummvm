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


#ifndef QDENGINE_QDCORE_QD_INTERFACE_OBJECT_BASE_H
#define QDENGINE_QDCORE_QD_INTERFACE_OBJECT_BASE_H

#include "qdengine/qdcore/qd_named_object_base.h"


namespace QDEngine {

//! Базовый класс для всех интерфейсных объектов.
class qdInterfaceObjectBase : public qdNamedObjectBase {
public:
	qdInterfaceObjectBase();
	qdInterfaceObjectBase(const qdInterfaceObjectBase &obj);
	~qdInterfaceObjectBase();

	//! Устанавливает владельца объекта.
	void set_owner(qdInterfaceObjectBase *p) {
		_owner = p;
	}
	//! Возвращает указатель на владельца объекта.
	qdInterfaceObjectBase *owner() const {
		return _owner;
	}

protected:

	qdInterfaceObjectBase &operator = (const qdInterfaceObjectBase &obj);

private:

	//! Владелец объекта.
	mutable qdInterfaceObjectBase *_owner;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_INTERFACE_OBJECT_BASE_H
