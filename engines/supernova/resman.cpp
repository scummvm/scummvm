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
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"
#include "audio/mods/protracker.h"
#include "common/memstream.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/palette.h"

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

static const AudioInfo audioInfo[kAudioNumSamples] = {
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
	{54, 31040,    -1}
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


ResourceManager::ResourceManager()
	: _audioRate(11931) {
	initSoundFiles();
	initGraphics();
}

void ResourceManager::initSoundFiles() {
	// Sound
	// Note:
	//   - samples start with a header of 6 bytes: 01 SS SS 00 AD 00
	//     where SS SS (LE uint16) is the size of the sound sample + 2
	//   - samples end with a footer of 4 bytes: 00 00
	// Skip those in the buffer
	Common::File file;

	for (int i = 0; i < kAudioNumSamples; ++i) {
		if (!file.open(Common::String::format("msn_data.%03d", audioInfo[i]._filenumber))) {
			error("File %s could not be read!", file.getName());
		}

		int length = 0;
		byte *buffer = nullptr;

		if (audioInfo[i]._offsetEnd == -1) {
			file.seek(0, SEEK_END);
			length = file.pos() - audioInfo[i]._offsetStart - 10;
		} else {
			length = audioInfo[i]._offsetEnd - audioInfo[i]._offsetStart - 10;
		}
		buffer = new byte[length];
		file.seek(audioInfo[i]._offsetStart + 6);
		file.read(buffer, length);
		file.close();

		byte streamFlag = Audio::FLAG_UNSIGNED | Audio::FLAG_LITTLE_ENDIAN;
		_soundSamples[i].reset(Audio::makeRawStream(buffer, length, _audioRate,
													streamFlag, DisposeAfterUse::YES));
	}

	_musicIntroBuffer.reset(convertToMod("msn_data.052"));
	_musicOutroBuffer.reset(convertToMod("msn_data.049"));
}

void ResourceManager::initGraphics() {
	Screen::initPalette();
	initCursorGraphics();
	initImages();
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

void ResourceManager::initImages() {
	for (int i = 0; i < 44; ++i) {
		if (!_images[i].init(i))
			error("Failed reading image file msn_data.%03d", i);
	}
	if (!_images[44].init(55))
			error("Failed reading image file msn_data.055");
}

Audio::SeekableAudioStream *ResourceManager::getSoundStream(AudioId index) {
	Audio::SeekableAudioStream *stream = _soundSamples[index].get();
	stream->rewind();

	return stream;
}

Audio::AudioStream *ResourceManager::getSoundStream(MusicId index) {
	switch (index) {
	case kMusicIntro:
		_musicIntro.reset(Audio::makeProtrackerStream(_musicIntroBuffer.get()));
		return _musicIntro.get();
	case kMusicOutro:
		_musicOutro.reset(Audio::makeProtrackerStream(_musicOutroBuffer.get()));
		return _musicOutro.get();
	default:
		error("Invalid music constant in playAudio()");
	}
}

const MSNImage *ResourceManager::getImage(int filenumber) const {
	if (filenumber < 44)
		return &_images[filenumber];
	else if (filenumber == 55)
		return &_images[44];
	else
		return nullptr;
}

const byte *ResourceManager::getImage(CursorId id) const {
	switch (id) {
	case kCursorNormal:
		return _cursorNormal;
	case kCursorWait:
		return _cursorWait;
	default:
		return nullptr;
	}
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
		sprintf(instr[i].iname, "instrument%d", i+1);
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

}
