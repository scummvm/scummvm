/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "autoroute.h"

#define ROUTE_GRID_WIDTH ((GAME_SCREEN_WIDTH/8)+2)
#define ROUTE_GRID_HEIGHT ((GAME_SCREEN_HEIGHT/8)+2)
#define ROUTE_GRID_SIZE (ROUTE_GRID_WIDTH*ROUTE_GRID_HEIGHT*2)
#define WALK_JUMP 8      // walk in blocks of 8

const int16 SkyAutoRoute::_routeDirections[4] = {    -1,     1, -ROUTE_GRID_WIDTH, ROUTE_GRID_WIDTH };
const uint16 SkyAutoRoute::_logicCommands[4] = { RIGHTY, LEFTY,             DOWNY,              UPY };

SkyAutoRoute::SkyAutoRoute(SkyGrid *pGrid) {

	_grid = pGrid;
	_routeGrid = (uint16 *)malloc(ROUTE_GRID_SIZE);
	_routeBuf = (uint16 *)malloc(ROUTE_SPACE);
}

SkyAutoRoute::~SkyAutoRoute(void) {

	free(_routeGrid);
	free(_routeBuf);
}

uint16 SkyAutoRoute::checkBlock(uint16 *blockPos) {

	uint16 retVal = 0xFFFF;

	for (uint8 cnt = 0; cnt < 4; cnt++) {
		uint16 fieldVal = *(blockPos + _routeDirections[cnt]);
		if (fieldVal && (fieldVal < retVal))
			retVal = fieldVal;
	}
	return retVal;
}

void SkyAutoRoute::clipCoordX(uint16 x, uint8 &blkX, int16 &initX) {
	if (x < TOP_LEFT_X) {
		blkX = 0;
		initX = x - TOP_LEFT_X;
	} else if (x >= TOP_LEFT_X + GAME_SCREEN_WIDTH) {
		blkX = (GAME_SCREEN_WIDTH - 1) >> 3;
		initX = x - (TOP_LEFT_X + GAME_SCREEN_WIDTH);
	} else {
		blkX = (x - TOP_LEFT_X) >> 3;
		initX = 0;
	}
}

void SkyAutoRoute::clipCoordY(uint16 y, uint8 &blkY, int16 &initY) {
	if (y < TOP_LEFT_Y) {
		blkY = 0;
		initY = y - TOP_LEFT_Y;
	} else if (y >= TOP_LEFT_Y + GAME_SCREEN_HEIGHT) {
		blkY = (GAME_SCREEN_HEIGHT - 1) >> 3;
		initY = y - (TOP_LEFT_Y + GAME_SCREEN_WIDTH);
	} else {
		blkY = (y - TOP_LEFT_Y) >> 3;
		initY = 0;
	}
}

void SkyAutoRoute::initWalkGrid(uint8 screen, uint8 width) {

	uint16 *wGridPos;
	uint8 stretch = 0;
	uint8 *screenGrid = _grid->giveGrid(screen);
	screenGrid += GRID_SIZE;
	wGridPos = _routeGrid + (ROUTE_GRID_SIZE >> 1) - ROUTE_GRID_WIDTH - 2;

    memset(_routeGrid, 0, ROUTE_GRID_SIZE);
	uint8 bitsLeft = 0; uint32 gridData = 0;
	for (uint8 gridCntY = 0; gridCntY < ROUTE_GRID_HEIGHT - 2; gridCntY++) {
		for (uint8 gridCntX = 0; gridCntX < ROUTE_GRID_WIDTH - 2; gridCntX++) {
			if (!bitsLeft) {
				screenGrid -= 4;
				gridData = READ_LE_UINT32(screenGrid);
				bitsLeft = 32;
			}
			if (gridData & 1) {
				*wGridPos = 0xFFFF; // block is not accessible
				stretch = width;
			} else if (stretch) {
				*wGridPos = 0xFFFF;
				stretch--;
			}
			wGridPos--;
			bitsLeft--;
			gridData >>= 1;
		}
		wGridPos -= 2;
		stretch = 0;
	}
}

bool SkyAutoRoute::calcWalkGrid(uint8 startX, uint8 startY, uint8 destX, uint8 destY) {

	int16 directionX, directionY;
	uint8 roiX, roiY; // Rectangle Of Interest in the walk grid
	if (startY > destY) {
		directionY = -ROUTE_GRID_WIDTH;
		roiY = startY;
	} else {
		directionY = ROUTE_GRID_WIDTH;
		roiY = (ROUTE_GRID_HEIGHT-1) - startY;
	}
	if (startX > destX) {
		directionX = -1;
		roiX = startX + 2;
	} else {
		directionX = 1;
		roiX = (ROUTE_GRID_WIDTH - 1) - startX;
	}

	uint16 *walkDest  = _routeGrid + (destY + 1) * ROUTE_GRID_WIDTH + destX + 1;
	uint16 *walkStart = _routeGrid + (startY + 1) * ROUTE_GRID_WIDTH + startX + 1;
	*walkStart = 1;

	// if we are on the edge, move diagonally from start
	if (roiY < ROUTE_GRID_HEIGHT-3)
		walkStart -= directionY;

	if (roiX < ROUTE_GRID_WIDTH-2)
		walkStart -= directionX;
    
	bool gridChanged = true;
	bool foundRoute = false;

	while ((!foundRoute) && gridChanged) {
		gridChanged = false;
		uint16 *yWalkCalc = walkStart;
		for (uint8 cnty = 0; cnty < roiY; cnty++) {
			uint16 *xWalkCalc = yWalkCalc;
			for (uint8 cntx = 0; cntx < roiX; cntx++) {
				if (!*xWalkCalc) { // block wasn't done, yet
					uint16 blockRet = checkBlock(xWalkCalc);
					if (blockRet < 0xFFFF) {
						*xWalkCalc = blockRet + 1;
						gridChanged = true;
					}
				}
                xWalkCalc += directionX;
			}
			yWalkCalc += directionY;
		}
		if (*walkDest) { // okay, finished
			foundRoute = true;
		} else { // we couldn't find the route, let's extend the ROI
			if (roiY < ROUTE_GRID_HEIGHT - 4) {
				walkStart -= directionY;
				roiY++;
			}
			if (roiX < ROUTE_GRID_WIDTH - 4) {
				walkStart -= directionX;
				roiX++;
			}
		}
	}
	return foundRoute;
}

uint16 *SkyAutoRoute::makeRouteData(uint8 startX, uint8 startY, uint8 destX, uint8 destY) {

	memset(_routeBuf, 0, ROUTE_SPACE);

	uint16 *routePos = _routeGrid + (destY + 1) * ROUTE_GRID_WIDTH + destX + 1;
	uint16 *dataTrg = _routeBuf + (ROUTE_SPACE >> 1) - 2;

	uint16 lastVal = (*routePos) - 1;
	while (lastVal) { // lastVal == 0 means route is done.
		dataTrg -= 2;
		
		int16 walkDirection = 0;
		for (uint8 cnt = 0; cnt < 4; cnt++)
			if (lastVal == *(routePos + _routeDirections[cnt])) {
				*(dataTrg + 1) = _logicCommands[cnt];
				walkDirection = _routeDirections[cnt];
				break;
			}

		if (!walkDirection)
			error("makeRouteData:: can't find way through walkGrid (pos %d)", lastVal);
		while (lastVal && (lastVal == *(routePos + walkDirection))) {
            *dataTrg += WALK_JUMP;
			lastVal--;
			routePos += walkDirection;
		}
	}
    return dataTrg;
}

uint16 *SkyAutoRoute::checkInitMove(uint16 *data, int16 initStaX) {
	if (initStaX < 0) {
		data -= 2;
		*(data + 1) = RIGHTY;
		*data = ((-initStaX) + 7) & 0xFFF8;
	} else if (initStaX > 0) {
		data -= 2;
		*(data + 1) = LEFTY;
		*data = (initStaX + 7) & 0xFFF8;
	}
	return data;
}

uint16 SkyAutoRoute::autoRoute(Compact *cpt) {

	uint8 cptScreen = (uint8)cpt->screen;
	uint8 cptWidth = (uint8)SkyCompact::getMegaSet(cpt, cpt->extCompact->megaSet)->gridWidth;
	initWalkGrid(cptScreen, cptWidth);

	uint8 startX, startY, destX, destY;
	int16 initStaX, initStaY, initDestX, initDestY;

	clipCoordX(cpt->xcood, startX, initStaX);
	clipCoordY(cpt->ycood, startY, initStaY);
	clipCoordX(cpt->extCompact->arTargetX, destX, initDestX);
	clipCoordY(cpt->extCompact->arTargetY, destY, initDestY);

	memset(cpt->extCompact->animScratch, 0, 32);
	if ((startX == destX) && (startY == destY))
		return 2;

	if (_routeGrid[(destY + 1) * ROUTE_GRID_WIDTH + destX + 1])
		return 1; // AR destination is an unaccessible block

	if (!calcWalkGrid(startX, startY, destX, destY))
		return 1; // can't find route to block

	uint16 *routeData = makeRouteData(startX, startY, destX, destY);
	// the route is done.
	// if there was an initial x movement (due to clipping) tag it onto the start
	routeData = checkInitMove(routeData, initStaX);

    uint8 cnt = 0;
	do {
		((uint16*)cpt->extCompact->animScratch)[cnt]     = routeData[cnt];
		((uint16*)cpt->extCompact->animScratch)[cnt + 1] = routeData[cnt + 1];
		cnt += 2;
	} while (routeData[cnt - 2]);
	return 0;
}

