#ifndef __SERIALIZEABLE_H_INCLUDED__
#define __SERIALIZEABLE_H_INCLUDED__

#include "Handle.h"

class Archive;

class SerializerImplBase : public ShareHandleBase{
public:
	SerializerImplBase(const char* name, const char* nameAlt, const char* typeName, int filter = 0)
	: name_(name), nameAlt_(nameAlt), typeName_(typeName), filter_(filter)
	{
	}
	virtual ~SerializerImplBase() {}
	virtual bool serialize(Archive&, const char* name, const char* nameAlt) = 0;
	virtual void* getPointer() const = 0;

	const char* name() const{ return name_; }
	const char* nameAlt() const{ return nameAlt_; }
	const char* typeName() const{ return typeName_; }
protected:
	const char* name_;
	const char* nameAlt_;
	const char* typeName_;
	int filter_;
};

template<class T>
struct SerializerImpl : SerializerImplBase {
	SerializerImpl(const T& _data, const char* name, const char* nameAlt, int filter = 0)
	: SerializerImplBase(name, nameAlt, typeid(T).name(), filter)
	, data_(const_cast<T&>(_data))
	{
	}
	virtual ~SerializerImpl () {}
	bool serialize(Archive& ar, const char* name, const char* nameAlt){
		ar.setFilter(filter_);
		return ar.serialize(data_, name, nameAlt);
	}
	void* getPointer() const{
		return reinterpret_cast<void*>(&data_);
	}
	T& data_;
};

class Serializer{
public:
    Serializer()
	: impl_(0)
	{}

	template<class T>
    explicit Serializer(const T& _data, const char* name = "", const char* nameAlt = "", int filter = 0){
        set(_data, name, nameAlt, filter);
    }

	void setImpl(SerializerImplBase* impl){
		impl_ = impl;
	}

    ~Serializer(){}

    template<class T>
    Serializer& set(const T& _data, const char* name, const char* nameAlt, int filter = 0){
        impl_ = new SerializerImpl<T>(const_cast<T&>(_data), name, nameAlt, filter);
        return *this;
    }

    template<class T>
    Serializer& set(const T& _data){
        impl_ = new SerializerImpl<T>(const_cast<T&>(_data), "", "");
        return *this;
    }

    operator bool() const{ return impl_ != 0; }

	bool serialize(Archive& ar){
        if(impl_)
            return impl_->serialize(ar, impl_->name(), impl_->nameAlt());
		return false;
    }

	bool serialize(Archive& ar, const char* name, const char* nameAlt){
		if(impl_)
           return impl_->serialize(ar, name, nameAlt);
		else
			return false;
    }

    const char* name() const{ return impl_ ? impl_->name() : 0; }
    const char* nameAlt() const{ return impl_ ? impl_->nameAlt() : 0; }
	const char* typeName() const { return impl_ ? impl_->typeName() : ""; }

    void* getPointer() const{
        return impl_ ? impl_->getPointer() : 0;
    }

    void release(){
        impl_ = 0;
    }
protected:

    ShareHandle<SerializerImplBase> impl_;
};

typedef std::vector<Serializer> Serializers;

#endif
