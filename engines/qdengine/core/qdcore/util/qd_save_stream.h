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

#ifndef __QD_SAVE_STREAM_H__
#define __QD_SAVE_STREAM_H__

//#define __QD_SAVE_DEBUG__
#include "qdengine/core/system/app_log_file.h"


namespace QDEngine {

#ifdef __QD_SAVE_DEBUG__

class qdSaveStream {
public:
	qdSaveStream(int err = 1);
	qdSaveStream(const char *name, unsigned flags, int err = 1);
	~qdSaveStream();

	int open(const char *name, unsigned f = XS_IN);
	void close();

	unsigned long read(void *buf, unsigned long len);
	unsigned long write(const void *buf, unsigned long len);

	template<class T> qdSaveStream &operator < (T value) {
		stream_ < value;
		log_stream_ << log_record_index_++ << "\t\t" << value << "\r\n";
		return *this;
	}

	template<class T> qdSaveStream &operator > (T &value) {
		stream_ > value;
		log_stream_ << log_record_index_++ << "\t\t" << value << "\r\n";
		return *this;
	}

private:

	XStream stream_;
	appLogFile log_stream_;

	int log_record_index_;

	void open_log(const char *fname, int flags);
};

#else
class XStream;
typedef XStream qdSaveStream;
#endif // __QD_SAVE_DEBUG__

} // namespace QDEngine

#endif // __QD_SAVE_STREAM_H__
