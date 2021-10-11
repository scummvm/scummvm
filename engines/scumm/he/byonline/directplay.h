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

#ifndef SCUMM_HE_DIRECTPLAY_H
#define SCUMM_HE_DIRECTPLAY_H

typedef struct _ENetHost ENetHost;
typedef struct _ENetPeer ENetPeer;
// struct ENetAddress;
typedef struct _ENetAddress ENetAddress;

#ifdef ENABLE_HE

#include "common/json.h"

// DirectPlay opcodes.
#define OP_NET_REMOTE_START_SCRIPT			1492
#define OP_NET_QUERY_SESSIONS						1501
#define OP_NET_JOIN_SESSION							1504
#define OP_NET_END_SESSION							1505
#define OP_NET_ADD_USER									1506
#define OP_NET_WHO_SENT_THIS						1508
#define OP_NET_REMOTE_SEND_ARRAY				1509
#define OP_NET_WHO_AM_I									1510
#define OP_NET_INIT 										1513

namespace Scumm {

class DirectPlay {
public:
	DirectPlay(ScummEngine_v90he *s);
	~DirectPlay();
	void startOfFrame();

  int32 dispatch(int op, int numArgs, int32 *args);

	void hostSession(int userId);
	void joinSession(int userId, int sessionId);

	void joinRelay(int tunnelId);

private:
	ScummEngine_v90he *_vm;

	int32 init();
	void send(ENetPeer* peer, Common::JSONObject data, bool reliable = true);

	void handleSessionPacket(Common::String data);
	void handleGamePacket(Common::String data);

	void disconnect(bool dropped = false);

	bool connectToSessionServer();
	bool disconnectSessionServer();

	void handleHostResp(int sessionId, Common::String host, int port);

	void handlePeerJoin(Common::String host, int port);
	void handleJoinResp(Common::String host, int port, int hostPort);

	void startRelay();
	void handleRelayResp(int tunnelId);

	void handleRelayJoin();

	void sendRemoteStartScript(int typeOfSend, int sendTypeParam, int priority, int argsCount, int32 *args);
	void handleRemoteStartScript(int fromId, Common::JSONArray params);

	void sendRemoteArray(int typeOfSend, int sendTypeParam, int priority, int arrayIndex);
	void handleRemoteArray(int fromId, int type, int dim1start, int dim1end, int dim2start, int dim2end, Common::JSONArray arrayData);

protected:
	bool _inited;
	int _userId; // Our user ID. used by OP_NET_WHO_AM_I

	int _fromId; // Our receiving user ID.  used by OP_NET_WHO_SENT_THIS

	// Host for P2P communication
	ENetHost *_host;

	bool _hosting;

	// Connection to the session server
	ENetPeer *_sessionPeer;

	// Connection to a peer
	ENetPeer *_playerPeer;

	unsigned short _hostPort;

	Common::String _peerHost;
	unsigned short _peerPort;

	// HACK
	signed char _roomCounter;

	signed char _connectTimeout;

};

}

#endif // ENABLE_HE

#endif // SCUMM_HE_DIRECTPLAY_H
