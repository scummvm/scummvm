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

#ifndef DIRECTOR_CASTMEMBER_SOUND_H
#define DIRECTOR_CASTMEMBER_SOUND_H

#include "director/castmember/castmember.h"

namespace Director {

class AudioDecoder;

class SoundCastMember : public CastMember {
public:
	SoundCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);
	SoundCastMember(Cast *cast, uint16 castId, SoundCastMember &source);
	~SoundCastMember();

	CastMember *duplicate(Cast *cast, uint16 castId) override { return (CastMember *)(new SoundCastMember(cast, castId, *this)); }

	void load() override;
	void unload() override;
	Common::String formatInfo() override;

	bool hasField(int field) override;
	Datum getField(int field) override;
	bool setField(int field, const Datum &value) override;

	uint32 getCastDataSize() override;
	void writeCastData(Common::MemoryWriteStream *writeStream) override;

	bool _looping;
	AudioDecoder *_audio;
};

} // End of namespace Director

#endif
