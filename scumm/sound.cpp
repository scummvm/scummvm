/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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

#include "stdafx.h"
#include "actor.h"
#include "bundle.h"
#include "imuse.h"
#include "imuse_digi.h"
#include "player_v2.h"
#include "scumm.h"
#include "sound.h"
#include "sound/mididrv.h"
#include "sound/mixer.h"
#include "common/config-file.h"
#include "common/timer.h"
#include "common/util.h"

#include "sound/midiparser.h"


enum {
	SOUND_HEADER_SIZE = 26,
	SOUND_HEADER_BIG_SIZE = 26 + 8
};


class DigitalTrackInfo {
public:
	virtual bool error() = 0;
	virtual int play(SoundMixer *mixer, int startFrame, int endFrame) = 0;
	virtual ~DigitalTrackInfo() { }
};

#ifdef USE_MAD
class MP3TrackInfo : public DigitalTrackInfo {
private:
	struct mad_header _mad_header;
	long _size;
	File *_file;
	bool _error_flag;

public:
	MP3TrackInfo(File *file);
	~MP3TrackInfo();
	bool error() { return _error_flag; }
	int play(SoundMixer *mixer, int startFrame, int endFrame);
};
#endif

#ifdef USE_VORBIS
class VorbisTrackInfo : public DigitalTrackInfo {
private:
	File *_file;
	OggVorbis_File _ov_file;
	bool _error_flag;

public:
	VorbisTrackInfo(File *file);
	~VorbisTrackInfo();
	bool error() { return _error_flag; }
	int play(SoundMixer *mixer, int startFrame, int endFrame);
};
#endif



Sound::Sound(Scumm *parent) {
	memset(this,0,sizeof(Sound));	// palmos
	
	_scumm = parent;
	_nameBundleMusic = "";
	_musicBundleBufFinal = NULL;
	_musicBundleBufOutput = NULL;
	_musicDisk = 0;
	_talkChannelHandle = 0;
	_current_cache = 0;
	_currentCDSound = 0;

	_sfxFile = 0;
	
	_bundle = new Bundle();
}

Sound::~Sound() {
	delete _sfxFile;
	delete _bundle;
}

void Sound::addSoundToQueue(int sound) {
	if (!(_scumm->_features & GF_DIGI_IMUSE)) {
		_scumm->VAR(_scumm->VAR_LAST_SOUND) = sound;
		_scumm->ensureResourceLoaded(rtSound, sound);
		addSoundToQueue2(sound);
	} else {
		// WARNING ! This may break something, maybe this sould be put inside if (_gameID == GID_FT) ? 
		// But why addSoundToQueue should not queue sound ?
		_scumm->ensureResourceLoaded(rtSound, sound);
		addSoundToQueue2(sound);
	}
}

void Sound::addSoundToQueue2(int sound) {
	if (_soundQue2Pos < 10) {
		_soundQue2[_soundQue2Pos++] = sound;
	}
}

void Sound::processSoundQues() {
	int i = 0, d, num;
	int16 data[16];

	processSfxQueues();

	while (_soundQue2Pos) {
		d = _soundQue2[--_soundQue2Pos];
		if (d)
			playSound(d);
	}

	while (i < _soundQuePos) {
		num = _soundQue[i++];
		if (i + num > _soundQuePos) {
			warning("processSoundQues: invalid num value");
			break;
		}
		memset(data, 0, sizeof(data));
		if (num > 0) {
			for (int j = 0; j < num; j++)
				data[j] = _soundQue[i + j];
			i += num;

#if 0
			debug(1, "processSoundQues(%d,%d,%d,%d,%d,%d,%d,%d,%d)",
						data[0] >> 8,
						data[0] & 0xFF,
						data[1], data[2], data[3], data[4], data[5], data[6], data[7]
				);
#endif
			
			if (_scumm->_features & GF_DIGI_IMUSE) {
				if (_scumm->_imuseDigital)
					_scumm->_imuseDigital->doCommand(data[0], data[1], data[2], data[3], data[4],
																	data[5], data[6], data[7]);
			} else {
				if (_scumm->_imuse)
					_scumm->VAR(_scumm->VAR_SOUNDRESULT) =
						(short)_scumm->_imuse->doCommand(data[0], data[1], data[2], data[3], data[4],
																	data[5], data[6], data[7]);
			}
		}
	}
	_soundQuePos = 0;
}

void Sound::playSound(int soundID) {
	byte *ptr;
	char *sound;
	int size;
	int rate;
	byte flags = SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE;
	
	debug(3,"playSound #%d (room %d)", soundID, _scumm->getResourceRoomNr(rtSound, soundID));
	ptr = _scumm->getResourceAddress(rtSound, soundID);
	if (ptr) {
		if (READ_UINT32(ptr) == MKID('iMUS')){
			assert(_scumm->_imuseDigital);
			_scumm->_imuseDigital->startSound(soundID);
			return;
		}
		else if (READ_UINT32(ptr) == MKID('Crea')) {
			assert(_scumm->_imuseDigital);
			_scumm->_imuseDigital->startSound(soundID);
			return;
		}
		else if (READ_UINT32(ptr) == MKID('SOUN')) {
			ptr += 24;
			int track = ptr[0];
			int loops = ptr[1];
			int start = (ptr[2] * 60 + ptr[3]) * 75 + ptr[4];
			_scumm->VAR(_scumm->VAR_MUSIC_TIMER) = 0;
			playCDTrack(track, loops == 0xff ? -1 : loops, start, 0);

			_currentCDSound = soundID;
			return;
		}
		// Support for SFX in Monkey Island 1, Mac version
		// This is rather hackish right now, but works OK. SFX are not sounding
		// 100% correct, though, not sure right now what is causing this.
		else if (READ_UINT32(ptr) == MKID('Mac1')) {

			// Read info from the header
			size = READ_BE_UINT32(ptr+0x60);
			rate = READ_BE_UINT32(ptr+0x64) >> 16;

			// Skip over the header (fixed size)
			ptr += 0x72;

			// Allocate a sound buffer, copy the data into it, and play
			sound = (char *)malloc(size);
			memcpy(sound, ptr, size);
			_scumm->_mixer->playRaw(NULL, sound, size, rate, flags, soundID);
			return;
		}
		// Support for Putt-Putt sounds - very hackish, too 8-)
		else if (READ_UINT32(ptr) == MKID('DIGI')) {
			// TODO - discover what data the first chunk, HSHD, contains
			// it might be useful here.
			ptr += 8 + READ_BE_UINT32(ptr+12);
			if (READ_UINT32(ptr) != MKID('SDAT'))
				return;	// abort

			size = READ_BE_UINT32(ptr+4) - 8;
			// FIXME - what value here ?!? 11025 is just a guess based on strings in w32 bin, prev guess 8000
			rate = 11025;

			// Allocate a sound buffer, copy the data into it, and play
			sound = (char *)malloc(size);
			memcpy(sound, ptr + 8, size);
			_scumm->_mixer->playRaw(NULL, sound, size, rate, flags, soundID);
			return;
		}
		else if (READ_UINT32(ptr) == MKID('MRAW')) {
			// pcm music in 3DO humongous games
			// TODO play via imuse so isSoundRunning can properly report music value?
			ptr += 8 + READ_BE_UINT32(ptr+12);
			if (READ_UINT32(ptr) != MKID('SDAT'))
				return;
	
			size = READ_BE_UINT32(ptr+4) - 8;
			rate = 22050;
			flags = SoundMixer::FLAG_AUTOFREE;

			// Allocate a sound buffer, copy the data into it, and play
			sound = (char *)malloc(size);
			memcpy(sound, ptr + 8, size);
			_scumm->_mixer->playRaw(NULL, sound, size, rate, flags, soundID);
			
			return;
		}	
		// XMIDI 
		else if ((READ_UINT32(ptr) == MKID('MIDI')) && (_scumm->_features & GF_HUMONGOUS)) {
			// Pass XMIDI on to IMuse unprocessed.
			// IMuse can handle XMIDI resources now.
		}
		else if (READ_UINT32(ptr) == MKID('ADL ')) {
			// played as MIDI, just to make perhaps the later use
			// of WA possible (see "else if" with GF_OLD256 below)
		}
		// Support for sampled sound effects in Monkey1 and Monkey2
		else if (READ_UINT32(ptr) == MKID('SBL ')) {
			debug(2, "Using SBL sound effect");

			// TODO - Figuring out how the SBL chunk works. Here's
			// an example:
			//
			// 53 42 4c 20 00 00 11 ae  |SBL ....|
			// 41 55 68 64 00 00 00 03  |AUhd....|
			// 00 00 80 41 55 64 74 00  |...AUdt.|
			// 00 11 9b 01 96 11 00 a6  |........|
			// 00 7f 7f 7e 7e 7e 7e 7e  |...~~~~~|
			// 7e 7f 7f 80 80 7f 7f 7f  |~.......|
			// 7f 80 80 7f 7e 7d 7d 7e  |....~}}~|
			// 7e 7e 7e 7e 7e 7e 7e 7f  |~~~~~~~.|
			//
			// The length of the AUhd chunk always seems to be 3
			// bytes. Let's skip that for now.
			//
			// The starting offset, length and sample rate is all
			// pure guesswork. The result sounds reasonable to me,
			// but I've never heard the original.
			//
			// I've since discovered that the non-interactive
			// Sam & Max demo also uses SBL sound effects, but
			// with different sub-chunks. Example:
			//
			// 53 42 4c 20 00 01 15 6e  |SBL ...n|
			// 57 56 68 64 00 00 00 03  |WVhd....|
			// 00 00 80 57 56 64 74 00  |...WVdt.|
			// 01 15 5b 01 56 15 01 a6  |..[.V...|
			// 00 80 80 80 80 80 80 80  |........|
			// 80 80 80 80 80 80 80 80  |........|
			// 80 80 80 80 80 80 80 80  |........|
			// 80 80 80 80 80 80 80 80  |........|
			//
			// I'm going to assume that the sample frequency is
			// the only important difference between the two.

			// FIXME: SBL resources are apparently horribly
			// distorted on segacd even though it shares the same
			// header etc. So don't try to play them for now.
			if (_scumm->_gameId == GID_MONKEY_SEGA)	{
				return;
			}
	
			if (READ_UINT32(ptr + 8) == MKID('WVhd'))
				rate = 11025;
			else
				rate = 8000;

			size = READ_BE_UINT32(ptr + 4) - 27;

			// Allocate a sound buffer, copy the data into it, and play
			sound = (char *)malloc(size);
			memcpy(sound, ptr + 33, size);
			_scumm->_mixer->playRaw(NULL, sound, size, rate, flags, soundID);
			return;
		} else if (_scumm->_features & GF_OLD256) {
			size = READ_LE_UINT32(ptr);
#if 0
			// FIXME - this is just some debug output for Zak256
			if (size != 30) {
				char name[9];
				memcpy(name, ptr+22, 8);
				name[8] = 0;
				printf("Going to play Zak256 sound '%s':\n", name);
				hexdump(ptr, 0x40);
			}
			/*
			There seems to be some pattern in the Zak256 sound data. Two typical
			examples are these:
			
			d7 10 00 00 53 4f d1 10  |....SO..|
			00 00 00 00 04 00 ff 00  |........|
			64 00 00 00 01 00 64 6f  |d.....do|
			6f 72 6f 70 65 6e 40 a8  |oropen@.|
			57 14 a1 10 00 00 50 08  |W.....P.|
			00 00 00 00 00 00 b3 07  |........|
			00 00 3c 00 00 00 04 80  |..<.....|
			03 02 0a 01 8c 82 87 81  |........|
	
			5b 07 00 00 53 4f 55 07  |[...SOU.|
			00 00 00 00 04 00 ff 00  |........|
			64 00 00 00 01 00 64 72  |d.....dr|
			77 6f 70 65 6e 00 53 a8  |wopen.S.|
			57 14 25 07 00 00 92 03  |W.%.....|
			00 00 00 00 00 00 88 03  |........|
			00 00 3c 00 00 00 82 82  |..<.....|
			83 84 86 88 89 8b 89 89  |........|
			
			As you can see, there are quite some patterns, e.g.
			the "3c 00 00 00" - the sound data starts at offset 0x36
			
			Indy 3 uses a different format. The very first sound played
			in Indy 3 looks as follows:
			5a 25 00 00 53 4f 54 25  |Z%..SOT%|
			00 00 53 4f db 0a 00 00  |..SO....|
			57 41 c8 00 18 00 00 00  |WA......|
			00 00 00 00 9e 05 00 00  |........|
			00 00 00 00 fd 32 00 f8  |.....2..|
			02 f9 08 ff 22 08 00 ff  |...."...|
			20 5c 00 ff 10 03 00 fd  | \......|
			64 00 f8 00 f9 00 ff 22  |d......"|

			Indy 3, opening a door:
			d1 00 00 00 53 4f cb 00  |....SO..|
			00 00 53 4f a2 00 00 00  |..SO....|
			57 41 64 00 18 00 00 00  |WAd.....|
			00 00 00 00 00 00 00 00  |........|
			00 00 7e 00 f9 0c ff 20  |..~.... |
			90 01 ff 22 c2 01 ff 0a  |..."....|
			03 00 ff 04 57 06 ff 00  |....W...|
			04 00 ff 0a 00 00 ff 00  |........|
			
			So there seems to be a "SO" chunk which contains again a SO chunk and a WA chunk.
			WA probably again contains audio data?
			*/
#endif
			rate = 11000;
			int type = *(ptr + 0x0D);

			switch(type) {
				case 0:	{ // Sound effect
					int waveSize = READ_LE_UINT32(ptr + 0x22);
					int loopStart = READ_LE_UINT32(ptr + 0x26);
					int loopEnd = READ_LE_UINT32(ptr + 0x2A);

					if (size - 0x36 < waveSize) {
						warning("Wrong wave size in sound #%i: %i", soundID, waveSize);
						waveSize = size - 0x36;
					}
					ptr += 0x36;
					sound = (char *)malloc(waveSize);
					for (int x = 0; x < waveSize; x++) {
						int bit = *ptr++;
						if (bit < 0x80)
							sound[x] = 0x7F - bit;
						else
							sound[x] = bit;
					}
	
					if (loopEnd > 0) {
						flags |= SoundMixer::FLAG_LOOP;
						if ((loopEnd < waveSize) || (loopStart > 0)) {
							// FIXME: Implement partial loops
							warning("Partial loops not implemented. Loop at 0x%X thru 0x%X", loopStart, loopEnd);
						}
					}

					_scumm->_mixer->playRaw(NULL, sound, waveSize, 11000, flags, soundID);
					break;
				}

				case 1: { // Music (Euphony format)
					int numInstruments = *(ptr + 0x14);
					int tuneSize = 0, tempo = 0;

					ptr += (0x16 + (numInstruments * 48));	// Skip instrument definitions
					ptr += (32*4);	// Skip preset values (mute, channel, volume, transpose)
					ptr += 8;	// (Unknown)
					
					ptr += numInstruments;	// Instrument channel's

					tuneSize = READ_LE_UINT32(ptr);
					ptr += 5;

					tempo = *ptr++;
					ptr += 2;
					// Music data begins here

					warning("Euphony tune #%d unsupported", soundID);
					break;
				}
			
				case 2: { // CD track resource
					ptr += 0x16;
					int track = ptr[0];
					int loops = ptr[1];
					int start = (ptr[2] * 60 + ptr[3]) * 75 + ptr[4];
					int end = (ptr[5] * 60 + ptr[6]) * 75 + ptr[7];

					if (soundID == _currentCDSound)
						if (pollCD() == 1)
							return;

					playCDTrack(track, loops == 0xff ? -1 : loops, start, end);
					_currentCDSound = soundID;
					break;
				}			
			}
			return;
		}
	
	}
	
	if (_scumm->_gameId == GID_LOOM && (READ_LE_UINT16(ptr + 4) == 'so')) {
		// Mac version of Loom uses yet another sound format
		/*
		playSound #9 (room 70)
		000000: 55 00 00 45  73 6f 00 64  01 00 00 00  00 00 00 00   |U..Eso.d........|
		000010: 00 05 00 8e  2a 8f 2d 1c  2a 8f 2a 8f  2d 1c 00 28   |....*.-.*.*.-..(|
		000020: 00 31 00 3a  00 43 00 4c  00 01 00 00  00 01 00 64   |.1.:.C.L.......d|
		000030: 5a 00 01 00  00 00 01 00  64 00 00 01  00 00 00 01   |Z.......d.......|
		000040: 00 64 5a 00  01 00 00 00  01 00 64 5a  00 01 00 00   |.dZ.......dZ....|
		000050: 00 01 00 64  00 00 00 00  00 00 00 07  00 00 00 64   |...d...........d|
		000060: 64 00 00 4e  73 6f 00 64  01 00 00 00  00 00 00 00   |d..Nso.d........|
		000070: 00 05 00 89  3d 57 2d 1c  3d 57 3d 57  2d 1c 00 28   |....=W-.=W=W-..(|
		playSound #16 (room 69)
		000000: dc 00 00 a5  73 6f 00 64  01 00 00 00  00 00 00 00   |....so.d........|
		000010: 00 05 00 00  2a 8f 03 e8  03 e8 03 e8  03 e8 00 28   |....*..........(|
		000020: 00 79 00 7f  00 85 00 d6  00 01 00 00  00 19 01 18   |.y..............|
		000030: 2f 00 18 00  01 18 32 00  18 00 01 18  36 00 18 00   |/.....2.....6...|
		000040: 01 18 3b 00  18 00 01 18  3e 00 18 00  01 18 42 00   |..;.....>.....B.|
		000050: 18 00 01 18  47 00 18 00  01 18 4a 00  18 00 01 18   |....G.....J.....|
		000060: 4e 00 10 00  01 18 53 00  10 00 01 18  56 00 10 00   |N.....S.....V...|
		000070: 01 18 5a 00  10 00 02 28  5f 00 01 00  00 00 00 00   |..Z....(_.......|
		*/
		return;
	}
	
	if ((_scumm->_features & GF_AMIGA) && (_scumm->_version == 3))
	{
		// experimental support for Indy3 Amiga sound effects
		if (READ_BE_UINT16(ptr + 26) == 0x00FF)		// looped sound
			// TODO: support looping sounds
			// ptr + 14 seems to be looping duration
			flags = 0;
		else if (READ_BE_UINT16(ptr + 26) == 0x0001)	// nonlooped sound
			flags = 0;	// 
		else if (READ_BE_UINT16(ptr + 26) == 0x0101)	// background music
			// TODO: support music
			return;
		size = READ_BE_UINT16(ptr + 12);
		rate = 11000;
		_scumm->_mixer->playRaw(NULL, ptr + 28, size, rate, flags, soundID);
		return;
	}

	if (_scumm->_features & GF_OLD_BUNDLE) {
		//TODO: support maniac v1 sounds
		if ((_scumm->_version == 1) && (_scumm->_gameId == GID_MANIAC)) 
			return;
		// other versions seem to be 0000 at this point...
		// hopefully this test is correct
		// 0xfe7f seems to be sound and 0x764a music
		bool amigatest = (READ_LE_UINT16(ptr + 12) == 0xfe7f) || (READ_LE_UINT16(ptr + 12) == 0x764a);
		if (amigatest) {
			// TODO: support amiga sounds
		} else {
			if (_scumm->_playerV2)
				_scumm->_playerV2->startSound (soundID, ptr);
		}
		return;
	}

	if (_scumm->_gameId == GID_MONKEY_VGA || _scumm->_gameId == GID_MONKEY_EGA) {
		// Sound is currently not supported at all in the amiga versions of these games
		if (_scumm->_features & GF_AMIGA)
			return;

		// FIXME: This evil hack works around the fact that in some
		// places in MonkeyVGA, the music is never explicitly stopped.
		// Rather it seems that starting a new music is supposed to 
		// automatically stop the old song.
		// This hack relays on the fact that we currently don't support SFX
		// in these games, only music. Once we add SFX support, we'll have to
		// revise it / replace it by a proper fix.
		if (ptr) {
			_scumm->_imuse->stop_all_sounds();
		}
	}

	if (_scumm->_imuse) {
		_scumm->getResourceAddress(rtSound, soundID);
		_scumm->_imuse->startSound(soundID);
	}
}

void Sound::processSfxQueues() {
	Actor *a;
	int act;
	bool b, finished;

	if (_talk_sound_mode != 0) {
		if (_talk_sound_mode & 1)
			startTalkSound(_talk_sound_a1, _talk_sound_b1, 1);
		if (_talk_sound_mode & 2) {
			startTalkSound(_talk_sound_a2, _talk_sound_b2, 2, &_talkChannelHandle);
		}
		_talk_sound_mode = 0;
	}

	if ((_sfxMode & 2) && _scumm->VAR(_scumm->VAR_TALK_ACTOR)) {
		act = _scumm->VAR(_scumm->VAR_TALK_ACTOR);

		finished = !_talkChannelHandle;

		if (act != 0 && (uint) act < 0x80 && !_scumm->_string[0].no_talk_anim) {
			a = _scumm->derefActor(act, "processSfxQueues");
			if (a->isInCurrentRoom() && (finished || !_endOfMouthSync)) {
				b = true;
				if (!finished)
					b = isMouthSyncOff(_curSoundPos);
				if (_mouthSyncMode != b) {
					_mouthSyncMode = b;
					if (_talk_sound_frame != -1) {
						a->startAnimActor(_talk_sound_frame);
						_talk_sound_frame = -1;
					} else
						a->startAnimActor(b ? a->talkStopFrame : a->talkStartFrame);
				}
			}
		}
		
		if (finished && _scumm->_talkDelay == 0) {
			_scumm->stopTalk();
		}
	}
		
	if (_sfxMode & 1) {
		if (isSfxFinished()) {
			_sfxMode &= ~1;
		}
	}
}

static int compareMP3OffsetTable(const void *a, const void *b) {
	return ((const MP3OffsetTable *)a)->org_offset - ((const MP3OffsetTable *)b)->org_offset;
}

void Sound::startTalkSound(uint32 offset, uint32 b, int mode, PlayingSoundHandle *handle) {
	int num = 0, i;
	int size;
	byte *sound;

	if (_sfxFile->isOpen() == false) {
		warning("startTalkSound: SFX file is not open");
		return;
	}

	// FIXME hack until more is known
	// the size of the data after the sample isn't known
	// 64 is just a guess
	if (_scumm->_features & GF_HUMONGOUS) {
		// SKIP TLKB (8) TALK (8) HSHD (24) and SDAT (8)
		_sfxFile->seek(offset + 48, SEEK_SET);
		sound = (byte *)malloc(b - 64);
		_sfxFile->read(sound, b - 64);
		_scumm->_mixer->playRaw(handle, sound, b - 64, 11025, SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
		return;
	}

	// Some games frequently assume that starting one sound effect will
	// automatically stop any other that may be playing at that time. So
	// that is what we do here, but we make an exception for speech.
	//
	// Do any other games than these need this hack?
	//
	// HACK: Checking for script 99 in Sam & Max is to keep Conroy's song
	// from being interrupted.

	int talkChannel = (_talkChannelHandle - 1);
	if (mode == 1 && (_scumm->_gameId == GID_TENTACLE
		|| (_scumm->_gameId == GID_SAMNMAX && !_scumm->isScriptRunning(99)))) {
		for (i = 0; i < _scumm->_mixer->NUM_CHANNELS; i++) {
			if (i != talkChannel) {
				_scumm->_mixer->stop(i);
			}
		}
	}

	if (b > 8) {
		num = (b - 8) >> 1;
	}

	if (offset_table != NULL) {
		MP3OffsetTable *result = NULL, key;

		key.org_offset = offset;
		result = (MP3OffsetTable *)bsearch(&key, offset_table, num_sound_effects,
												sizeof(MP3OffsetTable), compareMP3OffsetTable);

		if (result == NULL) {
			warning("startTalkSound: did not find sound at offset %d !", offset);
			return;
		}
		if (2 * num != result->num_tags) {
			warning("startTalkSound: number of tags do not match (%d - %d) !", b,
							result->num_tags);
			num = result->num_tags;
		}
		offset = result->new_offset;
		size = result->compressed_size;
	} else {
		offset += 8;
		size = -1;
	}

	_sfxFile->seek(offset, SEEK_SET);

	assert(num+1 < (int)ARRAYSIZE(_mouthSyncTimes));
	for (i = 0; i < num; i++)
		_mouthSyncTimes[i] = _sfxFile->readUint16BE();

	_mouthSyncTimes[i] = 0xFFFF;
	_sfxMode |= mode;
	_curSoundPos = 0;
	_mouthSyncMode = true;

	startSfxSound(_sfxFile, size, handle);
}

void Sound::stopTalkSound() {
	if (_sfxMode & 2) {
		_scumm->_mixer->stopHandle(_talkChannelHandle);
		_sfxMode &= ~2;
	}
}

bool Sound::isMouthSyncOff(uint pos) {
	uint j;
	bool val = true;
	uint16 *ms = _mouthSyncTimes;

	_endOfMouthSync = false;
	do {
		val = !val;
		j = *ms++;
		if (j == 0xFFFF) {
			_endOfMouthSync = true;
			break;
		}
	} while (pos > j);
	return val;
}


int Sound::isSoundRunning(int sound) const {
	int i;

	if (sound == _currentCDSound)
		return pollCD();
	
	if (_scumm->_features & GF_HUMONGOUS) {
		if (sound == -2) {
			return isSfxFinished();
		} else if (sound == -1) {
			// getSoundStatus(), with a -1, will return the
			// ID number of the first active music it finds.
			// TODO handle MRAW (pcm music) in humongous games
			return _scumm->_imuse->getSoundStatus(sound);
		}
	}
	
	i = _soundQue2Pos;
	while (i--) {
		if (_soundQue2[i] == sound)
			return 1;
	}

	if (isSoundInQueue(sound))
		return 1;

	if (!_scumm->isResourceLoaded(rtSound, sound))
		return 0;

	if (_scumm->_imuseDigital)
		return _scumm->_imuseDigital->getSoundStatus(sound);

	if (_scumm->_imuse)
		return _scumm->_imuse->getSoundStatus(sound);

	if (_scumm->_playerV2)
		return _scumm->_playerV2->getSoundStatus(sound);

	return 0;
}

// This is exactly the same as isSoundRunning except that it
// calls IMuse::get_sound_active() instead of IMuse::getSoundStatus().
// This is necessary when determining what resources to
// expire from memory.
bool Sound::isSoundActive(int sound) const {
	int i;

	if (sound == _currentCDSound)
		return pollCD() != 0;

	i = _soundQue2Pos;
	while (i--) {
		if (_soundQue2[i] == sound)
			return true;
	}

	if (isSoundInQueue(sound))
		return true;

	if (!_scumm->isResourceLoaded(rtSound, sound))
		return false;

	if (_scumm->_imuseDigital) {
		return _scumm->_imuseDigital->getSoundStatus(sound) != 0;
	}

	if (!_scumm->_imuse)
		return false;
	return _scumm->_imuse->get_sound_active(sound);
}

bool Sound::isSoundInQueue(int sound) const {
	int i = 0, j, num;
	int16 table[16];

	while (i < _soundQuePos) {
		num = _soundQue[i++];

		memset(table, 0, sizeof(table));

		if (num > 0) {
			for (j = 0; j < num; j++)
				table[j] = _soundQue[i + j];
			i += num;
			if (table[0] == 0x10F && table[1] == 8 && table[2] == sound)
				return 1;
		}
	}
	return 0;
}

void Sound::stopSound(int a) {
	int i;

	if (a != 0 && a == _currentCDSound) {
		_currentCDSound = 0;
		stopCD();
	}

	if (_scumm->_features & GF_FMTOWNS) {
		_scumm->_mixer->stopID(a);
	} else if (_scumm->_imuseDigital) {
		_scumm->_imuseDigital->stopSound(a);
	} else if (_scumm->_imuse) {
		_scumm->_imuse->stopSound(a);
	} else if (_scumm->_playerV2) {
		_scumm->_playerV2->stopSound (a);
	} 

	for (i = 0; i < 10; i++)
		if (_soundQue2[i] == a)
			_soundQue2[i] = 0;
}

void Sound::stopAllSounds() {
	if (_currentCDSound != 0) {
		_currentCDSound = 0;
		stopCD();
	}

	if (_scumm->_imuse) {
		_scumm->_imuse->stop_all_sounds();
		_scumm->_imuse->clear_queue();
	} else if (_scumm->_playerV2) {
		_scumm->_playerV2->stopAllSounds();
	}

	clearSoundQue();
	stopSfxSound();
}

void Sound::clearSoundQue() {
	_soundQue2Pos = 0;
	memset(_soundQue2, 0, sizeof(_soundQue2));
}

void Sound::soundKludge(int *list, int num) {
	int i;

	if (list[0] == -1) {
		processSoundQues();
		return;
	}

	if ((_soundQuePos + num) > 0x100) {
		// FIXME: temporarily changed this to an error to help track down what
		// is causing the sound queue overflows(in particular, to figure out
		// the room/script/offset where the bug occurs). Please report your
		// findings to Fingolfin.
		error("Sound que buffer overflow (%d + %d = %d)", _soundQuePos, num, _soundQuePos+num);
		return;
	}

	_soundQue[_soundQuePos++] = num;
	
	for (i = 0; i < num; i++) {
		_soundQue[_soundQuePos++] = list[i];
	}
}

void Sound::talkSound(uint32 a, uint32 b, int mode, int frame) {
	if (mode == 1) {
		_talk_sound_a1 = a;
		_talk_sound_b1 = b;
	} else {
		_talk_sound_a2 = a;
		_talk_sound_b2 = b;
	}

	_talk_sound_frame = frame;
	_talk_sound_mode |= mode;
}

/* The sound code currently only supports General Midi.
 * General Midi is used in Day Of The Tentacle.
 * Roland music is also playable, but doesn't sound well.
 * A mapping between roland instruments and GM instruments
 * is needed.
 */

void Sound::setupSound() {
	if (_scumm->_imuse) {
		_scumm->_imuse->setBase(_scumm->res.address[rtSound]);

		_scumm->_imuse->set_master_volume(_sound_volume_master);
		_scumm->_imuse->set_music_volume(_sound_volume_music);
	}
	_scumm->_mixer->setVolume(_sound_volume_sfx * _sound_volume_master / 255);
	_scumm->_mixer->setMusicVolume(_sound_volume_music);
	delete _sfxFile;
	_sfxFile = openSfxFile();
}

void Sound::pauseSounds(bool pause) {
	if (_scumm->_imuse)
		_scumm->_imuse->pause(pause);

	// Don't pause sounds if the game isn't active
	// FIXME - this is quite a nasty hack, replace with something cleaner, and w/o
	// having to access member vars directly!
	if (!_scumm->_roomResource)
		return;

	_soundsPaused = pause;
	_scumm->_mixer->pause(pause);

	_scumm->_sound->pauseBundleMusic(pause);

	if (_scumm->_imuseDigital) {
		_scumm->_imuseDigital->pause(pause);
	}

	if ((_scumm->_features & GF_AUDIOTRACKS) && _scumm->VAR(_scumm->VAR_MUSIC_TIMER) > 0) {
		if (pause)
			stopCDTimer();
		else
			startCDTimer();
	}
}

void Sound::startSfxSound(File *file, int file_size, PlayingSoundHandle *handle) {
	char ident[8];
	int block_type;
	byte work[8];
	uint size = 0;
	int rate, comp;
	byte *data;

	if (_scumm->_noDigitalSamples)
		return;

	if (file_size > 0) {
		int alloc_size = file_size;
#ifdef USE_MAD
		if (!_vorbis_mode)
			alloc_size += MAD_BUFFER_GUARD;
#endif
		data = (byte *)calloc(alloc_size, 1);

		if (file->read(data, file_size) != (uint)file_size) {
			/* no need to free the memory since error will shut down */
			error("startSfxSound: cannot read %d bytes", size);
		}
		if (_vorbis_mode)
			playSfxSound_Vorbis(data, file_size, handle);
		else
			playSfxSound_MP3(data, file_size, handle);
		return;
	}

	if (file->read(ident, 8) != 8)
		goto invalid;

	if (!memcmp(ident, "VTLK", 4)) {
		file->seek(SOUND_HEADER_BIG_SIZE - 8, SEEK_CUR);
	} else if (!memcmp(ident, "Creative", 8)) {
		file->seek(SOUND_HEADER_SIZE - 8, SEEK_CUR);
	} else {
	invalid:;
		warning("startSfxSound: invalid header");
		return;
	}

	block_type = file->readByte();
	if (block_type != 1) {
		warning("startSfxSound: Expecting block_type == 1, got %d", block_type);
		return;
	}

	file->read(work, 3);

	size = (work[0] | (work[1] << 8) | (work[2] << 16)) - 2;
	rate = file->readByte();
	comp = file->readByte();

	if (comp != 0) {
		warning("startSfxSound: Unsupported compression type %d", comp);
		return;
	}

	data = (byte *)malloc(size);
	if (data == NULL) {
		error("startSfxSound: out of memory");
	}

	if (file->read(data, size) != size) {
		/* no need to free the memory since error will shut down */
		error("startSfxSound: cannot read %d bytes", size);
	}

	playSfxSound(data, size, 1000000 / (256 - rate), true, handle);
}

File *Sound::openSfxFile() {
	char buf[256];
	File *file = new File();

	/* Try opening the file <_exe_name>.sou first, eg tentacle.sou.
	 * That way, you can keep .sou files for multiple games in the
	 * same directory */
	offset_table = NULL;

#ifdef USE_MAD
	sprintf(buf, "%s.so3", _scumm->getExeName());
	if (!file->open(buf, _scumm->getGameDataPath())) {
		file->open("monster.so3", _scumm->getGameDataPath());
	}
	if (file->isOpen())
		_vorbis_mode = false;
#endif

#ifdef USE_VORBIS
	if (!file->isOpen()) {
		sprintf(buf, "%s.sog", _scumm->getExeName());
		if (!file->open(buf, _scumm->getGameDataPath()))
			file->open("monster.sog", _scumm->getGameDataPath());
		if (file->isOpen())
			_vorbis_mode = true;
	}
#endif

	if (file->isOpen()) {
		/* Now load the 'offset' index in memory to be able to find the MP3 data

		   The format of the .SO3 file is easy :
		   - number of bytes of the 'index' part
		   - N times the following fields (4 bytes each) :
		   + offset in the original sound file
		   + offset of the MP3 data in the .SO3 file WITHOUT taking into account
		   the index field and the 'size' field
		   + the number of 'tags'
		   + the size of the MP3 data
		   - and then N times :
		   + the tags
		   + the MP3 data
		 */
		int size, compressed_offset;
		MP3OffsetTable *cur;
		compressed_offset = file->readUint32BE();
		offset_table = (MP3OffsetTable *) malloc(compressed_offset);
		num_sound_effects = compressed_offset / 16;

		size = compressed_offset;
		cur = offset_table;
		while (size > 0) {
			cur[0].org_offset = file->readUint32BE();
			cur[0].new_offset = file->readUint32BE() + compressed_offset + 4; /* The + 4 is to take into accound the 'size' field */
			cur[0].num_tags = file->readUint32BE();
			cur[0].compressed_size = file->readUint32BE();
			size -= 4 * 4;
			cur++;
		}
		return file;
	}

	sprintf(buf, "%s.sou", _scumm->getExeName());
	if (!file->open(buf, _scumm->getGameDataPath())) {
		file->open("monster.sou", _scumm->getGameDataPath());
	}

	if (!file->isOpen()) {
		sprintf(buf, "%s.tlk", _scumm->getExeName());
		file->open(buf, _scumm->getGameDataPath(), 1, 0x69);
	}
	return file;
}

void Sound::stopSfxSound() {
	if (_scumm->_imuseDigital) {
		_scumm->_imuseDigital->stopAll();
	} else {
		_scumm->_mixer->stopAll();
	}
}

bool Sound::isSfxFinished() const {
	return !_scumm->_mixer->hasActiveSFXChannel();
}

uint32 Sound::decode12BitsSample(byte *src, byte **dst, uint32 size, bool stereo) {
	uint32 s_size = (size / 3) * 4;
	uint32 loop_size = s_size / 4;
	if (stereo) {
		s_size *= 2;
	}
	byte *ptr = *dst = (byte *)malloc(s_size);

	uint32 tmp;
	while (loop_size--) {
		byte v1 = *src++;
		byte v2 = *src++;
		byte v3 = *src++;
		tmp = ((((v2 & 0x0f) << 8) | v1) << 4) - 0x8000;
		*ptr++ = (byte)((tmp >> 8) & 0xff);
		*ptr++ = (byte)(tmp & 0xff);
		if (stereo) {
			*ptr++ = (byte)((tmp >> 8) & 0xff);
			*ptr++ = (byte)(tmp & 0xff);
		}
		tmp = ((((v2 & 0xf0) << 4) | v3) << 4) - 0x8000;
		*ptr++ = (byte)((tmp >> 8) & 0xff);
		*ptr++ = (byte)(tmp & 0xff);
		if (stereo) {
			*ptr++ = (byte)((tmp >> 8) & 0xff);
			*ptr++ = (byte)(tmp & 0xff);
		}
	}
	return s_size;
}

static void music_handler (void *engine) {
	g_scumm->_sound->bundleMusicHandler(g_scumm);
}

void Sound::playBundleMusic(const char *song) {
	if (_scumm->_silentDigitalImuse) {
		return;
	}

	if (_nameBundleMusic[0] == 0) {
		_outputMixerSize = 66150; // ((22050 * 2 * 2) / 4) * 3
		if (_scumm->_gameId == GID_CMI) {
			char bunfile[20];
			sprintf(bunfile, "musdisk%d.bun", _scumm->VAR(_scumm->VAR_CURRENTDISK));
			if (_musicDisk != _scumm->VAR(_scumm->VAR_CURRENTDISK)) 
				_bundle->closeMusicFile();

			if (_bundle->openMusicFile(bunfile, _scumm->getGameDataPath()) == false) {
				if (_bundle->openMusicFile("music.bun", _scumm->getGameDataPath()) == false) {
					_outputMixerSize = 0;
					return;
				}
			}

			_musicDisk = (byte)_scumm->VAR(_scumm->VAR_CURRENTDISK);
			_outputMixerSize = 88140; // ((22050 * 2 * 2)
		} else {
			if (_bundle->openMusicFile("digmusic.bun", _scumm->getGameDataPath()) == false)
				return;
		}
		_musicBundleBufFinal = (byte *)malloc(_outputMixerSize);
		_musicBundleBufOutput = (byte *)malloc(((_outputMixerSize / 0x2000) + 1) * _outputMixerSize);
		_currentSampleBundleMusic = 0;
		_offsetSampleBundleMusic = 0;
		_offsetBufBundleMusic = 0;
		_bundleMusicPosition = 0;
		_pauseBundleMusic = false;
		_musicBundleToBeRemoved = false;
		_musicBundleToBeChanged = false;
		_bundleMusicTrack = -1;
		_numberSamplesBundleMusic = _bundle->getNumberOfMusicSamplesByName(song);
		_nameBundleMusic = song;
		_scumm->_timer->installProcedure(&music_handler, 1000000);
		return;
	}
	if (strcmp(_nameBundleMusic, song) != 0) {
		_newNameBundleMusic = song;
		_musicBundleToBeRemoved = false;
		_musicBundleToBeChanged = true;
	}
}

void Sound::pauseBundleMusic(bool state) {
	_pauseBundleMusic = state;
}

void Sound::stopBundleMusic() {
	_musicBundleToBeRemoved = true;
}

void Sound::bundleMusicHandler(Scumm *scumm) {
	byte *ptr;
	int32 l, num = _numberSamplesBundleMusic, length, k;
	int32 rate = 22050;
	int32 tag, size = -1, header_size = 0;

	if (_pauseBundleMusic)
		return;

	if (_musicBundleToBeRemoved) {
		_scumm->_timer->releaseProcedure(&music_handler);
		_nameBundleMusic = "";
		if (_bundleMusicTrack != -1) {
			_scumm->_mixer->stop(_bundleMusicTrack);
			_bundleMusicTrack = -1;
		}
		if (_musicBundleBufFinal) {
			free(_musicBundleBufFinal);
			_musicBundleBufFinal = NULL;
		}
		if (_musicBundleBufOutput) {
			free(_musicBundleBufOutput);
			_musicBundleBufOutput = NULL;
		}
		return;
	}

	if (_musicBundleToBeChanged) {
		_nameBundleMusic = _newNameBundleMusic;
		_numberSamplesBundleMusic = _bundle->getNumberOfMusicSamplesByName(_nameBundleMusic);
		_currentSampleBundleMusic = 0;
		_offsetSampleBundleMusic = 0;
		_offsetBufBundleMusic = 0;
		_musicBundleToBeChanged = false;
		_bundleMusicPosition = 0;
	}

	ptr = _musicBundleBufOutput;

	for (k = 0, l = _currentSampleBundleMusic; l < num; k++) {
		length = _bundle->decompressMusicSampleByName(_nameBundleMusic, l, (_musicBundleBufOutput + ((k * 0x2000) + _offsetBufBundleMusic)));
		_offsetSampleBundleMusic += length;

		if (l == 0) {
			tag = READ_BE_UINT32(ptr); ptr += 4;
			if (tag != MKID_BE('iMUS')) {
				warning("Decompression of bundle song failed");
				_musicBundleToBeRemoved = true;
				return;
			}

			ptr += 12;
			while (tag != MKID_BE('DATA')) {
				tag = READ_BE_UINT32(ptr);  ptr += 4;
				switch(tag) {
				case MKID_BE('FRMT'):
					ptr += 12;
					_bundleMusicSampleBits = READ_BE_UINT32(ptr); ptr += 4;
					rate = READ_BE_UINT32(ptr); ptr += 4;
					_bundleSampleChannels = READ_BE_UINT32(ptr); ptr += 4;
				break;
				case MKID_BE('TEXT'):
				case MKID_BE('REGN'):
				case MKID_BE('STOP'):
				case MKID_BE('JUMP'):
				case MKID_BE('SYNC'):
					size = READ_BE_UINT32(ptr); ptr += size + 4;
				break;
					case MKID_BE('DATA'):
					size = READ_BE_UINT32(ptr); ptr += 4;
				break;

				default:
					error("Unknown sound header %c%c%c%c",
						(byte)(tag >> 24),
						(byte)(tag >> 16),
						(byte)(tag >> 8),
						(byte)tag);
				}
			}
			if (size < 0) {
				warning("Decompression sound failed (no size field)");
				_musicBundleToBeRemoved = true;
				return;
			}
			header_size = (ptr - _musicBundleBufOutput);
		}

		l++;
		_currentSampleBundleMusic = l;

		if (_offsetSampleBundleMusic >= _outputMixerSize + header_size) {
			memcpy(_musicBundleBufFinal, (_musicBundleBufOutput + header_size), _outputMixerSize);
			_offsetBufBundleMusic = _offsetSampleBundleMusic - _outputMixerSize - header_size;
			memcpy(_musicBundleBufOutput, (_musicBundleBufOutput + (_outputMixerSize + header_size)), _offsetBufBundleMusic);
			_offsetSampleBundleMusic = _offsetBufBundleMusic;
			break;
		}
	}

	if (_currentSampleBundleMusic == num) {
		_currentSampleBundleMusic = 0;
		_offsetSampleBundleMusic = 0;
		_offsetBufBundleMusic = 0;
		_bundleMusicPosition = 0;
	}

	ptr = _musicBundleBufFinal;

	byte *buffer = NULL;
	uint32 final_size;
	if (_bundleMusicSampleBits == 12) {
		final_size = decode12BitsSample(ptr, &buffer, _outputMixerSize, false);
	} else if (_bundleMusicSampleBits == 16) {
		buffer = (byte *)malloc(_outputMixerSize);
		final_size = _outputMixerSize;
		memcpy(buffer, ptr, _outputMixerSize);
	} else {
		warning("Sound::bundleMusicHandler TODO: more newStream options...");
		return;
	}

	_bundleMusicPosition += final_size;
	if (_bundleMusicTrack == -1) {
		_bundleMusicTrack = _scumm->_mixer->newStream(buffer, final_size, rate,
															SoundMixer::FLAG_16BITS | SoundMixer::FLAG_STEREO, 300000);
	} else {
		_scumm->_mixer->appendStream(_bundleMusicTrack, buffer, final_size);
	}
	free(buffer);
}

void Sound::playBundleSound(char *sound, PlayingSoundHandle *handle) {
	byte *ptr = 0, *orig_ptr = 0;
	byte *final;
	bool result;

	if (_scumm->_noDigitalSamples)
		return;	

	if (_scumm->_gameId == GID_CMI) {
		char voxfile[20];
		sprintf(voxfile, "voxdisk%d.bun", _scumm->VAR(_scumm->VAR_CURRENTDISK));
		if (_voiceDisk != _scumm->VAR(_scumm->VAR_CURRENTDISK))
			_bundle->closeVoiceFile();

		result = _bundle->openVoiceFile(voxfile, _scumm->getGameDataPath());

		if (result == false) 
			result = _bundle->openVoiceFile("voice.bun", _scumm->getGameDataPath());
		_voiceDisk = (byte)_scumm->VAR(_scumm->VAR_CURRENTDISK);
	} else if (_scumm->_gameId == GID_DIG)
		result = _bundle->openVoiceFile("digvoice.bun", _scumm->getGameDataPath());
	else
		error("Don't know which bundle file to load");

	if (!result)
		return;

	int32 rate = 22050, channels, output_size = 0;
	int32 tag, size = -1, bits = 0;

	if (_scumm->_gameId == GID_CMI) {
		char name[20];
		strcpy(name, sound);
		if (_scumm->_maxRooms != 6) // CMI demo does not have .IMX for voice but does for music...
			strcat(name, ".IMX");
		output_size = _bundle->decompressVoiceSampleByName(name, &ptr);
	} else {
		output_size = _bundle->decompressVoiceSampleByName(sound, &ptr);
	}

	orig_ptr = ptr;
	if (output_size == 0 || orig_ptr == 0) {
		goto bail;
	}

	tag = READ_BE_UINT32(ptr); ptr += 4;
	if (tag != MKID_BE('iMUS')) {
		warning("Decompression of bundle sound failed");
		goto bail;
	}

	ptr += 12;
	while (tag != MKID_BE('DATA')) {
		tag = READ_BE_UINT32(ptr); ptr += 4;
		switch(tag) {
			case MKID_BE('FRMT'):
				ptr += 12;
				bits = READ_BE_UINT32(ptr); ptr += 4;
				rate = READ_BE_UINT32(ptr); ptr += 4;
				channels = READ_BE_UINT32(ptr); ptr += 4;
			break;
			case MKID_BE('TEXT'):
			case MKID_BE('REGN'):
			case MKID_BE('STOP'):
			case MKID_BE('JUMP'):
			case MKID_BE('SYNC'):
				size = READ_BE_UINT32(ptr); ptr += size + 4;
			break;

			case MKID_BE('DATA'):
				size = READ_BE_UINT32(ptr); ptr += 4;
			break;

			default:
				error("Unknown sound header %c%c%c%c",
					(byte)(tag >> 24),
					(byte)(tag >> 16),
					(byte)(tag >> 8),
					(byte)tag);
		}
	}

	if (size < 0) {
		warning("Decompression sound failed (no size field)");
		goto bail;
	}

	final = (byte *)malloc(size);
	memcpy(final, ptr, size);

	if (_scumm->_actorToPrintStrFor != 0xFF && _scumm->_actorToPrintStrFor != 0) {
		Actor *a = _scumm->derefActor(_scumm->_actorToPrintStrFor, "playBundleSound");
		rate = (rate * a->talkFrequency) / 256;
	}
	
	// Stop any sound currently playing on the given handle
	if (handle)
		_scumm->_mixer->stopHandle(*handle);

	if (bits == 8) {
		_scumm->_mixer->playRaw(handle, final, size, rate, SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
	} else if (bits == 16) {
		_scumm->_mixer->playRaw(handle, final, size, rate, SoundMixer::FLAG_16BITS | SoundMixer::FLAG_AUTOFREE);
	} else {
		warning("Sound::playBundleSound() to do more options to playRaw...");
	}
	
bail:
	free(orig_ptr);
}

void Sound::playSfxSound(void *sound, uint32 size, uint rate, bool isUnsigned, PlayingSoundHandle *handle) {
	if (_soundsPaused)
		return;
	byte flags = SoundMixer::FLAG_AUTOFREE;
	if (isUnsigned)
		flags |= SoundMixer::FLAG_UNSIGNED;
	_scumm->_mixer->playRaw(handle, sound, size, rate, flags);
}

void Sound::playSfxSound_MP3(void *sound, uint32 size, PlayingSoundHandle *handle) {
#ifdef USE_MAD
	if (!_soundsPaused && !_scumm->_noDigitalSamples)
		_scumm->_mixer->playMP3(handle, sound, size, SoundMixer::FLAG_AUTOFREE);
#endif
}

#ifdef USE_VORBIS
// Provide a virtual file to vorbisfile based on preloaded data
struct data_file_info {
	char *data;
	uint32 size;
	int curr_pos;
};

static size_t read_data(void *ptr, size_t size, size_t nmemb, void *datasource) {
	data_file_info *f = (data_file_info *) datasource;

	nmemb *= size;
	if (f->curr_pos < 0)
		return (size_t) -1;
	if (f->curr_pos > (int) f->size)
		nmemb = 0;
	else if (f->curr_pos + nmemb > f->size)
		nmemb = f->size - f->curr_pos;

	memcpy(ptr, f->data + f->curr_pos, nmemb);
	f->curr_pos += nmemb;
	return nmemb / size;
}

static int seek_data(void *datasource, ogg_int64_t offset, int whence) {
	data_file_info *f = (data_file_info *) datasource;

	switch (whence) {
	case SEEK_SET:
		f->curr_pos = offset;
		break;
	case SEEK_CUR:
		f->curr_pos += offset;
		break;
	case SEEK_END:
		f->curr_pos = f->size + offset;
		break;
	default:
		return -1;
	}
	return f->curr_pos;
}

static int close_data(void *datasource) {
	data_file_info *f = (data_file_info *) datasource;

	free(f->data);
	delete f;
	return 0;
}

static long tell_data(void *datasource) {
	data_file_info *f = (data_file_info *) datasource;

	return f->curr_pos;
}

static ov_callbacks data_wrap = {
	read_data, seek_data, close_data, tell_data
};
#endif

void Sound::playSfxSound_Vorbis(void *sound, uint32 size, PlayingSoundHandle *handle) {
#ifdef USE_VORBIS
	if (!_soundsPaused && !_scumm->_noDigitalSamples) {
		OggVorbis_File *ov_file = new OggVorbis_File;
		data_file_info *f = new data_file_info;
		f->data = (char *) sound;
		f->size = size;
		f->curr_pos = 0;
	
		if (ov_open_callbacks((void *) f, ov_file, NULL, 0, data_wrap) < 0) {
			warning("Invalid file format");
			delete ov_file;
			delete f;
		} else
			_scumm->_mixer->playVorbis(handle, ov_file, 0, false);
	}
#endif
}

// We use a real timer in an attempt to get better sync with CD tracks. This is
// necessary for games like Loom CD.

static void cd_timer_handler(void *ptr) {
	Scumm *scumm = (Scumm *) ptr;

	// FIXME: Turn off the timer when it's no longer needed. In theory, it
	// should be possible to check with pollCD(), but since CD sound isn't
	// properly restarted when reloading a saved game, I don't dare to.

	scumm->VAR(scumm->VAR_MUSIC_TIMER) += 6;
}

void Sound::startCDTimer() {
	int timer_interval;

	// The timer interval has been tuned for Loom CD and the Monkey 1
	// intro. I have to use 100 for Loom, or there will be a nasty stutter
	// when Chaos first appears, and I have to use 101 for Monkey 1 or the
	// intro music will be cut short.

	if (_scumm->_gameId == GID_LOOM256)
		timer_interval = 100;
	else 
		timer_interval = 101;

	_scumm->_timer->releaseProcedure(&cd_timer_handler);
	_scumm->_timer->installProcedure(&cd_timer_handler, 1000 * timer_interval);
}

void Sound::stopCDTimer() {
	_scumm->_timer->releaseProcedure(&cd_timer_handler);
}

void Sound::playCDTrack(int track, int numLoops, int startFrame, int endFrame) {
	if (playMP3CDTrack(track, numLoops, startFrame, endFrame) == -1)
		_scumm->_system->play_cdrom(track, numLoops, startFrame, endFrame);

	// Start the timer after starting the track. Starting an MP3 track is
	// almost instantaneous, but a CD player may take some time. Hopefully
	// play_cdrom() will block during that delay.

	startCDTimer();
}

void Sound::stopCD() {
	stopCDTimer();
	if (stopMP3CD() == -1)
		_scumm->_system->stop_cdrom();
}

int Sound::pollCD() const {
	if (pollMP3CD())
		return 1;

	return _scumm->_system->poll_cdrom();
}

void Sound::updateCD() {
	if (updateMP3CD() == -1)
		_scumm->_system->update_cdrom();
}

int Sound::getCachedTrack(int track) {
	int i;
#if defined(USE_MAD) || defined(USE_VORBIS)
	char track_name[1024];
	File *file = new File();
#endif
	int current_index;

	// See if we find the track in the cache
	for (i = 0; i < CACHE_TRACKS; i++)
		if (_cached_tracks[i] == track) {
			if (_track_info[i])
				return i;
			else
				return -1;
		}
	current_index = _current_cache++;
	_current_cache %= CACHE_TRACKS;

	// Not found, see if it exists

	// First, delete the previous track info object
	delete _track_info[current_index];
	_track_info[current_index] = NULL;

	_cached_tracks[current_index] = track;

#ifdef USE_MAD
	sprintf(track_name, "track%d.mp3", track);
	file->open(track_name, _scumm->getGameDataPath());

	if (file->isOpen()) {
		_track_info[current_index] = new MP3TrackInfo(file);
		if (_track_info[current_index]->error()) {
			delete _track_info[current_index];
			_track_info[current_index] = NULL;
			return -1;
		}
		return current_index;
	}
#endif

#ifdef USE_VORBIS
	sprintf(track_name, "track%d.ogg", track);
	file->open(track_name, _scumm->getGameDataPath());

	if (file->isOpen()) {
		_track_info[current_index] = new VorbisTrackInfo(file);
		if (_track_info[current_index]->error()) {
			delete _track_info[current_index];
			_track_info[current_index] = NULL;
			return -1;
		}
		return current_index;
	}
#endif

	debug(2, "Track %d not available in compressed format", track);
	return -1;
}

int Sound::playMP3CDTrack(int track, int numLoops, int startFrame, int endFrame) {
	int index;
	_scumm->VAR(_scumm->VAR_MUSIC_TIMER) = 0;

	if (_soundsPaused)
		return 0;

	if ((numLoops == 0) && (startFrame == 0)) {
		return 0;
	}

	index = getCachedTrack(track);
	if (index < 0)
		return -1;

	if (_dig_cd_playing)
		_scumm->_mixer->stop(_dig_cd_index);
	_dig_cd_index = _track_info[index]->play(_scumm->_mixer, startFrame, endFrame);
	_dig_cd_playing = true;
	_dig_cd_track = track;
	_dig_cd_num_loops = numLoops;
	_dig_cd_start = startFrame;
	_dig_cd_end = endFrame;
	return 0;
}

int Sound::stopMP3CD() {
	if (_dig_cd_playing) {
		_scumm->_mixer->stop(_dig_cd_index);
		_dig_cd_playing = false;
		_dig_cd_track = 0;
		_dig_cd_num_loops = 0;
		_dig_cd_start = 0;
		_dig_cd_end = 0;
		return 0;
	}
	return -1;
}

int Sound::pollMP3CD() const {
	if (_dig_cd_playing)
		return 1;
	return 0;
}

int Sound::updateMP3CD() {
	if (_dig_cd_playing == false)
		return -1;

	if (_scumm->_mixer->_channels[_dig_cd_index] == NULL) {
		warning("Error in MP3 decoding");
		return -1;
	}

	if (!_scumm->_mixer->isActiveChannel(_dig_cd_index)) {
		if (_dig_cd_num_loops == -1 || --_dig_cd_num_loops > 0)
			playMP3CDTrack(_dig_cd_track, _dig_cd_num_loops, _dig_cd_start, _dig_cd_end);
		else
			stopMP3CD();
	}

	return 0;
}

#ifdef USE_MAD
MP3TrackInfo::MP3TrackInfo(File *file) {
	struct mad_stream stream;
	struct mad_frame frame;
	unsigned char buffer[8192];
	unsigned int buflen = 0;
	int count = 0;

	// Check the format and bitrate
	mad_stream_init(&stream);
	mad_frame_init(&frame);

	while (1) {
		if (buflen < sizeof(buffer)) {
			int bytes;

			bytes = file->read(buffer + buflen, sizeof(buffer) - buflen);
			if (bytes <= 0) {
				if (bytes == -1) {
					warning("Invalid file format");
					goto error;
				}
				break;
			}

			buflen += bytes;
		}

		mad_stream_buffer(&stream, buffer, buflen);

		while (1) {
			if (mad_frame_decode(&frame, &stream) == -1) {
				if (!MAD_RECOVERABLE(stream.error))
					break;

				if (stream.error != MAD_ERROR_BADCRC)
					continue;
			}

			if (count++)
				break;
		}

		if (count || stream.error != MAD_ERROR_BUFLEN)
			break;

		memmove(buffer, stream.next_frame,
		        buflen = &buffer[buflen] - stream.next_frame);
	}

	if (count)
		memcpy(&_mad_header, &frame.header, sizeof(mad_header));
	else {
		warning("Invalid file format");
		goto error;
	}

	mad_frame_finish(&frame);
	mad_stream_finish(&stream);
	// Get file size
	_size = file->size();
	_file = file;
	_error_flag = false;
	return;

error:
	mad_frame_finish(&frame);
	mad_stream_finish(&stream);
	_error_flag = true;
	delete file;
}

int MP3TrackInfo::play(SoundMixer *mixer, int startFrame, int endFrame) {
	unsigned int offset;
	mad_timer_t duration;

	// Calc offset. As all bitrates are in kilobit per seconds, the division by 200 is always exact
	offset = (startFrame * (_mad_header.bitrate / (8 * 25))) / 3;

	// Calc delay
	if (!endFrame) {
		mad_timer_set(&duration, (_size * 8) / _mad_header.bitrate,
		              (_size * 8) % _mad_header.bitrate, _mad_header.bitrate);
	} else {
		mad_timer_set(&duration, endFrame / 75, endFrame % 75, 75);
	}	

	// Go
	_file->seek(offset, SEEK_SET);

	return mixer->playMP3CDTrack(NULL, _file, duration);
}

MP3TrackInfo::~MP3TrackInfo() {
	if (! _error_flag)
		_file->close();
}

#endif

#ifdef USE_VORBIS
// These are wrapper functions to allow using a File object to
// provide data to the OggVorbis_File object.

struct file_info {
	File *file;
	int start, curr_pos;
	size_t len;
};

static size_t read_wrap(void *ptr, size_t size, size_t nmemb, void *datasource) {
	file_info *f = (file_info *) datasource;
	int result;

	nmemb *= size;
	if (f->curr_pos > (int) f->len)
		nmemb = 0;
	else if (nmemb > f->len - f->curr_pos)
		nmemb = f->len - f->curr_pos;
	result = f->file->read(ptr, nmemb);
	if (result == -1) {
		f->curr_pos = f->file->pos() - f->start;
		return (size_t) -1;
	}
	else {
		f->curr_pos += result;
		return result / size;
	}
}

static int seek_wrap(void *datasource, ogg_int64_t offset, int whence) {
	file_info *f = (file_info *) datasource;

	if (whence == SEEK_SET)
		offset += f->start;
	else if (whence == SEEK_END) {
		offset += f->start + f->len;
		whence = SEEK_SET;
	}

	f->file->seek(offset, whence);
	f->curr_pos = f->file->pos() - f->start;
	return f->curr_pos;
}

static int close_wrap(void *datasource) {
	file_info *f = (file_info *) datasource;

	f->file->close();
	delete f;
	return 0;
}

static long tell_wrap(void *datasource) {
	file_info *f = (file_info *) datasource;

	return f->file->pos();
}

static ov_callbacks File_wrap = {
	read_wrap, seek_wrap, close_wrap, tell_wrap
};

VorbisTrackInfo::VorbisTrackInfo(File *file) {
	file_info *f = new file_info;

	f->file = file;
	f->start = 0;
	f->len = file->size();
	f->curr_pos = file->pos();

	if (ov_open_callbacks((void *) f, &_ov_file, NULL, 0, File_wrap) < 0) {
		warning("Invalid file format");
		_error_flag = true;
		delete f;
		delete file;
	}
	else {
		_error_flag = false;
		_file = file;

		// Check the file format
		if (ov_info(&_ov_file, -1)->rate != 22050)
			warning("Vorbis code currently only supports files encoded at 22050 Hz");
	}
}

#ifdef CHUNKSIZE
#define VORBIS_TREMOR
#endif

int VorbisTrackInfo::play(SoundMixer *mixer, int startFrame, int endFrame) {
#ifdef VORBIS_TREMOR
	ov_time_seek(&_ov_file, (ogg_int64_t)(startFrame / 75.0 * 1000));
#else
	ov_time_seek(&_ov_file, startFrame / 75.0);
#endif
	return mixer->playVorbis(NULL, &_ov_file,
				 endFrame * ov_info(&_ov_file, -1)->rate / 75,
				 true);
}

VorbisTrackInfo::~VorbisTrackInfo() {
	if (! _error_flag) {
		ov_clear(&_ov_file);
		delete _file;
	}
}

#endif
