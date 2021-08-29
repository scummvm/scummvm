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
#include "scumm/he/byonline/byonline.h"

#include "directplay.h"

#include <enet/enet.h>

namespace Scumm {

DirectPlay::DirectPlay(ScummEngine_v90he *s) {
	_vm = s;
	_inited = false;

	_host = nullptr;
	_hosting = false;

	_sessionPeer = nullptr;
	_playerPeer = nullptr;

	_roomCounter = -1;
}

DirectPlay::~DirectPlay() {
	if (_inited) {
		if (_sessionPeer) {
			disconnectSessionServer();
		}
		if (_playerPeer) {
			disconnect();
		}
		enet_deinitialize();
	}
}

void DirectPlay::startOfFrame() {
	if (!_inited) {
		return;
	}

	// HACK: See comment in handleRemoteStartScript
	if (_roomCounter > -1) {
		_roomCounter += 1;
		if (_roomCounter == 5) {
			_vm->startScene(6, 0, 0);
			_roomCounter = -1;
		}
	}

	if (_host) {
		ENetEvent event;
		if (enet_host_service(_host, &event, 0) > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_NONE:
				break;
			case ENET_EVENT_TYPE_CONNECT:
				if (event.peer->address.port == _peerPort) {
					printf("DirectPlay: Connected to peer!\n");
					_playerPeer = event.peer;
					// Let the game know that we've successfully connected.
					if (_hosting) {
						_vm->writeVar(111, 99);
					} else {
						_vm->_byonline->connectedToSession();
					}
				}
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				printf("DirectPlay: Peer dropped us!\n");
				disconnect(true);
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				Common::String data((const char*)event.packet->data, event.packet->dataLength);
				if (event.peer == _sessionPeer) {
					handleSessionPacket(data);
				} else {
					handleGamePacket(data);
				}
				// Clean up packet now that we are done using it.
				enet_packet_destroy(event.packet);
				break;
			}
		}
	}
}

int32 DirectPlay::dispatch(int op, int numArgs, int32 *args) {
	int res = 0;

	switch (op) {
	case OP_NET_INIT:
		res = init();
		break;
	case OP_NET_WHO_AM_I:
		res = _userId;
		break;
	case OP_NET_QUERY_SESSIONS:
	case OP_NET_JOIN_SESSION:
	case OP_NET_ADD_USER:
		if (_host != nullptr) {
			res = 1;
		}
		break;
	case OP_NET_END_SESSION:
		disconnect();
		break;
	case OP_NET_WHO_SENT_THIS:
		res = _fromId;
		break;
	case OP_NET_REMOTE_START_SCRIPT:
		sendRemoteStartScript(args[0], args[1], args[2], numArgs - 3, &args[3]);
		break;
	case OP_NET_REMOTE_SEND_ARRAY:
		sendRemoteArray(args[0], args[1], args[2], args[3]);
		break;
	default:
		Common::String str = Common::String::format("DirectPlay: unknown op: (%d, %d, [", op, numArgs);
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

int32 DirectPlay::init() {
	if (!_inited) {
		debugC(DEBUG_DIRECTPLAY, "DirectPlay: Initializing ENet...");
		if (enet_initialize() != 0) {
			warning("Failed to initialize ENet.  You'll most likely won't be able to play online games.");
			return _inited;
		}
		debugC(DEBUG_DIRECTPLAY, "DirectPlay: ENet successfully initialized.");
		_inited = true;
	}

	// While we're at it, might as well clean up some stuff, because we might still be
	// connected to/hosting some servers before the game restarted.
	if (_playerPeer) {
		disconnect();
	}
	if (_sessionPeer) {
		disconnectSessionServer();
	}
	return _inited;
}

void DirectPlay::send(ENetPeer* peer, Common::JSONObject data, bool reliable) {
	if (_host == nullptr) {
		warning("DirectPlay: Attempted to send data without connecting first!");
		return;
	}
	Common::JSONValue value(data);
	Common::String valueString = Common::JSON::stringify(&value);

	debugC(DEBUG_DIRECTPLAY, "DirectPlay: Sending data: %s", valueString.c_str());

	// TODO: Determine which packets (mostly start scripts and send arrays) are to be sent reliably or not.
	ENetPacket *packet = enet_packet_create(valueString.c_str(), strlen(valueString.c_str()), ENET_PACKET_FLAG_RELIABLE ? reliable : 0);
	enet_peer_send(peer, 0, packet);
	enet_host_flush(_host);
}

void DirectPlay::handleSessionPacket(Common::String data) {
	debugC(DEBUG_DIRECTPLAY, "DirectPlay: Received Data from session server: %s", data.c_str());
	Common::JSONValue *json = Common::JSON::parse(data.c_str());
	if (!json) {
		warning("DirectPlay: Received trunciated data from session server! %s", data.c_str());
		return;
	}
	if (!json->isObject()){
		warning("DirectPlay: Received non JSON object from session server! %s", data.c_str());
		return;
	}

	Common::JSONObject root = json->asObject();
	if (root.find("cmd") != root.end() && root["cmd"]->isString()) {
		Common::String command = root["cmd"]->asString();

		if (command == "host_resp") {
			int sessionId = root["session"]->asIntegerNumber();
			Common::String host = root["host"]->asString();
			int port = root["port"]->asIntegerNumber();

			handleHostResp(sessionId, host, port);
		} else if (command == "peer_join") {
			Common::String host = root["host"]->asString();
			int port = root["port"]->asIntegerNumber();

			handlePeerJoin(host, port);
		} else if (command == "join_resp") {
			Common::String host = root["host"]->asString();
			int port = root["port"]->asIntegerNumber();
			int hostPort = root["host_port"]->asIntegerNumber();

			handleJoinResp(host, port, hostPort);
		}
	}
}

void DirectPlay::handleGamePacket(Common::String data) {
	debugC(DEBUG_DIRECTPLAY, "DirectPlay: Received Data from peer: %s", data.c_str());
	Common::JSONValue *json = Common::JSON::parse(data.c_str());
	if (!json) {
		warning("DirectPlay: Received trunciated data from peer! %s", data.c_str());
		return;
	}
	if (!json->isObject()){
		warning("DirectPlay: Received non JSON object from peer! %s", data.c_str());
		return;
	}

	Common::JSONObject root = json->asObject();
	if (root.find("cmd") != root.end() && root["cmd"]->isString()) {
		Common::String command = root["cmd"]->asString();

		if (command == "remote_start_script") {
			int fromId = root["from"]->asIntegerNumber();
			Common::JSONArray params = root["params"]->asArray();
			handleRemoteStartScript(fromId, params);
		} else if (command == "remote_array") {
			int fromId = root["from"]->asIntegerNumber();
			int type = root["type"]->asIntegerNumber();
			int dim1start = root["dim1start"]->asIntegerNumber();
			int dim1end = root["dim1end"]->asIntegerNumber();
			int dim2start = root["dim2start"]->asIntegerNumber();
			int dim2end = root["dim2end"]->asIntegerNumber();
			Common::JSONArray arrayData = root["data"]->asArray();
			handleRemoteArray(fromId, type, dim1start, dim1end, dim2start, dim2end, arrayData);
		}
	}
}

void DirectPlay::disconnect(bool dropped) {
	if (_host == nullptr) {
		return;
	}

	if (_playerPeer && !dropped) {
		printf("DirectPlay: Disconnecting peer...\n");
		enet_peer_disconnect(_playerPeer, 0);

		ENetEvent event;
		bool success = false;
		while (!success && enet_host_service(_host, &event, 3000) > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_NONE:
				printf("DirectPlay: Disconnection timed out, force dropping...\n");
				enet_peer_reset(_playerPeer);
				success = true;
				break;
			case ENET_EVENT_TYPE_CONNECT:
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				printf("DirectPlay: Peer disconnected.\n");
				success = true;
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				enet_packet_destroy(event.packet);
				break;
			}
		}
	}

	enet_host_destroy(_host);
	_host = nullptr;

	_playerPeer = nullptr;
	_fromId = 0;
	_hosting = false;
}

bool DirectPlay::connectToSessionServer() {
	_host = enet_host_create(nullptr, 1, 1, 0, 0);
	if (_host == nullptr) {
		warning("DirectPlay (connectToSessionServer): Failed to create host");
		return false;
	}

	ENetAddress address;
	ENetEvent event;

	Common::String host;
	if (ConfMan.hasKey("byonline_session_address"))
		host = ConfMan.get("byonline_session_address");
	else
		// Default address
		host = Common::String("backyardsports.online");

	uint16 port = 9130;
	if (host.contains(":")) {
		// If the address contains a custom port, use that.
		int pos = host.findFirstOf(':');
		port = (uint16)strtol(host.substr(pos + 1).c_str(), nullptr, 0);
		// Strip the port out of the host string
		host = host.substr(0, pos);
	}

	enet_address_set_host(&address, host.c_str());
	address.port = port;

	printf("DirectPlay: Connecting to %s:%u...\n", host.c_str(), port);

	_sessionPeer = enet_host_connect(_host, &address, 1, 0);
	if (_sessionPeer == nullptr) {
		warning("No available peers for initiating an ENet connection.");
		return false;
	}

	if (enet_host_service(_host, &event, 5000) > 0 &&
			event.type == ENET_EVENT_TYPE_CONNECT) {
		printf("DirectPlay: Connection to session server succeeded.\n");
		return true;
	} else {
		warning("DirectPlay: Connection to session server failed.");
		return false;
	}
}

bool DirectPlay::disconnectSessionServer() {
	if (_sessionPeer == nullptr && _host == nullptr) {
		return true;
	}

	printf("DirectPlay: Disconnecting session server...\n");
	enet_peer_disconnect(_sessionPeer, 0);

	ENetEvent event;
	/* Allow up to 3 seconds for the disconnect to succeed
	 * and drop any packets received packets.
	 */
	while (enet_host_service(_host, &event, 3000) > 0) {
		switch (event.type) {
		case ENET_EVENT_TYPE_NONE:
			break;
		case ENET_EVENT_TYPE_CONNECT:
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			printf("DirectPlay: Disconnection to session server successful.\n");
			enet_host_destroy(_host);
			_sessionPeer = nullptr;
			_host = nullptr;
			return true;
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			enet_packet_destroy(event.packet);
			break;
		}
	}

	printf("DirectPlay: Disconnection timed out, force dropping...\n");
	enet_peer_reset(_sessionPeer);

	enet_host_destroy(_host);
	_sessionPeer = nullptr;
	_host = nullptr;
	return true;
}

void DirectPlay::hostSession(int userId) {
	_userId = userId;

	if (connectToSessionServer()) {
		Common::JSONObject hostSessionRequest;
		hostSessionRequest.setVal("cmd", new Common::JSONValue("host_session"));
		send(_sessionPeer, hostSessionRequest);
	} else {
		// Failed
		_vm->writeVar(111, 1);
	}
}

void DirectPlay::handleHostResp(int sessionId, Common::String host, int port) {
	_hostPort = port;

	_vm->_byonline->sendSession(sessionId);
}

void DirectPlay::joinSession(int userId, int sessionId) {
	_userId = userId;

	if (connectToSessionServer()) {
		Common::JSONObject joinSessionRequest;
		joinSessionRequest.setVal("cmd", new Common::JSONValue("join_session"));
		joinSessionRequest.setVal("session", new Common::JSONValue((long long int)sessionId));
		send(_sessionPeer, joinSessionRequest);
	} else {
		// Failed
	}
}

void DirectPlay::handlePeerJoin(Common::String host, int port) {
	_peerHost = host;
	_peerPort = port;

	disconnectSessionServer();

	_hosting = true;

	ENetAddress address;
	// TODO: We shouldn't accept connections from any IP, figure out how to get
	// our local IP and use it as host.
	address.host = ENET_HOST_ANY;
	address.port = _hostPort;

	_host = enet_host_create(&address, 1, 1, 0, 0);
	if (_host == NULL) {
		warning("Failed to create server host");
		_host = nullptr;
		return;
	}

	// Now create our peer address.
	ENetAddress peerAddr;
	ENetPeer *peer;

	enet_address_set_host(&peerAddr, host.c_str());
	peerAddr.port = port;

	peer = enet_host_connect(_host, &peerAddr, 1, 0);
	if (peer == NULL) {
		warning("No available peers for initiating an ENet connection.");
		return;
	}

	// Service it up for a second or so and attempt to connect to a non-existant
	// peer on the other side.  This will basically hole punch our port to that connection.
	ENetEvent pingEvent;
	printf("DirectPlay: Pinging peer...\n");
	enet_host_service(_host, &pingEvent, 1000);

	// Drop the peer,
	enet_peer_reset(peer);

	// And listen for the CONNECT event.
	ENetEvent event;
	printf("DirectPlay: Waiting for peer to connect...\n");
	if (enet_host_service(_host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
		printf("DirectPlay: Peer connected!\n");

		_playerPeer = event.peer;
		_vm->writeVar(111, 99);
	}
}

void DirectPlay::handleJoinResp(Common::String host, int port, int hostPort) {
	_peerHost = host;
	_peerPort = port;

	disconnectSessionServer();

	// Create a new client and connect to the peer

	// This contains the port that we'll be connecting to our host with.
	ENetAddress localAddr;
	localAddr.host = ENET_HOST_ANY;
	localAddr.port = hostPort;

	_host = enet_host_create(&localAddr, 1, 1, 0, 0);
	if (_host == NULL) {
		warning("Failed to create client host");
		_host = nullptr;
		return;
	}

	ENetAddress address;
	// ENetEvent event;
	ENetPeer *peer;

	enet_address_set_host(&address, host.c_str());
	address.port = port;

	printf("DirectPlay: Connecting to remote peer...\n");
	peer = enet_host_connect(_host, &address, 1, 0);
	if (peer == NULL) {
		warning("No available peers for initiating an ENet connection.");
		return;
	}
	// The connection procedure continues at startOfFrame
}

void DirectPlay::sendRemoteStartScript(int typeOfSend, int sendTypeParam, int priority, int argsCount, int32 *args) {
	if (!_playerPeer) {
		warning("DirectPlay: Tried to send remote start script without a peer");
		return;
	}

	debugC(DEBUG_DIRECTPLAY, "sendRemoteStartScript(%d, %d, %d, %d, ...)", typeOfSend, sendTypeParam, priority, argsCount);

	Common::JSONObject remoteStartScriptRequest;
	remoteStartScriptRequest.setVal("cmd", new Common::JSONValue("remote_start_script"));
	remoteStartScriptRequest.setVal("from", new Common::JSONValue((long long int)_userId));

	Common::JSONArray argArray;
	for (int i = 0; i < argsCount; i++) {
		argArray.push_back(new Common::JSONValue((long long int)args[i]));
	}
	remoteStartScriptRequest.setVal("params", new Common::JSONValue(argArray));
	send(_playerPeer, remoteStartScriptRequest, true ? typeOfSend != 4 : false);
}

void DirectPlay::handleRemoteStartScript(int fromId, Common::JSONArray params) {
	_fromId = fromId;
	int args[25];
	memset(args, 0, sizeof(args));
	for (uint i = 0; i < params.size(); i++) {
		args[i] = (int)params[i]->asIntegerNumber();
	}

	// HACK: In Baseball, there's this crazy race condition(?) script bug
	// where we couldn't switch the team type and the game setup process
	// would get out of sync with the game hoster (Specifically, the hoster
	// will be one page ahead while the guest will be one behind).
	// So, let's wait for a few frames before moving on to the team selection room.
	if (_vm->_game.id == GID_BASEBALL2001 && args[0] == 2 && args[1] == 6) {
		_roomCounter = 0;
		return;
	}

	_vm->_byonline->runRemoteStartScript(args);
}

void DirectPlay::sendRemoteArray(int typeOfSend, int sendTypeParam, int priority, int arrayIndex) {
	if (!_playerPeer) {
		warning("DirectPlay: Tried to send remote start script without a peer!");
		return;
	}

	ScummEngine_v90he::ArrayHeader *ah = (ScummEngine_v90he::ArrayHeader *)_vm->getResourceAddress(rtString, arrayIndex & ~0x33539000);
	int32 size = (FROM_LE_32(ah->dim1end) - FROM_LE_32(ah->dim1start) + 1) *
		(FROM_LE_32(ah->dim2end) - FROM_LE_32(ah->dim2start) + 1);

	debugC(DEBUG_DIRECTPLAY, "sendRemoteArray(%d, %d, %d, %d)", typeOfSend, sendTypeParam, priority, arrayIndex);
	debugC(DEBUG_DIRECTPLAY, "%d, %d, %d, %d)", ah->dim1start, ah->dim1end, ah->dim2start, ah->dim2end);


	Common::JSONObject remoteArrayRequest;
	remoteArrayRequest.setVal("cmd", new Common::JSONValue("remote_array"));
	remoteArrayRequest.setVal("from", new Common::JSONValue((long long int)_userId));
	remoteArrayRequest.setVal("type", new Common::JSONValue((long long int)ah->type));

	remoteArrayRequest.setVal("dim1start", new Common::JSONValue((long long int)ah->dim1start));
	remoteArrayRequest.setVal("dim1end", new Common::JSONValue((long long int)ah->dim1end));

	remoteArrayRequest.setVal("dim2start", new Common::JSONValue((long long int)ah->dim2start));
	remoteArrayRequest.setVal("dim2end", new Common::JSONValue((long long int)ah->dim2end));

	Common::JSONArray arrayData;

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
			error("DirectPlay::sendRemoteArray(): Unknown array type %d for array %d", FROM_LE_32(ah->type), arrayIndex);
		}

	arrayData.push_back(new Common::JSONValue((long long int)data));
	}

	remoteArrayRequest.setVal("data", new Common::JSONValue(arrayData));
	send(_playerPeer, remoteArrayRequest, true ? typeOfSend != 4 : false);
}

void DirectPlay::handleRemoteArray(int fromId, int type, int dim1start, int dim1end, int dim2start, int dim2end, Common::JSONArray arrayData) {
	_fromId = fromId;

	int newArray = 0;

	byte *data = _vm->defineArray(0, type, dim2start, dim2end, dim1start, dim1end, true, &newArray);

	int32 size = (dim1end - dim1start + 1) * (dim2end - dim2start + 1);

	int32 value;

	for (int i = 0; i < size; i++) {
		value = arrayData[i]->asIntegerNumber();

		switch (type) {
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
			error("DirectPlay::handleRemoteArray(): Unknown array type %d", type);
		}
	}

	int args[25];
	memset(args, 0, sizeof(args));

	args[0] = newArray;

	if (!_vm->VAR(_vm->VAR_NETWORK_RECEIVE_ARRAY_SCRIPT)) {
		warning("DirectPlay: VAR_NETWORK_RECEIVE_ARRAY_SCRIPT not defined!");
		return;
	}
	_vm->runScript(_vm->VAR(_vm->VAR_NETWORK_RECEIVE_ARRAY_SCRIPT), 1, 0, args);
}

}
