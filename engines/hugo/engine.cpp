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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo 1-3 Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/system.h"
#include "common/random.h"
#include "common/EventRecorder.h"

#include "hugo/game.h"
#include "hugo/hugo.h"
#include "hugo/engine.h"
#include "hugo/global.h"
#include "hugo/file.h"
#include "hugo/schedule.h"
#include "hugo/display.h"
#include "hugo/parser.h"
#include "hugo/route.h"
#include "hugo/util.h"
#include "hugo/sound.h"

namespace Hugo {

#define EDGE           10                       // Closest object can get to edge of screen
#define EDGE2          (EDGE * 2)           // Push object further back on edge collision
#define SHIFT          8                            // Place hero this far inside bounding box
#define MAX_OBJECTS    128              // Used in Update_images()
#define BOUND(X, Y)   ((_boundary[Y * XBYTES + X / 8] & (0x80 >> X % 8)) != 0)  // Boundary bit set

config_t    _config;                                // User's config
maze_t      _maze = {false, 0, 0, 0, 0, 0, 0, 0, 0}; // Default to not in maze
hugo_boot_t _boot;                                  // Boot info structure file
char        _textBoxBuffer[MAX_BOX];                // Buffer for text box
command_t   _line = "";                             // Line of user text input


// Sets the playlist to be the default tune selection
void HugoEngine::initPlaylist(bool playlist[MAX_TUNES]) {
	debugC(1, kDebugEngine, "initPlaylist");

	for (int16 i = 0; i < MAX_TUNES; i++)
		playlist[i] = false;
	for (int16 i = 0; _defltTunes[i] != -1; i++)
		playlist[_defltTunes[i]] = true;
}

// Initialize the dynamic game status
void HugoEngine::initStatus() {
	debugC(1, kDebugEngine, "initStatus");
	_status.initSaveFl    = false;                  // Don't force initial save
	_status.storyModeFl   = false;                  // Not in story mode
	_status.gameOverFl    = false;                  // Hero not knobbled yet
	_status.recordFl      = false;                  // Not record mode
	_status.playbackFl    = false;                  // Not playback mode
	_status.demoFl        = false;                  // Not demo mode
	_status.textBoxFl     = false;                  // Not processing a text box
//	Strangerke - Not used ?
//	_status.mmtime        = false;                   // Multimedia timer support
	_status.lookFl        = false;                  // Toolbar "look" button
	_status.recallFl      = false;                  // Toolbar "recall" button
	_status.leftButtonFl  = false;                  // Left mouse button pressed
	_status.rightButtonFl = false;                  // Right mouse button pressed
	_status.newScreenFl   = false;                  // Screen not just loaded
	_status.jumpExitFl    = false;                  // Can't jump to a screen exit
	_status.godModeFl     = false;                  // No special cheats allowed
	_status.helpFl        = false;                  // Not calling WinHelp()
	_status.path[0]       = 0;                      // Path to write files
	_status.saveSlot      = 0;                      // Slot to save/restore game
	_status.screenWidth   = 0;                      // Desktop screen width

	// Initialize every start of new game
	_status.tick            = 0;                    // Tick count
	_status.saveTick        = 0;                    // Time of last save
	_status.viewState       = V_IDLE;               // View state
	_status.inventoryState  = I_OFF;                // Inventory icon bar state
	_status.inventoryHeight = 0;                    // Inventory icon bar pos
	_status.inventoryObjId  = -1;                   // Inventory object selected (none)
	_status.routeIndex      = -1;                   // Hero not following a route
	_status.go_for          = GO_SPACE;             // Hero walking to space
	_status.go_id           = -1;                   // Hero not walking to anything
}

// Initialize default config values.  Must be done before Initialize().
// Reset needed to save config.cx,cy which get splatted during OnFileNew()
void HugoEngine::initConfig(inst_t action) {
	static int16 cx, cy;                    // Save window size, pos
	int16        i;

	debugC(1, kDebugEngine, "initConfig(%d)", action);

	switch (action) {
	case INSTALL:
		_config.musicFl = true;                     // Music state initially on
		_config.soundFl = true;                     // Sound state initially on
		_config.turboFl = false;                    // Turbo state initially off
		_config.backgroundMusicFl = false;          // No music when inactive
		_config.cx = VIEW_DX * 2;                   // Window view size
		_config.cy = VIEW_DY * 2;

//		_config.wx = 0;
//		_config.wy = 0;

		_config.musicVolume = 85;                   // Music volume %
		_config.soundVolume = 100;                  // Sound volume %
		initPlaylist(_config.playlist);             // Initialize default tune playlist

		HugoEngine::get().file().readBootFile();    // Read startup structure
		HugoEngine::get().file().readConfig();      // Read user's saved config

		cx = _config.cx;                            // Save these around OnFileNew()
		cy = _config.cy;
//		wx = _config.wx;
//		wy = _config.wy;
		break;
	case RESET:
		_config.cx = cx;                            // Restore cx, cy
		_config.cy = cy;
//		_config.wx = wx;
//		_config.wy = wy;

		// Find first tune and play it
		for (i = 0; i < MAX_TUNES; i++)
			if (_config.playlist[i]) {
				sound().playMusic(i);
				break;
			}

		HugoEngine::get().file().initSavedGame();   // Initialize saved game
		break;
	case RESTORE:
		warning("Unhandled action RESTORE");
		break;
	}
}
void HugoEngine::initialize() {
	debugC(1, kDebugEngine, "initialize");

	sound().initSound(INSTALL);
	HugoEngine::get().scheduler().initEventQueue(); // Init scheduler stuff
	screen().initDisplay();                         // Create Dibs and palette
	HugoEngine::get().file().openDatabaseFiles();   // Open database files
	calcMaxScore();                                 // Initialise maxscore

	_rnd = new Common::RandomSource();
	g_eventRec.registerRandomSource(*_rnd, "hugo");

	_rnd->setSeed(42);                              // Kick random number generator

	switch (getGameType()) {
	case kGameTypeHugo1:
		_episode = "\"HUGO'S HOUSE OF HORRORS\"";
		_picDir = "";
		break;
	case kGameTypeHugo2:
		_episode = "\"Hugo's Mystery Adventure\"";
		_picDir = "hugo2/";
		break;
	case kGameTypeHugo3:
		_episode = "\"Hugo's Amazon Adventure\"";
		_picDir = "hugo3/";
		break;
	default:
		error("Unknown game");
	}
}

// Restore all resources before termination
void HugoEngine::shutdown() {
	debugC(1, kDebugEngine, "shutdown");

	sound().initSound(RESTORE);

	HugoEngine::get().file().closeDatabaseFiles();
	if (_status.recordFl || _status.playbackFl)
		HugoEngine::get().file().closePlaybackFile();
	freeObjects();
}

void HugoEngine::readObjectImages() {
	debugC(1, kDebugEngine, "readObjectImages");

	for (int i = 0; i < _numObj; i++)
		HugoEngine::get().file().readImage(i, &_objects[i]);
}

// Read the uif image file (inventory icons)
void HugoEngine::readUIFImages() {
	debugC(1, kDebugEngine, "readUIFImages");

	HugoEngine::get().file().readUIFItem(UIF_IMAGES, screen().getGUIBuffer());              // Read all uif images
}

// Read scenery, overlay files for given screen number
void HugoEngine::readScreenFiles(int screenNum) {
	debugC(1, kDebugEngine, "readScreenFiles(%d)", screenNum);

	HugoEngine::get().file().readBackground(screenNum);                     // Scenery file
	memcpy(screen().getBackBuffer(), screen().getFrontBuffer(), sizeof(screen().getFrontBuffer()));// Make a copy
	HugoEngine::get().file().readOverlay(screenNum, _boundary, BOUNDARY);       // Boundary file
	HugoEngine::get().file().readOverlay(screenNum, _overlay, OVERLAY);         // Overlay file
	HugoEngine::get().file().readOverlay(screenNum, _ovlBase, OVLBASE);         // Overlay base file
}

// Update all object positions.  Process object 'local' events
// including boundary events and collisions
void HugoEngine::moveObjects() {
	object_t *obj;
	seq_t    *currImage;
	int       x1, x2, y1, y2;                       // object coordinates
	int       dx, dy;                               // Allowable motion wrt boundary
	char      radius;                               // Radius for chase (8 bit signed)

	debugC(4, kDebugEngine, "moveObjects");

	// If route mode enabled, do special route processing
	if (_status.routeIndex >= 0)
		route().processRoute();

	// Perform any adjustments to velocity based on special path types
	// and store all (visible) object baselines into the boundary file.
	// Don't store foreground or background objects
	for (int i = 0; i < _numObj; i++) {
		obj = &_objects[i];                         // Get pointer to object
		currImage = obj->currImagePtr;              // Get ptr to current image
		if (obj->screenIndex == *_screen_p) {
			switch (obj->pathType) {
			case CHASE:
			case CHASE2:
				radius = obj->radius;               // Default to object's radius
				if (radius < 0)                     // If radius infinity, use closer value
					radius = DX;

				dx = _hero->x + _hero->currImagePtr->x1 - obj->x - currImage->x1;
				dy = _hero->y + _hero->currImagePtr->y2 - obj->y - currImage->y2 - 1;
				if (abs(dx) <= radius)
					obj->vx = 0;
				else
					obj->vx = dx > 0 ? MIN(dx, obj->vxPath) : MAX(dx, -obj->vxPath);
				if (abs(dy) <= radius)
					obj->vy = 0;
				else
					obj->vy = dy > 0 ? MIN(dy, obj->vyPath) : MAX(dy, -obj->vyPath);

				// Set first image in sequence (if multi-seq object)
				switch (obj->seqNumb) {
				case 4:
					if (!obj->vx) {                 // Got 4 directions
						if (obj->vx != obj->oldvx)  // vx just stopped
							if (dy >= 0)
								obj->currImagePtr = obj->seqList[DOWN].seqPtr;
							else
								obj->currImagePtr = obj->seqList[_UP].seqPtr;
					} else if (obj->vx != obj->oldvx)
						if (dx > 0)
							obj->currImagePtr = obj->seqList[RIGHT].seqPtr;
						else
							obj->currImagePtr = obj->seqList[LEFT].seqPtr;
					break;
				case 3:
				case 2:
					if (obj->vx != obj->oldvx)      // vx just stopped
						if (dx > 0)                 // Left & right only
							obj->currImagePtr = obj->seqList[RIGHT].seqPtr;
						else
							obj->currImagePtr = obj->seqList[LEFT].seqPtr;
					break;
				}

				if (obj->vx || obj->vy)
					obj->cycling = CYCLE_FORWARD;
				else {
					obj->cycling = NOT_CYCLING;
					boundaryCollision(obj);         // Must have got hero!
				}
				obj->oldvx = obj->vx;
				obj->oldvy = obj->vy;
				currImage = obj->currImagePtr;      // Get (new) ptr to current image
				break;
			case WANDER2:
			case WANDER:
				if (!_rnd->getRandomNumber(3 * NORMAL_TPS)) {       // Kick on random interval
					obj->vx = _rnd->getRandomNumber(obj->vxPath << 1) - obj->vxPath;
					obj->vy = _rnd->getRandomNumber(obj->vyPath << 1) - obj->vyPath;

					// Set first image in sequence (if multi-seq object)
					if (obj->seqNumb > 1) {
						if (!obj->vx && (obj->seqNumb >= 4)) {
							if (obj->vx != obj->oldvx)  // vx just stopped
								if (obj->vy > 0)
									obj->currImagePtr = obj->seqList[DOWN].seqPtr;
								else
									obj->currImagePtr = obj->seqList[_UP].seqPtr;
						} else if (obj->vx != obj->oldvx)
							if (obj->vx > 0)
								obj->currImagePtr = obj->seqList[RIGHT].seqPtr;
							else
								obj->currImagePtr = obj->seqList[LEFT].seqPtr;
					}
					obj->oldvx = obj->vx;
					obj->oldvy = obj->vy;
					currImage = obj->currImagePtr;  // Get (new) ptr to current image
				}
				if (obj->vx || obj->vy)
					obj->cycling = CYCLE_FORWARD;
				break;
			default:
				; // Really, nothing
			}
			// Store boundaries
			if ((obj->cycling > ALMOST_INVISIBLE) && (obj->priority == FLOATING))
				storeBoundary(obj->x + currImage->x1, obj->x + currImage->x2, obj->y + currImage->y2);
		}
	}

	// Move objects, allowing for boundaries
	for (int i = 0; i < _numObj; i++) {
		obj = &_objects[i];                         // Get pointer to object
		if ((obj->screenIndex == *_screen_p) && (obj->vx || obj->vy)) {
			// Only process if it's moving

			// Do object movement.  Delta_x,y return allowed movement in x,y
			//  to move as close to a boundary as possible without crossing it.
			currImage = obj->currImagePtr;          // Get ptr to current image
			x1 = obj->x + currImage->x1;            // Left edge of object
			x2 = obj->x + currImage->x2;            // Right edge
			y1 = obj->y + currImage->y1;            // Top edge
			y2 = obj->y + currImage->y2;            // Bottom edge

			if ((obj->cycling > ALMOST_INVISIBLE) && (obj->priority == FLOATING))
				clearBoundary(x1, x2, y2);          // Clear our own boundary
			dx = deltaX(x1, x2, obj->vx, y2);
			if (dx != obj->vx) {
				// An object boundary collision!
				boundaryCollision(obj);
				obj->vx = 0;
			}

			dy = deltaY(x1, x2, obj->vy, y2);

			if (dy != obj->vy) {
				// An object boundary collision!
				boundaryCollision(obj);
				obj->vy = 0;
			}

			if ((obj->cycling > ALMOST_INVISIBLE) && (obj->priority == FLOATING))
				storeBoundary(x1, x2, y2);          // Re-store our own boundary

			obj->x += dx;                           // Update object position
			obj->y += dy;

			// Don't let object go outside screen
			if (x1 < EDGE)
				obj->x = EDGE2;
			if (x2 > (XPIX - EDGE))
				obj->x = XPIX - EDGE2 - (x2 - x1);
			if (y1 < EDGE)
				obj->y = EDGE2;
			if (y2 > (YPIX - EDGE))
				obj->y = YPIX - EDGE2 - (y2 - y1);

			if ((obj->vx == 0) && (obj->vy == 0) && (obj->pathType != WANDER2) && (obj->pathType != CHASE2))
				obj->cycling = NOT_CYCLING;
		}
	}

	// Clear all object baselines from the boundary file.
	for (int i = 0; i < _numObj; i++) {
		obj = &_objects[i];                         // Get pointer to object
		currImage = obj->currImagePtr;              // Get ptr to current image
		if ((obj->screenIndex == *_screen_p) && (obj->cycling > ALMOST_INVISIBLE) && (obj->priority == FLOATING))
			clearBoundary(obj->oldx + currImage->x1, obj->oldx + currImage->x2, obj->oldy + currImage->y2);
	}

	// If maze mode is enabled, do special maze processing
	if (_maze.enabledFl)
		processMaze();
}

// Return maximum allowed movement (from zero to vx) such that object does
// not cross a boundary (either background or another object)
int HugoEngine::deltaX(int x1, int x2, int vx, int y) {
// Explanation of algorithm:  The boundaries are drawn as contiguous
// lines 1 pixel wide.  Since DX,DY are not necessarily 1, we must
// detect boundary crossing.  If vx positive, examine each pixel from
// x1 old to x2 new, else x2 old to x1 new, both at the y2 line.
// If vx zero, no need to check.  If vy non-zero then examine each
// pixel on the line segment x1 to x2 from y old to y new.
// Fix from Hugo I v1.5:
// Note the diff is munged in the return statement to cater for a special
// cases arising from differences in image widths from one sequence to
// another.  The problem occurs reversing direction at a wall where the
// new image intersects before the object can move away.  This is cured
// by comparing the intersection with half the object width pos. If the
// intersection is in the other half wrt the intended direction, use the
// desired vx, else use the computed delta.  i.e. believe the desired vx
	int b;

	debugC(3, kDebugEngine, "deltaX(%d, %d, %d, %d)", x1, x2, vx, y);

	if (vx == 0)
		return(0);                                  // Object stationary

	y *= XBYTES;                                    // Offset into boundary file
	if (vx > 0) {
		// Moving to right
		for (int i = x1 >> 3; i <= (x2 + vx) >> 3; i++) // Search by byte
			if ((b = Utils::firstBit((byte)(_boundary[y + i] | _objBound[y + i]))) < 8) {   // b is index or 8
				// Compute x of boundary and test if intersection
				b += i << 3;
				if ((b >= x1) && (b <= x2 + vx))
					return((b < x1 + ((x2 - x1) >> 1)) ? vx : b - x2 - 1); // return dx
			}
	} else {
		// Moving to left
		for (int i = x2 >> 3; i >= (x1 + vx) >> 3; i--)// Search by byte
			if ((b = Utils::lastBit((byte)(_boundary[y + i] | _objBound[y + i]))) < 8) {    // b is index or 8
				// Compute x of boundary and test if intersection
				b += i << 3;
				if ((b >= x1 + vx) && (b <= x2))
					return((b > x1 + ((x2 - x1) >> 1)) ? vx : b - x1 + 1); // return dx
			}
	}
	return(vx);
}

// Similar to Delta_x, but for movement in y direction.  Special case of
// bytes at end of line segment; must only count boundary bits falling on
// line segment.
int HugoEngine::deltaY(int x1, int x2, int vy, int y) {
	int inc, i, j, b;

	debugC(3, kDebugEngine, "deltaY(%d, %d, %d, %d)", x1, x2, vy, y);

	if (vy == 0)
		return(0);                                  // Object stationary

	inc = (vy > 0 ? 1 : -1);
	for (j = y + inc; j != (y + vy + inc); j += inc) //Search by byte
		for (i = x1 >> 3; i <= x2 >> 3; i++)
			if (b = _boundary[j * XBYTES + i] | _objBound[j * XBYTES + i]) {    // Any bit set
				// Make sure boundary bits fall on line segment
				if (i == (x2 >> 3))                 // Adjust right end
					b &= 0xff << ((i << 3) + 7 - x2);
				else if (i == (x1 >> 3))            // Adjust left end
					b &= 0xff >> (x1 - (i << 3));
				if (b)
					return(j - y - inc);
			}
	return(vy);
}

// Store a horizontal line segment in the object boundary file
void HugoEngine::storeBoundary(int x1, int x2, int y) {
	byte *b;                                        // ptr to boundary byte

	debugC(5, kDebugEngine, "storeBoundary(%d, %d, %d)", x1, x2, y);

	for (int i = x1 >> 3; i <= x2 >> 3; i++) {      // For each byte in line
		b = &_objBound[y * XBYTES + i];             // get boundary byte
		if (i == x2 >> 3)                           // Adjust right end
			*b |= 0xff << ((i << 3) + 7 - x2);
		else if (i == x1 >> 3)                      // Adjust left end
			*b |= 0xff >> (x1 - (i << 3));
		else
			*b = 0xff;
	}
}

// Clear a horizontal line segment in the object boundary file
void HugoEngine::clearBoundary(int x1, int x2, int y) {
	int  i;
	byte *b;                                        // ptr to boundary byte

	debugC(5, kDebugEngine, "clearBoundary(%d, %d, %d)", x1, x2, y);

	for (i = x1 >> 3; i <= x2 >> 3; i++) {          // For each byte in line
		b = &_objBound[y * XBYTES + i];             // get boundary byte
		if (i == x2 >> 3)                           // Adjust right end
			*b &= ~(0xff << ((i << 3) + 7 - x2));
		else if (i == x1 >> 3)                      // Adjust left end
			*b &= ~(0xff >> (x1 - (i << 3)));
		else
			*b = 0;
	}
}

// Maze mode is enabled.  Check to see whether hero has crossed the maze
// bounding box, if so, go to the next room */
void HugoEngine::processMaze() {
	seq_t      *currImage;
	int         x1, x2, y1, y2;                     // hero coordinates

	debugC(1, kDebugEngine, "processMaze");

	//actlist     alnewscr  = {&aheroxy,&astophero,&aherostop,&anewscr,NULL};
	//actlist_pt  alist     = &alnewscr[0];

	currImage = _hero->currImagePtr;                // Get ptr to current image
	x1 = _hero->x + currImage->x1;                  // Left edge of object
	x2 = _hero->x + currImage->x2;                  // Right edge
	y1 = _hero->y + currImage->y1;                  // Top edge
	y2 = _hero->y + currImage->y2;                  // Bottom edge

	if (x1 < _maze.x1) {
		// Exit west
//		anewscr.screen = *_screen_p - 1;
		_actListArr[_alNewscrIndex][3].a8.screenIndex = *_screen_p - 1;
//		aheroxy.x = _maze.x2 - SHIFT - (x2 - x1);
		_actListArr[_alNewscrIndex][0].a2.x = _maze.x2 - SHIFT - (x2 - x1);
//		aheroxy.y = _hero_p->y;
		_actListArr[_alNewscrIndex][0].a2.y = _hero->y;
		_status.routeIndex = -1;
		HugoEngine::get().scheduler().insertActionList(_alNewscrIndex);
	} else if (x2 > _maze.x2) {
		// Exit east
//			anewscr.screen = *_screen_p + 1;
		_actListArr[_alNewscrIndex][3].a8.screenIndex = *_screen_p + 1;
//			aheroxy.x = _maze.x1 + SHIFT;
		_actListArr[_alNewscrIndex][0].a2.x = _maze.x1 + SHIFT;
//			aheroxy.y = _hero_p->y;
		_actListArr[_alNewscrIndex][0].a2.y = _hero->y;
		_status.routeIndex = -1;
		HugoEngine::get().scheduler().insertActionList(_alNewscrIndex);
	} else if (y1 < _maze.y1 - SHIFT) {
		// Exit north
//				anewscr.screen = *_screen_p - _maze.size;
		_actListArr[_alNewscrIndex][3].a8.screenIndex = *_screen_p - _maze.size;
//				aheroxy.x = _maze.x3;             // special offset for perspective
		_actListArr[_alNewscrIndex][0].a2.x = _maze.x3;
//				aheroxy.y = _maze.y2 - SHIFT - (y2 - y1);
		_actListArr[_alNewscrIndex][0].a2.y = _maze.y2 - SHIFT - (y2 - y1);
		_status.routeIndex = -1;
		HugoEngine::get().scheduler().insertActionList(_alNewscrIndex);
	} else if (y2 > _maze.y2 - SHIFT / 2) {
		// Exit south
//					anewscr.screen = *_screen_p + _maze.size;
		_actListArr[_alNewscrIndex][3].a8.screenIndex = *_screen_p + _maze.size;
//					aheroxy.x = _maze.x4;            // special offset for perspective
		_actListArr[_alNewscrIndex][0].a2.x = _maze.x4;
//					aheroxy.y = _maze.y1 + SHIFT;
		_actListArr[_alNewscrIndex][0].a2.y = _maze.y1 + SHIFT;
		_status.routeIndex = -1;
		HugoEngine::get().scheduler().insertActionList(_alNewscrIndex);
	}
}

// Compare function for the quicksort.  The sort is to order the objects in
// increasing vertical position, using y+y2 as the baseline
// Returns -1 if ay2 < by2 else 1 if ay2 > by2 else 0
int y2comp(const void *a, const void *b) {
	int       ay2, by2;

	debugC(6, kDebugEngine, "y2comp");

	const object_t *p1 = &HugoEngine::get()._objects[*(const byte *)a];
	const object_t *p2 = &HugoEngine::get()._objects[*(const byte *)b];

	if (p1 == p2)
		// Why does qsort try the same indexes?
		return (0);

	if (p1->priority == BACKGROUND)
		return (-1);

	if (p2->priority == BACKGROUND)
		return (1);

	if (p1->priority == FOREGROUND)
		return (1);

	if (p2->priority == FOREGROUND)
		return (-1);

	ay2 = p1->y + p1->currImagePtr->y2;
	by2 = p2->y + p2->currImagePtr->y2;

	return(ay2 - by2);
}

// Draw all objects on screen as follows:
// 1. Sort 'FLOATING' objects in order of y2 (base of object)
// 2. Display new object frames/positions in dib
// Finally, cycle any animating objects to next frame
void HugoEngine::updateImages() {
	int       i, j, num_objs;
	object_t *obj;                                  // Pointer to object
	seq_t    *seqPtr;                               // Save curr_seq_p
	byte      objindex[MAX_OBJECTS];                // Array of indeces to objects

	debugC(5, kDebugEngine, "updateImages");

	// Initialise the index array to visible objects in current screen
	for (i = 0, num_objs = 0; i < _numObj; i++) {
		obj = &_objects[i];
		if ((obj->screenIndex == *_screen_p) && (obj->cycling >= ALMOST_INVISIBLE))
			objindex[num_objs++] = i;
	}

	// Sort the objects into increasing y+y2 (painter's algorithm)
	qsort(objindex, num_objs, sizeof(objindex[0]), y2comp);

	// Add each visible object to display list
	for (i = 0; i < num_objs; i++) {
		obj = &_objects[objindex[i]];
		// Count down inter-frame timer
		if (obj->frameTimer)
			obj->frameTimer--;

		if (obj->cycling > ALMOST_INVISIBLE)        // Only if visible
			switch (obj->cycling) {
			case NOT_CYCLING:
				screen().displayFrame(obj->x, obj->y, obj->currImagePtr, obj->priority == OVEROVL);
				break;
			case CYCLE_FORWARD:
				if (obj->frameTimer)                // Not time to see next frame yet
					screen().displayFrame(obj->x, obj->y, obj->currImagePtr, obj->priority == OVEROVL);
				else
					screen().displayFrame(obj->x, obj->y, obj->currImagePtr->nextSeqPtr, obj->priority == OVEROVL);
				break;
			case CYCLE_BACKWARD:
				seqPtr = obj->currImagePtr;
				if (!obj->frameTimer)               // Show next frame
					while (seqPtr->nextSeqPtr != obj->currImagePtr)
						seqPtr = seqPtr->nextSeqPtr;
				screen().displayFrame(obj->x, obj->y, seqPtr, obj->priority == OVEROVL);
				break;
			default:
				break;
			}
	}

	// Cycle any animating objects
	for (i = 0; i < num_objs; i++) {
		obj = &_objects[objindex[i]];
		if (obj->cycling != INVISIBLE) {
			// Only if it's visible
			if (obj->cycling == ALMOST_INVISIBLE)
				obj->cycling = INVISIBLE;

			// Now Rotate to next picture in sequence
			switch (obj->cycling) {
			case NOT_CYCLING:
				break;
			case CYCLE_FORWARD:
				if (!obj->frameTimer) {
					// Time to step to next frame
					obj->currImagePtr = obj->currImagePtr->nextSeqPtr;
					// Find out if this is last frame of sequence
					// If so, reset frame_timer and decrement n_cycle
					if (obj->frameInterval || obj->cycleNumb) {
						obj->frameTimer = obj->frameInterval;
						for (j = 0; j < obj->seqNumb; j++)
							if (obj->currImagePtr->nextSeqPtr == obj->seqList[j].seqPtr)
								if (obj->cycleNumb)     // Decr cycleNumb if Non-continous
									if (!--obj->cycleNumb)
										obj->cycling = NOT_CYCLING;
					}
				}
				break;
			case CYCLE_BACKWARD:
				if (!obj->frameTimer) {
					// Time to step to prev frame
					seqPtr = obj->currImagePtr;
					while (obj->currImagePtr->nextSeqPtr != seqPtr)
						obj->currImagePtr = obj->currImagePtr->nextSeqPtr;
					// Find out if this is first frame of sequence
					// If so, reset frame_timer and decrement n_cycle
					if (obj->frameInterval || obj->cycleNumb) {
						obj->frameTimer = obj->frameInterval;
						for (j = 0; j < obj->seqNumb; j++)
							if (obj->currImagePtr == obj->seqList[j].seqPtr)
								if (obj->cycleNumb)     // Decr cycleNumb if Non-continous
									if (!--obj->cycleNumb)
										obj->cycling = NOT_CYCLING;
					}
				}
				break;
			default:
				break;
			}
			obj->oldx = obj->x;
			obj->oldy = obj->y;
		}
	}
}

// Return object index of the topmost object under the cursor, or -1 if none
// Objects are filtered if not "useful"
int16 HugoEngine::findObject(uint16 x, uint16 y) {
	object_t *obj;
	seq_t    *curImage;
	int16     objIndex = -1;                        // Index of found object
	uint16    y2Max = 0;                            // Greatest y2
	int       i;

	debugC(3, kDebugEngine, "findObject(%d, %d)", x, y);

	// Check objects on screen
	for (i = 0, obj = _objects; i < _numObj; i++, obj++)    {
		// Object must be in current screen and "useful"
		if (obj->screenIndex == *_screen_p && (obj->genericCmd || obj->objValue || obj->cmdIndex)) {
			curImage = obj->currImagePtr;
			// Object must have a visible image...
			if (curImage != NULL && obj->cycling != INVISIBLE) {
				// If cursor inside object
				if (x >= (uint16)obj->x && x <= obj->x + curImage->x2 && y >= (uint16)obj->y && y <= obj->y + curImage->y2)
					// If object is closest so far
					if (obj->y + curImage->y2 > y2Max) {
						y2Max = obj->y + curImage->y2;
						objIndex = i;               // Found an object!
					}
			} else
				// ...or a dummy object that has a hotspot rectangle
				if (curImage == NULL && obj->vxPath != 0 && !obj->carriedFl) {
					// If cursor inside special rectangle
					if ((int16)x >= obj->oldx && (int16)x < obj->oldx + obj->vxPath && (int16)y >= obj->oldy && (int16)y < obj->oldy + obj->vyPath)
						// If object is closest so far
						if (obj->oldy + obj->vyPath - 1 > (int16)y2Max) {
							y2Max = obj->oldy + obj->vyPath - 1;
							objIndex = i;           // Found an object!
						}
				}
		}
	}
	return objIndex;
}

// Find a clear space around supplied object that hero can walk to
bool HugoEngine::findObjectSpace(object_t *obj, int16 *destx, int16 *desty) {
//	bool   found = false;                            // TRUE if we found a clear space
	bool   foundFl;
	seq_t *curImage = obj->currImagePtr;
	int16  x;
	int16  y  = obj->y + curImage->y2 - 1;

	debugC(1, kDebugEngine, "findObjectSpace(obj, %d, %d)", *destx, *desty);

//	if (!found)                                      // Try left rear corner
	for (foundFl = true, *destx = x = obj->x + curImage->x1; x < *destx + HERO_MAX_WIDTH; x++)
		if (BOUND(x, y))
			foundFl = false;

	if (!foundFl)                                       // Try right rear corner
		for (foundFl = true, *destx = x = obj->x + curImage->x2 - HERO_MAX_WIDTH + 1; x <= obj->x + (int16)curImage->x2; x++)
			if (BOUND(x, y))
				foundFl = false;

	if (!foundFl)                                       // Try left front corner
		for (foundFl = true, y += 2, *destx = x = obj->x + curImage->x1; x < *destx + HERO_MAX_WIDTH; x++)
			if (BOUND(x, y))
				foundFl = false;

	if (!foundFl)                                       // Try right rear corner
		for (foundFl = true, *destx = x = obj->x + curImage->x2 - HERO_MAX_WIDTH + 1; x <= obj->x + (int16)curImage->x2; x++)
			if (BOUND(x, y))
				foundFl = false;

	*desty = y;
	return(foundFl);
}

// Search background command list for this screen for supplied object.
// Return first associated verb (not "look") or NULL if none found.
char *HugoEngine::useBG(char *name) {
	int i;
	objectList_t p = _backgroundObjects[*_screen_p];

	debugC(1, kDebugEngine, "useBG(%s)", name);

	for (i = 0; *_arrayVerbs[p[i].verbIndex]; i++)
		if ((name == _arrayNouns[p[i].nounIndex][0] &&
		        p[i].verbIndex != _look) &&
		        ((p[i].roomState == DONT_CARE) || (p[i].roomState == _screenStates[*_screen_p])))
			return (_arrayVerbs[p[i].verbIndex][0]);

	return (NULL);
}

// If status.objid = -1, pick up objid, else use status.objid on objid,
// if objid can't be picked up, use it directly
void HugoEngine::useObject(int16 objId) {
	object_t *obj = &_objects[objId];               // Ptr to object
	uses_t *use;                                    // Ptr to use entry
	target_t *target;                               // Ptr to target entry
	bool foundFl;                                   // TRUE if found target entry
	char *verb;                                     // Background verb to use directly

	debugC(1, kDebugEngine, "useObject(%d)", objId);

	if (_status.inventoryObjId == -1) {
		// Get or use objid directly
		if ((obj->genericCmd & TAKE) || obj->objValue)  // Get collectible item
			sprintf(_line, "%s %s", _arrayVerbs[_take][0], _arrayNouns[obj->nounIndex][0]);
		else if (obj->cmdIndex != 0)            // Use non-collectible item if able
			sprintf(_line, "%s %s", _arrayVerbs[_cmdList[obj->cmdIndex][1].verbIndex][0], _arrayNouns[obj->nounIndex][0]);
		else if ((verb = useBG(_arrayNouns[obj->nounIndex][0])) != NULL)
			sprintf(_line, "%s %s", verb, _arrayNouns[obj->nounIndex][0]);
		else
			return;                         // Can't use object directly
	} else {
		// Use status.objid on objid
		// Default to first cmd verb
		sprintf(_line, "%s %s %s", _arrayVerbs[_cmdList[_objects[_status.inventoryObjId].cmdIndex][1].verbIndex][0], _arrayNouns[_objects[_status.inventoryObjId].nounIndex][0], _arrayNouns[obj->nounIndex][0]);

		// Check valid use of objects and override verb if necessary
		for (use = _uses; use->objId != _numObj; use++)
			if (_status.inventoryObjId == use->objId) {
				// Look for secondary object, if found use matching verb
				for (foundFl = false, target = use->targets; _arrayNouns[target->nounIndex] != NULL; target++)
					if (_arrayNouns[target->nounIndex][0] == _arrayNouns[obj->nounIndex][0]) {
						foundFl = true;
						sprintf(_line, "%s %s %s", _arrayVerbs[target->verbIndex][0], _arrayNouns[_objects[_status.inventoryObjId].nounIndex][0], _arrayNouns[obj->nounIndex][0]);
					}

				// No valid use of objects found, print failure string
				if (!foundFl) {
					// Deselect dragged icon if inventory not active
					if (_status.inventoryState != I_ACTIVE)
						_status.inventoryObjId  = -1;
					Utils::Box(BOX_ANY, HugoEngine::get()._textData[use->dataIndex]);
					return;
				}
			}
	}

	if (_status.inventoryState == I_ACTIVE)         // If inventory active, remove it
		_status.inventoryState = I_UP;
	_status.inventoryObjId  = -1;                   // Deselect any dragged icon
	parser().lineHandler();                         // and process command
}

// Issue "Look at <object>" command
// Note special case of swapped hero image
void HugoEngine::lookObject(object_t *obj) {
	debugC(1, kDebugEngine, "lookObject");

	if (obj == _hero) {
		// Hero swapped - look at other
		obj = &_objects[_heroImage];
	}
	parser().command("%s %s", _arrayVerbs[_look][0], _arrayNouns[obj->nounIndex][0]);
}

// Free all object images
void HugoEngine::freeObjects() {
	object_t *obj;
	seq_t *seq;

	debugC(1, kDebugEngine, "freeObjects");

	// Nothing to do if not allocated yet
	if (_hero->seqList[0].seqPtr == NULL)
		return;

	// Free all sequence lists and image data
	for (int i = 0; i < _numObj; i++) {
		obj = &_objects[i];
		for (int j = 0; j < obj->seqNumb; j++) {        // for each sequence
			seq = obj->seqList[j].seqPtr;           // Free image
			if (seq == NULL)                        // Failure during database load
				break;
			do {
				free(seq->imagePtr);
				seq = seq->nextSeqPtr;
			} while (seq != obj->seqList[j].seqPtr);
			free(seq);                              // Free sequence record
		}
	}
}

// Add action lists for this screen to event queue
void HugoEngine::screenActions(int screenNum) {
	uint16 *screenAct = _screenActs[screenNum];

	debugC(1, kDebugEngine, "screenActions(%d)", screenNum);

	if (screenAct) {
		for (int i = 0; screenAct[i]; i++)
			HugoEngine::get().scheduler().insertActionList(screenAct[i]);
	}
}

// Set the new screen number into the hero object and any carried objects
void HugoEngine::setNewScreen(int screenNum) {
	debugC(1, kDebugEngine, "setNewScreen(%d)", screenNum);

	*_screen_p = screenNum;                             // HERO object
	for (int i = HERO + 1; i < _numObj; i++)            // Any others
		if (_objects[i].carriedFl)                      // being carried
			_objects[i].screenIndex = screenNum;
}

// An object has collided with a boundary.  See if any actions are required
void HugoEngine::boundaryCollision(object_t *obj) {
	int         x, y, dx, dy;
	char        radius;                             // 8 bits signed
	hotspot_t   *hotspot;

	debugC(1, kDebugEngine, "boundaryCollision");

	if (obj == _hero) {
		// Hotspots only relevant to HERO
		if (obj->vx > 0)
			x = obj->x + obj->currImagePtr->x2;
		else
			x = obj->x + obj->currImagePtr->x1;
		y = obj->y + obj->currImagePtr->y2;

		for (int i = 0; _hotspots[i].screenIndex >= 0; i++) {
			hotspot = &_hotspots[i];
			if (hotspot->screenIndex == obj->screenIndex)
				if ((x >= hotspot->x1) && (x <= hotspot->x2) && (y >= hotspot->y1) && (y <= hotspot->y2)) {
					HugoEngine::get().scheduler().insertActionList(hotspot->actIndex);
					break;
				}
		}
	} else {
		// Check whether an object collided with HERO
		dx = _hero->x + _hero->currImagePtr->x1 - obj->x - obj->currImagePtr->x1;
		dy = _hero->y + _hero->currImagePtr->y2 - obj->y - obj->currImagePtr->y2;
		// If object's radius is infinity, use a closer value
		radius = obj->radius;
		if (radius < 0)
			radius = DX * 2;
		if ((abs(dx) <= radius) && (abs(dy) <= radius))
			HugoEngine::get().scheduler().insertActionList(obj->actIndex);
	}
}

// Initialize screen components and display results
void HugoEngine::initNewScreenDisplay() {
	debugC(1, kDebugEngine, "initNewScreenDisplay");

	screen().displayList(D_INIT);
	screen().setBackgroundColor(_TBLACK);
	screen().displayBackground();

	// Stop premature object display in Display_list(D_DISPLAY)
	_status.newScreenFl = true;
}

// Add up all the object values and all the bonus points
void HugoEngine::calcMaxScore() {
	int i;

	debugC(1, kDebugEngine, "calcMaxScore");

	for (i = 0; i < _numObj; i++)
		_maxscore += _objects[i].objValue;

	for (i = 0; i < _numBonuses; i++)
		_maxscore += _points[i].score;
}

// Exit game, advertise trilogy, show copyright
void HugoEngine::endGame() {
	debugC(1, kDebugEngine, "endGame");

	if (!_boot.registered)
		Utils::Box(BOX_ANY, HugoEngine::get()._textEngine[kEsAdvertise]);
	Utils::Box(BOX_ANY, "%s\n%s", _episode, COPYRIGHT);
	_status.viewState = V_EXIT;
}

} // end of namespace Hugo
