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

#ifndef TWINE_PARSER_ANIM3DS_H
#define TWINE_PARSER_ANIM3DS_H

#include "twine/parser/parser.h"
#include "twine/shared.h"

namespace TwinE {

struct T_ANIM_3DS {
	char Name[4]; // Name of the animation
	int16 Deb;    // Start frame in the HQR
	int16 Fin;    // End frame in the HQR
};

class Anim3DSData : public Parser {
private:
	Common::Array<T_ANIM_3DS> _anims; // ListAnim3DS

public:
	bool loadFromStream(Common::SeekableReadStream &stream, bool lba1) override;

	const Common::Array<T_ANIM_3DS> &getAnims() const { return _anims; }
};

} // End of namespace TwinE

#endif
