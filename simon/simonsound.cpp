#include "simon/simonsound.h"
#include "common/file.h"
#include "common/engine.h"

enum {
	GAME_SIMON2 = 1,
	GAME_WIN = 2,
	GAME_TALKIE = 4,
	GAME_DEMO = 8,

	GAME_SIMON1DOS = 0,
	GAME_SIMON2DOS = GAME_SIMON2,
	GAME_SIMON1TALKIE = GAME_TALKIE,
	GAME_SIMON2TALKIE = GAME_SIMON2 + GAME_TALKIE,
	GAME_SIMON1WIN = GAME_WIN + GAME_TALKIE,
	GAME_SIMON2WIN = GAME_SIMON2 + GAME_WIN + GAME_TALKIE,
	GAME_SIMON1DEMO = GAME_DEMO,
};

SimonSound::SimonSound(const byte game, const GameSpecificSettings *gss, const char *gameDataPath, SoundMixer *mixer)
{
	_game = game;
	_mixer = mixer;
	
	_effects_paused = false;
	_ambient_paused = false;

	_voice_handle = 0;
	_effects_handle = 0;
	_ambient_handle = 0;

	_ambient_playing = 0;

	File *file = new File();
	File *file2 = new File();
	const char *s;

#ifdef USE_MAD
	file->open(gss->mp3_filename, gameDataPath);
	if (file->isOpen() == false) {
#endif
		if (_game & GAME_WIN) {
			s = gss->wav_filename;
			file->open(s, gameDataPath);
			if (file->isOpen() == false) {
				warning("Cannot open voice file %s", s);
			} else	{
				_voice = new WavSound(_mixer, file, gss->NUM_VOICE_RESOURCES);
			}
		} else if (_game & GAME_TALKIE) {
			s = gss->voc_filename;
			file->open(s, gameDataPath);
			if (file->isOpen() == false) {
				warning("Cannot open voice file %s", s);
			} else {
				_voice = new VocSound(_mixer, file, gss->NUM_VOICE_RESOURCES);
			}
		}
#ifdef USE_MAD
	} else {
		_voice = new MP3Sound(_mixer, file, gss->NUM_VOICE_RESOURCES);
	}
#endif

	if (_game == GAME_SIMON1TALKIE) {
#ifdef USE_MAD
		file2->open(gss->mp3_effects_filename, gameDataPath);
		if (file2->isOpen() == false) {
#endif
			s = gss->voc_effects_filename;
			file2->open(s, gameDataPath);
			if (file2->isOpen() == false) {
				warning("Cannot open effects file %s", s);
			} else {
				_effects = new VocSound(_mixer, file2, gss->NUM_VOICE_RESOURCES);
			}
#ifdef USE_MAD
		} else {
			_effects = new MP3Sound(_mixer, file2, gss->NUM_VOICE_RESOURCES);
		}
#endif
	}
}

void SimonSound::readSfxFile(const char *filename, const char *gameDataPath)
{
	stopAll();

	File *file = new File();
	file->open(filename, gameDataPath);

	if (file->isOpen() == false) {
		warning("readSfxFile: Cannot load sfx file %s", filename);
		return;
	}

	_effects = new WavSound(_mixer, file);
}

void SimonSound::loadSfxTable(File *gameFile, uint32 offs, int set) 
{
	if (!set)
		return;

	stopAll();

	if (_game & GAME_WIN)
		_effects = new WavSound(_mixer, gameFile, offs, set);
	else
		_effects = new VocSound(_mixer, gameFile, offs, set);
}

void SimonSound::playVoice(uint sound)
{
	if (!_voice)
		return;
	
	_voice->playSound(sound, &_voice_handle, 0);
}

void SimonSound::playEffects(uint sound)
{
	if (!_effects)
		return;
	
	if (_effects_paused)
		return;

	_effects->playSound(sound, &_effects_handle, 0);
}

void SimonSound::playAmbient(uint sound)
{
	if (!_effects)
		return;

	if (sound == _ambient_playing)
		return;

	_ambient_playing = sound;

	if (_ambient_paused)
		return;

	if (_ambient_handle)
		_mixer->stop(_ambient_index);

	_ambient_index = _effects->playSound(sound, &_ambient_handle, SoundMixer::FLAG_LOOP);
}

void SimonSound::stopAll()
{
	_mixer->stopAll();
	_ambient_playing = 0;
}

void SimonSound::effectsPause(bool b)
{
	_effects_paused = b;
}

void SimonSound::ambientPause(bool b)
{
	_ambient_paused = b;

	if (_ambient_paused && _ambient_playing) {
		_mixer->stop(_ambient_index);
	} else if (_ambient_playing) {
		uint tmp = _ambient_playing;
		_ambient_playing = 0;
		playAmbient(tmp);
	}
}

/******************************************************************************/

SimonSound::WavSound::WavSound(SoundMixer *mixer, File *file, uint resources)
{
	_mixer = mixer;
	_file = file;
	loadOffsets(resources);
}

SimonSound::WavSound::WavSound(SoundMixer *mixer, File *file) /* only used in simon1win */
{
	_mixer = mixer;
	_file = file;
	loadOffsets();
}

SimonSound::WavSound::WavSound(SoundMixer *mixer, File *file, uint32 offs, int set) /* only used in simon2 */
{
	_mixer = mixer;
	_file = file;
	loadOffsets(offs, set);
}

SimonSound::VocSound::VocSound(SoundMixer *mixer, File *file, uint resources)
{
	_mixer = mixer;
	_file = file;
	loadOffsets(resources);
}

SimonSound::VocSound::VocSound(SoundMixer *mixer, File *file, uint32 offs, int set) /* only used in simon2 */
{
	_mixer = mixer;
	_file = file;
	loadOffsets(offs, set);
}

#ifdef USE_MAD
SimonSound::MP3Sound::MP3Sound(SoundMixer *mixer, File *file, uint resources)
{
	_mixer = mixer;
	_file = file;

	_offsets = (uint32 *)malloc((resources + 1) * sizeof(uint32));

	if (_offsets == NULL)
		error("Out of memory for voice offsets");
	
	if (_file->read(_offsets, resources * sizeof(uint32)) != resources * sizeof(uint32))
		error("Cannot read offsets");

#ifdef SCUMM_BIG_ENDIAN
	for (uint i = 0; i < resources; i++)
		_offsets[i] = FROM_LE_32(_offsets[i]);
#endif
	_file->seek(0, SEEK_END);
	_offsets[resources] = _file->pos();
}
#endif

/******************************************************************************/

void SimonSound::Sound::loadOffsets(uint resources)
{
	_offsets = (uint32 *)malloc(resources * sizeof(uint32));

	if (_offsets == NULL)
		error("Out of memory for offsets");

	if (_file->read(_offsets, resources * sizeof(uint32)) != resources * sizeof(uint32))
		error("Cannot read offsets");

#ifdef SCUMM_BIG_ENDIAN
	for (uint i = 0; i < resources; i++)
		_offsets[i] = FROM_LE_32(_offsets[i]);
#endif
}

void SimonSound::Sound::loadOffsets()
{
	uint num = 0;

	uint32 size;

	_file->seek(4, SEEK_SET);
	size = _file->readUint32LE();

	num = size / sizeof(uint32);

	_offsets = (uint32 *)malloc(size);

	_file->seek(0, SEEK_SET);
	_file->read(_offsets, size);

#if defined(SCUMM_BIG_ENDIAN)
	for (uint r = 0; r < num; r++)
		_offsets[r] = FROM_LE_32(_offsets[r]);
#endif
}

void SimonSound::Sound::loadOffsets(uint32 offs, int set)
{
	int num_per_set[] = {0, 188, 223, 217, 209, 179, 187, 189, 116, 174, 203,
			173, 176, 38, 205, 134, 213, 212, 167, 141};

	uint num;
	uint i;

	num = num_per_set[set];

	if (num == 0)
		return;

	_offsets = (uint32 *)malloc(num * sizeof(uint32));

	_file->seek(offs, SEEK_SET);
	_file->read(_offsets, num * sizeof(uint32));

	for (i = 0; i < num; i++) {
#if defined(SCUMM_BIG_ENDIAN)
		_offsets[i] = FROM_LE_32(_offsets[i]);
#endif
		_offsets[i] += offs;
	}
}

/******************************************************************************/

#if !defined(__GNUC__)
#pragma START_PACK_STRUCTS
#endif

struct WaveHeader {
	uint32 riff;
	uint32 unk;
	uint32 wave;
	uint32 fmt;

	uint32 size;

	uint16 format_tag;
	uint16 channels;
	uint32 samples_per_sec;
	uint32 avg_bytes;

	uint16 block_align;
	uint16 bits_per_sample;
} GCC_PACK;

struct VocHeader {
	uint8 desc[20];
	uint16 datablock_offset;
	uint16 version;
	uint16 id;
} GCC_PACK;

struct VocBlockHeader {
	uint8 blocktype;
	uint8 size[3];
	uint8 sr;
	uint8 pack;
} GCC_PACK;

#if !defined(__GNUC__)
#pragma END_PACK_STRUCTS
#endif
	
#ifdef USE_MAD
int SimonSound::MP3Sound::playSound(uint sound, PlayingSoundHandle *handle, byte flags)
{
	if (_offsets == NULL)
		return 0;

	flags |= SoundMixer::FLAG_AUTOFREE;

	_file->seek(_offsets[sound], SEEK_SET);

	uint32 size = _offsets[sound+1] - _offsets[sound];

	byte *buffer = (byte *)malloc(size);
	_file->read(buffer, size);

	return _mixer->playMP3(handle, buffer, size, flags);
}
#endif

int SimonSound::VocSound::playSound(uint sound, PlayingSoundHandle *handle, byte flags)
{
	if (_offsets == NULL)
		return 0;

	VocHeader voc_hdr;
	VocBlockHeader voc_block_hdr;
	uint32 size;

	flags |= SoundMixer::FLAG_UNSIGNED|SoundMixer::FLAG_AUTOFREE;

	_file->seek(_offsets[sound], SEEK_SET);

	if (_file->read(&voc_hdr, sizeof(voc_hdr)) != sizeof(voc_hdr) ||
			strncmp((char *)voc_hdr.desc, "Creative Voice File\x1A", 10) != 0) {
		error("playVoc(%d): cannot read voc header", sound);
	}

	_file->read(&voc_block_hdr, sizeof(voc_block_hdr));

	size = voc_block_hdr.size[0] + (voc_block_hdr.size[1] << 8) + (voc_block_hdr.size[2] << 16) - 2;
	uint32 samples_per_sec;

	/* workaround for voc weakness */
	if (voc_block_hdr.sr == 0xa6) {
		samples_per_sec = 11025;
	} else if (voc_block_hdr.sr == 0xd2) {
		samples_per_sec = 22050;
	} else {
		samples_per_sec = 1000000L / (256L - (long)voc_block_hdr.sr);
		warning("inexact sample rate used: %i", samples_per_sec);
	}

	byte *buffer = (byte *)malloc(size);
	_file->read(buffer, size);

	return _mixer->playRaw(handle, buffer, size, samples_per_sec, flags);
}

int SimonSound::WavSound::playSound(uint sound, PlayingSoundHandle *handle, byte flags)
{
	if (_offsets == NULL)
		return 0;

	WaveHeader wave_hdr;
	uint32 data[2];

	flags |= SoundMixer::FLAG_UNSIGNED|SoundMixer::FLAG_AUTOFREE;

	_file->seek(_offsets[sound], SEEK_SET);

	if (_file->read(&wave_hdr, sizeof(wave_hdr)) != sizeof(wave_hdr) ||
			wave_hdr.riff != MKID('RIFF') || wave_hdr.wave != MKID('WAVE')
			|| wave_hdr.fmt != MKID('fmt ') || READ_LE_UINT16(&wave_hdr.format_tag) != 1
			|| READ_LE_UINT16(&wave_hdr.channels) != 1
			|| READ_LE_UINT16(&wave_hdr.bits_per_sample) != 8) {
		error("playWav(%d): cannot read RIFF header", sound);
	}

	_file->seek(FROM_LE_32(wave_hdr.size) - sizeof(wave_hdr) + 20, SEEK_CUR);

	data[0] = _file->readUint32LE();
	data[1] = _file->readUint32LE();
	if (//fread(data, sizeof(data), 1, sound_file) != 1 ||
			 data[0] != 'atad') {
		error("playWav(%d): cannot read data header", sound);
	}

	byte *buffer = (byte *)malloc(data[1]);
	_file->read(buffer, data[1]);

	return _mixer->playRaw(handle, buffer, data[1], FROM_LE_32(wave_hdr.samples_per_sec), flags);
}

