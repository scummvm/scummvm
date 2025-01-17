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

#include "got/sound.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "common/memstream.h"
#include "got/got.h"
#include "got/utils/file.h"

namespace Got {

static const byte SOUND_PRIORITY[] = {1, 2, 3, 3, 3, 1, 4, 4, 4, 5, 4, 3, 1, 2, 2, 5, 1, 3, 1};

void Sound::load() {
	File f("DIGSOUND");

	// Load index
	for (int i = 0; i < 16; ++i)
		_digiSounds[i].load(&f);

	// Allocate memory and load sound data
	_soundData = new byte[f.size() - 16 * 8];
	f.read(_soundData, f.size() - 16 * 8);
}

void Sound::play_sound(int index, bool priority_override) {
	if (index >= NUM_SOUNDS)
		return;

	// If a sound is playing, stop it unless there is a priority override
	if (sound_playing()) {
		if (!priority_override && _currentPriority < SOUND_PRIORITY[index])
			return;

		g_engine->_mixer->stopHandle(_soundHandle);
	}

	// Play the new sound
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(
		_soundData + _digiSounds[index]._offset, _digiSounds[index]._length);
	Audio::AudioStream *audioStream = Audio::makeVOCStream(stream, Audio::FLAG_UNSIGNED,
														   DisposeAfterUse::YES);
	g_engine->_mixer->playStream(Audio::Mixer::kSFXSoundType,
								 &_soundHandle, audioStream);
}

void Sound::play_sound(const Gfx::GraphicChunk &src) {
	if (sound_playing())
		g_engine->_mixer->stopHandle(_soundHandle);

	// Play the new sound
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(
		src._data, src._uncompressedSize);
	Audio::AudioStream *audioStream = Audio::makeVOCStream(stream, Audio::FLAG_UNSIGNED,
														   DisposeAfterUse::YES);
	g_engine->_mixer->playStream(Audio::Mixer::kSFXSoundType,
								 &_soundHandle, audioStream);
}

bool Sound::sound_playing() const {
	return g_engine->_mixer->isSoundHandleActive(_soundHandle);
}

void Sound::music_play(const char *name, bool override) {
	if (name != _currentMusic || override) {
		g_engine->_mixer->stopHandle(_musicHandle);
		_currentMusic = name;

		File file(name);

#ifdef TODO
		// FIXME: Completely wrong. Don't know music format yet
		// Open it up for access
		Common::SeekableReadStream *f = file.readStream(file.size());
		Audio::AudioStream *audioStream = Audio::makeRawStream(
			f, 11025, 0, DisposeAfterUse::YES);
		g_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType,
									 &_musicHandle, audioStream);
#else
		warning("TODO: play_music %s", name);

		// The following is a dump of the music data in the hopes
		// it will help someone write a decoder for ScummVM based on it.
		// After an unknown header that doesn't seem to be used, the
		// music seems to be a set of pauses followed by what I think
		// are single byte combinations of frequency and duration that
		/*
        Following is a dump of the "play note" method, in case it's useful:
        MU_playNote     proc far

        freq            = byte ptr  6
        duration        = byte ptr  8

                        push    bp
                        mov     bp, sp
                        pushf
                        cli
                        mov     al, [bp+freq]
                        mov     ah, 0
                        mov     dl, [bp+duration]
                        mov     bx, ax
                        mov     MU_lookupTable[bx], dl
                        mov     dx, 388h
                        mov     al, [bp+freq]
                        out     dx, al
                        in      al, dx
                        in      al, dx
                        in      al, dx
                        in      al, dx
                        in      al, dx
                        in      al, dx
                        inc     dx
                        mov     al, [bp+duration]
                        out     dx, al
                        popf
                        dec     dx
                        mov     cx, 35

        loc_246C8:
                        in      al, dx
                        loop    loc_246C8
                        pop     bp
                        retf
        MU_playNote     endp
        */

		int hdrCount = file.readUint16LE();
		file.skip((hdrCount - 1) * 2);

		while (!file.eos()) {
			int pause = file.readByte();
			if (pause & 0x80)
				pause = ((pause & 0x7f) << 8) | file.readByte();

			int freq = file.readByte();
			int duration = file.readByte();
			if (freq == 0 && duration == 0) {
				debug(1, "End of song");
				break;
			}

			debug(1, "Pause %d, freq %d, duration %d", pause, freq, duration);
		}
#endif
	}
}

void Sound::music_pause() {
	g_engine->_mixer->pauseHandle(_musicHandle, true);
}

void Sound::music_resume() {
	g_engine->_mixer->pauseHandle(_musicHandle, false);
}

void Sound::music_stop() {
	music_pause();
	_currentMusic = nullptr;
}

bool Sound::music_is_on() const {
	return g_engine->_mixer->isSoundHandleActive(_musicHandle);
}

const char *Sound::getMusicName(int num) const {
	const char *name = nullptr;

	switch (_G(area)) {
	case 1:
		switch (num) {
		case 0:
			name = "SONG1";
			break;
		case 1:
			name = "SONG2";
			break;
		case 2:
			name = "SONG3";
			break;
		case 3:
			name = "SONG4";
			break;
		case 4:
			name = "WINSONG";
			break;
		case 5:
			name = "BOSSSONG";
			break;
		default:
			break;
		}
		break;

	case 2:
		switch (num) {
		case 0:
			name = "SONG21";
			break;
		case 1:
			name = "SONG22";
			break;
		case 2:
			name = "SONG23";
			break;
		case 3:
			name = "SONG24";
			break;
		case 4:
			name = "SONG35";
			break;
		case 5:
			name = "SONG25";
			break;
		case 6:
			name = "WINSONG";
			break;
		case 7:
			name = "BOSSSONG";
			break;
		default:
			break;
		}
		break;

	case 3:
		switch (num) {
		case 0:
			name = "SONG31";
			break;
		case 1:
			name = "SONG32";
			break;
		case 2:
			name = "SONG33";
			break;
		case 3:
			name = "SONG34";
			break;
		case 4:
			name = "SONG35";
			break;
		case 5:
			name = "SONG36";
			break;
		case 6:
			name = "WINSONG";
			break;
		case 7:
			name = "BOSSSONG";
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}

	if (!name)
		error("Invalid music");

	return name;
}

void play_sound(int index, bool priority_override) {
	_G(sound).play_sound(index, priority_override);
}

void play_sound(const Gfx::GraphicChunk &src) {
	_G(sound).play_sound(src);
}

bool sound_playing() {
	return _G(sound).sound_playing();
}

void music_play(int num, bool override) {
	_G(sound).music_play(num, override);
}

void music_play(const char *name, bool override) {
	_G(sound).music_play(name, override);
}

void music_pause() {
	_G(sound).music_pause();
}

void music_resume() {
	_G(sound).music_resume();
}

} // namespace Got
