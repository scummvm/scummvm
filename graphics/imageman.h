/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/singleton.h"
#include "common/str.h"
#include "common/list.h"
#include "common/unzip.h"

namespace Graphics {
struct Surface;
 
class ImageManager : public Common::Singleton<ImageManager> {
public:
	~ImageManager();

	/**
	 * adds an .zip archive to the pool there the ImagaManager searches
	 * for image files
	 *
	 * @param name the name of the archive
	 * @return true on success and false on failure
	 */
	bool addArchive(const Common::String &name);

	/** 
	 * registers a surface to the ImageManager.
	 * surf->free(), also delete surf, will be called when the ImageManager will
	 * be destroyed or if ImageManager::unregisterSurface is called.
	 * if the parameter 'surf' is 0 the Manger will try to load an image with
	 * the filename 'name'
	 *
	 * @param name the name of the new handle
	 * @param surf the surface which should be associated to the given name
	 * @return returns true on success and false on failure
	 */
	bool registerSurface(const Common::String &name, Surface *surf);

	/**
	 * unregisters a surface, after this the returned surface from
	 * getSurface should NOT be used anymore
	 *
	 * @param name the handle
	 * @return true on success, false on failure
	 */
	bool unregisterSurface(const Common::String &name);

	/**
	 * gets a surface registered to a handle
	 *
	 * @param name the name of the handle
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
#ifdef USE_ZLIB
	typedef Common::List<unzFile>::iterator ZipIterator;
#endif
	
	Iterator searchHandle(const Common::String &name);

	Common::List<Entry*> _surfaces;
#ifdef USE_ZLIB
	Common::List<unzFile> _archives;
#endif
};

} // end of namespace Graphics

/** Shortcut for accessing the font manager. */
#define ImageMan		(Graphics::ImageManager::instance())

#endif

