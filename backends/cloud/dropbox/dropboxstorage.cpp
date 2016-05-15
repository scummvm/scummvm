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
#include "backends/cloud/dropbox/curlrequest.h"
#include "common/debug.h"
#include <curl/curl.h>

namespace Cloud {
namespace Dropbox {

static void curlCallback(void *ptr) {
	debug("--- curl request is complete ---");
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
	addRequest(new CurlRequest(curlCallback, "tkachov.ru"));
	addRequest(new CurlRequest(curlCallback, "scummvm.org"));
}

} //end of namespace Dropbox
} //end of namespace Cloud
