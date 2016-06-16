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
#include "gui/storagewizarddialog.h"

namespace Networking {

IndexPageHandler::IndexPageHandler(): CommandSender(nullptr) {}

IndexPageHandler::~IndexPageHandler() {
	LocalServer.removePathHandler("/");
}

void IndexPageHandler::handle(Client &client) {	
	Common::String code = client.queryParameter("code");

	if (code == "") {
		LocalWebserver::setClientGetHandler(client, "<html><head><title>ScummVM</title></head><body>This is a local webserver index page.</body></html>");
		return;
	}

	_code = code;
	sendCommand(GUI::kStorageCodePassedCmd, 0);
	LocalWebserver::setClientGetHandler(client, "<html><head><title>ScummVM</title></head><body>ScummVM got the code and already connects to your cloud storage!</body></html>");
}

void IndexPageHandler::addPathHandler(LocalWebserver &server) {
	// we can't use LocalServer yet, because IndexPageHandler is created while LocalWebserver is created
	// (thus no _instance is available and it causes stack overflow)
	server.addPathHandler("/", new Common::Callback<IndexPageHandler, Client &>(this, &IndexPageHandler::handle));
}

Common::String IndexPageHandler::code() { return _code; }

} // End of namespace Networking
