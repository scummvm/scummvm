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

#include "backends/networking/sdl_net/indexpagehandler.h"
#include "backends/networking/sdl_net/localwebserver.h"
#include "backends/saves/default/default-saves.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/translation.h"
#include "common/unzip.h"
#include "gui/storagewizarddialog.h"

namespace Networking {

#define ARCHIVE_NAME "wwwroot.zip"
#define INDEX_PAGE_NAME "index.html"
#define FILES_PAGE_NAME "files.html"

IndexPageHandler::IndexPageHandler(): CommandSender(nullptr) {}

IndexPageHandler::~IndexPageHandler() {
	LocalServer.removePathHandler("/");
	LocalServer.removePathHandler("/files/");

	Common::ArchiveMemberList fileList = listArchive();
	for (Common::ArchiveMemberList::iterator it = fileList.begin(); it != fileList.end(); ++it) {
		Common::ArchiveMember const &m = **it;
		if (m.getName() == INDEX_PAGE_NAME) continue;
		LocalServer.removePathHandler("/" + m.getName());
	}
}

void IndexPageHandler::handle(Client &client) {
	Common::String response = "<html><head><title>ScummVM</title></head><body>{message}</body></html>";

	// load stylish response page from the archive
	Common::SeekableReadStream *const stream = getArchiveFile(INDEX_PAGE_NAME);
	if (stream) response = readEverythingFromStream(stream);

	Common::String code = client.queryParameter("code");

	if (code == "") {		
		replace(response, "{message}", _("This is a local webserver index page."));
		LocalWebserver::setClientGetHandler(client, response);
		return;
	}

	_code = code;
	sendCommand(GUI::kStorageCodePassedCmd, 0);
	replace(response, "{message}", _("ScummVM got the code and already connects to your cloud storage!"));
	LocalWebserver::setClientGetHandler(client, response);
}

void IndexPageHandler::handleFiles(Client &client) {
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
	replace(response, "{upload_files_button}", _("Upload files")); //tab
	replace(response, "{upload_file_button}", _("Upload files")); //button in the tab
	replace(response, "{create_directory_desc}", _("Type new directory name:"));
	replace(response, "{upload_file_desc}", _("Select a file to upload:"));
	replace(response, "{content}", content);	
	LocalWebserver::setClientGetHandler(client, response);
}

void IndexPageHandler::handleResource(Client &client) {
	Common::String filename = client.path();
	filename.deleteChar(0);
	LocalWebserver::setClientGetHandler(client, getArchiveFile(filename), 200, LocalWebserver::determineMimeType(filename));
}

void IndexPageHandler::handleErrorMessage(Client &client, Common::String message) {
	Common::String response = "<html><head><title>ScummVM</title></head><body>{message}</body></html>";

	// load stylish response page from the archive
	Common::SeekableReadStream *const stream = getArchiveFile(INDEX_PAGE_NAME);
	if (stream) response = readEverythingFromStream(stream);

	replace(response, "{message}", message);
	LocalWebserver::setClientGetHandler(client, response);
}

/// "files/"-related

Common::String IndexPageHandler::parentPath(Common::String path) {
	if (path.size() && (path.lastChar() == '/' || path.lastChar() == '\\')) path.deleteLastChar();
	if (!path.empty()) {
		for (int i = path.size() - 1; i >= 0; --i)
			if (i == 0 || path[i] == '/' || path[i] == '\\') {
				path.erase(i);
				break;
			}
	}
	if (path.size() && path.lastChar() != '/' && path.lastChar() != '\\') path += '/';
	return path;
}

void IndexPageHandler::addItem(Common::String &content, const Common::String &itemTemplate, bool isDirectory, Common::String path, Common::String name, Common::String size) {
	Common::String item = itemTemplate;
	replace(item, "{link}", (isDirectory ? "files?path=" : "download?path=") + path);
	replace(item, "{name}", name);
	replace(item, "{size}", size);
	content += item;
}

bool IndexPageHandler::listDirectory(Common::String path, Common::String &content, const Common::String &itemTemplate) {
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

bool IndexPageHandler::transformPath(Common::String &path, Common::String &prefixToRemove, Common::String &prefixToAdd) {
	// <path> is not empty, but could lack the trailing slash	
	if (path.lastChar() != '/' && path.lastChar() != '\\') path += '/';

	if (path.hasPrefix("/root")) {
		prefixToAdd = "/root/";
		prefixToRemove = "";
		path.erase(0, 5);
		if (path == "") path = "/"; // absolute root is '/'
		if (path != "/") path.deleteChar(0); // if that was "/root/ab/c", it becomes "/ab/c", but we need "ab/c"
		return true;
	}

	if (path.hasPrefix("/saves")) {
		prefixToAdd = "/saves/";

		// determine savepath (prefix to remove)
		DefaultSaveFileManager *manager = dynamic_cast<DefaultSaveFileManager *>(g_system->getSavefileManager());
		prefixToRemove = (manager ? manager->concatWithSavesPath("") : ConfMan.get("savepath"));
		if (prefixToRemove.size() && prefixToRemove.lastChar() != '/' && prefixToRemove.lastChar() != '\\')
			prefixToRemove += '/';

		path.erase(0, 6);
		if (path.size() && (path[0] == '/' || path[0] == '\\'))
			path.deleteChar(0);
		path = prefixToRemove + path;
		return true;
	}

	return false;
}

/// public

void IndexPageHandler::addPathHandler(LocalWebserver &server) {
	// we can't use LocalServer yet, because IndexPageHandler is created while LocalWebserver is created
	// (thus no _instance is available and it causes stack overflow)
	server.addPathHandler("/", new Common::Callback<IndexPageHandler, Client &>(this, &IndexPageHandler::handle));
	server.addPathHandler("/files", new Common::Callback<IndexPageHandler, Client &>(this, &IndexPageHandler::handleFiles));

	Common::ArchiveMemberList fileList = listArchive();
	for (Common::ArchiveMemberList::iterator it = fileList.begin(); it != fileList.end(); ++it) {
		Common::ArchiveMember const &m = **it;
		if (m.getName() == INDEX_PAGE_NAME) continue;
		if (m.getName() == FILES_PAGE_NAME) continue;
		server.addPathHandler("/" + m.getName(), new Common::Callback<IndexPageHandler, Client &>(this, &IndexPageHandler::handleResource));		
	}
}

Common::String IndexPageHandler::code() { return _code; }

/// utils

Common::Archive *IndexPageHandler::getZipArchive() {
	// first search in themepath
	if (ConfMan.hasKey("themepath")) {
		const Common::FSNode &node = Common::FSNode(ConfMan.get("themepath"));
		if (!node.exists() || !node.isReadable() || !node.isDirectory())
			return nullptr;

		Common::FSNode fileNode = node.getChild(ARCHIVE_NAME);
		if (fileNode.exists() && fileNode.isReadable() && !fileNode.isDirectory()) {
			Common::SeekableReadStream *const stream = fileNode.createReadStream();
			Common::Archive *zipArchive = Common::makeZipArchive(stream);
			if (zipArchive) return zipArchive;
		}
	}

	// then use SearchMan to find it
	Common::ArchiveMemberList fileList;
	SearchMan.listMatchingMembers(fileList, ARCHIVE_NAME);
	for (Common::ArchiveMemberList::iterator it = fileList.begin(); it != fileList.end(); ++it) {
		Common::ArchiveMember       const &m = **it;
		Common::SeekableReadStream *const stream = m.createReadStream();
		Common::Archive *zipArchive = Common::makeZipArchive(stream);
		if (zipArchive) return zipArchive;
	}

	return nullptr;
}

Common::ArchiveMemberList IndexPageHandler::listArchive() {
	Common::ArchiveMemberList resultList;
	Common::Archive *zipArchive = getZipArchive();
	if (zipArchive) {
		zipArchive->listMembers(resultList);
		delete zipArchive;
	}
	return resultList;
}

Common::SeekableReadStream *IndexPageHandler::getArchiveFile(Common::String name) {
	Common::SeekableReadStream *result = nullptr;
	Common::Archive *zipArchive = getZipArchive();
	if (zipArchive) {
		const Common::ArchiveMemberPtr ptr = zipArchive->getMember(name);
		result = ptr->createReadStream();
		delete zipArchive;
	}
	return result;
}

Common::String IndexPageHandler::readEverythingFromStream(Common::SeekableReadStream *const stream) {
	Common::String result;
	char buf[1024];
	uint32 readBytes;
	while (!stream->eos()) {
		readBytes = stream->read(buf, 1024);
		result += Common::String(buf, readBytes);
	}
	return result;
}

} // End of namespace Networking
