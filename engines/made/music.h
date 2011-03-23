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
 * $URL$
 * $Id$
 *
 */

// Music class

#ifndef MADE_MUSIC_H
#define MADE_MUSIC_H

#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "common/mutex.h"

#include "made/resource.h"

namespace Made {

enum MusicFlags {
	MUSIC_NORMAL = 0,
	MUSIC_LOOP = 1
};

class MusicPlayer : public MidiDriver_BASE {
public:
	MusicPlayer();
	~MusicPlayer();

	bool isPlaying() const { return _isPlaying; }
	void setPlaying(bool playing) { _isPlaying = playing; }

	void setVolume(int volume);
	int getVolume() const { return _masterVolume; }

	bool hasNativeMT32() const { return _nativeMT32; }
	void playXMIDI(GenericResource *midiResource, MusicFlags flags = MUSIC_NORMAL);
	void playSMF(GenericResource *midiResource, MusicFlags flags = MUSIC_NORMAL);
	void stop();
	void pause();
	void resume();
	void setLoop(bool loop) { _looping = loop; }

	void setGM(bool isGM) { _isGM = isGM; }

	// MidiDriver_BASE interface implementation
	virtual void send(uint32 b);
	virtual void metaEvent(byte type, byte *data, uint16 length);

protected:

	static void onTimer(void *data);

	MidiParser *_parser;
	Common::Mutex _mutex;

	MidiChannel *_channel[16];
	MidiDriver *_driver;
	MidiParser *_xmidiParser, *_smfParser;
	byte _channelVolume[16];
	bool _nativeMT32;
	bool _isGM;

	bool _isPlaying;
	bool _looping;
	byte _masterVolume;
};

} // End of namespace Made

#endif
