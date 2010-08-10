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

// Code is based on:
//
// A very simple program, that converts an AGI-song into a MIDI-song.
// Feel free to use it for anything.
//
// The default instrument is "piano" for all the channels, what gives
// good results on most games. But I found, that some songs are interesting
// with other instruments. If you want to experiment, modify the "instr"
// array.
//
// Timing is not perfect, yet. It plays correct, when I use the
// Gravis-Midiplayer, but the songs are too fast when I use playmidi on
// Linux.
// 
// Original program developed by Jens. Christian Restemeier
//

// MIDI and digital music class

#include "sound/audiostream.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/stream.h"

#include "agi/agi.h"

#include "agi/sound.h"
#include "agi/sound_midi.h"

#define SPEED_FACTOR 6

namespace Agi {

static uint32 convertSND2MIDI(byte *snddata, byte **data);

MIDISound::MIDISound(uint8 *data, uint32 len, int resnum, SoundMgr &manager) : AgiSound(manager) {
	_data = data; // Save the resource pointer
	_len  = len;  // Save the resource's length
	_type = READ_LE_UINT16(data); // Read sound resource's type
	_isValid = (_type == AGI_SOUND_4CHN) && (_data != NULL) && (_len >= 2);

	if (!_isValid) // Check for errors
		warning("Error creating MIDI sound from resource %d (Type %d, length %d)", resnum, _type, len);
}

SoundGenMIDI::SoundGenMIDI(AgiEngine *vm, Audio::Mixer *pMixer) : SoundGen(vm, pMixer), _parser(0), _isPlaying(false), _passThrough(false), _isGM(false) {
	DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB);
	_driver = MidiDriver::createMidi(dev);

	memset(_channel, 0, sizeof(_channel));
	memset(_channelVolume, 255, sizeof(_channelVolume));
	_masterVolume = 0;
	this->open();
	_smfParser = MidiParser::createParser_SMF();
	_midiMusicData = NULL;
}

SoundGenMIDI::~SoundGenMIDI() {
	_driver->setTimerCallback(NULL, NULL);
	stop();
	this->close();
	_smfParser->setMidiDriver(NULL);
	delete _smfParser;
	delete[] _midiMusicData;
}

void SoundGenMIDI::setChannelVolume(int channel) {
	int newVolume = _channelVolume[channel] * _masterVolume / 255;
	_channel[channel]->volume(newVolume);
}

void SoundGenMIDI::setVolume(int volume) {
	Common::StackLock lock(_mutex);

	volume = CLIP(volume, 0, 255);
	if (_masterVolume == volume)
		return;
	_masterVolume = volume;

	for (int i = 0; i < 16; ++i) {
		if (_channel[i]) {
			setChannelVolume(i);
		}
	}
}

int SoundGenMIDI::open() {
	// Don't ever call open without first setting the output driver!
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;

	_driver->setTimerCallback(this, &onTimer);
	return 0;
}

void SoundGenMIDI::close() {
	stop();
	if (_driver)
		_driver->close();
	_driver = 0;
}

void SoundGenMIDI::send(uint32 b) {
	if (_passThrough) {
		_driver->send(b);
		return;
	}

	byte channel = (byte)(b & 0x0F);
	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by master volume
		byte volume = (byte)((b >> 16) & 0x7F);
		_channelVolume[channel] = volume;
		volume = volume * _masterVolume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xF0) == 0xC0 && !_isGM && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}
	else if ((b & 0xFFF0) == 0x007BB0) {
		//Only respond to All Notes Off if this channel
		//has currently been allocated
		if (_channel[b & 0x0F])
			return;
	}

	if (!_channel[channel]) {
		_channel[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
		// If a new channel is allocated during the playback, make sure
		// its volume is correctly initialized.
		if (_channel[channel])
			setChannelVolume(channel);
	}

	if (_channel[channel])
		_channel[channel]->send(b);
}

void SoundGenMIDI::metaEvent(byte type, byte *data, uint16 length) {

	switch (type) {
	case 0x2F:	// End of Track
		stop();
		_vm->_sound->soundIsFinished();
		break;
	default:
		//warning("Unhandled meta event: %02x", type);
		break;
	}
}

void SoundGenMIDI::onTimer(void *refCon) {
	SoundGenMIDI *music = (SoundGenMIDI *)refCon;
	Common::StackLock lock(music->_mutex);

	if (music->_parser)
		music->_parser->onTimer();
}

void SoundGenMIDI::play(int resnum) {
	MIDISound *track;
 
	stop();

	_isGM = true;

	track = (MIDISound *)_vm->_game.sounds[resnum];

	// Convert AGI Sound data to MIDI
	int midiMusicSize = convertSND2MIDI(track->_data, &_midiMusicData);

	if (_smfParser->loadMusic(_midiMusicData, midiMusicSize)) {
		MidiParser *parser = _smfParser;
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(getBaseTempo());
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

		_parser = parser;

		syncVolume();

		_isPlaying = true;
	}
}

void SoundGenMIDI::stop() {
	Common::StackLock lock(_mutex);

	if (!_isPlaying)
		return;

	_isPlaying = false;
	if (_parser) {
		_parser->unloadMusic();
		_parser = NULL;
	}
}

void SoundGenMIDI::pause() {
	setVolume(-1);
	_isPlaying = false;
}

void SoundGenMIDI::resume() {
	syncVolume();
	_isPlaying = true;
}

void SoundGenMIDI::syncVolume() {
	int volume = ConfMan.getInt("music_volume");
	if (ConfMan.getBool("mute")) {
		volume = -1;
	}
	setVolume(volume);
}

/* channel / intrument setup: */

/* most songs are good with this: */
unsigned char instr[] = {0, 0, 0};

/* cool for sq2:
unsigned char instr[] = {50, 51, 19};
*/

static void writeDelta(Common::MemoryWriteStreamDynamic *st, int32 delta) {
	int32 i;

	i = delta >> 21; if (i > 0) st->writeByte((i & 127) | 128); 
	i = delta >> 14; if (i > 0) st->writeByte((i & 127) | 128);
	i = delta >> 7;  if (i > 0) st->writeByte((i & 127) | 128);
	st->writeByte(delta & 127);
}

static uint32 convertSND2MIDI(byte *snddata, byte **data) {
	int32 lp, ep;
	int n;
	double ll;

	Common::MemoryWriteStreamDynamic st;

	ll = log10(pow(2.0, 1.0 / 12.0));

	/* Header */
	st.write("MThd", 4);
	st.writeUint32BE(6);
	st.writeUint16BE(1);    /* mode */
	st.writeUint16BE(3);    /* number of tracks */
	st.writeUint16BE(192);  /* ticks / quarter */

	for (n = 0; n < 3; n++) {
		uint16 start, end, pos;
        
		st.write("MTrk", 4);
		lp = st.pos();
		st.writeUint32BE(0);        /* chunklength */
		writeDelta(&st, 0);       /* set instrument */
		st.writeByte(0xc0 + n);
		st.writeByte(instr[n]);
		start = snddata[n * 2 + 0] | (snddata[n * 2 + 1] << 8);
		end = ((snddata[n * 2 + 2] | (snddata[n * 2 + 3] << 8))) - 5;

		for (pos = start; pos < end; pos += 5) {
			uint16 freq,  dur;
			dur = (snddata[pos + 0] | (snddata[pos + 1] << 8)) * SPEED_FACTOR;
			freq = ((snddata[pos + 2] & 0x3F)  <<  4)  +  (snddata[pos + 3] & 0x0F);
			if (snddata[pos + 2] > 0) {
				double fr;
				int note;
				/* I don't know,  what frequency equals midi note 0 ... */
				/* This moves the song 4 octaves down: */
				fr = (log10(111860.0 / (double)freq) / ll) - 48; 
				note = (int)floor(fr + 0.5);
				if (note < 0) note = 0;
				if (note > 127) note = 127;
				/* note on */
				writeDelta(&st, 0);
				st.writeByte(144 + n);
				st.writeByte(note);
				st.writeByte(100);
				/* note off */
				writeDelta(&st, dur);
				st.writeByte(128 + n);
				st.writeByte(note);
				st.writeByte(0);
			} else {
				/* note on */
				writeDelta(&st, 0);
				st.writeByte(144 + n);
				st.writeByte(0);
				st.writeByte(0);
				/* note off */
				writeDelta(&st, dur);
				st.writeByte(128 + n);
				st.writeByte(0);
				st.writeByte(0);
			}
		}       
		writeDelta(&st, 0);
		st.writeByte(0xff);
		st.writeByte(0x2f);
		st.writeByte(0x0);
		ep = st.pos();
		st.seek(lp, SEEK_SET);
		st.writeUint32BE((ep - lp) - 4);
		st.seek(ep, SEEK_SET);
	}

	*data = st.getData();

	return st.pos();
}

} // End of namespace Agi
