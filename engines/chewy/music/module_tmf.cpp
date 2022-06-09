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

#include "chewy/music/module_tmf.h"

#include "common/array.h"
#include "common/stream.h"

const uint8 Chewy::Module_TMF::TMF_MOD_SONG_NAME[] = {
	'S', 'C', 'U', 'M', 'M',
	'V', 'M', ' ', 'M', 'O',
	'D', 'U', 'L', 'E', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0;'
};
const uint8 Chewy::Module_TMF::TMF_MOD_INSTRUMENT_NAME[] = {
	'S', 'C', 'U', 'M', 'M',
	'V', 'M', ' ', 'I', 'N',
	'S', 'T', 'R', 'U', 'M',
	'E', 'N', 'T', ' ', '\0',
	'\0', '\0', '\0'
};
const uint16 Chewy::Module_TMF::TMF_MOD_PERIODS[] = {
	856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
	428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
	214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113
};

bool Chewy::Module_TMF::load(Common::SeekableReadStream& stream, int offs) {
	stream.seek(offs);

	// Check TMF fourCC.
	if (stream.readUint32BE() != MKTAG('T', 'M', 'F', '\0'))
		error("Corrupt TMF resource");

	// Write song name (not present in TMF data).
	Common::copy(TMF_MOD_SONG_NAME, TMF_MOD_SONG_NAME + 21, songname);

	// Copy instrument data.
	uint8 fineTune, instVolume;
	uint32 repeatPoint, repeatLength, sampleLength;
	uint32 totalSampleLength = 0;

	for (int i = 0; i < NUM_SAMPLES; ++i) {
		fineTune = stream.readByte();
		instVolume = stream.readByte();
		// Repeat point, repeat length and sample length are 32 bit LE in bytes
		// instead of 16 bit BE in words.
		repeatPoint = stream.readUint32LE();
		assert(repeatPoint <= 0x1FFFF && repeatPoint % 2 == 0);
		repeatLength = stream.readUint32LE();
		assert(repeatLength <= 0x1FFFF && repeatLength % 2 == 0);
		// Sample length is at the end instead of at the start.
		sampleLength = stream.readUint32LE();
		assert(sampleLength <= 0x1FFFF && sampleLength % 2 == 0);
		totalSampleLength += sampleLength;

		// Instrument name is not present in TMF data.
		Common::copy(TMF_MOD_INSTRUMENT_NAME, TMF_MOD_INSTRUMENT_NAME + 23, sample[i].name);
		sample[i].name[19] = '0' + i / 10;
		sample[i].name[20] = '0' + i % 10;

		sample[i].len = sampleLength;
		// Finetune is a signed nibble in MOD, but TMF uses a signed byte
		// (within nibble range).
		sample[i].finetune = fineTune & 0x0F;
		sample[i].vol = instVolume;
		sample[i].repeat = repeatPoint;
		sample[i].replen = repeatLength;
	}

	// Copy pattern table.
	songlen = stream.readByte();
	// Second byte is the number of different patterns in TMF. This byte is
	// unused in MOD (usually set to 0x7F).
	uint8 numPatterns = stream.readByte();
	undef = 0x7F;
	stream.read(songpos, 128);
	// M.K. fourCC is not present in TMF.
	sig = signatures[0];

	// TMF has a 32 bit LE number for each instrument here; these are probably
	// offsets for each sample. They are not present in MOD and not needed, so
	// they are skipped.
	stream.skip(4 * 31);

	// Copy pattern data.
	pattern = new Modules::pattern_t[numPatterns];
	for (int i = 0; i < numPatterns; ++i) {
		for (int j = 0; j < 64; ++j) {
			for (int k = 0; k < 4; ++k) {
				// TMF channel data has this format:
				// 1 byte note (0-0x23 or 0x30 for "use previous value")
				// 1 byte sample
				// 2 bytes effect (byte 3 high nibble is unused)
				byte note = stream.readByte();
				assert(note == 0x30 || note < 36);
				byte sampleNum = stream.readByte();
				uint16 effect = stream.readUint16BE();
				assert((effect & 0xF000) == 0);
				// Note is converted to a MOD 12 bit period using a lookup array.
				uint16 period = note == 0x30 ? 0 : TMF_MOD_PERIODS[note];
				pattern[i][j][k].sample = sampleNum;
				pattern[i][j][k].period = period;
				pattern[i][j][k].effect = effect;
				pattern[i][j][k].note = periodToNote(period);
			}
		}
	}

	// Copy sample data.
	for (int i = 0; i < NUM_SAMPLES; ++i) {
		if (!sample[i].len) {
			sample[i].data = nullptr;
		} else {
			sample[i].data = new int8[sample[i].len];
			for (int j = 0; j < sample[i].len; j++) {
				int tmfSample = stream.readByte();
				// Convert from unsigned to signed.
				tmfSample -= 0x80;
				sample[i].data[j] = tmfSample & 0xFF;
			}
		}
	}

	return true;
}
