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

#ifndef SCUMM_HE_NET_MAIN_H
#define SCUMM_HE_NET_MAIN_H

#include "common/formats/json.h"
#include "backends/networking/enet/enet.h"
#include "backends/networking/enet/host.h"
#include "backends/networking/enet/socket.h"
namespace Scumm {

class ScummEngine_v90he;

class Net {
public:
	Net(ScummEngine_v90he *vm);
	~Net();

private:
	struct Address {
		Common::String host;
		int port;
		bool operator==(const Address &other) {
			return host == other.host && port == other.port;
		};
	};

	struct Session {
		bool local = false;
		int id = -1;
		Common::String host;
		int port;
		Common::String name;
		int players;
		uint32 timestamp;
	};

	Address getAddressFromString(Common::String address);
	Common::String getStringFromAddress(Address address);
public:
	int hostGame(char *sessionName, char *userName);
	int joinGame(Common::String IP, char *userName);
	int addUser(char *shortName, char *longName);
	int removeUser();
	int whoSentThis();
	int whoAmI();
	int createSession(char *name);
	int joinSession(int sessionIndex);
	int joinSessionById(int sessionId);
	int ifSessionExist(int sessionId);
	int endSession();
	void setSessionServer(Common::String sessionServer);
	void disableSessionJoining();
	void enableSessionJoining();
	void setBotsCount(int botsCount);
	int32 setProviderByName(int32 parameter1, int32 parameter2);
	void setFakeLatency(int time);
	bool destroyPlayer(int32 userId);
	int32 startQuerySessions(bool connectToSessionServer = true);
	int32 updateQuerySessions();
	void stopQuerySessions();
	int querySessions();
	int queryProviders();
	int setProvider(int providerIndex);
	int closeProvider();
	bool initAll();
	bool initProvider();
	bool initSession();
	bool initUser();
	void remoteStartScript(int typeOfSend, int sendTypeParam, int priority, int argsCount, int32 *args);
	int remoteSendData(int typeOfSend, int sendTypeParam, int type, Common::String data, int priority, int defaultRes = 0, bool wait = false, int callid = 0);
	void remoteSendArray(int typeOfSend, int sendTypeParam, int priority, int arrayIndex);
	int remoteStartScriptFunction(int typeOfSend, int sendTypeParam, int priority, int defaultReturnValue, int argsCount, int32 *args);
	void doNetworkOnceAFrame(int msecs);
	void handleGameData(Common::JSONValue *json, int peerIndex);
	void handleGameDataHost(Common::JSONValue *json, int peerIndex);

private:
	bool connectToSession(Common::String address, int port);
	int doJoinSession(Session session);
	bool serviceBroadcast();
	void handleBroadcastData(Common::String data, Common::String host, int port);
	void serviceSessionServer();
	void handleSessionServerData(Common::String data);
	bool remoteReceiveData(uint32 tickCount);

public:
	//getters
	bool getHostName(char *hostname, int length);
	bool getIPfromName(char *ip, int ipLength, char *nameBuffer);
	void getSessionName(int sessionNumber, char *buffer, int length);
	int getSessionPlayerCount(int sessionNumber);
	void getProviderName(int providerIndex, char *buffer, int length);

private:
	//mostly getters
	int getTotalPlayers();

public:
	//fields
	int _latencyTime; // ms
	bool _fakeLatency;

	ScummEngine_v90he *_vm;

	Common::String _gameName;
	Common::String _gameVersion;

	Networking::ENet *_enet;

	byte *_tmpbuffer;

	int _numUsers;
	int _numBots;
	int _maxPlayers;
	int _userIdCounter;
	Common::HashMap<int, Common::String> _userIdToName;
	Common::HashMap<int, int> _userIdToPeerIndex;
	Common::HashMap<int, Common::String> _userIdToAddress;
	Common::HashMap<Common::String, int> _addressToUserId;

	int _myUserId;
	int _fromUserId;

	int _sessionId; // Session ID received from the session server.
	Common::String _sessionName;
	Networking::Host *_sessionHost;

	bool _isHost;  // true = hosting game, false = joined game.
	bool _isShuttingDown;

	Common::Queue<Common::JSONValue *> _hostDataQueue;
	Common::Queue<int> _peerIndexQueue;

	Common::Array<Session> _sessions;
	int _hostPort;

	// For broadcasting our game session over LAN.
	Networking::Socket *_broadcastSocket;

	// For creating/joining sessions over the Internet.
	Networking::Host *_sessionServerHost;
	Address _sessionServerAddress;
	bool _gotSessions;
	int _sessionServerPeer;
	bool _isRelayingGame; // If we're relaying in-game data over the session server or not.
};

} // End of namespace Scumm

#endif