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
#ifdef USE_ENET
#include "scumm/he/net/net_main.h"
#ifdef USE_LIBCURL
#include "scumm/he/net/net_lobby.h"
#endif
#include "scumm/he/net/net_defines.h"
#endif

#include "scumm/he/logic_he.h"

// DirectPlay opcodes:
#define OP_NET_REMOTE_START_SCRIPT		1492
#define OP_NET_QUERY_PROVIDERS			1497
#define OP_NET_CLOSE_PROVIDER			1500
#define OP_NET_QUERY_SESSIONS			1501
#define OP_NET_GET_SESSION_NAME			1502
#define OP_NET_JOIN_SESSION				1504
#define OP_NET_END_SESSION				1505
#define OP_NET_ADD_USER					1506
#define OP_NET_WHO_SENT_THIS			1508
#define OP_NET_REMOTE_SEND_ARRAY		1509
#define OP_NET_INIT						1513
#define OP_NET_WHO_AM_I					1510
#define OP_NET_INIT_LAN_GAME			1515
#define OP_NET_SET_PROVIDER_BY_NAME		1516

// MAIA (Updater) opcodes.
#define OP_NET_CHECK_INTERNET_STATUS	3001
#define OP_NET_SHUT_DOWN_MAIA			3004


namespace Scumm {

/**
 * Logic code for:
 *     Backyard Baseball 2001
 */
class LogicHEbaseball2001 : public LogicHE {
public:
	LogicHEbaseball2001(ScummEngine_v90he *vm) : LogicHE(vm) {}

	int versionID() override;
	int startOfFrame() override;
	int32 dispatch(int op, int numArgs, int32 *args) override;

protected:
#ifdef USE_ENET
	void netRemoteStartScript(int numArgs, int32 *args);
	void netRemoteSendArray(int32 *args);
#endif
};

int LogicHEbaseball2001::versionID() {
	return 1;
}

int LogicHEbaseball2001::startOfFrame() {
#ifdef USE_ENET
#ifdef USE_LIBCURL
	_vm->_lobby->doNetworkOnceAFrame();
#endif
	_vm->_net->doNetworkOnceAFrame(15);
#endif
	return 0;
}

int32 LogicHEbaseball2001::dispatch(int op, int numArgs, int32 *args) {
#if defined(USE_ENET) && defined(USE_LIBCURL)
	if (op > 2120 && op < 3003 && op != OP_NET_CHECK_INTERNET_STATUS)
		return _vm->_lobby->dispatch(op, numArgs, args);
#endif

	int res = 0;

	switch (op) {
case OP_NET_INIT:
		// Initialize network system, this gets called at boot up and
		// sets VAR_NETWORK_AVAILABLE (100).  We just return a 1 if
		// ENet is compiled.
#ifdef USE_ENET
		res = 1;
#endif
		break;

#ifdef USE_ENET
	case OP_NET_REMOTE_START_SCRIPT:
		netRemoteStartScript(numArgs, args);
		break;

	case OP_NET_QUERY_SESSIONS:
#ifdef USE_LIBCURL
		if (_vm->_lobby->_sessionId) {
			_vm->_net->querySessions();
			// Only proceed if we've found the session
			// we're looking for.
			res = _vm->_net->ifSessionExist(_vm->_lobby->_sessionId);
		}
#endif
		break;

	case OP_NET_JOIN_SESSION:
#ifdef USE_LIBCURL
		if (_vm->_lobby->_sessionId) {
			res = _vm->_net->joinSessionById(_vm->_lobby->_sessionId);
			if (res) {
				_vm->_net->stopQuerySessions();
			}
		}
#endif
		break;

	case OP_NET_END_SESSION:
		res = _vm->_net->endSession();
		break;

	case OP_NET_ADD_USER:
		char userName[MAX_PLAYER_NAME];
		_vm->getStringFromArray(args[0], userName, sizeof(userName));
		res = _vm->_net->addUser(userName, userName);
		break;

	case OP_NET_WHO_SENT_THIS:
		res = _vm->_net->whoSentThis();
		break;

	case OP_NET_REMOTE_SEND_ARRAY:
		netRemoteSendArray(args);
		break;

	case OP_NET_WHO_AM_I:
		res = _vm->_net->whoAmI();
		break;
#endif // USE_ENET

	case OP_NET_CHECK_INTERNET_STATUS:
#if defined(USE_ENET) && defined(USE_LIBCURL)
		// We can only use the lobby system if both
		// libcurl (for lobby communication) and
		// ENet (for gameplay communication) is enabled.

		// TODO: Actually check if we're connected to the
		// Internet.
		res = 1;
#endif
		break;

	case OP_NET_SHUT_DOWN_MAIA:
		break;

	default:
		LogicHE::dispatch(op, numArgs, args);
	}

	return res;
}

#ifdef USE_ENET
void LogicHEbaseball2001::netRemoteStartScript(int numArgs, int32 *args) {
	_vm->_net->remoteStartScript(args[0], args[1], args[2], numArgs - 3, &args[3]);
}

void LogicHEbaseball2001::netRemoteSendArray(int32 *args) {
	_vm->_net->remoteSendArray(args[0], args[1], args[2], args[3]);
}
#endif // USE_ENET

LogicHE *makeLogicHEbaseball2001(ScummEngine_v90he *vm) {
	return new LogicHEbaseball2001(vm);
}

} // End of namespace Scumm
