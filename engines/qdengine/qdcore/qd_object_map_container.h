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

#ifndef QDENGINE_QDCORE_QD_OBJECT_MAP_CONTAINER_H
#define QDENGINE_QDCORE_QD_OBJECT_MAP_CONTAINER_H

#include "common/system.h"

#include "qdengine/qdengine.h"


namespace QDEngine {

template <class T>
class qdObjectMapContainer {
public:
	typedef Std::list<T *> object_list_t;

	qdObjectMapContainer();
	~qdObjectMapContainer();

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

	typedef Common::HashMap<Common::String, T *> object_map_t;

	object_list_t _object_list;
	object_map_t _object_map;
};

template <class T>
bool qdObjectMapContainer<T>::add_object(T *p) {
	typename object_map_t::iterator it = _object_map.find(p->name());
	if (it != _object_map.end())
		return false;

	_object_map[p->name()] = p;
	_object_list.push_back(p);

	return true;
}

template <class T>
const T *qdObjectMapContainer<T>::get_object(const char *name) const {

	typename object_map_t::const_iterator it = _object_map.find(name);
	if (it != _object_map.end())
		return it->_value;

	return NULL;
}

template <class T>
T *qdObjectMapContainer<T>::get_object(const char *name) {
	typename object_map_t::iterator it = _object_map.find(name);
	if (it != _object_map.end())
		return it->_value;

	return NULL;
}

template <class T>
bool qdObjectMapContainer<T>::remove_object(T *p) {
	typename object_list_t::iterator it;
	for (it = _object_list.begin(); it != _object_list.end(); it++) {
		if (*it == p) {
			_object_list.erase(it);
			typename object_map_t::iterator im = _object_map.find(p->name());
			if (im != _object_map.end())
				_object_map.erase(im);

			return true;
		}
	}

	return false;
}

template <class T>
bool qdObjectMapContainer<T>::remove_object(const char *name) {
	T *p = get_object(name);
	if (!p) return false;

	return remove_object(p);
}

template <class T>
bool qdObjectMapContainer<T>::rename_object(T *p, const char *name) {
	typename object_map_t::iterator im = _object_map.find(p->name());
	if (im != _object_map.end()) {
		_object_map.erase(im);
		p->set_name(name);
		_object_map[p->name()] = p;

		return true;
	}
	return false;
}

template <class T>
qdObjectMapContainer<T>::qdObjectMapContainer() {
}

template <class T>
qdObjectMapContainer<T>::~qdObjectMapContainer() {
	clear();
}

template <class T>
bool qdObjectMapContainer<T>::clear() {
	_object_map.clear();

	for (typename object_list_t::iterator it = _object_list.begin(); it != _object_list.end(); ++it)
		delete *it;

	_object_list.clear();

	return true;
}

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_OBJECT_MAP_CONTAINER_H
