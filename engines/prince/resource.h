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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PRINCE_RESOURCE_H
#define PRINCE_RESOURCE_H

#include "common/stream.h"
#include "common/archive.h"
#include "common/debug-channels.h"

namespace Prince {

namespace Resource {

	template <typename T>
	bool loadFromStream(T &resource, Common::SeekableReadStream &stream) {
		return resource.loadFromStream(stream);
	}


	template<typename T>
	bool loadResource(T *resource, const char *resourceName, bool required = true) {
		Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName);
		if (!stream) {
			if (required) 
				error("Can't load %s", resourceName);
			return false;
		}

		bool ret = loadFromStream(*resource, *stream);

		delete stream;

		return ret;
	} 

	template <typename T>
	bool loadResource(Common::Array<T> &array, const char *resourceName, bool required = true) {
		Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName);
		if (!stream) {
			if (required)
				error("Can't load %s", resourceName);
			return false;
		}

		T t;
		while (t.loadFromStream(*stream))
			array.push_back(t);

		delete stream;
		return true;
	}

	template <typename T>
	bool loadResource(Common::Array<T *> &array, const char *resourceName, bool required = true) {
		Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName);
		if (!stream) {
			if (required)
				error("Can't load %s", resourceName);
			return false;
		}

		// FIXME: This is stupid. Maybe loadFromStream should be helper method that returns initiailzed object
		while (true) {	
			T* t = new T();
			if (!t->loadFromStream(*stream)) {
				delete t;
				break;
			}
			array.push_back(t);
		}

		delete stream;
		return true;
	}
}

}

#endif

/* vim: set tabstop=4 noexpandtab: */
