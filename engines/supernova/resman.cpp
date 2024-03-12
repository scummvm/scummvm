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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"
#include "audio/mods/protracker.h"
#include "common/memstream.h"
#include "common/system.h"
#include "graphics/cursorman.h"

#include "supernova/graphics.h"
#include "supernova/resman.h"
#include "supernova/screen.h"
#include "supernova/supernova.h"

namespace Supernova {

struct AudioInfo {
	int _filenumber;
	int _offsetStart;
	int _offsetEnd;
};

static Common::MemoryReadStream *convertToMod(const char *filename, int version = 1);

static const AudioInfo audioInfo1[] = {
	{44,     0,    -1},
	{45,     0,    -1},
	{46,     0,  2510},
	{46,  2510,  4020},
	{46,  4020,    -1},
	{47,     0, 24010},
	{47, 24010,    -1},
	{48,     0,  2510},
	{48,  2510, 10520},
	{48, 10520, 13530},
	{48, 13530,    -1},
	{50,     0, 12786},
	{50, 12786,    -1},
	{51,     0,    -1},
	{53,     0,    -1},
	{54,     0,  8010},
	{54,  8010, 24020},
	{54, 24020, 30030},
	{54, 30030, 31040},
	{54, 31040,    -1},
};

static const AudioInfo audioInfo2[] = {
	{55,     18230,    -1},
	{47,     0,     16010},
	{47,     16010, 17020},
	{49,     8010,     -1},
	{49,     0,      8010},
	{53,     30020,    -1},
	{55,     7010,  17020},
	{55,     0,      7010},
	{53,     5010,  30020},
	{55,     18230,    -1},
	{55,     17020, 18230},
	{53,     0,      5010},
	{47,     17020,    -1},
	{51,     9020,     -1},
	{51,     0,      6010},
	{50,     0,        -1},
	{51,     6010,   9020},
	{54,     0,        -1},
	{48,     0,        -1}
};

static const byte mouseNormal[64] = {
	0xff,0x3f,0xff,0x1f,0xff,0x0f,0xff,0x07,
	0xff,0x03,0xff,0x01,0xff,0x00,0x7f,0x00,
	0x3f,0x00,0x1f,0x00,0x0f,0x00,0x0f,0x00,
	0xff,0x00,0x7f,0x18,0x7f,0x38,0x7f,0xfc,

	0x00,0x00,0x00,0x40,0x00,0x60,0x00,0x70,
	0x00,0x78,0x00,0x7c,0x00,0x7e,0x00,0x7f,
	0x80,0x7f,0xc0,0x7f,0xe0,0x7f,0x00,0x7e,
	0x00,0x66,0x00,0x43,0x00,0x03,0x00,0x00
};

static const byte mouseWait[64] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x80,
	0x01,0x80,0x01,0x80,0x11,0x88,0x31,0x8c,
	0x31,0x8c,0x11,0x88,0x01,0x80,0x01,0x80,
	0x01,0x80,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0xfe,0x7f,0xf4,0x2f,0xf4,0x2f,
	0x14,0x28,0x24,0x24,0x44,0x22,0x84,0x21,
	0x84,0x21,0xc4,0x23,0xe4,0x27,0x74,0x2e,
	0x34,0x2c,0x14,0x28,0xfe,0x7f,0x00,0x00
};


ResourceManager::ResourceManager(SupernovaEngine *vm)
	: _audioRate(11931)
	, _vm(vm) {
	if (_vm->_MSPart == 1)
		_soundSamples = new Common::ScopedPtr<Audio::SeekableAudioStream>[kAudioNumSamples1];
	else if (_vm->_MSPart == 2)
		_soundSamples = new Common::ScopedPtr<Audio::SeekableAudioStream>[kAudioNumSamples2];
	initGraphics();
}

ResourceManager::~ResourceManager() {
	if (_vm->_MSPart == 1) {
		for (int i = 0; i < 44; i++)
			delete _images[i];
	}
	if (_vm->_MSPart == 2) {
		for (int i = 0; i < 47; i++)
			delete _images[i];
	}
	delete[] _soundSamples;
	delete[] _images;
}

void ResourceManager::initGraphics() {
	Screen::initPalette();
	initCursorGraphics();
	if (_vm->_MSPart == 1)
		initImages1();
	else if (_vm->_MSPart == 2)
		initImages2();
}

void ResourceManager::initCursorGraphics() {
	const uint16 *bufferNormal = reinterpret_cast<const uint16 *>(mouseNormal);
	const uint16 *bufferWait = reinterpret_cast<const uint16 *>(mouseWait);
	for (uint i = 0; i < sizeof(mouseNormal) / 4; ++i) {
		for (uint bit = 0; bit < 16; ++bit) {
			uint mask = 0x8000 >> bit;
			uint bitIndex = i * 16 + bit;

			_cursorNormal[bitIndex] = (READ_LE_UINT16(bufferNormal + i) & mask) ?
										 kColorCursorTransparent : kColorBlack;
			if (READ_LE_UINT16(bufferNormal + i + 16) & mask)
				_cursorNormal[bitIndex] = kColorLightRed;

			_cursorWait[bitIndex] = (READ_LE_UINT16(bufferWait + i) & mask) ?
									   kColorCursorTransparent : kColorBlack;
			if (READ_LE_UINT16(bufferWait + i + 16) & mask)
				_cursorWait[bitIndex] = kColorLightRed;
		}
	}
}

void ResourceManager::initImages1() {
	_images = new MSNImage *[kNumImageFiles1];
	for (int i = 0; i < kNumImageFiles1; ++i) {
		_images[i] = nullptr;
	}
}

void ResourceManager::initImages2() {
	_images = new MSNImage *[kNumImageFiles2];
	for (int i = 0; i < kNumImageFiles2; ++i) {
		_images[i] = nullptr;
	}
}

// Sound
// Note:
//   - samples start with a header of 6 bytes: 01 SS SS 00 AD 00
//     where SS SS (LE uint16) is the size of the sound sample + 2
//   - samples end with a footer of 4 bytes: 00 00
// Skip those in the buffer
void ResourceManager::loadSound1(AudioId id) {
	Common::File file;
	if (!file.open(Common::Path(Common::String::format("msn_data.%03d", audioInfo1[id]._filenumber)))) {
		error("File %s could not be read!", file.getName());
	}

	int length = 0;
	byte *buffer = nullptr;

	if (audioInfo1[id]._offsetEnd == -1) {
		file.seek(0, SEEK_END);
		length = file.pos() - audioInfo1[id]._offsetStart - 10;
	} else {
		length = audioInfo1[id]._offsetEnd - audioInfo1[id]._offsetStart - 10;
	}
	buffer = new byte[length];
	file.seek(audioInfo1[id]._offsetStart + 6);
	file.read(buffer, length);
	file.close();

	byte streamFlag = Audio::FLAG_UNSIGNED | Audio::FLAG_LITTLE_ENDIAN;
	_soundSamples[id].reset(Audio::makeRawStream(buffer, length, _audioRate,
												streamFlag, DisposeAfterUse::YES));
}

void ResourceManager::loadSound2(AudioId id) {
	Common::File file;
	if (!file.open(Common::Path(Common::String::format("ms2_data.%03d", audioInfo2[id]._filenumber)))) {
		error("File %s could not be read!", file.getName());
	}

	int length = 0;
	byte *buffer = nullptr;

	if (audioInfo2[id]._offsetEnd == -1) {
		file.seek(0, SEEK_END);
		length = file.pos() - audioInfo2[id]._offsetStart - 10;
	} else {
		length = audioInfo2[id]._offsetEnd - audioInfo2[id]._offsetStart - 10;
	}
	buffer = new byte[length];
	file.seek(audioInfo2[id]._offsetStart + 6);
	file.read(buffer, length);
	file.close();

	byte streamFlag = Audio::FLAG_UNSIGNED | Audio::FLAG_LITTLE_ENDIAN;
	_soundSamples[id].reset(Audio::makeRawStream(buffer, length, _audioRate,
												streamFlag, DisposeAfterUse::YES));
}

void ResourceManager::loadImage(int filenumber) {
	if (_vm->_MSPart == 1) {
		if (filenumber < 44) {
			_images[filenumber] = new MSNImage(_vm);
			if (!_images[filenumber]->init(filenumber))
				error("Failed reading image file msn_data.%03d", filenumber);
		} else {
			_images[44] = new MSNImage(_vm);
			if (!_images[44]->init(filenumber))
				error("Failed reading image file msn_data.%03d", filenumber);
		}
	} else if (_vm->_MSPart == 2) {
		_images[filenumber] = new MSNImage(_vm);
		if (!_images[filenumber]->init(filenumber))
			error("Failed reading image file ms2_data.%03d", filenumber);
	}
}

Audio::SeekableAudioStream *ResourceManager::getSoundStream(AudioId index) {
	if (!_soundSamples[index]) {
		if (_vm->_MSPart == 1)
			loadSound1(index);
		else if (_vm->_MSPart == 2)
			loadSound2(index);
	}
	Audio::SeekableAudioStream *stream;
	stream = _soundSamples[index].get();
	stream->rewind();

	return stream;
}

Audio::AudioStream *ResourceManager::getSoundStream(MusicId index) {
	switch (index) {
	case kMusicIntro:
		if (!_musicIntroBuffer) {
			if (_vm->_MSPart == 1)
				_musicIntroBuffer.reset(convertToMod("msn_data.052", 1));
			else if (_vm->_MSPart == 2)
				_musicIntroBuffer.reset(convertToMod("ms2_data.052", 2));
		}
		_musicIntro.reset(Audio::makeProtrackerStream(_musicIntroBuffer.get()));
		return _musicIntro.get();
	case kMusicMadMonkeys:
		// fall through
	case kMusicOutro:
		if (!_musicOutroBuffer) {
			if (_vm->_MSPart == 1)
				_musicOutroBuffer.reset(convertToMod("msn_data.049", 1));
			else if (_vm->_MSPart == 2)
				_musicOutroBuffer.reset(convertToMod("ms2_data.056", 2));
		}
		_musicOutro.reset(Audio::makeProtrackerStream(_musicOutroBuffer.get()));
		return _musicOutro.get();
	default:
		error("Invalid music constant in playAudio()");
	}
}

Audio::AudioStream *ResourceManager::getSirenStream() {
	if (!_sirenStream)
		initSiren();
	return _sirenStream.get();
}

MSNImage *ResourceManager::getImage(int filenumber) {
	//check array boundaries
	if (_vm->_MSPart == 1 && filenumber > 43 && filenumber != 55)
		return nullptr;
	if (_vm->_MSPart == 2 && filenumber > 46)
		return nullptr;

	if (filenumber == 55) {
		if (!_images[44])
			loadImage(filenumber);
		return _images[44];
	} else {
		if (!_images[filenumber])
			loadImage(filenumber);
		return _images[filenumber];
	}
}

// Generate a tone which minimal length is the length and ends at the end
// of sine period
// NOTE: Size of the SineTable has to be the same as audioRate and a multiple of 4
byte *ResourceManager::generateTone(byte *buffer, int frequency, int length, int audioRate, Math::SineTable &table) {
	int i = 0;

	// Make sure length is a multiple of audioRate / frequency to end on a full sine wave and not in the middle.
	// Also the length we have is a minimum length, so only increase it.
	int r = 1 + (length - 1) * frequency / audioRate;
	length = (1 + 2 * r * audioRate / frequency) / 2;
	for(; i < length; i++) {
		buffer[i] = (byte)
			((table.at((i * frequency) % audioRate) * 127) + 127);
	}
	return buffer + length;
}

// Tones with frequencies between 1500 Hz and 1800 Hz, frequencies go up and down
// with a step of 10 Hz.
void ResourceManager::initSiren() {
	int audioRate = 44000;
	int length = audioRate / 90; // minimal length of each tone

	// * 60 for the minimal length, another 20 * length as a spare, for longer tones
	byte *buffer = new byte[length * 80];
	byte *pBuffer = buffer;
	Math::SineTable table(audioRate);

	for (int i = 0; i < 30; i++)
		pBuffer = generateTone(pBuffer, 1800 - i * 10, length, audioRate, table);

	for (int i = 0; i < 30; i++)
		pBuffer = generateTone(pBuffer, 1500 + i * 10, length, audioRate, table);

	byte streamFlag = Audio::FLAG_UNSIGNED;

	_sirenStream.reset(Audio::makeLoopingAudioStream(
			Audio::makeRawStream(buffer, pBuffer - buffer, audioRate,
									streamFlag, DisposeAfterUse::YES), 0));
}

static Common::MemoryReadStream *convertToMod(const char *filename, int version) {
	// MSN format
	struct {
		uint16 seg;
		uint16 start;
		uint16 end;
		uint16 loopStart;
		uint16 loopEnd;
		char volume;
		char dummy[5];
	} instr2[22];
	int nbInstr2; // 22 for version1, 15 for version 2
	int16 songLength;
	char arrangement[128];
	int16 patternNumber;
	int32 note2[28][64][4];

	nbInstr2 = ((version == 1) ? 22 : 15);

	Common::File msnFile;
	msnFile.open(filename);
	if (!msnFile.isOpen()) {
		warning("Data file '%s' not found", msnFile.getName());
		return nullptr;
	}

	for (int i = 0 ; i < nbInstr2 ; ++i) {
		instr2[i].seg = msnFile.readUint16LE();
		instr2[i].start = msnFile.readUint16LE();
		instr2[i].end = msnFile.readUint16LE();
		instr2[i].loopStart = msnFile.readUint16LE();
		instr2[i].loopEnd = msnFile.readUint16LE();
		instr2[i].volume = msnFile.readByte();
		msnFile.read(instr2[i].dummy, 5);
	}
	songLength = msnFile.readSint16LE();
	msnFile.read(arrangement, 128);
	patternNumber = msnFile.readSint16LE();
	for (int p = 0 ; p < patternNumber ; ++p) {
		for (int n = 0 ; n < 64 ; ++n) {
			for (int k = 0 ; k < 4 ; ++k) {
				note2[p][n][k] = msnFile.readSint32LE();
			}
		}
	}

	/* MOD format */
	struct {
		char iname[22];
		uint16 length;
		char finetune;
		char volume;
		uint16 loopStart;
		uint16 loopLength;
	} instr[31];
	int32 note[28][64][4];

	// We can't recover some MOD effects since several of them are mapped to 0.
	// Assume the MSN effect of value 0 is Arpeggio (MOD effect of value 0).
	const char invConvEff[8] = {0, 1, 2, 3, 10, 12, 13 ,15};

	// Reminder from convertToMsn
	// 31 30 29 28 27 26 25 24 - 23 22 21 20 19 18 17 16 - 15 14 13 12 11 10 09 08 - 07 06 05 04 03 02 01 00
	//  h  h  h  h  g  g  g  g    f  f  f  f  e  e  e  e    d  d  d  d  c  c  c  c    b  b  b  b  a  a  a  a
	//
	// MSN:
	//  hhhh             (4 bits) Cleared to 0
	//  dddd c           (5 bits) Sample index   | after mapping through convInstr
	//        ccc        (3 bits) Effect type    | after mapping through convEff
	//  bbbb aaaa        (8 bits) Effect value   | unmodified
	//  gggg ffff eeee  (12 bits) Sample period  | unmodified
	//
	// MS2:
	//  hhhh             (4 bits) Cleared to 0
	//  dddd             (4 bits) Sample index   | after mapping through convInstr
	//  cccc             (4 bits) Effect type    | unmodified
	//  bbbb aaaa        (8 bits) Effect value   | unmodified
	//  gggg ffff eeee  (12 bits) Sample period  | transformed (0xE000 / p) - 256
	//
	// MOD:
	//  hhhh dddd        (8 bits) Sample index
	//  cccc             (4 bits) Effect type for this channel/division
	//  bbbb aaaa        (8 bits) Effect value
	//  gggg ffff eeee  (12 bits) Sample period

	// Can we recover the instruments mapping? I don't think so as part of the original instrument index is cleared.
	// And it doesn't really matter as long as we are consistent.
	// However we need to make sure 31 (or 15 in MS2) is mapped to 0 in MOD.
	// We just add 1 to all other values, and this means a 1 <-> 1 mapping for the instruments
	for (int p = 0; p < patternNumber; ++p) {
		for (int n = 0; n < 64; ++n) {
			for (int k = 0; k < 4; ++k) {
				int32* l = &(note[p][n][k]);
				*l = note2[p][n][k];
				int32 i = 0;
				if (nbInstr2 == 22) { // version 1
					i = ((*l & 0xF800) >> 11);
					int32 e = ((*l & 0x0700) >> 8);
					int32 e1 = invConvEff[e];
					*l &= 0x0FFF00FF;
					*l |= (e1 << 8);
				} else { // version 2
					int32 h = (*l >> 16);
					i = ((*l & 0xF000) >> 12);
					*l &= 0x00000FFF;
					if (h)
						h = 0xE000 / (h + 256);
					*l |= (h << 16);
					if (i == 15)
						i = 31;
				}

				// Add back index in note
				if (i != 31) {
					++i;
					*l |= ((i & 0x0F) << 12);
					*l |= ((i & 0xF0) << 24);
				}
			}
		}
	}

	for (int i = 0; i < 31; ++i) {
		// iname is not stored in the mod file. Just set it to 'instrument#'
		// finetune is not stored either. Assume 0.
		memset(instr[i].iname, 0, 22);
		Common::sprintf_s(instr[i].iname, "instrument%d", i+1);
		instr[i].length = 0;
		instr[i].finetune = 0;
		instr[i].volume = 0;
		instr[i].loopStart = 0;
		instr[i].loopLength = 0;

		if (i < nbInstr2) {
			instr[i].length = ((instr2[i].end - instr2[i].start) >> 1);
			instr[i].loopStart = ((instr2[i].loopStart - instr2[i].start) >> 1);
			instr[i].loopLength = (( instr2[i].loopEnd - instr2[i].loopStart) >> 1);
			instr[i].volume = instr2[i].volume;
		}
	}

	// The ciaaSpeed is kind of useless and not present in the MSN file.
	// Traditionally 0x78 in SoundTracker. Was used in NoiseTracker as a restart point.
	// ProTracker uses 0x7F. FastTracker uses it as a restart point, whereas ScreamTracker 3 uses 0x7F like ProTracker.
	// You can use this to roughly detect which tracker made a MOD, and detection gets more accurate for more obscure MOD types.
	char ciaaSpeed = 0x7F;

	// The mark cannot be recovered either. Since we have 4 channels and 31 instrument it can be either ID='M.K.' or ID='4CHN'.
	// Assume 'M.K.'
	const char mark[4] = { 'M', '.', 'K', '.' };

	Common::MemoryWriteStreamDynamic buffer(DisposeAfterUse::NO);

	buffer.write(msnFile.getName(), 19);
	buffer.writeByte(0);

	for (int i = 0 ; i < 31 ; ++i) {
		buffer.write(instr[i].iname, 22);
		buffer.writeUint16BE(instr[i].length);
		buffer.writeByte(instr[i].finetune);
		buffer.writeByte(instr[i].volume);
		buffer.writeUint16BE(instr[i].loopStart);
		buffer.writeUint16BE(instr[i].loopLength);
	}
	buffer.writeByte((char)songLength);
	buffer.writeByte(ciaaSpeed);
	buffer.write(arrangement, 128);
	buffer.write(mark, 4);

	for (int p = 0 ; p < patternNumber ; ++p) {
		for (int n = 0 ; n < 64 ; ++n) {
			for (int k = 0 ; k < 4 ; ++k) {
//				buffer.writeUint32BE(*((uint32*)(note[p][n]+k)));
				buffer.writeSint32BE(note[p][n][k]);
			}
		}
	}

	uint nb;
	char buf[4096];
	while ((nb = msnFile.read(buf, 4096)) > 0)
		buffer.write(buf, nb);

	return new Common::MemoryReadStream(buffer.getData(), buffer.size(), DisposeAfterUse::YES);
}

int ResourceManager::getAudioRate() {
	return _audioRate;
}

const byte *ResourceManager::getCursor(CursorId id) const {
	switch (id) {
	case kCursorNormal:
		return _cursorNormal;
	case kCursorWait:
		return _cursorWait;
	default:
		return nullptr;
	}
}

}
