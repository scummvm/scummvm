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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/system.h"
#include "common/config-manager.h"

#include "scumm/he/intern_he.h"

#include "byonline.h"

#include "backends/networking/curl/connectionmanager.h"
#include <SDL_net.h>

namespace Scumm {

BYOnline::BYOnline(ScummEngine_v90he *s) {
	_vm = s;

	_inArea = false;

	_socket = nullptr;
	_socketSet = SDLNet_AllocSocketSet(1);
	if (!_socketSet) {
		warning("BYOnline: Unable to allocate socket set, you may be unable to connect to server. (%s)", SDLNet_GetError());
	}
}

BYOnline::~BYOnline() {
	disconnect();
}

void BYOnline::writeStringArray(int array, Common::String string) {
	int newArray = 0;
	byte* data = _vm->defineArray(array, ScummEngine_v90he::kStringArray, 0, 0, 0, strlen(string.c_str()), true, &newArray);
	memcpy(data, string.c_str(), strlen(string.c_str()));
	_vm->writeVar(array, newArray);
}

void BYOnline::connect() {
	if (connected()) return;

	IPaddress ip;
	int numused;
	Common::String address;

	if (ConfMan.hasKey("byonline_server_address"))
		address = ConfMan.get("byonline_server_address");
	else
		// Default address
		address = Common::String("server.backyardsports.online");

	uint16 port = 9130;
	if (address.contains(":")) {
		// If the address contains a custom port, use that.
		int pos = address.findFirstOf(':');
		port = (uint16)strtol(address.substr(pos + 1).c_str(), nullptr, 0);
		// Strip the port out of the address string
		address = address.substr(0, pos);
	}

	if (SDLNet_ResolveHost(&ip, address.c_str(), port) == -1) {
		warning("BYOnline: SDLNet_ResolveHost: %s", SDLNet_GetError());
		goto connection_failure;
	}

	printf("BYOnline: Connecting to %s:%u...\n", address.c_str(), port);

	_socket = SDLNet_TCP_Open(&ip);
	if (!_socket) {
		warning("BYOnline: SDLNet_TCP_Open: %s", SDLNet_GetError());
		goto connection_failure;
	}

	numused = SDLNet_TCP_AddSocket(_socketSet, _socket);
	if (numused == -1) {
		warning("BYOnline: SDLNet_AddSocket: %s", SDLNet_GetError());
		goto connection_failure;
	}

	printf("BYOnline: Connected!\n");
	return;

connection_failure:
	writeStringArray(109, "Unable to contact server");
	_vm->writeVar(108, -99);
}

void BYOnline::disconnect(bool lost) {
	if (!connected()) return;

	if (!lost) {
		Common::JSONObject disconnectObject;
		disconnectObject.setVal("cmd", new Common::JSONValue("disconnect"));
		send(disconnectObject);
	} else {
		systemAlert(901, "You have been disconnected from BYOnline. Returning to login screen.");
	}

	_userId = 0;
	_playerId = 0;
	_gamesPlaying = 0;

	_inArea = false;
	_inGame = false;

	SDLNet_TCP_Close(_socket);

	int numused = SDLNet_TCP_DelSocket(_socketSet, _socket);
	if(numused == -1) {
		warning("BYOnline: SDLNet_DelSocket: %s", SDLNet_GetError());
	}
	_socket = nullptr;
}

void BYOnline::send(Common::JSONObject data) {
	if (!connected()) {
		warning("BYOnline: Attempted to send data while not connected to server");
		return;
	}
	Common::JSONValue value(data);
	Common::String valueString = Common::JSON::stringify(&value);
	// Add new line.
	valueString += "\n";

	debugC(DEBUG_BYONLINE, "BYOnline: Sending data: %s", valueString.c_str());

	size_t result = SDLNet_TCP_Send(_socket, valueString.c_str(), strlen(valueString.c_str()));
	if (result < strlen(valueString.c_str())) {
		// Disconnected.
		printf("BYOnline: Lost connection to server.\n");
		disconnect(true);
		// connectionLost();
	}

}

void BYOnline::startOfFrame() {
	if (!connected()) return;

	int ready = SDLNet_CheckSockets(_socketSet, 0);
	if (ready) {
		receiveData();
	} else if (ready == -1) {
		warning("SDLNet_CheckSockets: %s", SDLNet_GetError());
	}

	if (_inArea && !_inGame) {
		// Update games playing
		_vm->writeVar(110, _gamesPlaying);
	}

}

void BYOnline::receiveData() {
	char data[1024];
	int len=SDLNet_TCP_Recv(_socket, data, 1024);
	if (!len) {
		// Assume disconnection
		printf("BYOnline: Lost connection to server.\n");
		disconnect(true);
		return;
	}
	Common::String data_str(data, len);
	_buffer += data_str;

	while (_buffer.contains("\n")) {
		int pos = _buffer.findFirstOf('\n');
		processLine(_buffer.substr(0, pos));
		_buffer = _buffer.substr(pos + 1);
	}
}

void BYOnline::processLine(Common::String line) {
	debugC(DEBUG_BYONLINE, "BYOnline: Received Data: %s", line.c_str());
	Common::JSONValue *json = Common::JSON::parse(line.c_str());
	if (!json) {
		warning("BYOnline: Received trunciated data from server! %s", line.c_str());
		return;
	}
	if (!json->isObject()){
		warning("BYOnline: Received non JSON object from server! %s", line.c_str());
		return;
	}

	Common::JSONObject root = json->asObject();
	if (root.find("cmd") != root.end() && root["cmd"]->isString()) {
		Common::String command = root["cmd"]->asString();
		if (command == "heartbeat") {
			handleHeartbeat();
		} else if (command == "disconnect") {
			int type = root["type"]->asIntegerNumber();
			Common::String message = root["message"]->asString();
			systemAlert(type, message);
			_vm->writeVar(747, 0);
			disconnect(true);
		} else if (command == "login_resp") {
			long long int errorCode = root["error_code"]->asIntegerNumber();
			long long int userId = root["id"]->asIntegerNumber();
			Common::String response = root["response"]->asString();
			handleLoginResp((int)errorCode, (int)userId, response);
		} else if (command == "profile_info") {
			Common::JSONArray profile = root["profile"]->asArray();
			handleProfileInfo(profile);
		} else if (command == "population_resp") {
			long long int areaId = root["area"]->asIntegerNumber();
			long long int population = root["population"]->asIntegerNumber();
			handlePopulation((int)areaId, (int)population);
		} else if (command == "locate_resp") {
			int code = root["code"]->asIntegerNumber();
			int areaId = root["areaId"]->asIntegerNumber();
			Common::String area = root["area"]->asString();
			handleLocateResp(code, areaId, area);
		} else if (command == "players_list") {
			Common::JSONArray playersList = root["players"]->asArray();
			handlePlayersList(playersList);
		} else if (command == "games_playing") {
			int games = root["games"]->asIntegerNumber();
			handleGamesPlaying(games);
		} else if (command == "receive_challenge") {
			long long int user = root["user"]->asIntegerNumber();
			long long int stadium = root["stadium"]->asIntegerNumber();
			Common::String name = root["name"]->asString();
			handleReceiveChallenge((int)user, (int)stadium, name);
		} else if (command == "receiver_busy") {
			handleReceiverBusy();
		} else if (command == "considering_challenge") {
			handleConsideringChallenge();
		} else if (command == "counter_challenge") {
			long long int stadium = root["stadium"]->asIntegerNumber();
			handleCounterChallenge((int)stadium);
		} else if (command == "decline_challenge") {
			long long int notResponding = root["not_responding"]->asIntegerNumber();
			handleDeclineChallenge((int)notResponding);
		} else if (command == "accept_challenge") {
			handleAcceptChallenge();
		} else if (command == "game_session") {
			long long int session = root["session"]->asIntegerNumber();
			handleGameSession((int)session);
		} else if (command == "game_relay") {
			int relay = root["relay"]->asIntegerNumber();
			handleGameRelay(relay);
		} else if (command == "teams") {
			Common::JSONArray userTeam = root["user"]->asArray();
			Common::JSONArray opponentTeam = root["opponent"]->asArray();
			handleTeams(userTeam, opponentTeam);
		}
	}
}

void BYOnline::handleHeartbeat() {
	Common::JSONObject heartbeat;
	heartbeat.setVal("cmd", new Common::JSONValue("heartbeat"));
	send(heartbeat);
}

bool BYOnline::connected() {
	return (_socket != nullptr);
}

int32 BYOnline::dispatch(int op, int numArgs, int32 *args) {
	int res = 0;

	switch (op) {
	case OP_NET_CONNECT:
		// connect();
		break;
	case OP_NET_DISCONNECT:
		disconnect();
		break;
	case OP_NET_LOGIN:
		login(args);
		break;
	case OP_NET_ENTER_AREA:
		enterArea(args[0]);
		break;
	case OP_NET_GET_NUM_PLAYERS_IN_AREA:
		// Refreshed with var115 = 2
		res = _playersList.size();
		break;
	case OP_NET_FETCH_PLAYERS_INFO_IN_AREA:
		getPlayersList(args[0], args[1]);
		break;
	case OP_NET_GET_PLAYERS_INFO:
		getPlayerInfo(args[0]);
		break;
	case OP_NET_START_HOSTING_GAME:
		startHostingGame(args[0]);
		break;
	case OP_NET_CALL_PLAYER:
		challengePlayer(args[0], args[1]);
		break;
	case OP_NET_RECEIVER_BUSY:
		sendBusy(args[0]);
		break;
	case OP_NET_COUNTER_CHALLENGE:
		counterChallenge(args[0]);
		break;
	case OP_NET_GET_PROFILE:
		handleGetProfile(args);
		break;
	case OP_NET_DECLINE_CHALLENGE:
		declineChallenge(args[0]);
		break;
	case OP_NET_ACCEPT_CHALLENGE:
		acceptChallenge(args[0]);
		break;
	case OP_NET_STOP_CALLING:
		challengeTimeout(args[0]);
		break;
	case OP_NET_LEAVE_AREA:
		leaveArea();
		break;
	case OP_NET_GAME_FINISHED:
		gameFinished();
		break;
	case OP_NET_GAME_STARTED:
		gameStarted(args[0], args[1], args[2]);
		break;
	case OP_NET_UPDATE_PROFILE_ARRAY:
		sendGameResults(args[0], args[1], args[2]);
		break;
	case OP_NET_LOCATE_PLAYER:
		locatePlayer(args[0]);
		break;
	case OP_NET_GET_POPULATION:
		getPopulation(args[0], args[1]);
		break;
	case OP_NET_CHANGE_ICON:
		setProfile("icon", args[0]);
		break;
	case OP_NET_SET_PHONE_STATUS:
		setPhoneStatus(args[0]);
		break;
	case OP_NET_ANSWER_PHONE:
		res = answerPhone(args[0]);
		break;
	case OP_NET_DOWNLOAD_FILE:
		// TODO
		_vm->writeVar(135, 1);
		break;
	case OP_NET_CHECK_INTERNET_STATUS:
		res = 1;
		break;
	case OP_NET_UPDATE_INIT:
		break;
	case OP_NET_FETCH_UPDATES:
		_vm->writeVar(111, 2);
		break;
	default:
		Common::String str = Common::String::format("BYOnline: unknown op: (%d, %d, [", op, numArgs);
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

void BYOnline::runRemoteStartScript(int *args) {
	if (!_vm->VAR(_vm->VAR_REMOTE_START_SCRIPT)) {
		warning("BYOnline: VAR_REMOTE_START_SCRIPT not defined!");
		return;
	}
	_vm->runScript(_vm->VAR(_vm->VAR_REMOTE_START_SCRIPT), 1, 0, args);
	// These scripts always returns a 1 into the stack.  Let's pop it out.
	_vm->pop();
}

void BYOnline::systemAlert(int type, Common::String message) {
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

void BYOnline::login(int32 *args) {
	// Script 49 (Football)
	debugC(DEBUG_BYONLINE, "BYOnline: Logging in...");

	char userName[16];
	char password[16];
	_vm->getStringFromArray(args[0], userName, sizeof(userName));
	_vm->getStringFromArray(args[1], password, sizeof(password));

	if (!ConfMan.hasKey("byonline_web_login") || ConfMan.getBool("byonline_web_login")) {
		Common::String url;
		if (ConfMan.hasKey("byonline_web_url")) {
			url = ConfMan.get("byonline_web_url");
		} else {
			url = "https://backyardsports.online/api/login";
		}

		Networking::PostRequest rq(url,
			new Common::Callback<BYOnline, Common::JSONValue *>(this, &BYOnline::loginCallback),
			new Common::Callback<BYOnline, Networking::ErrorResponse>(this, &BYOnline::loginErrorCallback));

		Common::JSONObject loginRequestParameters;
		loginRequestParameters.setVal("user", new Common::JSONValue((Common::String)userName));
		loginRequestParameters.setVal("pass", new Common::JSONValue((Common::String)password));
		loginRequestParameters.setVal("game", new Common::JSONValue((Common::String)((_vm->_game.id == GID_FOOTBALL) ? "football" : "baseball")));

		Common::JSONValue value(loginRequestParameters);
		rq.setJSONData(&value);
		rq.start();

		while(rq.state() == Networking::PROCESSING) {
			g_system->delayMillis(5);
		}
	} else {
		connect();
		if (connected()) {
			Common::JSONObject loginRequestParameters;
			loginRequestParameters.setVal("cmd", new Common::JSONValue("login"));
			loginRequestParameters.setVal("user", new Common::JSONValue((Common::String)userName));
			loginRequestParameters.setVal("pass", new Common::JSONValue((Common::String)password));
			loginRequestParameters.setVal("game", new Common::JSONValue((Common::String)((_vm->_game.id == GID_FOOTBALL) ? "football" : "baseball")));

			send(loginRequestParameters);
		}
	}
}

void BYOnline::loginCallback(Common::JSONValue *response) {
	Common::JSONObject info = response->asObject();

	if (info.contains("token")) {
		connect();
		if (connected()) {
			Common::String token = info["token"]->asString();
			Common::JSONObject loginTokenRequest;
			loginTokenRequest.setVal("cmd", new Common::JSONValue("login_token"));
			loginTokenRequest.setVal("token", new Common::JSONValue(token));
			loginTokenRequest.setVal("game", new Common::JSONValue((Common::String)((_vm->_game.id == GID_FOOTBALL) ? "football" : "baseball")));

			send(loginTokenRequest);

			_vm->writeVar(110, 7); // Verifying Clearance
		}
	} else if (info.contains("message")) {
		writeStringArray(109, info["message"]->asString());
		_vm->writeVar(108, -99);
	} else {
		writeStringArray(109, "Unknown error has occured when trying to log in.  Please try again later.");
		_vm->writeVar(108, -99);
	}
}

void BYOnline::loginErrorCallback(Networking::ErrorResponse error) {
	writeStringArray(109, "An internal error has occured when trying to log in.  Please try again later.");
	_vm->writeVar(108, -99);
}

void BYOnline::handleLoginResp(int errorCode, int userId, Common::String response) {
	if (errorCode > 0) {
		writeStringArray(109, response);
		_vm->writeVar(108, -99);
		return;
	}
	_userId = userId;
	_vm->writeVar(108, 99);
}

void BYOnline::handleGetProfile(int32 *args) {
	if (!connected()) {
		warning("BYOnline: Got get profile op without connecting to server first!");
		return;
	}

	Common::JSONObject getProfileRequest;
	getProfileRequest.setVal("cmd", new Common::JSONValue("get_profile"));
	if (args[0]) {
		getProfileRequest.setVal("user_id", new Common::JSONValue((long long int)args[0]));
	}

	send(getProfileRequest);
}

void BYOnline::handleProfileInfo(Common::JSONArray profile) {
	int newArray = 0;
	_vm->defineArray(108, ScummEngine_v90he::kDwordArray, 0, 0, 0, profile.size(), true, &newArray);
	_vm->writeVar(108, newArray);

	for (uint i = 0; i < profile.size(); i++) {
		if (profile[i]->isIntegerNumber()) {
			_vm->writeArray(108, 0, i, profile[i]->asIntegerNumber());
		} else {
			warning("BYOnline: Value for profile index %d is not an integer!", i);
		}
	}
	// _vm->writeArray(108, 0, 30, 1);
	_vm->writeVar(111, 1);
}

void BYOnline::handleTeams(Common::JSONArray userTeam, Common::JSONArray opponentTeam) {
	// We're going to store our team in array 748, which seems to be otherwise unused
	// Then we'll pull from that array as needed later
	int userTeamArray = 0;
	_vm->defineArray(748, ScummEngine_v90he::kIntArray, 0, 0, 0, userTeam.size(), true, &userTeamArray);
	_vm->writeVar(748, userTeamArray);

	for (uint i = 0; i < userTeam.size(); i++) {
		if (userTeam[i]->isIntegerNumber()) {
			_vm->writeArray(748, 0, i, userTeam[i]->asIntegerNumber());
		} else {
			warning("BYOnline: Value for user team index %d is not an integer!", i);
		}
	}

	// And similarly store the opponent's team in array 749
	int opponentTeamArray = 0;
	_vm->defineArray(749, ScummEngine_v90he::kIntArray, 0, 0, 0, opponentTeam.size(), true, &opponentTeamArray);
	_vm->writeVar(749, opponentTeamArray);

	for (uint i = 0; i < opponentTeam.size(); i++) {
		if (opponentTeam[i]->isIntegerNumber()) {
			_vm->writeArray(749, 0, i, opponentTeam[i]->asIntegerNumber());
		} else {
			warning("BYOnline: Value for opponent team index %d is not an integer!", i);
		}
	}

	// Write a one to var747 to indicate that Prince Rupert teams should be pulled from arrays 748 and 749
	_vm->writeVar(747, 1);
}

void BYOnline::setProfile(Common::String field, int32 value) {
	if (!connected()) {
		warning("BYOnline: Got set profile op without connecting to server first!");
		return;
	}

	Common::JSONObject setProfileRequest;
	setProfileRequest.setVal("cmd", new Common::JSONValue("set_icon"));
	setProfileRequest.setVal(field, new Common::JSONValue((long long int)value));
	send(setProfileRequest);
}

void BYOnline::sendGameResults(int userId, int arrayIndex, int unknown) {
	if (!connected()) {
		warning("BYOnline: Attempted to send game results without connecting to server first!");
		return;
	}

	Common::JSONObject setProfileRequest;
	setProfileRequest.setVal("cmd", new Common::JSONValue("game_results"));
	setProfileRequest.setVal("user", new Common::JSONValue((long long int)userId));

	ScummEngine_v90he::ArrayHeader *ah = (ScummEngine_v90he::ArrayHeader *)_vm->getResourceAddress(rtString, arrayIndex & ~0x33539000);
	int32 size = (FROM_LE_32(ah->dim1end) - FROM_LE_32(ah->dim1start) + 1) *
		(FROM_LE_32(ah->dim2end) - FROM_LE_32(ah->dim2start) + 1);

	Common::JSONArray arrayData;
	for (int i = 0; i < size; i++) {
		// Assuming they're dword type
		int32 data = (int32)READ_LE_UINT32(ah->data + i * 4);
		arrayData.push_back(new Common::JSONValue((long long int)data));
	}
	setProfileRequest.setVal("fields", new Common::JSONValue(arrayData));
	send(setProfileRequest);
}

void BYOnline::getPopulation(int areaId, int unknown) {
	_areaIdForPopulation = areaId;

	Common::JSONObject getPopulationRequest;
	getPopulationRequest.setVal("cmd", new Common::JSONValue("get_population"));
	getPopulationRequest.setVal("area", new Common::JSONValue((long long int)areaId));
	send(getPopulationRequest);
}

void BYOnline::handlePopulation(int areaId, int population) {
		if (areaId == _areaIdForPopulation) {
			_vm->writeVar(((_vm->_game.id == GID_FOOTBALL) ? 108 : 136), population + 1); // Game deducts the one
			_areaIdForPopulation = 0;
		}
}

void BYOnline::locatePlayer(int usernameArray) {
	if (!connected()) {
		warning("BYOnline: Attempted to locate player without connecting to server first!");
		return;
	}

	char userName[16];
	_vm->getStringFromArray(usernameArray, userName, sizeof(userName));

	Common::JSONObject locatePlayerRequest;
	locatePlayerRequest.setVal("cmd", new Common::JSONValue("locate_player"));
	locatePlayerRequest.setVal("user", new Common::JSONValue((Common::String)userName));
	send(locatePlayerRequest);
}

void BYOnline::handleLocateResp(int code, int areaId, Common::String area) {
	_vm->writeVar(108, 1);
	_vm->writeVar(110, code);
	if (code == 1) {
		writeStringArray(109, area);
		_vm->writeVar(111, areaId);
	}
}

void BYOnline::enterArea(int32 areaId) {
	if (!areaId) {
		warning("Backyard Online (enterArea): Missing area id!");
		return;
	}
	if (!connected()) {
		warning("BYOnline: Tried to enter area %d without connecting to server first!", (int)areaId);
		return;
	}

	// HACK: After you log in on Baseball, you would join this "lobby area".
	// My best guess is so you could potationally chat with other players, but
	// unsure if that's actually implemented in-game, so let's just ignore
	// that for now.
	if (_vm->_game.id == GID_BASEBALL2001 && areaId == 33) {
		return;
	}

	debugC(DEBUG_BYONLINE, "BYOnline: Entering area %d", int(areaId));

	Common::JSONObject enterAreaRequest;
	enterAreaRequest.setVal("cmd", new Common::JSONValue("enter_area"));
	enterAreaRequest.setVal("area", new Common::JSONValue((long long int)areaId));
	send(enterAreaRequest);

	_inArea = true;
}

void BYOnline::leaveArea() {
	debugC(DEBUG_BYONLINE, "BYOnline: Leaving area.");
	_playersList.clear();

	if (connected()) {
		Common::JSONObject leaveAreaRequest;
		leaveAreaRequest.setVal("cmd", new Common::JSONValue("leave_area"));
		send(leaveAreaRequest);

		_inArea = false;
	}
}

void BYOnline::getPlayersList(int start, int end) {
	if (!connected()) {
		warning("BYOnline: Tried to fetch players list without connecting to server first!");
		return;
	}

	Common::JSONObject playersListRequest;
	playersListRequest.setVal("cmd", new Common::JSONValue("get_players"));
	playersListRequest.setVal("start", new Common::JSONValue((long long int)start));
	playersListRequest.setVal("end", new Common::JSONValue((long long int)end));
	send(playersListRequest);

}

bool BYOnline::_checkPlayersLists(Common::JSONArray other) {
	// Check if the two players lists are different.
	// This exists because (_playersList != other) doesn't work.
	if (_playersList.size() != other.size())
		return true;
	for (uint i = 0; i < _playersList.size(); i++) {
		Common::JSONArray playerInfo = _playersList[i]->asArray();
		Common::JSONArray otherInfo = other[i]->asArray();

		// Check if names are different.
		if (playerInfo[0]->asString() != otherInfo[0]->asString())
			return true;
		for (uint o = 1; o < 7; o++) {
			if (playerInfo[o]->asIntegerNumber() != otherInfo[o]->asIntegerNumber())
				return true;
		}
	}
	return false;
}

void BYOnline::handlePlayersList(Common::JSONArray playersList) {
	// If the list exactly the same as before, don't do anything.
	if (_checkPlayersLists(playersList)) {
		_playersList = playersList;
		if (!_inGame) {
			// Tell the game to redisplay the list.
			_vm->writeVar(115, 2);
		}
	}
}

void BYOnline::getPlayerInfo(int32 idx) {
	if ((uint)idx - 1 > _playersList.size()) {
		warning("BYOnline: _playersList is too small for index. (%d > %d)", (int)idx, (int)_playersList.size());
		return;
	}

	Common::JSONArray playerInfo = _playersList[idx - 1]->asArray();
	int newArray = 0;
	_vm->defineArray(108, ScummEngine_v90he::kDwordArray, 0, 0, 0, 6, true, &newArray);
	_vm->writeVar(108, newArray);

	_vm->writeVar(109, 0);
	// Write player name.
	writeStringArray(109, playerInfo[0]->asString());
	for (uint i = 1; i < 7; ++i) {
		_vm->writeArray(108, 0, i - 1, (int32)playerInfo[i]->asIntegerNumber());
	}
}

void BYOnline::handleGamesPlaying(int games) {
	if (!_inGame)
		_gamesPlaying = games;
}

void BYOnline::setPhoneStatus(int status) {
	if (!connected()) {
		warning("BYOnline: Tried to set phone status without connecting to server first!");
		return;
	}

	Common::JSONObject phoneStatus;
	phoneStatus.setVal("cmd", new Common::JSONValue("set_phone_status"));
	phoneStatus.setVal("status", new Common::JSONValue((long long int)status));
	send(phoneStatus);
}

void BYOnline::challengePlayer(int32 playerId, int32 stadium) {
	if (!connected()) {
		warning("BYOnline: Tried to challenge player without connecting to server first!");
		return;
	}

	Common::JSONObject challengePlayerRequest;
	challengePlayerRequest.setVal("cmd", new Common::JSONValue("challenge_player"));
	challengePlayerRequest.setVal("user", new Common::JSONValue((long long int)playerId));
	challengePlayerRequest.setVal("stadium", new Common::JSONValue((long long int)stadium));
	send(challengePlayerRequest);
}

void BYOnline::handleReceiveChallenge(int playerId, int stadium, Common::String name) {
	int args[25];
	memset(args, 0, sizeof(args));

	writeStringArray(0, name);

	// Setup the arguments
	args[0] = OP_REMOTE_RECEIVE_CHALLENGE;
	args[1] = playerId;
	args[2] = stadium;
	args[3] = _vm->VAR(0);

	// Run the script
	runRemoteStartScript(args);
}

void BYOnline::challengeTimeout(int playerId) {
	if (!connected()) {
		warning("BYOnline: Tried to timeout challenge without connecting to server first!");
		return;
	}

	Common::JSONObject challengeTimeoutRequuest;
	challengeTimeoutRequuest.setVal("cmd", new Common::JSONValue("challenge_timeout"));
	challengeTimeoutRequuest.setVal("user", new Common::JSONValue((long long int)playerId));
	send(challengeTimeoutRequuest);
}

void BYOnline::sendBusy(int playerId) {
	if (!connected()) {
		return;
	}

	Common::JSONObject busyRequest;
	busyRequest.setVal("cmd", new Common::JSONValue("receiver_busy"));
	busyRequest.setVal("user", new Common::JSONValue((long long int)playerId));
	send(busyRequest);
}

void BYOnline::handleReceiverBusy() {
	int args[25];
	memset(args, 0, sizeof(args));

	// Setup the arguments
	args[0] = OP_REMOTE_OPPONENT_BUSY;

	// Run the script
	runRemoteStartScript(args);
}

int32 BYOnline::answerPhone(int playerId) {
	if (!connected()) {
		warning("BYOnline: Tried to answer phone without connecting to server first!");
		return 0;
	}

	Common::JSONObject answerPhoneRequest;
	answerPhoneRequest.setVal("cmd", new Common::JSONValue("considering_challenge"));
	answerPhoneRequest.setVal("user", new Common::JSONValue((long long int)playerId));
	send(answerPhoneRequest);

	if (_playersList.size()) {
		for (uint i = 0; i < _playersList.size(); i++) {
			Common::JSONArray playerInfo = _playersList[i]->asArray();
			if ((int)playerInfo[1]->asIntegerNumber() == playerId) {
				// Write player name.
				writeStringArray(109, playerInfo[0]->asString());
				return 1;
			}
		}
	}
	return 0;
}

void BYOnline::handleConsideringChallenge() {
	int args[25];
	memset(args, 0, sizeof(args));

	// Setup the arguments
	args[0] = OP_REMOTE_OPPONENT_ANSWERS;

	// Run the script
	runRemoteStartScript(args);
}

void BYOnline::counterChallenge(int stadium) {
	if (!connected()) {
		warning("BYOnline: Tried to counter challenge without connecting to server first!");
		return;
	}

	Common::JSONObject counterChallengeRequest;
	counterChallengeRequest.setVal("cmd", new Common::JSONValue("counter_challenge"));
	counterChallengeRequest.setVal("stadium", new Common::JSONValue((long long int)stadium));
	send(counterChallengeRequest);
}

void BYOnline::handleCounterChallenge(int stadium) {
	int args[25];
	memset(args, 0, sizeof(args));

	// Setup the arguments
	args[0] = OP_REMOTE_COUNTER_CHALLENGE;
	args[1] = stadium;

	// Run the script
	runRemoteStartScript(args);
}

void BYOnline::declineChallenge(int playerId) {
	if (!connected()) {
		warning("BYOnline: Tried to decline challenge without connecting to server first!");
		return;
	}

	Common::JSONObject declineChallengeRequest;
	declineChallengeRequest.setVal("cmd", new Common::JSONValue("decline_challenge"));
	declineChallengeRequest.setVal("user", new Common::JSONValue((long long int)playerId));
	send(declineChallengeRequest);
}

void BYOnline::handleDeclineChallenge(int notResponding) {
	int args[25];
	memset(args, 0, sizeof(args));

	// Setup the arguments
	args[0] = OP_REMOTE_OPPONENT_DECLINES;
	args[1] = notResponding;

	// Run the script
	runRemoteStartScript(args);
}

void BYOnline::acceptChallenge(int playerId) {
	if (!connected()) {
		warning("BYOnline: Tried to accept challenge without connecting to server first!");
		return;
	}

	_playerId = playerId;

	Common::JSONObject acceptChallengeRequest;
	acceptChallengeRequest.setVal("cmd", new Common::JSONValue("accept_challenge"));
	acceptChallengeRequest.setVal("user", new Common::JSONValue((long long int)playerId));
	send(acceptChallengeRequest);
}

void BYOnline::handleAcceptChallenge() {
	int args[25];
	memset(args, 0, sizeof(args));

	// Setup the arguments
	args[0] = OP_REMOTE_OPPONENT_ACCEPTS;

	// Run the script
	runRemoteStartScript(args);
}

void BYOnline::startHostingGame(int playerId) {
	if (!connected()) {
		warning("BYOnline: Tried to host game without connecting to server first!");
		return;
	}

	_playerId = playerId;
	_vm->writeVar(111, 0); // Preparing

	_vm->_directPlay->hostSession(_userId);
}

void BYOnline::sendSession(int sessionId) {
	if (!connected()) {
		warning("BYOnline: Tried to send session without connecting to server first!");
		return;
	}

	Common::JSONObject sendSessionRequest;
	sendSessionRequest.setVal("cmd", new Common::JSONValue("send_session"));
	sendSessionRequest.setVal("user", new Common::JSONValue((long long int)_playerId));
	sendSessionRequest.setVal("session", new Common::JSONValue((long long int)sessionId));
	send(sendSessionRequest);
}

void BYOnline::handleGameSession(int sessionId) {
	_inGame = true;
	_vm->_directPlay->joinSession(_userId, sessionId);
}

void BYOnline::sendRelay(int relayId) {
	if (!connected()) {
		warning("BYOnline: Tried to send relay without connecting to server first!");
		return;
	}

	Common::JSONObject sendRelayRequest;
	sendRelayRequest.setVal("cmd", new Common::JSONValue("send_relay"));
	sendRelayRequest.setVal("user", new Common::JSONValue((long long int)_playerId));
	sendRelayRequest.setVal("relay", new Common::JSONValue((long long int)relayId));
	send(sendRelayRequest);
}

void BYOnline::handleGameRelay(int relayId) {
	_inGame = true;
	_vm->_directPlay->joinRelay(relayId);
}

void BYOnline::handleHostGameResp(int resp) {
	if (resp == 99) {
		_inGame = true;
	}
	_vm->writeVar(111, resp);
}

void BYOnline::connectedToSession() {
	_inGame = true;

	int args[25];
	memset(args, 0, sizeof(args));

	// Setup the arguments
	args[0] = OP_REMOTE_START_CONNECTION;

	// Run the script
	runRemoteStartScript(args);
}

void BYOnline::gameStarted(int hoster, int player, int playerNameArray) {
	if (_vm->_game.id == GID_BASEBALL2001 && _vm->readVar(399) == 1 && _vm->readVar(686) == 1) {  // Only if we're online and in Prince Rupert
		// Request teams for this client and opponent
		Common::JSONObject getTeamsRequest;
		getTeamsRequest.setVal("cmd", new Common::JSONValue("get_teams"));
		getTeamsRequest.setVal("opponent_id", new Common::JSONValue((long long int)player));
		send(getTeamsRequest);
	}

	char playerName[16];
	_vm->getStringFromArray(playerNameArray, playerName, sizeof(playerName));

	if (hoster != _userId) {
		warning("BYOnline: Got game started op but the hoster wasn't us!");
		return;
	}

	Common::JSONObject gameStartedRequest;
	gameStartedRequest.setVal("cmd", new Common::JSONValue("game_started"));
	gameStartedRequest.setVal("user", new Common::JSONValue((long long int)player));

	send(gameStartedRequest);
}

void BYOnline::gameFinished() {
	_inGame = false;

	Common::JSONObject gameFinishedRequest;
	gameFinishedRequest.setVal("cmd", new Common::JSONValue("game_finished"));

	send(gameFinishedRequest);
}

}
