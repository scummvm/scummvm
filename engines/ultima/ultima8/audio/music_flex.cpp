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
#include "common/memstream.h"

namespace Ultima {
namespace Ultima8 {

MusicFlex::MusicFlex(Common::SeekableReadStream *rs) : Archive(rs) {
	Std::memset(_info, 0, sizeof(SongInfo *) * 128);
	_songs = new XMidiData *[_count];
	Std::memset(_songs, 0, sizeof(XMidiData *) * _count);
	loadSongInfo();
}

MusicFlex::~MusicFlex() {
	uint32 i;
	for (i = 0; i < 128; i++) {
		delete _info[i];
	}
	for (i = 0; i < _count; i++) {
		delete _songs[i];
	}
	delete [] _songs;
}

MusicFlex::SongInfo::SongInfo() : _numMeasures(0), _loopJump(0) {
	Std::memset(_filename, 0, 16);
	Std::memset(_transitions, 0, 128 * sizeof(int *));
}

MusicFlex::SongInfo::~SongInfo() {
	for (int i = 0; i < 128; i++) {
		delete [] _transitions[i];
	}
}

MusicFlex::XMidiData *MusicFlex::getXMidi(uint32 index) {
	if (index >= _count)
		return nullptr;
	cache(index);
	return _songs[index];
}

const MusicFlex::SongInfo *MusicFlex::getSongInfo(uint32 index) const {
	if (index > 127)
		return nullptr;
	return _info[index];
}

void MusicFlex::cache(uint32 index) {
	if (index >= _count) return;
	uint32 size;
	uint8 *data = getRawObject(index, &size);
	if (!data) {
		// Note: multiple sorcerer scenes (such as MALCHIR::03F2)
		// request track 122, which is blank in the Gold Edition
		// music flex.
		warning("Unable to cache song %d from sound/music.flx", index);
		return;
	}
	_songs[index] = new XMidiData(data, size);
}

void MusicFlex::uncache(uint32 index) {
	if (index >= _count) return;
	delete _songs[index];
	_songs[index] = nullptr;
}

bool MusicFlex::isCached(uint32 index) const {
	if (index >= _count) return false;
	return (_songs[index] != nullptr);
}

Common::SeekableReadStream *MusicFlex::getAdlibTimbres() {
	uint32 size;
	const uint8 *data = getRawObject(259, &size);
	return new Common::MemoryReadStream(data, size, DisposeAfterUse::YES);
}

void MusicFlex::loadSongInfo() {
	uint32 size;
	const uint8 *buf = getRawObject(0, &size);

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

		// Now finally _loopJump
		begIdx = line.findFirstNotOf(' ', endIdx);
		endIdx = line.findFirstOf(' ', begIdx);
		int loopJump = Std::atoi(line.substr(begIdx, endIdx - begIdx).c_str());

		// Uh oh
		if (num < 0 || num > 127)
			error("Invalid Section 1 song _info data. num out of range");

		if (_info[num])
			error("Invalid Section 1 song _info data. num already defined");

		_info[num] = new SongInfo();

		Std::strncpy(_info[num]->_filename, name.c_str(), 16);
		_info[num]->_numMeasures = measures;
		_info[num]->_loopJump = loopJump;
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

	// Read 'Section4' (trans _info)
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
			if (_info[fi] && from == _info[fi]->_filename) break;
		}

		if (fi == 128)
			error("Invalid Section 4 song _info data. Unable to find 'from' index (%s)", from.c_str());

		// Find index of to name
		int ti;
		for (ti = 0; ti < 128; ti++) {
			if (_info[ti] && to == _info[ti]->_filename) break;
		}

		if (ti == 128)
			error("Invalid Section 4 song _info data. Unable to find 'to' index (%s)", to.c_str());

		// Allocate Transition _info
		_info[fi]->_transitions[ti] = new int[_info[fi]->_numMeasures];

		// Now attempt to read the trans _info for the
		for (int m = 0; m < _info[fi]->_numMeasures; m++) {
			// Get trans _info name
			begIdx = line.findFirstNotOf(' ', endIdx);
			endIdx = line.findFirstOf(' ', begIdx);

			if (begIdx == Std::string::npos)
				error("Invalid Section 4 song _info data. Unable to read _transitions for all measures");

			Std::string trans = line.substr(begIdx, endIdx - begIdx);
			const char *str = trans.c_str();

			int num = 0;

			// Overlayed
			if (*str == '!')
				num = -1 - atoi(str + 1);
			else
				num = atoi(str + 1);

			_info[fi]->_transitions[ti][m] = num;
		}
	}

	// Skip all remaining sections

	delete[] buf;
}

} // End of namespace Ultima8
} // End of namespace Ultima
