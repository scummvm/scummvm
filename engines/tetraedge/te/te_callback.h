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

#ifndef TETRAEDGE_TE_TE_CALLBACK_H
#define TETRAEDGE_TE_TE_CALLBACK_H

//#include "common/callback.h"

namespace Tetraedge {

class TeICallback0Param {
public:
	virtual ~TeICallback0Param() {}
	virtual bool operator()() = 0;
	virtual bool call() = 0;
	virtual float priority() const = 0;
	virtual bool equals(const TeICallback0Param *other) const = 0;
};

/**
 * TeCallback is similar to Common::Callback, but it takes 0 parameters,
 * supports priority, and the function returns bool.
 */
template<class T> class TeCallback0Param : public TeICallback0Param {
public:
	typedef bool(T::*TMethod)();
protected:
	T *_object;
	TMethod _method;
	float _priority;
public:
	TeCallback0Param(T *object, TMethod method, float priority_ = 0.0f): _object(object), _method(method), _priority(priority_) {}
	virtual ~TeCallback0Param() {}
	bool operator()() override { return (_object->*_method)(); }
	bool call() override { return (_object->*_method)(); }

	virtual float priority() const override { return _priority; }

	bool equals(const TeICallback0Param *other) const override {
		const TeCallback0Param<T> *o = dynamic_cast<const TeCallback0Param<T> *>(other);
		return o && _object == o->_object && _method == o->_method;
	}
	//virtual void setPriority()
};

template<class T> class TeICallback1Param {
public:
	virtual ~TeICallback1Param() {}
	virtual bool operator()(T data) = 0;
	virtual bool call(T data) = 0;
	virtual float &priority() = 0;
	virtual bool equals(const TeICallback1Param *other) const = 0;
};


template<class T, typename S> class TeCallback1Param : public TeICallback1Param<S> {
public:
	typedef bool(T::*TMethod)(S);
protected:
	T *_object;
	TMethod _method;
	float _priority;
public:
	TeCallback1Param(T *object, TMethod method, float priority_ = 0.0f): _object(object), _method(method), _priority(priority_) {}
	virtual ~TeCallback1Param() {}
	bool operator()(S data) override { return (_object->*_method)(data); }
	bool call(S data) override { return (_object->*_method)(data); }

	virtual float &priority() override { return _priority; }

	bool equals(const TeICallback1Param<S> *other) const override {
		const TeCallback1Param<T, S> *o = dynamic_cast<const TeCallback1Param<T, S> *>(other);
		return o && _object == o->_object && _method == o->_method;
	}
	//virtual void setPriority()
};


template<class S, class T> class TeICallback2Param {
public:
	virtual ~TeICallback2Param() {}
	virtual bool operator()(S data1, T data2) = 0;
	virtual bool call(S data1, T data2) = 0;
	virtual float &priority() = 0;
	virtual bool equals(const TeICallback2Param *other) const = 0;
};


template<class C, class S, typename T> class TeCallback2Param : public TeICallback2Param<S, T> {
public:
	typedef bool(C::*TMethod)(S, T);
protected:
	C *_object;
	TMethod _method;
	float _priority;
public:
	TeCallback2Param(C *object, TMethod method, float priority_ = 0.0f): _object(object), _method(method), _priority(priority_) {}
	virtual ~TeCallback2Param() {}
	bool operator()(S data1, T data2) override { return (_object->*_method)(data1, data2); }
	bool call(S data1, T data2) override { return (_object->*_method)(data1, data2); }

	virtual float &priority() override { return _priority; }

	bool equals(const TeICallback2Param<S, T> *other) const override {
		const TeCallback2Param<C, S, T> *o = dynamic_cast<const TeCallback2Param<C, S, T> *>(other);
		return o && _object == o->_object && _method == o->_method;
	}
	//virtual void setPriority()
};



} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_CALLBACK_H
