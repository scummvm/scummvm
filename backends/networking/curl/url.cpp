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
#include <curl/curl.h>
#include "backends/networking/curl/url.h"
#include "common/debug.h"

namespace Networking {

CurlURL::CurlURL() {
	_url = nullptr;
}

// This requires libcurl version 7.62.0, If we're using
// a lower version, stub all of this.
#if LIBCURL_VERSION_NUM < 0x073E00
CurlURL::~CurlURL() {
}

Common::String CurlURL::getScheme() {
	return "";
}

Common::String CurlURL::getHost() {
	return "";
}

int CurlURL::getPort(bool defaultPort) {
	return -1;
}

bool CurlURL::parseURL(Common::String url) {
	warning("libcurl: curl_url requires curl 7.62.0 or later");
	return false;
}

#else

CurlURL::~CurlURL() {
	if (_url) {
		curl_url_cleanup(_url);
		_url = nullptr;
	}
}

bool CurlURL::parseURL(Common::String url) {
	if (_url)
		curl_url_cleanup(_url);

	_url = curl_url();
	if (!_url) {
		warning("libcurl: Could not create curl_url handle");
		return false;
	}
	CURLUcode rc = curl_url_set(_url, CURLUPART_URL, url.c_str(), 0);
	if (rc) {
		warning("libcurl: Unable to parse URL: \"%s\"", url.c_str());
		return false;
	}
	return true;
}

Common::String CurlURL::getScheme() {
	if (!_url)
		return "";
	char *scheme;
	CURLUcode rc = curl_url_get(_url, CURLUPART_SCHEME, &scheme, 0);
	if (rc) {
		warning("libcurl: Unable to get scheme");
		return "";
	}
	Common::String schemeString(scheme);
	curl_free(scheme);
	return schemeString;
}

Common::String CurlURL::getHost() {
	if (!_url)
		return "";
	char *host;
	CURLUcode rc = curl_url_get(_url, CURLUPART_HOST, &host, 0);
	if (rc) {
		warning("libcurl: Unable to get host");
		return "";
	}
	Common::String hostString(host);
	curl_free(host);
	return hostString;
}

int CurlURL::getPort(bool defaultPort) {
	if (!_url)
		return -1;

	char *portChr;
	CURLUcode rc = curl_url_get(_url, CURLUPART_PORT, &portChr, (defaultPort) ? CURLU_DEFAULT_PORT : CURLU_NO_DEFAULT_PORT);
	if (rc) {
		if (rc == CURLUE_NO_PORT)
			return 0;
		warning("libcurl: Unable to get port");
		return -1;
	}
	int port = atoi(portChr);
	curl_free(portChr);
	return port;
}
#endif

} // End of namespace Networking
