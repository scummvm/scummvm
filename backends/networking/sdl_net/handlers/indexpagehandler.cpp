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

#include "backends/networking/sdl_net/handlers/indexpagehandler.h"
#include "backends/networking/sdl_net/localwebserver.h"
#include "backends/saves/default/default-saves.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/translation.h"
#include "gui/storagewizarddialog.h"

namespace Networking {

#define INDEX_PAGE_NAME ".index.html"

IndexPageHandler::IndexPageHandler(): CommandSender(nullptr) {}

IndexPageHandler::~IndexPageHandler() {}

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

/// public

Common::String IndexPageHandler::code() { return _code; }

ClientHandlerCallback IndexPageHandler::getHandler() {
	return new Common::Callback<IndexPageHandler, Client &>(this, &IndexPageHandler::handle);
}

} // End of namespace Networking
