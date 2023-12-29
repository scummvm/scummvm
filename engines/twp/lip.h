
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

#ifndef TWP_LIP_H
#define TWP_LIP_H

#include "common/stream.h"
#include "common/array.h"

namespace Twp {

struct LipItem {
	float time;
	char letter;
};

// This contains the lip animation for a specific text.
//
// A lip animation contains a list of moth shape at a specific time.
// You can see https://github.com/DanielSWolf/rhubarb-lip-sync to
// have additional information about the mouth shapes.
class Lip {
public:
	void load(Common::SeekableReadStream* stream);
	// Gets the letter corresponding to a mouth shape at a spcific time.
	char letter(float time);

private:
	Common::Array<LipItem> _items;
};

}

#endif
