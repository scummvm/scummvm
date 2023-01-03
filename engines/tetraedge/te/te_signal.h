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

#ifndef TETRAEDGE_TE_TE_SIGNAL_H
#define TETRAEDGE_TE_TE_SIGNAL_H

#include "common/array.h"
#include "common/ptr.h"
#include "tetraedge/te/te_callback.h"

namespace Tetraedge {

template<class C> bool _teCallbackSorter(const C &c1, const C &c2) {
	// sort in *descending* priority.
	float p1 = c1->priority();
	float p2 = c2->priority();
	return p2 < p1;
}

typedef Common::SharedPtr<TeICallback0Param> TeICallback0ParamPtr;

class TeSignal0Param : public Common::Array<TeICallback0ParamPtr> {
public:
	TeSignal0Param() : Common::Array<TeICallback0ParamPtr>() {};

	bool call() {
		Common::sort(this->begin(), this->end(), _teCallbackSorter<TeICallback0ParamPtr>);
		typename Common::Array<TeICallback0ParamPtr>::iterator i = this->begin();
		typename Common::Array<TeICallback0ParamPtr>::iterator end_ = this->end();
		for (; i < end_; i++) {
			if ((*i)->call())
				return true;
		}
		return false;
	}

	void remove(const TeICallback0ParamPtr &item) {
		typename Common::Array<TeICallback0ParamPtr>::iterator i = this->begin();
		typename Common::Array<TeICallback0ParamPtr>::iterator end_ = this->end();
		for (; i < end_; i++) {
			if ((*i)->equals(item.get())) {
				i = this->erase(i);
			}
		}
	}

	template<class T> void add(T *obj, typename TeCallback0Param<T>::TMethod method) {
		this->push_back(TeICallback0ParamPtr(new TeCallback0Param<T>(obj, method)));
	}

	template<class T> void remove(T *obj, typename TeCallback0Param<T>::TMethod method) {
		TeICallback0ParamPtr temp(new TeCallback0Param<T>(obj, method));
		this->remove(temp);
	}

};

/* Pointer to a callback with a single parameter of type T */
template<class T> using TeICallback1ParamPtr = Common::SharedPtr<TeICallback1Param<T>>;

/* Array of callbacks with a single parameter of type T */
template<class T> class TeSignal1Param : public Common::Array<TeICallback1ParamPtr<T>> {
public:
	TeSignal1Param() : Common::Array<TeICallback1ParamPtr<T>>() {};

	bool call(T t) {
		Common::sort(this->begin(), this->end(), _teCallbackSorter<TeICallback1ParamPtr<T>>);
		typename Common::Array<TeICallback1ParamPtr<T>>::iterator i = this->begin();
		typename Common::Array<TeICallback1ParamPtr<T>>::iterator end_ = this->end();
		for (; i < end_; i++) {
			if ((*i)->call(t))
				return true;
		}
		return false;
	}

	void remove(const TeICallback1ParamPtr<T> &item) {
		typename Common::Array<TeICallback1ParamPtr<T>>::iterator i = this->begin();
		typename Common::Array<TeICallback1ParamPtr<T>>::iterator end_ = this->end();
		for (; i < end_; i++) {
			if ((*i)->equals(item.get())) {
				i = this->erase(i);
			}
		}
	}

	template<class S> void add(S *obj, typename TeCallback1Param<S, T>::TMethod method) {
		this->push_back(TeICallback1ParamPtr<T>(new TeCallback1Param<S, T>(obj, method)));
	}

	template<class S> void remove(S *obj, typename TeCallback1Param<S, T>::TMethod method) {
		TeICallback1ParamPtr<T> temp(new TeCallback1Param<S, T>(obj, method));
		this->remove(temp);
	}
};

/* Pointer to a callback with 2 paramets parameter of type T and S */
template<class S, class T> using TeICallback2ParamPtr = Common::SharedPtr<TeICallback2Param<S, T>>;

/* Array of callbacks with a two parameters of type T */
template<class S, class T> class TeSignal2Param : public Common::Array<TeICallback2ParamPtr<S, T>> {
public:
	TeSignal2Param() : Common::Array<TeICallback2ParamPtr<S, T>>() {};

	bool call(S s, T t) {
		Common::sort(this->begin(), this->end(), _teCallbackSorter<TeICallback2ParamPtr<S, T>>);
		typename Common::Array<TeICallback2ParamPtr<S, T>>::iterator i = this->begin();
		typename Common::Array<TeICallback2ParamPtr<S, T>>::iterator end_ = this->end();
		for (; i < end_; i++) {
			if ((*i)->call(s, t))
				return true;
		}
		return false;
	}

	void remove(const TeICallback2ParamPtr<S, T> &item) {
		typename Common::Array<TeICallback2ParamPtr<S, T>>::iterator i = this->begin();
		typename Common::Array<TeICallback2ParamPtr<S, T>>::iterator end_ = this->end();
		for (; i < end_; i++) {
			if ((*i)->equals(item.get())) {
				i = this->erase(i);
			}
		}
	}

	template<class C> void add(C *obj, typename TeCallback2Param<C, S, T>::TMethod method) {
		this->push_back(TeICallback2ParamPtr<S, T>(new TeCallback2Param<C, S, T>(obj, method)));
	}

	template<class C> void remove(C *obj, typename TeCallback2Param<C, S, T>::TMethod method) {
		TeICallback2ParamPtr<S, T> temp(new TeCallback2Param<C, S, T>(obj, method));
		this->remove(temp);
	}
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_SIGNAL_H
