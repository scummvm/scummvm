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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "allfiles.h"
#include "objtypes.h"
#include "region.h"
#include "newfatal.h"
#include "sludger.h"
#include "moreio.h"
#include "backdrop.h"

namespace Sludge {

screenRegion *allScreenRegions = NULL;
screenRegion *overRegion = NULL;
extern inputType input;
extern int cameraX, cameraY;

void showBoxes() {
	screenRegion *huntRegion = allScreenRegions;

	while (huntRegion) {
		drawVerticalLine(huntRegion->x1, huntRegion->y1, huntRegion->y2);
		drawVerticalLine(huntRegion->x2, huntRegion->y1, huntRegion->y2);
		drawHorizontalLine(huntRegion->x1, huntRegion->y1, huntRegion->x2);
		drawHorizontalLine(huntRegion->x1, huntRegion->y2, huntRegion->x2);
		huntRegion = huntRegion->next;
	}
}

void removeScreenRegion(int objectNum) {
	screenRegion * * huntRegion = & allScreenRegions;
	screenRegion *killMe;

	while (* huntRegion) {
		if ((* huntRegion)->thisType->objectNum == objectNum) {
			killMe = * huntRegion;
			* huntRegion = killMe->next;
			removeObjectType(killMe->thisType);
			if (killMe == overRegion) overRegion = NULL;
			delete killMe;
			killMe = NULL;
		} else {
			huntRegion = & ((* huntRegion)->next);
		}
	}
}

void saveRegions(Common::WriteStream *stream) {
	int numRegions = 0;
	screenRegion *thisRegion = allScreenRegions;
	while (thisRegion) {
		thisRegion = thisRegion->next;
		numRegions ++;
	}
	put2bytes(numRegions, stream);
	thisRegion = allScreenRegions;
	while (thisRegion) {
		put2bytes(thisRegion->x1, stream);
		put2bytes(thisRegion->y1, stream);
		put2bytes(thisRegion->x2, stream);
		put2bytes(thisRegion->y2, stream);
		put2bytes(thisRegion->sX, stream);
		put2bytes(thisRegion->sY, stream);
		put2bytes(thisRegion->di, stream);
		saveObjectRef(thisRegion->thisType, stream);

		thisRegion = thisRegion->next;
	}
}

void loadRegions(Common::SeekableReadStream *stream) {
	int numRegions = get2bytes(stream);

	screenRegion *newRegion;
	screenRegion * * pointy = & allScreenRegions;

	while (numRegions --) {
		newRegion = new screenRegion;
		* pointy = newRegion;
		pointy = & (newRegion->next);

		newRegion->x1 = get2bytes(stream);
		newRegion->y1 = get2bytes(stream);
		newRegion->x2 = get2bytes(stream);
		newRegion->y2 = get2bytes(stream);
		newRegion->sX = get2bytes(stream);
		newRegion->sY = get2bytes(stream);
		newRegion->di = get2bytes(stream);
		newRegion->thisType = loadObjectRef(stream);
	}
	* pointy = NULL;
}

void killAllRegions() {
	screenRegion *killRegion;
	while (allScreenRegions) {
		killRegion = allScreenRegions;
		allScreenRegions = allScreenRegions->next;
		removeObjectType(killRegion->thisType);
		delete killRegion;
	}
	overRegion = NULL;
}

bool addScreenRegion(int x1, int y1, int x2, int y2, int sX, int sY, int di, int objectNum) {
	screenRegion *newRegion = new screenRegion;
	if (!checkNew(newRegion)) return false;
	newRegion->di = di;
	newRegion->x1 = x1;
	newRegion->y1 = y1;
	newRegion->x2 = x2;
	newRegion->y2 = y2;
	newRegion->sX = sX;
	newRegion->sY = sY;
	newRegion->thisType = loadObjectType(objectNum);
	newRegion->next = allScreenRegions;
	allScreenRegions = newRegion;
	return (bool)(newRegion->thisType != NULL);
}

void getOverRegion() {
	screenRegion *thisRegion = allScreenRegions;
	while (thisRegion) {
		if ((input.mouseX >= thisRegion->x1 - cameraX) && (input.mouseY >= thisRegion->y1 - cameraY) &&
		        (input.mouseX <= thisRegion->x2 - cameraX) && (input.mouseY <= thisRegion->y2 - cameraY)) {
			overRegion = thisRegion;
			return;
		}
		thisRegion = thisRegion->next;
	}
	overRegion = NULL;
	return;
}

screenRegion *getRegionForObject(int obj) {
	screenRegion *thisRegion = allScreenRegions;

	while (thisRegion) {
		if (obj == thisRegion->thisType->objectNum) {
			return thisRegion;
		}
		thisRegion = thisRegion->next;
	}

	return NULL;
}

} // End of namespace Sludge
