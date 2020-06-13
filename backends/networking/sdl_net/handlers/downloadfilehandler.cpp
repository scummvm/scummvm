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

#include "backends/networking/sdl_net/handlers/downloadfilehandler.h"
#include "backends/fs/fs-factory.h"
#include "backends/networking/sdl_net/getclienthandler.h"
#include "backends/networking/sdl_net/handlerutils.h"
#include "backends/networking/sdl_net/localwebserver.h"
#include "common/translation.h"

namespace Networking {

DownloadFileHandler::DownloadFileHandler() {}

DownloadFileHandler::~DownloadFileHandler() {}

/// public

void DownloadFileHandler::handle(Client &client) {
	Common::String path = client.queryParameter("path");

	// check that <path> is not an absolute root
	if (path == "" || path == "/") {
		HandlerUtils::setFilesManagerErrorMessageHandler(client, Common::convertFromU32String(_("Invalid path!")));
		return;
	}

	// check that <path> contains no '../'
	if (HandlerUtils::hasForbiddenCombinations(path)) {
		HandlerUtils::setFilesManagerErrorMessageHandler(client, Common::convertFromU32String(_("Invalid path!")));
		return;
	}

	// transform virtual path to actual file system one
	Common::String prefixToRemove = "", prefixToAdd = "";
	if (!transformPath(path, prefixToRemove, prefixToAdd, false) || path.empty()) {
		HandlerUtils::setFilesManagerErrorMessageHandler(client, Common::convertFromU32String(_("Invalid path!")));
		return;
	}

	// check that <path> exists, is directory and isn't forbidden
	AbstractFSNode *node = g_system->getFilesystemFactory()->makeFileNodePath(path);
	if (!HandlerUtils::permittedPath(node->getPath())) {
		HandlerUtils::setFilesManagerErrorMessageHandler(client, Common::convertFromU32String(_("Invalid path!")));
		return;
	}
	if (!node->exists()) {
		HandlerUtils::setFilesManagerErrorMessageHandler(client, Common::convertFromU32String(_("The file doesn't exist!")));
		return;
	}
	if (node->isDirectory()) {
		HandlerUtils::setFilesManagerErrorMessageHandler(client, Common::convertFromU32String(_("Can't download a directory!")));
		return;
	}
	Common::SeekableReadStream *stream = node->createReadStream();
	if (stream == nullptr) {
		HandlerUtils::setFilesManagerErrorMessageHandler(client, Common::convertFromU32String(_("Failed to read the file!")));
		return;
	}

	GetClientHandler *handler = new GetClientHandler(stream);
	handler->setResponseCode(200);
	handler->setHeader("Content-Type", "application/force-download");
	handler->setHeader("Content-Disposition", "attachment; filename=\"" + node->getDisplayName() + "\"");
	handler->setHeader("Content-Transfer-Encoding", "binary");
	client.setHandler(handler);
}

} // End of namespace Networking
