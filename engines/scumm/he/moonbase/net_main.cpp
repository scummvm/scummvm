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

namespace Scumm {

Net::Net(ScummEngine_v100he *vm) : _vm(vm) {
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
	return 0;
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

bool Net::getHostName(char *hostname, int length) {
	warning("STUB: Net::getHostName(\"%s\", %d)", hostname, length); // PN_GetHostName
	return false;
}

bool Net::getIPfromName(char *ip, int ipLength, char *nameBuffer) {
	warning("STUB: Net::getIPfromName(\"%s\", %d, \"%s\")", ip, ipLength, nameBuffer); // PN_GetIPfromName
	return false;
}

} // End of namespace Scumm
