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
	_lastResult = 0;

	_sessionid = -1;
	_sessions = nullptr;
	_packetdata = nullptr;

	_serverprefix = "http://localhost/moonbase";
}

Net::~Net() {
	free(_tmpbuffer);
	free(_packbuffer);

	delete _sessions;
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
	}
	debug(1, "addUserCallback: got: '%s' as %d", response->stringify().c_str(), _myUserId);
}

void Net::addUserErrorCallback(Networking::ErrorResponse error) {
	warning("Error in addUser(): %ld %s", error.httpResponseCode, error.response.c_str());
}

int Net::removeUser() {
	warning("STUB: Net::removeUser()"); // PN_RemoveUser
	return 0;
}

int Net::whoSentThis() {
	warning("STUB: Net::whoSentThis()"); // PN_WhoSentThis
	return 0;
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
	warning("STUB: Net::endSession()"); // PN_EndSession
	return 0;
}

void Net::disableSessionJoining() {
	warning("STUB: Net::disableSessionJoining()"); // PN_DisableSessionPlayerJoin
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
	debug("NETWORK: Setting Fake Latency to %d ms \n", _latencyTime); // TODO: is it OK to use debug instead of SPUTM_xprintf?
	_fakeLatency = true;
}

bool Net::destroyPlayer(int32 playerDPID) {
	// bool PNETWIN_destroyplayer(DPID idPlayer)
	warning("STUB: Net::destroyPlayer(%d)", playerDPID);
	return false;
}

int32 Net::startQuerySessions() {
	debug(1, "Net::startQuerySessions()"); // StartQuerySessions

	Networking::PostRequest rq(_serverprefix + "/lobbies",
		new Common::Callback<Net, Common::JSONValue *>(this, &Net::startQuerySessionsCallback),
		new Common::Callback<Net, Networking::ErrorResponse>(this, &Net::startQuerySessionsErrorCallback));

	delete _sessions;

	rq.start();

	while(rq.state() == Networking::PROCESSING) {
		g_system->delayMillis(5);
	}

	if (!_sessions)
		return 0;

	debug(1, "Net::startQuerySessions(): got %lu", _sessions->countChildren());

	return _sessions->countChildren();
}

void Net::startQuerySessionsCallback(Common::JSONValue *response) {
	debug(1, "startQuerySessions: Got: '%s' which is %lu", response->stringify().c_str(), response->countChildren());

	_sessions = new Common::JSONValue(*response);
}

void Net::startQuerySessionsErrorCallback(Networking::ErrorResponse error) {
	warning("Error in startQuerySessions(): %ld %s", error.httpResponseCode, error.response.c_str());
}

int32 Net::updateQuerySessions() {
	warning("STUB: Net::updateQuerySessions()"); // UpdateQuerySessions
	return startQuerySessions();
}

void Net::stopQuerySessions() {
	debug(1, "Net::stopQuerySessions()"); // StopQuerySessions

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
	byte *ptr = _tmpbuffer;
	for (int i = 0; i < argsCount; i++, ptr += 4)
		WRITE_UINT32(ptr, args[i]);

	warning("STUB: Net::remoteStartScript(%d, %d, %d, %d, ...)", typeOfSend, sendTypeParam, priority, argsCount); // PN_RemoteStartScriptCommand

	remoteSendData(typeOfSend, sendTypeParam, PACKETTYPE_REMOTESTARTSCRIPTRETURN, _tmpbuffer, argsCount * 4, 0);
}

int Net::remoteSendData(int typeOfSend, int sendTypeParam, int type, byte *data, int len, int defaultRes) {
	// Since I am lazy, instead of constructing the JSON object manually
	// I'd rather parse it
	Common::String res = Common::String::format(
		"{\"sessionid\":%d, \"userid\":%d, \"to\":%d, \"toparam\": %d, "
		"\"type\":%d, \"timestamp\": %d, \"size\": %d, \"data\": [", _sessionid, _myUserId,
		typeOfSend, sendTypeParam, type, g_system->getMillis(), len);

	for (int i = 0; i < len - 1; i++)
		res += Common::String::format("%d, ", data[i]);

	res += Common::String::format("%d] }", data[len - 1]);

	byte *buf = (byte *)malloc(res.size() + 1);
	strncpy((char *)buf, res.c_str(), res.size());

	debug("Package to send: %s", res.c_str());

	Networking::PostRequest rq(_serverprefix + "/packet",
		new Common::Callback<Net, Common::JSONValue *>(this, &Net::remoteSendDataCallback),
		new Common::Callback<Net, Networking::ErrorResponse>(this, &Net::remoteSendDataErrorCallback));

	rq.setPostData(buf, res.size());
	rq.setContentType("application/json");

	rq.start();

	while(rq.state() == Networking::PROCESSING) {
		g_system->delayMillis(5);
	}

	if (!_sessions)
		return 0;

	return defaultRes;
}

void Net::remoteSendDataCallback(Common::JSONValue *response) {
	debug(1, "remoteSendData: Got: '%s'", response->stringify().c_str());

	_sessions = new Common::JSONValue(*response);
}

void Net::remoteSendDataErrorCallback(Networking::ErrorResponse error) {
	warning("Error in remoteSendData(): %ld %s", error.httpResponseCode, error.response.c_str());
}

void Net::remoteSendArray(int typeOfSend, int sendTypeParam, int priority, int arrayIndex) {
	byte *arr = _vm->getResourceAddress(rtString, arrayIndex & ~0x33539000);
	int len = _vm->getResourceSize(rtString, arrayIndex & ~0x33539000);

	warning("STUB: Net::remoteSendArray(%d, %d, %d, %d)", typeOfSend, sendTypeParam, priority, arrayIndex & ~0x33539000); // PN_RemoteSendArrayCommand

	remoteSendData(typeOfSend, sendTypeParam, PACKETTYPE_REMOTESENDSCUMMARRAY, arr, len, 0);
}

int Net::remoteStartScriptFunction(int typeOfSend, int sendTypeParam, int priority, int defaultReturnValue, int argsCount, int32 *args) {
	byte *ptr = _tmpbuffer;
	for (int i = 0; i < argsCount; i++, ptr += 4)
		WRITE_UINT32(ptr, args[i]);

	warning("STUB: Net::remoteStartScriptFunction(%d, %d, %d, %d, %d, ...)", typeOfSend, sendTypeParam, priority, defaultReturnValue, argsCount); // PN_RemoteStartScriptFunction

	return remoteSendData(typeOfSend, sendTypeParam, PACKETTYPE_REMOTESTARTSCRIPTRETURN, _tmpbuffer, argsCount * 4, defaultReturnValue);
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

	int datalen = _packetdata->child("data")->asArray().size();
	for (uint i = 0; i < datalen; i++) {
		_packbuffer[i] = _packetdata->child("data")->asArray()[i]->asIntegerNumber();
	}

	Common::MemoryReadStream pack(_packbuffer, datalen);

	uint32 *params;

	switch (type) {
	case PACKETTYPE_REMOTESTARTSCRIPT:
		{
			params = (uint32 *)_tmpbuffer;

			for (int i = 0; i < 24; i++) {
				*params = pack.readUint32LE();
				params++;
			}

			_vm->runScript(_vm->VAR(_vm->VAR_REMOTE_START_SCRIPT), 1, 0, (int *)_tmpbuffer);
		}
		break;

	case PACKETTYPE_REMOTESTARTSCRIPTRETURN:
		{
			params = (uint32 *)_tmpbuffer;

			for (int i = 0; i < 24; i++) {
				*params = pack.readUint32LE();
				params++;
			}

			_vm->runScript(_vm->VAR(_vm->VAR_REMOTE_START_SCRIPT), 1, 0, (int *)_tmpbuffer);
			int res = _vm->pop();

			WRITE_UINT32(_tmpbuffer, res);

			remoteSendData(PN_SENDTYPE_INDIVIDUAL, from, PACKETTYPE_REMOTESTARTSCRIPTRESULT, _tmpbuffer, 4, 0);
		}
		break;

	case PACKETTYPE_REMOTESTARTSCRIPTRESULT:
		//
		// Ignore it.
		//

		break;

	case PACKETTYPE_REMOTESENDSCUMMARRAY:
		{
			int newArray;

			// Assume that the packet data contains a "SCUMM PACKAGE"
			// and unpack it into an scumm array :-)

			newArray = _vm->findFreeArrayId();
			unpackageArray(newArray, _packbuffer + DATA_HEADER_SIZE, datalen);
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

void Net::unpackageArray(int arrayId, byte *data, int len) {
	warning("STUB: unpackageArray");
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
