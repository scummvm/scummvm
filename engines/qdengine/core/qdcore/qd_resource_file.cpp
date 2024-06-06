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

/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/system/app_error_handler.h"
#include "qdengine/core/qdcore/qd_resource_file.h"


namespace QDEngine {

bool qd_open_resource(const char *file_name, class XStream &fh, bool readonly, bool err_message) {
	if (!readonly) {
		fh.open(file_name, XS_OUT);
		return true;
	}

//	int err = fh.ErrHUsed;
//	fh.ErrHUsed = 0;

	int result = fh.open(file_name, XS_IN);
	if (!result) {
		if (err_message) {
			while (!result) {
				switch (app_errH.message_box(file_name, appErrorHandler::ERR_FILE_NOT_FOUND, appErrorHandler::ERR_MB_ABORTRETRYIGNORE)) {
				case appErrorHandler::ERR_RETRY:
					result = fh.open(file_name, XS_IN);
					break;
				case appErrorHandler::ERR_IGNORE:
//						fh.ErrHUsed = err;
					return false;
				case appErrorHandler::ERR_ABORT:
					exit(1);
				}
			}
		}

//		fh.ErrHUsed = err;
		return false;
	} else {
//		fh.ErrHUsed = err;
		return true;
	}
}

} // namespace QDEngine
