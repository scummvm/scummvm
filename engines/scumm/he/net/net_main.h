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

#include "backends/networking/enet/enet.h"
#include "backends/networking/enet/host.h"
#include "backends/networking/enet/socket.h"
#include "common/formats/json.h"
namespace Scumm {

class ScummEngine_v90he;

class Net {
public:
	Net(ScummEngine_v90he *vm);
	~Net();

private:
	/**
	 * An address structure which stores the host and port.
	 */
	struct Address {
		Common::String host;
		int port;
		bool operator==(const Address &other) {
			return host == other.host && port == other.port;
		};
	};
	/**
	 * Structure for session storage.
	 */
	struct Session {
		bool local = false; ///< Indicates if session is found over LAN.
		int id = -1;
		Common::String host;
		int port;
		Common::String name;
		int players;
		uint32 timestamp;
	};
	/**
	 * Converts a formatted string into an Address object.
	 * 
	 * @param address A proper formatted string e.g. "127.0.0.1:9120", it may or may not contain a port number.
	 * @return Address
	 */
	Address getAddressFromString(Common::String address);
	/**
	 * Converts an Address object into a formatted string.
	 * 
	 * @param address An address object.
	 * @return Common::String
	 */
	Common::String getStringFromAddress(Address address);

public:
	/**
	 * Creates a session and add ourselves as a user.
	 * 
	 * @param sessionName Name if a session.
	 * @param userName User name to add.
	 * @retval 1 on success.
	 * @retval 0 on falure.
	 */
	int hostGame(char *sessionName, char *userName);
	/**
	 * Joins a session with the speicified IP address and user name.
	 * This gets called when attempting to join a Moonbase Commander
	 * game with the --join-game command line param.
	 * 
	 * @param IP IP Address to join
	 * @param userName Username to use.
	 * @retval 1 on success.
	 * @retval 0 on failure.
	 */
	int joinGame(Common::String IP, char *userName);

	/**
	 * Adds a user to the session and assigns a user id.
	 * 
	 * @param shortName Short username
	 * @param longName Long username.
	 * @retval 1 on success.
	 * @retval 0 on failure.
	 */
	int addUser(char *shortName, char *longName);
	/**
	 * Remove ourselves if we have a user id for this session.
	 * 
	 * @retval Always returns 1.
	 * @see destroyPlayer
	 * 
	 */
	int removeUser();

	/**
	 * Gets the user id who sent the previously received packet.
	 * 
	 * @retval Last sent user id.
	 */
	int whoSentThis();
	/**
	 * Gets our assigned user id.
	 * 
	 * @retval Our user id.
	 */
	int whoAmI();

	/**
	 * @brief Creates and host an network game session.
	 * 
	 * @param name Session name
	 * @retval 1 on success.
	 * @retval 0 on failure.
	 */
	int createSession(char *name);
	/**
	 * Join a session by their given index.
	 * 
	 * @param sessionIndex Index of a session to join.
	 * @retval 1 on success.
	 * @retval 0 on falure.
	 * 
	 * @note Use the QuerySessions methods to get sessions.
	 */
	int joinSession(int sessionIndex);
	/**
	 * Join a session by their given id instead of index.
	 * 
	 * @param sessionId ID of session to join
	 * @retval 1 on success.
	 * @retval 0 on falure.
	 * 
	 * @note Use the QuerySessions methods to get sessions.
	 */
	int joinSessionById(int sessionId);
	/**
	 * Checks if the queried session id exist.
	 * 
	 * @param sessionId Session ID to search for.
	 * @retval 1 if found.
	 * @retval 0 if not found.
	 */
	int ifSessionExist(int sessionId);

	/**
	 * Ends and closes an active game session.
	 * 
	 * @retval Always returns 1
	 */
	int endSession();

	/**
	 * Force a session server address to connect to when creating and joining sessions,
	 * overriding an existing configuration if any.
	 * 
	 * @param sessionServer Address to a session server address.
	 * @note This will disable LAN hosting.
	 */
	void setSessionServer(Common::String sessionServer);
	/**
	 * Disallows anymore players from joining our session.
	 */
	void disableSessionJoining();
	/**
	 * Allows more players to join our session.
	 * @note Currently stubbed.
	 * 
	 */
	void enableSessionJoining();

	/**
	 * @brief Set AI Player count.
	 * 
	 * @param botsCount Number of AI players currently created.
	 */
	void setBotsCount(int botsCount);

	/**
	 * @brief Set and initializes the provider given by their name. 
	 * 
	 * @param parameter1 SCUMM string array providing the name of the provider.
	 * @param parameter2 SCUMM string array providing the optional paramater
	 * 
	 * @retval 1 if successful
	 * @retval 0 on failure.
	 * 
	 * @note Currently this will only initialize the ENet provider, regardless of name given.
	 */
	int32 setProviderByName(int32 parameter1, int32 parameter2);

	/**
	 * @brief Sets the fake latency.
	 * 
	 * @param time Fake latency time in milliseconds.
	 */
	void setFakeLatency(int time);

	/**
	 * Destroys and remove an existing player.
	 * 
	 * @param userId ID of player to remove.
	 * @retval true on removed 
	 * @retval false on failure. 
	 */
	bool destroyPlayer(int32 userId);
	/**
	 * Setup and begin to query for active game sessions
	 * 
	 * @param connectToSessionServer Indicates that it should connect to the session server for Internet-wide connections
	 * @retval 1 if ready
	 * @retval 0 if not.
	 * @see updateQuerySessions()
	 * @see stopQuerySessions()
	 */
	int32 startQuerySessions(bool connectToSessionServer = true);
	/**
	 * Make a session query request and updates the session list.
	 * 
	 * @return Number of sessions found.
	 */
	int32 updateQuerySessions();
	/**
	 * Stops and shuts down querying for sessions.
	 * @note This will clear the sessions list.
	 */
	void stopQuerySessions();
	/**
	 * Shortcut for querying sessions, calls startQuerySessions and returns updateQuerySessions.
	 * 
	 * @return Number of sessions found.
	 */
	int querySessions();

	/**
	 * Query aviliable providers.
	 * 
	 * @return Always 1 currently.
	 */
	int queryProviders();
	/**
	 * @brief Set the provider by the index.
	 * 
	 * @param providerIndex index of a provider
	 * @retval 1 if successful
	 * @retval 0 on failure
	 * 
	 * @note Currently stubbed.
	 */
	int setProvider(int providerIndex);
	/**
	 * Close and shutsdown an active provider.
	 * 
	 * @return Always returns 1.
	 */
	int closeProvider();

	/**
	 * Initializes the provider, session, and adds user all at once.
	 * 
	 * @retval true on success.
	 * @retval false on failure. 
	 * 
	 * @note Currently stubbed.
	 */
	bool initAll();
	/**
	 * Initializes the provider.
	 * 
	 * @return true on success.
	 * @return false on failure.
	 * 
	 * @note Currently stubbed.
	 */
	bool initProvider();
	/**
	 * Initializes the session.
	 * 
	 * @return true on success.
	 * @return false on failure.
	 * 
	 * @note Currently stubbed.
	 */
	bool initSession();
	/**
	 * Initializes the user.
	 * 
	 * @return true on success.
	 * @return false on failure.
	 * 
	 * @note Currently stubbed.
	 */
	bool initUser();

/**
	 * Sends a packet to a remote peer(s) which will call VAR_REMOTE_START_SCRIPT.
	 * 
	 * @param typeOfSend A type of send this packet goes to, can be an indiviual peer, a group, host, or everybody.
	 * @param sendTypeParam A parameter for this type of send, e.g. for an indiviual send, this can be a user id of a peer to send to.
	 * @param priority Tells the provider to ensure that this packet has been sent and received.
	 * @param argsCount Number of args it should contain for the VAR_REMOTE_START_SCRIPT call.
	 * @param args The arguments themselves.
	 */
	void remoteStartScript(int typeOfSend, int sendTypeParam, int priority, int argsCount, int32 *args);
	/**
	 * Sends an SCUMM array to a remote peer(s), calling VAR_NETWORK_RECEIVE_ARRAY_SCRIPT there.
	 * 
	 * @param typeOfSend A type of send this packet goes to, can be an indiviual peer, a group, host, or everybody.
	 * @param sendTypeParam A parameter for this type of send, e.g. for an indiviual send, this can be a user id of a peer to send to.
	 * @param priority Tells the provider to ensure that this packet has been sent and received.
	 * @param arrayIndex An index pointing to an SCUMM array to pack and send with.
	 */
	void remoteSendArray(int typeOfSend, int sendTypeParam, int priority, int arrayIndex);

	/**
	 * Sends a packet to a peer calling VAR_NETWORK_RECEIVE_ARRAY_SCRIPT, and it'll return its return value back to us.
	 * 
	 * @param typeOfSend A type of send this packet goes to, can be an indiviual peer, a group, host, or everybody.
	 * @param sendTypeParam A parameter for this type of send, e.g. for an indiviual send, this can be a user id of a peer to send to.
	 * @param priority Tells the provider to ensure that this packet has been sent and received.
	 * @param defaultReturnValue The default return value to return in case of a time out.
	 * @param argsCount Number of args it should contain for the VAR_REMOTE_START_SCRIPT call.
	 * @param args The arguments themselves.
	 * @retval A peer's return value.
	 * @retval defaultReturnValue if timed out.
	 * 
	 * @note This has been stubbed, as no game seems to use this.
	 */
	int remoteStartScriptFunction(int typeOfSend, int sendTypeParam, int priority, int defaultReturnValue, int argsCount, int32 *args);

	/**
	 * A method that should be called once at the beginning of a game loop to send/receive network data. 
	 * 
	 * @param msecs milliseconds to service networks for.
	 */
	void doNetworkOnceAFrame(int msecs);

private:
	/**
	 * Attempt to connect to a game session with its address and port.
	 * 
	 * @param address Address of an session to connect to.
	 * @param port Port number of an session to connect to.
	 * @retval true on success 
	 * @retval false on failure.
	 * 
	 * @see joinGame
	 * @see doJoinSession
	 */
	bool connectToSession(Common::String address, int port);

	/**
	 * Method that actually attemps to join a session.
	 * 
	 * @param session Session structure to join to.
	 * @retval 1 on success
	 * @retval 0 on failure.
	 * 
	 * @see joinSession
	 * @see joinSessionById
	 */
	int doJoinSession(Session session);

	/**
	 * Sends remote data to peer(s).
	 * 
	 * @param typeOfSend A type of send this packet goes to, can be an indiviual peer, a group, host, or everybody.
	 * @param sendTypeParam A parameter for this type of send, e.g. for an indiviual send, this can be a user id of a peer to send to.
	 * @param type Type of packet.
	 * @param data Data of contain in the packet.
	 * @param priority Tells the provider to ensure that this packet has been sent and received. 
	 * @param defaultRes Default return value (0 by default)
	 * @param wait Wait for return value (Not currently being used).
	 * @param callid Call ID of this packet send. (Not currently being used).
	 * @return Always the default return value currently.
	 */
	int remoteSendData(int typeOfSend, int sendTypeParam, int type, Common::String data, int priority, int defaultRes = 0, bool wait = false, int callid = 0);

	/**
	 * Services the broadcast (LAN) socket.
	 * 
	 * @return true 
	 * @return false 
	 */
	bool serviceBroadcast();
	/**
	 * Handles the data received from the broadcast (LAN) socket.
	 * 
	 * @param data Data received.
	 * @param host Host name who sent the data.
	 * @param port Port number who sent the data.
	 */
	void handleBroadcastData(Common::String data, Common::String host, int port);
	/**
	 * Servies the connection to the session server.
	 */
	void serviceSessionServer();
	/**
	 * Handles data received from the session server.
	 * 
	 * @param data Data received.
	 */
	void handleSessionServerData(Common::String data);
	/**
	 * Service sesssion host.
	 */
	void remoteReceiveData();
	/**
	 * Handle game data received from session host.
	 * 
	 * @param json JSON object containing game data.
	 * @param peerIndex Index of a peer who sent this data.
	 */
	void handleGameData(Common::JSONValue *json, int peerIndex);
	/**
	 * Handle game data received and transfer to peers if needed.
	 * 
	 * @param json JSON object containing game data.
	 * @param peerIndex Index of a peer who sent this data.
	 * 
	 * @see handleGameData
	 */
	void handleGameDataHost(Common::JSONValue *json, int peerIndex);

public:
	// getters

	/**
	 * Gets the host name of the local machine.
	 * 
	 * @param hostname Pointer to store the host name to.
	 * @param length The length of the pointer.
	 * @retval true on successful .
	 * @retval false on failure.
	 * 
	 * @note This is currently subbed.
	 */
	bool getHostName(char *hostname, int length);
	/**
	 * Gets the IP address of the given local host name.
	 * 
	 * @param ip Pointer to store the address to.
	 * @param ipLength The length of the pointer.
	 * @param nameBuffer the Host name itself.
	 * @retval true on successful .
	 * @retval false on failure..
	 * 
	 * @note This is currently subbed.
	 */
	bool getIPfromName(char *ip, int ipLength, char *nameBuffer);
	/**
	 * Get the session name from the session index.
	 * 
	 * @param sessionNumber The session's index.
	 * @param buffer A buffer to store the name to.
	 * @param length Maxinum length of the buffer.
	 */
	void getSessionName(int sessionNumber, char *buffer, int length);
	/**
	 * Get the session's player count.
	 * 
	 * @param sessionNumber The session's index.
	 * @return Player count.
	 */
	int getSessionPlayerCount(int sessionNumber);
	/**
	 * @brief Get the name of a provider of the given index.
	 * 
	 * @param providerIndex 
	 * @param buffer 
	 * @param length 
	 */
	void getProviderName(int providerIndex, char *buffer, int length);

private:
	// mostly getters

	/**
	 * @brief Get total players joined in the session, including AI players.
	 * 
	 * @return Player count.
	 */
	int getTotalPlayers();

public:
	// fields
	int _latencyTime; // ms
	bool _fakeLatency;

	bool _isHost; // true = hosting game, false = joined game.

	int _myUserId;
	int _fromUserId;

	int _sessionId; // Session ID received from the session server.

private:
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

	Common::String _sessionName;
	Networking::Host *_sessionHost;

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
	bool _forcedAddress;
	bool _gotSessions;
	int _sessionServerPeer;
	bool _isRelayingGame; ///< If we're relaying in-game data over the session server or not.
};

} // End of namespace Scumm

#endif
