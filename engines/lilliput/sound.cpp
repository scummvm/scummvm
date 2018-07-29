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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "lilliput/lilliput.h"
#include "lilliput/sound.h"

#include "common/debug.h"

namespace Lilliput {

static const byte _aliasArr[40] = {
	44,  0,  1,  2, 37,  3, 24,   45, 20, 19,
	16, 10, 11, 12, 41, 39, 40,   21, 22, 23,
	 4,  5,  6, 52,  7,  8,  9,   33, 13, 14,
	15, 18, 26, 25, 38, 29, 36, 0xFF, 28, 40
};

static const bool _loopArr[40] = {
	0, 0, 0, 1, 1, 1, 0, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 0, 1, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const byte _soundType [40] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 1, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0, 0, 0
};

LilliputSound::LilliputSound() {
	_unpackedFiles = nullptr;
	_unpackedSizes = nullptr;
	_fileNumb = 0;

	_isGM = false;

	MidiPlayer::createDriver();

	int ret = _driver->open();
	if (ret == 0) {
		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();

		_driver->setTimerCallback(this, &timerCallback);
	}
}

LilliputSound::~LilliputSound() {
	Audio::MidiPlayer::stop();

	if (_unpackedFiles) {
		for (int i = 0; i < _fileNumb; i++)
			free(_unpackedFiles[i]);
	}
	free(_unpackedFiles);
	free(_unpackedSizes);
}

byte LilliputSound::readByte(const byte *data, uint32 offset) {
	uint16 al = data[0x201 + (offset >> 1)];
	return data[1 + (offset & 1) + (al << 1)];
}

uint32 LilliputSound::decode(const byte *src, byte *dst, uint32 len, uint32 start) {
	uint32 i = start;
	for (; i < len; ++i) {
		*dst++ = readByte(src, i);
	}
	return i;
}

void LilliputSound::loadMusic(Common::String filename) {
	debugC(1, kDebugSound, "loadMusic(%s)", filename.c_str());

	Common::File f;

	if (!f.open(filename))
		error("Missing music file %s", filename.c_str());

	_fileNumb = f.readUint16LE();

	int *fileSizes = new int[_fileNumb + 1];
	for (int i = 0; i < _fileNumb; ++i)
		fileSizes[i] = f.readUint16LE();
	f.seek(0, SEEK_END);
	fileSizes[_fileNumb] = f.pos();

	_unpackedFiles = new byte *[_fileNumb];
	_unpackedSizes = new uint16[_fileNumb];
	int pos = (_fileNumb + 1) * 2; // file number + file sizes
	for (int i = 0; i < _fileNumb; ++i) {
		int packedSize = fileSizes[i + 1] - fileSizes[i];
		byte *srcBuf = new byte[packedSize];
		f.seek(pos, SEEK_SET);
		f.read(srcBuf, packedSize);
		if (srcBuf[0] == 'c' || srcBuf[0] == 'C') {
			int shift = (srcBuf[0] == 'c') ? 1 : 0;
			_unpackedSizes[i] = (1 + packedSize - 0x201) * 2 - shift;
			byte *dstBuf = new byte[_unpackedSizes[i]];
			decode(srcBuf, dstBuf, _unpackedSizes[i], shift);
			_unpackedFiles[i] = dstBuf;
		} else {
			_unpackedSizes[i] = packedSize;
			byte *dstBuf = new byte[packedSize];
			for (int j = 0; j < packedSize; ++j)
				dstBuf[j] = srcBuf[j];
			_unpackedFiles[i] = dstBuf;
		}
		delete[] srcBuf;
		pos += packedSize;
	}

	delete[] fileSizes;
	f.close();

	/* Debug code
	for (int i = 0; i < _fileNumb; ++i) {
		Common::DumpFile dmp;
		Common::String name = Common::String::format("dmp%d.mid", i);
		dmp.open(name);
		dmp.write(_unpackedFiles[i], _unpackedSizes[i]);
		dmp.close();
	}
	*/
}

void LilliputSound::send(uint32 b) {
	if (((b & 0xF0) == 0xC0) && !_isGM && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}

	Audio::MidiPlayer::send(b);
}

void LilliputSound::sendToChannel(byte channel, uint32 b) {
	if (!_channelsTable[channel]) {
		_channelsTable[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
		// If a new channel is allocated during the playback, make sure
		// its volume is correctly initialized.
		if (_channelsTable[channel])
			_channelsTable[channel]->volume(_channelsVolume[channel] * _masterVolume / 255);
	}

	if (_channelsTable[channel])
		_channelsTable[channel]->send(b);
}

void LilliputSound::init() {
	debugC(1, kDebugSound, "LilliputSound::init()");

	loadMusic("ROBIN.MUS");
}

void LilliputSound::refresh() {
	debugC(1, kDebugSound, "LilliputSound::refresh()");
}

void LilliputSound::playSound(int var1, Common::Point var2, Common::Point var3, Common::Point var4) {
	debugC(1, kDebugSound, "LilliputSound::playSound(%d, %d - %d, %d - %d, %d - %d)", var1, var2.x, var2.y, var3.x, var3.y, var4.x, var4.y);
	// warning("LilliputSound::playSound(%d, %d - %d, %d - %d, %d - %d)", var1, var2.x, var2.y, var3.x, var3.y, var4.x, var4.y);

	// save camera (var2)
	if (_aliasArr[var1] == 0xFF) {
		return;
	}

	if (var3 == Common::Point(-1, -1)) {
		playMusic(var1);
	} else if (_soundType[var1] == 0) {
		warning("Transient");
	} else {
		warning("longterm");
	}
}

void LilliputSound::playMusic(int var1) {
	int idx = _aliasArr[var1];
	bool loop = _loopArr[var1];

	_isGM = true;

	if (_parser)
		_parser->stopPlaying();

	MidiParser *parser = MidiParser::createParser_SMF();
	if (parser->loadMusic(_unpackedFiles[idx], _unpackedSizes[idx])) {
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(_driver->getBaseTempo());
		parser->property(MidiParser::mpAutoLoop, loop);
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

		_parser = parser;

		syncVolume();

		_isLooping = loop;
		_isPlaying = true;
	}
}

void LilliputSound::stopSound(Common::Point pos) {
	debugC(1, kDebugSound, "LilliputSound::stopSound(%d - %d)", pos.x, pos.y);
	warning("LilliputSound::stopSound(%d - %d)", pos.x, pos.y);
	// FIXME: Audio::MidiPlayer::stop() call required?
}

void LilliputSound::toggleOnOff() {
	debugC(1, kDebugSound, "LilliputSound::toggleOnOff()");
	warning("LilliputSound::toggleOnOff()");
}

void LilliputSound::update() {
	debugC(1, kDebugSound, "LilliputSound::update()");
	warning("LilliputSound::update()");
}

void LilliputSound::remove() {
	debugC(1, kDebugSound, "Lilliput::remove()");

	_parser->stopPlaying();
}

} // End of namespace
