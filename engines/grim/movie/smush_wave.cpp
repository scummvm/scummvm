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

#include "common/scummsys.h"
#include "common/endian.h"

#include "engines/grim/movie/codecs/vima.h"
#include "engines/grim/movie/smush_sound.h"
#include "engines/grim/movie/smush_wave.h"

namespace Grim {

SmushWave::SmushWave(SmushSound *sound) {
	_sound = sound;
	_packetBuffer = nullptr;
	_trackId = -1;
	_chunkCounter = 0;
	_totalSize = 0;
	memset(_vimaTable, 0, sizeof(_vimaTable));
	_vimaTableBuilt = false;
}

SmushWave::~SmushWave() {
	moduleShutdown();
}

int SmushWave::moduleStartup() {
	_packetBuffer = (byte *)calloc(SMUSH_WAVE_PACKET_SIZE, 1);
	return (_packetBuffer != nullptr) - 1;
}

int SmushWave::moduleShutdown() {
	free(_packetBuffer);
	_packetBuffer = nullptr;
	return 0;
}

void SmushWave::flobdecReset() {
	_chunkCounter = 0;
}

void SmushWave::flobdecDecodeHeader(const byte *header) {
	_chunkCounter = 0;
	_totalSize = READ_LE_UINT32(header + 8);
}

void SmushWave::flobdecDecodeBody(const byte *body) {
	const byte *data = body + 4;
	int sampleCount = READ_BE_INT32(body);
	if (sampleCount < 0) {
		int trackId = READ_BE_INT32(body + 4);
		if (trackId != _trackId) {
			_trackId = trackId;
			_chunkCounter = 0;
		}

		sampleCount = READ_BE_INT32(body + 8);
		data = body + 12;
	}

	int channelCount = (data[0] & 0x80) ? 2 : 1;

	if (_sound) {
		if (!_vimaTableBuilt) {
			vimaInit(_vimaTable);
			_vimaTableBuilt = true;
		}

		byte *packet = _packetBuffer;
		int pcmSize = 2 * channelCount * sampleCount;

		if (_chunkCounter) {
			memcpy(packet, "PSAD", 4);
			WRITE_BE_UINT32(packet + 4, pcmSize + 10);
			WRITE_LE_UINT16(packet + 8, 100);
			WRITE_LE_UINT16(packet + 10, _chunkCounter);
			WRITE_LE_UINT16(packet + 12, 100);
			packet[16] = 127;
			packet[17] = 0;

			decompressVima(data, (int16 *)(packet + 18), pcmSize, _vimaTable, true);
		} else {
			memcpy(packet, "PSAD", 4);
			WRITE_BE_UINT32(packet + 4, pcmSize + 46);
			WRITE_LE_UINT16(packet + 8, 100);
			WRITE_LE_UINT16(packet + 10, _chunkCounter);
			WRITE_LE_UINT16(packet + 12, 100);
			packet[16] = 127;
			packet[17] = 0;
			WRITE_BE_UINT32(packet + 22, _totalSize + 36);
			WRITE_LE_UINT32(packet + 30, 201326592);
			packet[34] = 1;
			packet[35] = 8;
			WRITE_LE_UINT32(packet + 36, 0);
			WRITE_BE_UINT32(packet + 40, _totalSize);
			packet[44] = 5;
			packet[45] = 0;
			memcpy(packet + 46, (channelCount == 1) ? "S6MZ" : "S6SZ", 4);
			WRITE_BE_UINT32(packet + 50, _totalSize);

			decompressVima(data, (int16 *)(packet + 54), pcmSize, _vimaTable, true);
		}

		_sound->souEngineStart(packet, 127, 0, 64);
		_chunkCounter++;
	}
}

} // End of namespace Grim
