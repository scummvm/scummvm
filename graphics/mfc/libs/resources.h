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

#ifndef GRAPHICS_MFC_LIBS_RESOURCES_H
#define GRAPHICS_MFC_LIBS_RESOURCES_H

#include "common/list.h"
#include "common/path.h"
#include "common/hashmap.h"
#include "common/formats/winexe.h"
#include "graphics/mfc/minwindef.h"

namespace Graphics {
namespace MFC {
namespace Libs {

struct Resource {
	Common::WinResources *_file = nullptr;
	Common::WinResourceID _type;
	Common::WinResourceID _id;
	size_t _size = 0;

	Resource() {}
	Resource(Common::WinResources *file,
		const Common::WinResourceID &type,
		Common::WinResourceID &id, size_t size) :
		_file(file), _type(type), _id(id),
		_size(size) {}
};
typedef Common::List<Resource> ResCache;

struct ResourceFile {
	Common::String _filename;
	Common::WinResources *_res = nullptr;

	~ResourceFile() {
		delete _res;
	}
};
typedef Common::List<ResourceFile> ResList;

class Resources : public ResList {
private:
	ResCache _cache;

public:
	~Resources();

	/**
	 * Adds a Windows file containing resources
	 */
	void addResources(const Common::Path &file);

	/**
	 * Removes a Windows file containing resources
	 */
	void removeResources(const Common::Path &file);

#if 0
	Common::WinResources *getCoreResources() const {
		return _resources.begin()->_value;
	}
#endif
	/**
	 * Find a resource
	 */
	HRSRC findResource(const char *lpName, const char *lpType);

	/**
	 * Return the size of a resource
	 */
	size_t resourceSize(HRSRC res) const {
		return ((Resource *)res)->_size;
	}

	/**
	 * Return a resource as a memory block
	 */
	HGLOBAL loadResource(HRSRC hResInfo);
};

} // namespace Libs
} // namespace MFC
} // namespace Graphics

#endif
