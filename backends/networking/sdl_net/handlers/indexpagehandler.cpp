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
#include "backends/networking/sdl_net/handlerutils.h"
#include "backends/networking/sdl_net/localwebserver.h"
#include "common/translation.h"
#include "gui/storagewizarddialog.h"

namespace Networking {

IndexPageHandler::IndexPageHandler(): CommandSender(nullptr) {}

IndexPageHandler::~IndexPageHandler() {}

/// public

Common::String IndexPageHandler::code() const { return _code; }

void IndexPageHandler::handle(Client &client) {
	Common::String queryCode = client.queryParameter("code");

	if (queryCode == "") {
		// redirect to "/filesAJAX"
		HandlerUtils::setMessageHandler(
			client,
			Common::String::format(
				"%s<br/><a href=\"files\">%s</a>",
				_("This is a local webserver index page."),
				_("Open Files manager")
			),
			"/filesAJAX"
		);
		return;
	}

	_code = queryCode;
	sendCommand(GUI::kStorageCodePassedCmd, 0);
	HandlerUtils::setMessageHandler(client, _("ScummVM got the code and already connects to your cloud storage!"));
}

bool IndexPageHandler::minimalModeSupported() {
	return true;
}

} // End of namespace Networking
