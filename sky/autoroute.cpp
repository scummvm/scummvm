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

SkyAutoRoute::SkyAutoRoute(SkyGrid *pGrid) {

	_grid = pGrid;
	_routeGrid = (uint16*)malloc(ROUTE_GRID_SIZE);
}

SkyAutoRoute::~SkyAutoRoute(void) {

	free(_routeGrid);
}

uint16 SkyAutoRoute::checkBlock(uint16 *blockPos) {

	uint16 fieldVal, retVal = 0xFFFF;
	fieldVal = blockPos[1]; // field to the right
	if ((!(fieldVal & 0x8000)) && (fieldVal != 0)) retVal = fieldVal;
	fieldVal = (blockPos - 1)[0]; // field to the left
	if ((!(fieldVal & 0x8000)) && (fieldVal != 0)) {
		if ((fieldVal < retVal) || (retVal == 0xFFFF)) retVal = fieldVal;
	}
	fieldVal = (blockPos + ROUTE_GRID_WIDTH)[0]; // upper field
	if ((!(fieldVal & 0x8000)) && (fieldVal != 0)) {
		if ((fieldVal < retVal) || (retVal == 0xFFFF)) retVal = fieldVal;
	}
	fieldVal = (blockPos - ROUTE_GRID_WIDTH)[0]; // upper field
	if ((!(fieldVal & 0x8000)) && (fieldVal != 0)) {
		if ((fieldVal < retVal) || (retVal == 0xFFFF)) retVal = fieldVal;
	}
	return retVal;
}

#undef ARDEBUG

uint16 SkyAutoRoute::autoRoute(Compact *cpt, uint16 **pSaveRoute) {

	if (!cpt->extCompact)
		error("SkyAutoRoute::autoRoute: fatal error. cpt->extCompact == NULL!\n");
	uint16* routeData = (uint16*)cpt->extCompact->animScratch;
	uint8* screenGrid = _grid->giveGrid(cpt->screen);
	screenGrid += GRID_SIZE-4; // all arrays are processed from behind, so make
	// screenGrid point to the last element of our grid.

	uint16 *routeCalc = _routeGrid + (ROUTE_GRID_SIZE >> 1) - ROUTE_GRID_WIDTH - 2;

	uint8 stretch1, stretch2; // bl / bh
	stretch1 = 0;
	MegaSet *mega = SkyCompact::getMegaSet(cpt, cpt->extCompact->megaSet);
	stretch2 = (uint8)(mega->gridWidth & 0xff);

	uint16 cnt;
	//First clear the bottom line and right hand edge of next line
	for (cnt = 0; cnt < ROUTE_GRID_WIDTH + 1; cnt++)
		_routeGrid[(ROUTE_GRID_SIZE >> 1) - 1 - cnt] = 0;

	uint16 gridCntX = ROUTE_GRID_WIDTH - 2; // ch
	uint16 gridCntY = ROUTE_GRID_HEIGHT - 2; // ebp
	uint16 bitsLeft = 32;
	uint32 gridData = screenGrid[0] | (screenGrid[1] << 8) |
		(screenGrid[2] << 16) | (screenGrid[3] << 24);
	screenGrid -= 4;
	do {
		//stretch:
		uint8 shiftBit = (uint8)gridData&1;
		gridData >>= 1;
		if (shiftBit) {
			//bit_set:
			routeCalc[0] = 0xFFFF;
			stretch1 = stretch2; // set up stretch factor
		} else {
			if (stretch1) {
				//still_stretching:
				stretch1--;
				routeCalc[0] = 0xFFFF;
			} else {
				routeCalc[0] = 0; // this block is free
			}
		}
		// next_stretch:
		routeCalc--;
		bitsLeft--;
		if (!bitsLeft) {
			gridData = screenGrid[0] | (screenGrid[1] << 8) |
				(screenGrid[2] << 16) | (screenGrid[3] << 24);
			screenGrid -= 4;
			bitsLeft = 32;
		}
		// still bits:
		gridCntX--;
		if (gridCntX == 0) {
			routeCalc--;
			routeCalc[0] = routeCalc[1] = 0; // do edges
			routeCalc--;
			gridCntX = ROUTE_GRID_WIDTH - 2;
			stretch1 = 0; // clear stretch factor
			gridCntY--;
		}
	} while(gridCntY);
	for (cnt = 0; cnt < ROUTE_GRID_WIDTH - 1; cnt++) 
		_routeGrid[cnt] = 0; // clear top line (right hand edge already done
	
	// the grid has been initialised

	// calculate start and end points
	int16 initX = 0, initY = 0, postX = 0, postY = 0;
	uint8 initBlockY; // bh
	uint8 initBlockX; // bl
	uint8 postBlockY; // ch
	uint8 postBlockX; // cl

	if (cpt->ycood < TOP_LEFT_Y)  {
		initY = cpt->ycood - TOP_LEFT_Y;
		initBlockY = 0;
	} else if (cpt->ycood - TOP_LEFT_Y >= GAME_SCREEN_HEIGHT) { // no_init_y1
		initY = cpt->ycood - TOP_LEFT_Y - GAME_SCREEN_HEIGHT;
		initBlockY = (GAME_SCREEN_HEIGHT - 1) >> 3; // convert to blocks
	} else { // no_init_y2
		initBlockY = (cpt->ycood - TOP_LEFT_Y) >> 3; // convert to blocks
	}
	
	if (cpt->xcood < TOP_LEFT_X) {
		initX = cpt->xcood - TOP_LEFT_X;
		initBlockX = 0;
	} else if (cpt->xcood - TOP_LEFT_X >= GAME_SCREEN_WIDTH) { // no_init_x1
		initX = cpt->xcood - TOP_LEFT_X - (GAME_SCREEN_WIDTH - 1); // -1 to match amiga
		initBlockX = (GAME_SCREEN_WIDTH - 1) >> 3;
	} else { // no_init_x2
		initBlockX = (cpt->xcood - TOP_LEFT_X) >> 3;
	}

	// destination coords:

	if (cpt->extCompact->arTargetY < TOP_LEFT_Y) {
		postY = cpt->extCompact->arTargetY - TOP_LEFT_Y;
		postBlockY = 0;
	} else if (cpt->extCompact->arTargetY - TOP_LEFT_Y >= GAME_SCREEN_HEIGHT) { // no_post_y1
		postY = cpt->extCompact->arTargetY - TOP_LEFT_Y - (GAME_SCREEN_HEIGHT - 1);
		postBlockY = (GAME_SCREEN_HEIGHT - 1) >> 3;
	} else { // no_post_y2
		postBlockY = (cpt->extCompact->arTargetY - TOP_LEFT_Y) >> 3;
	}

	if (cpt->extCompact->arTargetX < TOP_LEFT_X) {
		postX = cpt->extCompact->arTargetX - TOP_LEFT_X;
		postBlockX = 0;
	} else if (cpt->extCompact->arTargetX - TOP_LEFT_X >= GAME_SCREEN_WIDTH) {
		postX = cpt->extCompact->arTargetX - TOP_LEFT_X - (GAME_SCREEN_WIDTH - 1);
		postBlockX = (GAME_SCREEN_WIDTH - 1) >> 3;
	} else {
		postBlockX = (cpt->extCompact->arTargetX - TOP_LEFT_X) >> 3;
	}
	if ((postBlockX == initBlockX) && (postBlockY == initBlockY)) {
		// empty route
		routeData[0] = 0;
		return 1;
	}

	int32 directionX, directionY;
	uint8 numLines, numCols; // number of lines / columns to go
	if (initBlockY > postBlockY) {
		directionY = -ROUTE_GRID_WIDTH;
		numLines = initBlockY;
	} else { // go_down:
		directionY = ROUTE_GRID_WIDTH;
		numLines = (ROUTE_GRID_HEIGHT-1)-initBlockY;
	}
	if (initBlockX > postBlockX) {
		directionX = -1;
		numCols = initBlockX+2;
	} else {
		directionX = 1;
		numCols = (ROUTE_GRID_WIDTH - 1) - initBlockX;
	}
	// calculate destination address
	uint16 *routeDestCalc;
	routeDestCalc = (postBlockY + 1) * ROUTE_GRID_WIDTH + postBlockX + 1 + _routeGrid;

	uint16 *routeSrcCalc;
	routeSrcCalc = (initBlockY + 1) * ROUTE_GRID_WIDTH + initBlockX + 1 + _routeGrid;
	routeSrcCalc[0] = 1; //start this one off
	// means: mark the block we start from as accessible
#ifdef ARDEBUG
	uint16 dcnt1, dcnt2;
	for (dcnt1 = 0; dcnt1 < ROUTE_GRID_HEIGHT; dcnt1++) {
		for (dcnt2 = 0; dcnt2 < ROUTE_GRID_WIDTH; dcnt2++) {
			if (!_routeGrid[dcnt1*ROUTE_GRID_WIDTH + dcnt2]) printf("_"); 
			else if (_routeGrid[dcnt1*ROUTE_GRID_WIDTH + dcnt2] == 1) printf("S");
			else printf("X");
		}
		printf("\n");
	}
	getchar();
#endif

	// if we are on the edge, move diagonally from start
	if (numLines < ROUTE_GRID_HEIGHT-3)
		routeSrcCalc -= directionY;

	if (numCols < ROUTE_GRID_WIDTH-2)
		routeSrcCalc -= directionX;

	if (routeDestCalc[0]) {
		// If destination is a wall then we have no route
		// By the way, we could improve this algorithm by moving as close to the
		// wall as possible. The original pathfinding of SKY sucked, if I remember correctly
		return 2;
	}
	uint8 cnty; // ch
	uint8 cntx; // cl
	// numLines = dh, numCols = dl
	uint16 blockRet;
	bool gridChanged, foundRoute;
	do { // wallow_y
		gridChanged = false;
		cnty = numLines;
		uint16 *yPushedSrc = routeSrcCalc;
		do { // wallow_x
			cntx = numCols;
			uint16 *xPushedSrc = routeSrcCalc;
			do { // wallow
				if (!routeSrcCalc[0]) {
					// block wasn't yet done
					blockRet = checkBlock(routeSrcCalc);
					if (blockRet != 0xFFFF) {
						// this block is accessible
						routeSrcCalc[0] = blockRet+1;
						gridChanged = true;
					}
				}
				routeSrcCalc += directionX;
				cntx--;
			} while (cntx);
			routeSrcCalc = xPushedSrc + directionY;
			cnty--;
		} while (cnty);
		routeSrcCalc = yPushedSrc;

		foundRoute = false;
		if (!routeDestCalc[0]) {
			// we have done a section, see if we want to shift backwards (what?)
			if (numLines < ROUTE_GRID_HEIGHT - 4) {
				routeSrcCalc -= directionY;
				numLines++;
			}
			if (numCols < ROUTE_GRID_WIDTH - 4) {
				routeSrcCalc -= directionX;
				numCols++;
			}
		} else foundRoute = true;			
	} while ((!foundRoute) && gridChanged);
#ifdef ARDEBUG
	for (dcnt1 = 0; dcnt1 < ROUTE_GRID_HEIGHT; dcnt1++) {
		for (dcnt2 = 0; dcnt2 < ROUTE_GRID_WIDTH; dcnt2++) {
            printf(" %02X",_routeGrid[dcnt1*ROUTE_GRID_WIDTH + dcnt2]&0xFF);
		}
		printf("\n");
	}
#endif
	if (!routeDestCalc[0]) {
		// no route exists from routeSrc to routeDest
		return 2;
	}
	// ok, we know now that it's possible to get from the start position to the desired
	// destination. Let's see how.
	uint16 *saveRoute = routeData + (ROUTE_SPACE >> 1) - 1; // route_space is given in bytes so >> 1
	saveRoute[0] = 0; // route is null terminated
	uint16 lastVal;
	lastVal = routeDestCalc[0];
	lastVal--;
	bool routeDone = false;
	do {
		// check_dir:
		if (lastVal == (routeDestCalc-1)[0]) {
			// look_left
			saveRoute -= 2;
			saveRoute[1] = RIGHTY;
			saveRoute[0] = 0;
			while ((lastVal == (routeDestCalc-1)[0]) && (!routeDone)) {
				routeDestCalc--; // keep checking left
				saveRoute[0] += WALK_JUMP;
#ifdef ARDEBUG
				printf("left\n");
#endif
				lastVal--;
				if (lastVal == 0) routeDone = true;
			}
		} else if (lastVal == routeDestCalc[1]) {
			// look_right
			saveRoute -= 2;
			saveRoute[1] = LEFTY;
			saveRoute[0] = 0;
			while ((lastVal == routeDestCalc[1]) && (!routeDone)) {
				routeDestCalc++; // keep checking right
				saveRoute[0] += WALK_JUMP;
#ifdef ARDEBUG
				printf("right\n");
#endif
				lastVal--;
				if (lastVal == 0) routeDone = true;
			}
		} else if (lastVal == (routeDestCalc - ROUTE_GRID_WIDTH)[0]) {
			// look_up
			saveRoute -= 2;
			saveRoute[1] = DOWNY;
			saveRoute[0] = 0;
			while ((lastVal == (routeDestCalc - ROUTE_GRID_WIDTH)[0]) && (!routeDone)) {
				routeDestCalc -= ROUTE_GRID_WIDTH; // keep checking up
				saveRoute[0] += WALK_JUMP;
#ifdef ARDEBUG
				printf("up\n");
#endif
				lastVal--;
				if (lastVal == 0) routeDone = true;
			}
		} else if (lastVal == (routeDestCalc + ROUTE_GRID_WIDTH)[0]) {
			// look_down
			saveRoute -= 2;
			saveRoute[1] = UPY;
			saveRoute[0] = 0;
			while ((lastVal == (routeDestCalc + ROUTE_GRID_WIDTH)[0]) && (!routeDone)) {
				routeDestCalc += ROUTE_GRID_WIDTH; // keep checking right
				saveRoute[0] += WALK_JUMP;
#ifdef ARDEBUG
				printf("down\n");
#endif
				lastVal--;
				if (lastVal == 0) routeDone = true;
			}
		} else {
			error("AutoRoute:: Can't find way backwards through _routeGrid");
		}
	} while (!routeDone);
#ifdef ARDEBUG
	getchar();
#endif
	// the route is done. if there was an initial x/y movement tag it onto the start
	if (initX < 0) {
        saveRoute -= 2;
		saveRoute[1] = RIGHTY;
		saveRoute[0] = ((-initX) + 7) & 0xFFF8;
	} else if (initX > 0) {
		saveRoute -= 2;
		saveRoute[1] = LEFTY;
		saveRoute[0] = (initX + 7) & 0xFFF8;
	}
	// I wonder why initY isn't checked
	// saveRoute should now point to routeData
	if (routeData > saveRoute) error("Autoroute: Internal pointer error! routeData overflow.");
	*pSaveRoute = saveRoute;
	return 1;
}

