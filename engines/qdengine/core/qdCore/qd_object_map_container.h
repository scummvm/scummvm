#ifndef __QD_OBJECT_MAP_CONTAINER_H__
#define __QD_OBJECT_MAP_CONTAINER_H__

template <class T>
class qdObjectMapContainer
{
public:
	typedef std::list<T*> object_list_t;

	qdObjectMapContainer();
	~qdObjectMapContainer();

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

private:

	struct eqstr
	{
		bool operator()(const char* s1, const char* s2) const {
			if(!s1 || !s2) return false;
			return strcmp(s1, s2) == 0;
		}
	};

	typedef std::hash_map<const char*,T*,std::hash<const char*>,eqstr> object_map_t;

	object_list_t object_list_;
	object_map_t object_map_;
};

template <class T>
bool qdObjectMapContainer<T>::add_object(T* p)
{
	object_map_t::iterator it = object_map_.find(p -> name());
	if(it != object_map_.end())
		return false;

	object_map_.insert(object_map_t::value_type(p -> name(),p));
	object_list_.push_back(p);

	return true;
}

template <class T>
const T* qdObjectMapContainer<T>::get_object(const char* name) const
{
	object_map_t::const_iterator it = object_map_.find(name);
	if(it != object_map_.end())
		return it -> second;
	
	return NULL;
}

template <class T>
T* qdObjectMapContainer<T>::get_object(const char* name)
{
	object_map_t::iterator it = object_map_.find(name);
	if(it != object_map_.end())
		return it -> second;

	return NULL;
}

template <class T>
bool qdObjectMapContainer<T>::remove_object(T* p)
{
	object_list_t::iterator it;
	FOR_EACH(object_list_,it){
		if(*it == p){
			object_list_.erase(it);
			object_map_t::iterator im = object_map_.find(p -> name());
			if(im != object_map_.end())
				object_map_.erase(im);

			return true;
		}
	}

	return false;
}

template <class T>
bool qdObjectMapContainer<T>::remove_object(const char* name)
{
	T* p = get_object(name);
	if(!p) return false;
	
	return remove_object(p);
}

template <class T>
bool qdObjectMapContainer<T>::rename_object(T* p,const char* name)
{
	object_map_t::iterator im = object_map_.find(p -> name());
	if(im != object_map_.end()){
		object_map_.erase(im);
		p -> set_name(name);
		object_map_.insert(object_map_t::value_type(p -> name(),p));

		return true;
	}
	return false;
}

template <class T>
qdObjectMapContainer<T>::qdObjectMapContainer()
{
}

template <class T>
qdObjectMapContainer<T>::~qdObjectMapContainer()
{
	clear();
}

template <class T>
bool qdObjectMapContainer<T>::clear()
{
	object_map_.clear();

	for(object_list_t::iterator it = object_list_.begin(); it != object_list_.end(); ++it)
		delete *it;

	object_list_.clear();

	return true;
}

#endif /* __QD_OBJECT_MAP_CONTAINER_H__ */
