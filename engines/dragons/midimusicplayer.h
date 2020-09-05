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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef DRAGONS_MIDIMUSICPLAYER_H
#define DRAGONS_MIDIMUSICPLAYER_H

#include "audio/midiplayer.h"
#include "vabsound.h"
#include "bigfile.h"

namespace Dragons {

class MidiMusicPlayer : public Audio::MidiPlayer {
private:
	VabSound *_musicVab;
	uint32 _midiDataSize;
public:
	MidiMusicPlayer(BigfileArchive *bigFileArchive, VabSound *musicVab);
	~MidiMusicPlayer();

	void setVolume(int volume) override;

	void playSong(Common::SeekableReadStream *seqData);

	// The original sets the "sequence timing" to 109 Hz, whatever that
	// means. The default is 120.
	uint32 getBaseTempo()	{ return _driver ? (109 * _driver->getBaseTempo()) / 120 : 0; }

	void sendToChannel(byte channel, uint32 b) override;
private:
	byte *resizeMidiBuffer(uint32 desiredSize);
	Common::SeekableReadStream *loadSoundFont(BigfileArchive *bigFileArchive);
};

} // End of namespace Dragons

#endif //DRAGONS_MIDIMUSICPLAYER_H
