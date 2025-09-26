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
#include "darkseed/midiparser_sbr.h"

#include "audio/mididrv_ms.h"
#include "audio/midiparser.h"
#include "common/mutex.h"

namespace Darkseed {

class DarkseedEngine;

class MusicPlayer {
protected:
	static const uint8 NUM_SFX_PARSERS = 5;

	DarkseedEngine *_vm;

	Common::Mutex _mutex;
	MidiDriver_Multisource *_driver;
	MidiDriver_DarkSeedFloppy_AdLib *_floppyAdLibDriver;

	MidiParser *_musicParser;
	MidiParser *_sfxParsers[NUM_SFX_PARSERS];
	MidiParser_SBR *_sfxParserSbr;
	// The type of the music device selected for playback.
	MusicType _deviceType;
	byte *_musicData;
	byte *_sfxData;
	byte *_tosInstrumentBankData;
	bool _tosInstrumentBankLoaded;
	bool _useFloppyMusic;
	bool _useFloppySfx;

	bool _paused;

	static void onTimer(void *data);

public:
	MusicPlayer(DarkseedEngine *vm, bool useFloppyMusic, bool useFloppySfx);
	~MusicPlayer();

	int open();

	void load(Common::SeekableReadStream *in, int32 size = -1, bool sfx = false);
	void loadTosInstrumentBankData(Common::SeekableReadStream *in, int32 size = -1);
	void loadTosInstrumentBank();
	void loadInstrumentBank(Common::SeekableReadStream *in, int32 size = -1);
	bool isSampleSfx(uint8 sfxId);

	void playMusic(uint8 priority = 0xFF, bool loop = false);
	void setLoopMusic(bool loop);
	bool isPlayingMusic();
	void stopMusic();
	void pauseMusic(bool pause);
	void playSfx(uint8 sfxId, uint8 priority = 0xFF);
	bool isPlayingSfx();
	bool isPlayingSfx(uint8 sfxId);
	void stopAllSfx();
	bool stopSfx(uint8 sfxId);

	void startFadeOutMusic();
	bool isFadingMusic();

	void syncSoundSettings();

protected:
	uint8 assignSfxParser();
};

} // namespace Darkseed

#endif // DARKSEED_MUSIC_H
