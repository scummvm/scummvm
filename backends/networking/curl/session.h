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

#ifndef BACKENDS_NETWORKING_CURL_SESSION_H
#define BACKENDS_NETWORKING_CURL_SESSION_H

#include "backends/networking/curl/sessionrequest.h"

namespace Networking {

class Session {
protected:
	Common::String _prefix;
	SessionRequest *_request;

public:
	Session(Common::String prefix = "");
	~Session();

	SessionRequest *get(Common::String url, Common::String localFile, DataCallback cb = nullptr, ErrorCallback ecb = nullptr, bool binary = false);
	/**
	 * @brief Gracefully close the session
	 *
	 */
	void close();

	/**
	 * @brief Abort session and remove unfinished downloads if they go to local file
	 *
	 */
	void abortRequest();
};

} // End of namespace Networking

#endif
