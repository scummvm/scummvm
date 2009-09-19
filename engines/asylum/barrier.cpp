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
#include "asylum/shared.h"

namespace Asylum {

Barrier::Barrier() {
	// TODO Auto-generated constructor stub

}

Barrier::~Barrier() {
	// TODO Auto-generated destructor stub
}

bool Barrier::visible() {
	if ((flags & 0xFF) & 1) {
		for (int f = 0; f < 10; f++) {
			bool   isSet = false;
			uint32 flag  = gameFlags[f];

			if (flag <= 0)
				isSet = Shared.isGameFlagNotSet(flag); // -flag
			else
				isSet = Shared.isGameFlagSet(flag);

			if(!isSet)
				return false;
		}
		return true;
	}
	return false;
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

bool Barrier::onscreen() {
	WorldStats *ws = Shared.getScene()->worldstats();
	Common::Rect screenRect  = Common::Rect(ws->xLeft, ws->yTop, ws->xLeft + 640, ws->yTop + 480);
	Common::Rect barrierRect = boundingRect;
	barrierRect.translate(x, y);
	return visible() && (flags & 1) && screenRect.intersects(barrierRect);
}

} // end of namespace Asylum
