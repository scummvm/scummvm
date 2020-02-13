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

#include "backends/networking/sdl_net/handlers/filesajaxpagehandler.h"
#include "backends/networking/sdl_net/handlerutils.h"
#include "backends/networking/sdl_net/localwebserver.h"
#include "common/translation.h"

namespace Networking {

#define FILES_PAGE_NAME ".filesAJAX.html"

FilesAjaxPageHandler::FilesAjaxPageHandler() {}

FilesAjaxPageHandler::~FilesAjaxPageHandler() {}

namespace {

Common::String encodeDoubleQuotesAndSlashes(Common::String s) {
	Common::String result = "";
	for (uint32 i = 0; i < s.size(); ++i)
		if (s[i] == '"') {
			result += "\\\"";
		} else if (s[i] == '\\') {
			result += "\\\\";
		} else {
			result += s[i];
		}
	return result;
}

}

/// public

void FilesAjaxPageHandler::handle(Client &client) {
	// load stylish response page from the archive
	Common::SeekableReadStream *const stream = HandlerUtils::getArchiveFile(FILES_PAGE_NAME);
	if (stream == nullptr) {
		HandlerUtils::setFilesManagerErrorMessageHandler(client, HandlerUtils::toUtf8(_("The page is not available without the resources. Make sure file wwwroot.zip from ScummVM distribution is available in 'themepath'.")));
		return;
	}

	Common::String response = HandlerUtils::readEverythingFromStream(stream);
	Common::String path = client.queryParameter("path");

	//these occur twice:
	replace(response, "{create_directory_button}", HandlerUtils::toUtf8(_("Create directory")));
	replace(response, "{create_directory_button}", HandlerUtils::toUtf8(_("Create directory")));
	replace(response, "{upload_files_button}", HandlerUtils::toUtf8(_("Upload files"))); //tab
	replace(response, "{upload_file_button}", HandlerUtils::toUtf8(_("Upload files"))); //button in the tab
	replace(response, "{create_directory_desc}", HandlerUtils::toUtf8(_("Type new directory name:")));
	replace(response, "{upload_file_desc}", HandlerUtils::toUtf8(_("Select a file to upload:")));
	replace(response, "{or_upload_directory_desc}", HandlerUtils::toUtf8(_("Or select a directory (works in Chrome only):")));
	replace(response, "{index_of}", HandlerUtils::toUtf8(_("Index of ")));
	replace(response, "{loading}", HandlerUtils::toUtf8(("Loading...")));
	replace(response, "{error}", HandlerUtils::toUtf8(_("Error occurred")));
	replace(response, "{start_path}", encodeDoubleQuotesAndSlashes(path));
	LocalWebserver::setClientGetHandler(client, response);
}

} // End of namespace Networking
