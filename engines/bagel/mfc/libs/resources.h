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

#ifndef BAGEL_MFC_LIBS_RESOURCES_H
#define BAGEL_MFC_LIBS_RESOURCES_H

#include "common/list.h"
#include "common/path.h"
#include "common/stack.h"
#include "common/formats/winexe.h"
#include "bagel/mfc/minwindef.h"

namespace Bagel {
namespace MFC {
namespace Libs {

class Resources {
	struct Resource {
		Common::WinResourceID _type;
		Common::WinResourceID _id;
		size_t _size = 0;

		Resource() {}
		Resource(const Common::WinResourceID &type,
			Common::WinResourceID &id, size_t size) :
			_type(type), _id(id), _size(size) {}
	};
	typedef Common::List<Resource> ResCache;

private:
	Common::Stack<Common::WinResources *> _resources;
	ResCache _cache;

public:
	~Resources();

	/**
	 * Adds a Windows file containing resources
	 */
	void addResources(const Common::Path &file);

	/**
	 * Pops a resources file from the stack
	 */
	void popResources();

	/**
	 * Find a resource
	 */
	HRSRC findResource(LPCSTR lpName, LPCSTR lpType);

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

	/**
	 * Get the current resources
	 */
	Common::WinResources *getResources() const;
};

} // namespace Libs
} // namespace MFC
} // namespace Bagel

#endif
