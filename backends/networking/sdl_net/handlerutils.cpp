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

#include "backends/networking/sdl_net/handlerutils.h"
#include "backends/networking/sdl_net/localwebserver.h"
#include "backends/saves/default/default-saves.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/translation.h"
#include "common/unzip.h"

namespace Networking {

#define ARCHIVE_NAME "wwwroot.zip"

#define INDEX_PAGE_NAME ".index.html"

Common::Archive *HandlerUtils::getZipArchive() {
	// first search in themepath
	if (ConfMan.hasKey("themepath")) {
		const Common::FSNode &node = Common::FSNode(ConfMan.get("themepath"));
		if (node.exists() && node.isReadable() && node.isDirectory()) {
			Common::FSNode fileNode = node.getChild(ARCHIVE_NAME);
			if (fileNode.exists() && fileNode.isReadable() && !fileNode.isDirectory()) {
				Common::SeekableReadStream *const stream = fileNode.createReadStream();
				Common::Archive *zipArchive = Common::makeZipArchive(stream);
				if (zipArchive)
					return zipArchive;
			}
		}
	}

	// then use SearchMan to find it
	Common::ArchiveMemberList fileList;
	SearchMan.listMatchingMembers(fileList, ARCHIVE_NAME);
	for (Common::ArchiveMemberList::iterator it = fileList.begin(); it != fileList.end(); ++it) {
		Common::ArchiveMember       const &m = **it;
		Common::SeekableReadStream *const stream = m.createReadStream();
		Common::Archive *zipArchive = Common::makeZipArchive(stream);
		if (zipArchive)
			return zipArchive;
	}

	return nullptr;
}

Common::ArchiveMemberList HandlerUtils::listArchive() {
	Common::ArchiveMemberList resultList;
	Common::Archive *zipArchive = getZipArchive();
	if (zipArchive) {
		zipArchive->listMembers(resultList);
		delete zipArchive;
	}
	return resultList;
}

Common::SeekableReadStream *HandlerUtils::getArchiveFile(Common::String name) {
	Common::SeekableReadStream *result = nullptr;
	Common::Archive *zipArchive = getZipArchive();
	if (zipArchive) {
		const Common::ArchiveMemberPtr ptr = zipArchive->getMember(name);
		if (ptr.get() == nullptr)
			return nullptr;
		result = ptr->createReadStream();
		delete zipArchive;
	}
	return result;
}

Common::String HandlerUtils::readEverythingFromStream(Common::SeekableReadStream *const stream) {
	Common::String result;
	char buf[1024];
	uint32 readBytes;
	while (!stream->eos()) {
		readBytes = stream->read(buf, 1024);
		result += Common::String(buf, readBytes);
	}
	return result;
}

Common::String HandlerUtils::normalizePath(const Common::String &path) {
	Common::String normalized;
	bool slash = false;
	for (uint32 i = 0; i < path.size(); ++i) {
		char c = path[i];
		if (c == '\\' || c == '/') {
			slash = true;
			continue;
		}

		if (slash) {
			normalized += '/';
			slash = false;
		}

		if ('A' <= c && c <= 'Z') {
			normalized += c - 'A' + 'a';
		} else {
			normalized += c;
		}
	}
	if (slash) normalized += '/';
	return normalized;
}

bool HandlerUtils::hasForbiddenCombinations(const Common::String &path) {
	return (path.contains("/../") || path.contains("\\..\\") || path.contains("\\../") || path.contains("/..\\"));
}

bool HandlerUtils::isBlacklisted(const Common::String &path) {
	const char *blacklist[] = {
		"/etc",
		"/bin",
		"c:/windows" // just saying: I know guys who install windows on another drives
	};

	// normalize path
	Common::String normalized = normalizePath(path);

	uint32 size = sizeof(blacklist) / sizeof(const char *);
	for (uint32 i = 0; i < size; ++i)
		if (normalized.hasPrefix(blacklist[i]))
			return true;

	return false;
}

bool HandlerUtils::hasPermittedPrefix(const Common::String &path) {
	// normalize path
	Common::String normalized = normalizePath(path);

	// prefix for /root/
	Common::String prefix;
	if (ConfMan.hasKey("rootpath", "cloud")) {
		prefix = normalizePath(ConfMan.get("rootpath", "cloud"));
		if (prefix == "/" || normalized.hasPrefix(prefix))
			return true;
	}

	// prefix for /saves/
#ifdef USE_LIBCURL
	DefaultSaveFileManager *manager = dynamic_cast<DefaultSaveFileManager *>(g_system->getSavefileManager());
	prefix = (manager ? manager->concatWithSavesPath("") : ConfMan.get("savepath"));
#else
	prefix = ConfMan.get("savepath");
#endif
	return normalized.hasPrefix(normalizePath(prefix))
	       || normalizePath(prefix).compareTo(normalized + "/") == 0;
}

bool HandlerUtils::permittedPath(const Common::String path) {
	return hasPermittedPrefix(path) && !isBlacklisted(path);
}

void HandlerUtils::setMessageHandler(Client &client, Common::String message, Common::String redirectTo) {
	Common::String response = "<html><head><title>ScummVM</title><meta charset=\"utf-8\"/></head><body>{message}</body></html>";

	// load stylish response page from the archive
	Common::SeekableReadStream *const stream = getArchiveFile(INDEX_PAGE_NAME);
	if (stream)
		response = readEverythingFromStream(stream);

	replace(response, "{message}", message);
	if (redirectTo.empty())
		LocalWebserver::setClientGetHandler(client, response);
	else
		LocalWebserver::setClientRedirectHandler(client, response, redirectTo);
}

void HandlerUtils::setFilesManagerErrorMessageHandler(Client &client, Common::String message, Common::String redirectTo) {
	setMessageHandler(
		client,
		Common::String::format(
			"%s<br/><a href=\"files%s?path=%s\">%s</a>",
			message.c_str(),
			client.queryParameter("ajax") == "true" ? "AJAX" : "",
			"%2F", //that's encoded "/"
			Common::convertFromU32String(_("Back to the files manager")).c_str()
		),
		redirectTo
	);
}

} // End of namespace Networking
