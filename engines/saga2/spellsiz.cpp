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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/spelshow.h"

namespace Saga2 {

SPELLHEIGHTFUNCTION(ShortTillThere) {
	if (effectron->stepNo <= effectron->totalSteps) {
		return 8;
	} else if (effectron->stepNo - effectron->totalSteps <= 8) {
		return 8 * (effectron->stepNo - effectron->totalSteps);
	}
	return 0;
}

SPELLBREADTHFUNCTION(ThinTillThere) {
	if (effectron->stepNo <= effectron->totalSteps) {
		return 8;
	} else if (effectron->stepNo - effectron->totalSteps <= 8) {
		return 8 * (effectron->stepNo - effectron->totalSteps);
	}
	return 0;
}



SPELLHEIGHTFUNCTION(GrowLinear) {
	return 0;
}

SPELLBREADTHFUNCTION(BulkLinear) {
	return 0;
}



SPELLBREADTHFUNCTION(StaticHeight) {
	return 1;
}

SPELLBREADTHFUNCTION(StaticBreadth) {
	return 1;
}

} // end of namespace Saga2
