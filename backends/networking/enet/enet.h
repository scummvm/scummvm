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

#ifndef BACKENDS_NETWORKING_ENET_ENET_H
#define BACKENDS_NETWORKING_ENET_ENET_H

#include "common/str.h"

namespace Networking {

class Host;
class Socket;

class ENet {
public:
	ENet();
	~ENet();

	bool initalize();
	Host *createHost(Common::String address, int port, int numClients, int numChannels = 1, int incBand = 0, int outBand = 0);
	Host *connectToHost(Common::String address, int port, int timeout = 5000, int numChannels = 1, int incBand = 0, int outBand = 0);
	Socket *createSocket(Common::String address, int port);
private:
	bool _initialized;
};

} // End of namespace Networking


#endif
