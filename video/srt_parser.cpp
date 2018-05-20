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

#include "common/file.h"

#include "video/srt_parser.h"

namespace Video {

SRTParser::SRTParser() {
}

SRTParser::~SRTParser() {
	cleanup();
}

void SRTParser::cleanup() {
	for (Common::Array<SRTEntry *>::const_iterator item = _entries.begin(); item != _entries.end(); ++item)
		delete *item;
}

bool parseTime(const char **pptr, uint32 *res) {
	int hours, mins, secs, msecs;
	const char *ptr = *pptr;

	hours = (*ptr++ - '0') * 10;
	hours += *ptr++ - '0';

	if (hours > 24 || hours < 0)
		return false;

	if (*ptr++ != ':')
		return false;

	mins = (*ptr++ - '0') * 10;
	mins += *ptr++ - '0';

	if (mins > 60 || mins < 0)
		return false;

	if (*ptr++ != ':')
		return false;

	secs = (*ptr++ - '0') * 10;
	secs += *ptr++ - '0';

	if (secs > 60 || secs < 0)
		return false;

	if (*ptr++ != ',')
		return false;

	msecs =  (*ptr++ - '0') * 100;
	msecs += (*ptr++ - '0') * 10;
	msecs +=  *ptr++ - '0';

	if (msecs > 1000 || msecs < 0)
		return false;

	*res = 1000 * (3600 * hours + 60 * mins + secs) + msecs;

	*pptr = ptr;

	return true;
}

int SRTEntryComparator(const void *item1, const void *item2) {
	const SRTEntry *l = *(const SRTEntry **)item1;
	const SRTEntry *r = *(const SRTEntry **)item2;

	return l->start - r->start;
}

bool SRTParser::parseFile(const char *fname) {
	Common::File f;

	if (!f.open(fname))
		return false;

	byte buf[3];
	f.read(buf, 3);

	int line = 0;

	// Skip UTF header if present
	if (buf[0] != 0xef || buf[1] != 0xbb || buf[2] != 0xbf)
		f.seek(0);

	while (!f.eos()) {
		Common::String sseq, stimespec, stmp, text;

		sseq = f.readLine(); line++;
		stimespec = f.readLine(); line++;
		text = f.readLine(); line++;

		if (sseq.empty()) {
			if (f.eos()) {
				// Normal end of stream
				break;
			} else {
				warning("Bad SRT file format (spec): %s at line %d", fname, line);
				break;
			}
		}

		if (stimespec.empty() || text.empty()) {
			warning("Bad SRT file format (spec): %s at line %d", fname, line);
			break;
		}

		// Read all multiline text
		while (!f.eos()) {
			stmp = f.readLine(); line++;

			if (!stmp.empty()) {
				text += '\n';
				text += stmp;
			} else {
				break;
			}
		}

		uint32 seq = atol(sseq.c_str());
		if (seq == 0) {
			warning("Bad SRT file format (seq): %s at line %d", fname, line);
			break;
		}

		// 00:20:41,150 --> 00:20:45,109
		if (stimespec.size() < 29) {
			warning("Bad SRT file format (timespec length %d): %s at line %d", stimespec.size(), fname, line);
			break;
		}

		const char *ptr = stimespec.c_str();
		uint32 start, end;
		if (!parseTime(&ptr, &start)) {
			warning("Bad SRT file format (timespec start): %s at line %d", fname, line);
			break;
		}

		while (*ptr == ' ')
			ptr++;

		while (*ptr == '-')
			ptr++;

		if (*ptr != '>') {
			warning("Bad SRT file format (timespec middle ('%c')): %s at line %d", *ptr, fname, line);
			break;
		}

		ptr++;

		while (*ptr == ' ')
			ptr++;

		if (!parseTime(&ptr, &end)) {
			warning("Bad SRT file format (timespec end): %s at line %d", fname, line);
			break;
		}

		_entries.push_back(new SRTEntry(seq, start, end, text));
	}

	qsort(_entries.data(), _entries.size(), sizeof(SRTEntry *), &SRTEntryComparator);

	return true;
}

Common::String SRTParser::getSubtitle(uint32 timestamp) {
	SRTEntry test(0, timestamp, 0, "");

	const SRTEntry *entry = (const SRTEntry *)bsearch(&test, _entries.data(), _entries.size(), sizeof(SRTEntry *), &SRTEntryComparator);

	if (entry->start >= timestamp && entry->end <= timestamp)
		return entry->text;

	return "";
}

} // End of namespace Video
