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

#include "backends/networking/sdl_net/handlers/filespagehandler.h"
#include "backends/networking/sdl_net/localwebserver.h"
#include "backends/fs/fs-factory.h"
#include "common/file.h"
#include "common/translation.h"

namespace Networking {

#define INDEX_PAGE_NAME ".index.html"
#define FILES_PAGE_NAME ".files.html"

FilesPageHandler::FilesPageHandler() {}

FilesPageHandler::~FilesPageHandler() {}

void FilesPageHandler::handle(Client &client) {
	if (client.path() == "/files") handleFiles(client);
	else handleCreateDirectory(client);
}

void FilesPageHandler::handleFiles(Client &client) {
	Common::String response = "<html><head><title>ScummVM</title></head><body><table>{content}</table></body></html>"; //TODO: add controls
	Common::String itemTemplate = "<tr><td><a href=\"{link}\">{name}</a></td><td>{size}</td></tr>\n"; //TODO: load this template too?

	// load stylish response page from the archive
	Common::SeekableReadStream *const stream = getArchiveFile(FILES_PAGE_NAME);
	if (stream) response = readEverythingFromStream(stream);

	Common::String path = client.queryParameter("path");
	Common::String content = "";
	
	// show an error message if failed to list directory
	if (!listDirectory(path, content, itemTemplate)) {
		handleErrorMessage(
			client,
			Common::String::format(
				"%s<br/><a href=\"files?path=/\">%s</a>",
				_("ScummVM couldn't list the directory you specified."),
				_("Back to the files manager")
			)
		);
		return;
	}

	//these occur twice:
	replace(response, "{create_directory_button}", _("Create directory"));
	replace(response, "{create_directory_button}", _("Create directory"));
	replace(response, "{path}", client.queryParameter("path"));
	replace(response, "{path}", client.queryParameter("path"));
	replace(response, "{upload_files_button}", _("Upload files")); //tab
	replace(response, "{upload_file_button}", _("Upload files")); //button in the tab
	replace(response, "{create_directory_desc}", _("Type new directory name:"));
	replace(response, "{upload_file_desc}", _("Select a file to upload:"));
	replace(response, "{content}", content);	
	LocalWebserver::setClientGetHandler(client, response);
}

void FilesPageHandler::handleCreateDirectory(Client &client) {
	Common::String path = client.queryParameter("path");
	Common::String name = client.queryParameter("directory_name");
	Common::String errorMessage = "";

	// show an error message if failed to create directory	
	if (!createDirectory(path, name, errorMessage)) {
		handleErrorMessage(
			client,
			Common::String::format(
				"%s<br/><a href=\"files?path=/\">%s</a>",
				errorMessage.c_str(),
				_("Back to the files manager")
				)
			);
		return;
	}

	handleFiles(client);
}

void FilesPageHandler::handleErrorMessage(Client &client, Common::String message) {
	Common::String response = "<html><head><title>ScummVM</title></head><body>{message}</body></html>";

	// load stylish response page from the archive
	Common::SeekableReadStream *const stream = getArchiveFile(INDEX_PAGE_NAME);
	if (stream) response = readEverythingFromStream(stream);

	replace(response, "{message}", message);
	LocalWebserver::setClientGetHandler(client, response);
}

bool FilesPageHandler::createDirectory(Common::String path, Common::String name, Common::String &errorMessage) {
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

bool FilesPageHandler::listDirectory(Common::String path, Common::String &content, const Common::String &itemTemplate) {
	if (path == "" || path == "/") {
		addItem(content, itemTemplate, true, "/root/", _("File system root"));
		addItem(content, itemTemplate, true, "/saves/", _("Saved games"));
		return true;
	}

	Common::String prefixToRemove = "", prefixToAdd = "";
	if (!transformPath(path, prefixToRemove, prefixToAdd)) return false;

	Common::FSNode node = Common::FSNode(path);
	if (path == "/") node = node.getParent(); // absolute root
	if (!node.isDirectory()) return false;

	// list directory
	Common::FSList _nodeContent;
	if (!node.getChildren(_nodeContent, Common::FSNode::kListAll, false)) // do not show hidden files
		_nodeContent.clear();
	else
		Common::sort(_nodeContent.begin(), _nodeContent.end());

	// add parent directory link
	{
		Common::String filePath = path;
		if (filePath.hasPrefix(prefixToRemove))
			filePath.erase(0, prefixToRemove.size());
		if (filePath == "" || filePath == "/" || filePath == "\\")
			filePath = "/";
		else
			filePath = parentPath(prefixToAdd + filePath);
		addItem(content, itemTemplate, true, filePath, _("Parent directory"));
	}

	// fill the content
	for (Common::FSList::iterator i = _nodeContent.begin(); i != _nodeContent.end(); ++i) {
		Common::String name = i->getDisplayName();
		if (i->isDirectory()) name += "/";

		Common::String filePath = i->getPath();
		if (filePath.hasPrefix(prefixToRemove))
			filePath.erase(0, prefixToRemove.size());
		filePath = prefixToAdd + filePath;

		addItem(content, itemTemplate, i->isDirectory(), filePath, name);
	}

	return true;
}

void FilesPageHandler::addItem(Common::String &content, const Common::String &itemTemplate, bool isDirectory, Common::String path, Common::String name, Common::String size) {
	Common::String item = itemTemplate;
	replace(item, "{link}", (isDirectory ? "files?path=" : "download?path=") + path);
	replace(item, "{name}", name);
	replace(item, "{size}", size);
	content += item;
}

/// public

ClientHandlerCallback FilesPageHandler::getHandler() {
	return new Common::Callback<FilesPageHandler, Client &>(this, &FilesPageHandler::handle);
}

} // End of namespace Networking
