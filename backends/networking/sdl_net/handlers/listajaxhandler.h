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

#ifndef BACKENDS_NETWORKING_SDL_NET_LISTAJAXHANDLER_H
#define BACKENDS_NETWORKING_SDL_NET_LISTAJAXHANDLER_H

#include "backends/networking/sdl_net/handlers/filesbasehandler.h"
#include "common/formats/json.h"

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
	Common::JSONObject listDirectory(const Common::String &path);

	/** Helper method for detecting items' type. */
	static ItemType detectType(bool isDirectory, const Common::String &name);

	/** Helper method for adding items into the files list. */
	static void addItem(Common::JSONArray &responseItemsList, ItemType itemType, const Common::String &path, const Common::String &name, const Common::String &size = "");

public:
	ListAjaxHandler();
	~ListAjaxHandler() override;

	void handle(Client &client) override;
};

} // End of namespace Networking

#endif
