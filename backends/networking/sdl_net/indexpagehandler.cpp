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
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/translation.h"
#include "common/unzip.h"
#include "gui/storagewizarddialog.h"

namespace Networking {

#define ARCHIVE_NAME "wwwroot.zip"
#define INDEX_PAGE_NAME "index.html"

IndexPageHandler::IndexPageHandler(): CommandSender(nullptr) {}

IndexPageHandler::~IndexPageHandler() {
	LocalServer.removePathHandler("/");

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

void IndexPageHandler::handleResource(Client &client) {
	Common::String filename = client.path();
	filename.deleteChar(0);
	LocalWebserver::setClientGetHandler(client, getArchiveFile(filename), 200, LocalWebserver::determineMimeType(filename));
}

/// public

void IndexPageHandler::addPathHandler(LocalWebserver &server) {
	// we can't use LocalServer yet, because IndexPageHandler is created while LocalWebserver is created
	// (thus no _instance is available and it causes stack overflow)
	server.addPathHandler("/", new Common::Callback<IndexPageHandler, Client &>(this, &IndexPageHandler::handle));

	Common::ArchiveMemberList fileList = listArchive();
	for (Common::ArchiveMemberList::iterator it = fileList.begin(); it != fileList.end(); ++it) {
		Common::ArchiveMember const &m = **it;
		if (m.getName() == INDEX_PAGE_NAME) continue;		
		server.addPathHandler("/" + m.getName(), new Common::Callback<IndexPageHandler, Client &>(this, &IndexPageHandler::handleResource));		
	}
}

Common::String IndexPageHandler::code() { return _code; }

/// utils

void IndexPageHandler::replace(Common::String &source, const Common::String &what, const Common::String &with) {
	const char *cstr = source.c_str();
	const char *position = strstr(cstr, what.c_str());
	if (position) {
		uint32 index = position - cstr;
		source.replace(index, what.size(), with);
	}
}

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
