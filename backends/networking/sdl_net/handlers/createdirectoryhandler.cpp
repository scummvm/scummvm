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

#include "backends/networking/sdl_net/handlers/createdirectoryhandler.h"
#include "backends/networking/sdl_net/localwebserver.h"
#include "backends/fs/fs-factory.h"
#include "common/file.h"
#include "common/translation.h"

namespace Networking {

#define INDEX_PAGE_NAME ".index.html"

CreateDirectoryHandler::CreateDirectoryHandler() {}

CreateDirectoryHandler::~CreateDirectoryHandler() {}

void CreateDirectoryHandler::handle(Client &client) {
	Common::String path = client.queryParameter("path");
	Common::String name = client.queryParameter("directory_name");
	Common::String errorMessage = "";

	// show an error message if failed to create directory	
	if (!createDirectory(path, name, errorMessage)) {
		handleErrorMessage(
			client,
			Common::String::format(
				"%s<br/><a href=\"files?path=%s\">%s</a>",
				errorMessage.c_str(),
				"%2F", //that's encoded "/"
				_("Back to the files manager")
				)
			);
		return;
	}

	Common::String response = "<html><head><title>ScummVM</title></head><body>{message}</body></html>";

	// load stylish response page from the archive
	Common::SeekableReadStream *const stream = getArchiveFile(INDEX_PAGE_NAME);
	if (stream) response = readEverythingFromStream(stream);

	replace(response, "{message}", Common::String::format(
			"%s<br/><a href=\"files?path=%s\">%s</a>",
			_("Directory created successfully!"),
			client.queryParameter("path").c_str(),
			_("Back to parent directory")
		)
	);
	LocalWebserver::setClientRedirectHandler(
		client, response,
		"/files?path=" + LocalWebserver::urlEncodeQueryParameterValue(client.queryParameter("path"))
	);
}

void CreateDirectoryHandler::handleErrorMessage(Client &client, Common::String message) {
	Common::String response = "<html><head><title>ScummVM</title></head><body>{message}</body></html>";

	// load stylish response page from the archive
	Common::SeekableReadStream *const stream = getArchiveFile(INDEX_PAGE_NAME);
	if (stream) response = readEverythingFromStream(stream);

	replace(response, "{message}", message);
	LocalWebserver::setClientGetHandler(client, response);
}

bool CreateDirectoryHandler::createDirectory(Common::String path, Common::String name, Common::String &errorMessage) {
	// check that <path> is not an absolute root
	if (path == "" || path == "/") {
		errorMessage = _("Can't create directory here!");
		return false;
	}

	// transform virtual path to actual file system one
	Common::String prefixToRemove = "", prefixToAdd = "";
	if (!transformPath(path, prefixToRemove, prefixToAdd) || path.empty()) {
		errorMessage = _("Invalid path!");
		return false;
	}

	// check that <path> exists and is directory
	AbstractFSNode *node = g_system->getFilesystemFactory()->makeFileNodePath(path);
	if (!node->exists()) {
		errorMessage = _("Parent directory doesn't exists!");
		return false;
	}
	if (!node->isDirectory()) {
		errorMessage = _("Can't create a directory within a file!");
		return false;
	}
	
	// check that <directory_name> doesn't exist or is directory
	if (path.lastChar() != '/' && path.lastChar() != '\\') path += '/';
	node = g_system->getFilesystemFactory()->makeFileNodePath(path + name);
	if (node->exists()) {
		if (!node->isDirectory()) {
			errorMessage = _("There is a file with that name in the parent directory!");
			return false;
		} else return true;
	}
	
	// create the <directory_name> in <path>
	if (!node->create(true)) {
		errorMessage = _("Failed to create the directory!");
		return false;
	}

	return true;
}

/// public

ClientHandlerCallback CreateDirectoryHandler::getHandler() {
	return new Common::Callback<CreateDirectoryHandler, Client &>(this, &CreateDirectoryHandler::handle);
}

} // End of namespace Networking
