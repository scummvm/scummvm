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

#include "scumm/he/intern_he.h"
#include "scumm/he/net/net_main.h"
#include "scumm/he/net/net_defines.h"

namespace Scumm {

Net::Net(ScummEngine_v90he *vm) : _latencyTime(1), _fakeLatency(false), _vm(vm) {
	//some defaults for fields

	_gameName = _vm->_game.gameid;
	_gameVersion = "";
	if (_vm->_game.variant)
		_gameVersion = _vm->_game.variant; // 1.0/1.1/Demo

	_tmpbuffer = (byte *)malloc(MAX_PACKET_SIZE);

	_enet = nullptr;

	_sessionHost = nullptr;
	_broadcastSocket = nullptr;

	_sessionServerPeer = -1;
	_sessionServerHost = nullptr;
	_isRelayingGame = false;

	_numUsers = 0;
	_numBots = 0;

	_maxPlayers = 2;
	if (_gameName == "moonbase")
		_maxPlayers = 4;

	_userIdCounter = 0;

	_myUserId = -1;
	_fromUserId = -1;

	_sessionId = -1;
	_isHost = false;
	_isShuttingDown = false;
	_sessionName = Common::String();
	_sessions = Common::Array<Session>();

	_hostPort = 0;

	_hostDataQueue = Common::Queue<Common::JSONValue *>();
	_peerIndexQueue = Common::Queue<int>();
}

Net::~Net() {
	free(_tmpbuffer);
	closeProvider();
}

Net::Address Net::getAddressFromString(Common::String addressStr) {
	Address address;
	int portPos = addressStr.findFirstOf(":");
	if (portPos > -1) {
		address.port = atoi(addressStr.substr(portPos + 1).c_str());
		address.host = addressStr.substr(0, portPos);
	} else {
		// Assume that the string has no port defined.
		address.host = addressStr;
		address.port = 0;
	}
	return address;
}

Common::String Net::getStringFromAddress(Address address) {
	return Common::String::format("%s:%d", address.host.c_str(), address.port);
}

int Net::hostGame(char *sessionName, char *userName) {
	if (createSession(sessionName)) {
		if (addUser(userName, userName)) {
			_myUserId = _userIdCounter;
			_userIdToPeerIndex[_myUserId] = -1;
			return 1;
		} else {
			_vm->displayMessage(0, "Error Adding User \"%s\" to Session \"%s\"", userName, sessionName);
			endSession();
			closeProvider();
		}
	} else {
		_vm->displayMessage(0, "Error creating session \"%s\"", userName );

		closeProvider();
	}

	return 0;
}

int Net::joinGame(Common::String IP, char *userName) {
	// This gets called when attempting to join with the --join-game command line param.
	debug(1, "Net::joinGame(\"%s\", \"%s\")", IP.c_str(), userName); // PN_JoinTCPIPGame
	Address address = getAddressFromString(IP);

	bool isLocal = false;
	// TODO: 20-bit block address (172.16.0.0 – 172.31.255.255)
	if (address.host == "127.0.0.1" || address.host == "localhost" || address.host == "255.255.255.255" ||
		address.host.matchString("10.*.*.*") || address.host.matchString("192.168.*.*")) {
		isLocal = true;
	}

	if (isLocal) {
		if (!address.port) {
			// Local connection with no port specified.  Send a session request to get port:
			startQuerySessions(false);
			if (!_broadcastSocket) {
				return 0;
			}

			_sessions.clear();
			_broadcastSocket->send(address.host.c_str(), 9130, "{\"cmd\": \"get_session\"}");

			uint tickCount = 0;
			while(!_sessions.size()) {
				serviceBroadcast();
				// Wait for one minute for response before giving up
				tickCount += 5;
				g_system->delayMillis(5);
				if (tickCount >= 1000)
					break;
			}

			if (!_sessions.size())
				return 0;

			if (address.host == "255.255.255.255")
				address.host = _sessions[0].host;
			address.port = _sessions[0].port;
			stopQuerySessions();
		}
		// We got our address and port, attempt connection:
		if (connectToSession(address.host, address.port)) {
			// Connected, add our user.
			return addUser(userName, userName);
		} else {
			warning("NETWORK: Failed to connect to %s:%d", address.host.c_str(), address.port);
		}
	} else {
		warning("STUB: joinGame: Public IP connection %s", address.host.c_str());
	}

	return 0;
}

bool Net::connectToSession(Common::String address, int port) {
	if (_hostPort)
		_sessionHost = _enet->connectToHost("0.0.0.0", _hostPort, address, port);
	else
		_sessionHost = _enet->connectToHost(address, port);
	if (!_sessionHost)
		return false;

	_isHost = false;
	return true;
}

int Net::addUser(char *shortName, char *longName) {
	debug(1, "Net::addUser(\"%s\", \"%s\")", shortName, longName); // PN_AddUser
	// TODO: What's the difference between shortName and longName?

	if (_isHost) {
		if (getTotalPlayers() > 4) {
			// We are full.
			return 0;
		}
		_userIdToName[++_userIdCounter] = longName;
		_numUsers++;
		if (_sessionId && _sessionServerPeer > -1) {
			// Update player count to session server {
			Common::String updatePlayers = Common::String::format(
				"{\"cmd\":\"update_players\",\"game\":\"%s\",\"version\":\"%s\",\"players\":%d}",
				_gameName.c_str(), _gameVersion.c_str(), getTotalPlayers());
			_sessionHost->send(updatePlayers.c_str(), _sessionServerPeer);
		}
		return 1;
	}

	// Client:
	if (_myUserId != -1)
		return 1;

	Common::String addUser = Common::String::format(
		"{\"cmd\":\"add_user\",\"name\":\"%s\"}", longName);

	_sessionHost->send(addUser.c_str(), 0, 0, true);

	uint tickCount = 0;
	while(_myUserId == -1) {
		remoteReceiveData(12);
		// Wait for five seconds for our user id before giving up
		tickCount += 5;
		g_system->delayMillis(5);
		if (tickCount >= 5000)
			break;
	}
	return (_myUserId > -1) ? 1 : 0;
}

int Net::removeUser() {
	debug(1, "Net::removeUser()"); // PN_RemoveUser

	if (_myUserId != -1)
		destroyPlayer(_myUserId);

	return 1;
}

int Net::whoSentThis() {
	debug(1, "Net::whoSentThis(): return %d", _fromUserId); // PN_WhoSentThis
	return _fromUserId;
}

int Net::whoAmI() {
	debug(1, "Net::whoAmI(): return %d", _myUserId); // PN_WhoAmI
	return _myUserId;
}

int Net::createSession(char *name) {
	debug(1, "Net::createSession(\"%s\")", name); // PN_CreateSession

	if (!_enet) {
		return 0;
	};

	_sessionId = -1;
	_sessionName = name;
	// Normally we would do only one peer (0) or three peers (2) but we are reserving one
	// for our connection to the session server.
	_sessionHost = _enet->createHost("0.0.0.0", 0, _maxPlayers);

	if (!_sessionHost) {
		return 0;
	}

	_isHost = true;
	// TODO: Config to enable/disable Internet sessions.
	if (_sessionHost->connectPeer("127.0.0.1", 9120)) {
		_sessionServerPeer = _sessionHost->getPeerIndexFromHost("127.0.0.1", 9120);
		// Create session to the session server.
		Common::String req = Common::String::format(
			"{\"cmd\":\"host_session\",\"game\":\"%s\",\"version\":\"%s\",\"name\":\"%s\"}",
			_gameName.c_str(), _gameVersion.c_str(), name);
		_sessionHost->send(req.c_str(), _sessionServerPeer);
	} else {
		warning("Failed to connect to session server!  This game will not be listed on the Internet");
	}

	// TODO: Config to enable/disable LAN discovery.
	_broadcastSocket = _enet->createSocket("0.0.0.0", 9130);
	if (!_broadcastSocket) {
		warning("NETWORK: Unable to create broadcast socket, your game will not be broadcast over LAN");
	}

	return 1;
}

int Net::getTotalPlayers() {
	return _numUsers + _numBots;
}

int Net::joinSession(int sessionIndex) {
	debug(1, "Net::joinSession(%d)", sessionIndex); // PN_JoinSession
	if (_sessions.empty()) {
		warning("Net::joinSession(): no sessions");
		return 0;
	}

	if (sessionIndex >= (int)_sessions.size()) {
		warning("Net::joinSession(): session number too big: %d >= %d", sessionIndex, _sessions.size());
		return 0;
	}

	Session session = _sessions[sessionIndex];
	if (!session.local && _sessionServerHost) {
		Common::String joinSession = Common::String::format(
			"{\"cmd\":\"join_session\",\"game\":\"%s\",\"version\":\"%s\",\"session\":%d}",
			_gameName.c_str(), _gameVersion.c_str(), sessionIndex);
		_sessionServerHost->send(joinSession.c_str(), 0);

		// Give the host time to hole punch us.
		g_system->delayMillis(500);
	}

	// Disconnect the session server to free up and use the same port we've connected previously.
	if (_sessionServerHost) {
		_sessionServerHost->disconnectPeer(0);
		delete _sessionServerHost;
		_sessionServerHost = nullptr;
	}

	bool success = connectToSession(session.host, session.port);
	if (!success) {
		if (!session.local) {
			// Start up a relay session with the host.

			// This will re-connect us to the session server.
			startQuerySessions();
			if (_sessionServerHost) {
				Common::String startRelay = Common::String::format(
					"{\"cmd\":\"start_relay\",\"game\":\"%s\",\"version\":\"%s\",\"session\":%d}",
					_gameName.c_str(), _gameVersion.c_str(), session.id);
				_sessionServerHost->send(startRelay.c_str(), 0);

				uint tickCount = 0;
				while(_myUserId == -1) {
					serviceSessionServer();
					// Wait for five seconds for our user id before giving up
					tickCount += 5;
					g_system->delayMillis(5);
					if (tickCount >= 5000)
						break;
				}

				if (_myUserId > -1)
					// If we have gotten our user id, that means that we are now relaying.
					return true;
			}
		}
		_vm->displayMessage(0, "Unable to join game session with address \"%s:%d\"", session.host.c_str(), session.port);
		return false;
	}

	return true;
}

int Net::endSession() {
	debug(1, "Net::endSession()"); // PN_EndSession

	if (_isHost && _hostDataQueue.size()) {
		_isShuttingDown = true;
		// Send out any remaining data from the queue before shutting down.
		while (_hostDataQueue.size()) {
			if (_hostDataQueue.size() != _hostDataQueue.size())
				warning("NETWORK: Sizes of data and peer index queues does not match!  Expect some wonky stuff");
			Common::JSONValue *json = _hostDataQueue.pop();
			int peerIndex = _peerIndexQueue.pop();
			handleGameDataHost(json, peerIndex);
		_isShuttingDown = false;
		}
	}

	if (_sessionHost && _sessionServerPeer > -1) {
		_sessionHost->disconnectPeer(_sessionServerPeer);
		_sessionServerPeer = -1;
	}

	if (_sessionHost) {
		delete _sessionHost;
		_sessionHost = nullptr;
	}
	if (_sessionServerHost) {
		_sessionServerHost->disconnectPeer(0);
		delete _sessionServerHost;
		_sessionServerHost = nullptr;
	}
	if (_broadcastSocket) {
		delete _broadcastSocket;
		_broadcastSocket = nullptr;
	}

	_hostPort = 0;

	_numUsers = 0;
	_numBots = 0;

	_userIdCounter = 0;
	_userIdToName.clear();
	_userIdToPeerIndex.clear();

	_sessionId = -1;
	_sessionName.clear();

	_myUserId = -1;
	_fromUserId = -1;

	_hostDataQueue.clear();
	_peerIndexQueue.clear();

	_isRelayingGame = false;

	return 0;
}

void Net::disableSessionJoining() {
	debug(1, "Net::disableSessionJoining()"); // PN_DisableSessionPlayerJoin
	if (_sessionHost && _sessionServerPeer > -1 && !_isRelayingGame) {
		_sessionHost->disconnectPeer(_sessionServerPeer);
		_sessionServerPeer = -1;
	}
	if (_broadcastSocket) {
		delete _broadcastSocket;
		_broadcastSocket = nullptr;
	}
}

void Net::enableSessionJoining() {
	warning("STUB: Net::enableSessionJoining()"); // PN_EnableSessionPlayerJoin
}

void Net::setBotsCount(int botsCount) {
	debug(1, "Net::setBotsCount(%d)", botsCount); // PN_SetAIPlayerCountKludge
	_numBots = botsCount;
}

int32 Net::setProviderByName(int32 parameter1, int32 parameter2) {
	// char name[MAX_PROVIDER_NAME];
	// char ipaddress[MAX_IP_SIZE];

	// ipaddress[0] = '\0';

	// _vm->getStringFromArray(parameter1, name, sizeof(name));
	// if (parameter2)
	// 	_vm->getStringFromArray(parameter2, ipaddress, sizeof(ipaddress));

	// debug(1, "Net::setProviderByName(\"%s\", \"%s\")", name, ipaddress); // PN_SetProviderByName

	// Emulate that we found a TCP/IP provider

	// Create a new ENet instance and initialize the library.
	if (_enet) {
		warning("Net::setProviderByName: ENet instance already exists.");
		return 1;
	}
	_enet = new Networking::ENet();
	if (!_enet->initialize()) {
		_vm->displayMessage(0, "Unable to initialize ENet library.");
		Net::closeProvider();
		return 0;
	}
	return 1;
}

void Net::setFakeLatency(int time) {
	_latencyTime = time;
	debug("NETWORK: Setting Fake Latency to %d ms", _latencyTime);
	_fakeLatency = true;
}

bool Net::destroyPlayer(int32 userId) {
	// bool PNETWIN_destroyplayer(DPID idPlayer)
	debug(1, "Net::destroyPlayer(%d)", userId);
	if (_isHost) {
		if (userId == 1)
			return true;
		if (_userIdToName.contains(userId)) {
			_userIdToName.erase(userId);
			_numUsers--;

			if (_userIdToAddress.contains(userId)) {
				Common::String address = _userIdToAddress[userId];
				_addressToUserId.erase(address);
				_userIdToAddress.erase(userId);
			}

			if (_userIdToPeerIndex.contains(userId) && _userIdToPeerIndex[userId] != _sessionServerPeer) {
				_sessionHost->disconnectPeer(_userIdToPeerIndex[userId]);
				_userIdToPeerIndex.erase(userId);
			}
			return true;
		}
		warning("NETWORK: destoryPlayer(%d): User does not exist!", userId);
		return false;
	}

	Common::String removerUser = "{\"cmd\":\"remove_user\"}";
	_sessionHost->send(removerUser.c_str(), 0, 0, true);
	_sessionHost->disconnectPeer(0);

	return true;
}

int32 Net::startQuerySessions(bool connectToSessionServer) {
	debug(1, "Net::startQuerySessions()");

	// TODO: Config to enable/disable Internet sessions.
	if (!_sessionServerHost && connectToSessionServer) {
		// TODO: Configurable session server address and port
		_sessionServerHost = _enet->connectToHost("127.0.0.1", 9120);
		if (!_sessionServerHost)
			warning("Failed to connect to session server!  You'll won't be able to join internet sessions");
	}

	// TODO: Config to enable/disable LAN discovery.
	if (!_broadcastSocket) {
		_broadcastSocket = _enet->createSocket("0.0.0.0", 0);
	}
	return 0;
}

int32 Net::updateQuerySessions() {
	debug(1, "Net::updateQuerySessions(): begin"); // UpdateQuerySessions

	if (_sessionServerHost) {
		// Get internet-based sessions from the sessin server.
		Common::String getSessions = Common::String::format(
			"{\"cmd\":\"get_sessions\",\"game\":\"%s\",\"version\":\"%s\"}",
			_gameName.c_str(), _gameVersion.c_str());
		_sessionServerHost->send(getSessions.c_str(), 0);

		uint32 tickCount = g_system->getMillis() + 100;
		while(g_system->getMillis() < tickCount) {
			serviceSessionServer();
		}
	}
	if (_broadcastSocket) {
		// Send a session query to the broadcast address.
		_broadcastSocket->send("255.255.255.255", 9130, "{\"cmd\": \"get_session\"}");

		uint32 tickCount = g_system->getMillis() + 500;
		while(g_system->getMillis() < tickCount) {
			serviceBroadcast();
		}
	}

	for (Common::Array<Session>::iterator i = _sessions.begin(); i != _sessions.end();) {
		if (g_system->getMillis() - i->timestamp > 5000) {
			// It has been 5 seconds since we have last seen this session, remove it.
			i = _sessions.erase(i);
		} else {
			i++;
		}
	}

	debug(1, "Net::updateQuerySessions(): got %d", _sessions.size());
	return _sessions.size();
}

void Net::stopQuerySessions() {
	debug(1, "Net::stopQuerySessions()"); // StopQuerySessions

	if (_sessionServerHost && !_isRelayingGame) {
		_sessionServerHost->disconnectPeer(0);
		delete _sessionServerHost;
		_sessionServerHost = nullptr;
	}

	if (_broadcastSocket) {
		delete _broadcastSocket;
		_broadcastSocket = nullptr;
	}

	_sessions.clear();
	// No op
}

int Net::querySessions() {
	debug(1, "Net::querySessions()"); // PN_QuerySessions
	// Deprecated OP used in Backyard Football 2002 to query sessions,
	// emulate this by using the functions used in Moonbase Commander:
	startQuerySessions();

	return updateQuerySessions();
}

int Net::queryProviders() {
	debug(1, "Net::queryProviders()"); // PN_QueryProviders

	// Emulate that we have 1 provider, TCP/IP
	return 1;
}

int Net::setProvider(int providerIndex) {
	warning("STUB: Net::setProvider(%d)", providerIndex); // PN_SetProvider
	return 0;
}

int Net::closeProvider() {
	debug(1, "Net::closeProvider()"); // PN_CloseProvider
	if (_enet) {
		// Destroy all ENet instances and deinitialize.
		if (_sessionHost) {
			endSession();
		}
		delete _enet;
		_enet = nullptr;
	}

	return 1;
}

bool Net::initAll() {
	warning("STUB: Net::initAll()"); // PN_DoInitAll
	return false;
}

bool Net::initProvider() {
	warning("STUB: Net::initProvider()"); // PN_DoInitProvider
	return false;
}

bool Net::initSession() {
	warning("STUB: Net::initSession()"); // PN_DoInitSession
	return false;
}

bool Net::initUser() {
	warning("STUB: Net::initUser()"); // PN_DoInitUser
	return false;
}

void Net::remoteStartScript(int typeOfSend, int sendTypeParam, int priority, int argsCount, int32 *args) {
	Common::String res = "\"params\": [";

	if (argsCount > 2)
		for (int i = 0; i < argsCount - 1; i++)
			res += Common::String::format("%d,", args[i]);

	if (argsCount > 1)
		res += Common::String::format("%d]", args[argsCount - 1]);
	else
		res += "]";

	debug(1, "Net::remoteStartScript(%d, %d, %d, %d, ...)", typeOfSend, sendTypeParam, priority, argsCount); // PN_RemoteStartScriptCommand

	remoteSendData(typeOfSend, sendTypeParam, PACKETTYPE_REMOTESTARTSCRIPT, res, priority);
}

int Net::remoteSendData(int typeOfSend, int sendTypeParam, int type, Common::String data, int priority, int defaultRes, bool wait, int callid) {
	if (!_enet || !_sessionHost || _myUserId == -1)
		return defaultRes;

	if (typeOfSend == PN_SENDTYPE_INDIVIDUAL && sendTypeParam == 0)
		// In DirectPlay, sending a message to 0 means all players
		// sooo, send all.
		typeOfSend = PN_SENDTYPE_ALL;

	// Since I am lazy, instead of constructing the JSON object manually
	// I'd rather parse it
	Common::String res = Common::String::format(
		"{\"cmd\":\"game\",\"from\":%d,\"to\":%d,\"toparam\":%d,"
		"\"type\":%d, \"reliable\":%s, \"data\":{%s}}",
		_myUserId, typeOfSend, sendTypeParam, type,
		priority == PN_PRIORITY_HIGH ? "true" : "false", data.c_str());

	debug(1, "NETWORK: Sending data: %s", res.c_str());
	Common::JSONValue *str = Common::JSON::parse(res.c_str());
	if (_isHost) {
		// handleGameDataHost(str, sendTypeParam - 1);
		_hostDataQueue.push(str);
		_peerIndexQueue.push(sendTypeParam - 1);
	} else
		_sessionHost->send(res.c_str(), 0, 0, priority == PN_PRIORITY_HIGH);
	return defaultRes;
}

void Net::remoteSendArray(int typeOfSend, int sendTypeParam, int priority, int arrayIndex) {
	debug(1, "Net::remoteSendArray(%d, %d, %d, %d)", typeOfSend, sendTypeParam, priority, arrayIndex & ~0x33539000); // PN_RemoteSendArrayCommand

	ScummEngine_v90he::ArrayHeader *ah = (ScummEngine_v90he::ArrayHeader *)_vm->getResourceAddress(rtString, arrayIndex & ~0x33539000);

	Common::String jsonData = Common::String::format(
		"\"type\":%d,\"dim1start\":%d,\"dim1end\":%d,\"dim2start\":%d,\"dim2end\":%d,\"data\":[",
		ah->type, ah->dim1start, ah->dim1end, ah->dim2start, ah->dim2end);

	int32 size = (FROM_LE_32(ah->dim1end) - FROM_LE_32(ah->dim1start) + 1) *
		(FROM_LE_32(ah->dim2end) - FROM_LE_32(ah->dim2start) + 1);

	for (int i = 0; i < size; i++) {
		int32 data;

		switch (FROM_LE_32(ah->type)) {
		case ScummEngine_v90he::kByteArray:
		case ScummEngine_v90he::kStringArray:
			data = ah->data[i];
			break;

		case ScummEngine_v90he::kIntArray:
			data = (int16)READ_LE_UINT16(ah->data + i * 2);
			break;

		case ScummEngine_v90he::kDwordArray:
			data = (int32)READ_LE_UINT32(ah->data + i * 4);
			break;

		default:
			error("Net::remoteSendArray(): Unknown array type %d for array %d", FROM_LE_32(ah->type), arrayIndex);
		}

		jsonData += Common::String::format("%d", data);

		if (i < size - 1)
			jsonData += ",";
		else
			jsonData += "]";
	}

	remoteSendData(typeOfSend, sendTypeParam, PACKETTYPE_REMOTESENDSCUMMARRAY, jsonData, priority);
}

int Net::remoteStartScriptFunction(int typeOfSend, int sendTypeParam, int priority, int defaultReturnValue, int argsCount, int32 *args) {
	warning("STUB: Net::remoteStartScriptFunction(%d, %d, %d, %d, %d, ...)", typeOfSend, sendTypeParam, priority, defaultReturnValue, argsCount);
	return 0;
	int callid = _vm->_rnd.getRandomNumber(1000000);

	Common::String res = Common::String::format("\"callid\":%d, \"params\": [", callid);

	if (argsCount > 2)
		for (int i = 0; i < argsCount - 1; i++)
			res += Common::String::format("%d, ", args[i]);

	if (argsCount > 1)
		res += Common::String::format("%d]", args[argsCount - 1]);
	else
		res += "]";

	debug(1, "Net::remoteStartScriptFunction(%d, %d, %d, %d, %d, ...)", typeOfSend, sendTypeParam, priority, defaultReturnValue, argsCount); // PN_RemoteStartScriptFunction

	return remoteSendData(typeOfSend, sendTypeParam, PACKETTYPE_REMOTESTARTSCRIPTRETURN, res, defaultReturnValue, true, callid);
}

bool Net::getHostName(char *hostname, int length) {
	warning("STUB: Net::getHostName(\"%s\", %d)", hostname, length); // PN_GetHostName
	return false;
}

bool Net::getIPfromName(char *ip, int ipLength, char *nameBuffer) {
	warning("STUB: Net::getIPfromName(\"%s\", %d, \"%s\")", ip, ipLength, nameBuffer); // PN_GetIPfromName
	return false;
}

void Net::getSessionName(int sessionNumber, char *buffer, int length) {
	debug(1, "Net::getSessionName(%d, ..., %d)", sessionNumber, length); // PN_GetSessionName

	if (_sessions.empty()) {
		*buffer = '\0';
		warning("Net::getSessionName(): no sessions");
		return;
	}

	if (sessionNumber >= (int)_sessions.size()) {
		*buffer = '\0';
		warning("Net::getSessionName(): session number too big: %d >= %d", sessionNumber, (int)_sessions.size());
		return;
	}

	Common::strlcpy(buffer, _sessions[sessionNumber].name.c_str(), length);
}

int Net::getSessionPlayerCount(int sessionNumber) {
	debug(1, "Net::getSessionPlayerCount(%d)", sessionNumber); // case GET_SESSION_PLAYER_COUNT_KLUDGE:

	if (_sessions.empty()) {
		warning("Net::getSessionPlayerCount(): no sessions");
		return 0;
	}

	if (sessionNumber >= (int)_sessions.size()) {
		warning("Net::getSessionPlayerCount(): session number too big: %d >= %d", sessionNumber, (int)_sessions.size());
		return 0;
	}

	if (_sessions[sessionNumber].players < 1) {
		warning("Net::getSessionPlayerCount(): no players in session");
		return 0;
	}

	return _sessions[sessionNumber].players;
}

void Net::getProviderName(int providerIndex, char *buffer, int length) {
	warning("STUB: Net::getProviderName(%d, \"%s\", %d)", providerIndex, buffer, length); // PN_GetProviderName
}

void Net::serviceSessionServer() {
	if (!_sessionServerHost)
		return;

	uint8 type = _sessionServerHost->service();
	switch(type) {
	case ENET_EVENT_TYPE_NONE:
		break;
	case ENET_EVENT_TYPE_DISCONNECT:
		warning("NETWORK: Lost connection to session server");
		delete _sessionServerHost;
		_sessionServerHost = nullptr;
		break;
	case ENET_EVENT_TYPE_RECEIVE:
		handleSessionServerData(_sessionServerHost->getPacketData());
		break;
	}
}

void Net::handleSessionServerData(Common::String data) {
	debug(1, "NETWORK: Received data from session server.  Data: %s", data.c_str());

	Common::JSONValue *json = Common::JSON::parse(data.c_str());
	if (!json) {
		warning("NETWORK: Received non-JSON string from session server ignoring.");
		return;
	}
	if (!json->isObject()){
		warning("NETWORK: Received non-JSON object from session server: \"%s\"", data.c_str());
		return;
	}

	Common::JSONObject root = json->asObject();
	if (root.contains("cmd") && root["cmd"]->isString()) {
		Common::String command = root["cmd"]->asString();
		if (_isHost && command == "host_session_resp") {
			if (root.contains("id")) {
				_sessionId = root["id"]->asIntegerNumber();
				debug(1, "NETWORK: Our session id from session server: %d", _sessionId);
			}
		} else if (!_isHost && command == "get_sessions_resp") {
			if (root.contains("address") && root.contains("sessions")) {
				_hostPort = getAddressFromString(root["address"]->asString()).port;
				Common::JSONArray sessions = root["sessions"]->asArray();
				for (uint i = 0; i != sessions.size(); i++) {
					Common::JSONObject sessionData = sessions[i]->asObject();
					Address sessionAddress = getAddressFromString(sessionData["address"]->asString());

					// Check if we already know about this session:
					bool makeNewSession = true;
					for (Common::Array<Session>::iterator j = _sessions.begin(); j != _sessions.end(); j++) {
						if (j->id == sessionData["id"]->asIntegerNumber()) {
							// Yes we do, Update the timestamp and player count.
							makeNewSession = false;
							if (!j->local) {
								// Only update if it's not a local session
								j->timestamp = g_system->getMillis();
								j->players = sessionData["players"]->asIntegerNumber();
							}
							break;
						}
					}

					if (!makeNewSession)
						continue;

					Session session;
					session.id = sessionData["id"]->asIntegerNumber();
					session.name = sessionData["name"]->asString();
					session.players = sessionData["players"]->asIntegerNumber();
					session.host = sessionAddress.host;
					session.port = sessionAddress.port;
					session.timestamp = g_system->getMillis();
					_sessions.push_back(session);
				}
			}
		} else if (_isHost && command == "joining_session") {
			// Someone is gonna attempt to join our session.  Get their address and hole-punch:
			if (_sessionHost && root.contains("address")) {
				Address address = getAddressFromString(root["address"]->asString());
				// By sending an UDP packet, the router will open a hole for the
				// destinated address, allowing someone with the same address to
				// communicate with us.  This does not work with every router though...
				//
				// More infomation: https://en.wikipedia.org/wiki/UDP_hole_punching
				debug(1, "NETWORK: Hole punching %s:%d", address.host.c_str(), address.port);
				_sessionHost->sendRawData(address.host, address.port, "");
			}
		} else if (_isHost && command == "add_user_for_relay") {
			// For cases that peer-to-peer communication is not possible, the session server
			// will act as a relay to the host.
			if (root.contains("address")) {
				// To be sent back for context.
				Common::String address = root["address"]->asString();

				if (addUser(const_cast<char *>(address.c_str()), const_cast<char *>(address.c_str()))) {
					_userIdToAddress[_userIdCounter] = "127.0.0.1:9120";
					_addressToUserId["127.0.0.1:9120"] = _userIdCounter;
					_userIdToPeerIndex[_userIdCounter] = _sessionServerPeer;

					_isRelayingGame = true;

					Common::String resp = Common::String::format(
						"{\"cmd\":\"add_user_resp\",\"game\":\"%s\",\"version\":\"%s\",\"address\":\"%s\",\"id\":%d}",
						_gameName.c_str(), _gameVersion.c_str(), address.c_str(), _userIdCounter);
					_sessionHost->send(resp.c_str(), _sessionServerPeer);
				}
			}
		} else if (!_isHost && command == "add_user_resp") {
			if (root.contains("id") && _myUserId == -1) {
				_myUserId = root["id"]->asIntegerNumber();

				// We are now relaying data to the session server,
				// set the sessionServerHost as the sessionHost.
				_isRelayingGame = true;
				_sessionHost = _sessionServerHost;
				_sessionServerHost = nullptr;
			}
		} else if (_isHost && command == "remove_user") {
			// Relay user wants their removal (if they haven't been removed already).
			if (root.contains("id")) {
				int userId = root["id"]->asIntegerNumber();
				if (_userIdToName.contains(userId)) {
					if (_userIdToPeerIndex[userId] == _sessionServerPeer) {
						debugC(1, "Removing relay user %d", userId);
						destroyPlayer(userId);
					} else {
						warning("NETWORK: Attempt to remove non-relay user: %d", userId);
					}
				}
			}
		} else if (command == "game") {
			// Received relayed data.
			if (_isHost)
				handleGameDataHost(json, _sessionServerPeer);
			else
				handleGameData(json, _sessionServerPeer);
		}
	}
}

bool Net::serviceBroadcast() {
	if (!_broadcastSocket)
		return false;

	if (!_broadcastSocket->receive())
		return false;

	handleBroadcastData(_broadcastSocket->getData(), _broadcastSocket->getHost(), _broadcastSocket->getPort());
	return true;
}

void Net::handleBroadcastData(Common::String data, Common::String host, int port) {
	debug(1, "NETWORK: Received data from broadcast socket.  Source: %s:%d  Data: %s", host.c_str(), port, data.c_str());

	Common::JSONValue *json = Common::JSON::parse(data.c_str());
	if (!json) {
		// Just about anything could come from the broadcast address, so do not warn.
		debug(1, "NETWORK: Not a JSON string, ignoring.");
		return;
	}
	if (!json->isObject()){
		warning("NETWORK: Received non JSON object from broadcast socket: \"%s\"", data.c_str());
		return;
	}

	Common::JSONObject root = json->asObject();
	if (root.contains("cmd") && root["cmd"]->isString()) {
		Common::String command = root["cmd"]->asString();

		if (command == "get_session") {
			// Session query.
			if (_sessionHost) {
				Common::String resp = Common::String::format(
					"{\"cmd\":\"session_resp\",\"version\":\"%s\",\"id\":%d,\"name\":\"%s\",\"players\":%d}",
					_gameVersion.c_str(), _sessionId, _sessionName.c_str(), getTotalPlayers());

				// Send this through the session host instead of the broadcast socket
				// because that will send the correct port to connect to.
				// They'll still receive it though, that's the power of connection-less sockets.
				_sessionHost->sendRawData(host, port, resp.c_str());
			}
		} else if (command == "session_resp") {
			if (!_sessionHost && root.contains("version") && root.contains("id") && root.contains("name") && root.contains("players")) {
				Common::String version = root["version"]->asString();
				int sessionId = root["id"]->asIntegerNumber();
				Common::String name = root["name"]->asString();
				int players = root["players"]->asIntegerNumber();

				// TODO: Check and match game name

				if (version != _gameVersion)
					// Version mismatch.
					return;

				if (players < 1 || players > _maxPlayers)
					// This session is either full or not finished initalizing (adding the host player itself)
					return;

				// Check if the session of the game ID (from the internet session server) exists.
				// if so, update it as a local session and swap the internet-based address to local.
				for (Common::Array<Session>::iterator i = _sessions.begin(); i != _sessions.end(); i++) {
					if (i->id == sessionId && !i->local) {
						i->local = true;
						i->host = host;
						i->port = port;
						i->timestamp = g_system->getMillis();
						i->players = players;
						return;
					}
				}
				// Check if we already know about this session:
				for (Common::Array<Session>::iterator i = _sessions.begin(); i != _sessions.end(); i++) {
					if (i->host == host && i->port == port) {
						// Yes we do, Update the timestamp and player count.
						i->timestamp = g_system->getMillis();
						i->players = players;
						return;
					}
				}
				// If we're here, assume that we had no clue about this session, store it.
				Session session;
				session.local = true;
				session.host = host;
				session.port = port;
				session.name = name;
				session.players = players;
				session.timestamp = g_system->getMillis();
				_sessions.push_back(session);
			}
		}
	}
}

bool Net::remoteReceiveData(uint32 tickCount) {
	uint8 messageType = _sessionHost->service();
	switch (messageType) {
	case ENET_EVENT_TYPE_NONE:
		return true;
	case ENET_EVENT_TYPE_CONNECT:
		{
			debug(1, "NETWORK: New connection from %s:%d", _sessionHost->getHost().c_str(), _sessionHost->getPort());
			return true;
		}
		return true;
	case ENET_EVENT_TYPE_DISCONNECT:
		{
			Common::String address = Common::String::format("%s:%d", _sessionHost->getHost().c_str(), _sessionHost->getPort());

			int userId = -1;
			if (_addressToUserId.contains(address))
				userId = _addressToUserId[address];
			if (userId > -1)
				debug(1, "NETWORK: User %s (%d) has disconnected.", _userIdToName[userId].c_str(), userId);
			else
				debug(1, "NETWORK: Connection from %s has disconnected.", address.c_str());

			if (_gameName == "moonbase") {
				// TODO: Host migration
				if (!_isHost && _vm->_currentRoom == 2) {
					_vm->displayMessage(0, "You have been disconnected from the host.\nNormally, host migration would take place, but ScummVM doesn't do that yet, so this game session will now end." );
					_vm->VAR(253) = 26; // gGameMode = GAME-OVER
					_vm->runScript(2104, 1, 0, 0); // leave-game
				}
			}
			return true;
		}
		return true;
	case ENET_EVENT_TYPE_RECEIVE:
		{
			Common::String host = _sessionHost->getHost();
			int port = _sessionHost->getPort();
			debug(1, "NETWORK: Got data from %s:%d", host.c_str(), port);

			int peerIndex = _sessionHost->getPeerIndexFromHost(host, port);
			if (peerIndex == -1) {
				warning("NETWORK: Unable to get peer index for host %s:%d", host.c_str(), port);
				_sessionHost->destroyPacket();
				return false;
			}

			Common::String data = _sessionHost->getPacketData();
			debug(1, "%s", data.c_str());

			if (peerIndex == _sessionServerPeer) {
				handleSessionServerData(data);
				return true;
			}

			Common::JSONValue *json = Common::JSON::parse(data.c_str());
			if (!json) {
				// Just about anything could come from the broadcast address, so do not warn.
				warning("NETWORK: Received non-JSON string.  Got: \"%s\"", data.c_str());
				_sessionHost->destroyPacket();
				return false;
			}
			if (!json->isObject()){
				warning("NETWORK: Received non JSON object from broadcast socket: \"%s\"", data.c_str());
				_sessionHost->destroyPacket();
				return false;
			}

			Common::JSONObject root = json->asObject();
			if (root.contains("cmd") && root["cmd"]->isString()) {
				Common::String command = root["cmd"]->asString();

				if (_isHost && command == "add_user") {
					if (root.contains("name")) {
						Common::String name = root["name"]->asString();
						if (getTotalPlayers() > 4) {
							// We are full.
							return 0;
						}
						_userIdToName[++_userIdCounter] = name;
						_numUsers++;
						if (_sessionId && _sessionServerPeer > -1) {
							// Update player count to session server
							Common::String updatePlayers = Common::String::format(
								"{\"cmd\":\"update_players\",\"game\":\"%s\",\"version\":\"%s\",\"players\":%d}",
								_gameName.c_str(), _gameVersion.c_str(), getTotalPlayers());
							_sessionHost->send(updatePlayers.c_str(), _sessionServerPeer);
						}

						Common::String address = Common::String::format("%s:%d", host.c_str(), port);
						_userIdToAddress[_userIdCounter] = address;
						_addressToUserId[address] = _userIdCounter;
						_userIdToPeerIndex[_userIdCounter] = peerIndex;

						Common::String resp = Common::String::format(
							"{\"cmd\":\"add_user_resp\",\"id\":%d}", _userIdCounter);
						_sessionHost->send(resp.c_str(), peerIndex);
					}
				} else if (!_isHost && command == "add_user_resp") {
					if (root.contains("id") && _myUserId == -1) {
						_myUserId = root["id"]->asIntegerNumber();
					}
				} else if (_isHost && command == "remove_user") {
					Common::String address = Common::String::format("%s:%d", host.c_str(), port);
					int userId = -1;
					userId = _addressToUserId[address];
					if (userId == -1) {
						warning("Got remove_user but we don't know the user for address: %s", address.c_str());
						return false;
					}
					destroyPlayer(userId);
				} else if (command == "game") {
					if (_isHost)
						handleGameDataHost(json, peerIndex);
					else
						handleGameData(json, peerIndex);
				}
			}
			if (_sessionHost)
				_sessionHost->destroyPacket();
		}
		return true;
		break;
	}
	return true;
}

void Net::doNetworkOnceAFrame(int msecs) {
	if (!_enet || !_sessionHost)
		return;

	remoteReceiveData(msecs);

	if (_sessionServerHost)
		serviceSessionServer();

	if (_broadcastSocket)
		serviceBroadcast();

	if (_isHost && _hostDataQueue.size()) {
		if (_hostDataQueue.size() != _hostDataQueue.size())
			warning("NETWORK: Sizes of data and peer index queues does not match!  Expect some wonky stuff");
		Common::JSONValue *json = _hostDataQueue.pop();
		int peerIndex = _peerIndexQueue.pop();
		handleGameDataHost(json, peerIndex);
	}
}

void Net::handleGameData(Common::JSONValue *json, int peerIndex) {
	if (!_enet || !_sessionHost)
		return;
	_fromUserId = json->child("from")->asIntegerNumber();
	uint type = json->child("type")->asIntegerNumber();

	uint32 *params;

	switch (type) {
	case PACKETTYPE_REMOTESTARTSCRIPT:
		{
			Common::JSONArray paramsArray = json->child("data")->child("params")->asArray();

			if (_gameName == "moonbase") {
				// Detect if the host has disconnected.
				if (paramsArray[0]->asIntegerNumber() == 145 && _fromUserId == 1) {
					if (!_isHost && _vm->_currentRoom == 2) {
						// TODO: Host migration
						_vm->displayMessage(0, "You have been disconnected from the host.\nNormally, host migration would take place, but ScummVM doesn't do that yet, so this game session will now end.");
						_vm->VAR(253) = 26; // GAME-OVER
						_vm->runScript(2104, 1, 0, 0); // leave-game
						return;
					}
				}
			}

			int datalen = paramsArray.size();
			params = (uint32 *)_tmpbuffer;

			for (int i = 0; i < datalen; i++) {
				*params = paramsArray[i]->asIntegerNumber();
				params++;
			}

			if (!_vm->VAR(_vm->VAR_REMOTE_START_SCRIPT)) {
				warning("NETWORK: VAR_REMOTE_START_SCRIPT not defined!");
				return;
			}

			debug("%d", _vm->VAR(_vm->VAR_REMOTE_START_SCRIPT));

			_vm->runScript(_vm->VAR(_vm->VAR_REMOTE_START_SCRIPT), 1, 0, (int *)_tmpbuffer);
			_vm->pop();
		}
		break;

	case PACKETTYPE_REMOTESTARTSCRIPTRETURN:
		{
			int datalen = json->child("data")->child("params")->asArray().size();
			params = (uint32 *)_tmpbuffer;

			for (int i = 0; i < datalen; i++) {
				*params = json->child("data")->child("params")->asArray()[i]->asIntegerNumber();
				params++;
			}

			_vm->runScript(_vm->VAR(_vm->VAR_REMOTE_START_SCRIPT), 1, 0, (int *)_tmpbuffer);
			int result = _vm->pop();

			Common::String res = Common::String::format("\"result\": %d, \"callid\": %d", result,
					(int)json->child("data")->child("callid")->asIntegerNumber());

			remoteSendData(PN_SENDTYPE_INDIVIDUAL, _fromUserId, PACKETTYPE_REMOTESTARTSCRIPTRESULT, res, PN_PRIORITY_HIGH);
		}
		break;

	case PACKETTYPE_REMOTESTARTSCRIPTRESULT:
		//
		// Ignore it.
		//

		break;

	case PACKETTYPE_REMOTESENDSCUMMARRAY:
		{
			int newArray = 0;

			// Assume that the packet data contains a "SCUMM PACKAGE"
			// and unpack it into an scumm array :-)

			int dim1start = json->child("data")->child("dim1start")->asIntegerNumber();
			int dim1end   = json->child("data")->child("dim1end")->asIntegerNumber();
			int dim2start = json->child("data")->child("dim2start")->asIntegerNumber();
			int dim2end   = json->child("data")->child("dim2end")->asIntegerNumber();
			int atype     = json->child("data")->child("type")->asIntegerNumber();

			byte *data = _vm->defineArray(0, atype, dim2start, dim2end, dim1start, dim1end, true, &newArray);

			int32 size = (dim1end - dim1start + 1) * (dim2end - dim2start + 1);

			int32 value;

			for (int i = 0; i < size; i++) {
				value = json->child("data")->child("data")->asArray()[i]->asIntegerNumber();

				switch (atype) {
				case ScummEngine_v90he::kByteArray:
				case ScummEngine_v90he::kStringArray:
					data[i] = value;
					break;

				case ScummEngine_v90he::kIntArray:
					WRITE_LE_UINT16(data + i * 2, value);
					break;

				case ScummEngine_v90he::kDwordArray:
					WRITE_LE_UINT32(data + i * 4, value);
					break;

				default:
					error("Net::remoteReceiveData(): Unknown array type %d", atype);
				}
			}

			memset(_tmpbuffer, 0, 25 * 4);
			WRITE_UINT32(_tmpbuffer, newArray);

			// Quick start the script (1st param is the new array)
			_vm->runScript(_vm->VAR(_vm->VAR_NETWORK_RECEIVE_ARRAY_SCRIPT), 1, 0, (int *)_tmpbuffer);
		}
		break;

	default:
		warning("NETWORK: Received unknown network command %d", type);
	}

}

void Net::handleGameDataHost(Common::JSONValue *json, int peerIndex) {
	int from = json->child("from")->asIntegerNumber();
	int to = json->child("to")->asIntegerNumber();
	int toparam = json->child("toparam")->asIntegerNumber();
	bool reliable = json->child("reliable")->asBool();

	switch(to) {
	case PN_SENDTYPE_INDIVIDUAL:
		{
			if (toparam == _myUserId) {
				// It's for us, handle it.
				handleGameData(json, peerIndex);
				return;
			}
			// It's for someone else, transfer it.
			if (!_userIdToName.contains(toparam)) {
				warning("NETWORK: Got individual message for %d, but we don't know this person!  Ignoring...", toparam);
				return;
			}
			debug(1, "NETWORK: Transfering message to %s (%d), peerIndex: %d", _userIdToName[toparam].c_str(), toparam, _userIdToPeerIndex[toparam]);
			Common::String str = Common::JSON::stringify(json);
			_sessionHost->send(str.c_str(), _userIdToPeerIndex[toparam], 0, reliable);
		}
		break;
	case PN_SENDTYPE_GROUP:
		warning("STUB: PN_SENDTYPE_GROUP");
		break;
	case PN_SENDTYPE_HOST:
		{
			// It's for us, handle it.
			handleGameData(json, peerIndex);
		}
		break;
	case PN_SENDTYPE_ALL:
		{
			// It's for all of us, including the host.
			// Don't handle data if we're shutting down, or the game will crash.
			if (!_isShuttingDown && from != _myUserId)
				handleGameData(json, peerIndex);
			Common::String str = Common::JSON::stringify(json);
			bool sentToSessionServer = false;
			for (int i = 0; i < _numUsers; i++) {
				if (i != _userIdToPeerIndex[from]) {
					if (i == _sessionServerPeer) {
						// If we are relaying game data, make sure that the data only get sent
						// to the session server only once.
						if (_isRelayingGame && !sentToSessionServer) {
							_sessionHost->send(str.c_str(), _sessionServerPeer, 0, reliable);
							sentToSessionServer = true;
						}
					} else
						_sessionHost->send(str.c_str(), i, 0, reliable);
				}
			}
		}
		break;
	default:
		warning("NETWORK: Unknown data type: %d", to);
	}
}

} // End of namespace Scumm