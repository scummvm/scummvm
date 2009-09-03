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

// FIXME: This is cribbed together from the SAGA music player. It needs cleanup
// and testing.

#include "m4/m4.h"
#include "m4/midi.h"
#include "common/stream.h"

namespace M4 {

MidiPlayer::MidiPlayer(M4Engine *vm, MidiDriver *driver) : _vm(vm), _midiData(NULL), _driver(driver), _isPlaying(false), _passThrough(false), _isGM(false) {
	memset(_channel, 0, sizeof(_channel));
	_masterVolume = 0;
	_parser = MidiParser::createParser_SMF();
	_parser->setMidiDriver(this);
	_parser->setTimerRate(getBaseTempo());
	open();
}

MidiPlayer::~MidiPlayer() {
	_driver->setTimerCallback(NULL, NULL);
	_parser->setMidiDriver(NULL);
	stopMusic();
	close();
	delete _parser;
	
	if (_midiData)
		free(_midiData);
}

void MidiPlayer::setVolume(int volume) {
	Common::StackLock lock(_mutex);

	if (volume < 0)
		volume = 0;
	else if (volume > 255)
		volume = 255;

	if (_masterVolume == volume)
		return;

	_masterVolume = volume;

	for (int i = 0; i < 16; ++i) {
		if (_channel[i]) {
			_channel[i]->volume(_channelVolume[i] * _masterVolume / 255);
		}
	}
}

int MidiPlayer::open() {
	// Don't ever call open without first setting the output driver!
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;

	_driver->setTimerCallback(this, &onTimer);
	return 0;
}

void MidiPlayer::close() {
	stopMusic();
	if (_driver)
		_driver->close();
	_driver = 0;
}

void MidiPlayer::send(uint32 b) {
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

	if (!_channel[channel])
		_channel[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();

	if (_channel[channel])
		_channel[channel]->send(b);
}

void MidiPlayer::metaEvent(byte type, byte *data, uint16 length) {
	switch (type) {
	case 0x2F:
		// End of track. (Not called when auto-looping.)
		stopMusic();
		break;
	case 0x51:
		// Set tempo. Handled by the standard MIDI parser already.
		break;
	default:
		warning("Unhandled meta event: %02x", type);
		break;
	}
}

void MidiPlayer::onTimer(void *refCon) {
	MidiPlayer *midi = (MidiPlayer *)refCon;
	Common::StackLock lock(midi->_mutex);

	if (midi->_isPlaying)
		midi->_parser->onTimer();
}

void MidiPlayer::playMusic(const char *name, int32 vol, bool loop, int32 trigger, int32 scene) {
	stopMusic();

	char fullname[144];
	_vm->res()->changeExtension(fullname, name, "HMP");

	Common::SeekableReadStream *midiFile = _vm->res()->get(fullname);
	byte *hmpData = new byte[midiFile->size()];
	uint32 smfSize;

	midiFile->read(hmpData, midiFile->size());
	_midiData = convertHMPtoSMF(hmpData, midiFile->size(), smfSize);
	delete[] hmpData;
	_vm->res()->toss(fullname);
	_vm->res()->purge();

	if (_midiData) {
		/*
		FILE *out = fopen("music.mid", "wb");
		fwrite(_midiData, smfSize, 1, out);
		fclose(out);
		*/
		_parser->loadMusic(_midiData, smfSize);
		_parser->property(MidiParser::mpAutoLoop, loop);
	}

	setVolume(255);

	_isPlaying = true;
}

void MidiPlayer::stopMusic() {
	Common::StackLock lock(_mutex);

	_isPlaying = false;
	if (_parser) {
		_parser->unloadMusic();
	}
	
	if (_midiData) {
		free(_midiData);
		_midiData = NULL;
	}
}

// This function will convert HMP music into type 1 SMF, which our SMF parser
// will be able to handle. It is based on Hans de Goede's HMP 2 MIDI file
// converter, which in turn is "based on the conversion algorithms found in
// d1x, d2x-xl and jjffe". Hans's original code is licensed under the LGPL.
//
// TODO: It would probably be nicer to write a MIDI parser class to deal with
// HMP data directly. Though the multi-track nature of HMP makes that tricky.

byte *MidiPlayer::convertHMPtoSMF(byte *data, uint32 inSize, uint32 &outSize) {
	Common::MemoryReadStream readS(data, inSize);
	Common::MemoryWriteStreamDynamic writeS;

	byte buf[8];

	readS.read(buf, sizeof(buf));
	if (memcmp(buf, "HMIMIDIP", 8) != 0) {
		warning("convertHMPtoSMF: Invalid HMP header");
		return NULL;
	}

	// Read the number of tracks. Note that all the tracks are still part
	// of the same song, just like in type 1 SMF files.

	readS.seek(0x30);

	uint32 numTracks = readS.readUint32LE();

	// The first track starts on offset 0x300. It's currently unknown what
	// the skipped data is for.

	readS.seek(0x300);

	// For some reason, we skip the first track entirely.

	byte a = readS.readByte();
	byte b = readS.readByte();
	byte c = readS.readByte();

	while (a != 0xFF || b != 0x2F || c != 0x00) {
		a = b;
		b = c;
		c = readS.readByte();
	}

	// The beginning of the MIDI header
	static const byte midiHeader1[] = { 'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 1 };
	// The last 2 bytes of the midi header and track 0
	static const byte midiHeader2[] = { 0, 0xC0, 'M', 'T', 'r', 'k', 0, 0, 0, 0x0B, 0, 0xFF, 0x51, 0x03, 0x18, 0x80, 0, 0, 0xFF, 0x2F, 0 };


	// Write the MIDI header
	writeS.write(midiHeader1, sizeof(midiHeader1));

	// Write the number of tracks
	writeS.writeUint16BE(numTracks);

	// Write the rest of the MIDI header and track 0.
	writeS.write(midiHeader2, sizeof(midiHeader2));

	// Read and convert all the tracks
	for (uint i = 1; i < numTracks; i++) {
		if (readS.readUint32LE() != i) {
			warning("convertHMPtoSMF: Invalid HMP track number");
			delete[] writeS.getData();
			return NULL;
		}

		uint32 trackLength = readS.readUint32LE() - 12;
		readS.readUint32LE();	// Unused?

		// Write the track header
		writeS.write("MTrk", 4);

		// This is where we will write the length of the track.
		uint32 trackLengthPos = writeS.pos();
		writeS.writeUint32LE(0);

		// In the original, this is cleared once at the beginning of
		// the function, but surely the last command does not carry
		// over to the next track?

		byte lastCmd = 0;

		// Now we can finally convert the track
		int32 endPos = readS.pos() + trackLength;
		while (readS.pos() < endPos) {
			// Convert the VLQ
			byte vlq[4];
			int j = -1;

			do {
				j++;
				vlq[j] = readS.readByte();
			} while (!(vlq[j] & 0x80));

			for (int k = 0; k <= j; k++) {
				a = vlq[j - k] & 0x7F;
				if (k != j)
					a |= 0x80;
				writeS.writeByte(a);
			}

			a = readS.readByte();

			if (a == 0xFF) {
				// META event
				b = readS.readByte();
				c = readS.readByte();

				writeS.writeByte(a);
				writeS.writeByte(b);
				writeS.writeByte(c);

				if (c > 0) {
					byte *metaBuf = new byte[c];
					readS.read(metaBuf, c);
					writeS.write(metaBuf, c);
					delete[] metaBuf;
				}

				if (b == 0x2F) {
					if (c != 0x00) {
						warning("convertHMPtoSMF: End of track with non-zero size");
						delete[] writeS.getData();
						return NULL;
					}
					break;
				}
			} else {
				if (a != lastCmd)
					writeS.writeByte(a);

				switch (a & 0xF0) {
				case 0x80:
				case 0x90:
				case 0xA0:
				case 0xB0:
				case 0xE0:
					b = readS.readByte();
					c = readS.readByte();
					writeS.writeByte(b);
					writeS.writeByte(c);
					break;
				case 0xC0:
				case 0xD0:
					b = readS.readByte();
					writeS.writeByte(b);
					break;
				default:
					warning("convertHMPtoSMF: Invalid HMP command %02X", a);
					delete[] writeS.getData();
					return NULL;
				}

				lastCmd = a;
			}
		}

		if (readS.pos() != endPos) {
			warning("convertHMPtoSMF: Invalid track length");
			delete[] writeS.getData();
			return NULL;
		}

		WRITE_BE_UINT32(writeS.getData() + trackLengthPos, writeS.pos() - trackLengthPos - 4);
	}

	outSize = writeS.size();
	return writeS.getData();
}

} // End of namespace M4
