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

#include "backends/networking/basic/curl/url.h"

#include "common/debug.h"
#include "common/textconsole.h"

namespace Networking {

// This requires libcurl version 7.62.0, If we're using
// a lower version, stub all of this.
#if LIBCURL_VERSION_NUM < 0x073E00

URL *URL::parseURL(const Common::String &url) {
	warning("libcurl: curl_url requires curl 7.62.0 or later");
	return nullptr;
}

#else

URL *URL::parseURL(const Common::String &url) {
	CURLU *curlu = curl_url();
	if (!curlu) {
		warning("libcurl: Could not create curl_url handle");
		return nullptr;
	}
	CURLUcode rc = curl_url_set(curlu, CURLUPART_URL, url.c_str(), 0);
	if (rc) {
		warning("libcurl: Unable to parse URL: \"%s\"", url.c_str());
		curl_url_cleanup(curlu);
		return nullptr;
	}
	return new CurlURL(curlu);
}

CurlURL::CurlURL(CURLU *curlu) : _url(curlu) {
}

CurlURL::~CurlURL() {
	curl_url_cleanup(_url);
}

Common::String CurlURL::getScheme() const {
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

Common::String CurlURL::getHost() const {
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

int CurlURL::getPort(bool defaultPort) const {
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
