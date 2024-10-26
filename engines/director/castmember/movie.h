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

#ifndef DIRECTOR_CASTMEMBER_MOVIE_H
#define DIRECTOR_CASTMEMBER_MOVIE_H

#include "director/castmember/castmember.h"

namespace Director {

class MovieCastMember : public CastMember {
public:
	MovieCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);
	MovieCastMember(Cast *cast, uint16 castId, MovieCastMember &source);

	bool hasField(int field) override;
	Datum getField(int field) override;
	bool setField(int field, const Datum &value) override;

	Common::String formatInfo() override;

	uint32 _flags;
	bool _looping;
	bool _enableScripts;
	bool _enableSound;
	bool _crop;
	bool _center;
};

} // End of namespace Director

#endif
