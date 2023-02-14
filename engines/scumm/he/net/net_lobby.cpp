/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "base/version.h"
#include "common/config-manager.h"

#include "scumm/he/intern_he.h"
#include "scumm/he/net/net_lobby.h"

namespace Scumm {

Lobby::Lobby(ScummEngine_v90he *vm) : _vm(vm) {
	_gameName = _vm->_game.gameid;
	if (_gameName == "baseball2001")
		_gameName = "baseball";
	_socket = nullptr;

	_userId = 0;
}

Lobby::~Lobby() {
	if (_socket)
		delete _socket;
}

void Lobby::writeStringArray(int array, Common::String string) {
	int newArray = 0;
	byte *data = _vm->defineArray(array, ScummEngine_v90he::kStringArray, 0, 0, 0, strlen(string.c_str()), true, &newArray);
	memcpy(data, string.c_str(), strlen(string.c_str()));
	_vm->writeVar(array, newArray);
}

void Lobby::doNetworkOnceAFrame() {
	if (!_socket)
		return;

	int ready = _socket->ready();
	if (ready) {
		receiveData();
	}
}

void Lobby::send(Common::JSONObject data) {
	if (!_socket) {
		warning("LOBBY: Attempted to send data while not connected to server");
		return;
	}
	Common::JSONValue value(data);
	Common::String valueString = Common::JSON::stringify(&value);
	// Add new line.
	valueString += "\n";

	debug(1, "LOBBY: Sending data: %s", valueString.c_str());
	_socket->send(valueString.c_str(), strlen(valueString.c_str()));
}

void Lobby::receiveData() {
	if (!_socket)
		return;

	char data[1024];
	size_t len = _socket->recv(data, 1024);
	if (!len) {
		// We have been disconnected.
		disconnect(true);
	}

	Common::String data_str(data, len);
	_buffer += data_str;

	while (_buffer.contains("\n")) {
		int pos = _buffer.findFirstOf('\n');
		processLine(_buffer.substr(0, pos));
		_buffer = _buffer.substr(pos + 1);
	}
}

void Lobby::processLine(Common::String line) {
	debug(1, "LOBBY: Received Data: %s", line.c_str());
	Common::JSONValue *json = Common::JSON::parse(line.c_str());
	if (!json) {
		warning("LOBBY: Received trunciated data from server! %s", line.c_str());
		return;
	}
	if (!json->isObject()){
		warning("LOBBY: Received non JSON object from server! %s", line.c_str());
		return;
	}

	Common::JSONObject root = json->asObject();
	if (root.find("cmd") != root.end() && root["cmd"]->isString()) {
		Common::String command = root["cmd"]->asString();
		if (command == "heartbeat") {
			handleHeartbeat();
		} else if (command == "login_resp") {
			int errorCode = root["error_code"]->asIntegerNumber();
			int userId = root["id"]->asIntegerNumber();
			Common::String response = root["response"]->asString();
			handleLoginResp(errorCode, userId, response);
		} else if (command == "profile_info") {
			Common::JSONArray profile = root["profile"]->asArray();
			handleProfileInfo(profile);
		}
	}
}

int32 Lobby::dispatch(int op, int numArgs, int32 *args) {
	int res = 0;

	switch(op) {
	case OP_NET_OPEN_WEB_URL:
		char url[128];
		_vm->getStringFromArray(args[0], url, sizeof(url));

		openUrl(url);
		break;
	case OP_NET_DOWNLOAD_PLAYBOOK:
		// TODO
		break;
	case OP_NET_CONNECT:
		connect();
		break;
	case OP_NET_DISCONNECT:
		disconnect();
		break;
	case OP_NET_LOGIN:
		char userName[16];
		char password[16];

		_vm->getStringFromArray(args[0], userName, sizeof(userName));
		_vm->getStringFromArray(args[1], password, sizeof(password));

		login(userName, password);
		break;
	case OP_NET_GET_PROFILE:
		getUserProfile(args[0]);
		break;
	case OP_NET_CHANGE_ICON:
		setIcon(args[0]);
		break;
	
	case OP_NET_DOWNLOAD_FILE:
		// TODO: News, Poll, and Banner downloads.
		_vm->writeVar(135, 1);
		break;
	// TODO: Should we actually implement update checks here
	// this at some point?
	case OP_NET_UPDATE_INIT:
		break;
	case OP_NET_FETCH_UPDATES:
		_vm->writeVar(111, 2);
		break;

	default:
		Common::String str = Common::String::format("LOBBY: unknown op: (%d, %d, [", op, numArgs);
		if (numArgs > 0)
			str += Common::String::format("%d", args[0]);
		for (int i = 1; i < numArgs; i++) {
			str += Common::String::format(", %d", args[i]);
		}
		str += "])";
		warning("%s", str.c_str());
	}

	return res;
}

void Lobby::handleHeartbeat() {
	Common::JSONObject heartbeat;
	heartbeat.setVal("cmd", new Common::JSONValue("heartbeat"));
	send(heartbeat);
}

void Lobby::openUrl(const char *url) {
	debug(1, "LOBBY: openURL: %s", url);
	Common::String urlString = Common::String(url);

	if (urlString == "http://www.jrsn.com/c_corner/cc_regframe.asp" ||
	    urlString == "http://www.humongoussports.com/backyard/registration/register.asp") {
		if (_vm->displayMessageYesNo("Online Play for this game is provided by Backyard Sports Online, which is a\nservice provided by the ScummVM project.\nWould you like to go to their registration page?")) {
			if (!g_system->openUrl("https://backyardsports.online/register")) {
				_vm->displayMessage(0, "Failed to open registration URL.  Please navigate to this page manually.\n\n\"https://backyardsports.online/register\"");
			}
		}
	} else {
		warning("LOBBY: URL not handled: %s", url);
	}
}

bool Lobby::connect() {
	if (_socket)
		return true;

	_socket = new Networking::CurlSocket();

	// NOTE: Even though the protocol starts with http(s), this is an entirely
	// different protocol.  This is done so we can achieve communicating over
	// TLS/SSL sockets.
	Common::String lobbyUrl = "https://multiplayer.scummvm.org:9130";
	if (ConfMan.hasKey("lobby_server")) {
		lobbyUrl = ConfMan.get("lobby_server");
	}

	// Parse the URL for checks:
	Networking::CurlURL url;
	if (url.parseURL(lobbyUrl)) {
		Common::String scheme = url.getScheme();
		if (!scheme.contains("http")) {
			warning("LOBBY: Unsupported scheme in URL: \"%s\"", scheme.c_str());
			writeStringArray(109, "Unsupported scheme in server address");
			_vm->writeVar(108, -99);
			return false;
		}

		int port = url.getPort();
		switch (port) {
		case -1:
			warning("LOBBY: Unable to get port.");
			writeStringArray(109, "Unable to get port in address");
			_vm->writeVar(108, -99);
			return false;
		case 0:
			// Add default port:
			lobbyUrl += ":9130";
			break;
		}
	} else
		warning("LOBBY: Could not parse URL, attempting to connect as is");


	// // Maybe this check could be done better...
	// int pos = lobbyUrl.findLastOf(":");
	// if (pos)
	// 	// If the URL missing a port at the end, add the default one in.
	// 	lobbyUrl += ":9130";

	debug(1, "LOBBY: Connecting to %s", lobbyUrl.c_str());

	if (_socket->connect(lobbyUrl)) {
		debug(1, "LOBBY: Successfully connected to %s", lobbyUrl.c_str());
		return true;
	} else {
		delete _socket;
		_socket = nullptr;
		writeStringArray(109, "Unable to contact server");
		_vm->writeVar(108, -99);
	}
	return false;
}

void Lobby::disconnect(bool lost) {
	if (!_socket)
		return;

	if (!lost) {
		debug(1, "LOBBY: Disconnecting connection to server.");
		Common::JSONObject disconnectObject;
		disconnectObject.setVal("cmd", new Common::JSONValue("disconnect"));
		send(disconnectObject);
	} else {
		systemAlert(901, "You have been disconnected from our server. Returning to login screen.");
	}

	delete _socket;
	_socket = nullptr;
}

void Lobby::runRemoteStartScript(int *args) {
	if (!_vm->VAR(_vm->VAR_REMOTE_START_SCRIPT)) {
		warning("LOBBY: VAR_REMOTE_START_SCRIPT not defined!");
		return;
	}
	_vm->runScript(_vm->VAR(_vm->VAR_REMOTE_START_SCRIPT), 1, 0, args);
	// These scripts always returns a 1 into the stack.  Let's pop it out.
	_vm->pop();
}

void Lobby::systemAlert(int type, Common::String message) {
	int args[25];
	memset(args, 0, sizeof(args));

	// Write the message as a string array.
	writeStringArray(0, message);

	// Setup the arguments
	args[0] = OP_REMOTE_SYSTEM_ALERT;
	args[1] = type;
	args[2] = _vm->VAR(0);

	// Run the script
	runRemoteStartScript(args);
}

void Lobby::login(const char *userName, const char *password) {
	Common::JSONObject loginRequestParameters;
	loginRequestParameters.setVal("cmd", new Common::JSONValue("login"));
	loginRequestParameters.setVal("user", new Common::JSONValue((Common::String)userName));
	loginRequestParameters.setVal("pass", new Common::JSONValue((Common::String)password));
	loginRequestParameters.setVal("game", new Common::JSONValue((Common::String)_gameName));
	loginRequestParameters.setVal("version", new Common::JSONValue(gScummVMVersionLite));

	send(loginRequestParameters);
}

void Lobby::handleLoginResp(int errorCode, int userId, Common::String response) {
	if (errorCode > 0) {
		writeStringArray(109, response);
		_vm->writeVar(108, -99);
		return;
	}
	_userId = userId;
	_vm->writeVar(108, 99);
}

void Lobby::getUserProfile(int userId) {
	if (!_socket)
		return;

	Common::JSONObject getProfileRequest;
	getProfileRequest.setVal("cmd", new Common::JSONValue("get_profile"));
	if (userId) {
		getProfileRequest.setVal("user_id", new Common::JSONValue((long long int)userId));
	}
	send(getProfileRequest);
}

void Lobby::handleProfileInfo(Common::JSONArray profile) {
	int newArray = 0;
	_vm->defineArray(108, ScummEngine_v90he::kDwordArray, 0, 0, 0, profile.size(), true, &newArray);
	_vm->writeVar(108, newArray);

	for (uint i = 0; i < profile.size(); i++) {
		if (profile[i]->isIntegerNumber()) {
			_vm->writeArray(108, 0, i, profile[i]->asIntegerNumber());
		} else {
			warning("LOBBY: Value for profile index %d is not an integer!", i);
		}
	}
	_vm->writeVar(111, 1);
}

void Lobby::setIcon(int icon) {
	if (!_socket)
		return;

	Common::JSONObject setIconRequest;
	setIconRequest.setVal("cmd", new Common::JSONValue("set_icon"));
	setIconRequest.setVal("icon", new Common::JSONValue((long long int)icon));
	send(setIconRequest);
}

} // End of namespace Scumm


