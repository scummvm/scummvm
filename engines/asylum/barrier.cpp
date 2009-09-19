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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "asylum/barrier.h"

namespace Asylum {

Barrier::Barrier() {
	// TODO Auto-generated constructor stub

}

Barrier::~Barrier() {
	// TODO Auto-generated destructor stub
}

uint32 Barrier::getRandomId() {
	int numRes = 0;
	uint32 rndResId[5];
	memset(&rndResId, 0, sizeof(rndResId));
	for (int i = 0; i < 5; i++) {
		if (field_68C[i]) {
			rndResId[numRes] = field_68C[i];
			numRes++;
		}
	}
	if(numRes > 0)
		return rndResId[rand() % numRes];

	return resId;
}

int Barrier::checkFlags() {
	return flags & 1 && (flags & 8 || flags & 0x10000);
}

void Barrier::setNextFrame(int targetFlags) {
	int newFlag = targetFlags | 1 | flags;
	flags |= targetFlags | 1;

	if (newFlag & 0x10000)
		frameIdx = frameCount - 1;
	else
		frameIdx = 0;
}

} // end of namespace Asylum
