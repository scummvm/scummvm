#ifndef __FACTORY_H__
#define __FACTORY_H__

#include "XTL\StaticMap.h"
#include "XTL\SafeCast.h"
#include "XTL\StaticString.h"
#include "Handle.h"
#include <typeinfo>

////////////////////////////////////////////////////////////////////////
//							VTableFactory
////////////////////////////////////////////////////////////////////////
class VTableFactory
{
public:
	struct VTableCreator
	{
		virtual void* vtable() const { return 0; }
	};

	struct VTableHolder : VTableCreator
	{
		VTableHolder(void* vtable) : vtable_(vtable) {}
		void* vtable() const { return vtable_; }
	private:
		void* vtable_;
	};

	static void* getVTable(const char* typeName){
		VTableCreator* creator = map()[typeName];
		xxassert(creator, XBuffer() < "Не зарегистрирован класс: " < typeName);
		if(!creator)
			return 0;
		void* vtable = map()[typeName]->vtable();
		xassert(vtable);
		return vtable;
	}

	static void addVtable(const char* typeName, VTableCreator& creator)
	{
		map()[typeName] = &creator;
	}

	template<class T>
	static void addVtable(const T& t)
	{
		addVtable(typeid(T).name(), *new VTableHolder(*(int**)&t));
	}

private:
	typedef StaticMap<StaticString, VTableCreator*> Map;
	static Map& map() {
		return Singleton<Map>::instance();
	}
};

////////////////////////////////////////////////////////////////////////
//						FactoryArgs
////////////////////////////////////////////////////////////////////////
template<class BaseType>
class FactoryArg0
{
public:
	template<class Derived>
	BaseType* createArg() const
	{
		return new Derived;
	}
};

template<class BaseType, class Arg0>
class FactoryArg1
{
public:
	FactoryArg1() {
		valid_ = false;
	}

	void setArgument(const Arg0& arg0) {
		arg0_ = arg0;
		valid_ = true;
	}

	template<class Derived>
	BaseType* createArg() const
	{
		xassert(valid_);
		valid_ = false;
		return new Derived(arg0_);
	}

private:
	Arg0 arg0_;
	mutable bool valid_;
};

template<class BaseType, class Arg0, class Arg1>
class FactoryArg2
{
public:
	FactoryArg2() {
		valid_ = false;
	}

	void setArguments(const Arg0& arg0, const Arg1& arg1) {
		arg0_ = arg0;
		arg1_ = arg1;
		valid_ = true;
	}

	template<class Derived>
	BaseType* createArg() const
	{
		xassert(valid_);
		valid_ = false;

		return new Derived(arg0_, arg1_);
	}

private:
	Arg0 arg0_;
	Arg1 arg1_;
	mutable bool valid_;
};

////////////////////////////////////////////////////////////////////////
//							Factory
////////////////////////////////////////////////////////////////////////
template<class Key, class BaseType, class FactoryArg = FactoryArg0<BaseType> >
class Factory : public FactoryArg, public VTableFactory
{
public:
	struct CreatorBase : VTableCreator
	{
        virtual BaseType* create() const { return 0; }
        virtual const char* typeName() const { return ""; }
	};
	
	template<class Derived>
	struct Creator : CreatorBase
	{
		Creator() {
		}
		Creator(const Key& id) {
			instance().add(id, *this);
		}
		BaseType* create() const
		{
			return instance().createArg<Derived>();
		}
		const char* typeName() const 
		{ 
			return typeid(Derived).name(); 
		}

		void* vtable() const {
			static BaseType* object = create();
			return object ? *(int**)object : 0;
		}
	};

	void add(const Key& key, CreatorBase& creator_op)
	{
		addVtable(creator_op.typeName(), creator_op);

		if(creators_.find(key) != creators_.end()) {
			XBuffer msg;
			msg < "Попытка повторной регистрации класса в "
				< typeid(this).name();
			xxassert(0, msg);
		} else {
			creators_.insert(Creators::value_type(key, &creator_op));
		}
	}

	template<class Derived>
	void add(const Key& key)
	{
		static Creator<Derived> creator(key);
		add(key, creator);
	}

	BaseType* create(const Key& key, bool silent = false) const 
	{
		Creators::const_iterator it = creators_.find(key);
		if(it != creators_.end())
			return it->second->create();

		xassert(silent && "Неопознанный идентификатор класса");
		return 0;
	}

	const char* typeName(const Key& key, bool silent = false) const
	{
		Creators::const_iterator it = creators_.find(key);
		if(it != creators_.end())
			return it->second->typeName();

		xassert(silent && "Неопознанный идентификатор класса");
		return "";
	}

	const CreatorBase* find(const Key& key) const
	{
		Creators::const_iterator it = creators_.find(key);
		if(it != creators_.end())
			return it->second;

		return 0;
	}

	static Factory& instance() {
		return Singleton<Factory>::instance();
	}

protected:
	typedef StaticMap<Key, CreatorBase*> Creators;
	Creators creators_;
};

#define INTERNAL_UNIQUE_NAME2(x,y) x##y
#define INTERNAL_UNIQUE_NAME1(x,y) INTERNAL_UNIQUE_NAME2(x,y)
#define INTERNAL_UNIQUE_NAME INTERNAL_UNIQUE_NAME1(registerClass, __COUNTER__) 

// Имена могут содержать <>, ::
#define REGISTER_CLASS_IN_FACTORY(Factory, classID, derivedClass) \
	static Factory::Creator<derivedClass > INTERNAL_UNIQUE_NAME(classID); 

// Когда линковщик выкидывает глобальные объекты
#define DECLARE_SEGMENT(fileName) int dataSegment##fileName; 

#define FORCE_SEGMENT(fileName) \
	extern int dataSegment##fileName; \
	int* dataSegmentPtr##fileName = &dataSegment##fileName;

#endif /* __FACTORY_H__ */
