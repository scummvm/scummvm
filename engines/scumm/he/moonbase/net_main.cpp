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

	_packbuffer = (byte *)malloc(MAX_PACKET_SIZE + 12);
	_tmpbuffer = (byte *)malloc(MAX_PACKET_SIZE);

	_myUserId = -1;
	_lastResult = 0;

	_sessionid = -1;
}

Net::~Net() {
	free(_tmpbuffer);
	free(_packbuffer);
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
	warning("STUB: Net::addUser(\"%s\", \"%s\")", shortName, longName); // PN_AddUser

	_myUserId = _vm->_rnd.getRandomNumber(1000000);

	// FAKE successful add. FIXME
	return 1;
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
	warning("STUB: Net::whoAmI()"); // PN_WhoAmI
	return _myUserId;
}

int Net::createSession(char *name) {
	warning("STUB: Net::createSession(\"%s\")", name); // PN_CreateSession

	Networking::PostRequest rq("http://localhost/moonbase/createsession",
		new Common::Callback<Net, Common::JSONValue *>(this, &Net::startQuerySessionsCallback),
		new Common::Callback<Net, Networking::ErrorResponse>(this, &Net::startQuerySessionsErrorCallback));

	snprintf((char *)_tmpbuffer, MAX_PACKET_SIZE, "{\"name\":\"%s\"}", name);
	rq.setPostData(_tmpbuffer, strlen((char *)_tmpbuffer));
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
	warning("Got: '%s' as %d", response->stringify().c_str(), _sessionid);
}

void Net::createSessionErrorCallback(Networking::ErrorResponse error) {
	warning("Error in createSession(): %ld %s", error.httpResponseCode, error.response.c_str());
}

int Net::joinSession(int sessionIndex) {
	warning("STUB: Net::joinSession(%d)", sessionIndex); // PN_JoinSession

	// FAKE successful join. FIXME
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
	warning("STUB: Net::startQuerySessions()"); // StartQuerySessions

	Networking::PostRequest rq("http://localhost/moonbase/lobbies",
		new Common::Callback<Net, Common::JSONValue *>(this, &Net::startQuerySessionsCallback),
		new Common::Callback<Net, Networking::ErrorResponse>(this, &Net::startQuerySessionsErrorCallback));

	rq.start();

	while(rq.state() == Networking::PROCESSING) {
		g_system->delayMillis(5);
	}

	// FAKE 1 session. FIXME
	return 1;
}

void Net::startQuerySessionsCallback(Common::JSONValue *response) {
	warning("Got: '%s'", response->stringify().c_str());
}

void Net::startQuerySessionsErrorCallback(Networking::ErrorResponse error) {
	warning("Error in startQuerySessions(): %ld %s", error.httpResponseCode, error.response.c_str());
}

int32 Net::updateQuerySessions() {
	warning("STUB: Net::updateQuerySessions()"); // UpdateQuerySessions
	return startQuerySessions();
}

void Net::stopQuerySessions() {
	warning("STUB: Net::stopQuerySessions()"); // StopQuerySessions
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
	warning("STUB: Net::closeProvider()"); // PN_CloseProvider
	return 0;
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
	WRITE_UINT32(_packbuffer, type);
	WRITE_UINT32(_packbuffer + 4, len);
	WRITE_UINT32(_packbuffer + 8, g_system->getMillis());
	memcpy(_packbuffer + 12, data, len);

	debug("Package to send, to: %d (%d), %d bytes", typeOfSend, sendTypeParam, len + 12);

	Common::hexdump(_packbuffer, len + 12);

	return defaultRes;
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
	// FIXME
	strcpy(buffer, "test");

	warning("STUB: Net::getSessionName(%d, \"%s\", %d)", sessionNumber, buffer, length); // PN_GetSessionName
}

int Net::getSessionPlayerCount(int sessionNumber) {
	warning("STUB: Net::getSessionPlayerCount(%d)", sessionNumber); // case GET_SESSION_PLAYER_COUNT_KLUDGE:
	//assert(sessionNumber >= 0 && sessionNumber < NUMELEMENTS(gdefMultiPlay.gamedescptr));
	//return gdefMultiPlay.gamedescptr[sessionNumber].currentplayers;

	// FAKE 2 players. FIXME
	return 2;
}

void Net::getProviderName(int providerIndex, char *buffer, int length) {
	warning("STUB: Net::getProviderName(%d, \"%s\", %d)", providerIndex, buffer, length); // PN_GetProviderName
}

int Net::getMessageCount() {
	return 0; // FIXME
}

void Net::remoteReceiveData() {
	// FIXME. Get data into _packbuffer
	uint type = READ_UINT32(_packbuffer);
	uint len = READ_UINT32(_packbuffer + 4);
	/*uint timestamp =*/ READ_UINT32(_packbuffer + 8);
	byte *p;
	uint32 *params;

	switch (type) {
	case PACKETTYPE_REMOTESTARTSCRIPT:
		{
			p = _packbuffer + 12;
			params = (uint32 *)_tmpbuffer;

			for (int i = 0; i < 24; i++) {
				*params = READ_UINT32(p);
				params++;
				p += 4;
			}

			_vm->runScript(_vm->VAR(_vm->VAR_REMOTE_START_SCRIPT), 1, 0, (int *)_tmpbuffer);
		}
		break;

	case PACKETTYPE_REMOTESTARTSCRIPTRETURN:
		{
			p = _packbuffer + 12;
			params = (uint32 *)_tmpbuffer;

			for (int i = 0; i < 24; i++) {
				*params = READ_UINT32(p);
				params++;
				p += 4;
			}

			_vm->runScript(_vm->VAR(_vm->VAR_REMOTE_START_SCRIPT), 1, 0, (int *)_tmpbuffer);
			int res = _vm->pop();

			WRITE_UINT32(_tmpbuffer, res);

			// FIXME
			remoteSendData(PN_SENDTYPE_INDIVIDUAL, 0 /* gdefMultiPlay.from */, PACKETTYPE_REMOTESTARTSCRIPTRESULT, _tmpbuffer, 4, 0);
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
			unpackageArray(newArray, _packbuffer + 12, len);
			memset(_tmpbuffer, 0, 25 * 4);
			WRITE_UINT32(_tmpbuffer, newArray);

			// Quick start the script (1st param is the new array)
			_vm->runScript(_vm->VAR(_vm->VAR_NETWORK_RECEIVE_ARRAY_SCRIPT), 1, 0, (int *)_tmpbuffer);
		}
		break;

	default:
		warning("Moonbase: Received unknown network command %d", type);
	}
}

void Net::unpackageArray(int arrayId, byte *data, int len) {
	warning("STUB: unpackageArray");
}


void Net::doNetworkOnceAFrame(int msecs) {
	uint32 tickCount = g_system->getMillis() + msecs;

	while (getMessageCount()) {
		remoteReceiveData();

		if (tickCount >= g_system->getMillis()) {
			break;
		}
	}
}

} // End of namespace Scumm
