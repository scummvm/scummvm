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

#include "common/formats/json.h"
#include "backends/networking/curl/socket.h"

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
	void send(Common::JSONObject data);

	void openUrl(const char *url);

	bool connect();
	void disconnect(bool lost = false);
	void login(const char *userName, const char *password);
	
	void getUserProfile(int userId);
	void setIcon(int icon);
protected:
	ScummEngine_v90he *_vm;
	Common::String _gameName;
	Networking::CurlSocket *_socket;

	Common::String _buffer;

	void writeStringArray(int array, Common::String string);
	void runRemoteStartScript(int *args);
	void systemAlert(int type, Common::String message);

	void receiveData();
	void processLine(Common::String line);

	void handleHeartbeat();

	void handleLoginResp(int errorCode, int userId, Common::String response);
	void handleProfileInfo(Common::JSONArray profile);

	int _userId;
};

} // End of namespace Scumm


#endif