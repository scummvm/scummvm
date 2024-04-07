#ifndef __FUNCTOR_H_INCLUDED__
#define __FUNCTOR_H_INCLUDED__
#include "Handle.h"

template<class T>
struct ReturnType{
	typedef typename T::ReturnType type;
};
template<class _ReturnType>
struct ReturnType<_ReturnType(*)(void)>{
	typedef _ReturnType type;                                    
};
template<class _ReturnType, class _Arg1>
struct ReturnType<_ReturnType(*)(_Arg1)>{
	typedef _ReturnType type;                                    
};

template<class T>
struct ReferenceCall{
    typedef typename ::ReturnType<T>::type ReturnType;
    ReferenceCall(T& t)
	: t_(t)
    {
    }
    ReturnType operator()(){
        return t_();
    }
    template<class U1>
    ReturnType operator()(U1 u1){
        return t_(u1);
    }
private:
    T& t_;
};

template<class Func>
ReferenceCall<Func> referenceCall(Func& f){
    return ReferenceCall<Func>(f);
}

template<class ResultType, class T>
struct BindMethod{
	typedef ResultType ReturnType;
	BindMethod(T& t, ResultType (T::*method)())
	: object_(&t)
	, method_(method)
	{        
	}
	ResultType operator()() const{
		return (object_->*method_)();
	}
private:
	T* object_;
	ResultType (T::*method_)();
};

template<class ResultType, class T>
struct BindMethodConst{
	typedef ResultType ReturnType;
	BindMethodConst(const T& t, ResultType (T::*method)(void)const)
	: object_(&t)
	, method_(method)
	{        
	}
	ResultType operator()() const{
		return (object_->*method_)();
	}
private:
	const T* object_;
	ResultType (T::*method_)() const;
};

template<class ResultType, class T, class A1>
struct BindMethod1{
	typedef A1 first_argument_type;
	typedef ResultType result_type;
	typedef ResultType ReturnType;
	BindMethod1(T& t, ResultType (T::*method)(A1))
	: object_(&t)
	, method_(method)
	{        
	}
	ResultType operator()(A1 arg1) const{
		return (object_->*method_)(arg1);
	}
private:
	T* object_;
	ResultType (T::*method_)(A1);
};

template<class ResultType, class T, class A1>
struct BindMethod1Const{
	typedef A1 first_argument_type;
	typedef ResultType result_type;
	typedef ResultType ReturnType;
	BindMethod1Const(const T& t, ResultType (T::*method)(A1) const)
	: object_(&t)
	, method_(method)
	{        
	}
	ResultType operator()(A1 arg1) const{
		return (object_->*method_)(arg1);
	}
private:
	const T* object_;
	ResultType (T::*method_)(A1) const;
};

template<class ResultType, class T, class A1, class A2>
struct BindMethod2{
	typedef A1 first_argument_type;
	typedef A2 second_argument_type;
	typedef ResultType result_type;

	typedef ResultType ReturnType;
	BindMethod2(T& t, ResultType (T::*method)(A1, A2))
	: object_(&t)
	, method_(method)
	{        
	}
	ResultType operator()(A1 arg1, A2 arg2) const{
		return (object_->*method_)(arg1, arg2);
	}
private:
	T* object_;
	ResultType (T::*method_)(A1, A2);
};

template<class ResultType, class T>
BindMethod<ResultType, T> bindMethod(T& object, ResultType (T::*method)()){
    return BindMethod<ResultType, T>(object, method);
}

template<class ResultType, class T>
BindMethodConst<ResultType, T> bindMethod(const T& object, ResultType (T::*method)() const){
    return BindMethodConst<ResultType, T>(object, method);
}

template<class ResultType, class T, class A1>
BindMethod1<ResultType, T, A1> bindMethod(T& object, ResultType (T::*method)(A1)){
    return BindMethod1<ResultType, T, A1>(object, method);
}

template<class ResultType, class T, class A1>
BindMethod1Const<ResultType, T, A1> bindMethod(const T& object, ResultType (T::*method)(A1) const){
    return BindMethod1<ResultType, T, A1>(object, method);
}

template<class ResultType, class T, class A1, class A2>
BindMethod2<ResultType, T, A1, A2> bindMethod(T& object, ResultType (T::*method)(A1, A2)){
    return BindMethod2<ResultType, T, A1, A2>(object, method);
}

template<class T>
struct CallByReference{
	CallByReference(T& t)
	:t_(t){}
	void operator() (){
		t_();
	}
	T& t_;
};

template<class Func, class Arg>
struct BindArgument{
    typedef typename ::ReturnType<Func>::type ReturnType;
    BindArgument(Func func, Arg arg)
    : func_(func)
    , arg_(arg)
    {
    }
    ReturnType operator()(){
        return func_(arg_);
    }
private:
    Func func_;
    Arg arg_;
};

template<class Func, class Arg>
BindArgument<Func, Arg> bindArgument(Func func, Arg arg){
    return BindArgument<Func, Arg>(func, arg);
}

template<class _ReturnType>
class Functor0{
public:
    typedef _ReturnType ReturnType;

    Functor0(){}
    template<class Func>
	Functor0(Func func) : impl_(new FunctorImpl<Func>(func)) {}

    _ReturnType operator()(){
        xassert(impl_);
        return (*impl_)();
    }
    operator bool() const{ return impl_ != 0; }
    void clear() { impl_ = 0; }
private:
	struct FunctorImplBase : ShareHandleBase{
        virtual _ReturnType operator()() = 0;
    };

    template<class Func>
    struct FunctorImpl : FunctorImplBase{
        Func func_;
        FunctorImpl(Func func) : func_(func) {}
        _ReturnType operator()(){ return func_(); }
    };

	template<class T>
	static FunctorImplBase* makeFunctorImpl(T t){
		return new FunctorImpl<T>(t);
	}

    ShareHandle<FunctorImplBase> impl_;
};

template<>
class Functor0<void>{
public:
    Functor0(){}
    template<class Func>
	Functor0(Func func) : impl_(new FunctorImpl<Func>(func)) {}

    void operator()(){
		if(impl_)
			(*impl_)();
		else
			xassert(0);
    }
    void clear() { impl_ = 0; }
    operator bool() const{ return impl_ != 0; }
private:
	struct FunctorImplBase : ShareHandleBase{
        virtual void operator()() = 0;
    };

    template<class Func>
    struct FunctorImpl : FunctorImplBase{
        Func func_;
        FunctorImpl(Func func) : func_(func) {}
        void operator()(){ func_(); }
    };

	template<class T>
	static FunctorImplBase* makeFunctorImpl(T t){
		return new FunctorImpl<T>(t);
	}

    ShareHandle<FunctorImplBase> impl_;
};


template<class _ReturnType, class T1>
class Functor1{
public:
    typedef _ReturnType ReturnType;
    Functor1() : impl_(0) {}
    template<class Func>
	Functor1(Func func) : impl_(new FunctorImpl<Func>(func)) {}

    ReturnType operator()(T1 t1){
        xassert(impl_);
		return (*impl_)(t1);
    }
    void clear() { impl_ = 0; }
    operator bool() const{ return impl_ != 0; }
private:
	struct FunctorImplBase : ShareHandleBase{
        virtual ReturnType operator()(T1 t1) = 0;
    };

    template<class Func>
    struct FunctorImpl : FunctorImplBase{
        FunctorImpl(Func func) : func_(func) {}
        ReturnType operator()(T1 t1){ return func_(t1); }

        Func func_;
    };

	template<class T>
	static FunctorImplBase* makeFunctorImpl(T t){ return new FunctorImpl<T>(t); }

    ShareHandle<FunctorImplBase> impl_;
};

template<class _ReturnType, class T1, class T2>
class Functor2{
public:
    typedef _ReturnType ReturnType;
    Functor2() : impl_(0) {}
    template<class Func>
	Functor2(Func func) : impl_(new FunctorImpl<Func>(func)) {}
    ReturnType operator()(T1 t1, T2 t2){
        xassert(impl_);
        return (*impl_)(t1, t2);
    }
    operator bool() const{ return impl_ != 0; }
    void clear() { impl_ = 0; }
private:
	struct FunctorImplBase : ShareHandleBase{
        virtual ReturnType operator()(T1 t1, T2 t2) = 0;
    };

    template<class Func>
    struct FunctorImpl : FunctorImplBase{
        FunctorImpl(Func func) : func_(func) {}
        ReturnType operator()(T1 t1, T2 t2){ return func_(t1, t2); }

        Func func_;
    };

	template<class T>
	static FunctorImplBase* makeFunctorImpl(T t){ return new FunctorImpl<T>(t); }

    ShareHandle<FunctorImplBase> impl_;
};

typedef Functor0<void> Functor;

#endif
