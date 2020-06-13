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

#include "backends/networking/sdl_net/handlers/listajaxhandler.h"
#include "backends/networking/sdl_net/handlerutils.h"
#include "backends/networking/sdl_net/localwebserver.h"
#include "common/config-manager.h"
#include "common/json.h"
#include "common/translation.h"

namespace Networking {

ListAjaxHandler::ListAjaxHandler() {}

ListAjaxHandler::~ListAjaxHandler() {}

Common::JSONObject ListAjaxHandler::listDirectory(Common::String path) {
	Common::JSONArray itemsList;
	Common::JSONObject errorResult;
	Common::JSONObject successResult;
	successResult.setVal("type", new Common::JSONValue("success"));
	errorResult.setVal("type", new Common::JSONValue("error"));

	if (path == "" || path == "/") {
		if (ConfMan.hasKey("rootpath", "cloud"))
			addItem(itemsList, IT_DIRECTORY, "/root/", Common::convertFromU32String(_("File system root")));
		addItem(itemsList, IT_DIRECTORY, "/saves/", Common::convertFromU32String(_("Saved games")));
		successResult.setVal("items", new Common::JSONValue(itemsList));
		return successResult;
	}

	if (HandlerUtils::hasForbiddenCombinations(path))
		return errorResult;

	Common::String prefixToRemove = "", prefixToAdd = "";
	if (!transformPath(path, prefixToRemove, prefixToAdd))
		return errorResult;

	Common::FSNode node = Common::FSNode(path);
	if (path == "/")
		node = node.getParent(); // absolute root

	if (!HandlerUtils::permittedPath(node.getPath()))
		return errorResult;

	if (!node.isDirectory())
		return errorResult;

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
		addItem(itemsList, IT_PARENT_DIRECTORY, filePath, Common::convertFromU32String(_("Parent directory")));
	}

	// fill the content
	for (Common::FSList::iterator i = _nodeContent.begin(); i != _nodeContent.end(); ++i) {
		Common::String name = i->getDisplayName();
		if (i->isDirectory()) name += "/";

		Common::String filePath = i->getPath();
		if (filePath.hasPrefix(prefixToRemove))
			filePath.erase(0, prefixToRemove.size());
		filePath = prefixToAdd + filePath;

		addItem(itemsList, detectType(i->isDirectory(), name), filePath, name);
	}

	successResult.setVal("items", new Common::JSONValue(itemsList));
	return successResult;
}

ListAjaxHandler::ItemType ListAjaxHandler::detectType(bool isDirectory, const Common::String &name) {
	if (isDirectory)
		return IT_DIRECTORY;
	if (name.hasSuffix(".txt"))
		return IT_TXT;
	if (name.hasSuffix(".zip"))
		return IT_ZIP;
	if (name.hasSuffix(".7z"))
		return IT_7Z;
	return IT_UNKNOWN;
}

void ListAjaxHandler::addItem(Common::JSONArray &responseItemsList, ItemType itemType, Common::String path, Common::String name, Common::String size) {
	Common::String icon;
	bool isDirectory = (itemType == IT_DIRECTORY || itemType == IT_PARENT_DIRECTORY);
	switch (itemType) {
	case IT_DIRECTORY:
		icon = "dir.png";
		break;
	case IT_PARENT_DIRECTORY:
		icon = "up.png";
		break;
	case IT_TXT:
		icon = "txt.png";
		break;
	case IT_ZIP:
		icon = "zip.png";
		break;
	case IT_7Z:
		icon = "7z.png";
		break;
	default:
		icon = "unk.png";
	}

	Common::JSONObject item;
	item.setVal("name", new Common::JSONValue(name));
	item.setVal("path", new Common::JSONValue(path));
	item.setVal("isDirectory", new Common::JSONValue(isDirectory));
	item.setVal("size", new Common::JSONValue(size));
	item.setVal("icon", new Common::JSONValue(icon));
	responseItemsList.push_back(new Common::JSONValue(item));
}

/// public

void ListAjaxHandler::handle(Client &client) {
	Common::String path = client.queryParameter("path");
	Common::JSONValue jsonResponse = listDirectory(path);
	Common::String response = jsonResponse.stringify(true);
	LocalWebserver::setClientGetHandler(client, response);
}

} // End of namespace Networking
