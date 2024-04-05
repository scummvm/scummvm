#ifndef __QD_OBJECT_LIST_CONTAINER_H__
#define __QD_OBJECT_LIST_CONTAINER_H__

template <class T>
class qdObjectListContainer
{
public:
	typedef std::list<T*> object_list_t;

	qdObjectListContainer();
	~qdObjectListContainer();

	const object_list_t& get_list() const { return object_list_; }

	T* get_object(const char* name);
	const T* get_object(const char* name) const;

	bool add_object(T* p);
	bool remove_object(T* p);
	bool rename_object(T* p,const char* name);
	bool remove_object(const char* name);
	bool is_in_list(const char* name) const { return (get_object(name) != 0); }
	bool is_in_list(const T* p) const { return (get_object(p -> name()) != 0); }
	bool clear();

#ifdef _QUEST_EDITOR
	bool insert_object(T* p, const T* before);
#endif
private:

	object_list_t object_list_;
};

template <class T>
bool qdObjectListContainer<T>::add_object(T* p)
{
	if(get_object(p -> name())) return false;
	object_list_.push_back(p);

	return true;
}


#ifdef _QUEST_EDITOR
template<class T>
bool qdObjectListContainer<T>::insert_object(T* p, const T* before){
	if(get_object(p -> name())) return false;
	object_list_t::iterator i = 
		std::find(object_list_.begin(), object_list_.end(), before);
	return (object_list_.insert(i, p) != object_list_.end());
}
#endif // _QUEST_EDITOR

template <class T>
const T* qdObjectListContainer<T>::get_object(const char* name) const
{
	if(!name) return NULL;

	for(object_list_t::const_iterator it = object_list_.begin(); it != object_list_.end(); ++it){
		if(!strcmp(name,(*it) -> name()))
			return *it;
	}
	
	return NULL;
}

template <class T>
T* qdObjectListContainer<T>::get_object(const char* name)
{
	if(!name) return NULL;

	for(object_list_t::const_iterator it = object_list_.begin(); it != object_list_.end(); ++it){
		if(!strcmp(name,(*it) -> name()))
			return *it;
	}
	
	return NULL;
}

template <class T>
bool qdObjectListContainer<T>::remove_object(T* p)
{
	for(object_list_t::iterator it = object_list_.begin(); it != object_list_.end(); ++it){
		if(*it == p){
			object_list_.erase(it);
			return true;
		}
	}

	return false;
}

template <class T>
bool qdObjectListContainer<T>::remove_object(const char* name)
{
	T* p = get_object(name);
	if(!p) return false;
	
	return remove_object(p);
}

template <class T>
bool qdObjectListContainer<T>::rename_object(T* p,const char* name)
{
	p -> set_name(name);
	return true;
}

template <class T>
qdObjectListContainer<T>::qdObjectListContainer()
{
}

template <class T>
qdObjectListContainer<T>::~qdObjectListContainer()
{
	clear();
}

template <class T>
bool qdObjectListContainer<T>::clear()
{
	for(object_list_t::iterator it = object_list_.begin(); it != object_list_.end(); ++it)
		delete *it;

	object_list_.clear();

	return true;
}

#endif /* __QD_OBJECT_LIST_CONTAINER_H__ */

