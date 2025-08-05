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

#ifndef SCUMM_HE_NET_LOBBY_H
#define SCUMM_HE_NET_LOBBY_H

#include "backends/networking/curl/socket.h"
#include "backends/networking/curl/url.h"
#include "common/formats/json.h"

#include "scumm/he/net/net_main.h"

#define MAX_USER_NAME 16

// Boneyards (Lobby) opcodes.
#define OP_NET_OPEN_WEB_URL						2121
#define OP_NET_DOWNLOAD_PLAYBOOK				2122
#define OP_NET_CONNECT 							2200
#define OP_NET_DISCONNECT 						2201
#define OP_NET_LOGIN							2202
#define OP_NET_ENTER_AREA						2204
#define OP_NET_GET_NUM_PLAYERS_IN_AREA			2205
#define OP_NET_FETCH_PLAYERS_INFO_IN_AREA		2206
#define OP_NET_GET_PLAYERS_INFO					2207
#define OP_NET_START_HOSTING_GAME				2208
#define OP_NET_CALL_PLAYER						2209
#define OP_NET_PING_OPPONENT					2211
#define OP_NET_RECEIVER_BUSY					2212
#define OP_NET_COUNTER_CHALLENGE				2213
#define OP_NET_GET_PROFILE						2214
#define OP_NET_DECLINE_CHALLENGE				2215
#define OP_NET_ACCEPT_CHALLENGE					2216
#define OP_NET_STOP_CALLING						2217
#define OP_NET_CHANGE_ICON						2218
#define OP_NET_SET_PHONE_STATUS					2220
#define OP_NET_ANSWER_PHONE						2221
#define OP_NET_LEAVE_AREA						2222
#define OP_NET_GAME_FINISHED					2223
#define OP_NET_GAME_STARTED						2224
#define OP_NET_UPDATE_PROFILE_ARRAY				2225
#define OP_NET_LOCATE_PLAYER					2226
#define OP_NET_GET_POPULATION					2227
#define OP_NET_SET_POLL_ANSWER					2228
#define OP_NET_UNKNOWN_2229						2229
// Used in baseball to get news, poll and banner.
#define OP_NET_DOWNLOAD_FILE					2238

// MAIA (Updater) opcodes.
#define OP_NET_UPDATE_INIT						3000
#define OP_NET_FETCH_UPDATES					3002

// Commands for VAR_REMOTE_START_SCRIPT. (55 in football; 324 in baseball)
#define OP_REMOTE_SYSTEM_ALERT			9911
#define OP_REMOTE_START_CONNECTION		9919
#define OP_REMOTE_RECEIVE_CHALLENGE		9920
#define OP_REMOTE_OPPONENT_ANSWERS		9921
#define OP_REMOTE_OPPONENT_BUSY			9922
#define OP_REMOTE_COUNTER_CHALLENGE		9923
#define OP_REMOTE_OPPONENT_DECLINES		9924
#define OP_REMOTE_OPPONENT_ACCEPTS		9925
#define OP_REMOTE_PING_TEST_RESULT		9927

namespace Scumm {

class ScummEngine_v90he;

class Lobby {
public:
	Lobby(ScummEngine_v90he *vm);
	~Lobby();

	void doNetworkOnceAFrame();
	void send(Common::JSONObject &data);

	int32 dispatch(int op, int numArgs, int32 *args);

	int _sessionId;
protected:
	ScummEngine_v90he *_vm;
	Common::String _gameName;
	Networking::CurlSocket *_socket;

	Common::String _buffer;

	Common::JSONArray _playersList;

	int _userId;
	Common::String _userName;
	int _playerId; // Opponent's user ID.

	int _areaIdForPopulation; // The area id we're waiting for population for (to prevent getting population for one area while wanting another).

	bool _inArea;
	int _gamesPlaying;

	bool _inGame;

	void writeStringArray(int array, Common::String string);
	void runRemoteStartScript(int *args);
	void systemAlert(int type, Common::String message);

	void receiveData();
	void processLine(Common::String line);

	void handleHeartbeat();

	void openUrl(const char *url);

	bool connect();
	void disconnect(bool lost = false);

	void login(const char *userName, const char *password);
	void handleLoginResp(int errorCode, int userId, Common::String sessionServer, Common::String response);

	void getUserProfile(int userId);
	void handleProfileInfo(Common::JSONArray profile);

	void handleTeams(Common::JSONArray userTeam, Common::JSONArray opponentTeam, int error, Common::String message);

	void downloadFile(const char *downloadPath, const char *filename);
	void handleFileData(Common::String filename, Common::String data);

	void setIcon(int icon);
	void setPollAnswer(int pollAnswer);

	void sendGameResults(int userId, int arrayIndex, int lastFlag);

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

	void pingPlayer(int playerId);
	void handlePingResult(int ping);

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

	void handleGameSession(int sessionId);

	void gameStarted(int hoster, int player, int playerNameArray);
	void gameFinished();

};

} // End of namespace Scumm

#endif
