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

#include "common/config-manager.h"

#include "scumm/he/intern_he.h"
#include "scumm/he/net/net_lobby.h"
#include "scumm/he/net/net_defines.h"

namespace Scumm {

Lobby::Lobby(ScummEngine_v90he *vm) : _vm(vm) {
	_gameName = _vm->_game.gameid;
	if (_gameName == "baseball2001")
		_gameName = "baseball";
	_socket = nullptr;

	_userId = 0;
	_userName = "";
	_playerId = 0;

	_areaIdForPopulation = 0;

	_inArea = false;
	_gamesPlaying = 0;

	_sessionId = 0;

	_inGame = false;
}

Lobby::~Lobby() {
	if (_socket)
		disconnect();
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

	if (_inArea && !_inGame) {
		// Update games playing
		_vm->writeVar(110, _gamesPlaying);
	}
}

void Lobby::send(Common::JSONObject &data) {
	if (!_socket) {
		warning("LOBBY: Attempted to send data while not connected to server");
		return;
	}
	Common::JSONValue value(data);
	Common::String valueString = Common::JSON::stringify(&value);
	// Add new line.
	valueString += "\n";

	debugC(DEBUG_NETWORK, "LOBBY: Sending data: %s", valueString.c_str());
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
	debugC(DEBUG_NETWORK, "LOBBY: Received Data: %s", line.c_str());
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
		} else if (command == "disconnect") {
			int type = root["type"]->asIntegerNumber();
			Common::String message = root["message"]->asString();
			systemAlert(type, message);
			disconnect();
		} else if (command == "login_resp") {
			int errorCode = root["error_code"]->asIntegerNumber();
			int userId = root["id"]->asIntegerNumber();
			Common::String sessionServer = root["sessionServer"]->asString();
			Common::String response = root["response"]->asString();
			handleLoginResp(errorCode, userId, sessionServer, response);
		} else if (command == "profile_info") {
			Common::JSONArray profile = root["profile"]->asArray();
			handleProfileInfo(profile);
		} else if (command == "file_data") {
			Common::String filename = root["filename"]->asString();
			Common::String data = root["data"]->asString();
			handleFileData(filename, data);
		} else if (command == "system_alert") {
			int type = root["type"]->asIntegerNumber();
			Common::String message = root["message"]->asString();
			systemAlert(type, message);
		} else if (command == "population_resp") {
			int areaId = root["area"]->asIntegerNumber();
			int population = root["population"]->asIntegerNumber();
			handlePopulation(areaId, population);
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
		} else if (command == "ping_result") {
			int result = root["result"]->asIntegerNumber();
			handlePingResult(result);
		} else if (command == "receive_challenge") {
			int user = root["user"]->asIntegerNumber();
			int stadium = root["stadium"]->asIntegerNumber();
			Common::String name = root["name"]->asString();
			handleReceiveChallenge(user, stadium, name);
		} else if (command == "receiver_busy") {
			handleReceiverBusy();
		} else if (command == "considering_challenge") {
			handleConsideringChallenge();
		} else if (command == "counter_challenge") {
			int stadium = root["stadium"]->asIntegerNumber();
			handleCounterChallenge(stadium);
		} else if (command == "decline_challenge") {
			int notResponding = root["not_responding"]->asIntegerNumber();
			handleDeclineChallenge(notResponding);
		} else if (command == "accept_challenge") {
			handleAcceptChallenge();
		} else if (command == "game_session") {
			int session = root["session"]->asIntegerNumber();
			handleGameSession(session);
		} else if (command == "teams") {
			int error = root["error"]->asIntegerNumber();
			Common::String message = root["message"]->asString();
			Common::JSONArray userTeam = root["user"]->asArray();
			Common::JSONArray opponentTeam = root["opponent"]->asArray();
			handleTeams(userTeam, opponentTeam, error, message);
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
		char userName[MAX_USER_NAME];
		char password[MAX_USER_NAME];

		_vm->getStringFromArray(args[0], userName, sizeof(userName));
		_vm->getStringFromArray(args[1], password, sizeof(password));

		login(userName, password);
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
	case OP_NET_PING_OPPONENT:
		// NOTE: See getUserProfile, this op only gets
		// called after an opponent picks up the phone.
		break;
	case OP_NET_RECEIVER_BUSY:
		sendBusy(args[0]);
		break;
	case OP_NET_COUNTER_CHALLENGE:
		counterChallenge(args[0]);
		break;
	case OP_NET_GET_PROFILE:
		getUserProfile(args[0]);
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
	case OP_NET_SET_POLL_ANSWER:
		setPollAnswer(args[0]);
		break;
	case OP_NET_UNKNOWN_2229:
		// TODO
		break;
	case OP_NET_CHANGE_ICON:
		setIcon(args[0]);
		break;
	case OP_NET_SET_PHONE_STATUS:
		setPhoneStatus(args[0]);
		break;
	case OP_NET_ANSWER_PHONE:
		res = answerPhone(args[0]);
		break;
	case OP_NET_DOWNLOAD_FILE:
		char downloadPath[MAX_USER_NAME];
		char filename[MAX_USER_NAME];

		_vm->getStringFromArray(args[0], downloadPath, sizeof(downloadPath));
		_vm->getStringFromArray(args[1], filename, sizeof(filename));

		downloadFile(downloadPath, filename);
		break;
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
	debugC(DEBUG_NETWORK, "LOBBY: openURL: %s", url);
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

	debugC(DEBUG_NETWORK, "LOBBY: Connecting to %s", lobbyUrl.c_str());

	if (_socket->connect(lobbyUrl)) {
		debugC(DEBUG_NETWORK, "LOBBY: Successfully connected to %s", lobbyUrl.c_str());
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
		debugC(DEBUG_NETWORK, "LOBBY: Disconnecting connection to server.");
		Common::JSONObject disconnectObject;
		disconnectObject.setVal("cmd", new Common::JSONValue("disconnect"));
		send(disconnectObject);
	} else {
		systemAlert(901, "You have been disconnected from our server. Returning to login screen.");
	}

	if (ConfMan.getBool("enable_competitive_mods"))
		// set var747 (custom teams status) to 0
		_vm->writeVar(747, 0);

	delete _socket;
	_socket = nullptr;

	_userId = 0;
	_userName = "";
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
	_userName = userName;
	Common::JSONObject loginRequestParameters;
	loginRequestParameters.setVal("cmd", new Common::JSONValue("login"));
	loginRequestParameters.setVal("user", new Common::JSONValue(_userName));
	loginRequestParameters.setVal("pass", new Common::JSONValue((Common::String)password));
	loginRequestParameters.setVal("game", new Common::JSONValue((Common::String)_gameName));
	loginRequestParameters.setVal("version", new Common::JSONValue(NETWORK_VERSION));
	loginRequestParameters.setVal("competitive_mods", new Common::JSONValue(ConfMan.getBool("enable_competitive_mods")));

	send(loginRequestParameters);
}

void Lobby::handleLoginResp(int errorCode, int userId, Common::String sessionServer, Common::String response) {
	if (errorCode > 0) {
		writeStringArray(109, response);
		_vm->writeVar(108, -99);
		disconnect();
		return;
	}
	_userId = userId;
	_vm->_net->setSessionServer(sessionServer);
	_vm->writeVar(108, 99);
}

void Lobby::getUserProfile(int userId) {
	if (!_socket)
		return;

	Common::JSONObject getProfileRequest;
	getProfileRequest.setVal("cmd", new Common::JSONValue("get_profile"));
	if (userId) {
		getProfileRequest.setVal("user_id", new Common::JSONValue((long long int)userId));
		if (ConfMan.getBool("enable_competitive_mods") && _vm->_game.id == GID_BASEBALL2001) {
			// NOTE: Since there are isn't any way to reliably ping the player (since in-game
			// communication takes place after accepting a challenge), we are substituting
			// the functionality to deterimine whether the opponent has competitive mods
			// enabled or not.
			pingPlayer(userId);
		}
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

void Lobby::handleTeams(Common::JSONArray userTeam, Common::JSONArray opponentTeam, int error, Common::String message) {
	if (ConfMan.getBool("enable_competitive_mods")) {
		if (error == 1) {
			warning("LOBBY: Unable to retrieve custom teams: %s", message.c_str());
			_vm->writeVar(747, 0);
			return;
		}
		// We're going to store our team in array 748, which seems to be otherwise unused
		// Then we'll pull from that array as needed later
		int userTeamArray = 0;
		_vm->defineArray(748, ScummEngine_v90he::kIntArray, 0, 0, 0, userTeam.size(), true, &userTeamArray);
		_vm->writeVar(748, userTeamArray);

		for (uint i = 0; i < userTeam.size(); i++) {
			if (userTeam[i]->isIntegerNumber()) {
				_vm->writeArray(748, 0, i, userTeam[i]->asIntegerNumber());
			} else {
				warning("LOBBY: Value for user team index %d is not an integer!", i);
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
				warning("LOBBY: Value for opponent team index %d is not an integer!", i);
			}
		}

		// Write a one to var747 to indicate that Prince Rupert teams should be pulled from arrays 748 and 749
		_vm->writeVar(747, 1);
	}
}

void Lobby::downloadFile(const char *downloadPath, const char *filename) {
	if (!_socket)
		return;

	Common::JSONObject downloadRequest;
	downloadRequest.setVal("cmd", new Common::JSONValue("download_file"));
	downloadRequest.setVal("filename", new Common::JSONValue((Common::String)filename));
	send(downloadRequest);
}

void Lobby::handleFileData(Common::String filename, Common::String data) {
	if (data.size()) {
		Common::OutSaveFile file = _vm->_saveFileMan->openForSaving(_vm->_targetName + '-' + filename);
		file.write(data.c_str(), data.size());
		file.finalize();
	}
	_vm->writeVar(135, 1);
}

void Lobby::setIcon(int icon) {
	if (!_socket)
		return;

	Common::JSONObject setIconRequest;
	setIconRequest.setVal("cmd", new Common::JSONValue("set_icon"));
	setIconRequest.setVal("icon", new Common::JSONValue((long long int)icon));
	send(setIconRequest);
}

void Lobby::setPollAnswer(int pollAnswer) {
	if (!_socket)
		return;

	Common::JSONObject setPollAnswerRequest;
	setPollAnswerRequest.setVal("cmd", new Common::JSONValue("set_poll_answer"));
	setPollAnswerRequest.setVal("answer", new Common::JSONValue((long long int)pollAnswer));
	send(setPollAnswerRequest);
}

void Lobby::sendGameResults(int userId, int arrayIndex, int lastFlag) {
	if (!_socket) {
		return;
	}

	// Football does not use the lastFlag argument (seems to be implemented in a
	// later patch?), so let's force set it to true.  This is safe because the game
	// only calls it once after a game has finished.
	if (_gameName == "football")
		lastFlag = 1;

	// Because the new netcode uses userIds 1 and 2 to determine between
	// host and opponent, we need to replace it to represent the correct user.
	if (userId == 1)
		userId = _userId;
	else
		userId = _playerId;

	Common::JSONObject setProfileRequest;
	setProfileRequest.setVal("cmd", new Common::JSONValue("game_results"));
	setProfileRequest.setVal("user", new Common::JSONValue((long long int)userId));

	ScummEngine_v90he::ArrayHeader *ah = (ScummEngine_v90he::ArrayHeader *)_vm->getResourceAddress(rtString, arrayIndex & ~MAGIC_ARRAY_NUMBER);
	int32 size = (FROM_LE_32(ah->acrossMax) - FROM_LE_32(ah->acrossMin) + 1) *
		(FROM_LE_32(ah->downMax) - FROM_LE_32(ah->downMin) + 1);

	Common::JSONArray arrayData;
	for (int i = 0; i < size; i++) {
		// Assuming they're dword type
		int32 data = (int32)READ_LE_UINT32(ah->data + i * 4);
		arrayData.push_back(new Common::JSONValue((long long int)data));
	}
	setProfileRequest.setVal("fields", new Common::JSONValue(arrayData));
	setProfileRequest.setVal("last", new Common::JSONValue((bool)lastFlag));
	send(setProfileRequest);
}

void Lobby::getPopulation(int areaId, int unknown) {
	_areaIdForPopulation = areaId;

	Common::JSONObject getPopulationRequest;
	getPopulationRequest.setVal("cmd", new Common::JSONValue("get_population"));
	getPopulationRequest.setVal("area", new Common::JSONValue((long long int)areaId));
	send(getPopulationRequest);
}

void Lobby::handlePopulation(int areaId, int population) {
		if (areaId == _areaIdForPopulation) {
			_vm->writeVar(((_vm->_game.id == GID_FOOTBALL) ? 108 : 136), population + 1); // Game deducts the one
			_areaIdForPopulation = 0;
		}
}

void Lobby::locatePlayer(int usernameArray) {
	if (!_socket) {
		return;
	}

	char userName[MAX_USER_NAME];
	_vm->getStringFromArray(usernameArray, userName, sizeof(userName));

	Common::JSONObject locatePlayerRequest;
	locatePlayerRequest.setVal("cmd", new Common::JSONValue("locate_player"));
	locatePlayerRequest.setVal("user", new Common::JSONValue((Common::String)userName));
	send(locatePlayerRequest);
}

void Lobby::handleLocateResp(int code, int areaId, Common::String area) {
	_vm->writeVar(108, 1);
	_vm->writeVar(110, code);
	if (code == 1) {
		writeStringArray(109, area);
		_vm->writeVar(111, areaId);
	}
}

void Lobby::enterArea(int32 areaId) {
	if (!areaId) {
		warning("Backyard Online (enterArea): Missing area id!");
		return;
	}
	if (!_socket) {
		warning("LOBBY: Tried to enter area %d without connecting to server first!", (int)areaId);
		return;
	}

	// HACK: After you log in on Baseball, you would join this "lobby area".
	// My best guess is so you could potationally chat with other players, but
	// unsure if that's actually implemented in-game, so let's just ignore
	// that for now.
	if (_vm->_game.id == GID_BASEBALL2001 && areaId == 33) {
		return;
	}

	// Bugfix: If a single-player game is played with pitch locator on, it
	// remains on for a subsequently played online game even though the areas'
	// stated rules do not allow it. Here we fix this bug/exploit by writing to
	// the variable that determines whether to use pitch locator
	if (_vm->_game.id == GID_BASEBALL2001) {
		_vm->writeVar(440, 0);
	}

	debugC(DEBUG_NETWORK, "LOBBY: Entering area %d", int(areaId));

	Common::JSONObject enterAreaRequest;
	enterAreaRequest.setVal("cmd", new Common::JSONValue("enter_area"));
	enterAreaRequest.setVal("area", new Common::JSONValue((long long int)areaId));
	send(enterAreaRequest);

	_inArea = true;
}

void Lobby::leaveArea() {
	debugC(DEBUG_NETWORK, "LOBBY: Leaving area.");
	_playersList.clear();

	if (_socket) {
		Common::JSONObject leaveAreaRequest;
		leaveAreaRequest.setVal("cmd", new Common::JSONValue("leave_area"));
		send(leaveAreaRequest);

		_inArea = false;
	}
}

void Lobby::getPlayersList(int start, int end) {
	if (!_socket) {
		warning("LOBBY: Tried to fetch players list without connecting to server first!");
		return;
	}

	Common::JSONObject playersListRequest;
	playersListRequest.setVal("cmd", new Common::JSONValue("get_players"));
	playersListRequest.setVal("start", new Common::JSONValue((long long int)start));
	playersListRequest.setVal("end", new Common::JSONValue((long long int)end));
	send(playersListRequest);

}

bool Lobby::_checkPlayersLists(Common::JSONArray other) {
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

void Lobby::handlePlayersList(Common::JSONArray playersList) {
	// If the list exactly the same as before, don't do anything.
	if (_checkPlayersLists(playersList)) {
		_playersList = playersList;
		if (!_inGame) {
			// Tell the game to redisplay the list.
			_vm->writeVar(115, 2);
		}
	}
}

void Lobby::getPlayerInfo(int32 idx) {
	if ((uint)idx - 1 > _playersList.size()) {
		warning("LOBBY: _playersList is too small for index. (%d > %d)", (int)idx, (int)_playersList.size());
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

void Lobby::handleGamesPlaying(int games) {
	if (!_inGame)
		_gamesPlaying = games;
}

void Lobby::setPhoneStatus(int status) {
	if (!_socket) {
		return;
	}

	Common::JSONObject phoneStatus;
	phoneStatus.setVal("cmd", new Common::JSONValue("set_phone_status"));
	phoneStatus.setVal("status", new Common::JSONValue((long long int)status));
	send(phoneStatus);
}

void Lobby::challengePlayer(int32 playerId, int32 stadium) {
	if (!_socket) {
		warning("LOBBY: Tried to challenge player without connecting to server first!");
		return;
	}

	Common::JSONObject challengePlayerRequest;
	challengePlayerRequest.setVal("cmd", new Common::JSONValue("challenge_player"));
	challengePlayerRequest.setVal("user", new Common::JSONValue((long long int)playerId));
	challengePlayerRequest.setVal("stadium", new Common::JSONValue((long long int)stadium));
	send(challengePlayerRequest);
}

void Lobby::handleReceiveChallenge(int playerId, int stadium, Common::String name) {
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

void Lobby::challengeTimeout(int playerId) {
	if (!_socket) {
		warning("LOBBY: Tried to timeout challenge without connecting to server first!");
		return;
	}

	Common::JSONObject challengeTimeoutRequuest;
	challengeTimeoutRequuest.setVal("cmd", new Common::JSONValue("challenge_timeout"));
	challengeTimeoutRequuest.setVal("user", new Common::JSONValue((long long int)playerId));
	send(challengeTimeoutRequuest);
}

void Lobby::sendBusy(int playerId) {
	if (!_socket) {
		return;
	}

	Common::JSONObject busyRequest;
	busyRequest.setVal("cmd", new Common::JSONValue("receiver_busy"));
	busyRequest.setVal("user", new Common::JSONValue((long long int)playerId));
	send(busyRequest);
}

void Lobby::handleReceiverBusy() {
	int args[25];
	memset(args, 0, sizeof(args));

	// Setup the arguments
	args[0] = OP_REMOTE_OPPONENT_BUSY;

	// Run the script
	runRemoteStartScript(args);
}

void Lobby::pingPlayer(int playerId) {
	Common::JSONObject pingRequest;
	pingRequest.setVal("cmd", new Common::JSONValue("ping_player"));
	pingRequest.setVal("user", new Common::JSONValue((long long int)playerId));
	send(pingRequest);
}

void Lobby::handlePingResult(int result) {
	int args[25];
	memset(args, 0, sizeof(args));

	// Setup the arguments
	args[0] = OP_REMOTE_PING_TEST_RESULT;
	args[1] = result;

	// Run the script
	runRemoteStartScript(args);
}

int32 Lobby::answerPhone(int playerId) {
	if (!_socket) {
		warning("LOBBY: Tried to answer phone without connecting to server first!");
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

void Lobby::handleConsideringChallenge() {
	int args[25];
	memset(args, 0, sizeof(args));

	// Setup the arguments
	args[0] = OP_REMOTE_OPPONENT_ANSWERS;

	// Run the script
	runRemoteStartScript(args);
}

void Lobby::counterChallenge(int stadium) {
	if (!_socket) {
		warning("LOBBY: Tried to counter challenge without connecting to server first!");
		return;
	}

	Common::JSONObject counterChallengeRequest;
	counterChallengeRequest.setVal("cmd", new Common::JSONValue("counter_challenge"));
	counterChallengeRequest.setVal("stadium", new Common::JSONValue((long long int)stadium));
	send(counterChallengeRequest);
}

void Lobby::handleCounterChallenge(int stadium) {
	int args[25];
	memset(args, 0, sizeof(args));

	// Setup the arguments
	args[0] = OP_REMOTE_COUNTER_CHALLENGE;
	args[1] = stadium;

	// Run the script
	runRemoteStartScript(args);
}

void Lobby::declineChallenge(int playerId) {
	if (!_socket) {
		warning("LOBBY: Tried to decline challenge without connecting to server first!");
		return;
	}

	Common::JSONObject declineChallengeRequest;
	declineChallengeRequest.setVal("cmd", new Common::JSONValue("decline_challenge"));
	declineChallengeRequest.setVal("user", new Common::JSONValue((long long int)playerId));
	send(declineChallengeRequest);
}

void Lobby::handleDeclineChallenge(int notResponding) {
	int args[25];
	memset(args, 0, sizeof(args));

	// Setup the arguments
	args[0] = OP_REMOTE_OPPONENT_DECLINES;
	args[1] = notResponding;

	// Run the script
	runRemoteStartScript(args);
}

void Lobby::acceptChallenge(int playerId) {
	if (!_socket) {
		warning("LOBBY: Tried to accept challenge without connecting to server first!");
		return;
	}

	_playerId = playerId;

	Common::JSONObject acceptChallengeRequest;
	acceptChallengeRequest.setVal("cmd", new Common::JSONValue("accept_challenge"));
	acceptChallengeRequest.setVal("user", new Common::JSONValue((long long int)playerId));
	send(acceptChallengeRequest);

	if (ConfMan.getBool("enable_competitive_mods")) {
		if (_vm->_game.id == GID_BASEBALL2001 && _vm->readVar(559) == 19) {  // Only if in Prince Rupert
			// Request teams for this client and opponent
			Common::JSONObject getTeamsRequest;
			getTeamsRequest.setVal("cmd", new Common::JSONValue("get_teams"));
			getTeamsRequest.setVal("opponent_id", new Common::JSONValue((long long int)playerId));
			send(getTeamsRequest);
		}
	}
}

void Lobby::handleAcceptChallenge() {
	int args[25];
	memset(args, 0, sizeof(args));

	// Setup the arguments
	args[0] = OP_REMOTE_OPPONENT_ACCEPTS;

	// Run the script
	runRemoteStartScript(args);
}

void Lobby::startHostingGame(int playerId) {
	if (!_socket)
		return;

	_playerId = playerId;
	_vm->writeVar(111, 0);

	// Create ENet instance.
	if (!_vm->_net->initProvider()) {
		// Tell the game that hosting has failed.
		_vm->writeVar(111, 1);
		return;
	}

	// TODO: Actual session name.
	if (_vm->_net->hostGame(const_cast<char *>(_userName.c_str()), const_cast<char *>(_userName.c_str()))) {
		// Wait till the session server assigns us a session id.
		uint tickCount = 0;
		while (_vm->_net->_sessionId == -1) {
			_vm->_net->doNetworkOnceAFrame(12);
			tickCount += 5;
			g_system->delayMillis(5);
			if (tickCount >= 5000)
				break;
		}
		int sessionId = _vm->_net->_sessionId;
		if (sessionId > 0) {
			_inGame = true;
			// Send our session over to our opponent.
			Common::JSONObject sendSessionRequest;
			sendSessionRequest.setVal("cmd", new Common::JSONValue("send_session"));
			sendSessionRequest.setVal("user", new Common::JSONValue((long long int)_playerId));
			sendSessionRequest.setVal("session", new Common::JSONValue((long long int)sessionId));
			send(sendSessionRequest);

			// Tell the game that we're hosting.
			_vm->writeVar(111, 99);
		} else
			// Tell the game that hosting has failed.
			_vm->writeVar(111, 1);
	} else
		// Tell the game that hosting has failed.
		_vm->writeVar(111, 1);
}

void Lobby::handleGameSession(int sessionId) {
	_sessionId = sessionId;
	_inGame = true;

	if (_vm->_net->initProvider()) {
		// Tell the game to start connecting to our host.
		int args[25];
		memset(args, 0, sizeof(args));

		// Setup the arguments
		args[0] = OP_REMOTE_START_CONNECTION;

		// Run the script
		runRemoteStartScript(args);
	}
}

void Lobby::gameStarted(int hoster, int player, int playerNameArray) {
	// NOTE: Due to how the new netcode works, hoster always returns 1 and player always returns 2.
	// This will break things if the actual host and opponent ids are not 1 and 2 respectively,
	// so we're using the stored variables here instead of what the game gave us.
	if (ConfMan.getBool("enable_competitive_mods")) {
		// Only if we're online and in Prince Rupert
		if (_vm->_game.id == GID_BASEBALL2001 && _vm->readVar(399) == 1 && _vm->readVar(686) == 1) {
			// Request teams for this client and opponent
			Common::JSONObject getTeamsRequest;
			getTeamsRequest.setVal("cmd", new Common::JSONValue("get_teams"));
			getTeamsRequest.setVal("opponent_id", new Common::JSONValue((long long int)_playerId));
			send(getTeamsRequest);
		}
	}

	char playerName[16];
	_vm->getStringFromArray(playerNameArray, playerName, sizeof(playerName));

	// Don't accept anymore sessions.
	_vm->_net->disableSessionJoining();

	Common::JSONObject gameStartedRequest;
	gameStartedRequest.setVal("cmd", new Common::JSONValue("game_started"));
	gameStartedRequest.setVal("user", new Common::JSONValue((long long int)_playerId));

	send(gameStartedRequest);
}

void Lobby::gameFinished() {
	_inGame = false;
	_vm->_net->closeProvider();

	// Bugfix: After finishing a game on an area with power ups disabled, the variable
	// for it (689) does not reset. This causes offline games to play without power ups at all,
	// so let's reset it ourselves.
	if (_vm->_game.id == GID_BASEBALL2001)
		_vm->writeVar(689, 0);

	Common::JSONObject gameFinishedRequest;
	gameFinishedRequest.setVal("cmd", new Common::JSONValue("game_finished"));

	send(gameFinishedRequest);
}

} // End of namespace Scumm
