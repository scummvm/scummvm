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

#ifndef BACKENDS_NETWORKING_SDL_NET_FILESPAGEHANDLER_H
#define BACKENDS_NETWORKING_SDL_NET_FILESPAGEHANDLER_H

#include "backends/networking/sdl_net/handlers/filesbasehandler.h"

namespace Networking {

class FilesPageHandler: public FilesBaseHandler {
	enum ItemType {
		IT_DIRECTORY,
		IT_PARENT_DIRECTORY,
		IT_TXT,
		IT_ZIP,
		IT_7Z,
		IT_UNKNOWN
	};

	/**
	 * Lists the directory <path>.
	 *
	 * Returns true on success.
	 */
	bool listDirectory(Common::String path, Common::String &content, const Common::String &itemTemplate);

	/** Helper method for detecting items' type. */
	static ItemType detectType(bool isDirectory, const Common::String &name);

	/** Helper method for adding items into the files list. */
	void addItem(Common::String &content, const Common::String &itemTemplate, ItemType itemType, Common::String path, Common::String name, Common::String size = "") const;

public:
	FilesPageHandler();
	virtual ~FilesPageHandler();

	virtual void handle(Client &client);
};

} // End of namespace Networking

#endif
