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
#ifndef BACKENDS_NETWORKING_CURL_URL_H
#define BACKENDS_NETWORKING_CURL_URL_H

typedef struct Curl_URL CURLU;

#include "common/str.h"

namespace Networking {

class CurlURL {
public:
	CurlURL();
	~CurlURL();

	/**
	 * Parses an URL string by calling curl_url_set  Must be used before using other methods.
	 * @param url is a string containing the URL. e.g. "https://scummvm.org".
	 * @retval true if successful.
	 * @retval false on failure or if using an older version of libcurl.
	 */
	bool parseURL(Common::String url);

	/**
	 * Extracts the scheme of an URL parsed previously by parseURL.
	 * @retval String of the URL's scheme. e.g. "https".
	 * @retval Empty string on failure.
	 */
	Common::String getScheme();
	/**
	 * Extracts the host name of an URL parsed previously by parseURL.
	 * @retval String of the URL's host name. e.g. "scummvm.org".
	 * @retval Empty string on failure.
	 */
	Common::String getHost();
	/**
	 * Extracts the port of an URL parsed previously by parseURL.
	 * @param returnDefault tells libcurl to return the default port according to the URL's scheme if not explicitly defined
	 * @retval The URL's port number if one exists.
	 * @retval 0 if no port found. 
	 * @retval default port if returnDefault is true.
	 * @retval -1 on failure.
	 */
	int getPort(bool returnDefault = false);
private:
	CURLU *_url;
};

} // End of Namespace Networking

#endif
