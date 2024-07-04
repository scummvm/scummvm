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

#ifndef QDENGINE_CORE_QDCORE_QD_RESOURCE_DISPATCHER_H
#define QDENGINE_CORE_QDCORE_QD_RESOURCE_DISPATCHER_H

#include "qdengine/core/qdcore/qd_resource.h"


namespace QDEngine {

//! Диспетчер ресурсов.
template<class T>
class qdResourceDispatcher {
public:
	qdResourceDispatcher() {}
	virtual ~qdResourceDispatcher() {}

	//! Регистрация ресурса.
	bool register_resource(qdResource *res, const T *res_owner) {
		qdResourceHandle<T> hres(res, res_owner);
		typename handle_container_t::iterator it = std::find(handles_.begin(), handles_.end(), hres);

		if (it != handles_.end()) return false;
		handles_.push_back(hres);

		return true;
	}

	//! Отмена регистрации ресурса.
	bool unregister_resource(qdResource *res, const T *res_owner) {
		qdResourceHandle<T> hres(res, res_owner);
		typename handle_container_t::iterator it = std::find(handles_.begin(), handles_.end(), hres);

		if (it != handles_.end()) {
			handles_.erase(it);
			return true;
		}

		return false;
	}

	//! Возвращает true, если ресурс res (опционально - с владельцем res_owner) есть в списке.
	bool is_registered(const qdResource *res, const T *res_owner = NULL) const {
		if (res_owner) {
			qdResourceHandle<T> hres(const_cast<qdResource *>(res), res_owner);
			typename handle_container_t::const_iterator it = std::find(handles_.begin(), handles_.end(), hres);
			return (it != handles_.end());
		} else {
			typename handle_container_t::const_iterator it = std::find(handles_.begin(), handles_.end(), *res);
			return (it != handles_.end());
		}
	}

	const T *find_owner(const qdResource *res) const {
		typename handle_container_t::const_iterator it = std::find(handles_.begin(), handles_.end(), *res);
		if (handles_.end() == it) return NULL;
		return (*it).resource_owner();
	}

	//! Загружает в память данные для ресурсов.
	void load_resources(const T *owner = NULL) const {
		if (owner) {
			for (typename handle_container_t::const_iterator it = handles_.begin(); it != handles_.end(); ++it) {
				if (it->resource_owner() == owner)
					it->load_resource();
			}
		} else {
			for (typename handle_container_t::const_iterator it = handles_.begin(); it != handles_.end(); ++it)
				it->load_resource();
		}
	}

	//! Выгружает из памяти данные ресурсов.
	void release_resources(const T *owner = NULL, const T *hold_owner = NULL) const {
		if (owner) {
			for (typename handle_container_t::const_iterator it = handles_.begin(); it != handles_.end(); ++it) {
				if (it->resource_owner() == owner && (!hold_owner || !is_registered(it->resource(), hold_owner)))
					it->release_resource();
			}
		} else {
			if (hold_owner) {
				for (typename handle_container_t::const_iterator it = handles_.begin(); it != handles_.end(); ++it) {
					if (it->resource_owner() != hold_owner)
						it->release_resource();
				}
			} else {
				for (typename handle_container_t::const_iterator it = handles_.begin(); it != handles_.end(); ++it)
					it->release_resource();
			}
		}
	}

	//! Загружает в память данные ресурса, если они еще не загружены.
	bool load_resource(qdResource *res, const T *res_owner) {
		qdResourceHandle<T> hres(res, res_owner);
		register_resource(res, res_owner);
		return hres.load_resource();
	}

	//! Выгружает из памяти данные ресурса, если на него нет больше ссылок.
	bool release_resource(qdResource *res, const T *res_owner) {
		unregister_resource(res, res_owner);
		if (!is_registered(res)) {
			qdResourceHandle<T> hres(res, res_owner);
			return hres.release_resource();
		}

		return false;
	}

protected:

	//! Хэндл для управления ресурсами.
	template<class U>
	class qdResourceHandle {
	public:
		qdResourceHandle(qdResource *res, const U *res_owner) : resource_(res), resource_owner_(res_owner) {}
		qdResourceHandle(const qdResourceHandle<U> &h) : resource_(h.resource_), resource_owner_(h.resource_owner_) {}
		~qdResourceHandle() {}

		qdResourceHandle<U> &operator = (const qdResourceHandle<U> &h) {
			if (this == &h) return *this;
			resource_ = h.resource_;
			resource_owner_ = h.resource_owner_;
			return *this;
		}

		bool operator == (const qdResource &res) const {
			return (resource_ == &res);
		}
		bool operator == (const qdResourceHandle<U> &h) const {
			return (resource_ == h.resource_ && resource_owner_ == h.resource_owner_);
		}

		//! Возвращает указатель на ресурс.
		qdResource *resource() const {
			return resource_;
		}
		//! Возвращает указатель на владельца ресурса.
		const U *resource_owner() const {
			return resource_owner_;
		}

		//! Загружает ресурс в память.
		bool load_resource() const {
			if (!resource_->is_resource_loaded())
				return resource_->load_resource();
			return true;
		}
		//! Выгружает ресурс из памяти.
		bool release_resource() const {
			if (resource_->is_resource_loaded())
				return resource_->free_resource();
			return true;
		}

	private:

		//! Указатель на ресурс.
		mutable qdResource *resource_;
		//! Указатель на владельца ресурса.
		const T *resource_owner_;
	};

	typedef std::list< qdResourceHandle<T> > handle_container_t;

	//! Хэндлы ресурсов.
	handle_container_t handles_;
};

} // namespace QDEngine

#endif // QDENGINE_CORE_QDCORE_QD_RESOURCE_DISPATCHER_H
