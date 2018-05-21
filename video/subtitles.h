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

#ifndef VIDEO_SUBTITLES_H
#define VIDEO_SUBTITLES_H

#include "common/str.h"
#include "common/array.h"

namespace Video {

struct SRTEntry {
	uint seq;
	uint32 start;
	uint32 end;

	Common::String text;

	SRTEntry(uint seq_, uint32 start_, uint32 end_, Common::String text_) {
		seq = seq_; start = start_; end = end_; text = text_;
	}
};

class SRTParser {
public:
	SRTParser();
	~SRTParser();

	void cleanup();
	bool parseFile(const char *fname);
	Common::String getSubtitle(uint32 timestamp);

private:
	Common::Array<SRTEntry *> _entries;
};

class Subtitles {
public:
	Subtitles();
	~Subtitles();

	void loadSRTFile(const char *fname);

private:
	SRTParser _srtParser;
	bool _loaded;
};

} // End of namespace Video

#endif
