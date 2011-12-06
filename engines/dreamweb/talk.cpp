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

#include "dreamweb/dreamweb.h"

namespace DreamGen {

uint16 DreamGenContext::getPersFrame(uint8 index) {
	return getSegment(data.word(kPeople)).word(kPersonframes + index * 2);
}

void DreamGenContext::convIcons() {
	uint8 index = data.byte(kCharacter) & 127;
	data.word(kCurrentframe) = getPersFrame(index);
	Frame *frame = findSource();
	uint16 frameNumber = (data.word(kCurrentframe) - data.word(kTakeoff)) & 0xff;
	showFrame(frame, 234, 2, frameNumber, 0);
}

} // End of namespace DreamGen
