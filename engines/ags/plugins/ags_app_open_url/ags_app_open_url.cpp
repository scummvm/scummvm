/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

// See https://github.com/ericoporto/agsappopenurl for original plugin source code

#include "ags/plugins/ags_app_open_url/ags_app_open_url.h"
#include "common/system.h"

#define MAX_URL_SIZE 2048

#define FAIL_LOG_AND_EXIT(X) \
	do { \
		_engine->PrintDebugConsole(X); \
		params._result = 0; \
		return;  \
	} while (0)

namespace AGS3 {
namespace Plugins {
namespace AGSAppOpenURL {

const char *AGSAppOpenURL::AGS_GetPluginName() {
	return "AGS AppOpenURL";
}

void AGSAppOpenURL::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	SCRIPT_METHOD(AppOpenURL, AGSAppOpenURL::AppOpenURL);
}

void AGSAppOpenURL::AppOpenURL(ScriptMethodParams &params) {
	PARAMS2(int, iags_protocol, const char *, iags_url);

	enum AgsUrlProtocol {
		eAgsUrlProt_https = 0,
		eAgsUrlProt_http
	};

	if (!g_system->hasFeature(OSystem::kFeatureOpenUrl)) {
		FAIL_LOG_AND_EXIT("AppOpenURL: open URL not supported on current platform.");
	}

	if (iags_url == nullptr || iags_url[0] == 0) {
		FAIL_LOG_AND_EXIT("AppOpenURL: empty URL received.");
	}

	const char *found = (const char *)memchr(iags_url, '\0', MAX_URL_SIZE);
	if (!found) {
		FAIL_LOG_AND_EXIT("AppOpenURL: URL is too big.");
	}

	Common::String url_str(iags_url);
	for (char c : {' ' , '\n', '\r', '\t'}) {
		size_t pos;
		while ((pos = url_str.rfind(c)) != Common::String::npos) {
			url_str.deleteChar(pos);
		}
	}
	if (url_str.empty()) {
		FAIL_LOG_AND_EXIT("AppOpenURL: URL is empty after clean up.");
	}

	if (url_str[0] == ':' || (url_str.rfind("://") != Common::String::npos))  {
		FAIL_LOG_AND_EXIT("AppOpenURL: URL includes protocol specifiers.");
	}

	switch (iags_protocol) {
	case eAgsUrlProt_http:
		url_str = "http://" + url_str;
		break;
	case eAgsUrlProt_https:
	default:
		url_str = "https://" + url_str;
		break;
	}


	if (!g_system->openUrl(url_str)) {
		FAIL_LOG_AND_EXIT("AppOpenURL: Fail to open URL.");
	}

	_engine->PrintDebugConsole("AppOpenURL: success opening url");
	params._result = 1;
}

} // namespace AGSAppOpenURL
} // namespace Plugins
} // namespace AGS3
