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
#include "backends/networking/sdl_net/localwebserver.h"
#include "backends/fs/fs-factory.h"
#include "common/file.h"
#include "common/translation.h"

namespace Networking {

#define INDEX_PAGE_NAME ".index.html"

UploadFileHandler::UploadFileHandler() {}

UploadFileHandler::~UploadFileHandler() {}

void UploadFileHandler::handle(Client &client) {
	Common::String errorMessage = "";

	// show an error message if failed to upload the file
	if (!uploadFile(client, errorMessage)) {
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
			_("Uploaded successfully!"),
			client.queryParameter("path").c_str(),
			_("Back to parent directory")
		)
	);
	LocalWebserver::setClientRedirectHandler(
		client, response,
		"/files?path=" + LocalWebserver::urlEncodeQueryParameterValue(client.queryParameter("path"))
	);
}

void UploadFileHandler::handleErrorMessage(Client &client, Common::String message) {
	Common::String response = "<html><head><title>ScummVM</title></head><body>{message}</body></html>";

	// load stylish response page from the archive
	Common::SeekableReadStream *const stream = getArchiveFile(INDEX_PAGE_NAME);
	if (stream) response = readEverythingFromStream(stream);

	replace(response, "{message}", message);
	LocalWebserver::setClientGetHandler(client, response);
}

namespace {
bool copyStream(Common::ReadStream *from, Common::WriteStream *into) {
	assert(from);
	assert(into);
	const uint32 BUFFER_SIZE = 1 * 1024 * 1024;
	void *buffer = malloc(BUFFER_SIZE);
	bool success = true;
	assert(buffer);

	while (!from->eos()) {
		uint32 readBytes = from->read(buffer, BUFFER_SIZE);

		if (from->err()) {
			warning("copyStream: failed to read bytes from the stream");
			success = false;
			break;
		}

		if (readBytes != 0)
			if (into->write(buffer, readBytes) != readBytes || into->err()) {
				warning("copyStream: failed to write all bytes into the file");
				success = false;
				break;
			}
	}

	free(buffer);
	return success;
}
}

bool UploadFileHandler::uploadFile(Client &client, Common::String &errorMessage) {
	Common::String path = client.queryParameter("path");
	Common::String originalFilename = client.queryParameter("upload_file");
	Common::String tempFilename = client.attachedFile(originalFilename);
	debug("path = <%s>", path.c_str());
	debug("filename = <%s>", originalFilename.c_str());
	debug("tempfile = <%s>", tempFilename.c_str());

	// check that <path> is not an absolute root
	if (path == "" || path == "/" || path == "\\") {
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
		errorMessage = _("The parent directory doesn't exist!");
		return false;
	}
	if (!node->isDirectory()) {
		errorMessage = _("Can't upload into a file!");
		return false;
	}

	// check that <path>/<originalFilename> doesn't exist
	if (path.lastChar() != '/' && path.lastChar() != '\\') path += '/';
	AbstractFSNode *originalNode = g_system->getFilesystemFactory()->makeFileNodePath(path + originalFilename);
	if (originalNode->exists()) {
		errorMessage = _("There is a file with that name in the parent directory!");
		return false;
	}

	// check that <tempFilename> exists
	AbstractFSNode *tempNode = g_system->getFilesystemFactory()->makeFileNodePath(tempFilename);
	if (!tempNode->exists() || tempNode->isDirectory()) {
		errorMessage = _("Failed to upload the file!");
		return false;
	}

	// copy <tempFilename> into <path>/<originalFilename>
	// FIXME: I think we should move/rename file with some system call
	// even though that might be less portable, that is much better than
	// making an actual copy of data (because user might had enough place
	// for one copy of the file, but not for two of them)
	Common::ReadStream *tempStream = tempNode->createReadStream();
	Common::WriteStream *fileStream = originalNode->createWriteStream();
	if (tempStream == nullptr || fileStream == nullptr || !copyStream(tempStream, fileStream)) {
		delete tempStream;
		delete fileStream;
		errorMessage = _("Failed to upload the file!");
		return false;
	}

	delete tempStream;
	delete fileStream;
	return true;
}

/// public

ClientHandlerCallback UploadFileHandler::getHandler() {
	return new Common::Callback<UploadFileHandler, Client &>(this, &UploadFileHandler::handle);
}

} // End of namespace Networking
