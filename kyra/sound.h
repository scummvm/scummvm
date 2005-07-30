/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SOUND_H
#define SOUND_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "kyra/kyra.h"

namespace Kyra {
	class MusicPlayer : public MidiDriver {

	public:

		MusicPlayer(MidiDriver* driver, KyraEngine* engine);
		~MusicPlayer();

		void setVolume(int volume);
		int getVolume() { return _volume; }

		void hasNativeMT32(bool nativeMT32) { _nativeMT32 = nativeMT32; }

		void playMusic(const char* file);
		void playMusic(uint8* data, uint32 size);
		void stopMusic();

		void playTrack(uint8 track);
		void setPassThrough(bool b)	{ _passThrough = b; }

		//MidiDriver interface implementation
		int open();
		void close();
		void send(uint32 b);
		void metaEvent(byte type, byte *data, uint16 length);

		void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }
		uint32 getBaseTempo(void) { return _driver ? _driver->getBaseTempo() : 0; }

		//Channel allocation functions
		MidiChannel *allocateChannel()		{ return 0; }
		MidiChannel *getPercussionChannel()	{ return 0; }

	protected:

		static void onTimer(void *data);

		MidiChannel* _channel[16];
		uint8 _channelVolume[16];
		MidiDriver* _driver;
		bool _nativeMT32;
		bool _passThrough;
		uint8 _volume;
		bool _isPlaying;
		MidiParser* _parser;
		KyraEngine* _engine;

	};
} // end of namespace Kyra

#endif
