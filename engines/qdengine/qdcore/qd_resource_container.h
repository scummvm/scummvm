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


#ifndef QDENGINE_QDCORE_QD_RESOURCE_CONTAINER_H
#define QDENGINE_QDCORE_QD_RESOURCE_CONTAINER_H

#include "qdengine/qdcore/qd_animation.h"
#include "qdengine/qdcore/qd_resource_dispatcher.h"
#include "qdengine/qdcore/qd_sound.h"


//! Контейнер для хранения игровых ресурсов.
/**
Параметр шаблона class T - тип владельца ресурса, достаточно его forward declaration.
Спрайты хранятся как однокадровые анимации.
*/

namespace QDEngine {

template<class T>
class qdResourceContainer {
public:
	qdResourceContainer();
	virtual ~qdResourceContainer();

	//! Добавляет ресурс из файла file_name и возвращает указатель на него.
	qdResource *add_resource(const Common::Path file_name, const T *owner);

	//! Удаляет ресурс, если он нужен только для объекта owner.
	/**
	Если на данный ресурс есть еще ссылки, то он не будет удален.
	*/
	bool remove_resource(const Common::Path file_name, const T *owner);

	//! Возвращает указатель на ресурс, соответствующий файлу с именем file_name.
	/**
	Если такой ресурс не найден, возвращает NULL.
	*/
	qdResource *get_resource(const Common::Path file_name) const;

	typedef Std::list<qdResource *> resource_list_t;
	//! Возвращает список ресурсов.
	const resource_list_t &resource_list() const {
		return _resource_list;
	}
	//! Возвращает всех владельцев ресурсов
	void get_owners(Std::list<T *> &owners);

private:

	typedef Common::HashMap<Common::String, qdResource *> resource_map_t;
	//! Хэш-мап с указателями на ресурсы, принадлежащие диспетчеру.
	resource_map_t _resource_map;

	//! Список ресурсов в контейнере.
	resource_list_t _resource_list;

	//! Диспетчер для отслеживания ссылок на ресурсы.
	qdResourceDispatcher<T> _resource_dispatcher;
};

template<class T>
qdResourceContainer<T>::qdResourceContainer() {}

template<class T>
qdResourceContainer<T>::~qdResourceContainer() {
	for (resource_list_t::iterator it = _resource_list.begin(); it != _resource_list.end(); ++it)
		delete *it;
}

template<class T>
qdResource *qdResourceContainer<T>::add_resource(const Common::Path file_name, const T *owner) {
	typename resource_map_t::iterator it = _resource_map.find(file_name.toString());
	if (it != _resource_map.end()) {
		_resource_dispatcher.register_resource(it->_value, owner);
		return it->_value;
	}

	qdResource *p = NULL;
	switch (qdResource::file_format(file_name)) {
	case qdResource::RES_ANIMATION:
		p = new qdAnimation;
		p->set_resource_file(file_name);
		break;
	case qdResource::RES_SOUND:
		p = new qdSound;
		p->set_resource_file(file_name);
		break;
	case qdResource::RES_SPRITE: {
			qdAnimation *anm = new qdAnimation;
			qdAnimationFrame *fr = new qdAnimationFrame;
			fr->set_file(file_name);
			anm->add_frame(fr);
			p = anm;
		}
		break;
	default:
		break;
	}

	if (!p)
		return NULL;

	_resource_map[file_name.toString()] = p;
	_resource_list.push_back(p);

	_resource_dispatcher.register_resource(p, owner);

	return p;
}

template<class T>
bool qdResourceContainer<T>::remove_resource(const Common::Path file_name, const T *owner) {
	typename resource_map_t::iterator it = _resource_map.find(file_name.toString());

	if (it == _resource_map.end()) return false;

	qdResource *p = it->_value;
	_resource_dispatcher.unregister_resource(p, owner);

	if (!_resource_dispatcher.is_registered(p)) {
		_resource_map.erase(it);
		resource_list_t::iterator it2 = Common::find(_resource_list.begin(), _resource_list.end(), p);
		if (it2 != _resource_list.end()) {
			delete p;
			_resource_list.erase(it2);
			return true;
		}

		return false;
	}

	return true;
}

template<class T>
qdResource *qdResourceContainer<T>::get_resource(const Common::Path file_name) const {
	if (file_name.empty()) return NULL;

	typename resource_map_t::const_iterator it = _resource_map.find(file_name.toString());
	if (it != _resource_map.end())
		return it->_value;

	return NULL;
}

template<class T>
void qdResourceContainer<T>::get_owners(Std::list<T *> &owners) {
	for (resource_list_t::iterator it = _resource_list.begin(); it != _resource_list.end(); ++it) {
		if (!(*it)->resource_file().empty()) {
			T *ptr = const_cast<T *>(_resource_dispatcher.find_owner(*it));
			owners.push_back(ptr);
		}
	}
}

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_RESOURCE_CONTAINER_H
