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

#include "backends/networking/curl/connectionmanager.h"

#include "scumm/he/intern_he.h"
#include "scumm/he/moonbase/moonbase.h"
#include "scumm/he/moonbase/net_main.h"
#include "scumm/he/moonbase/net_defines.h"

namespace Scumm {

Net::Net(ScummEngine_v100he *vm) : _latencyTime(1), _fakeLatency(false), _vm(vm) {
	//some defaults for fields

	_packbuffer = (byte *)malloc(MAX_PACKET_SIZE + DATA_HEADER_SIZE);
	_tmpbuffer = (byte *)malloc(MAX_PACKET_SIZE);

	_myUserId = -1;
	_myPlayerKey = -1;
	_lastResult = 0;

	_sessionsBeingQueried = false;

	_sessionid = -1;
	_sessions = nullptr;
	_packetdata = nullptr;

	_serverprefix = "http://localhost/moonbase";
}

Net::~Net() {
	free(_tmpbuffer);
	free(_packbuffer);

	delete _sessions;
	delete _packetdata;
}

int Net::hostGame(char *sessionName, char *userName) {
	if (createSession(sessionName)) {
		if (addUser(userName, userName)) {
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

int Net::joinGame(char *IP, char *userName) {
	warning("STUB: Net::joinGame(\"%s\", \"%s\")", IP, userName); // PN_JoinTCPIPGame
	return 0;
}

int Net::addUser(char *shortName, char *longName) {
	debug(1, "Net::addUser(\"%s\", \"%s\")", shortName, longName); // PN_AddUser

	Networking::PostRequest rq(_serverprefix + "/adduser",
		new Common::Callback<Net, Common::JSONValue *>(this, &Net::addUserCallback),
		new Common::Callback<Net, Networking::ErrorResponse>(this, &Net::addUserErrorCallback));

	char *buf = (char *)malloc(MAX_PACKET_SIZE);
	snprintf(buf, MAX_PACKET_SIZE, "{\"shortname\":\"%s\",\"longname\":\"%s\",\"sessionid\":%d}", shortName, longName, _sessionid);
	rq.setPostData((byte *)buf, strlen(buf));
	rq.setContentType("application/json");

	rq.start();

	_myUserId = -1;

	while(rq.state() == Networking::PROCESSING) {
		g_system->delayMillis(5);
	}

	if (_myUserId == -1)
		return 0;

	return 1;
}

void Net::addUserCallback(Common::JSONValue *response) {
	Common::JSONObject info = response->asObject();

	if (info.contains("userid")) {
		_myUserId = info["userid"]->asIntegerNumber();
		_myPlayerKey = info["playerkey"]->asIntegerNumber();
	}
	debug(1, "addUserCallback: got: '%s' as %d", response->stringify().c_str(), _myUserId);
}

void Net::addUserErrorCallback(Networking::ErrorResponse error) {
	warning("Error in addUser(): %ld %s", error.httpResponseCode, error.response.c_str());
}

int Net::removeUser() {
	debug(1, "Net::removeUser()"); // PN_RemoveUser

	if (_myUserId != -1)
		destroyPlayer(_myUserId);

	return 1;
}

int Net::whoSentThis() {
	debug(1, "Net::whoSentThis()"); // PN_WhoSentThis
	return _packetdata->child("from")->asIntegerNumber();
}

int Net::whoAmI() {
	debug(1, "Net::whoAmI()"); // PN_WhoAmI

	return _myUserId;
}

int Net::createSession(char *name) {
	debug(1, "Net::createSession(\"%s\")", name); // PN_CreateSession

	Networking::PostRequest rq(_serverprefix + "/createsession",
		new Common::Callback<Net, Common::JSONValue *>(this, &Net::createSessionCallback),
		new Common::Callback<Net, Networking::ErrorResponse>(this, &Net::createSessionErrorCallback));

	char *buf = (char *)malloc(MAX_PACKET_SIZE);
	snprintf(buf, MAX_PACKET_SIZE, "{\"name\":\"%s\"}", name);
	rq.setPostData((byte *)buf, strlen(buf));
	rq.setContentType("application/json");

	rq.start();

	_sessionid = -1;

	while(rq.state() == Networking::PROCESSING) {
		g_system->delayMillis(5);
	}

	if (_sessionid == -1)
		return 0;

	return 1;
}

void Net::createSessionCallback(Common::JSONValue *response) {
	Common::JSONObject info = response->asObject();

	if (info.contains("sessionid")) {
		_sessionid = info["sessionid"]->asIntegerNumber();
	}
	debug(1, "createSessionCallback: got: '%s' as %d", response->stringify().c_str(), _sessionid);
}

void Net::createSessionErrorCallback(Networking::ErrorResponse error) {
	warning("Error in createSession(): %ld %s", error.httpResponseCode, error.response.c_str());
}

int Net::joinSession(int sessionIndex) {
	debug(1, "Net::joinSession(%d)", sessionIndex); // PN_JoinSession

	if (!_sessions) {
		warning("Net::joinSession(): no sessions");
		return 0;
	}

	if (sessionIndex >= (int)_sessions->countChildren()) {
		warning("Net::joinSession(): session number too big: %d >= %lu", sessionIndex, _sessions->countChildren());
		return 0;
	}

	if (!_sessions->child(sessionIndex)->hasChild("sessionid")) {
		warning("Net::joinSession(): no sessionid in session");
		return 0;
	}

	_sessionid = _sessions->child(sessionIndex)->child("sessionid")->asIntegerNumber();

	return 1;
}

int Net::endSession() {
	debug(1, "Net::endSession()"); // PN_EndSession

	Networking::PostRequest rq(_serverprefix + "/endsession",
		new Common::Callback<Net, Common::JSONValue *>(this, &Net::endSessionCallback),
		new Common::Callback<Net, Networking::ErrorResponse>(this, &Net::endSessionErrorCallback));

	char *buf = (char *)malloc(MAX_PACKET_SIZE);
	snprintf(buf, MAX_PACKET_SIZE, "{\"sessionid\":%d, \"userid\":%d}", _sessionid, _myUserId);
	rq.setPostData((byte *)buf, strlen(buf));
	rq.setContentType("application/json");

	rq.start();

	while(rq.state() == Networking::PROCESSING) {
		g_system->delayMillis(5);
	}

	return _lastResult;
}

void Net::endSessionCallback(Common::JSONValue *response) {
	_lastResult = 1;
}

void Net::endSessionErrorCallback(Networking::ErrorResponse error) {
	warning("Error in endSession(): %ld %s", error.httpResponseCode, error.response.c_str());

	_lastResult = 0;
}


void Net::disableSessionJoining() {
	debug(1, "Net::disableSessionJoining()"); // PN_DisableSessionPlayerJoin

	Networking::PostRequest *rq = new Networking::PostRequest(_serverprefix + "/disablesession",
		nullptr,
		new Common::Callback<Net, Networking::ErrorResponse>(this, &Net::disableSessionJoiningErrorCallback));

	char *buf = (char *)malloc(MAX_PACKET_SIZE);
	snprintf(buf, MAX_PACKET_SIZE, "{\"sessionid\":%d}", _sessionid);
	rq->setPostData((byte *)buf, strlen(buf));
	rq->setContentType("application/json");

	rq->start();

	ConnMan.addRequest(rq);
}

void Net::disableSessionJoiningErrorCallback(Networking::ErrorResponse error) {
	warning("Error in disableSessionJoining(): %ld %s", error.httpResponseCode, error.response.c_str());
}

void Net::enableSessionJoining() {
	warning("STUB: Net::enableSessionJoining()"); // PN_EnableSessionPlayerJoin
}

void Net::setBotsCount(int botsCount) {
	warning("STUB: Net::setBotsCount(%d)", botsCount); // PN_SetAIPlayerCountKludge
}

int32 Net::setProviderByName(int32 parameter1, int32 parameter2) {
	char name[MAX_PROVIDER_NAME];
	char ipaddress[MAX_IP_SIZE];

	ipaddress[0] = '\0';

	_vm->getStringFromArray(parameter1, name, sizeof(name));
	if (parameter2)
		_vm->getStringFromArray(parameter2, ipaddress, sizeof(ipaddress));

	debug(1, "Net::setProviderByName(\"%s\", \"%s\")", name, ipaddress); // PN_SetProviderByName

	// Emulate that we found a TCP/IP provider

	return 1;
}

void Net::setFakeLatency(int time) {
	_latencyTime = time;
	debug("NETWORK: Setting Fake Latency to %d ms", _latencyTime);
	_fakeLatency = true;
}

bool Net::destroyPlayer(int32 playerDPID) {
	// bool PNETWIN_destroyplayer(DPID idPlayer)
	debug(1, "Net::destroyPlayer(%d)", playerDPID);

	Networking::PostRequest *rq = new Networking::PostRequest(_serverprefix + "/removeuser",
		nullptr,
		new Common::Callback<Net, Networking::ErrorResponse>(this, &Net::destroyPlayerErrorCallback));

	char *buf = (char *)malloc(MAX_PACKET_SIZE);
	snprintf(buf, MAX_PACKET_SIZE, "{\"sessionid\":%d, \"userid\":%d}", _sessionid, playerDPID);
	rq->setPostData((byte *)buf, strlen(buf));
	rq->setContentType("application/json");

	rq->start();

	ConnMan.addRequest(rq);

	return true;
}

void Net::destroyPlayerErrorCallback(Networking::ErrorResponse error) {
	warning("Error in destroyPlayer(): %ld %s", error.httpResponseCode, error.response.c_str());
}

int32 Net::startQuerySessions() {
	if (!_sessionsBeingQueried) { // Do not run parallel queries
		debug(1, "Net::startQuerySessions()"); // StartQuerySessions

		Networking::PostRequest *rq = new Networking::PostRequest(_serverprefix + "/lobbies",
			new Common::Callback<Net, Common::JSONValue *>(this, &Net::startQuerySessionsCallback),
			new Common::Callback<Net, Networking::ErrorResponse>(this, &Net::startQuerySessionsErrorCallback));

		_sessionsBeingQueried = true;

		rq->start();

		ConnMan.addRequest(rq);
	}

	if (!_sessions)
		return 0;

	debug(1, "Net::startQuerySessions(): got %lu", _sessions->countChildren());

	return _sessions->countChildren();
}

void Net::startQuerySessionsCallback(Common::JSONValue *response) {
	debug(1, "startQuerySessions: Got: '%s' which is %lu", response->stringify().c_str(), response->countChildren());

	_sessionsBeingQueried = false;

	delete _sessions;

	_sessions = new Common::JSONValue(*response);
}

void Net::startQuerySessionsErrorCallback(Networking::ErrorResponse error) {
	warning("Error in startQuerySessions(): %ld %s", error.httpResponseCode, error.response.c_str());

	_sessionsBeingQueried = false;
}

int32 Net::updateQuerySessions() {
	debug(1, "Net::updateQuerySessions()"); // UpdateQuerySessions
	return startQuerySessions();
}

void Net::stopQuerySessions() {
	debug(1, "Net::stopQuerySessions()"); // StopQuerySessions

	_sessionsBeingQueried = false;
	// No op
}

int Net::querySessions() {
	warning("STUB: Net::querySessions()"); // PN_QuerySessions
	return 0;
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
			res += Common::String::format("%d, ", args[i]);

	if (argsCount > 1)
		res += Common::String::format("%d]", args[argsCount - 1]);
	else
		res += "]";

	debug(1, "Net::remoteStartScript(%d, %d, %d, %d, ...)", typeOfSend, sendTypeParam, priority, argsCount); // PN_RemoteStartScriptCommand

	remoteSendData(typeOfSend, sendTypeParam, PACKETTYPE_REMOTESTARTSCRIPT, res);
}

int Net::remoteSendData(int typeOfSend, int sendTypeParam, int type, Common::String data, int defaultRes, bool wait, int callid) {
	// Since I am lazy, instead of constructing the JSON object manually
	// I'd rather parse it
	Common::String res = Common::String::format(
		"{\"sessionid\":%d, \"from\":%d, \"to\":%d, \"toparam\": %d, "
		"\"type\":%d, \"timestamp\": %d, \"size\": 1, \"data\": { %s } }", _sessionid, _myUserId,
		typeOfSend, sendTypeParam, type, g_system->getMillis(), data.c_str());

	byte *buf = (byte *)malloc(res.size() + 1);
	strncpy((char *)buf, res.c_str(), res.size());

	debug(2, "Package to send: %s", res.c_str());

	Networking::PostRequest *rq = new Networking::PostRequest(_serverprefix + "/packet",
		nullptr,
		new Common::Callback<Net, Networking::ErrorResponse>(this, &Net::remoteSendDataErrorCallback));

	rq->setPostData(buf, res.size());
	rq->setContentType("application/json");

	rq->start();

	ConnMan.addRequest(rq);

	if (!wait)
		return 0;

	uint32 timeout = g_system->getMillis() + 1000;

	while (g_system->getMillis() < timeout) {
		if (remoteReceiveData()) {
			if (_packetdata->child("data")->hasChild("callid")) {
				if (_packetdata->child("data")->child("callid")->asIntegerNumber() == callid) {
					return _packetdata->child("data")->child("result")->asIntegerNumber();
				}
			}

			warning("Net::remoteSendData(): Received wrong package: %s", _packetdata->stringify().c_str());
		}

		_vm->parseEvents();
	}

	if (!_sessions)
		return 0;

	return defaultRes;
}

void Net::remoteSendDataErrorCallback(Networking::ErrorResponse error) {
	warning("Error in remoteSendData(): %ld %s", error.httpResponseCode, error.response.c_str());
}

void Net::remoteSendArray(int typeOfSend, int sendTypeParam, int priority, int arrayIndex) {
	debug(1, "Net::remoteSendArray(%d, %d, %d, %d)", typeOfSend, sendTypeParam, priority, arrayIndex & ~0x33539000); // PN_RemoteSendArrayCommand

	ScummEngine_v100he::ArrayHeader *ah = (ScummEngine_v100he::ArrayHeader *)_vm->getResourceAddress(rtString, arrayIndex & ~0x33539000);

	Common::String jsonData = Common::String::format(
		"\"type\":%d, \"dim1start\":%d, \"dim1end\":%d, \"dim2start\":%d, \"dim2end\":%d, \"data\": [",
		ah->type, ah->dim1start, ah->dim1end, ah->dim2start, ah->dim2end);

	int32 size = (FROM_LE_32(ah->dim1end) - FROM_LE_32(ah->dim1start) + 1) *
		(FROM_LE_32(ah->dim2end) - FROM_LE_32(ah->dim2start) + 1);

	for (int i = 0; i < size; i++) {
		int32 data;

		switch (FROM_LE_32(ah->type)) {
		case ScummEngine_v100he::kByteArray:
		case ScummEngine_v100he::kStringArray:
			data = ah->data[i];
			break;

		case ScummEngine_v100he::kIntArray:
			data = (int16)READ_LE_UINT16(ah->data + i * 2);
			break;

		case ScummEngine_v100he::kDwordArray:
			data = (int32)READ_LE_UINT32(ah->data + i * 4);
			break;

		default:
			error("Net::remoteSendArray(): Unknown array type %d for array %d", FROM_LE_32(ah->type), arrayIndex);
		}

		jsonData += Common::String::format("%d", data);

		if (i < size - 1)
			jsonData += ", ";
		else
			jsonData += "]";
	}

	remoteSendData(typeOfSend, sendTypeParam, PACKETTYPE_REMOTESENDSCUMMARRAY, jsonData);
}

int Net::remoteStartScriptFunction(int typeOfSend, int sendTypeParam, int priority, int defaultReturnValue, int argsCount, int32 *args) {
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

	if (!_sessions) {
		*buffer = '\0';
		warning("Net::getSessionName(): no sessions");
		return;
	}

	if (sessionNumber >= (int)_sessions->countChildren()) {
		*buffer = '\0';
		warning("Net::getSessionName(): session number too big: %d >= %lu", sessionNumber, _sessions->countChildren());
		return;
	}

	Common::strlcpy(buffer, _sessions->child(sessionNumber)->child("name")->asString().c_str(), length);
}

int Net::getSessionPlayerCount(int sessionNumber) {
	debug(1, "Net::getSessionPlayerCount(%d)", sessionNumber); // case GET_SESSION_PLAYER_COUNT_KLUDGE:

	if (!_sessions) {
		warning("Net::getSessionPlayerCount(): no sessions");
		return 0;
	}

	if (sessionNumber >= (int)_sessions->countChildren()) {
		warning("Net::getSessionPlayerCount(): session number too big: %d >= %lu", sessionNumber, _sessions->countChildren());
		return 0;
	}

	if (!_sessions->child(sessionNumber)->hasChild("players")) {
		warning("Net::getSessionPlayerCount(): no players in session");
		return 0;
	}

	return _sessions->child(sessionNumber)->child("players")->countChildren();
}

void Net::getProviderName(int providerIndex, char *buffer, int length) {
	warning("STUB: Net::getProviderName(%d, \"%s\", %d)", providerIndex, buffer, length); // PN_GetProviderName
}

bool Net::remoteReceiveData() {
	Networking::PostRequest rq(_serverprefix + "/getpacket",
		new Common::Callback<Net, Common::JSONValue *>(this, &Net::remoteReceiveDataCallback),
		new Common::Callback<Net, Networking::ErrorResponse>(this, &Net::remoteReceiveDataErrorCallback));

	char *buf = (char *)malloc(MAX_PACKET_SIZE);
	snprintf(buf, MAX_PACKET_SIZE, "{\"sessionid\":%d, \"playerid\":%d}", _sessionid, _myUserId);
	rq.setPostData((byte *)buf, strlen(buf));
	rq.setContentType("application/json");

	delete _packetdata;
	_packetdata = nullptr;

	rq.start();

	while(rq.state() == Networking::PROCESSING) {
		g_system->delayMillis(5);
	}

	if (!_packetdata || _packetdata->child("size")->asIntegerNumber() == 0)
		return false;

	uint from = _packetdata->child("from")->asIntegerNumber();
	uint type = _packetdata->child("type")->asIntegerNumber();

	uint32 *params;

	switch (type) {
	case PACKETTYPE_REMOTESTARTSCRIPT:
		{
			int datalen = _packetdata->child("data")->child("params")->asArray().size();
			params = (uint32 *)_tmpbuffer;

			for (int i = 0; i < datalen; i++) {
				*params = _packetdata->child("data")->child("params")->asArray()[i]->asIntegerNumber();
				params++;
			}

			_vm->runScript(_vm->VAR(_vm->VAR_REMOTE_START_SCRIPT), 1, 0, (int *)_tmpbuffer);
		}
		break;

	case PACKETTYPE_REMOTESTARTSCRIPTRETURN:
		{
			int datalen = _packetdata->child("data")->child("params")->asArray().size();
			params = (uint32 *)_tmpbuffer;

			for (int i = 0; i < datalen; i++) {
				*params = _packetdata->child("data")->child("params")->asArray()[i]->asIntegerNumber();
				params++;
			}

			_vm->runScript(_vm->VAR(_vm->VAR_REMOTE_START_SCRIPT), 1, 0, (int *)_tmpbuffer);
			int result = _vm->pop();

			Common::String res = Common::String::format("\"result\": %d, \"callid\": %d", result,
					(int)_packetdata->child("data")->child("callid")->asIntegerNumber());

			remoteSendData(PN_SENDTYPE_INDIVIDUAL, from, PACKETTYPE_REMOTESTARTSCRIPTRESULT, res);
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

			int dim1start = _packetdata->child("data")->child("dim1start")->asIntegerNumber();
			int dim1end   = _packetdata->child("data")->child("dim1end")->asIntegerNumber();
			int dim2start = _packetdata->child("data")->child("dim2start")->asIntegerNumber();
			int dim2end   = _packetdata->child("data")->child("dim2end")->asIntegerNumber();
			int atype     = _packetdata->child("data")->child("type")->asIntegerNumber();

			byte *data = _vm->defineArray(0, atype, dim2start, dim2end, dim1start, dim1end, true, &newArray);

			int32 size = (dim1end - dim1start + 1) * (dim2end - dim2start + 1);

			int32 value;

			for (int i = 0; i < size; i++) {
				value = _packetdata->child("data")->child("data")->asArray()[i]->asIntegerNumber();

				switch (atype) {
				case ScummEngine_v100he::kByteArray:
				case ScummEngine_v100he::kStringArray:
					data[i] = value;
					break;

				case ScummEngine_v100he::kIntArray:
					WRITE_LE_UINT16(data + i * 2, value);
					break;

				case ScummEngine_v100he::kDwordArray:
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
		warning("Moonbase: Received unknown network command %d", type);
	}

	return true;
}

void Net::remoteReceiveDataCallback(Common::JSONValue *response) {
	_packetdata = new Common::JSONValue(*response);

	if (_packetdata->child("size")->asIntegerNumber() != 0)
		debug(1, "remoteReceiveData: Got: '%s'", response->stringify().c_str());
}

void Net::remoteReceiveDataErrorCallback(Networking::ErrorResponse error) {
	warning("Error in remoteReceiveData(): %ld %s", error.httpResponseCode, error.response.c_str());
}


void Net::doNetworkOnceAFrame(int msecs) {
	if (_sessionid == -1 || _myUserId == -1)
		return;

	uint32 tickCount = g_system->getMillis() + msecs;

	while (remoteReceiveData()) {
		if (tickCount >= g_system->getMillis()) {
			break;
		}
	}
}

} // End of namespace Scumm
