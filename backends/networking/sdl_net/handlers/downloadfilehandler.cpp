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
#include "backends/networking/sdl_net/localwebserver.h"
#include "backends/fs/fs-factory.h"
#include "common/file.h"
#include "common/translation.h"
#include "../getclienthandler.h"

namespace Networking {

#define INDEX_PAGE_NAME ".index.html"

DownloadFileHandler::DownloadFileHandler() {}

DownloadFileHandler::~DownloadFileHandler() {}

void DownloadFileHandler::handle(Client &client) {
	Common::String path = client.queryParameter("path");
	Common::String errorMessage = "";

	// show an error message if failed to download the file
	if (!downloadFile(client, path, errorMessage)) {
		handleErrorMessage(
			client,
			Common::String::format(
				"%s<br/><a href=\"files?path=/\">%s</a>",
				errorMessage.c_str(),
				_("Back to the files manager")
				)
			);
	}
}

void DownloadFileHandler::handleErrorMessage(Client &client, Common::String message) {
	Common::String response = "<html><head><title>ScummVM</title></head><body>{message}</body></html>";

	// load stylish response page from the archive
	Common::SeekableReadStream *const stream = getArchiveFile(INDEX_PAGE_NAME);
	if (stream) response = readEverythingFromStream(stream);

	replace(response, "{message}", message);
	LocalWebserver::setClientGetHandler(client, response);
}

bool DownloadFileHandler::downloadFile(Client &client, Common::String path, Common::String &errorMessage) {
	// check that <path> is not an absolute root
	if (path == "" || path == "/") {
		errorMessage = _("Invalid path!");
		return false;
	}

	// transform virtual path to actual file system one
	Common::String prefixToRemove = "", prefixToAdd = "";
	if (!transformPath(path, prefixToRemove, prefixToAdd, false) || path.empty()) {
		errorMessage = _("Invalid path!");
		return false;
	}

	// check that <path> exists and is directory
	AbstractFSNode *node = g_system->getFilesystemFactory()->makeFileNodePath(path);
	if (!node->exists()) {
		errorMessage = _("The file doesn't exist!");
		return false;
	}
	if (node->isDirectory()) {
		errorMessage = _("Can't download a directory!");
		return false;
	}
	Common::SeekableReadStream *stream = node->createReadStream();
	if (stream == nullptr) {
		errorMessage = _("Failed to read the file!");
		return false;
	}

	GetClientHandler *handler = new GetClientHandler(stream);
	handler->setResponseCode(200);
	handler->setHeader("Content-Type", "application/force-download");	
	handler->setHeader("Content-Disposition", "attachment; filename=\"" + node->getDisplayName() + "\"");
	handler->setHeader("Content-Transfer-Encoding", "binary");	
	client.setHandler(handler);
	return true;
}

/// public

ClientHandlerCallback DownloadFileHandler::getHandler() {
	return new Common::Callback<DownloadFileHandler, Client &>(this, &DownloadFileHandler::handle);
}

} // End of namespace Networking
