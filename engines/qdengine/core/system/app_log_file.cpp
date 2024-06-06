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

appLogFile::appLogFile(const char *fname) : file_(NULL) {
	if (fname)
		file_name_ = fname;
}

appLogFile::~appLogFile() {
	close();
}

bool appLogFile::open(const char *fname) {
	close();

	if (fname)
		file_name_ = fname;

	file_ = new XStream(file_name_.c_str(), XS_OUT);
	return true;
}

bool appLogFile::close() {
	if (file_) {
		file_ -> close();
		delete file_;
		file_ = NULL;
	}
	return true;
}

appLog &appLogFile::operator << (const char *str) {
#ifdef __APP_LOG_ENABLE__
	if (is_enabled() && str) *file() < str;
#endif
	return *this;
}

appLog &appLogFile::operator << (int data) {
#ifdef __APP_LOG_ENABLE__
	if (is_enabled()) *file() <= data;
#endif
	return *this;
}

appLog &appLogFile::operator << (unsigned int data) {
#ifdef __APP_LOG_ENABLE__
	if (is_enabled()) *file() <= data;
#endif
	return *this;
}

appLog &appLogFile::operator << (float data) {
#ifdef __APP_LOG_ENABLE__
	if (is_enabled()) *file() <= data;
#endif
	return *this;
}

} // namespace QDEngine
