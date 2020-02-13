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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/networking/curl/session.h"

namespace Networking {

Session::Session(Common::String prefix):
	_prefix(prefix), _request(nullptr) {}

Session::~Session() {
	close();
}

SessionRequest *Session::get(Common::String url, DataCallback cb, ErrorCallback ecb) {
	// check url prefix
	if (!_prefix.empty()) {
		if (url.contains("://")) {
			if (url.size() < _prefix.size() || url.find(_prefix) != 0) {
				warning("Session: given URL does not match the prefix!\n\t%s\n\t%s", url.c_str(), _prefix.c_str());
				return nullptr;
			}
		} else {
			// if no schema given, just append <url> to <_prefix>
			Common::String newUrl = _prefix;
			if (newUrl.lastChar() != '/' && (url.size() > 0 && url.firstChar() != '/'))
				newUrl += "/";
			newUrl += url;
			url = newUrl;
		}
	}

	// check if request has finished (ready to be replaced)
	if (_request) {
		if (!_request->complete()) {
			warning("Session: can't reuse Request that is being processed");
			return nullptr;
		}
	}

	if (!_request) {
		_request = new Networking::SessionRequest(url, cb, ecb); // automatically added to ConnMan
		_request->connectionKeepAlive();
	} else {
		_request->reuse(url, cb, ecb);
	}

	return _request;
}

void Session::close() {
	if (_request)
		_request->close();
}

} // End of namespace Networking
