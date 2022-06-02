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

#ifndef COMMON_SINGLETON_H
#define COMMON_SINGLETON_H

#include "common/noncopyable.h"
#include "common/ptr.h"

namespace Common {

/**
 * @defgroup common_singleton Singleton
 * @ingroup common
 *
 * @brief API for managing singletons.
 *
 * @{
 */

/**
 * Generic template base class for implementing the singleton design pattern.
 */
template<class T>
class Singleton : NonCopyable {
public:
	static T &instance() {
		return *internalInstance();
	}

	static void destroy() {
		internalInstance().reset();
	}
protected:
	Singleton<T>()		{ }
	virtual ~Singleton<T>()	{ }

	typedef T SingletonBaseType;

private:
	struct Deleter {
		inline void operator()(T *object) {
			delete object;
		}
	};

	static DisposablePtr<T, Deleter> &internalInstance() {
		// TODO: We don't leak, but the destruction order is nevertheless
		// semi-random. If we use multiple singletons, the destruction
		// order might become an issue. There are various approaches
		// to solve that problem, but for now this is sufficient
		static DisposablePtr<T, Deleter> _singleton(new T(), DisposeAfterUse::NO);
		// In case destroy was called, instantiate again. This is no thread-safe.
		if (!_singleton)
			_singleton.reset(new T(), DisposeAfterUse::NO);
		return _singleton;
	}
};

template<class T>
class OptionalSingleton : NonCopyable {
public:
	static bool hasInstance() {
		return _singleton != nullptr;
	}

	static T &instance() {
		// TODO: We aren't thread safe. For now we ignore it since the
		// only thing using this singleton template is the config manager,
		// and that is first instantiated long before any threads.
		// TODO: We don't leak, but the destruction order is nevertheless
		// semi-random. If we use multiple singletons, the destruction
		// order might become an issue. There are various approaches
		// to solve that problem, but for now this is sufficient
		if (!_singleton)
			_singleton = new T();
		return *_singleton;
	}

	static void destroy() {
		delete _singleton;
		_singleton = nullptr;
	}
protected:
	OptionalSingleton<T>()		{ }
	virtual ~OptionalSingleton<T>()	{ }

	typedef T SingletonBaseType;
	static T *_singleton;
};

#ifdef _MSC_VER
#define DECLARE_SINGLETON(T)
#else
#define DECLARE_SINGLETON(T) \
	template<> T *::Common::OptionalSingleton<T>::_singleton
#endif

#define DEFINE_SINGLETON(T) \
	template<> T *::Common::OptionalSingleton<T>::_singleton = nullptr

/** @} */

} // End of namespace Common

#endif
