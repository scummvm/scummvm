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

#ifndef DARKSEED_MUSIC_H
#define DARKSEED_MUSIC_H

#include "darkseed/adlib_dsf.h"
#include "darkseed/adlib_worx.h"

#include "audio/mididrv_ms.h"
#include "audio/midiparser.h"
#include "common/mutex.h"

namespace Darkseed {

class DarkseedEngine;

class MusicPlayer {
protected:
	DarkseedEngine *_vm;

	Common::Mutex _mutex;
	MidiDriver_Multisource *_driver;
	MidiDriver_DarkSeedFloppy_AdLib *_floppyAdLibDriver;

	MidiParser *_parser;
	byte *_musicData;
	byte *_tosInstrumentBankData;
	bool _tosInstrumentBankLoaded;
	bool _useFloppyMusic;

	bool _paused;

	static void onTimer(void *data);

public:
	MusicPlayer(DarkseedEngine *vm, bool useFloppyMusic);
	~MusicPlayer();

	int open();

	void load(Common::SeekableReadStream *in, int32 size = -1, bool sfx = false);
	void loadTosInstrumentBankData(Common::SeekableReadStream *in, int32 size = -1);
	void loadTosInstrumentBank();
	void loadInstrumentBank(Common::SeekableReadStream *in, int32 size = -1);

	void play(uint8 priority = 0xFF, bool loop = false);
	void setLoop(bool loop);
	bool isPlaying();
	void stop();
	void pause(bool pause);

	void startFadeOut();
	bool isFading();

	void syncSoundSettings();

private:
	// The type of the music device selected for playback.
	MusicType _deviceType;
};

} // namespace Darkseed

#endif // DARKSEED_MUSIC_H
