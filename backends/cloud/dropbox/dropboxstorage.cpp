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

#include "backends/cloud/dropbox/dropboxstorage.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "common/debug.h"
#include "common/json.h"
#include <curl/curl.h>

namespace Cloud {
namespace Dropbox {

static void curlJsonCallback(void *ptr) {
	Common::JSONValue *json = (Common::JSONValue *)ptr;
	if (json) {
		debug("curlJsonCallback:");
		debug("%s", json->stringify(true).c_str());
		delete json;
	} else {
		debug("curlJsonCallback: got NULL instead of JSON!");
	}
}

DropboxStorage::DropboxStorage() {
	curl_global_init(CURL_GLOBAL_ALL);
}

DropboxStorage::~DropboxStorage() {
	curl_global_cleanup();
}

void DropboxStorage::listDirectory(Common::String path) {
	startTimer(1000000); //in one second
}

void DropboxStorage::syncSaves() {
	//not so Dropbox, just testing JSON requesting & parsing:
	addRequest(new Networking::CurlJsonRequest(curlJsonCallback, "https://api.vk.com/method/users.get?v=5.50&user_ids=205387401"));
}

} //end of namespace Dropbox
} //end of namespace Cloud
