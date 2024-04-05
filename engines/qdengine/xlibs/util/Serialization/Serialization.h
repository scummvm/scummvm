#ifndef __SERIALIZATION_H_INCLUDED__
#define __SERIALIZATION_H_INCLUDED__

#include <vector>
#include <list>
#include <typeinfo>
using namespace std;

#include "my_stl.h"
#include "xutil.h"

#include "SerializationTypes.h"
#include "Serialization/ComboStrings.h"

class MemoryBlock;


class EnumDescriptor;
template<class Enum> const EnumDescriptor& getEnumDescriptor(const Enum& key);

template<class BaseType> class FactoryArg0;
template<class BaseType, class FactoryArg> class SerializationFactory;

template<class BaseType>
struct FactorySelector
{
	typedef SerializationFactory<BaseType, FactoryArg0<BaseType> > Factory;
};

namespace serialization_helpers{

template<bool C, class T1, class T2>
struct Selector{};

template<class T1, class T2>
struct Selector<false, T1, T2>{
	typedef T2 type;
};

template<class T1, class T2>
struct Selector<true, T1, T2>{
	typedef T1 type;
};

template<class C, class T1, class T2>
struct Select{
	typedef typename Selector<C::value, T1,T2>::type selected_type;
	typedef typename selected_type::type type;
};

template<class T>
struct Identity{
	typedef T type;
};

template<class T>
struct IsClass{
private:
	struct NoType { char dummy; };
	struct YesType { char dummy[100]; };

	template<class U>
	static YesType function_helper(void(U::*)());

	template<class U>
	static NoType function_helper(...);
public:
	enum{ value = (sizeof(function_helper<T>(0)) == sizeof(YesType))};
};

template<class T>
class IsPolymorphic {
	struct D : T { virtual ~D() {} };
public:
	enum{ value = sizeof(T) == sizeof(D) };
};
};

namespace std {
template<class T, class A> class list;
template<class T, class A> class vector;
template<class T1, class T2> struct pair;
}

template<class Pair>
struct PairSerializationTraits
{
	static const char* firstName() { return "&Имя"; }
	static const char* secondName() { return "&Значение"; }
};

////////////////////////////////////////////////////////////////////
//
// Базовый архив.
//
// 1. Основная функция serialize принимает константную
// ссылку на объект (которую меняет), чтобы нормально 
// работала перегрузка во всех случаях.
// 2. nameAlt == 0 - не редактировать данное поле.
// nameAlt начинается с '&' - добавлять значение этого поля к 
// родительскому.
// 3. Запрет на открытие нового блока при сериализации пользовательских
// типов производится определением функции serialize(Archive&, const char*, const char*),
// вместо стандартной serialize(Archive&). По умолчанию UDT всегда сериализуются
// с открытием блока.
// 4. Для сериализации полиморфных указателей использовать serializePolymorphic,
// ShareHandle или PolymorphicWrapper. Они записывают и воссоздают тип (требуется 
// регистрация классов (REGISTER_CLASS, когда линковщик отсекает - DECLARE_SEGMENT + FORCE_SEGMENT ).
// 5. Для сериализации неполиморфных указателей (крайне редкая задача), использовать
// serializePointer или PointerWrapper.
// 6. Для массивов - serializeArray, wrapper'а пока нет, но при необходимости  возможен.
//
////////////////////////////////////////////////////////////////////

class Archive : public ShareHandleBase {
public:
	Archive()
	: closure_(0),
	closureType_(0),
	inPlace_(false),
	mergeBlocks_(0),
	filter_(0)
	{}
	virtual ~Archive() {}
	
	virtual bool close() { return true; }
	
	virtual void setVersion(int version) {} 
	int version() const { return 0; }

    virtual bool openBlock(const char* name, const char* nameAlt) { return true; }
    virtual void closeBlock() {}

    virtual bool isText() const { return false; }
    virtual bool isOutput() const = 0;
	virtual bool isInput() const = 0;
	virtual bool isEdit() const { return false; }
	
	bool inPlace() const { return inPlace_; }

	void setFilter(int filter) { filter_ = filter; }
	bool filter(int filter) { xassert("Фильтр не установлен" && filter_); return (filter_ & filter) != 0; }

    template<class T>
	bool serialize(const T& t, const char* name, const char* nameAlt) {
		using serialization_helpers::Select;
		using serialization_helpers::Identity;
		using serialization_helpers::IsClass;
        return
        Select< IsClass<T>,
            Select< HaveAdvancedSerialization<T>,
                Identity< ProcessAdvancedNonPrimitiveImpl<T> >,
                Identity< ProcessNonPrimitiveImpl<T> >
			>,
            Identity< ProcessEnumImpl<T> >
        >::type::invoke (*this, const_cast<T&>(t), name, nameAlt);
	}

    template<class T1, class T2>
    bool serialize(const std::pair<T1, T2>& t, const char* name, const char* nameAlt) {
		typedef std::pair<T1, T2> T;
		if(openStruct(t, name, nameAlt)){
			serialize(const_cast<T1&>(t.first), "first", PairSerializationTraits<std::pair<T1, T2> >::firstName());
			serialize(const_cast<T2&>(t.second), "second", PairSerializationTraits<std::pair<T1, T2> >::secondName());
			closeStruct(name);
			return true;
		}
		else
			return isEdit();
    }

    template<class Enum>
    bool serialize(const BitVector<Enum>& t, const char* name, const char* nameAlt) {
        const EnumDescriptor& descriptor = getEnumDescriptor(Enum(0));
		return processBitVector((int&)t, descriptor, name, nameAlt);
    }

    template<class T, class A>
    bool serialize(const std::vector<T, A>& constCont, const char* name, const char* nameAlt) {

		std::vector<T, A>& cont = const_cast<std::vector<T, A>&>(constCont);
        int cont_size = (int)cont.size();
        
		if(!openContainer(&cont, cont_size, name, nameAlt, typeid(std::vector<T,A>).name(), typeid(T).name(), sizeof(T), false))
			return false;
        makeDefaultArchive<T>();

        if(isOutput() || inPlace_) {
            std::vector<T, A>::const_iterator it;
            FOR_EACH(cont, it){
                serialize(*it, "@", "@");
            }
        }
        else {
            if(cont_size != -1) {
                std::vector<T, A>::const_iterator it;
                // XXX: HINT!
                if(cont.size() != cont_size) {
                    cont.clear();
                    cont.resize(cont_size);
                }
                FOR_EACH(cont, it){
                    serialize(*it, "@", "@");
                }
            }
        }
        closeContainer(name);
		return true;
    }

    template<class T, class A>
    bool serialize(const std::list<T, A>& constCont, const char* name, const char* nameAlt) {

		xassert(!inPlace_);

		std::list<T, A>& cont = const_cast<std::list<T, A>&>(constCont);

        int cont_size = int(cont.size());
		if(!openContainer(&cont, cont_size, name, nameAlt, typeid(std::list<T,A>).name(), typeid(T).name(), sizeof(T), false))
			return false;

		makeDefaultArchive<T>();
        
		if(isOutput()) {
			std::list<T, A>::const_iterator it;
			FOR_EACH(cont, it){
				serialize(*it, "@", "@");
			}
		}
		else {
			if(cont_size != -1) {
				// XXX: HINT!
				if(cont.size() != cont_size) {
					if(!cont.empty()) 
						cont.clear();
					cont.resize(cont_size);
				}
				std::list<T, A>::const_iterator it;
				FOR_EACH(cont, it){
					serialize(*it, "@", "@");
				}
			}
		}
        closeContainer(name);
		return true;
    }

    bool serialize(const MemoryBlock& buffer, const char* name, const char* nameAlt) {
        return processBinary(const_cast<MemoryBlock&>(buffer), name, nameAlt);
    }

	enum {
		NULL_POINTER = -1
	};

    template<class Enum, class Type, Enum zeroValue, class TypeFactory>
    bool serialize(const EnumToClassSerializer<Enum, Type, zeroValue, TypeFactory>& constValue, const char* name, const char* nameAlt) {
		xassert(!inPlace());
		
		typedef EnumToClassSerializer<Enum, Type, zeroValue, TypeFactory> ValueType;
		ValueType& value = const_cast<ValueType&>(constValue);

		const EnumDescriptor& descriptor = getEnumDescriptor(value.key());
		const char* baseName = descriptor.name(zeroValue);
		const char* derivedName = descriptor.name(value.key());
		const char* derivedNameAlt = descriptor.nameAlt(value.key());

		if(isInput()){
			int openTypeIndex = openPointer((void*&)value.type_, name, nameAlt, baseName, descriptor.comboList(), descriptor.comboListAlt());
			if(openTypeIndex >= 0) {
				const char* openTypeName = descriptor.comboStrings()[openTypeIndex].c_str();
				value.setKey((Enum)descriptor.keyByName(openTypeName));

				if(value.type())
					value->serialize(*this);

				closePointer(name, openTypeName, derivedName);
				return true;
			}
			else{
				value.setKey(zeroValue);
				return false;
			}
        }
        else{
			bool need_default_archive_poly = needDefaultArchive(baseName);
			int openTypeIndex = openPointer((void*&)value.type_, name, nameAlt, baseName, derivedName, derivedNameAlt);

			if(need_default_archive_poly){
				const ComboStrings& comboList = descriptor.comboStrings();
				const ComboStrings& comboListAlt = descriptor.comboStringsAlt();
				for(int i = 0; i < comboList.size(); ++i){
					const char* name = comboList[i].c_str();
					ValueType type((Enum)descriptor.keyByName(name));
					const char* nameAlt = descriptor.nameAlt(type.key());
					if(type.type()){
						if(ShareHandle<Archive> archive = openDefaultArchive(baseName, name, nameAlt)){
							archive->serialize(*type, "name", "nameAlt");
							closeDefaultArchive(archive, baseName, name, nameAlt);
						}
						type.setKey(zeroValue); // delete type;
					}
				}
			}

			if(value.type())
				value->serialize(*this);
			
			closePointer(name, baseName, derivedName);
			return true;
        }
    }

    template<class T> // Для неполиморфных указателей
    bool serializePointer(const T*& t, const char* name, const char* nameAlt) {
		xassert(!inPlace_);
        if(isInput()) {
            if(!t)
				const_cast<T*&>(t) = FactorySelector<T>::Factory::instance().createArg<T>(); // FIXME: Создается ненужная фабрика
      		serialize(*t, name, nameAlt);
			return true;
        }
        else{
      		if(t)
      			serialize(*t, name, nameAlt);
      		else{
      			static T* defaultPointer = FactorySelector<T>::Factory::instance().createArg<T>();
      			serialize(*defaultPointer, name, nameAlt);
      			xassert("Attempt to save non-polymorphic zero pointer");
      		}
			return true;
        }
    }

    template<class T> // Для полиморфных указателей
	bool serializePolymorphic(const T*& t, const char* name, const char* nameAlt) {
		T*& ptr = const_cast<T*&>(t);

		typedef FactorySelector<T>::Factory Factory;
		Factory& factory = Factory::instance();

		const char* baseName = typeid(T).name();
		const char* derivedName = 0;
		const char* derivedNameAlt = 0;
		if(ptr) {
			derivedName = typeid(*ptr).name();
			derivedNameAlt = factory.nameAlt(derivedName, true);
		}

        if(isInput()) {
			const char* comboList = factory.comboList();
			const char* comboListAlt = factory.comboListAlt();
			int openTypeIndex = openPointer((void*&)ptr, name, nameAlt, baseName, comboList, comboListAlt);
			if(openTypeIndex >= 0){
				const char* openTypeName = factory.comboStrings()[openTypeIndex].c_str();

				if(ptr && (!derivedName || strcmp(openTypeName, derivedName) != 0)){
					delete ptr;
					ptr = 0;
				}
				if(!ptr)
					ptr = factory.createByIndex(openTypeIndex);

				if(ptr)
					ptr->serialize(*this);

				closePointer(name, ptr ? openTypeName : 0, derivedName);
				return true;
			}
			else{
				if(ptr){
					delete ptr;
					ptr = 0;
				}
				return false;
			}
        }
        else {
			bool need_default_archive_poly = needDefaultArchive(typeid(T).name());
			const char* openTypeName = "";
			openPointer((void*&)ptr, name, nameAlt, baseName, derivedName, derivedNameAlt);

			if(need_default_archive_poly)
				makeDefaultArchivePoly<T>();

			if(ptr){
				if(inPlace_) 
					openStructInternal((void*)ptr, Factory::instance().find(derivedName).sizeOf(), name, nameAlt, derivedName, serialization_helpers::IsPolymorphic<T>::value);

				const_cast<T*&>(ptr)->serialize(*this);

				if(inPlace_)
					closeStruct(name);
			}

			closePointer(name, baseName, derivedName);

			return true;
        }	
    }

    template<class T>
	bool serializeArray(const T& const_array, const char* name, const char* nameAlt) {
        T& array = const_cast<T&> (const_array);

        int array_size = sizeof(array)/sizeof(array[0]);

		int count = array_size;
		if(!openContainer(&array, count, name, nameAlt, typeid(T).name(), typeid(T).name(), sizeof(array[0]), true))
			return false;

		if(isInput() && count > array_size) 
			count = array_size;

		for(int i = 0; i < count; ++i) 
			serialize(array[i], "@", "@");

        closeContainer(name);
		return true;
    }

	bool serialize(const bool& value,               const char* name, const char* nameAlt) { return processValue(const_cast<bool&>(value), name, nameAlt); }
	bool serialize(const char& value,               const char* name, const char* nameAlt) { return processValue(const_cast<char&>(value), name, nameAlt); }

	bool serialize(const signed char& value,        const char* name, const char* nameAlt) { return processValue(const_cast<signed char&>(value), name, nameAlt); }
	bool serialize(const signed short& value,       const char* name, const char* nameAlt) { return processValue(const_cast<signed short&>(value), name, nameAlt); }
	bool serialize(const signed int& value,         const char* name, const char* nameAlt) { return processValue(const_cast<signed int&>(value), name, nameAlt); }
	bool serialize(const signed long& value,        const char* name, const char* nameAlt) { return processValue(const_cast<signed long&>(value), name, nameAlt); }

	bool serialize(const unsigned char& value,      const char* name, const char* nameAlt) { return processValue(const_cast<unsigned char&>(value), name, nameAlt); }
	bool serialize(const unsigned short& value,     const char* name, const char* nameAlt) { return processValue(const_cast<unsigned short&>(value), name, nameAlt); }
	bool serialize(const unsigned int& value,       const char* name, const char* nameAlt) { return processValue(const_cast<unsigned int&>(value), name, nameAlt); }
	bool serialize(const unsigned long& value,      const char* name, const char* nameAlt) { return processValue(const_cast<unsigned long&>(value), name, nameAlt); }

	bool serialize(const float& value,              const char* name, const char* nameAlt) { return processValue(const_cast<float&>(value), name, nameAlt); }
	bool serialize(const double& value,             const char* name, const char* nameAlt) { return processValue(const_cast<double&>(value), name, nameAlt); }

	bool serialize(const std::string& value,        const char* name, const char* nameAlt) { return processValue(const_cast<std::string&>(value), name, nameAlt); }
	bool serialize(const std::wstring& value,       const char* name, const char* nameAlt) { return processValue(const_cast<std::wstring&>(value), name, nameAlt); }
	bool serialize(const ComboListString& value,    const char* name, const char* nameAlt) { return processValue(const_cast<ComboListString&>(value), name, nameAlt); }

	template<class T>
	bool openStruct(T& t, const char* name, const char* nameAlt, const char* typeName = 0) { 
		return openStructInternal((void*)&t, sizeof(T), name, nameAlt, typeName ? typeName : typeid(T).name(), serialization_helpers::IsPolymorphic<T>::value); 
	}
	virtual void closeStruct(const char* name) = 0;

	template<class T>
	void setClosure(T& closure){
		closure_ = reinterpret_cast<void*>(&closure);
#ifdef _DEBUG
		closureType_ = typeid(T).name();
#endif
	}

	template<class T>
	T& closure() const{
		xassert(closure_);
#ifdef _DEBUG
		xassert(strcmp(closureType_, typeid(T).name())== 0);
#endif
		return *reinterpret_cast<T*>(closure_);
	}

	bool hasClosure() const{ return closure_ != 0; }

protected:
	bool inPlace_;
	int mergeBlocks_;
	int filter_;

    virtual bool processValue(bool&,               const char* name, const char* nameAlt) = 0;
    virtual bool processValue(char&,               const char* name, const char* nameAlt) = 0;

    virtual bool processValue(signed char&,        const char* name, const char* nameAlt) = 0;
    virtual bool processValue(signed short&,       const char* name, const char* nameAlt) = 0;
    virtual bool processValue(signed int&,         const char* name, const char* nameAlt) = 0;
    virtual bool processValue(signed long&,        const char* name, const char* nameAlt) = 0;

    virtual bool processValue(unsigned char&,      const char* name, const char* nameAlt) = 0;
    virtual bool processValue(unsigned short&,     const char* name, const char* nameAlt) = 0;
    virtual bool processValue(unsigned int&,       const char* name, const char* nameAlt) = 0;
    virtual bool processValue(unsigned long&,      const char* name, const char* nameAlt) = 0;

    virtual bool processValue(float&,              const char* name, const char* nameAlt) = 0;
    virtual bool processValue(double&,             const char* name, const char* nameAlt) = 0;

    virtual bool processValue(std::string&,        const char* name, const char* nameAlt) = 0;
	virtual bool processValue(std::wstring&,       const char* name, const char* nameAlt){ return false; }
    virtual bool processValue(ComboListString&,    const char* name, const char* nameAlt) = 0;

	virtual bool processEnum(int& value, const EnumDescriptor& descriptor, const char* name, const char* nameAlt) = 0;
    virtual bool processBitVector(int& flags, const EnumDescriptor& descriptor,const char* name, const char* nameAlt) = 0;

	virtual bool processBinary(MemoryBlock& buffer, const char* name, const char* nameAlt) {return false;}

	virtual bool openStructInternal(void* object, int size, const char* name, const char* nameAlt, const char* typeName, bool polymorphic) = 0;

    virtual bool openContainer(void* array, int& number, const char* name, const char* nameAlt, const char* typeName, const char* elementTypeName, int elementSize, bool readOnly) = 0;
    virtual void closeContainer(const char* name) = 0;

    virtual bool needDefaultArchive(const char* baseName) const { return false; }

    virtual int openPointer(void*& object, const char* name, const char* nameAlt, const char* baseName, const char* derivedName, const char* derivedNameAlt) = 0;
    virtual void closePointer(const char* name, const char* typeName, const char* derivedName) = 0;
	const char* typeComboList(const char* typeName) {
		return 0;
	}
    virtual Archive* openDefaultArchive(const char* typeName, const char* derivedTypeName, const char* derivedTypeNameAlt){ return 0; }
    virtual void closeDefaultArchive(ShareHandle<Archive>, const char* typeName, const char* derivedTypeName, const char* derivedTypeNameAlt) {
        xassert(0);
    }

private:
	template<class T> 
	struct DefaultValue
	{
		static T get() { 
			return T();
		}
	};
	template<class T> 
	struct DefaultValue<T*>
	{
		static T* get() { 
			return 0;
		}
	};
	template<class Enum> 
	struct DefaultValueEnum
	{
		static Enum get() { 
			return getEnumDescriptor(Enum(0)).defaultValue();
		}
	};

    template<class T>
    void makeDefaultArchive(){
		const char* typeName = typeid(T).name();
		if(ShareHandle<Archive> defaultArchive = openDefaultArchive(typeName, 0, 0)){
			T t = DefaultValue<T>::get();
			defaultArchive->serialize(t, "default", "[+]");
			closeDefaultArchive(defaultArchive, typeName, 0, 0);
		}
    };

    template<class Base>
    void makeDefaultArchivePoly() {
		typedef FactorySelector<Base>::Factory Factory;
		int count = Factory::instance().size();

        for(int i = 0; i < count; ++i) {
      		Base* t = Factory::instance().createByIndex(i);
			xassert(t != 0);
			const char* baseTypeName = typeid(Base).name();
			const char* name = typeid(*t).name();
			const char* nameAlt = Factory::instance().nameAlt(name, true);

			if(ShareHandle<Archive> archive = openDefaultArchive(baseTypeName, name, nameAlt)){
      			archive->serialize(*t, "name", "nameAlt");
      			closeDefaultArchive(archive, typeid(Base).name(), name, nameAlt);
			}
      		delete t;
        }
    }

    template<class Enum>
    struct ProcessEnumImpl {
        static bool invoke(Archive& ar, const Enum& value, const char* name, const char* nameAlt){
            const EnumDescriptor& descriptor = getEnumDescriptor (Enum (0));
            if(ar.isInput()){
				int val = int(Enum(value));
				if(ar.processEnum(val, descriptor, name, nameAlt)){
					const_cast<Enum&>(value) = Enum(val);
					return true;
				}
				return false;
            }
            else{
				if(ar.processEnum((int&)value, descriptor, name, nameAlt))
					return true;
				else{
					ErrH.Abort ("Unable to write bad/unregistered enum value!");
					return false;
				}
            }
        }
    };

    template<class T>
    struct ProcessAdvancedNonPrimitiveImpl{
        static bool invoke (Archive& ar, T& t, const char* name, const char* nameAlt){
			return t.serialize(ar, name, nameAlt);
        }
    };

    template<class T>
    struct ProcessNonPrimitiveImpl{
        static bool invoke(Archive& ar, T& t, const char* name, const char* nameAlt){
			if(ar.openStruct(t, name, nameAlt)){
				t.serialize(ar);
				ar.closeStruct(name);
				return true;
			}
			else
				return ar.isEdit();
        }
    };

	template<class T>
	struct HaveAdvancedSerialization {
	private:
		struct NoType { char dummy; };
		struct YesType { char dummy[100]; };

		static NoType function_helper(void (T::*arg)(Archive&));
		static YesType function_helper(bool (T::*arg)(Archive&, const char*, const char*));
	public:
		enum { value = (sizeof(function_helper(&T::serialize)) == sizeof(YesType)) };
	};

	void* closure_;
	const char* closureType_;

	friend class MergeBlocksAuto;
};

/// Обертка для сериализации неполиморфных указателей
template<class T>
class PointerWrapper
{
public:
	PointerWrapper(T* t = 0) : t_(t) {}
	operator T* () const { return t_; }
	T* operator->() const { return t_; }
	T& operator*() const { return *t_; }
	T* get() const { return t_; }
	bool serialize(Archive& ar, const char* name, const char* nameAlt) { return ar.serializePointer(t_, name, nameAlt); }

private:
	T* t_;
};

/// Обертка для сериализации полиморфных указателей
template<class T>
class PolymorphicWrapper
{
public:
	PolymorphicWrapper(T* t = 0) : t_(t) {}
	operator T* () const { return t_; }
	T* operator->() const { return t_; }
	T& operator*() const { return *t_; }
	T* get() const { return t_; }
	bool serialize(Archive& ar, const char* name, const char* nameAlt) {
		return ar.serializePolymorphic(t_, name, nameAlt);
	}

private:
	T* t_;
};

// Для чтения блоков памяти. Если для чтения передается не нулевой размер, то требуется совпадение.
class MemoryBlock
{
public:
	MemoryBlock(int size = 0);
	MemoryBlock(void* buffer, int size) : buffer_((char*)buffer), size_(size), makeFree_(false) {}
	MemoryBlock(XBuffer& buffer) : buffer_(buffer.buffer()), size_(buffer.tell()), makeFree_(false) {}

	MemoryBlock(const MemoryBlock& block); // Выделение и копирование памяти - затратно, но надежно
	MemoryBlock& operator=(const MemoryBlock& block);

	~MemoryBlock() { free(); }

	char* buffer() const { return buffer_; }
	int size() const { return size_; }

	void alloc(int size);
	void free();
	
private:
	char* buffer_; // Должен быть первым элементом в классе
	int size_;
	bool makeFree_;
};

class MergeBlocksAuto
{
public:
	MergeBlocksAuto(Archive& archive) : archive_(archive) { archive_.mergeBlocks_++; }
	~MergeBlocksAuto() { archive_.mergeBlocks_--; }
private:
	Archive& archive_;
};


bool saveFileSmart(const char* fname, const char* buffer, int size);

string transliterate(const char* name);

#endif //__SERIALIZATION_H_INCLUDED__
