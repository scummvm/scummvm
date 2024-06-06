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

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

namespace QDEngine {

#ifdef __QD_SAVE_DEBUG__
qdSaveStream::qdSaveStream(int err) : stream_(err),
	log_record_index_(0) {
}

qdSaveStream::qdSaveStream(const char *name, unsigned flags, int err) : stream_(name, flags, err),
	log_record_index_(0) {
	open_log(name, flags);
}

qdSaveStream::~qdSaveStream() {
	close();
}

int qdSaveStream::open(const char *name, unsigned f) {
	if (int ret = stream_.open(name, f)) {
		open_log(name, f);
		return ret;
	}

	return 0;
}

void qdSaveStream::close() {
	stream_.close();
	log_stream_.close();
}

unsigned long qdSaveStream::read(void *buf, unsigned long len) {
	unsigned long ret = stream_.read(buf, len);
	log_stream_ << log_record_index_++ << "\t\t" << (char *)buf << "\r\n";
	return ret;
}

unsigned long qdSaveStream::write(const void *buf, unsigned long len) {
	log_stream_ << log_record_index_++ << "\t\t" << (char *)buf << "\r\n";
	return stream_.write(buf, len);
}

void qdSaveStream::open_log(const char *fname, int flags) {
	XBuffer log_fname(2048);

	log_fname < fname < "_log";

	if (flags & XS_IN)
		log_fname < "_in";
	if (flags & XS_OUT)
		log_fname < "_out";

#ifdef _DEBUG
	log_fname < "_dbg";
#endif

	log_record_index_ = 0;
	log_stream_.open(log_fname.c_str());
}

#endif // __QD_SAVE_DEBUG__

} // namespace QDEngine
