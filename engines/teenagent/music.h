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

#ifndef TEEN_MUSIC_H
#define TEEN_MUSIC_H

#include "sound/mods/paula.h"

namespace TeenAgent {

class MusicPlayer : public Audio::Paula {
public:

	MusicPlayer();
	~MusicPlayer();

	bool load(int id);
	int getId() const { return _id; }

	void start();
	void stop();

protected:
	int _id;

	struct Row {
		struct Channel {
			byte sample;
			byte volume;
			byte note;
			Channel(): sample(0), volume(0), note(0) {}
		} channels[3];
	};

	struct {
		const byte *data;
		uint32 size;
	} _samples[256];
	byte sampleCount;

	struct {
		byte volume;
		const byte *data;
		uint32 size;
	} _channels[3];

	Common::Array<Row> _rows;
	uint _currRow;

	void interrupt();
};

} // End of namespace Teen

#endif // TEEN_MUSIC_H
