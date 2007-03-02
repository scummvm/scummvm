/* ScummVM - Scumm Interpreter
 * Copyright (C) 2007 The ScummVM project
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

#include "common/stdafx.h"
#include "common/system.h"
#include "kyra/resource.h"
#include "kyra/sound.h"
#include "kyra/screen.h"

#include "sound/audiocd.h"
#include "sound/audiostream.h"

namespace Kyra {

SoundTowns::SoundTowns(KyraEngine *engine, Audio::Mixer *mixer) : Sound(engine, mixer), _lastTrack(-1),
	_currentSFX(0),	_sfxFileData(0), _sfxFileIndex((uint)-1), _sfxWDTable(0) {
}

SoundTowns::~SoundTowns() {
	AudioCD.stop();
	delete [] _sfxFileData;
	stopSoundEffect();
}

bool SoundTowns::init() {
	_engine->checkCD();
	int unused = 0;
	_sfxWDTable = _engine->staticres()->loadRawData(kKyra1TownsSFXTable, unused);
	return true;
}

void SoundTowns::process() {
	AudioCD.updateCD();
}

namespace {

struct CDTrackTable {
	uint32 fileOffset;
	bool loop;
	int track;
};

} // end of anonymous namespace

void SoundTowns::playTrack(uint8 track) {
	if (track < 2)
		return;
	track -= 2;

	static const CDTrackTable tTable[] = {
		{ 0x04000, 1,  0 },
		{ 0x05480, 1,  6 },
		{ 0x05E70, 0,  1 },
		{ 0x06D90, 1,  3 },
		{ 0x072C0, 0, -1 },
		{ 0x075F0, 1, -1 },
		{ 0x07880, 1, -1 },
		{ 0x089C0, 0, -1 },
		{ 0x09080, 0, -1 },
		{ 0x091D0, 1,  4 },
		{ 0x0A880, 1,  5 },
		{ 0x0AF50, 0, -1 },
		{ 0x0B1A0, 1, -1 },
		{ 0x0B870, 0, -1 },
		{ 0x0BCF0, 1, -1 },
		{ 0x0C5D0, 1,  7 },
		{ 0x0D3E0, 1,  8 },
		{ 0x0e7b0, 1,  2 },
		{ 0x0edc0, 0, -1 },
		{ 0x0eef0, 1,  9 },
		{ 0x10540, 1, 10 },
		{ 0x10d80, 0, -1 },
		{ 0x10E30, 0, -1 },
		{ 0x10FC0, 0, -1 },
		{ 0x11310, 1, -1 },
		{ 0x11A20, 1, -1 },
		{ 0x12380, 0, -1 },
		{ 0x12540, 1, -1 },
		{ 0x12730, 1, -1 },
		{ 0x12A90, 1, 11 },
		{ 0x134D0, 0, -1 },
		{ 0x00000, 0, -1 },
		{ 0x13770, 0, -1 },
		{ 0x00000, 0, -1 },
		{ 0x00000, 0, -1 },
		{ 0x00000, 0, -1 },
		{ 0x00000, 0, -1 },
		{ 0x14710, 1, 12 },
		{ 0x15DF0, 1, 13 },
		{ 0x16030, 1, 14 },
		{ 0x17030, 0, -1 },
		{ 0x17650, 0, -1 },
		{ 0x134D0, 0, -1 },
		{ 0x178E0, 1, -1 },
		{ 0x18200, 0, -1 },
		{ 0x18320, 0, -1 },
		{ 0x184A0, 0, -1 },
		{ 0x18BB0, 0, -1 },
		{ 0x19040, 0, 19 },
		{ 0x19B50, 0, 20 },
		{ 0x17650, 0, -1 },
		{ 0x1A730, 1, 21 },
		{ 0x00000, 0, -1 },
		{ 0x12380, 0, -1 },
		{ 0x1B810, 0, -1 },
		{ 0x1BA50, 0, 15 },
		{ 0x1C190, 0, 16 },
		{ 0x1CA50, 0, 17 },
		{ 0x1D100, 0, 18 },
	};

	int trackNum = tTable[track].track;
	bool loop = tTable[track].loop;
	// could be that if the trackNum is -1, the music should be stopped
	// instead of letting the old music play on
	if (trackNum == -1 || trackNum == _lastTrack)
		return;

	haltTrack();
	AudioCD.play(trackNum+1, loop ? -1 : 1, 0, 0);
	AudioCD.updateCD();

	_lastTrack = trackNum;
}

void SoundTowns::haltTrack() {
	_lastTrack = -1;
	AudioCD.stop();
	AudioCD.updateCD();
}

void SoundTowns::loadSoundFile(uint file) {
	if (_sfxFileIndex == file)
		return;
	_sfxFileIndex = file;
	delete [] _sfxFileData;
	_sfxFileData = _engine->resource()->fileData(soundFilename(file), 0);
}

void SoundTowns::stopSoundEffect() {
	_sfxIsPlaying = false;	
	_mixer->stopHandle(_sfxHandle);
}

void SoundTowns::playSoundEffect(uint8 track) {
	if (!_sfxEnabled || !_sfxFileData)
		return;

	_sfxIsPlaying = true;
	
	uint8 pitch = 0x3c;
	if (_sfxFileIndex == 5) {		
		if (track == 0x10) {
			pitch = 0x3e;
			track = 0x0f;
		} else if (track == 0x11) {
			pitch = 0x40;
			track = 0x0f;
		} else if (track == 0x12) {
			pitch = 0x41;
			track = 0x0f;
		}
	}

	uint8 * fileBody = _sfxFileData + 0x01b8;
	int32 offset = (int32)READ_LE_UINT32(_sfxFileData + (track - 0x0b) * 4);
	if (offset == -1)
		return;

	struct SfxHeader {
		uint32 id;
		uint32 inBufferSize;
		uint32 unused1;
		uint32 outBufferSize;
		uint32 unused2;
		uint32 unused3;
		uint32 unknown1;
		uint32 pitch;
	} *sfxHeader = (SfxHeader*)(fileBody + offset);

	uint32 sfxHeaderID = TO_LE_32(sfxHeader->id);
	uint32 sfxHeaderInBufferSize = TO_LE_32(sfxHeader->inBufferSize);
	uint32 sfxHeaderOutBufferSize = TO_LE_32(sfxHeader->outBufferSize);
	sfxHeader->pitch = TO_LE_32(sfxHeader->pitch);

	uint32 playbackBufferSize = (sfxHeaderID == 1) ? sfxHeaderInBufferSize : sfxHeaderOutBufferSize;

	stopSoundEffect();
	uint8 *sfxPlaybackBuffer = (uint8 *)malloc(playbackBufferSize);
	memset(sfxPlaybackBuffer, 0x80, playbackBufferSize);

	uint8 *sfxBody = ((uint8 *)sfxHeader) + 0x20;

	if (!sfxHeaderID) {
		memcpy(sfxPlaybackBuffer, sfxBody, playbackBufferSize);
	} else if (sfxHeaderID == 1) {
		Screen::decodeFrame4(sfxBody, sfxPlaybackBuffer, playbackBufferSize);
	} else if (_sfxWDTable) {
		uint8 * tgt = sfxPlaybackBuffer;
		uint32 sfx_BtTable_Offset = 0;
		uint32 sfx_WdTable_Offset = 0;
		uint32 sfx_WdTable_Number = 5;
		
		for (uint32 i = 0; i < sfxHeaderInBufferSize; i++) {
			sfx_WdTable_Offset = (sfx_WdTable_Number * 3 << 9) + sfxBody[i] * 6;
			sfx_WdTable_Number = READ_LE_UINT16(_sfxWDTable + sfx_WdTable_Offset);

			sfx_BtTable_Offset += (int16)READ_LE_UINT16(_sfxWDTable + sfx_WdTable_Offset + 2);
			*tgt++ = _sfxBTTable[((sfx_BtTable_Offset >> 2) & 0xff)];

			sfx_BtTable_Offset += (int16)READ_LE_UINT16(_sfxWDTable + sfx_WdTable_Offset + 4);
			*tgt++ = _sfxBTTable[((sfx_BtTable_Offset >> 2) & 0xff)];
		}
	}

	for (uint32 i = 0; i < playbackBufferSize; i++) {		
		if (sfxPlaybackBuffer[i] < 0x80)
			sfxPlaybackBuffer[i] = 0x80 - sfxPlaybackBuffer[i];
	}

	playbackBufferSize -= 0x20;
	setPitch(sfxPlaybackBuffer, playbackBufferSize, sfxHeader->pitch, pitch);
	
	_currentSFX = Audio::makeLinearInputStream(sfxPlaybackBuffer, playbackBufferSize, 
		0x2b11,
		Audio::Mixer::FLAG_UNSIGNED | Audio::Mixer::FLAG_LITTLE_ENDIAN | Audio::Mixer::FLAG_AUTOFREE,
		0, 0);
	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, _currentSFX);
}

void SoundTowns::setPitch(uint8 *&data, uint32 &size, int8 sourcePitch, int8 targetPitch) {
	if (sourcePitch == targetPitch)
		return;

	if (sourcePitch < 0)
		sourcePitch = 0;
	if (sourcePitch > 119)
		sourcePitch = 119;
	if (targetPitch < 0)
		targetPitch = 0;
	if (targetPitch > 119)
		targetPitch = 119;
	
	static const float noteFrq[] = {
		0004.13f, 0004.40f, 0004.64f, 0004.95f, 0005.16f, 0005.50f, 0005.80f, 0006.19f, 0006.60f, 0006.86f,
		0007.43f, 0007.73f, 0008.25f, 0008.80f, 0009.28f, 0009.90f, 0010.31f, 0011.00f, 0011.60f, 0012.38f,
		0013.20f, 0013.75f, 0014.85f, 0015.47f,	0016.50f, 0017.60f, 0018.56f, 0019.80f, 0020.63f, 0022.00f,
		0023.21f, 0024.75f, 0026.40f, 0027.50f, 0029.70f, 0030.94f, 0033.00f, 0035.20f, 0037.16f, 0039.60f,
		0041.25f, 0044.00f, 0046.41f, 0049.50f, 0052.80f, 0055.00f, 0059.40f, 0061.88f, 0066.00f, 0070.40f,
		0074.25f, 0079.20f, 0082.50f, 0088.00f, 0092.83f, 0099.00f, 0105.60f, 0110.00f, 0118.80f, 0123.75f,
		0132.00f, 0140.80f, 0148.50f, 0158.40f, 0165.00f, 0176.00f, 0185.65f, 0198.00f, 0211.20f, 0220.00f,
		0237.60f, 0247.50f, 0264.00f, 0281.60f, 0297.00f, 0316.80f, 0330.00f, 0352.00f, 0371.30f, 0396.00f,
		0422.40f, 0440.00f, 0475.20f, 0495.00f,	0528.00f, 0563.20f, 0594.00f, 0633.60f, 0660.00f, 0704.00f,
		0742.60f, 0792.00f, 0844.80f, 0880.00f, 0950.40f, 0990.00f, 1056.00f, 1126.40f, 1188.00f, 1267.20f,
		1320.00f, 1408.00f, 1485.20f, 1584.00f, 1689.60f, 1760.00f, 1900.80f, 1980.00f, 2112.00f, 2252.80f,
		2376.00f, 2534.40f, 2640.00f, 2816.00f, 2970.40f, 3168.00f, 3379.20f, 3520.00f, 3801.60f, 3960.00f
	};

	const float inc = noteFrq[targetPitch] / noteFrq[sourcePitch];

	uint32 estimatedSize = (uint32)(((float) size / inc) + 1);
	uint32 exactSize = 0;
	uint8 * tmp = new uint8[estimatedSize];
	memset(tmp, 0x80, estimatedSize);

	int last = 0;
	for (float i = 0; i < size; i += inc) {
		int cur = (int) i;
		if (cur == last + 2)
			tmp[exactSize++] = (data[last] + data[cur - 1] + data[cur]) / 3;
		else if (cur == last)
			tmp[exactSize++] = (data[cur] + data[cur + 1]) / 2;
		else
			tmp[exactSize++] = data[cur];
		last = (int) i;
	}

    size = MIN(exactSize, estimatedSize);
	delete[] data;
	data = tmp;
}

const uint8 SoundTowns::_sfxBTTable[256] = {
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFE,
	0x7F, 0x7F, 0x7E, 0x7D, 0x7C, 0x7B, 0x7A, 0x79, 0x78, 0x77, 0x76, 0x75, 0x74, 0x73, 0x72, 0x71,
	0x70, 0x6F, 0x6E, 0x6D, 0x6C, 0x6B, 0x6A, 0x69, 0x68, 0x67, 0x66, 0x65, 0x64, 0x63, 0x62, 0x61,
	0x60, 0x5F, 0x5E, 0x5D, 0x5C, 0x5B, 0x5A, 0x59, 0x58, 0x57, 0x56, 0x55, 0x54, 0x53, 0x52, 0x51,
	0x50, 0x4F, 0x4E, 0x4D, 0x4C, 0x4B, 0x4A, 0x49, 0x48, 0x47, 0x46, 0x45, 0x44, 0x43, 0x42, 0x41,
	0x40, 0x3F, 0x3E, 0x3D, 0x3C, 0x3B, 0x3A, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31,
	0x30, 0x2F, 0x2E, 0x2D, 0x2C, 0x2B, 0x2A, 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21,
	0x20, 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11,
	0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01
};

} // end of namespace Kyra
