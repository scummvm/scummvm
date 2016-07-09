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

#include "backends/networking/sdl_net/handlers/uploadfilehandler.h"
#include "backends/networking/sdl_net/handlerutils.h"
#include "backends/networking/sdl_net/uploadfileclienthandler.h"
#include "backends/fs/fs-factory.h"
#include "common/system.h"
#include "common/translation.h"

namespace Networking {

UploadFileHandler::UploadFileHandler() {}

UploadFileHandler::~UploadFileHandler() {}

void UploadFileHandler::handle(Client &client) {
	Common::String path = client.queryParameter("path");

	// check that <path> is not an absolute root
	if (path == "" || path == "/" || path == "\\") {
		HandlerUtils::setFilesManagerErrorMessageHandler(client, _("Invalid path!"));
		return;
	}

	// transform virtual path to actual file system one
	Common::String prefixToRemove = "", prefixToAdd = "";
	if (!transformPath(path, prefixToRemove, prefixToAdd, false) || path.empty()) {
		HandlerUtils::setFilesManagerErrorMessageHandler(client, _("Invalid path!"));
		return;
	}

	// check that <path> exists and is directory
	AbstractFSNode *node = g_system->getFilesystemFactory()->makeFileNodePath(path);
	if (!node->exists()) {
		HandlerUtils::setFilesManagerErrorMessageHandler(client, _("The parent directory doesn't exist!"));
		return;
	}
	if (!node->isDirectory()) {
		HandlerUtils::setFilesManagerErrorMessageHandler(client, _("Can't upload into a file!"));
		return;
	}
	
	// if all OK, set special handler
	client.setHandler(new UploadFileClientHandler(path));
}

/// public

ClientHandlerCallback UploadFileHandler::getHandler() {
	return new Common::Callback<UploadFileHandler, Client &>(this, &UploadFileHandler::handle);
}

} // End of namespace Networking
