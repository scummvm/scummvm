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

#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/system/app_log_file.h"
#include "qdengine/xlibs/xutil/xutil.h"


namespace QDEngine {
/* ---------------------------- INCLUDE SECTION ----------------------------- */


/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

appLog::appLog() : time_(0) {
}

appLog::~appLog() {
}

appLog &appLog::default_log() {
	static appLogFile log("qd_engine.log");
	return log;
}

const char *appLog::time_string() const {
	static XBuffer text(1024, 1);
	text.init();

#ifndef _FINAL_VERSION_
	int hrs = time_ / (1000 * 60 * 60);
	if (hrs < 10) text < "0";
	text <= hrs < ":";

	int min = (time_ % (1000 * 60 * 60)) / (1000 * 60);
	if (min < 10) text < "0";
	text <= min < ":";

	int sec = (time_ % (1000 * 60)) / 1000;
	if (sec < 10) text < "0";
	text <= sec < ":";

	int hsec = (time_ % 1000) / 10;
	if (hsec < 10) text < "0";
	text <= hsec < " ";
#endif

	return text.c_str();
}
} // namespace QDEngine
