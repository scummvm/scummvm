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

// FIXME: This code is taken from MADE and may need more work (e.g. setVolume).

// MIDI and digital music class

#include "sound/audiostream.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "sound/audiocd.h"
#include "sound/adpcm.h"
#include "common/config-manager.h"
#include "common/file.h"

#include "tinsel/config.h"
#include "tinsel/sound.h"
#include "tinsel/music.h"
#include "tinsel/handle.h"
#include "tinsel/sysvar.h"

enum {
	MUSIC_JUMP = -1,
	MUSIC_END = -2,

	BLMAGIC  = -3458,

	DIM_SPEED = 8
};

namespace Tinsel {

//--------------------------- Midi data -------------------------------------

// sound buffer structure used for MIDI data and samples
struct SOUND_BUFFER {
	uint8 *pDat;		// pointer to actual buffer
	uint32 size;		// size of the buffer
};

// get set when music driver is installed
//static MDI_DRIVER *mDriver;
//static HSEQUENCE mSeqHandle;

// MIDI buffer
static SOUND_BUFFER midiBuffer = { 0, 0 };

static SCNHANDLE	currentMidi = 0;
static bool		currentLoop = false;

const SCNHANDLE midiOffsetsGRAVersion[] = {
		 4,	  4534,	 14298,	 18828,	 23358,	 38888,	 54418,	 57172,	 59926,	 62450,
	 62952,	 67482,	 72258,	 74538,	 79314,	 87722,	103252,	115176,	127100,	127898,
	130256,	132614,	134972,	137330,	139688,	150196,	152554,	154912,	167422,	174762,
	182102,	194612,	198880,	199536,	206128,	206380,	216372,	226364,	235676,	244988,
	249098,	249606,	251160,	252714,	263116,	268706,	274296,	283562,	297986,	304566,
	312028,	313524,	319192,	324860,	331772,	336548,	336838,	339950,	343062,	346174,
	349286,	356246,	359358,	360434,	361510,	369966,	374366,	382822,	384202,	394946,
	396022,	396730,	399524,	401020,	403814,	418364,	419466,	420568,	425132,	433540,
	434384,	441504,	452132,	462760,	472804,	486772,	491302,	497722,	501260,	507680,
	509726,	521858,	524136,	525452,	533480,	538236,	549018,	559870,	564626,	565306,
	566734,	567616,	570144,	574102,	574900,	582518,	586350,	600736,	604734,	613812,
	616566,	619626,	623460,	627294,	631128,	634188,	648738,	663288,	667864,	681832,
	682048,	683014,	688908,	689124,	698888,	708652,	718416,	728180,	737944,	747708,
	752238,	765522,	766554,	772944,	774546,	776148,	776994,	781698,	786262,	789016,
	794630,	796422,	798998
};

const SCNHANDLE midiOffsetsSCNVersion[] = {
		 4,	  4504,	 11762,	 21532,	 26070,	 28754,	 33254,	 40512,	 56310,	 72108,
	 74864,	 77620,	 80152,	 80662,	 85200,	 89982,	 92268,	 97050,	105466,	121264,
	133194,	145124,	145928,	148294,	150660,	153026,	155392,	157758,	168272,	170638,
	173004,	185522,	192866,	200210,	212728,	217000,	217662,	224254,	224756,	234754,
	244752,	245256,	245950,	255256,	264562,	268678,	269192,	270752,	272312,	282712,
	288312,	293912,	303186,	317624,	324210,	331680,	333208,	338884,	344560,	351478,
	356262,	356552,	359670,	362788,	365906,	369024,	376014,	379132,	380214,	381296,
	389758,	394164,	402626,	404012,	414762,	415844,	416552,	419352,	420880,	423680,
	438236,	439338,	440440,	445010,	453426,	454276,	461398,	472032,	482666,	492716,
	506690,	511226,	517654,	521198,	527626,	529676,	541814,	546210,	547532,	555562,
	560316,	571104,	581962,	586716,	587402,	588836,	589718,	592246,	596212,	597016,
	604636,	608474,	622862,	626860,	635944,	638700,	641456,	645298,	649140,	652982,
	655738,	670294,	684850,	689432,	703628,	703850,	704816,	706350,	706572,	716342,
	726112,	735882,	745652,	755422,	765192,	774962,	784732,	794502,	804272,	814042,
	823812,	832996,	846286,	847324,	853714,	855324,	856934,	857786,	862496,	867066,
	869822,	875436,	877234,	879818
};

const int enhancedAudioGRAVersion[] = {
	 1,   2,   1,   1,   3,   3,   4,   4,   5,   6, //   1-10
	 1,   7,   8,   9,  10,   3,  11,  11,  12,  13, //  11-20
	13,  13,  13,  13,  14,  13,  13,  15,  16,  17, //  21-30
	15,  18,  19,  20, 338,  21,  21,  22,  22,  23, //  31-40
	24,  25,  26,  27,  28,  29,  30,  31,  32,  33, //  41-50
	34,  35,  35,  36,  37,  38,  39,  39,  39,  39, //  51-60
	40,  39,  41,  41,  42,  43,  42,  44,  45,  41, //  61-70
	46,  48,  47,  48,  49,  50,  51,  52,  53,  54, //  71-80
	55,  56,  57,  58,  59,  60,  61,  62,  63,  61, //  81-90
	64,  65,  66,  67,  68,  69,  70,  68,  71,  72, //  91-100
	73,  74,  75,  12,  76,  77,  78,  79,  80,   4, // 101-110
	81,  82,  83,  82,  81,  84,  85,  86,  87,  88, // 111-120
	89,  90,  88,   2,   2,   2,   2,   2,   2,  60, // 121-130
	91,  92,  93,  94,  94,  95,  96,  52,   4,  97, // 131-140
	98,  99,  99                                     // 141-143
};

const int enhancedAudioSCNVersion[] = {
	 301, 302,   2,    1,   1, 301, 302,   3,   3,   4,	//   1-10
	   4,   5,   6,    1,   7,   8,   9,  10,   8,  11,	//  11-20
	  11,  12,  13,   13,  13,  13,  13,  14,  13,  13,	//  21-30
	  15,  16,  17,   15,  18,  19,  20, 338,  21,  21,	//  31-40
	 341, 342,  22,   22,  23,  24,  25,  26,  27,  28,	//  41-50
	  29,  30,  31,   32,  33,  34,  35,  35,  36,  37,	//  51-60
	  38,  39,  39,   39,  39,  40,  39,  41,  41,  42,	//  61-70
	  43,  42,  44,   45,  41,  46,  48,  47,  48,  49,	//  71-80
	  50,  51,  52,   53,  54,  55,  56,  57,  58,  59,	//  81-90
	  60,  61,  62,   63,  61,  64,  65,  66,  67,  68,	//  91-100
	  69,  70,  68,   71,  72,  73,  74,  75,  12,  76,	// 101-110
	  77,  78,  79,   80,   4,   4,  82,  83,  77,   4,	// 111-120
	  84,  85,  86, 3124,  88,  89,  90,  88,   2,   2,	// 121-130
	   2,   2,   2,    2,   2,   2,   2,   2,   2,   2,	// 131-140
	3141,  91,  92,   93,  94,  94,  95,  96,  52,   4,	// 141-150
	  97,  98,  99,   99                             	// 151-154
};

int GetTrackNumber(SCNHANDLE hMidi) {
	if (_vm->getFeatures() & GF_SCNFILES) {
		for (int i = 0; i < ARRAYSIZE(midiOffsetsSCNVersion); i++) {
			if (midiOffsetsSCNVersion[i] == hMidi)
				return i;
		}
	} else {
		for (int i = 0; i < ARRAYSIZE(midiOffsetsGRAVersion); i++) {
			if (midiOffsetsGRAVersion[i] == hMidi)
				return i;
		}
	}

	return -1;
}

SCNHANDLE GetTrackOffset(int trackNumber) {
	if (_vm->getFeatures() & GF_SCNFILES) {
		assert(trackNumber < ARRAYSIZE(midiOffsetsSCNVersion));
		return midiOffsetsSCNVersion[trackNumber];
	} else {
		assert(trackNumber < ARRAYSIZE(midiOffsetsGRAVersion));
		return midiOffsetsGRAVersion[trackNumber];
	}
}

/**
 * Plays the specified MIDI sequence through the sound driver.
 * @param dwFileOffset		File offset of MIDI sequence data
 * @param bLoop				Whether to loop the sequence
 */
bool PlayMidiSequence(uint32 dwFileOffset, bool bLoop) {
	currentMidi = dwFileOffset;
	currentLoop = bLoop;

	// Tinsel V1 PSX uses a different music format, so i
	// disable it here.
	// TODO: Maybe this should be moved to a better place...
	if (TinselV1PSX) return false;

	if (volMusic != 0) {
		SetMidiVolume(volMusic);
	}

	// the index and length of the last tune loaded
	static uint32 dwLastMidiIndex;
	//static uint32 dwLastSeqLen;

	uint32 dwSeqLen = 0;	// length of the sequence

	// Support for external music from the music enhancement project
	if (_vm->getFeatures() & GF_ENHANCED_AUDIO_SUPPORT) {
		int trackNumber = GetTrackNumber(dwFileOffset);
		int track = 0;
		if (trackNumber >= 0) {
			if (_vm->getFeatures() & GF_SCNFILES)
				track = enhancedAudioSCNVersion[trackNumber];
			else
				track = enhancedAudioGRAVersion[trackNumber];

			if (track > 0) {
				StopMidi();

				// StopMidi resets these fields, so set them again
				currentMidi = dwFileOffset;
				currentLoop = bLoop;

				// try to play track, but don't fall back to a true CD
				AudioCD.play(track, bLoop ? -1 : 1, 0, 0, true);

				// Check if an enhanced audio track is being played.
				// If it is, stop here and don't load a MIDI track
				if (AudioCD.isPlaying()) {
					return true;
				}
			}
		} else {
			warning("Unknown MIDI offset %d", dwFileOffset);
		}
	}

	if (dwFileOffset == 0)
		return true;

	if (dwFileOffset != dwLastMidiIndex) {
		Common::File midiStream;

		// open MIDI sequence file in binary mode
		if (!midiStream.open(MIDI_FILE))
			error(CANNOT_FIND_FILE, MIDI_FILE);

		// update index of last tune loaded
		dwLastMidiIndex = dwFileOffset;

		// move to correct position in the file
		midiStream.seek(dwFileOffset, SEEK_SET);

		// read the length of the sequence
		dwSeqLen = midiStream.readUint32LE();

		// make sure buffer is large enough for this sequence
		assert(dwSeqLen > 0 && dwSeqLen <= midiBuffer.size);

		// stop any currently playing tune
		_vm->_midiMusic->stop();

		// read the sequence
		if (midiStream.read(midiBuffer.pDat, dwSeqLen) != dwSeqLen)
			error(FILE_IS_CORRUPT, MIDI_FILE);

		midiStream.close();

		_vm->_midiMusic->playXMIDI(midiBuffer.pDat, dwSeqLen, bLoop);

		// Store the length
		//dwLastSeqLen = dwSeqLen;
	} else {
	 	// dwFileOffset == dwLastMidiIndex
		_vm->_midiMusic->stop();
		_vm->_midiMusic->playXMIDI(midiBuffer.pDat, dwSeqLen, bLoop);
	}

	return true;
}

/**
 * Returns TRUE if a Midi tune is currently playing.
 */
bool MidiPlaying(void) {
	if (_vm->getFeatures() & GF_ENHANCED_AUDIO_SUPPORT) {
		if (AudioCD.isPlaying())
			return true;
	}
	return _vm->_midiMusic->isPlaying();
}

/**
 * Stops any currently playing midi.
 */
bool StopMidi(void) {
	currentMidi = 0;
	currentLoop = false;

	if (_vm->getFeatures() & GF_ENHANCED_AUDIO_SUPPORT) {
		AudioCD.stop();
	}

	_vm->_midiMusic->stop();
	return true;
}


/**
 * Gets the volume of the MIDI music.
 */
int GetMidiVolume() {
	return volMusic;
}

static int priorVolMusic = 0;

/**
 * Sets the volume of the MIDI music.
 * @param vol			New volume - 0..MAXMIDIVOL
 */
void SetMidiVolume(int vol)	{
	assert(vol >= 0 && vol <= Audio::Mixer::kMaxChannelVolume);

	if (vol == 0 && priorVolMusic == 0)	{
		// Nothing to do
	} else if (vol == 0 && priorVolMusic != 0) {
		// Stop current midi sequence
		StopMidi();
		_vm->_midiMusic->setVolume(vol);
	} else if (vol != 0 && priorVolMusic == 0) {
		// Perhaps restart last midi sequence
		if (currentLoop) {
			PlayMidiSequence(currentMidi, true);
			_vm->_midiMusic->setVolume(vol);
		}
	} else if (vol != 0 && priorVolMusic != 0) {
		// Alter current volume
		_vm->_midiMusic->setVolume(vol);
	}

	priorVolMusic = vol;
}

/**
 * Opens and inits all MIDI sequence files.
 */
void OpenMidiFiles(void) {
	Common::File midiStream;

	// Demo version has no midi file
	// Also, Discworld PSX uses still unsupported psx SEQ format for music...
	if ((_vm->getFeatures() & GF_DEMO) || (TinselVersion == TINSEL_V2) || TinselV1PSX)
		return;

	if (midiBuffer.pDat)
		// already allocated
		return;

	// open MIDI sequence file in binary mode
	if (!midiStream.open(MIDI_FILE))
		error(CANNOT_FIND_FILE, MIDI_FILE);

	// gen length of the largest sequence
	midiBuffer.size = midiStream.readUint32LE();
	if (midiStream.ioFailed())
		error(FILE_IS_CORRUPT, MIDI_FILE);

	if (midiBuffer.size) {
		// allocate a buffer big enough for the largest MIDI sequence
		if ((midiBuffer.pDat = (uint8 *)malloc(midiBuffer.size)) != NULL) {
			// clear out the buffer
			memset(midiBuffer.pDat, 0, midiBuffer.size);
//			VMM_lock(midiBuffer.pDat, midiBuffer.size);
		} else {
			//mSeqHandle = NULL;
		}
	}

	midiStream.close();
}

void DeleteMidiBuffer() {
	free(midiBuffer.pDat);
	midiBuffer.pDat = NULL;
}

MidiMusicPlayer::MidiMusicPlayer(MidiDriver *driver) : _parser(0), _driver(driver), _looping(false), _isPlaying(false) {
	memset(_channel, 0, sizeof(_channel));
	_masterVolume = 0;
	this->open();
	_xmidiParser = MidiParser::createParser_XMIDI();
}

MidiMusicPlayer::~MidiMusicPlayer() {
	_driver->setTimerCallback(NULL, NULL);
	stop();
	this->close();
	_xmidiParser->setMidiDriver(NULL);
	delete _xmidiParser;
}

void MidiMusicPlayer::setVolume(int volume) {
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);

	if (_masterVolume == volume)
		return;

	_masterVolume = volume;

	Common::StackLock lock(_mutex);

	for (int i = 0; i < 16; ++i) {
		if (_channel[i]) {
			_channel[i]->volume(_channelVolume[i] * _masterVolume / 255);
		}
	}
}

int MidiMusicPlayer::open() {
	// Don't ever call open without first setting the output driver!
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;

	_driver->setTimerCallback(this, &onTimer);
	return 0;
}

void MidiMusicPlayer::close() {
	stop();
	if (_driver)
		_driver->close();
	_driver = 0;
}

void MidiMusicPlayer::send(uint32 b) {
	byte channel = (byte)(b & 0x0F);
	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by master volume
		byte volume = (byte)((b >> 16) & 0x7F);
		_channelVolume[channel] = volume;
		volume = volume * _masterVolume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xFFF0) == 0x007BB0) {
		//Only respond to All Notes Off if this channel
		//has currently been allocated
		if (_channel[b & 0x0F])
			return;
	}

	if (!_channel[channel])
		_channel[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();

	if (_channel[channel]) {
		_channel[channel]->send(b);

		if ((b & 0xFFF0) == 0x0079B0) {
			// We've just Reset All Controllers, so we need to
			// re-adjust the volume. Otherwise, volume is reset to
			// default whenever the music changes.
			_channel[channel]->send(0x000007B0 | (((_channelVolume[channel] * _masterVolume) / 255) << 16) | channel);
		}
	}
}

void MidiMusicPlayer::metaEvent(byte type, byte *data, uint16 length) {
	switch (type) {
	case 0x2F:	// End of Track
		if (_looping)
			_parser->jumpToTick(0);
		else
			stop();
		break;
	default:
		//warning("Unhandled meta event: %02x", type);
		break;
	}
}

void MidiMusicPlayer::onTimer(void *refCon) {
	MidiMusicPlayer *music = (MidiMusicPlayer *)refCon;
	Common::StackLock lock(music->_mutex);

	if (music->_isPlaying)
		music->_parser->onTimer();
}

void MidiMusicPlayer::playXMIDI(byte *midiData, uint32 size, bool loop) {
	if (_isPlaying)
		return;

	stop();

	// It seems like not all music (the main menu music, for instance) set
	// all the instruments explicitly. That means the music will sound
	// different, depending on which music played before it. This appears
	// to be a genuine glitch in the original. For consistency, reset all
	// instruments to the default one (piano).

	for (int i = 0; i < 16; i++) {
		_driver->send(0xC0 | i, 0, 0);
	}

	// Load XMID resource data

	if (_xmidiParser->loadMusic(midiData, size)) {
		MidiParser *parser = _xmidiParser;
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(getBaseTempo());
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

		_parser = parser;

		_looping = loop;
		_isPlaying = true;
	}
}

void MidiMusicPlayer::stop() {
	Common::StackLock lock(_mutex);

	_isPlaying = false;
	if (_parser) {
		_parser->unloadMusic();
		_parser = NULL;
	}
}

void MidiMusicPlayer::pause() {
	setVolume(-1);
	_isPlaying = false;
}

void MidiMusicPlayer::resume() {
	setVolume(GetMidiVolume());
	_isPlaying = true;
}

PCMMusicPlayer::PCMMusicPlayer() {
	_silenceSamples = 0;

	_curChunk = 0;
	_fileName = 0;
	_state = S_IDLE;
	_mState = S_IDLE;
	_scriptNum = -1;
	_scriptIndex = 0;
	_forcePlay = false;

	_volume = 255;
	_dimmed = false;
	_dimmedTinsel = false;
	_dimIteration = 0;

	_fadeOutVolume = 0;
	_fadeOutIteration = 0;

	_end = true;

	_vm->_mixer->playInputStream(Audio::Mixer::kMusicSoundType,
			&_handle, this, -1, _volume, 0, false, true);
}

PCMMusicPlayer::~PCMMusicPlayer() {
	_vm->_mixer->stopHandle(_handle);

	delete[] _fileName;
}

void PCMMusicPlayer::startPlay(int id) {
	if (!_fileName)
		return;

	debugC(DEBUG_DETAILED, kTinselDebugMusic, "Playing PCM music %s, index %d", _fileName, id);

	Common::StackLock slock(_mutex);

	stop();

	_scriptNum = id;
	_scriptIndex = 1;
	_state = S_NEW;

	play();
}

void PCMMusicPlayer::stopPlay() {
	Common::StackLock slock(_mutex);

	stop();
}

int PCMMusicPlayer::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock slock(_mutex);

	if (!_curChunk && ((_state == S_IDLE) || (_state == S_STOP)))
		return 0;

	int samplesLeft = numSamples;

	while (samplesLeft > 0) {
		if (_silenceSamples > 0) {
			int n = MIN(_silenceSamples, samplesLeft);

			memset(buffer, 0, n);

			buffer += n;
			_silenceSamples -= n;
			samplesLeft -= n;

		} else if (_curChunk &&
		          ((_state == S_MID) || (_state == S_NEXT) || (_state == S_NEW))) {
			int n = _curChunk->readBuffer(buffer, samplesLeft);

			buffer += n;
			samplesLeft -= n;

			if (_curChunk->endOfData()) {
				_state = S_END1;

				delete _curChunk;
				_curChunk = 0;
			}
		} else {

			if (!getNextChunk())
				break;
		}
	}

	return (numSamples - samplesLeft);
}

bool PCMMusicPlayer::isPlaying() const {
	return ((_state != S_IDLE) && (_state != S_STOP));
}

bool PCMMusicPlayer::isDimmed() const {
	return _dimmed;
}

void PCMMusicPlayer::getTunePlaying(void *voidPtr, int length) {
	Common::StackLock lock(_mutex);

	debugC(DEBUG_DETAILED, kTinselDebugMusic, "getTunePlaying");

	assert(length == (3 * sizeof(int32)));

	int32 *p = (int32 *) voidPtr;

	_mState = _state;

	p[0] = (int32) _mState;
	p[1] = _scriptNum;
	p[2] = _scriptIndex;
}

void PCMMusicPlayer::restoreThatTune(void *voidPtr) {
	Common::StackLock lock(_mutex);

	debugC(DEBUG_DETAILED, kTinselDebugMusic, "restoreThatTune");

	int32 *p = (int32 *) voidPtr;

	_mState = (State) p[0];
	_scriptNum = p[1];
	_scriptIndex = p[2];

	if (_mState != S_IDLE)
		_state = S_NEW;

	delete _curChunk;
	_curChunk = 0;

	_end = false;
}

void PCMMusicPlayer::setMusicSceneDetails(SCNHANDLE hScript,
		SCNHANDLE hSegment, const char *fileName) {

	Common::StackLock lock(_mutex);

	stop();

	debugC(DEBUG_INTERMEDIATE, kTinselDebugMusic, "Setting music scene details: %s", fileName);

	_hScript = hScript;
	_hSegment = hSegment;
	_fileName = new char[strlen(fileName) + 1];
	strcpy(_fileName, fileName);

	// Start scene with music not dimmed
	_dimmed = false;
	_dimmedTinsel = false;
	_dimIteration = 0;
	setVol(255);
}

void PCMMusicPlayer::setVolume(int volume) {
	assert((volume >= 0) && (volume <= 100));

	_dimmed = false;
	setVol((volume * 255) / 100);
}

void PCMMusicPlayer::setVol(uint8 volume) {
	_volume = volume;

	_vm->_mixer->setChannelVolume(_handle, _volume);
}

bool PCMMusicPlayer::getMusicTinselDimmed() const {
	return _dimmedTinsel;
}

void PCMMusicPlayer::dim(bool bTinselDim) {
	if (_dimmed || (_volume == 0) ||
			(_state == S_IDLE) || !_curChunk || (SysVar(SV_MUSICDIMFACTOR) == 0))
		return;

	_dimmed = true;
	if (bTinselDim)
		_dimmedTinsel = true;

	_dimmedVolume = _volume - (_volume / SysVar(SV_MUSICDIMFACTOR));

	// Iterate down, negative iteration
	if (!_dimIteration)
		_dimPosition = _volume;
	_dimIteration = (_dimmedVolume - _volume)/DIM_SPEED;

	debugC(DEBUG_DETAILED, kTinselDebugMusic, "Dimming music from %d to %d, steps %d", _dimPosition, _dimmedVolume, _dimIteration);

	// And SFX
	if (SysVar(SYS_SceneFxDimFactor))
		_vm->_sound->setSFXVolumes(255 - 255/SysVar(SYS_SceneFxDimFactor));
}

void PCMMusicPlayer::unDim(bool bTinselUnDim) {
	if (!_dimmed || (_dimmedTinsel && !bTinselUnDim))
		return; // not dimmed

	_dimmed = _dimmedTinsel = false;

	if ((_volume == 0) || (_state == S_IDLE) || !_curChunk)
		return;

	// Iterate up, positive iteration
	if (!_dimIteration)
		_dimPosition = _dimmedVolume;
	_dimIteration = (_volume - _dimmedVolume)/DIM_SPEED;

	debugC(DEBUG_DETAILED, kTinselDebugMusic, "UnDimming music from %d to %d, steps %d", _dimPosition, _volume, _dimIteration);

	// And SFX
	_vm->_sound->setSFXVolumes(255);
}

void PCMMusicPlayer::dimIteration() {
	if (_dimIteration != 0)
	{
		_dimPosition += _dimIteration;
		if (_dimPosition >= _volume)
		{
			_dimPosition = _volume;
			_dimIteration = 0;
		}
		else if (_dimPosition <= _dimmedVolume)
		{
			_dimPosition = _dimmedVolume;
			_dimIteration = 0;
		}

		_vm->_mixer->setChannelVolume(_handle, _dimPosition);
	}
}

void PCMMusicPlayer::startFadeOut(int ticks) {
	if ((_volume == 0) || (_state == S_IDLE) || !_curChunk)
		return;

	debugC(DEBUG_INTERMEDIATE, kTinselDebugMusic, "Fading out music...");

	if (_dimmed) {
		// Start from dimmed volume and go from there
		_dimmed = false;
		_fadeOutVolume = _volume - _volume/SysVar(SV_MUSICDIMFACTOR);
	} else
		_fadeOutVolume = _volume;

	assert(ticks != 0);
	_fadeOutIteration = _fadeOutVolume / ticks;

	fadeOutIteration();
}

void PCMMusicPlayer::fadeOutIteration() {
	if ((_volume == 0) || (_state == S_IDLE) || !_curChunk)
		return;

	_fadeOutVolume = CLIP<int>(_fadeOutVolume -= _fadeOutIteration, 0, 255);

	_vm->_mixer->setChannelVolume(_handle, _fadeOutVolume);
}

bool PCMMusicPlayer::getNextChunk() {
	MusicSegment *musicSegments;
	int32 *script, *scriptBuffer;
	int id;
	int snum;
	uint32 sampleOffset, sampleLength, sampleCLength;
	Common::File file;
	byte *buffer;
	Common::MemoryReadStream *sampleStream;

	switch (_state) {
	case S_NEW:
	case S_NEXT:
		_forcePlay = false;

		script = scriptBuffer = (int32 *)LockMem(_hScript);

		// Set parameters for this chunk of music
		id = _scriptNum;
		while (id--)
			script = scriptBuffer + READ_LE_UINT32(script);
		snum = FROM_LE_32(script[_scriptIndex++]);

		if (snum == MUSIC_JUMP || snum == MUSIC_END) {
			// Let usual code sort it out!
			_scriptIndex--;    // Undo increment
			_forcePlay = true; // Force a Play
			_state = S_END1;   // 'Goto' S_END1
			break;
		}

		musicSegments = (MusicSegment *) LockMem(_hSegment);

		assert(FROM_LE_32(musicSegments[snum].numChannels) == 1);
		assert(FROM_LE_32(musicSegments[snum].bitsPerSample) == 16);

		sampleOffset = FROM_LE_32(musicSegments[snum].sampleOffset);
		sampleLength = FROM_LE_32(musicSegments[snum].sampleLength);
		sampleCLength = (((sampleLength + 63) & ~63)*33)/64;

		if (!file.open(_fileName))
			error(CANNOT_FIND_FILE, _fileName);

		file.seek(sampleOffset);
		if (file.ioFailed() || (uint32)file.pos() != sampleOffset)
			error(FILE_IS_CORRUPT, _fileName);

		buffer = (byte *) malloc(sampleCLength);
		assert(buffer);

		// read all of the sample
		if (file.read(buffer, sampleCLength) != sampleCLength)
			error(FILE_IS_CORRUPT, _fileName);

		debugC(DEBUG_DETAILED, kTinselDebugMusic, "Creating ADPCM music chunk with size %d, "
				"offset %d (script %d.%d)", sampleCLength, sampleOffset,
				_scriptNum, _scriptIndex - 1);

		sampleStream = new Common::MemoryReadStream(buffer, sampleCLength, true);

		delete _curChunk;
		_curChunk = makeADPCMStream(sampleStream, true, sampleCLength,
				Audio::kADPCMTinsel8, 22050, 1, 32);

		_state = S_MID;
		return true;

	case S_END1:
		debugC(DEBUG_DETAILED, kTinselDebugMusic, "Music reached state S_END1 (script %d.%d)",
				_scriptNum, _scriptIndex);

		script = scriptBuffer = (int32 *) LockMem(_hScript);

		id = _scriptNum;
		while (id--)
			script = scriptBuffer + READ_LE_UINT32(script);
		snum = FROM_LE_32(script[_scriptIndex]);

		if (snum == MUSIC_END) {
			_state = S_END2;
		} else {
			if (snum == MUSIC_JUMP)
				_scriptIndex = FROM_LE_32(script[++_scriptIndex]);

			_state = _forcePlay ? S_NEW : S_NEXT;
			_forcePlay = false;
		}

		return true;

	case S_END2:
		debugC(DEBUG_DETAILED, kTinselDebugMusic, "Music reached state S_END2 (script %d.%d)",
				_scriptNum, _scriptIndex);

		_silenceSamples = 11025; // Half a second of silence
		return true;

	case S_END3:
		debugC(DEBUG_DETAILED, kTinselDebugMusic, "Music reached state S_END3 (script %d.%d)",
				_scriptNum, _scriptIndex);

		stop();
		_state = S_IDLE;
		return false;

	case S_IDLE:
		return false;

	default:
		break;
	}

	return true;
}

void PCMMusicPlayer::play() {
	if (_curChunk)
		return;
	if (_scriptNum == -1)
		return;

	_end = false;

	getNextChunk();
}

void PCMMusicPlayer::stop() {
	delete _curChunk;
	_curChunk = 0;
	_scriptNum = -1;
	_state = S_IDLE;
	_mState = S_IDLE;

	_end = true;
}

void CurrentMidiFacts(SCNHANDLE	*pMidi, bool *pLoop) {
	*pMidi = currentMidi;
	*pLoop = currentLoop;
}

void RestoreMidiFacts(SCNHANDLE	Midi, bool Loop) {
	StopMidi();

	currentMidi = Midi;
	currentLoop = Loop;

	if (volMusic != 0 && Loop) {
		PlayMidiSequence(currentMidi, true);
		SetMidiVolume(volMusic);
	}
}

#if 0
// Dumps all of the game's music in external XMIDI *.xmi files
void dumpMusic() {
	Common::File midiFile;
	Common::DumpFile outFile;
	char outName[20];
	midiFile.open(MIDI_FILE);
	int outFileSize = 0;
	char buffer[20000];

	int total = (_vm->getFeatures() & GF_SCNFILES) ?
				ARRAYSIZE(midiOffsetsSCNVersion) :
				ARRAYSIZE(midiOffsetsGRAVersion);

	for (int i = 0; i < total; i++) {
		sprintf(outName, "track%03d.xmi", i + 1);
		outFile.open(outName);

		if (_vm->getFeatures() & GF_SCNFILES) {
			if (i < total - 1)
				outFileSize = midiOffsetsSCNVersion[i + 1] - midiOffsetsSCNVersion[i] - 4;
			else
				outFileSize = midiFile.size() - midiOffsetsSCNVersion[i] - 4;

			midiFile.seek(midiOffsetsSCNVersion[i] + 4, SEEK_SET);
		} else {
			if (i < total - 1)
				outFileSize = midiOffsetsGRAVersion[i + 1] - midiOffsetsGRAVersion[i] - 4;
			else
				outFileSize = midiFile.size() - midiOffsetsGRAVersion[i] - 4;

			midiFile.seek(midiOffsetsGRAVersion[i] + 4, SEEK_SET);
		}

		assert(outFileSize < 20000);
		midiFile.read(buffer, outFileSize);
		outFile.write(buffer, outFileSize);

		outFile.close();
	}

	midiFile.close();
}
#endif

} // End of namespace Tinsel
