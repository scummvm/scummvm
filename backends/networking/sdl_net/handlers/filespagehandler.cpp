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
#include "backends/networking/sdl_net/handlerutils.h"
#include "backends/networking/sdl_net/localwebserver.h"
#include "common/config-manager.h"
#include "common/translation.h"

namespace Networking {

#define INDEX_PAGE_NAME ".index.html"
#define FILES_PAGE_NAME ".files.html"

FilesPageHandler::FilesPageHandler() {}

FilesPageHandler::~FilesPageHandler() {}

namespace {
Common::String encodeDoubleQuotes(Common::String s) {
	Common::String result = "";
	for (uint32 i = 0; i < s.size(); ++i)
		if (s[i] == '"') {
			result += "\\\"";
		} else {
			result += s[i];
		}
	return result;
}

Common::String encodeHtmlEntities(Common::String s) {
	Common::String result = "";
	for (uint32 i = 0; i < s.size(); ++i)
		if (s[i] == '<')
			result += "&lt;";
		else if (s[i] == '>')
			result += "&gt;";
		else if (s[i] == '&')
			result += "&amp;";
		else if ((byte)s[i] > (byte)0x7F)
			result += Common::String::format("&#%d;", (int)s[i]);
		else result += s[i];
	return result;
}

Common::String getDisplayPath(Common::String s) {
	Common::String result = "";
	for (uint32 i = 0; i < s.size(); ++i)
		if (s[i] == '\\')
			result += '/';
		else
			result += s[i];
	if (result == "")
		return "/";
	return result;
}
}

bool FilesPageHandler::listDirectory(Common::String path, Common::String &content, const Common::String &itemTemplate) {
	if (path == "" || path == "/") {
		if (ConfMan.hasKey("rootpath", "cloud"))
			addItem(content, itemTemplate, IT_DIRECTORY, "/root/", Common::convertFromU32String(_("File system root")));
		addItem(content, itemTemplate, IT_DIRECTORY, "/saves/", Common::convertFromU32String(_("Saved games")));
		return true;
	}

	if (HandlerUtils::hasForbiddenCombinations(path))
		return false;

	Common::String prefixToRemove = "", prefixToAdd = "";
	if (!transformPath(path, prefixToRemove, prefixToAdd))
		return false;

	Common::FSNode node = Common::FSNode(path);
	if (path == "/")
		node = node.getParent(); // absolute root

	if (!HandlerUtils::permittedPath(node.getPath()))
		return false;

	if (!node.isDirectory())
		return false;

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
		addItem(content, itemTemplate, IT_PARENT_DIRECTORY, filePath, Common::convertFromU32String(_("Parent directory")));
	}

	// fill the content
	for (Common::FSList::iterator i = _nodeContent.begin(); i != _nodeContent.end(); ++i) {
		Common::String name = i->getDisplayName();
		if (i->isDirectory())
			name += "/";

		Common::String filePath = i->getPath();
		if (filePath.hasPrefix(prefixToRemove))
			filePath.erase(0, prefixToRemove.size());
		filePath = prefixToAdd + filePath;

		addItem(content, itemTemplate, detectType(i->isDirectory(), name), filePath, name);
	}

	return true;
}

FilesPageHandler::ItemType FilesPageHandler::detectType(bool isDirectory, const Common::String &name) {
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

void FilesPageHandler::addItem(Common::String &content, const Common::String &itemTemplate, ItemType itemType, Common::String path, Common::String name, Common::String size) const {
	Common::String item = itemTemplate, icon;
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
	replace(item, "{icon}", icon);
	replace(item, "{link}", (isDirectory ? "files?path=" : "download?path=") + LocalWebserver::urlEncodeQueryParameterValue(path));
	replace(item, "{name}", encodeHtmlEntities(name));
	replace(item, "{size}", size);
	content += item;
}

/// public

void FilesPageHandler::handle(Client &client) {
	Common::String response =
		"<html>" \
		"<head><title>ScummVM</title><meta charset=\"utf-8\"/></head>" \
		"<body>" \
		"<p>{create_directory_desc}</p>" \
		"<form action=\"create\">" \
		"<input type=\"hidden\" name=\"path\" value=\"{path}\"/>" \
		"<input type=\"text\" name=\"directory_name\" value=\"\"/>" \
		"<input type=\"submit\" value=\"{create_directory_button}\"/>" \
		"</form>" \
		"<hr/>" \
		"<p>{upload_file_desc}</p>" \
		"<form action=\"upload?path={path}\" method=\"post\" enctype=\"multipart/form-data\">" \
		"<input type=\"file\" name=\"upload_file-f\" allowdirs multiple/>" \
		"<span>{or_upload_directory_desc}</span>" \
		"<input type=\"file\" name=\"upload_file-d\" directory webkitdirectory multiple/>" \
		"<input type=\"submit\" value=\"{upload_file_button}\"/>" \
		"</form>"
		"<hr/>" \
		"<h1>{index_of_directory}</h1>" \
		"<table>{content}</table>" \
		"</body>" \
		"</html>";
	Common::String itemTemplate = "<tr><td><img src=\"icons/{icon}\"/></td><td><a href=\"{link}\">{name}</a></td><td>{size}</td></tr>\n"; //TODO: load this template too?

																																		  // load stylish response page from the archive
	Common::SeekableReadStream *const stream = HandlerUtils::getArchiveFile(FILES_PAGE_NAME);
	if (stream)
		response = HandlerUtils::readEverythingFromStream(stream);

	Common::String path = client.queryParameter("path");
	Common::String content = "";

	// show an error message if failed to list directory
	if (!listDirectory(path, content, itemTemplate)) {
		HandlerUtils::setFilesManagerErrorMessageHandler(client, Common::convertFromU32String(_("ScummVM couldn't list the directory you specified.")).c_str());
		return;
	}

	//these occur twice:
	replace(response, "{create_directory_button}", _("Create directory").encode());
	replace(response, "{create_directory_button}", _("Create directory").encode());
	replace(response, "{path}", encodeDoubleQuotes(client.queryParameter("path")));
	replace(response, "{path}", encodeDoubleQuotes(client.queryParameter("path")));
	replace(response, "{upload_files_button}", _("Upload files").encode()); //tab
	replace(response, "{upload_file_button}", _("Upload files").encode()); //button in the tab
	replace(response, "{create_directory_desc}", _("Type new directory name:").encode());
	replace(response, "{upload_file_desc}", _("Select a file to upload:").encode());
	replace(response, "{or_upload_directory_desc}", _("Or select a directory (works in Chrome only):").encode());
	replace(response, "{index_of_directory}", Common::String::format("%s %s", _("Index of").encode().c_str(), encodeHtmlEntities(getDisplayPath(client.queryParameter("path"))).c_str()));
	replace(response, "{content}", content);
	LocalWebserver::setClientGetHandler(client, response);
}

} // End of namespace Networking
