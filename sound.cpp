/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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
#include "scumm.h"
#include "sound.h"

void Scumm::addSoundToQueue(int sound) {
	if(!(_features & GF_AFTER_V7)) {
		_vars[VAR_LAST_SOUND] = sound;
		ensureResourceLoaded(rtSound, sound);
		addSoundToQueue2(sound);
	}
	
	if(_features & GF_AUDIOTRACKS)
		warning("Requesting audio track: %d",sound);
}

void Scumm::addSoundToQueue2(int sound) {
	if (_soundQue2Pos < 10) {
		_soundQue2[_soundQue2Pos++] = sound;
	}
}

void Scumm::processSoundQues() {
	byte d;
	int i,j;
	int num;
	int16 data[16];
	SoundEngine *se;

	processSfxQueues();

	while (_soundQue2Pos){
		d=_soundQue2[--_soundQue2Pos];
		if (d)
			playSound(d);
	}

	for (i=0; i<_soundQuePos; ) {
		num = _soundQue[i++];
		if (i + num > _soundQuePos) {
			warning("processSoundQues: invalid num value");
			break;
		}
		for (j=0; j<16; j++)
			data[j] = 0;
		if (num>0) {
			for (j=0; j<num; j++)
				data[j] = _soundQue[i+j];
			i += num;

			se = (SoundEngine*)_soundEngine;
#if 0
			debug(1,"processSoundQues(%d,%d,%d,%d,%d,%d,%d,%d,%d)", 
				data[0]>>8,
				data[0]&0xFF,
				data[1],
				data[2],
				data[3],
				data[4],
				data[5],
				data[6],
				data[7]
				);
#endif
			if(!(_features & GF_AFTER_V7)) {
				if (se) 
					_vars[VAR_SOUNDRESULT] = (short)se->do_command(data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);
			}

		}
	}
	_soundQuePos = 0;
}

void Scumm::playSound(int sound) {
        SoundEngine *se = (SoundEngine*)_soundEngine;
        if (_features & GF_OLD256) return; /* FIXME */

	if (se) {
		getResourceAddress(rtSound, sound);
		se->start_sound(sound);
	}
}

void Scumm::processSfxQueues() {
	Actor *a;
	int act;
	bool b,finished;

	if (_talk_sound_mode != 0) {
		startTalkSound(_talk_sound_a, _talk_sound_b, _talk_sound_mode);
		_talk_sound_mode = 0;
	}
	
	if (_sfxMode==2) {
		act = _vars[VAR_TALK_ACTOR];
		finished = isSfxFinished();

		if (act!=0 && (uint)act<0x80 && !string[0].no_talk_anim) {
			a = derefActorSafe(act, "processSfxQueues");
			if (a->room==_currentRoom && (finished || !_endOfMouthSync)) {
				b = true;
				if (!finished)
					b = isMouthSyncOff(_curSoundPos);
				if (_mouthSyncMode != b) {
					_mouthSyncMode = b;
					startAnimActor(a, b ? a->talkFrame2 : a->talkFrame1);
				}
			}
		}
		if (finished && _talkDelay==0) {
			stopTalk();
			_sfxMode = 0;
		}
	} else if (_sfxMode==1) {
		if (isSfxFinished()) {
			_sfxMode = 0;
		}
	}
}

#ifdef COMPRESSED_SOUND_FILE
static int compar(const void *a, const void *b) {
  return ((OffsetTable *) a)->org_offset - ((OffsetTable *) b)->org_offset;
}
#endif

void Scumm::startTalkSound(uint32 offset, uint32 b, int mode) {
	int num = 0, i;
	byte file_byte,file_byte_2;	
	int size;

	if (!_sfxFile) {
		warning("startTalkSound: SFX file is not open");
		return;
	}

	if (b>8) {
		num = (b-8)>>1;
	}

#ifdef COMPRESSED_SOUND_FILE
	if (offset_table != NULL) {
	  OffsetTable *result, key;
	  
	  key.org_offset = offset;
	  result = (OffsetTable *) bsearch(&key, offset_table, num_sound_effects, sizeof(OffsetTable), compar);
	  if (result == NULL) {
	    warning("startTalkSound: did not find sound at offset %d !", offset);
	    return;
	  }
	  if (2 * num != result->num_tags) {
	    warning("startTalkSound: number of tags do not match (%d - %d) !", b, result->num_tags);
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

	fileSeek((FILE*)_sfxFile, offset, SEEK_SET);
	i = 0;
	while (num > 0) {
	  fileRead((FILE*)_sfxFile, &file_byte, sizeof(file_byte));
	  fileRead((FILE*)_sfxFile, &file_byte_2, sizeof(file_byte_2));
	  _mouthSyncTimes[i++] = file_byte | (file_byte_2<<8);
	  num--;
	}
	_mouthSyncTimes[i] = 0xFFFF;
	_sfxMode = mode;
	_curSoundPos = 0;
	_mouthSyncMode = true;

	startSfxSound(_sfxFile, size);
}

void Scumm::stopTalkSound() {
	if (_sfxMode==2) {
		stopSfxSound();
		_sfxMode = 0;
	}
}

bool Scumm::isMouthSyncOff(uint pos) {
	uint j;
	bool val = true;
	uint16 *ms = _mouthSyncTimes;

	_endOfMouthSync = false;
	do {
		val ^= 1;
		j = *ms++;
		if (j==0xFFFF) {
			_endOfMouthSync = true;
			break;
		}
	} while (pos > j);
	return val;
}


int Scumm::isSoundRunning(int sound) {
	SoundEngine *se;
	int i;

	i = _soundQue2Pos;
	while (i--) {
		if (_soundQue2[i] == sound)
			return 1;
	}

	if (isSoundInQueue(sound))
		return 1;

	if (!isResourceLoaded(rtSound, sound))
		return 0;
	
	se = (SoundEngine*)_soundEngine;
	if (!se)
		return 0;
	return se->get_sound_status(sound);
}

bool Scumm::isSoundInQueue(int sound) {
	int i = 0,j, num;
	int16 table[16];

	while (i < _soundQuePos) {
		num = _soundQue[i++];

		memset(table, 0, sizeof(table));

		if (num > 0) {
			for (j=0; j<num; j++)
				table[j] = _soundQue[i+j];
			i += num;
			if (table[0] == 0x10F && table[1]==8 && table[2] == sound)
				return 1;
		}
	}
	return 0;
}

void Scumm::stopSound(int a) {
	SoundEngine *se;
	int i;

	se = (SoundEngine*)_soundEngine;
	if (se)
		se->stop_sound(a);

	for (i=0; i<10; i++)
		if (_soundQue2[i] == (byte)a)
			_soundQue2[i] = 0;
}

void Scumm::stopAllSounds() {
	SoundEngine *se = (SoundEngine*)_soundEngine;
	if (se) {
		se->stop_all_sounds();
		se->clear_queue();
	}
	clearSoundQue();
	stopSfxSound();
}

void Scumm::clearSoundQue() {
	_soundQue2Pos = 0;
	memset(_soundQue2, 0, sizeof(_soundQue2));
}

void Scumm::soundKludge(int16 *list) {
	int16 *ptr;
	int i;

	if (list[0]==-1) {
		processSoundQues();
		return;
	}
	_soundQue[_soundQuePos++] = 8;

	ptr = _soundQue + _soundQuePos;
	_soundQuePos += 8;

	for (i=0; i<8; i++)
		*ptr++ = list[i];
	if (_soundQuePos > 0x100)
		error("Sound que buffer overflow");
}

void Scumm::talkSound(uint32 a, uint32 b, int mode) {
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

void Scumm::setupSound() {
	SoundEngine *se = (SoundEngine*)_soundEngine;
	SOUND_DRIVER_TYPE *driver = se->driver();
	if (se) {
		se->setBase(res.address[rtSound]);
        if (se->get_music_volume() == 0) se->set_music_volume(60);
		se->set_master_volume(125);
		
		driver->midiSetDriver(se->midiGetDriver());
	}
	_sfxFile = openSfxFile();
}

void Scumm::pauseSounds(bool pause) {
	SoundEngine *se = (SoundEngine*)_soundEngine;
	if (se)
		se->pause(pause);
	_soundsPaused = pause;
}

enum {
	SOUND_HEADER_SIZE = 26,
	SOUND_HEADER_BIG_SIZE = 26+8,

};

void Scumm::startSfxSound(void *file, int file_size) {
	char ident[8];
	int block_type;
	byte work[8];
	uint size,i;
	int rate,comp;
	byte *data;

#ifdef COMPRESSED_SOUND_FILE
	if (file_size > 0) {
	  data = (byte *) calloc(file_size + MAD_BUFFER_GUARD, 1);

	  if (fread(data, file_size, 1, (FILE*) file) != 1) {
	    /* no need to free the memory since error will shut down */
	    error("startSfxSound: cannot read %d bytes", size);
	    return;
	  }
	  playSfxSound_MP3(data, file_size);
	  return;
	}
#endif
	if ( fread(ident, 8, 1, (FILE*)file) != 1)
		goto invalid;

	if (!memcmp(ident, "VTLK", 4)) {
		fseek((FILE*)file, SOUND_HEADER_BIG_SIZE - 8, SEEK_CUR);
	} else if (!memcmp(ident, "Creative", 8)) {
		fseek((FILE*)file, SOUND_HEADER_SIZE - 8, SEEK_CUR);
	} else {
invalid:;
		warning("startSfxSound: invalid header");
		return;
	}

	block_type = getc( (FILE*)file );
	if (block_type != 1) {
		warning("startSfxSound: Expecting block_type == 1, got %d", block_type);
		return;
	}

	fread(work, 3, 1, (FILE*)file);

	size = ( work[0] | ( work[1] << 8 ) | ( work[2] << 16 ) ) - 2;
	rate = getc( (FILE*)file );
	comp = getc( (FILE*)file );

	if (comp != 0) {
		warning("startSfxSound: Unsupported compression type %d", comp);
		return;
	}
	
	data = (byte*) malloc(size);
	if (data==NULL) {
		error("startSfxSound: out of memory");
		return;
	}
	
	if (fread(data, size, 1, (FILE*)file) != 1) {
		/* no need to free the memory since error will shut down */
		error("startSfxSound: cannot read %d bytes", size);
		return;
	}
	for(i=0;i<size; i++) {
		// Fixme: From WinCE port
		if (_sound_volume_sfx != 100)
 			data[i] = _sound_volume_sfx * data[i] / 100;

		data[i] ^= 0x80;
	}
	playSfxSound(data, size, 1000000 / (256 - rate) );
}


#ifdef COMPRESSED_SOUND_FILE
static int get_int(FILE *f) {
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

void *Scumm::openSfxFile() {
	char buf[50];
	FILE *file;

	/* Try opening the file <_exe_name>.sou first, eg tentacle.sou.
	 * That way, you can keep .sou files for multiple games in the
	 * same directory */
#ifdef COMPRESSED_SOUND_FILE
	offset_table = NULL;

	sprintf(buf, "%s%s.so3", _gameDataPath, _exe_name);
	file = fopen(buf, "rb");
	if (!file) {
	  sprintf(buf, "%smonster.so3", _gameDataPath);
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
	  OffsetTable *cur;
	  
	  compressed_offset = get_int(file);
	  offset_table = (OffsetTable *) malloc(compressed_offset);
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
	sprintf(buf, "%s%s.sou", _gameDataPath, _exe_name);
	file = fopen(buf, "rb");
	if (!file) {
	  sprintf(buf, "%smonster.sou", _gameDataPath);
	  file = fopen(buf, "rb");
	}
	return file;
}

MixerChannel *Scumm::allocateMixer() {
	int i;
	MixerChannel *mc = _mixer_channel;
	for(i=0; i<NUM_MIXER; i++,mc++) {
		if (!mc->_sfx_sound)
			return mc;
	}
	return NULL;
}

void Scumm::stopSfxSound() {
	MixerChannel *mc = _mixer_channel;
	int i;
	for(i=0; i<NUM_MIXER; i++,mc++) {
		if (mc->_sfx_sound)
			mc->clear();
	}
}


bool Scumm::isSfxFinished() {
	int i;
	for(i=0; i<NUM_MIXER; i++)
		if (_mixer_channel[i]._sfx_sound)
			return false;
	return true;
}

#ifdef COMPRESSED_SOUND_FILE
void Scumm::playSfxSound_MP3(void *sound, uint32 size) {
  MixerChannel *mc = allocateMixer();

  if (!mc) {
    warning("No mixer channel available");
    return;
  }

  mc->type = MIXER_MP3;
  mc->_sfx_sound = sound;

  mad_stream_init(&mc->sound_data.mp3.stream);
  mad_frame_init(&mc->sound_data.mp3.frame);
  mad_synth_init(&mc->sound_data.mp3.synth);
  mc->sound_data.mp3.position = 0;
  mc->sound_data.mp3.pos_in_frame = 0xFFFFFFFF;
  mc->sound_data.mp3.size = size;
  /* This variable is the number of samples to cut at the start of the MP3
     file. This is needed to have lip-sync as the MP3 file have some miliseconds
     of blank at the start (as, I suppose, the MP3 compression algorithm need to
     have some silence at the start to really be efficient and to not distort
     too much the start of the sample).

     This value was found by experimenting out. If you recompress differently your
     .SO3 file, you may have to change this value.

     When using Lame, it seems that the sound starts to have some volume about 50 ms
     from the start of the sound => we skip about 1024 samples.
  */
  mc->sound_data.mp3.silence_cut = 1024; 
}
#endif

void Scumm::playSfxSound(void *sound, uint32 size, uint rate) {
	MixerChannel *mc = allocateMixer();

	if (!mc) {
		warning("No mixer channel available");
		return;
	}

	mc->type = MIXER_STANDARD;
	mc->_sfx_sound = sound;
	mc->sound_data.standard._sfx_pos = 0;	
	mc->sound_data.standard._sfx_fp_pos = 0;

#ifdef _WIN32_WCE
	mc->sound_data.standard._sfx_fp_speed = (1<<16) * rate / 11025;
#else
 	mc->sound_data.standard._sfx_fp_speed = (1<<16) * rate /22050;
#endif
	while (size&0xFFFF0000) size>>=1, rate>>=1;

	
#ifdef _WIN32_WCE
 	mc->sound_data.standard._sfx_size = size * 11025 / rate;
#else 
	mc->sound_data.standard._sfx_size = size * 22050 / rate;
#endif
}

#ifdef COMPRESSED_SOUND_FILE
static inline int scale_sample(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize and scale to not saturate when mixing a lot of channels */
  return sample >> (MAD_F_FRACBITS + 2 - 16);
}
#endif

void MixerChannel::mix(int16 *data, uint32 len) {
	if (!_sfx_sound)
	  return;

#ifdef COMPRESSED_SOUND_FILE
	if (type == MIXER_STANDARD) {
#endif
	  int8 *s;	
	  uint32 fp_pos, fp_speed;
	  
	  if (len > sound_data.standard._sfx_size)
	    len = sound_data.standard._sfx_size;
	  sound_data.standard._sfx_size -= len;
	  
	  s = (int8*)_sfx_sound + sound_data.standard._sfx_pos;
	  fp_pos = sound_data.standard._sfx_fp_pos;
	  fp_speed = sound_data.standard._sfx_fp_speed;
	  
	  do {
	    fp_pos += fp_speed;
	    *data++ += (*s<<6);
	    s += fp_pos >> 16;
	    fp_pos &= 0x0000FFFF;
	  } while (--len);
	  
	  sound_data.standard._sfx_pos = s - (int8*)_sfx_sound;
	  sound_data.standard._sfx_fp_speed = fp_speed;
	  sound_data.standard._sfx_fp_pos = fp_pos;
	  
	  if (!sound_data.standard._sfx_size)
	    clear();
#ifdef COMPRESSED_SOUND_FILE
	} else {
	  mad_fixed_t const *ch;
	  while (1) {
	    ch = sound_data.mp3.synth.pcm.samples[0] + sound_data.mp3.pos_in_frame;
	    while ((sound_data.mp3.pos_in_frame < sound_data.mp3.synth.pcm.length) && 
		   (len > 0)) {
	      if (sound_data.mp3.silence_cut > 0) {
		sound_data.mp3.silence_cut--;
	      } else {
		*data++ += scale_sample(*ch++);
		len--;
	      }
	      sound_data.mp3.pos_in_frame++;
	    }
	    if (len == 0) return;

	    if (sound_data.mp3.position >= sound_data.mp3.size) {
	      clear();
	      return;
	    }

	    mad_stream_buffer(&sound_data.mp3.stream, 
			      ((unsigned char *) _sfx_sound) + sound_data.mp3.position, 
			      sound_data.mp3.size + MAD_BUFFER_GUARD - sound_data.mp3.position);
	    
	    if (mad_frame_decode(&sound_data.mp3.frame, &sound_data.mp3.stream) == -1) {
	      /* End of audio... */
	      if (sound_data.mp3.stream.error == MAD_ERROR_BUFLEN) {
		clear();
		return;
	      } else if (!MAD_RECOVERABLE(sound_data.mp3.stream.error)) {
		error("MAD frame decode error !");
	      }
	    }
	    mad_synth_frame(&sound_data.mp3.synth, &sound_data.mp3.frame);
	    sound_data.mp3.pos_in_frame = 0;
	    sound_data.mp3.position = (unsigned char *) sound_data.mp3.stream.next_frame - (unsigned char *) _sfx_sound;
	  }
	}
#endif
}

void MixerChannel::clear() {
	free(_sfx_sound);
	_sfx_sound = NULL;

#ifdef COMPRESSED_SOUND_FILE
	if (type == MIXER_MP3) {
	  mad_synth_finish(&sound_data.mp3.synth);
	  mad_frame_finish(&sound_data.mp3.frame);
	  mad_stream_finish(&sound_data.mp3.stream);
	}
#endif
}

void Scumm::mixWaves(int16 *sounds, int len) {
	int i;

	memset(sounds, 0,len * sizeof(int16));

	if (_soundsPaused)
		return;
	
	SoundEngine *se = (SoundEngine*)_soundEngine;
	if (se) {
		se->driver()->generate_samples(sounds, len);
	}
	
	for(i=NUM_MIXER-1; i>=0;i--) {
		_mixer_channel[i].mix(sounds, len);
	}

	if (_soundsPaused2)
		memset(sounds, 0x0, len * sizeof(int16));
}

