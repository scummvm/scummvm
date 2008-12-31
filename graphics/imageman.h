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
 * $URL$
 * $Id$
 */

#ifndef GRAPHICS_IMAGEMAN_H
#define GRAPHICS_IMAGEMAN_H

#include "common/scummsys.h"

#include "common/archive.h"
#include "common/singleton.h"
#include "common/str.h"
#include "common/list.h"

namespace Graphics {

struct Surface;

class ImageManager : public Common::Singleton<ImageManager> {
public:
	~ImageManager();

	/**
	 * Adds a directory or .zip archive to the list of places where
	 * the ImageManager searches for image files.
	 *
	 * @param name an FSNode pointing to the archive
	 * @return true on success and false on failure
	 */
	bool addArchive(const Common::String &name);

	/**
	 * Removes a directory or .zip archive from the the list of places where
	 * the ImageManager searches for image files.
	 *
	 * @param name the name of the archive
	 */
	void removeArchive(const Common::String &name);

	/**
	 * Registers a surface with the ImageManager.
	 * The ImageManager takes over ownership of the surface, in particular
	 * when the surface gets unregistered, Surface::free() is invoked and
	 * the surface gets deleted by the ImageManager.
	 * If the parameter 'surf' is 0 the ImageManager tries to load an image with
	 * the filename 'name'.
	 *
	 * @param name the name of the new handle
	 * @param surf the surface which should be associated to the given name
	 * @return returns true on success and false on failure
	 */
	bool registerSurface(const Common::String &name, Surface *surf);

	/**
	 * Unregisters and delete a surface which was previously registered
	 * with the ImageManager. After unregistering a surface, it must NOT
	 * be used anymore.
	 *
	 * @param name the handle
	 * @return true on success, false on failure
	 */
	bool unregisterSurface(const Common::String &name);

	/**
	 * Gets a surface registered to a certain name.
	 *
	 * @param name the name of the surface
	 * @return returns an pointer to an Surface object or 0 on failure
	 */
	Surface *getSurface(const Common::String &name);
private:
	friend class Common::Singleton<SingletonBaseType>;
	ImageManager();

	struct Entry {
		Common::String name;
		Surface *surface;
	};
	typedef Common::List<Entry*>::iterator Iterator;

	Iterator searchHandle(const Common::String &name);

	Common::List<Entry*> _surfaces;
	Common::SearchSet _archives;
};

} // end of namespace Graphics

/** Shortcut for accessing the image manager. */
#define ImageMan		(Graphics::ImageManager::instance())

#endif

