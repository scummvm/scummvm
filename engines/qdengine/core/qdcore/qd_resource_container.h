
#ifndef __QD_RESOURCE_CONTAINER_H__
#define __QD_RESOURCE_CONTAINER_H__

#include "qd_animation.h"
#include "qd_sound.h"

#include "qd_resource_dispatcher.h"

//! Контейнер для хранения игровых ресурсов.
/**
Параметр шаблона class T - тип владельца ресурса, достаточно его forward declaration.
Спрайты хранятся как однокадровые анимации.
*/
template<class T>
class qdResourceContainer
{
public:
	qdResourceContainer();
	virtual ~qdResourceContainer();

	//! Добавляет ресурс из файла file_name и возвращает указатель на него.
	qdResource* add_resource(const char* file_name,const T* owner);

	//! Удаляет ресурс, если он нужен только для объекта owner.
	/**
	Если на данный ресурс есть еще ссылки, то он не будет удален.
	*/
	bool remove_resource(const char* file_name,const T* owner);

	//! Возвращает указатель на ресурс, соответствующий файлу с именем file_name.
	/**
	Если такой ресурс не найден, возвращает NULL.
	*/
	qdResource* get_resource(const char* file_name) const;

	typedef std::list<qdResource*> resource_list_t;
	//! Возвращает список ресурсов.
	const resource_list_t& resource_list() const { return resource_list_; }
	//! Возвращает всех владельцев ресурсов
	void get_owners(std::list<T*>& owners);
	//! Пишет список всех файлов в list.
	bool get_file_list(qdFileNameList& list) const;

private:

	struct eqstr
	{
		bool operator()(const char* s1, const char* s2) const {
			if(!s1 || !s2) return false;
			return strcmp(s1, s2) == 0;
		}
	};

	typedef std::hash_map<const char*,qdResource*,std::hash<const char*>,eqstr> resource_map_t;
	//! Хэш-мап с указателями на ресурсы, принадлежащие диспетчеру.
	resource_map_t resource_map_;

	//! Список ресурсов в контейнере.
	resource_list_t resource_list_;

	//! Диспетчер для отслеживания ссылок на ресурсы.
	qdResourceDispatcher<T> resource_dispatcher_;
};

template<class T>
qdResourceContainer<T>::qdResourceContainer(){}

template<class T>
qdResourceContainer<T>::~qdResourceContainer()
{
	for(resource_list_t::iterator it = resource_list_.begin(); it != resource_list_.end(); ++it)
		delete *it;
}

template<class T>
qdResource* qdResourceContainer<T>::add_resource(const char* file_name,const T* owner)
{
	resource_map_t::iterator it = resource_map_.find(file_name);
	if(it != resource_map_.end()){
		resource_dispatcher_.register_resource(it -> second,owner);
		return it -> second;
	}

	qdResource* p = NULL;
	switch(qdResource::file_format(file_name)){
		case qdResource::RES_ANIMATION:
			p = new qdAnimation;
			p -> set_resource_file(file_name);
			break;
		case qdResource::RES_SOUND:
			p = new qdSound;
			p -> set_resource_file(file_name);
			break;
		case qdResource::RES_SPRITE: {
				qdAnimation* anm = new qdAnimation;
				qdAnimationFrame* fr = new qdAnimationFrame;
				fr -> set_file(file_name);
				anm -> add_frame(fr);
				p = anm;
			}
			break;
	}

	if(!p) return NULL;

	resource_map_.insert(resource_map_t::value_type(file_name,p));
	resource_list_.push_back(p);

	resource_dispatcher_.register_resource(p,owner);

	return p;
}

template<class T>
bool qdResourceContainer<T>::remove_resource(const char* file_name,const T* owner)
{
	resource_map_t::iterator it = resource_map_.find(file_name);

	if(it == resource_map_.end()) return false;

	qdResource* p = it -> second;
	resource_dispatcher_.unregister_resource(p,owner);

	if(!resource_dispatcher_.is_registered(p)){
		resource_map_.erase(it);
		resource_list_t::iterator it = std::find(resource_list_.begin(),resource_list_.end(),p);
		if(it != resource_list_.end()){
			delete p;
			resource_list_.erase(it);
			return true;
		}

		return false;
	}

	return true;
}

template<class T>
qdResource* qdResourceContainer<T>::get_resource(const char* file_name) const
{
	if(!file_name) return NULL;

	resource_map_t::const_iterator it = resource_map_.find(file_name);
	if(it != resource_map_.end()) return it -> second;

	return NULL;
}

template<class T>
void qdResourceContainer<T>::get_owners(std::list<T*>& owners)
{
	bool all_ok = true;
	for(resource_list_t::iterator it = resource_list_.begin(); it != resource_list_.end(); ++it){
		if((*it) -> resource_file())
		{
			T* ptr = const_cast<T*>(resource_dispatcher_.find_owner(*it));
			owners.push_back(ptr);
		}
	}
}

template<class T>
bool qdResourceContainer<T>::get_file_list(qdFileNameList& list) const
{
	for(resource_list_t::const_iterator it = resource_list_.begin(); it != resource_list_.end(); ++it){
		if((*it) -> resource_file())
			list.push_back((*it) -> resource_file());
	}
	return true;
}

#endif /* __QD_RESOURCE_CONTAINER_H__ */
