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

#ifndef BACKENDS_NETWORKING_SDL_NET_LISTAJAXHANDLER_H
#define BACKENDS_NETWORKING_SDL_NET_LISTAJAXHANDLER_H

#include "backends/networking/sdl_net/handlers/filesbasehandler.h"
#include "common/json.h"

namespace Networking {

class ListAjaxHandler: public FilesBaseHandler {
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
	 * Returns JSON with either listed directory or error response.
	 */
	Common::JSONObject listDirectory(Common::String path);

	/** Helper method for detecting items' type. */
	static ItemType detectType(bool isDirectory, const Common::String &name);

	/** Helper method for adding items into the files list. */
	static void addItem(Common::JSONArray &responseItemsList, ItemType itemType, Common::String path, Common::String name, Common::String size = "");

public:
	ListAjaxHandler();
	virtual ~ListAjaxHandler();

	virtual void handle(Client &client);
};

} // End of namespace Networking

#endif
