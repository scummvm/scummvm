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

#ifndef QDENGINE_CORE_QDCORE_QD_OBJECT_MAP_CONTAINER_H
#define QDENGINE_CORE_QDCORE_QD_OBJECT_MAP_CONTAINER_H

#include "qdengine/qdengine.h"


namespace QDEngine {

template <class T>
class qdObjectMapContainer {
public:
	typedef std::list<T *> object_list_t;

	qdObjectMapContainer();
	~qdObjectMapContainer();

	const object_list_t &get_list() const {
		return object_list_;
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
		return (get_object(p -> name()) != 0);
	}
	bool clear();

private:

	typedef Common::HashMap<Common::String, T *> object_map_t;

	object_list_t object_list_;
	object_map_t object_map_;
};

template <class T>
bool qdObjectMapContainer<T>::add_object(T *p) {
	typename object_map_t::iterator it = object_map_.find(p -> name());
	if (it != object_map_.end())
		return false;

	object_map_[p->name()] = p;
	object_list_.push_back(p);

	return true;
}

template <class T>
const T *qdObjectMapContainer<T>::get_object(const char *name) const {

	typename object_map_t::const_iterator it = object_map_.find(name);
	if (it != object_map_.end())
		return it->_value;

	return NULL;
}

template <class T>
T *qdObjectMapContainer<T>::get_object(const char *name) {
	typename object_map_t::iterator it = object_map_.find(name);
	if (it != object_map_.end())
		return it->_value;

	return NULL;
}

template <class T>
bool qdObjectMapContainer<T>::remove_object(T *p) {
	typename object_list_t::iterator it;
	FOR_EACH(object_list_, it) {
		if (*it == p) {
			object_list_.erase(it);
			typename object_map_t::iterator im = object_map_.find(p -> name());
			if (im != object_map_.end())
				object_map_.erase(im);

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
	typename object_map_t::iterator im = object_map_.find(p -> name());
	if (im != object_map_.end()) {
		object_map_.erase(im);
		p -> set_name(name);
		object_map_[p->name()] = p;

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
	object_map_.clear();

	for (typename object_list_t::iterator it = object_list_.begin(); it != object_list_.end(); ++it)
		delete *it;

	object_list_.clear();

	return true;
}

} // namespace QDEngine

#endif /* QDENGINE_CORE_QDCORE_QD_OBJECT_MAP_CONTAINER_H */
