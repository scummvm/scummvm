/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
#include "scumm/scumm.h"
#include "sound/mididrv.h"
#include "scumm/imuse.h"
#include "scumm/actor.h"
#include "config-file.h"
#include "util.h"

#ifdef _WIN32_WCE
extern void *bsearch(const void *, const void *, size_t,
										 size_t, int (*x) (const void *, const void *));
#endif

Sound::Sound(Scumm *parent) {
	_scumm = parent;
	_numberBundleMusic = -1;
	_musicBundleBufFinal = NULL;
	_musicBundleBufOutput = NULL;
}

Sound::~Sound() {
}

void Sound::addSoundToQueue(int sound) {
	if (!(_scumm->_features & GF_AFTER_V7)) {
		_scumm->_vars[_scumm->VAR_LAST_SOUND] = sound;
		_scumm->ensureResourceLoaded(rtSound, sound);
		addSoundToQueue2(sound);
	} else {
		// WARNING ! This may break something, maybe this sould be put inside if(_gameID == GID_FT) ? 
		// But why addSoundToQueue should not queue sound ?
		_scumm->ensureResourceLoaded(rtSound, sound);
		addSoundToQueue2(sound);
	}

//	if (_features & GF_AUDIOTRACKS)
//		warning("Requesting audio track: %d", sound);
}

void Sound::addSoundToQueue2(int sound) {
	if (_soundQue2Pos < 10) {
		_soundQue2[_soundQue2Pos++] = sound;
	}
}

void Sound::processSoundQues() {
	byte d;
	int i, j;
	int num;
	int16 data[16];
	IMuse *se;

	processSfxQueues();

	while (_soundQue2Pos) {
		d = _soundQue2[--_soundQue2Pos];
		if (d)
			playSound(d);
	}

	for (i = 0; i < _soundQuePos;) {
		num = _soundQue[i++];
		if (i + num > _soundQuePos) {
			warning("processSoundQues: invalid num value");
			break;
		}
		for (j = 0; j < 16; j++)
			data[j] = 0;
		if (num > 0) {
			for (j = 0; j < num; j++)
				data[j] = _soundQue[i + j];
			i += num;

			se = _scumm->_imuse;
#if 0
			debug(1, "processSoundQues(%d,%d,%d,%d,%d,%d,%d,%d,%d)",
						data[0] >> 8,
						data[0] & 0xFF,
						data[1], data[2], data[3], data[4], data[5], data[6], data[7]
				);
#endif
			
			if ((_scumm->_gameId == GID_DIG) && (data[0] == 4096)){
					playBundleMusic(data[1] - 1);
			}

			if (!(_scumm->_features & GF_AFTER_V7)) {
				if (se)
					_scumm->_vars[_scumm->VAR_SOUNDRESULT] =
						(short)se->do_command(data[0], data[1], data[2], data[3], data[4],
																	data[5], data[6], data[7]);
			}

		}
	}
	_soundQuePos = 0;
}

static char * read_creative_voc_file(byte * ptr, int & size, int & rate) {
	assert(strncmp((char*)ptr, "Creative Voice File\x1A", 20) == 0);
	int offset = READ_LE_UINT16(ptr+20);
	short version = READ_LE_UINT16(ptr+22);
	short code = READ_LE_UINT16(ptr+24);
	assert(version == 0x010A || version == 0x0114);
	assert(code == ~version + 0x1234);
	bool quit = 0;
	char * ret_sound = 0; size = 0;
	while(!quit) {
		int len = READ_LE_UINT32(ptr + offset);
		offset += 4;
		int code = len & 0xFF;		// FIXME not sure this is endian correct
		len >>= 8;
		switch(code) {
			case 0: quit = 1; break;
			case 1: {
				int time_constant = ptr[offset++];
				int packing = ptr[offset++];
				len -= 2;
				rate = 1000000L / (256L - time_constant);
				debug(9, "VOC Data Bloc : %d, %d, %d", rate, packing, len);
				if(packing == 0) {
					if(size) {
						ret_sound = (char*)realloc(ret_sound, size + len);
					} else {
						ret_sound = (char*)malloc(len);
					}
					memcpy(ret_sound + size, ptr + offset, len);
					size += len;
				} else {
					warning("VOC file packing %d unsupported", packing);
				}
				} break;
			case 6:	// begin of loop
				debug(3, "loops in Creative files not supported");
				break;
			case 7:	// end of loop
				break;
			default:
				warning("Invalid code in VOC file : %d", code);
				//~ quit = 1;
				break;
		}
		offset += len;
	}
	debug(9, "VOC Data Size : %d", size);
	return ret_sound;
}

void Sound::playSound(int sound) {
	byte *ptr;
	
	ptr = _scumm->getResourceAddress(rtSound, sound);
	if (ptr != NULL && READ_UINT32_UNALIGNED(ptr) == MKID('SOUN')) {
		ptr += 8;
		_scumm->_vars[_scumm->VAR_MI1_TIMER] = 0;
#ifdef COMPRESSED_SOUND_FILE
		if ((playMP3CDTrack(ptr[16], ptr[17] == 0xff ? -1 : ptr[17],
						(ptr[18] * 60 + ptr[19]) * 75 + ptr[20], 0)) == -1)
#endif
 		_scumm->_system->play_cdrom(ptr[16], ptr[17] == 0xff ? -1 : ptr[17],
 						(ptr[18] * 60 + ptr[19]) * 75 + ptr[20], 0);

		_scumm->current_cd_sound = sound;
		return;
	}
	// Support for SFX in Monkey Island 1, Mac version
	// This is rather hackish right now, but works OK. SFX are not sounding
	// 100% correct, though, not sure right now what is causing this.
	else if (ptr != NULL && READ_UINT32_UNALIGNED(ptr) == MKID('Mac1')) {

		// Read info from the header
		int size = READ_UINT32_UNALIGNED(ptr+0x60);
		int rate = READ_UINT32_UNALIGNED(ptr+0x64) >> 16;

		// Skip over the header (fixed size)
		ptr += 0x72;
		
		// Allocate a sound buffer, copy the data into it, and play
		char *sound = (char*)malloc(size);
		memcpy(sound, ptr, size);
		_scumm->_mixer->playRaw(NULL, sound, size, rate, SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
		return;
	}
	// Support for Putt-Putt sounds - very hackish, too 8-)
	else if (ptr != NULL && READ_UINT32_UNALIGNED(ptr) == MKID('DIGI')) {
		// TODO - discover what data the first chunk, HSHD, contains
		// it might be useful here.
		ptr += 8 + READ_BE_UINT32_UNALIGNED(ptr+12);
		if (READ_UINT32_UNALIGNED(ptr) != MKID('SDAT'))
			return;	// abort

		int size = READ_BE_UINT32_UNALIGNED(ptr+4);
		int rate = 8000;	// FIXME - what value here ?!? this is just a guess for now
		
		// Allocate a sound buffer, copy the data into it, and play
		char *sound = (char*)malloc(size);
		memcpy(sound, ptr + 8, size);
		_scumm->_mixer->playRaw(NULL, sound, size, rate, SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
		return;
	}
	else if (ptr != NULL && READ_UINT32_UNALIGNED(ptr) == MKID('Crea')) {
		int size, rate;
		char * sound = read_creative_voc_file(ptr, size, rate);
		if(sound != NULL) {
			_scumm->_mixer->playRaw(NULL, sound, size, rate, SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
		}
		return;
	}
	// Support for sampled sound effects in Monkey1 and Monkey2
	else if (ptr != NULL && READ_UINT32_UNALIGNED(ptr) == MKID('SBL ')) {
		debug(2, "Using SBL sound effect");

		// TODO - Figuring out how the SBL chunk works. Here's an
		// example:
		//
		// 53 42 4c 20 00 00 11 ae  |SBL ....|
		// 41 55 68 64 00 00 00 03  |AUhd....|
		// 00 00 80 41 55 64 74 00  |...AUdt.|
		// 00 11 9b 01 96 11 00 a6  |........|
		// 00 7f 7f 7e 7e 7e 7e 7e  |...~~~~~|
		// 7e 7f 7f 80 80 7f 7f 7f  |~.......|
		// 7f 80 80 7f 7e 7d 7d 7e  |....~}}~|
		// 7e 7e 7e 7e 7e 7e 7e 7f  |~~~~~~~.|
		// 7f 7f 7f 80 80 80 80 80  |........|
		// 80 81 80 80 7f 7f 80 85  |........|
		// 8b 8b 83 78 72 6d 6f 75  |...xrmou|
		// 7a 78 77 7d 83 84 83 81  |zxw}....|
		//
		// The length of the AUhd chunk always seems to be 3 bytes.
		// Let's skip that for now.
		//
		// The starting offset, length and sample rate is all pure
		// guesswork. The result sounds reasonable to me, but I've
		// never heard the original.

		int size = READ_BE_UINT32_UNALIGNED(ptr + 4) - 27;
		int rate = 8000;

		// Allocate a sound buffer, copy the data into it, and play
		char *sound = (char*)malloc(size);
		memcpy(sound, ptr + 33, size);
		_scumm->_mixer->playRaw(NULL, sound, size, rate, SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
		return;
	}

	if ((_scumm->_features & GF_OLD256) && (ptr != NULL)) {
		char *sound;
		int size = READ_LE_UINT32(ptr);
		
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
		the 00 00 00 3c - the sound data seems to start at
		offset 54.
		*/
#endif
		
		ptr += 0x16;
		if (size == 30) {
			int result = 0;
			int track = *ptr;

			if (track == _scumm->current_cd_sound)
#ifdef COMPRESSED_SOUND_FILE
				if (pollMP3CD())
					result = 1;
				else
#endif
				result = _scumm->_system->poll_cdrom();
			if (result == 1) return;

#ifdef COMPRESSED_SOUND_FILE
        	        if (playMP3CDTrack(track, 1, 0, 0) == -1)
#endif
	                _scumm->_system->play_cdrom(track, 0, 0, 0);
	                _scumm->current_cd_sound = track;
			return;
		}

		size -= 0x36;
		sound = (char*)malloc(size);
		for (int x = 0; x < size; x++) {
			int bit = *ptr++;
			if (bit<0x80) sound[x] = 0x7F-bit; else sound[x] = bit;
		}

		// FIXME: Something in the header signifies looping. Need to track it down and add a 
		//	  mixer flag or something.
		_scumm->_mixer->playRaw(NULL, sound, size, 11000, SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
		return;
	}

	if (_scumm->_gameId == GID_MONKEY_VGA)
		return;											/* FIXME */

	IMuse *se = _scumm->_imuse;
	if (se) {
		_scumm->getResourceAddress(rtSound, sound);
		se->start_sound(sound);
	}
}

void Sound::processSfxQueues() {
	Actor *a;
	int act;
	bool b, finished;

	if (_talk_sound_mode != 0) {
		if (_talk_sound_mode == 2)
			_talkChannel = startTalkSound(_talk_sound_a, _talk_sound_b, _talk_sound_mode);
		else
			startTalkSound(_talk_sound_a, _talk_sound_b, _talk_sound_mode);
		_talk_sound_mode = 0;
	}

	if (_scumm->_vars[_scumm->VAR_TALK_ACTOR]) { //_sfxMode == 2) {
		act = _scumm->_vars[_scumm->VAR_TALK_ACTOR];
		if (_talkChannel < 0)
			finished = false;
		else if (_scumm->_mixer->_channels[_talkChannel] == NULL)
			finished = true;
		else
			finished = false;
		

		if (act != 0 && (uint) act < 0x80 && !_scumm->string[0].no_talk_anim) {
			a = _scumm->derefActorSafe(act, "processSfxQueues");
			if (a->room == _scumm->_currentRoom && (finished || !_endOfMouthSync)) {
				b = true;
				if (!finished)
					b = isMouthSyncOff(_curSoundPos);
				if (_mouthSyncMode != b) {
					_mouthSyncMode = b;
					a->startAnimActor(b ? a->talkFrame2 : a->talkFrame1);
				}
			}
		}
		
		if (finished  && _scumm->_talkDelay == 0) {
			_scumm->stopTalk();
			_sfxMode = 0;
			_talkChannel = -1;
		}
	}
		
	if (_sfxMode == 1) {
		if (isSfxFinished()) {
			_sfxMode = 0;
		}
	}
}

#ifdef COMPRESSED_SOUND_FILE
static int compar(const void *a, const void *b)
{
	return ((MP3OffsetTable *) a)->org_offset -
		((MP3OffsetTable *) b)->org_offset;
}
#endif

int Sound::startTalkSound(uint32 offset, uint32 b, int mode) {
	int num = 0, i;
	byte file_byte, file_byte_2;
	int size;

	if (!_sfxFile) {
		warning("startTalkSound: SFX file is not open");
		return -1;
	}

	if (b > 8) {
		num = (b - 8) >> 1;
	}
#ifdef COMPRESSED_SOUND_FILE
	if (offset_table != NULL) {
		MP3OffsetTable *result = NULL, key;

		key.org_offset = offset;
		result = (MP3OffsetTable *) bsearch(&key, offset_table, num_sound_effects,
		                                    sizeof(MP3OffsetTable), compar);

		if (result == NULL) {
			warning("startTalkSound: did not find sound at offset %d !", offset);
			return -1;
		}
		if (2 * num != result->num_tags) {
			warning("startTalkSound: number of tags do not match (%d - %d) !", b,
							result->num_tags);
			num = result->num_tags;
		}
		offset = result->new_offset;
		size = result->compressed_size;
	} else
#endif
	{
		offset += 8;
		size = -1;
	}

	_scumm->fileSeek((FILE *) _sfxFile, offset, SEEK_SET);
	i = 0;
	while (num > 0) {
		_scumm->fileRead((FILE *) _sfxFile, &file_byte, sizeof(file_byte));
		_scumm->fileRead((FILE *) _sfxFile, &file_byte_2, sizeof(file_byte_2));
		_mouthSyncTimes[i++] = file_byte | (file_byte_2 << 8);
		num--;
	}
	_mouthSyncTimes[i] = 0xFFFF;
	_sfxMode = mode;
	_curSoundPos = 0;
	_mouthSyncMode = true;

	return startSfxSound(_sfxFile, size);
}

void Sound::stopTalkSound() {
	if (_sfxMode == 2) {
		stopSfxSound();
		_sfxMode = 0;
	}
}

bool Sound::isMouthSyncOff(uint pos) {
	uint j;
	bool val = true;
	uint16 *ms = _mouthSyncTimes;

	_endOfMouthSync = false;
	do {
		val ^= 1;
		j = *ms++;
		if (j == 0xFFFF) {
			_endOfMouthSync = true;
			break;
		}
	} while (pos > j);
	return val;
}


int Sound::isSoundRunning(int sound) {
	IMuse *se;
	int i;

	if (sound == _scumm->current_cd_sound)
#ifdef COMPRESSED_SOUND_FILE
		if (pollMP3CD())
			return 1;
		else
#endif
			return _scumm->_system->poll_cdrom();

	i = _soundQue2Pos;
	while (i--) {
		if (_soundQue2[i] == sound)
			return 1;
	}

	if (isSoundInQueue(sound))
		return 1;

	if (!_scumm->isResourceLoaded(rtSound, sound))
		return 0;

	se = _scumm->_imuse;
	if (!se)
		return 0;
	return se->get_sound_status(sound);
}

bool Sound::isSoundInQueue(int sound) {
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
	IMuse *se;
	int i;

	if (a != 0 && a == _scumm->current_cd_sound) {
		_scumm->current_cd_sound = 0;
#ifdef COMPRESSED_SOUND_FILE
		if (stopMP3CD() == -1)
#endif
			_scumm->_system->stop_cdrom();
	}

	se = _scumm->_imuse;
	if (se)
		se->stop_sound(a);

	for (i = 0; i < 10; i++)
		if (_soundQue2[i] == (byte)a)
			_soundQue2[i] = 0;
}

void Sound::stopAllSounds()
{
	IMuse *se = _scumm->_imuse;

	if (_scumm->current_cd_sound != 0) {
		_scumm->current_cd_sound = 0;
#ifdef COMPRESSED_SOUND_FILE
		if (stopMP3CD() == -1)
#endif
			_scumm->_system->stop_cdrom();
	}

	if (se) {
		se->stop_all_sounds();
		se->clear_queue();
	}
	clearSoundQue();
	stopSfxSound();
}

void Sound::clearSoundQue() {
	_soundQue2Pos = 0;
	memset(_soundQue2, 0, sizeof(_soundQue2));
}

void Sound::soundKludge(int16 * list) {
	int16 *ptr;
	int i;

	if (list[0] == -1) {
		processSoundQues();
		return;
	}
	_soundQue[_soundQuePos++] = 8;

	ptr = _soundQue + _soundQuePos;
	_soundQuePos += 8;

	for (i = 0; i < 8; i++)
		*ptr++ = list[i];
	if (_soundQuePos > 0x100)
		error("Sound que buffer overflow");
}

void Sound::talkSound(uint32 a, uint32 b, int mode) {
	_talk_sound_a = a;
	_talk_sound_b = b;
	_talk_sound_mode = mode;
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

		_sound_volume_music = scummcfg->getInt("music_volume", kDefaultMusicVolume);
		_sound_volume_master = scummcfg->getInt("master_volume", kDefaultMasterVolume);
		_sound_volume_sfx = scummcfg->getInt("sfx_volume", kDefaultSFXVolume);

		_scumm->_imuse->set_master_volume(_sound_volume_master);
		_scumm->_imuse->set_music_volume(_sound_volume_music);
		_scumm->_mixer->setVolume(_sound_volume_sfx);
		_scumm->_mixer->setMusicVolume(_sound_volume_music);
	}
	_sfxFile = openSfxFile();
}

void Sound::pauseSounds(bool pause) {
	IMuse *se = _scumm->_imuse;
	if (se)
		se->pause(pause);

	_soundsPaused = pause;
	_scumm->_mixer->pause(pause);	
}

int Sound::startSfxSound(void *file, int file_size) {
	char ident[8];
	int block_type;
	byte work[8];
	uint size = 0;
	int rate, comp;
	byte *data;

#ifdef COMPRESSED_SOUND_FILE
	if (file_size > 0) {
		data = (byte *)calloc(file_size + MAD_BUFFER_GUARD, 1);

		if (fread(data, file_size, 1, (FILE *) file) != 1) {
			/* no need to free the memory since error will shut down */
			error("startSfxSound: cannot read %d bytes", size);
			return -1;
		}
		return playSfxSound_MP3(data, file_size);
	}
#endif
	if (fread(ident, 8, 1, (FILE *) file) != 1)
		goto invalid;

	if (!memcmp(ident, "VTLK", 4)) {
		fseek((FILE *) file, SOUND_HEADER_BIG_SIZE - 8, SEEK_CUR);
	} else if (!memcmp(ident, "Creative", 8)) {
		fseek((FILE *) file, SOUND_HEADER_SIZE - 8, SEEK_CUR);
	} else {
	invalid:;
		warning("startSfxSound: invalid header");
		return -1;
	}

	block_type = getc((FILE *) file);
	if (block_type != 1) {
		warning("startSfxSound: Expecting block_type == 1, got %d", block_type);
		return -1;
	}

	fread(work, 3, 1, (FILE *) file);

	size = (work[0] | (work[1] << 8) | (work[2] << 16)) - 2;
	rate = getc((FILE *) file);
	comp = getc((FILE *) file);

	if (comp != 0) {
		warning("startSfxSound: Unsupported compression type %d", comp);
		return -1;
	}

	data = (byte *)malloc(size);
	if (data == NULL) {
		error("startSfxSound: out of memory");
		return -1;
	}

	if (fread(data, size, 1, (FILE *) file) != 1) {
		/* no need to free the memory since error will shut down */
		error("startSfxSound: cannot read %d bytes", size);
		return -1;
	}

	return playSfxSound(data, size, 1000000 / (256 - rate), true);
}


#ifdef COMPRESSED_SOUND_FILE
static int get_int(FILE * f) {
	int ret = 0;
	for (int size = 0; size < 4; size++) {
		int c = fgetc(f);
		if (c == EOF) {
			error("Unexpected end of file !!!");
		}
		ret <<= 8;
		ret |= c;
	}
	return ret;
}
#endif

void * Sound::openSfxFile() {
	char buf[256];
	FILE *file = NULL;

	/* Try opening the file <_exe_name>.sou first, eg tentacle.sou.
	 * That way, you can keep .sou files for multiple games in the
	 * same directory */
#ifdef COMPRESSED_SOUND_FILE
	offset_table = NULL;

	sprintf(buf, "%s%s.so3", _scumm->getGameDataPath(), _scumm->_exe_name);
	file = fopen(buf, "rb");
	if (!file) {
		sprintf(buf, "%smonster.so3", _scumm->getGameDataPath());
		file = fopen(buf, "rb");
	}
	if (file != NULL) {
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

		compressed_offset = get_int(file);
		offset_table = (MP3OffsetTable *) malloc(compressed_offset);
		num_sound_effects = compressed_offset / 16;

		size = compressed_offset;
		cur = offset_table;
		while (size > 0) {
			cur[0].org_offset = get_int(file);
			cur[0].new_offset = get_int(file) + compressed_offset + 4; /* The + 4 is to take into accound the 'size' field */
			cur[0].num_tags = get_int(file);
			cur[0].compressed_size = get_int(file);
			size -= 4 * 4;
			cur++;
		}
		return file;
	}
#endif
	sprintf(buf, "%s%s.sou", _scumm->getGameDataPath(), _scumm->_exe_name);
	file = fopen(buf, "rb");
	if (!file) {
		sprintf(buf, "%smonster.sou", _scumm->getGameDataPath());
		file = fopen(buf, "rb");
	}
	return file;
}

void Sound::stopSfxSound() {
	_scumm->_mixer->stopAll();
}


bool Sound::isSfxFinished() {
	return !_scumm->_mixer->hasActiveChannel();
}

uint32 Sound::decode12BitsSample(byte * src, byte ** dst, uint32 size) {
	uint32 s_size = (size * 4) / 3;
	byte * ptr = *dst = (byte*)malloc (s_size + 4);

	uint32 r = 0, tmp, l;
	for (l = 0; l < size; l += 3) {
		tmp = (src[l + 1] & 0x0f) << 8;
		tmp = (tmp | src[l + 0]) << 4;
		tmp -= 0x8000;
		ptr[r++] = (byte)((tmp >> 8) & 0xff);
		ptr[r++] = (byte)(tmp & 0xff);

		tmp = (src[l + 1] & 0xf0) << 4;
		tmp = (tmp | src[l + 2]) << 4;
		tmp -= 0x8000;
		ptr[r++] = (byte)((tmp >> 8) & 0xff);
		ptr[r++] = (byte)(tmp & 0xff);
	}

	return r;
}

static void music_handler (Scumm * scumm) {
	scumm->_sound->bundleMusicHandler(scumm);
}

#define OUTPUT_SIZE 66150 // ((22050 * 2 * 2) / 4) * 3

void Sound::playBundleMusic(int32 song) {
	char buf[256];

	if (_numberBundleMusic == -1) {
		sprintf(buf, "%s%smusic.bun", _scumm->getGameDataPath(), _scumm->_exe_name);
		if (_scumm->_bundle->openMusicFile((char*)&buf) == false)
			return;
		_musicBundleBufFinal = (byte*)malloc(OUTPUT_SIZE);
		_musicBundleBufOutput = (byte*)malloc(10 * 0x2000);
		_currentSampleBundleMusic = 0;
		_offsetSampleBundleMusic = 0;
		_offsetBufBundleMusic = 0;
		_pauseBundleMusic = false;	
		_numberSamplesBundleMusic = _scumm->_bundle->getNumberOfMusicSamplesByIndex(song);
		_numberBundleMusic = song;
		_scumm->_timer->installProcedure(&music_handler, 1000);
		return;
	}
	if (_numberBundleMusic != song) {
		_numberSamplesBundleMusic = _scumm->_bundle->getNumberOfMusicSamplesByIndex(song);
		_numberBundleMusic = song;
		_currentSampleBundleMusic = 0;
		_offsetSampleBundleMusic = 0;
		_offsetBufBundleMusic = 0;
	}
}

void Sound::pauseBundleMusic(bool state) {
	_pauseBundleMusic = state;
}

void Sound::stopBundleMusic() {
	_scumm->_timer->releaseProcedure(&music_handler);
	_numberBundleMusic = -1;
	if (_musicBundleBufFinal) {
		free(_musicBundleBufFinal);
		_musicBundleBufFinal = NULL;
	}
	if (_musicBundleBufOutput) {
		free(_musicBundleBufOutput);
		_musicBundleBufOutput = NULL;
	}
}

void Sound::bundleMusicHandler(Scumm * scumm) {
	byte * ptr;
	int32 l, num = _numberSamplesBundleMusic, length, k;
	int32 rate = 22050;
	int32 tag, size = -1, header_size = 0;
	
	ptr = _musicBundleBufOutput;
	
	if (_pauseBundleMusic)
		return;

	for (k = 0, l = _currentSampleBundleMusic; l < num; k++) {
		length = _scumm->_bundle->decompressMusicSampleByIndex(_numberBundleMusic, l, (_musicBundleBufOutput + ((k * 0x2000) + _offsetBufBundleMusic)));
		_offsetSampleBundleMusic += length;

		if (l == 0) {
			tag = READ_BE_UINT32(ptr); ptr += 4;
			if (tag != MKID_BE('iMUS')) {
				warning("Decompression of bundle sound failed");
				_numberBundleMusic = -1;
				return;
			}

			ptr += 12;
			while(tag != MKID_BE('DATA')) {
				tag = READ_BE_UINT32(ptr);  ptr += 4;
				switch(tag) {
				case MKID_BE('FRMT'):
					size = READ_BE_UINT32(ptr); ptr += 24;
				break;
				case MKID_BE('TEXT'):
				case MKID_BE('REGN'):
				case MKID_BE('STOP'):
				case MKID_BE('JUMP'):
					size = READ_BE_UINT32(ptr); ptr += size + 4;
				break;
					case MKID_BE('DATA'):
					size = READ_BE_UINT32(ptr); ptr += 4;
				break;

				default:
					error("Unknown sound header %c%c%c%c", tag>>24, tag>>16, tag>>8, tag);
				}
			}
			if (size < 0) {
				warning("Decompression sound failed (no size field)");
				_numberBundleMusic = -1;
				return;
			}
			header_size = (ptr - _musicBundleBufOutput);
		}
	
		l++;
		_currentSampleBundleMusic = l;

		if (_offsetSampleBundleMusic >= OUTPUT_SIZE + header_size) {
			memcpy(_musicBundleBufFinal, (_musicBundleBufOutput + header_size), OUTPUT_SIZE);
			_offsetBufBundleMusic = _offsetSampleBundleMusic - OUTPUT_SIZE - header_size;
			memcpy(_musicBundleBufOutput, (_musicBundleBufOutput + (OUTPUT_SIZE + header_size)), _offsetBufBundleMusic);
			_offsetSampleBundleMusic = _offsetBufBundleMusic;
			break;
		}
	}

	if (_currentSampleBundleMusic == num)
		_currentSampleBundleMusic = 0;

	size = OUTPUT_SIZE;
	ptr = _musicBundleBufFinal;

	byte * buffer = NULL;
	uint32 final_size = decode12BitsSample(ptr, &buffer, size);
	_scumm->_mixer->playRaw(NULL, buffer, final_size, rate, SoundMixer::FLAG_AUTOFREE | SoundMixer::FLAG_16BITS | SoundMixer::FLAG_STEREO);
}

void Sound::playBundleSound(char *sound) {
	char buf[256];
	byte * ptr;

	sprintf(buf, "%s%svoice.bun", _scumm->getGameDataPath(), _scumm->_exe_name);
	_scumm->_bundle->openVoiceFile((char*)&buf);
	ptr = (byte *)malloc(1000000);
	if (_scumm->_bundle->decompressVoiceSampleByName(sound, ptr) == 0) {
		delete ptr;
		return;
	}

	int rate = 22050;
	int tag, size = -1;

	tag = READ_BE_UINT32(ptr); ptr+=4;
	if (tag != MKID_BE('iMUS')) {
		warning("Decompression of bundle sound failed");
		free(ptr);
		return;
	}

	ptr += 12;
	while(tag != MKID_BE('DATA')) {
		tag = READ_BE_UINT32(ptr);  ptr+=4;
		switch(tag) {
			case MKID_BE('FRMT'):
				size = READ_BE_UINT32(ptr); ptr+=16;					
				rate = READ_BE_UINT32(ptr); ptr+=8;
			break;
			case MKID_BE('TEXT'):
			case MKID_BE('REGN'):
			case MKID_BE('STOP'):
			case MKID_BE('JUMP'):
				size = READ_BE_UINT32(ptr); ptr+=size+4;
			break;

			case MKID_BE('DATA'):
				size = READ_BE_UINT32(ptr); ptr+=4;
			break;

			default:
			error("Unknown sound header %c%c%c%c", tag>>24, tag>>16, tag>>8, tag);
		}
	}

	if (size < 0) {
		warning("Decompression sound failed (no size field)");
		free(ptr);
		return;
	}
	
	byte * final = (byte *)malloc(size);
	memcpy(final, ptr, size);
	_scumm->_mixer->playRaw(NULL, final, size, rate, SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
}

int Sound::playSfxSound(void *sound, uint32 size, uint rate, bool isUnsigned) {
	if (_soundsPaused)
		return -1;
	byte flags = SoundMixer::FLAG_AUTOFREE;
	if (isUnsigned)
		flags |= SoundMixer::FLAG_UNSIGNED;
	return _scumm->_mixer->playRaw(NULL, sound, size, rate, flags);
}

int Sound::playSfxSound_MP3(void *sound, uint32 size) {
#ifdef COMPRESSED_SOUND_FILE
	if (_soundsPaused)
		return -1;
	return _scumm->_mixer->playMP3(NULL, sound, size, SoundMixer::FLAG_AUTOFREE);
#endif
	return -1;
}

#ifdef COMPRESSED_SOUND_FILE

int Sound::getCachedTrack(int track) {
	int i;
	char track_name[1024];
	FILE* file;
	int current_index;
	struct mad_stream stream;
	struct mad_frame frame;
	unsigned char buffer[8192];
	unsigned int buflen = 0;
	int count = 0;

	// See if we find the track in the cache
	for (i=0; i<CACHE_TRACKS; i++)
		if (_cached_tracks[i] == track) {
			if (_mp3_tracks[i])
				return i;
			else
				return -1;
		}
	current_index = _current_cache++;
	_current_cache %= CACHE_TRACKS;

	// Not found, see if it exists
	sprintf(track_name, "%strack%d.mp3", _scumm->getGameDataPath(), track);
	file = fopen(track_name, "rb");
	_cached_tracks[current_index] = track;

	/* First, close the previous file */
	if (_mp3_tracks[current_index])
		fclose(_mp3_tracks[current_index]);
	_mp3_tracks[current_index] = NULL;
	if (!file) {
		// This warning is pretty pointless.
		debug(1, "Track %d not available in mp3 format", track);
		return -1;
	}
	// Check the format and bitrate
	mad_stream_init(&stream);
	mad_frame_init(&frame);

	while (1) {
		if (buflen < sizeof(buffer)) {
			int bytes;

			bytes = fread(buffer + buflen, 1, sizeof(buffer) - buflen, file);
			if (bytes <= 0) {
				if (bytes == -1) {
					warning("Invalid format for track %d", track);
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
		memcpy(&_mad_header[current_index], &frame.header, sizeof(mad_header));
	else {
		warning("Invalid format for track %d", track);
		goto error;
	}

	mad_frame_finish(&frame);
	mad_stream_finish(&stream);
	// Get file size
	fseek(file, 0, SEEK_END);
	_mp3_size[current_index] = ftell(file);
	_mp3_tracks[current_index] = file;
	
	return current_index;

 error:
	mad_frame_finish(&frame);
	mad_stream_finish(&stream);
	fclose(file);

	return -1;
}

int Sound::playMP3CDTrack(int track, int num_loops, int start, int delay) {
	int index;
	unsigned int offset;
	mad_timer_t duration;
	_scumm->_vars[_scumm->VAR_MI1_TIMER] = 0;

	if (_soundsPaused)
		return 0;

	if ((num_loops == 0) && (start == 0)) {
		return 0;
	}

	index = getCachedTrack(track);
	if (index < 0)
		return -1;

	// Calc offset. As all bitrates are in kilobit per seconds, the division by 200 is always exact
	offset = (start * (_mad_header[index].bitrate / (8 * 25))) / 3;

	// Calc delay
	if (!delay) {
		mad_timer_set(&duration, (_mp3_size[index] * 8) / _mad_header[index].bitrate,
		              (_mp3_size[index] * 8) % _mad_header[index].bitrate, _mad_header[index].bitrate);
	} else {
		mad_timer_set(&duration, delay / 75, delay % 75, 75);
	}	

	// Go
	fseek(_mp3_tracks[index], offset, SEEK_SET);

	if (_mp3_cd_playing == true)
		_scumm->_mixer->stop(_mp3_index);		
	_mp3_index = _scumm->_mixer->playMP3CDTrack(NULL, _mp3_tracks[index], duration);
	_mp3_cd_playing = true;
	return 0;
}

int Sound::stopMP3CD() {
	if (_mp3_cd_playing == true) {
		_scumm->_mixer->stop(_mp3_index);
		_mp3_cd_playing = false;
		return 0;
	}
	return -1;
}

int Sound::pollMP3CD() {
	if (_mp3_cd_playing == true)
		return 1;
	return 0;
}

int Sound::updateMP3CD() {
	if (_mp3_cd_playing == false)
		return -1;

	if (_scumm->_mixer->_channels[_mp3_index] == NULL) {
		warning("Error in MP3 decoding");
		return -1;
	}

	if (_scumm->_mixer->_channels[_mp3_index]->soundFinished())
		stopMP3CD();
	return 0;
}
#endif
