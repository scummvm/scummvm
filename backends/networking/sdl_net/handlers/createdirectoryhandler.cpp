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
#include "backends/fs/fs-factory.h"
#include "backends/networking/sdl_net/handlerutils.h"
#include "backends/networking/sdl_net/localwebserver.h"
#include "common/translation.h"
#include <common/callback.h>

namespace Networking {

CreateDirectoryHandler::CreateDirectoryHandler() {}

CreateDirectoryHandler::~CreateDirectoryHandler() {}

void CreateDirectoryHandler::handleError(Client &client, Common::String message) const {
	if (client.queryParameter("answer_json") == "true")
		setJsonResponseHandler(client, "error", message);
	else
		HandlerUtils::setFilesManagerErrorMessageHandler(client, message);
}

void CreateDirectoryHandler::setJsonResponseHandler(Client &client, Common::String type, Common::String message) const {
	Common::JSONObject response;
	response.setVal("type", new Common::JSONValue(type));
	response.setVal("message", new Common::JSONValue(message));

	Common::JSONValue json = response;
	LocalWebserver::setClientGetHandler(client, json.stringify(true));
}

/// public

void CreateDirectoryHandler::handle(Client &client) {
	Common::String path = client.queryParameter("path");
	Common::String name = client.queryParameter("directory_name");

	// check that <path> is not an absolute root
	if (path == "" || path == "/") {
		handleError(client, Common::convertFromU32String(_("Can't create directory here!")));
		return;
	}

	// check that <path> contains no '../'
	if (HandlerUtils::hasForbiddenCombinations(path)) {
		handleError(client, Common::convertFromU32String(_("Invalid path!")));
		return;
	}

	// transform virtual path to actual file system one
	Common::String prefixToRemove = "", prefixToAdd = "";
	if (!transformPath(path, prefixToRemove, prefixToAdd) || path.empty()) {
		handleError(client, Common::convertFromU32String(_("Invalid path!")));
		return;
	}

	// check that <path> exists, is directory and isn't forbidden
	AbstractFSNode *node = g_system->getFilesystemFactory()->makeFileNodePath(path);
	if (!HandlerUtils::permittedPath(node->getPath())) {
		handleError(client, Common::convertFromU32String(_("Invalid path!")));
		return;
	}
	if (!node->exists()) {
		handleError(client, Common::convertFromU32String(_("Parent directory doesn't exists!")));
		return;
	}
	if (!node->isDirectory()) {
		handleError(client, Common::convertFromU32String(_("Can't create a directory within a file!")));
		return;
	}

	// check that <directory_name> doesn't exist or is directory
	if (path.lastChar() != '/' && path.lastChar() != '\\')
		path += '/';
	node = g_system->getFilesystemFactory()->makeFileNodePath(path + name);
	if (node->exists()) {
		if (!node->isDirectory()) {
			handleError(client, Common::convertFromU32String(_("There is a file with that name in the parent directory!")));
			return;
		}
	} else {
		// create the <directory_name> in <path>
		if (!node->createDirectory()) {
			handleError(client, Common::convertFromU32String(_("Failed to create the directory!")));
			return;
		}
	}

	// if json requested, respond with it
	if (client.queryParameter("answer_json") == "true") {
		setJsonResponseHandler(client, "success", Common::convertFromU32String(_("Directory created successfully!")));
		return;
	}

	// set redirect on success
	HandlerUtils::setMessageHandler(
		client,
		Common::String::format(
			"%s<br/><a href=\"files?path=%s\">%s</a>",
			Common::convertFromU32String(_("Directory created successfully!")).c_str(),
			client.queryParameter("path").c_str(),
			Common::convertFromU32String(_("Back to parent directory")).c_str()
		),
		(client.queryParameter("ajax") == "true" ? "/filesAJAX?path=" : "/files?path=") +
		LocalWebserver::urlEncodeQueryParameterValue(client.queryParameter("path"))
	);
}

} // End of namespace Networking
