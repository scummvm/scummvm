/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef COMMON_SINGLETON_H
#define COMMON_SINGLETON_H

namespace Common {

/**
 * Generic template base class for implementing the singleton design pattern.
 */
template <class T>
class Singleton
{
private:
	Singleton<T>(const Singleton<T>&);
	Singleton<T>& operator= (const Singleton<T>&);
	
	static T* _singleton;

public:
	static T& instance() {
		// TODO: We aren't thread safe. For now we ignore it since the
		// only thing using this singleton template is the config manager,
		// and that is first instantiated long before any threads.
		// TODO: We don't leak, but the destruction order is nevertheless
		// semi-random. If we use multiple singletons, the destruction
		// order might become an issue. There are various approaches
		// to solve that problem, but for now this is sufficient
		if (!_singleton)
			_singleton = new T;
		return *_singleton;
	}
protected:
	Singleton<T>()		{ }
	~Singleton<T>()		{ }
}; 

template <class T>
T* Singleton<T>::_singleton=0;

}	// End of namespace Common

#endif
