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

#ifndef BACKENDS_NETWORKING_HTTP_CURL_CONNECTIONMANAGERCURL_H
#define BACKENDS_NETWORKING_HTTP_CURL_CONNECTIONMANAGERCURL_H

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/networking/http/connectionmanager.h"

#include <curl/curl.h>

namespace Networking {

class ConnectionManagerCurl : public ConnectionManager {
private:
	CURLM *_multi;

	void processTransfers() override;

public:
	ConnectionManagerCurl();
	~ConnectionManagerCurl() override;

	/**
	 * All libcurl transfers are going through this ConnectionManager.
	 * So, if you want to start any libcurl transfer, you must create
	 * an easy handle and register it using this method.
	 */
	void registerEasyHandle(CURL *easy) const;

};

} // End of namespace Networking

#endif
