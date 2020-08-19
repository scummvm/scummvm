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

#ifndef BACKENDS_NETWORKING_SDL_NET_HANDLERUTILS_H
#define BACKENDS_NETWORKING_SDL_NET_HANDLERUTILS_H

#include "backends/networking/sdl_net/client.h"
#include "common/archive.h"

namespace Networking {

class HandlerUtils {
public:
	static Common::Archive *getZipArchive();
	static Common::ArchiveMemberList listArchive();
	static Common::SeekableReadStream *getArchiveFile(Common::String name);
	static Common::String readEverythingFromStream(Common::SeekableReadStream *const stream);

	static Common::String normalizePath(const Common::String &path);
	static bool hasForbiddenCombinations(const Common::String &path);
	static bool isBlacklisted(const Common::String &path);
	static bool hasPermittedPrefix(const Common::String &path);
	static bool permittedPath(const Common::String path);

	static void setMessageHandler(Client &client, Common::String message, Common::String redirectTo = "");
	static void setFilesManagerErrorMessageHandler(Client &client, Common::String message, Common::String redirectTo = "");
};

} // End of namespace Networking

#endif
