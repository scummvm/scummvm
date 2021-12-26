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

#ifndef ULTIMA8_AUDIO_MUSICFLEX_H
#define ULTIMA8_AUDIO_MUSICFLEX_H

#include "ultima/ultima8/filesys/archive.h"

namespace Ultima {
namespace Ultima8 {


class MusicFlex : public Archive {
public:
	struct SongInfo {
		SongInfo();
		~SongInfo();

		char        _filename[17];
		int         _numMeasures;
		int         _loopJump;
		int         *_transitions[128];
	};

	struct XMidiData {
		XMidiData(byte *data, uint32 size) : _data(data), _size(size)
		{}

		byte 	*_data;
		uint32 	_size;
	};

	MusicFlex(Common::SeekableReadStream *rs);
	~MusicFlex() override;

	//! Get an xmidi
	XMidiData *getXMidi(uint32 index);

	//! Get song info
	const SongInfo *getSongInfo(uint32 index) const;

	//! Get the Adlib Timbres (index 259)
	Common::SeekableReadStream *getAdlibTimbres();

	void cache(uint32 index) override;
	void uncache(uint32 index) override;
	bool isCached(uint32 index) const override;

private:
	SongInfo   *_info[128];
	XMidiData  **_songs;

	//! Load the song info
	void       loadSongInfo();
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
