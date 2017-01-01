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

#include "backends/networking/sdl_net/handlers/resourcehandler.h"
#include "backends/networking/sdl_net/handlerutils.h"
#include "backends/networking/sdl_net/localwebserver.h"

namespace Networking {

ResourceHandler::ResourceHandler() {}

ResourceHandler::~ResourceHandler() {}

const char *ResourceHandler::determineMimeType(Common::String &filename) {
	// text
	if (filename.hasSuffix(".html")) return "text/html";
	if (filename.hasSuffix(".css")) return "text/css";
	if (filename.hasSuffix(".txt")) return "text/plain";
	if (filename.hasSuffix(".js")) return "application/javascript";

	// images
	if (filename.hasSuffix(".jpeg") || filename.hasSuffix(".jpg") || filename.hasSuffix(".jpe")) return "image/jpeg";
	if (filename.hasSuffix(".gif")) return "image/gif";
	if (filename.hasSuffix(".png")) return "image/png";
	if (filename.hasSuffix(".svg")) return "image/svg+xml";
	if (filename.hasSuffix(".tiff")) return "image/tiff";
	if (filename.hasSuffix(".ico")) return "image/vnd.microsoft.icon";
	if (filename.hasSuffix(".wbmp")) return "image/vnd.wap.wbmp";

	if (filename.hasSuffix(".zip")) return "application/zip";
	return "application/octet-stream";
}

/// public

void ResourceHandler::handle(Client &client) {
	Common::String filename = client.path();
	filename.deleteChar(0);

	// if archive hidden file is requested, ignore
	if (filename.size() && filename[0] == '.')
		return;

	// if file not found, don't set handler either
	Common::SeekableReadStream *file = HandlerUtils::getArchiveFile(filename);
	if (file == nullptr)
		return;

	LocalWebserver::setClientGetHandler(client, file, 200, determineMimeType(filename));
}

bool ResourceHandler::minimalModeSupported() {
	return true;
}

} // End of namespace Networking
