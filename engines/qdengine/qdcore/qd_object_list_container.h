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

#ifndef QDENGINE_QDCORE_QD_OBJECT_LIST_CONTAINER_H
#define QDENGINE_QDCORE_QD_OBJECT_LIST_CONTAINER_H

#include "common/str.h"
#include "common/std/list.h"


namespace QDEngine {

template <class T>
class qdObjectListContainer {
public:
	typedef Std::list<T *> object_list_t;

	qdObjectListContainer();
	~qdObjectListContainer();

	const object_list_t &get_list() const {
		return _object_list;
	}

	T *get_object(const char *name);
	const T *get_object(const char *name) const;

	bool add_object(T *p);
	bool remove_object(T *p);
	bool rename_object(T *p, const char *name);
	bool remove_object(const char *name);
	bool is_in_list(const char *name) const {
		return (get_object(name) != 0);
	}
	bool is_in_list(const T *p) const {
		return (get_object(p->name()) != 0);
	}
	bool clear();

private:

	object_list_t _object_list;
};

template <class T>
bool qdObjectListContainer<T>::add_object(T *p) {
	if (get_object(p->name())) return false;
	_object_list.push_back(p);

	return true;
}

template <class T>
const T *qdObjectListContainer<T>::get_object(const char *name) const {
	if (!name) return NULL;

	for (typename object_list_t::const_iterator it = _object_list.begin(); it != _object_list.end(); ++it) {
		if (!scumm_stricmp(name, (*it)->name()))
			return *it;
	}

	return NULL;
}

template <class T>
T *qdObjectListContainer<T>::get_object(const char *name) {
	if (!name) return NULL;

	for (typename object_list_t::const_iterator it = _object_list.begin(); it != _object_list.end(); ++it) {
		if (!scumm_stricmp(name, (*it)->name()))
			return *it;
	}

	return NULL;
}

template <class T>
bool qdObjectListContainer<T>::remove_object(T *p) {
	for (typename object_list_t::iterator it = _object_list.begin(); it != _object_list.end(); ++it) {
		if (*it == p) {
			_object_list.erase(it);
			return true;
		}
	}

	return false;
}

template <class T>
bool qdObjectListContainer<T>::remove_object(const char *name) {
	T *p = get_object(name);
	if (!p) return false;

	return remove_object(p);
}

template <class T>
bool qdObjectListContainer<T>::rename_object(T *p, const char *name) {
	p->set_name(name);
	return true;
}

template <class T>
qdObjectListContainer<T>::qdObjectListContainer() {
}

template <class T>
qdObjectListContainer<T>::~qdObjectListContainer() {
	clear();
}

template <class T>
bool qdObjectListContainer<T>::clear() {
	for (typename object_list_t::iterator it = _object_list.begin(); it != _object_list.end(); ++it)
		delete *it;

	_object_list.clear();

	return true;
}

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_OBJECT_LIST_CONTAINER_H
