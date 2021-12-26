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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/networking/curl/session.h"

namespace Networking {

Session::Session(Common::String prefix):
	_prefix(prefix), _request(nullptr) {}

Session::~Session() {
	close();
}

static Common::String constructUrl(Common::String prefix, Common::String url) {
	// check url prefix
	if (!prefix.empty()) {
		if (url.contains("://")) {
			if (url.size() < prefix.size() || url.find(prefix) != 0) {
				warning("Session: given URL does not match the prefix!\n\t%s\n\t%s", url.c_str(), prefix.c_str());
				return "";
			}
		} else {
			// if no schema given, just append <url> to <_prefix>
			Common::String newUrl = prefix;
			if (newUrl.lastChar() != '/' && (url.size() > 0 && url.firstChar() != '/'))
				newUrl += "/";
			newUrl += url;
			url = newUrl;
		}
	}

	return url;
}

SessionRequest *Session::get(Common::String url, Common::String localFile, DataCallback cb, ErrorCallback ecb, bool binary) {
	url = constructUrl(_prefix, url);

	if (url.empty())
		return nullptr;

	// check if request has finished (ready to be replaced)
	if (_request) {
		if (!_request->complete()) {
			warning("Session: can't reuse Request that is being processed");
			return nullptr;
		}
	}

	if (!_request) {
		_request = new SessionRequest(url, localFile, cb, ecb, binary); // automatically added to ConnMan
		_request->connectionKeepAlive();
	} else {
		_request->reuse(url, localFile, cb, ecb, binary);
	}

	return _request;
}

void Session::close() {
	if (_request)
		_request->close();
}

void Session::abortRequest() {
	if (_request)
		_request->abortRequest();
}

} // End of namespace Networking
