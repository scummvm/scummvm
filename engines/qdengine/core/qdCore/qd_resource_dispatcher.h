#ifndef __QD_RESOURCE_DISPATCHER_H__
#define __QD_RESOURCE_DISPATCHER_H__

#include "qd_resource.h"

//! Диспетчер ресурсов.
template<class T>
class qdResourceDispatcher
{
public:
	qdResourceDispatcher(){}
	virtual ~qdResourceDispatcher(){}

	//! Регистрация ресурса.
	bool register_resource(qdResource* res,const T* res_owner)
	{
		qdResourceHandle<T> hres(res,res_owner);
		handle_container_t::iterator it = std::find(handles_.begin(),handles_.end(),hres);

		if(it != handles_.end()) return false;
		handles_.push_back(hres);

		return true;
	}

	//! Отмена регистрации ресурса.
	bool unregister_resource(qdResource* res,const T* res_owner)
	{
		qdResourceHandle<T> hres(res,res_owner);
		handle_container_t::iterator it = std::find(handles_.begin(),handles_.end(),hres);

		if(it != handles_.end()){
			handles_.erase(it);
			return true;
		}

		return false;
	}

	//! Возвращает true, если ресурс res (опционально - с владельцем res_owner) есть в списке.
	bool is_registered(const qdResource* res,const T* res_owner = NULL) const
	{
		if(res_owner){
			qdResourceHandle<T> hres(const_cast<qdResource*>(res),res_owner);
			handle_container_t::const_iterator it = std::find(handles_.begin(),handles_.end(),hres);
			return (it != handles_.end());
		}
		else {
			handle_container_t::const_iterator it = std::find(handles_.begin(),handles_.end(),*res);
			return (it != handles_.end());
		}
	}

	const T* find_owner(const qdResource* res) const
	{
		handle_container_t::const_iterator it = std::find(handles_.begin(),handles_.end(),*res);
		if (handles_.end() == it) return NULL;
		return (*it).resource_owner();
	}

	//! Загружает в память данные для ресурсов.
	void load_resources(const T* owner = NULL) const
	{
		if(owner){
			for(handle_container_t::const_iterator it = handles_.begin(); it != handles_.end(); ++it){
				if(it -> resource_owner() == owner)
					it -> load_resource();
			}
		}
		else {
			for(handle_container_t::const_iterator it = handles_.begin(); it != handles_.end(); ++it)
				it -> load_resource();
		}
	}

	//! Выгружает из памяти данные ресурсов.
	void release_resources(const T* owner = NULL,const T* hold_owner = NULL) const
	{
		if(owner){
			for(handle_container_t::const_iterator it = handles_.begin(); it != handles_.end(); ++it){
				if(it -> resource_owner() == owner && (!hold_owner || !is_registered(it -> resource(),hold_owner)))
					it -> release_resource();
			}
		}
		else {
			if(hold_owner){
				for(handle_container_t::const_iterator it = handles_.begin(); it != handles_.end(); ++it){
					if(it -> resource_owner() != hold_owner)
						it -> release_resource();
				}
			}
			else {
				for(handle_container_t::const_iterator it = handles_.begin(); it != handles_.end(); ++it)
					it -> release_resource();
			}
		}
	}

	//! Загружает в память данные ресурса, если они еще не загружены.
	bool load_resource(qdResource* res,const T* res_owner)
	{
		qdResourceHandle<T> hres(res,res_owner);
		register_resource(res,res_owner);
		return hres.load_resource();
	}

	//! Выгружает из памяти данные ресурса, если на него нет больше ссылок.
	bool release_resource(qdResource* res,const T* res_owner)
	{
		unregister_resource(res,res_owner);
		if(!is_registered(res)){
			qdResourceHandle<T> hres(res,res_owner);
			return hres.release_resource();
		}

		return false;
	}

protected:

	//! Хэндл для управления ресурсами.
	template<class T>
	class qdResourceHandle
	{
	public:
		qdResourceHandle(qdResource* res,const T* res_owner) : resource_(res), resource_owner_(res_owner) {}
		qdResourceHandle(const qdResourceHandle<T>& h) : resource_(h.resource_), resource_owner_(h.resource_owner_) {}
		~qdResourceHandle(){}

		qdResourceHandle<T>& operator = (const qdResourceHandle<T>& h){
			if(this == &h) return *this;
			resource_ = h.resource_;
			resource_owner_ = h.resource_owner_;
			return *this;
		}

		bool operator == (const qdResource& res) const { return (resource_ == &res); }
		bool operator == (const qdResourceHandle<T>& h) const { return (resource_ == h.resource_ && resource_owner_ == h.resource_owner_); }

		//! Возвращает указатель на ресурс.
		qdResource* resource() const { return resource_; }
		//! Возвращает указатель на владельца ресурса.
		const T* resource_owner() const { return resource_owner_; }

		//! Загружает ресурс в память.
		bool load_resource() const {
			if(!resource_ -> is_resource_loaded())
				return resource_ -> load_resource();
			return true;
		}
		//! Выгружает ресурс из памяти.
		bool release_resource() const {
			if(resource_ -> is_resource_loaded())
				return resource_ -> free_resource();
			return true;
		}

	private:

		//! Указатель на ресурс.
		mutable qdResource* resource_;
		//! Указатель на владельца ресурса.
		const T* resource_owner_;
	};

	typedef std::list< qdResourceHandle<T> > handle_container_t;

	//! Хэндлы ресурсов.
	handle_container_t handles_;
};

#endif /* __QD_RESOURCE_DISPATCHER_H__ */
