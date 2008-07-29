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
#include "common/config-manager.h"
#include "common/file.h"

#include "tinsel/config.h"
#include "tinsel/sound.h"
#include "tinsel/music.h"

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

// if non-zero this is the index position of the next MIDI sequence to play
static uint32 dwMidiIndex = 0;

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

// TODO: finish this (currently unmapped tracks are 0)
const int enhancedAudioSCNVersion[] = {
	 0,  0,  2,  0,  0,  0,  0,  3,  3,  4,
	 4,  0,  0,  0,  0,  0,  0, 10,  3, 11,
	11,  0, 13, 13, 13, 13, 13,  0, 13, 13,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0, 24,  0,  0, 27,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0, 55, 56, 56,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  4,  4, 83, 83, 83,  4,
	 0,  0,  0,  0,  0,  0,  0,  0,  2,  2,
	 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
	 0,  0,  0,  0,  0,  0,  0,  0, 52,  4,
	 0,  0,  0,  0	
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

	if (volMidi != 0) {
		SetMidiVolume(volMidi);
		// Support for compressed music from the music enhancement project
		AudioCD.stop();

		int trackNumber = GetTrackNumber(dwFileOffset);
		if (trackNumber >= 0) {
#if 0
			// TODO: GRA version
			int track = enhancedAudioSCNVersion[trackNumber];
			if (track > 0)
				AudioCD.play(track, -1, 0, 0);
#endif
		} else {
			warning("Unknown MIDI offset %d", dwFileOffset);
		}

		if (AudioCD.isPlaying())
			return true;
	}

	// set file offset for this sequence
	dwMidiIndex = dwFileOffset;

	// the index and length of the last tune loaded
	static uint32 dwLastMidiIndex;
	static uint32 dwLastSeqLen;

	uint32 dwSeqLen = 0;	// length of the sequence

	if (dwMidiIndex == 0)
		return true;

	if (dwMidiIndex != dwLastMidiIndex) {
		Common::File midiStream;

		// open MIDI sequence file in binary mode
		if (!midiStream.open(MIDI_FILE))
			error("Cannot find file %s", MIDI_FILE);

		// update index of last tune loaded
		dwLastMidiIndex = dwMidiIndex;

		// move to correct position in the file
		midiStream.seek(dwMidiIndex, SEEK_SET);

		// read the length of the sequence
		dwSeqLen = midiStream.readUint32LE();

		// make sure buffer is large enough for this sequence
		assert(dwSeqLen > 0 && dwSeqLen <= midiBuffer.size);

		// stop any currently playing tune
		_vm->_music->stop();

		// read the sequence
		if (midiStream.read(midiBuffer.pDat, dwSeqLen) != dwSeqLen)
			error("File %s is corrupt", MIDI_FILE);

		midiStream.close();

		_vm->_music->playXMIDI(midiBuffer.pDat, dwSeqLen, bLoop);

		// Store the length
		dwLastSeqLen = dwSeqLen;
	} else {
	  	// dwMidiIndex == dwLastMidiIndex
		_vm->_music->stop();
		_vm->_music->playXMIDI(midiBuffer.pDat, dwSeqLen, bLoop);
	}

	// allow another sequence to play
	dwMidiIndex = 0;

	return true;
}

/**
 * Returns TRUE if a Midi tune is currently playing.
 */
bool MidiPlaying(void) {
	if (AudioCD.isPlaying()) return true;
	return _vm->_music->isPlaying();
}

/**
 * Stops any currently playing midi.
 */
bool StopMidi(void) {
	currentMidi = 0;
	currentLoop = false;

	AudioCD.stop();
	_vm->_music->stop();
	return true;
}


/**
 * Gets the volume of the MIDI music.
 */
int GetMidiVolume() {
	return volMidi;
}

/**
 * Sets the volume of the MIDI music.
 * @param vol			New volume - 0..MAXMIDIVOL
 */
void SetMidiVolume(int vol)	{
	assert(vol >= 0 && vol <= MAXMIDIVOL);

	if (vol == 0 && volMidi == 0) 	{
		// Nothing to do
	} else if (vol == 0 && volMidi != 0) {
		// Stop current midi sequence
		AudioCD.stop();
		StopMidi();
	} else if (vol != 0 && volMidi == 0) {
		// Perhaps restart last midi sequence
		if (currentLoop) {
			PlayMidiSequence(currentMidi, true);
			_vm->_music->setVolume(vol);
		}
	} else if (vol != 0 && volMidi != 0) {
		// Alter current volume
		_vm->_music->setVolume(vol);
	}

	volMidi = vol;
}

/**
 * Opens and inits all MIDI sequence files.
 */
void OpenMidiFiles(void) {
	Common::File midiStream;

	// Demo version has no midi file
	if (_vm->getFeatures() & GF_DEMO)
		return;

	if (midiBuffer.pDat)
		// already allocated
		return;

	// open MIDI sequence file in binary mode
	if (!midiStream.open(MIDI_FILE))
		error("Cannot find file %s", MIDI_FILE);

	// gen length of the largest sequence
	midiBuffer.size = midiStream.readUint32LE();
	if (midiStream.ioFailed())
		error("File %s is corrupt", MIDI_FILE);

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

MusicPlayer::MusicPlayer(MidiDriver *driver) : _parser(0), _driver(driver), _looping(false), _isPlaying(false) {
	memset(_channel, 0, sizeof(_channel));
	_masterVolume = 0;
	this->open();
	_xmidiParser = MidiParser::createParser_XMIDI();
}

MusicPlayer::~MusicPlayer() {
	_driver->setTimerCallback(NULL, NULL);
	stop();
	this->close();
	_xmidiParser->setMidiDriver(NULL);
	delete _xmidiParser;
}

void MusicPlayer::setVolume(int volume) {
	Common::StackLock lock(_mutex);

	// FIXME: Could we simply change MAXMIDIVOL to match ScummVM's range?
	volume = CLIP((255 * volume) / MAXMIDIVOL, 0, 255);
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);

	if (_masterVolume == volume)
		return;

	_masterVolume = volume;

	for (int i = 0; i < 16; ++i) {
		if (_channel[i]) {
			_channel[i]->volume(_channelVolume[i] * _masterVolume / 255);
		}
	}
}

int MusicPlayer::open() {
	// Don't ever call open without first setting the output driver!
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;

	_driver->setTimerCallback(this, &onTimer);
	return 0;
}

void MusicPlayer::close() {
	stop();
	if (_driver)
		_driver->close();
	_driver = 0;
}

void MusicPlayer::send(uint32 b) {
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

void MusicPlayer::metaEvent(byte type, byte *data, uint16 length) {
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

void MusicPlayer::onTimer(void *refCon) {
	MusicPlayer *music = (MusicPlayer *)refCon;
	Common::StackLock lock(music->_mutex);

	if (music->_isPlaying)
		music->_parser->onTimer();
}

void MusicPlayer::playXMIDI(byte *midiData, uint32 size, bool loop) {
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

void MusicPlayer::stop() {
	Common::StackLock lock(_mutex);

	_isPlaying = false;
	if (_parser) {
		_parser->unloadMusic();
		_parser = NULL;
	}
}

void MusicPlayer::pause() {
	setVolume(-1);
	_isPlaying = false;
}

void MusicPlayer::resume() {
	setVolume(GetMidiVolume());
	_isPlaying = true;
}

void CurrentMidiFacts(SCNHANDLE	*pMidi, bool *pLoop) {
	*pMidi = currentMidi;
	*pLoop = currentLoop;
}

void RestoreMidiFacts(SCNHANDLE	Midi, bool Loop) {
	AudioCD.stop();
	StopMidi();

	currentMidi = Midi;
	currentLoop = Loop;

	if (volMidi != 0 && Loop) {
		PlayMidiSequence(currentMidi, true);
		SetMidiVolume(volMidi);
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

} // End of namespace Made
