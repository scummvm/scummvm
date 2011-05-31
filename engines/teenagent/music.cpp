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
 */

#include "teenagent/music.h"
#include "teenagent/resources.h"
#include "common/debug.h"
#include "common/ptr.h"
#include "common/textconsole.h"

namespace TeenAgent {

static const uint32 noteToPeriod[3][12] = {
	{855, 807, 761, 720, 678, 640, 604, 569, 537, 508, 480, 453},
	{428, 404, 381, 360, 338, 320, 301, 285, 269, 254, 239, 226},
	{214, 201, 189, 179, 170, 160, 151, 143, 135, 127, 120, 113}
};

MusicPlayer::MusicPlayer() : Paula(false, 44100, 5000), _id(0) {
}

MusicPlayer::~MusicPlayer() {
}

bool MusicPlayer::load(int id) {
	Resources *res = Resources::instance();

	Common::ScopedPtr<Common::SeekableReadStream> stream(res->mmm.getStream(id));
	if (!stream)
		return false;

	char header[4];
	stream->read(header, 4);
	//check header?

	// Load the samples
	sampleCount = stream->readByte();

	debug(0, "sampleCount = %d", sampleCount);

	for (byte currSample = 0; currSample < sampleCount; currSample++) {
		byte sample = stream->readByte();

		// Load the sample data
		byte sampleResource = ((sample >> 4) & 0x0F) * 10 + (sample & 0x0F);
		debug(0, "currSample = %d, sample = 0x%02x, resource: %d", currSample, sample, sampleResource);
		uint32 sampleSize = res->sam_mmm.getSize(sampleResource);
		if (sampleSize == 0) {
			warning("load: invalid sample %d (0x%02x)", sample, sample);
			_samples[sample].clear();
			continue;
		}

		_samples[sample].resize(sampleSize);
		res->sam_mmm.read(sampleResource, _samples[sample].data, sampleSize);
	}

	// Load the music data
	_rows.clear();

	Row row;
	while (!stream->eos()) {

		byte cmd = stream->readByte();

		if (cmd < 0x40) {
			row.channels[0].note = cmd;
			row.channels[1].note = stream->readByte();
			row.channels[2].note = stream->readByte();
			_rows.push_back(row);
		} else if ((cmd & 0xF0) == 0x50) {
			byte sample = stream->readByte();
			//debug(1, "%02x: set sample %02x", cmd, sample);
			row.channels[(cmd & 0x0F) - 1].sample = sample;
		} else if ((cmd & 0xF0) == 0x40) {
			byte vol = stream->readByte();
			//debug(1, "%02x: set volume %02x -> %02x", cmd, row.channels[(cmd & 0x0F) - 1].volume, vol);
			//channel volume 0x40 * music volume 0x40 mixed with high bytes
			row.channels[(cmd & 0x0F) - 1].volume = vol * 16;
		} else {
			debug(0, "unhandled music command %02x", cmd);
		}
	}
	_currRow = 0;
	_id = id;
	return true;
}

void MusicPlayer::start() {
	_currRow = 0;
	startPaula();
}

void MusicPlayer::stop() {
	stopPaula();
}

void MusicPlayer::interrupt() {
	if (_rows.empty())
		return;

	_currRow %= _rows.size();

	Row *row = &_rows[_currRow];
	for (int chn = 0; chn < 3; ++chn) {
		setChannelVolume(chn, row->channels[chn].volume);

		//debug(0, "row->channels[%d].volume = %d", chn, row->channels[chn].volume);

		byte sample = (row->channels[chn].sample);
		if (row->channels[chn].note != 0 && sample != 0) {

			//debug(0, "row->channels[%d].note = %d", chn, row->channels[chn].note);
			//debug(0, "row->channels[%d].sample = %d", chn, row->channels[chn].sample);

			byte note = row->channels[chn].note;
			if (_samples[sample].size == 0) {
				warning("interrupt: invalid sample %u (0x%02x)", sample, sample);
				continue;
			}

			setChannelData(chn, (const int8*)_samples[sample].data, NULL, _samples[sample].size, 0);
			setChannelPeriod(chn, noteToPeriod[((note >> 4) & 0x0F) - 1][(note & 0x0F)]);
		}
	}

	//debug(0, "------------------------------------------------");

	++_currRow;
}

} // End of namespace TeenAgent
