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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/audio/music_flex.h"
#include "ultima/ultima8/filesys/idata_source.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MusicFlex, Archive)


MusicFlex::MusicFlex(IDataSource *ds) : Archive(ds) {
	songs = 0;
	Std::memset(info, 0, sizeof(SongInfo *) * 128);
	loadSongInfo();
}

MusicFlex::~MusicFlex() {
	uint32 i;
	for (i = 0; i < 128; i++) {
		delete info[i];
		info[i] = 0;
	}

	Archive::uncache();
	delete [] songs;
}

MusicFlex::SongInfo::SongInfo() : num_measures(0), loop_jump(0) {
	Std::memset(filename, 0, 16);
	Std::memset(transitions, 0, 128 * sizeof(int *));
}

MusicFlex::SongInfo::~SongInfo() {
	for (int i = 0; i < 128; i++) {
		delete [] transitions[i];
		transitions[i] = 0;
	}
}

void MusicFlex::cache(uint32 index) {
	// Caching not currently supported
}

void MusicFlex::uncache(uint32 index) {
	// Caching not currently supported
}

bool MusicFlex::isCached(uint32 index) {
	if (index >= count) return false;
	if (!songs) return false;

	return (songs[index] != 0);
}

IDataSource *MusicFlex::getAdlibTimbres() {
	uint32 size;
	uint8 *data = getRawObject(259, &size);
	return new IBufferDataSource(data, size, false, true);
}

void MusicFlex::loadSongInfo() {
	uint32 size;
	uint8 *buf = getRawObject(0, &size);

	if (!buf || !size) {
		error("Unable to load song info from sound/music.flx");
	}
	IBufferDataSource ds(buf, size);
	Std::string line;

	// Read first section till we hit a #
	for (;;) {
		ds.readline(line);

		// We have hit the end of the section
		if (line.at(0) == '#') break;

		Std::string::size_type  begIdx, endIdx;

		// Find the first not space, which will get us the name
		begIdx = line.findFirstNotOf(' ');
		endIdx = line.findFirstOf(' ', begIdx);
		Std::string name = line.substr(begIdx, endIdx - begIdx);

		// Now find the first not space after the name, which will get us the num
		begIdx = line.findFirstNotOf(' ', endIdx);
		endIdx = line.findFirstOf(' ', begIdx);
		int num = line.at(begIdx);

		// Now number of measures
		begIdx = line.findFirstNotOf(' ', endIdx);
		endIdx = line.findFirstOf(' ', begIdx);
		int measures = Std::atoi(line.substr(begIdx, endIdx - begIdx).c_str());

		// Now finally loop_jump
		begIdx = line.findFirstNotOf(' ', endIdx);
		endIdx = line.findFirstOf(' ', begIdx);
		int loop_jump = Std::atoi(line.substr(begIdx, endIdx - begIdx).c_str());

		// Uh oh
		if (num < 0 || num > 127)
			error("Invalid Section 1 song info data. num out of range");

		if (info[num])
			error("Invalid Section 1 song info data. num already defined");

		info[num] = new SongInfo();

		Std::strncpy(info[num]->filename, name.c_str(), 16);
		info[num]->num_measures = measures;
		info[num]->loop_jump = loop_jump;
	};

	// Read 'Section2', or more like skip it, since it's only trans.xmi
	// Read first section till we hit a #
	for (;;) {
		ds.readline(line);

		// We have hit the end of the section
		if (line.at(0) == '#') break;
	}

	// Skip 'Section3'
	for (;;) {
		ds.readline(line);

		// We have hit the end of the section
		if (line.at(0) == '#') break;
	}

	// Read 'Section4' (trans info)
	for (;;) {
		ds.readline(line);

		// We have hit the end of the section
		if (line.at(0) == '#') break;

		Std::string::size_type  begIdx, endIdx;

		// Get 'from' name
		begIdx = line.findFirstNotOf(' ');
		endIdx = line.findFirstOf(' ', begIdx);
		Std::string from = line.substr(begIdx, endIdx - begIdx);

		// Get 'to' name
		begIdx = line.findFirstNotOf(' ', endIdx);
		endIdx = line.findFirstOf(' ', begIdx);
		Std::string to = line.substr(begIdx, endIdx - begIdx);

		// Find index of from name
		int fi;
		for (fi = 0; fi < 128; fi++) {
			if (info[fi] && from == info[fi]->filename) break;
		}

		if (fi == 128)
			error("Invalid Section 4 song info data. Unable to find 'from' index (%s)", from.c_str());

		// Find index of to name
		int ti;
		for (ti = 0; ti < 128; ti++) {
			if (info[ti] && to == info[ti]->filename) break;
		}

		if (ti == 128)
			error("Invalid Section 4 song info data. Unable to find 'to' index (%s)", to.c_str());

		// Allocate Transition info
		info[fi]->transitions[ti] = new int[info[fi]->num_measures];

		// Now attempt to read the trans info for the
		for (int m = 0; m < info[fi]->num_measures; m++) {
			// Get trans info name
			begIdx = line.findFirstNotOf(' ', endIdx);
			endIdx = line.findFirstOf(' ', begIdx);

			if (begIdx == Std::string::npos)
				error("Invalid Section 4 song info data. Unable to read transitions for all measures");

			Std::string trans = line.substr(begIdx, endIdx - begIdx);
			const char *str = trans.c_str();

			int num = 0;

			// Overlayed
			if (*str == '!')
				num = -1 - atoi(str + 1);
			else
				num = atoi(str + 1);

			info[fi]->transitions[ti][m] = num;
		}
	}

	// Skip all remaining sections

	delete[] buf;
}

} // End of namespace Ultima8
} // End of namespace Ultima
