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

#ifndef SCUMM_HE_BYONLINE_H
#define SCUMM_HE_BYONLINE_H

typedef struct _SDLNet_SocketSet *SDLNet_SocketSet;
typedef struct _TCPsocket *TCPsocket;

#ifdef ENABLE_HE

#include "common/json.h"

#include "backends/networking/curl/postrequest.h"

#include "directplay.h"

// Boneyards (Lobby) opcodes.
#define OP_NET_DOWNLOAD_PLAYBOOK						2122
#define OP_NET_CONNECT 											2200
#define OP_NET_DISCONNECT 									2201
#define OP_NET_LOGIN												2202
#define OP_NET_ENTER_AREA										2204
#define OP_NET_GET_NUM_PLAYERS_IN_AREA			2205
#define OP_NET_FETCH_PLAYERS_INFO_IN_AREA		2206
#define OP_NET_GET_PLAYERS_INFO							2207
#define OP_NET_START_HOSTING_GAME						2208
#define OP_NET_CALL_PLAYER									2209
#define OP_NET_RECEIVER_BUSY								2212
#define OP_NET_COUNTER_CHALLENGE						2213
#define OP_NET_GET_PROFILE									2214
#define OP_NET_DECLINE_CHALLENGE						2215
#define OP_NET_ACCEPT_CHALLENGE							2216
#define OP_NET_STOP_CALLING									2217
#define OP_NET_CHANGE_ICON									2218
#define OP_NET_SET_PHONE_STATUS							2220
#define OP_NET_ANSWER_PHONE									2221
#define OP_NET_LEAVE_AREA										2222
#define OP_NET_GAME_FINISHED								2223
#define OP_NET_GAME_STARTED									2224
#define OP_NET_UPDATE_PROFILE_ARRAY					2225
#define OP_NET_LOCATE_PLAYER								2226
#define OP_NET_GET_POPULATION								2227
// Used in baseball to get news, poll and banner.
#define OP_NET_DOWNLOAD_FILE								2238
// MAIA (Updater) opcodes.
#define OP_NET_UPDATE_INIT									3000
#define OP_NET_CHECK_INTERNET_STATUS				3001
#define OP_NET_FETCH_UPDATES								3002

// Commands for VAR_REMOTE_START_SCRIPT. (55 in football; 324 in baseball)
#define OP_REMOTE_SYSTEM_ALERT				9911
#define OP_REMOTE_START_CONNECTION		9919
#define OP_REMOTE_RECEIVE_CHALLENGE		9920
#define OP_REMOTE_OPPONENT_ANSWERS		9921
#define OP_REMOTE_OPPONENT_BUSY				9922
#define OP_REMOTE_COUNTER_CHALLENGE		9923
#define OP_REMOTE_OPPONENT_DECLINES		9924
#define OP_REMOTE_OPPONENT_ACCEPTS		9925
#define OP_REMOTE_PING_TEST_RESULT		9927

namespace Scumm {

class BYOnline {
public:
	BYOnline(ScummEngine_v90he *s);
	~BYOnline();

	void startOfFrame();
	int32 dispatch(int op, int numArgs, int32 *args);
	bool connected();

	// Used by DirectPlay
	void sendSession(int sessionId);
	void sendRelay(int relayId);

	void connectedToSession();

	void runRemoteStartScript(int *args);

private:
	ScummEngine_v90he *_vm;

	void writeStringArray(int array, Common::String string);
	void connect();
	void receiveData();
	void processLine(Common::String line);
	void disconnect(bool lost = false);

	void send(Common::JSONObject data);

	void handleHeartbeat();

	void login(int32 *args);
	void handleLoginResp(int errorCode, int userId, Common::String response);

	void loginCallback(Common::JSONValue *response);
	void loginErrorCallback(Networking::ErrorResponse error);

	void systemAlert(int type, Common::String message);

	void handleGetProfile(int32 *args);
	void handleProfileInfo(Common::JSONArray profile);

	void handleTeams(Common::JSONArray userTeam, Common::JSONArray opponentTeam, int error, Common::String message);

	void setProfile(Common::String field, int32 value);
	void sendGameResults(int userId, int arrayIndex, int unknown);

	void getPopulation(int areaId, int unknown);
	void handlePopulation(int areaId, int population);

	void locatePlayer(int userNameArray);
	void handleLocateResp(int code, int areaId, Common::String area);

	void enterArea(int32 areaId);
	void leaveArea();

	void getPlayersList(int start, int end);
	bool _checkPlayersLists(Common::JSONArray other);
	void handlePlayersList(Common::JSONArray playersList);
	void getPlayerInfo(int32 idx);

	void handleGamesPlaying(int games);

	void setPhoneStatus(int status);

	void sendBusy(int playerId);
	void handleReceiverBusy();

	void challengePlayer(int32 playerId, int32 stadium);
	void handleReceiveChallenge(int playerId, int stadium, Common::String name);

	void challengeTimeout(int playerId);

	int32 answerPhone(int playerId);
	void handleConsideringChallenge();

	void counterChallenge(int stadium);
	void handleCounterChallenge(int stadium);

	void declineChallenge(int playerId);
	void handleDeclineChallenge(int notResponding);

	void acceptChallenge(int playerId);
	void handleAcceptChallenge();

	void startHostingGame(int playerId);
	void handleHostGameResp(int resp);

	void handleGameSession(int sessionId);
	void handleGameRelay(int relayId);

	void gameStarted(int hoster, int player, int playerNameArray);
	void gameFinished();

protected:
	TCPsocket _socket;
	SDLNet_SocketSet _socketSet; // For checking for activity
	Common::String _buffer;

	Common::JSONArray _playersList;

	int _userId; // Our user ID. used by OP_NET_WHO_AM_I
	int _playerId; // Opponent's user ID.

	int _areaIdForPopulation; // The area id we're waiting for population for (to prevent getting population for one area while wanting another).

	bool _inArea;
	int _gamesPlaying;

	bool _inGame;
};

}

#endif // ENABLE_HE

#endif // SCUMM_HE_BYONLINE_H
