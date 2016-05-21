/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMMON_CALLBACK_H
#define COMMON_CALLBACK_H

namespace Common {

class BaseCallback {
public:
	BaseCallback() {}
	virtual ~BaseCallback() {}
	virtual void operator()(void *ptr) = 0;
};

class GlobalFunctionCallback: public BaseCallback {
	typedef void(*GlobalFunction)(void *result);
	GlobalFunction _callback;

public:
	GlobalFunctionCallback(GlobalFunction cb): _callback(cb) {}
	virtual ~GlobalFunctionCallback() {}
	virtual void operator()(void *ptr) {
		if (_callback) _callback(ptr);
	}
};

template<class T> class Callback: public BaseCallback {
	typedef void(T::*TMethod)(void *);
	T *_object;
	TMethod _method;
public:
	Callback(T *object, TMethod method): _object(object), _method(method) {}
	virtual ~Callback() {}
	void operator()(void *ptr) { (_object->*_method)(ptr); }
};

template<class T> class CallbackBridge: public BaseCallback {
	typedef void(T::*TCallbackMethod)(BaseCallback *, void *);
	T *_object;
	TCallbackMethod _method;
	BaseCallback *_outerCallback;
public:
	CallbackBridge(T *object, TCallbackMethod method, BaseCallback *outerCallback):
		_object(object), _method(method), _outerCallback(outerCallback) {}
	virtual ~CallbackBridge() {}
	void operator()(void *ptr) { (_object->*_method)(_outerCallback, ptr); }
};

} // End of namespace Common

#endif
