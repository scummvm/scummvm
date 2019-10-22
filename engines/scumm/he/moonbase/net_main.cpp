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
}

int Net::hostGame(char *sessionName, char *userName) {
	warning("STUB: op_net_host_tcpip_game(\"%s\", \"%s\")", sessionName, userName); // PN_HostTCPIPGame
	return 0;
}

int Net::joinGame(char *IP, char *userName) {
	warning("STUB: Net::joinGame(\"%s\", \"%s\")", IP, userName); // PN_JoinTCPIPGame
	return 0;
}

int Net::addUser(char *shortName, char *longName) {
	warning("STUB: Net::addUser(\"%s\", \"%s\")", shortName, longName); // PN_AddUser

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
	return 0;
}

int Net::createSession(char *name) {
	warning("STUB: Net::createSession(\"%s\")", name); // PN_CreateSession
	return 0;
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

	// FAKE 1 session. FIXME
	return 1;
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
	warning("STUB: Net::remoteStartScript(%d, %d, %d, %d, ...)", typeOfSend, sendTypeParam, priority, argsCount); // PN_RemoteStartScriptCommand
}

byte packbuffer[MAX_PACKET_SIZE + 8];

void Net::remoteSendData(int type, byte *data, int len) {
	WRITE_UINT32(packbuffer, type);
	WRITE_UINT32(packbuffer + 4, len);
	memcpy(packbuffer + 8, data, len);

	debug("Package to send, %d bytes", len + 8);

	Common::hexdump(packbuffer, len + 8);
}

void Net::remoteSendArray(int typeOfSend, int sendTypeParam, int priority, int arrayIndex) {
	byte *arr = _vm->getResourceAddress(rtString, arrayIndex & ~0x33539000);
	int len = _vm->getResourceSize(rtString, arrayIndex & ~0x33539000);

	warning("STUB: Net::remoteSendArray(%d, %d, %d, %d)", typeOfSend, sendTypeParam, priority, arrayIndex & ~0x33539000); // PN_RemoteSendArrayCommand

	remoteSendData(PACKETTYPE_REMOTESENDSCUMMARRAY, arr, len);
}

int Net::remoteStartScriptFunction(int typeOfSend, int sendTypeParam, int priority, int defaultReturnValue, int argsCount, int32 *args) {
	warning("STUB: Net::remoteStartScriptFunction(%d, %d, %d, %d, %d, ...)", typeOfSend, sendTypeParam, priority, defaultReturnValue, argsCount); // PN_RemoteStartScriptFunction
	return 0;
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

} // End of namespace Scumm
