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


#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/cursorman.h"
#include "sky/control.h"
#include "sky/disk.h"
#include "sky/logic.h"
#include "sky/mouse.h"
#include "sky/screen.h"
#include "sky/sky.h"
#include "sky/skydefs.h"
#include "sky/struc.h"
#include "sky/compact.h"

namespace Sky {

#define MICE_FILE	60300
#define NO_MAIN_OBJECTS	24
#define NO_LINC_OBJECTS	21

uint32 Mouse::_mouseMainObjects[24] = {
	65,
	9,
	66,
	64,
	8,
	63,
	10,
	11,
	71,
	76,
	37,
	36,
	42,
	75,
	79,
	6,
	74,
	39,
	49,
	43,
	34,
	35,
	77,
	38
};

uint32 Mouse::_mouseLincObjects[21] = {
	24625,
	24649,
	24827,
	24651,
	24583,
	24581,
	24582,
	24628,
	24650,
	24629,
	24732,
	24631,
	24584,
	24630,
	24626,
	24627,
	24632,
	24643,
	24828,
	24830,
	24829
};

Mouse::Mouse(OSystem *system, Disk *skyDisk, SkyCompact *skyCompact, Screen *skyScreen) {
	_skyDisk = skyDisk;
	_skyCompact = skyCompact;
	_system = system;
	_skyScreen = skyScreen;
	_mouseB = 0;
	_currentCursor = 6;
	_touchId = 0;
	_mouseX = GAME_SCREEN_WIDTH / 2;
	_mouseY = GAME_SCREEN_HEIGHT / 2;

	_miceData = _skyDisk->loadFile(MICE_FILE);

	//load in the object mouse file
	_objectMouseData = _skyDisk->loadFile(MICE_FILE + 1);

	_proxFrame = 0;
	_proxFrameSpeed = 0;
	resetUI();
}

void Mouse::resetUI() {
	// popup
	_clickedNum = 0;
	_prevMouseOn = false;
	_touchId = 0;
	_holding = false;
	_timeOn = 0;
	_fadeOut = 0;
	_mMode = GAMEPLAY; // master ui mode
	_isFloor = false;
	_isExit = false;
	_actionFlash = false;
	_floorLock = false;

	if (_skyScreen) {
		_skyScreen->clearAllProximityIcons(false);
		_skyScreen->clearAllIbassIcons(false);
	}

}

Mouse::~Mouse( ){
	free (_miceData);
	free (_objectMouseData);
}

void Mouse::replaceMouseCursors(uint16 fileNo) {
	free(_objectMouseData);
	_objectMouseData = _skyDisk->loadFile(fileNo);
}

bool Mouse::fnAddHuman() {
	//reintroduce the mouse so that the human can control the player
	//could still be switched out at high-level

	if (_mMode == GAMEPLAY)
		_mMode = MUST_RELEASE;

	if (!Logic::_scriptVariables[MOUSE_STOP]) {
		Logic::_scriptVariables[MOUSE_STATUS] |= 6;	//cursor & mouse

		if (_mouseY < 2) //stop mouse activating top line
			_mouseY = 2;

		_system->warpMouse(_mouseX, _mouseY);

		//force the pointer engine into running a get-off
		//even if it's over nothing

		//KWIK-FIX
		//get off may contain script to remove mouse pointer text
		//surely this script should be run just in case
		//I am going to try it anyway
		if (Logic::_scriptVariables[GET_OFF])
			_skyLogic->script((uint16)Logic::_scriptVariables[GET_OFF],(uint16)(Logic::_scriptVariables[GET_OFF] >> 16));

		Logic::_scriptVariables[SPECIAL_ITEM] = 0xFFFFFFFF;
		Logic::_scriptVariables[GET_OFF] = RESET_MOUSE;
	}

	return true;
}

void Mouse::fnSaveCoods() {
	Logic::_scriptVariables[SAFEX] = _mouseX + TOP_LEFT_X;
	Logic::_scriptVariables[SAFEY] = _mouseY + TOP_LEFT_Y;
}

void Mouse::lockMouse() {
	SkyEngine::_systemVars->systemFlags |= SF_MOUSE_LOCKED;
}

void Mouse::unlockMouse() {
	SkyEngine::_systemVars->systemFlags &= ~SF_MOUSE_LOCKED;
}

void Mouse::restoreMouseData(uint16 frameNum) {
	warning("Stub: Mouse::restoreMouseData");
}

void Mouse::drawNewMouse() {
	warning("Stub: Mouse::drawNewMouse");
	//calculateMouseValues();
	//saveMouseData();
	//drawMouse();
}

void Mouse::waitMouseNotPressed(int minDelay) {
	bool mousePressed = true;
	uint32 now = _system->getMillis();
	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();
	while (mousePressed || _system->getMillis() < now + minDelay) {

		if (eventMan->shouldQuit()) {
			minDelay = 0;
			mousePressed = false;
		}

		if (!eventMan->getButtonState())
			mousePressed = false;

		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kSkyActionSkip) {
					minDelay = 0;
					mousePressed = false;
				}
				break;
			default:
				break;
			}
		}
		_system->updateScreen();
		_system->delayMillis(20);
	}
}

void Mouse::spriteMouse(uint16 frameNum, uint8 mouseX, uint8 mouseY) {
	_currentCursor = frameNum;

	byte *newCursor = _miceData;
	newCursor += ((DataFileHeader *)_miceData)->s_sp_size * frameNum;
	newCursor += sizeof(DataFileHeader);

	uint16 mouseWidth = ((DataFileHeader *)_miceData)->s_width;
	uint16 mouseHeight = ((DataFileHeader *)_miceData)->s_height;

	CursorMan.replaceCursor(newCursor, mouseWidth, mouseHeight, mouseX, mouseY, 0);
	if (frameNum == MOUSE_BLANK)
		CursorMan.showMouse(false);
	else
		CursorMan.showMouse(true);
}

bool Mouse::isUILive() {
	if (!Logic::_scriptVariables[MOUSE_STOP] && (Logic::_scriptVariables[MOUSE_STATUS] & (1 << 1)) && (Logic::_scriptVariables[MOUSE_STATUS] & (1 << 2)) )
		return	true;

	return false;
}

void Mouse::mouseEngine() {
	_logicClick = (_mouseB > 0); // click signal is available for Logic for one gamecycle

	if (!Logic::_scriptVariables[MOUSE_STOP]) {
		if (Logic::_scriptVariables[MOUSE_STATUS] & (1 << 1)) {
			pointerEngine(_mouseX + TOP_LEFT_X, _mouseY + TOP_LEFT_Y);
			if (Logic::_scriptVariables[MOUSE_STATUS] & (1 << 2)) //buttons enabled?
				buttonEngine1();
		}
	}
	_mouseB = 0;	//don't save up buttons
}

void Mouse::mouseEngineIBASS() {
	_logicClick = (_mouseB > 0);

	if (_actionFlash) {
		_actionFlashTime--;
		if (!_actionFlashTime)
			_actionFlash = false;

		if (!(_actionFlashTime & 1))
			_skyScreen->clearIbassIcon(_actionFlashIcon, false);
		else
			_skyScreen->setIcon(_actionFlashIcon, _actionFlashX, _actionFlashY);
	}

	switch (_mMode) {
	case ALERT_TO_GAME:
		if (!_mouseB)
			return;

		_skyScreen->hideInventory();
		_mMode = MUST_RELEASE;
		break;
	case MUST_RELEASE:
		if (_mouseB)
			return;
		resetUI();
		_mMode = GAMEPLAY;
		break;
	case GAMEPLAY:
		pointerEngineIBASS(_mouseX + TOP_LEFT_X, _mouseY + TOP_LEFT_Y);
		break;
	case PRE_INVENTORY:
		if (_mouseB)
			return;

		_mMode = INVENTORY;
		break;
	case INVENTORY:
		if (isLincInv())
			lincInvMouse(_mouseX + TOP_LEFT_X, _mouseY + TOP_LEFT_Y);
		else
			invMouse(_mouseX + TOP_LEFT_X, _mouseY + TOP_LEFT_Y);
		break;
	case INVENTORY_USE_ON:
		invUseOn(_mouseX + TOP_LEFT_X, _mouseY + TOP_LEFT_Y);
		break;
	case TEXT_CHOOSER:
		break;
	case INV_TEMP_EXAMINE:
		break;
	}
	_mouseB = 0;
	return;
}

int Mouse::doProximityHighlights(uint16 xPos, uint16 yPos) {
	uint32 currentListNum = Logic::_scriptVariables[MOUSE_LIST_NO];
	uint16 *currentList;
	int midx, midy;
	Compact *itemData;
	uint16 itemNum;
	int found = 0;
	int nearestDist = 99999;
	int nearestId = 0;
	_nearestProximityIconId = 0;

	_proxFrameSpeed++;
	if (_proxFrameSpeed & 1)
		_proxFrame++;
	if (_proxFrame == 3)
		_proxFrame = 0;

	// near inv button?
	if (yPos > HOTSPOT_INVY && xPos < HOTSPOT_INVX)
		return 0;

	// not on same as last time, or was not on anything previously, so re-scan all objects
	do {
		currentList = (uint16 *)_skyCompact->fetchCpt(currentListNum);
		while ((*currentList != 0) && (*currentList != 0xFFFF)) {
			itemNum = *currentList;
			itemData = _skyCompact->fetchCpt(itemNum);
			currentList++;
			if ((itemData->screen == Logic::_scriptVariables[SCREEN]) && (itemData->status & 16)) {
				// hotspot, if a non-floor object
				if (itemData->mouseOn && yPos > (20 + TOP_LEFT_Y)) {
					midx = giveXCood(itemData, itemNum);
					midy = giveYCood(itemData, itemNum);

					// skip very high hotspots - eg the old inventory catch object
					if (midy < (TOP_LEFT_Y + 5))
						continue;

					int d = abs(xPos - midx) + abs(yPos - midy);
					if (d < GLOW_DIST) {
						float opacity = 1.0 - ((d * 1.0) / GLOW_DIST);
						_skyScreen->setProximityIcon(found++, midx - TOP_LEFT_X - 4, ((midy - TOP_LEFT_Y) - 4), opacity, _proxFrame);
					}

					if (d < nearestDist) {
						nearestDist = d;
						nearestId = itemNum;
						_nearestProximityIconId = (found - 1);
					}
				}
			}
		}
		if (*currentList == 0xFFFF)
			currentListNum = currentList[1];
	} while (*currentList != 0);

	return  nearestId;
}

UIIcon Mouse::getInteractIcon(uint32 id) {
	switch (id) {
	case 70: // screen 1 stairs
	case 4119: // elevator hole
	case 8210: // outside stairs to furnace
		return UI_ICON_DOWN;

	case 69: // screen 1 stairs
		return UI_ICON_UP;//

	case 1: // joey
	case 4122: // hobbins
	case 16: // lamb
	case 8324: // guard
	case 8211: // sam
	case 8301: // norville
	case 8544: // clipboard man
	case 136: // power room man
	case 12546: // galagher
	case 12442: // insurance man
	case 12430: // anchor man
	case 12407: // dr burke
	case 16516: // henri
	case 16601: // guard
	case 16441: // piermont
	case 16599: // gameboy
	case 16600: // gardener
	case 16701: // barman
	case 16772: // babs
	case 16737: // man1
	case 16731: // man2
	case 20911: // ken
	case 137: // anita
	case 12289: // mr cool
	case 8205: // rad suit man
	case 8309: // guard walkway
	case 21014: // father @ end
		return UI_ICON_MOUTH; // talk

	case 4151: // notice
	case 4152: // ledge-side notice
	case 12383: // sculpture
	case 2388: // mural
	case 12386: // poster
	case 12387: // ins poster
	case 12378: // surgery poster
	case 12375: // surgery poster
	case 12376: // surgery poster
	case 12377: // surgery poster
	case 12401: // dr poster
	case 12402: // dr poster
	case 12545: // ins poster
	case 20678: // crowbar on floor
	case 12508: // reich poster
	case 12507: // reich cert
	case 12495: // reich poster
	case 8500: // sensors
	case 20576: // medical computer monitor
	case 20578: // medical computer
	case 8543: // securty hq linc terminal
		return UI_ICON_LOOK;//look at

	case 4113: // sandwhich
	case 4114: // sandwhich
	case 8: // reich card
	case 9: // sunglasses
	case 8442: // key
	case 8443: // wd40
	case 8446: // putty
	case 24642: // linc item
	case 24602: // linc item
	case 16501: // secateurs
	case 16747: // glass
	case 20527: // brick
	case 20660: // tongs
	case 71: // anita card
	case 49: // dog biscuits
	case 24638: // linc
	case 24639: // linc
	case 24604: // linc
	case 12588: // light bulb
	case 12501: // magazine
	case 12565: // tour ticket
	case 8517: // reactor anita card??!
	case 20526: // plaster
	case 24641: // linc world tuning fork
	case 24787: // virus
	case 12517: // video
	case 12434: // anchor
	case 12435: // anchor
		return  UI_ICON_HAND;
	}

	return UI_ICON_USE;
}

bool Mouse::hasSingleInteractIcon(uint32 id) {
	switch (id) {
	case    1: // joey
	case    4122: // hobbins
	case    4151: // notice
	case    4152: // ledge-side notice
	case    16: // lamb
	case    8500: // sensors
	case    8324: // guard
	case    8211: // sam
	case    8301: // norville
	case    8544: // clipboard man
	case    136: // power room man
	case    12546: // galagher
	case    12383: // sculpture
	case    12442: // insurance man
	case    12388: // mural
	case    12386: // ins poster
	case    12387: // ins poster
	case    12430: // anchor man
	case    12378: // surgery poster
	case    12375: // surgery poster
	case    12376: // surgery poster
	case    12377: // surgery poster
	case    12407: // dr burke
	case    12401: // dr poster
	case    12402: // dr poster
	case    12545: // ins poster
	case    24810: // linc window button
	case    16516: // henri
	case    16601: // guard
	case    16441: // piermont
	case    16599: // gameboy
	case    16600: // gardener
	case    16701: // barman
	case    16772: // babs
	case    16737: // man1
	case    16731: // man2
	case    20911: // ken
	case    137: // anita
	case    12508: // reich poster
	case    12507: // reich cert
	case    12495: // reich poster
	case    12289: // mr cool
	case    8205: // rad suit man
	case    8309: // guard walkway

	case    20576: // medical computer monitor
	case    20578: // medical computer

	case    24586: // linc maze
	case    24592: // linc maze
	case    24633: // linc maze
	case    24634: // linc maze
	case    24593: // linc maze
	case    24594: // linc maze
	case    24595: // linc maze
	case    24598: // linc maze
	case    8543: // securty hq linc terminal

	case    69: // screen 1 stairs
	case    70: // screen 1 stairs
	case    4119: // elevator hole
	case    8210: // outside stairs to furnace

	case    21014: // father @ end
		return  true;
	}

	return  false;
}

struct MouseXMap {
	int id;
	int mid;
} mouseXMap[] = {
	{ 4112, 258 }, // cupboard door
	{ 4113, 254 }, // sandwich
	{ 4114, 254 }, // spanner
	{ 12358, 428 }, // bellevue - missing exit
	{ 12681, 428 }, // bellevue
	{ 12349, 142 }, // bellevue
	{ 12347, 428 }, // bellevue
	{ 90, 404 }, // first room, door
	{ 97, 190 }, // ledge room, door
	{ 4116, 269 }, // elevator
	{ 4119, 269 }, // hole
	{ 4110, 356 }, // lathe
	{ 8248, 357 }, // walkway to security lobby
	{ 8238, 181 }, // walkway to power
	{ 8317, 426 }, // walkway to crash
	{ 8341, 173 }, // factory to walkway
	{ 8344, 433 }, // factory to factory 2
	{ 8500, 224 }, // sensors
	{ 8355, 428 }, // factory 2 to factory 3
	{ 8446, 226 }, // putty
	{ 8438, 226 }, // gangway
	{ 12680, 201 }, // exit
	{ 12679, 307 }, // exit
	{ 12678, 179 }, // exit
	{ 12677, 319 }, // exit
	{ 12324, 191 }, // exit
	{ 12336, 182 }, // exit
	{ 12442, 376 }, // exit
	{ 8483, 225 }, // console
	{ 12641, 158 }, // exit
	// linc
	{ 12633, 190 }, // maze
	{ 24586, 210 }, // maze
	{ 24592, 288 }, // maze
	{ 24593, 400 }, // maze
	{ 24594, 203 }, // maze
	{ 24595, 299 }, // maze
	// hyde
	{ 16496, 263 }, // plant
	// cathedral
	{ 16462, 168 }, // exit
	// lockers
	{ 16576, 246 }, // body == 16569
	{ 16577, 271 }, // body == 16570
	{ 16578, 296 }, // body == 16571
	{ 16579, 321 }, // body == 16572
	{ 16580, 346 }, // body == 16573
	// underworld
	{ 20506, 389 }, // metal door
	// pit world
	{ 20600, 283 }, // cover
	{ 20648, 283 }, // pit
	{ 20570, 247 }, // exit to medical droid room
	{ 20575, 277 }, // slot
	{ 20577, 297 }, // recharge unit
	{ 24786, 288 }, // linc crystal
	{ 24787, 288 }, // virus
	{ 20712, 305 }, // console
	{ 20713, 356 }, // console
	{ 20728, 388 }, // exit from door room
	{ 8272, 278 }, // power room chair
	{ 12390, 187 }, // burke door
	{ 12541, 187 }, // burke exit
	{ 0, 0 }
};

uint16 Mouse::giveXCood(Compact *itemData, uint32 id) {
	uint16 mid, midy;

	mid = itemData->xcood + ((int16)itemData->mouseRelX) + (itemData->mouseSizeX >> 1);
	midy = itemData->ycood + ((int16)itemData->mouseRelY) + (itemData->mouseSizeY >> 1);

	if (mid < HOTSPOT_INVX && midy > HOTSPOT_INVY)
		return  HOTSPOT_INVX + 16;

	// not lamb
	if (id != 16) {
		if (mid > 428)
			return  428;
		if (mid < 142)
			return  142;
	}

	if (_hotspotXMap.size() == 0) {
		for (MouseXMap *m = mouseXMap; m->id != 0; m++)
			_hotspotXMap[m->id] = m->mid;
	}

	if (_hotspotXMap.contains(id))
		return _hotspotXMap[id];

	return mid;
}

struct MouseYMap {
	uint32 id;
	uint16 mid;
} mouseYMap[] = {
	{ 4113, 218 },
	{ 4114, 226 },
	{ 4108, 0 },
	{ 4111, 0 },
	{ 4115, 0 },
	{ 4117, 0 },
	{ 4154, 0 },
	{ 12349, TWEEKY_EXIT_ADJUST },
	{ 12347, TWEEKY_EXIT_ADJUST },
	{ 12358, TWEEKY_EXIT_ADJUST },
	{ 90, 208 },
	{ 97, 216 },
	{ 4116, 259 },
	{ 4119, 259 },
	{ 4315, 261 },
	{ 4103, 242 },
	{ 4105, 241 },
	{ 4110, 230 },
	{ 8248, 222 },
	{ 8238, 222 },
	{ 8310, 222 },
	{ 8246, 273 },
	{ 8317, 273 },
	{ 8331, 230 },
	{ 8341, 239 },
	{ 8344, 282 },
	{ 8500, 197 },
	{ 8355, 255 },
	{ 8366, 227 },
	{ 8375, 238 },
	{ 8446, 247 },
	{ 8438, 247 },
	{ 8364, 252 },
	{ 8459, 252 },
	{ 8256, 297 },
	{ 12304, 252 },
	{ 12338, 286 },
	{ 12681, 250 },
	{ 12370, 289 },
	{ 12361, 267 },
	{ 12679, 179 },
	{ 12678, 179 },
	{ 12677, 179 },
	{ 12313, 284 },
	{ 12676, 241 },
	{ 12327, 244 },
	{ 12315, 286 },
	{ 12324, 308 },
	{ 12336, 306 },
	{ 12442, 246 },
	{ 12447, 242 },
	{ 12680, 179 },
	{ 12459, 281 },
	{ 12471, 302 },
	{ 12399, 257 },
	{ 12390, 264 },
	{ 12541, 264 },
	{ 12620, 220 },
	{ 12621, 220 },
	{ 12622, 220 },
	{ 12623, 220 },
	{ 12613, 220 },
	{ 12624, 220 },
	{ 12631, 273 },
	{ 12641, 290 },
	{ 12642, 273 },
	{ 12474, 282 },
	{ 12486, 298 },
	{ 8295, 299 },
	{ 16415, 261 },
	{ 16403, 261 },
	{ 16393, 246 },
	{ 16487, 256 },
	{ 16492, 287 },
	{ 16394, 313 },
	{ 16412, 276 },
	{ 16428, 233 },
	{ 16424, 236 },
	{ 16462, 296 },
	{ 16464, 250 },
	{ 16465, 226 },
	{ 16474, 263 },
	{ 16576, 229 },
	{ 16577, 229 },
	{ 16578, 229 },
	{ 16579, 229 },
	{16580, 229 },
	{ 8478, 291 },
	{ 8481, 253 },
	{ 8511, 271 },
	{ 16538, 279 },
	{ 16439, 267 },
	{ 16592, 267 },
	{ 16649, 291 },
	{ 16660, 285 },
	{ 16662, 254 },
	{ 16661, 291 },
	{ 16671, 291 },
	{ 16681, 282 },
	{ 16682, 282 },
	{ 16719, 287 },
	{ 16720, 287 },
	{ 16729, 291 },
	{ 16730, 301 },
	{ 20506, 276 },
	{ 20518, 276 },
	{ 20532, 264 },
	{ 20558, 296 },
	{ 20598, 237 },
	{ 20601, 183 },
	{ 20600, 237 },
	{ 20648, 237 },
	{ 20570, 268 },
	{ 20573, 290 },
	{ 20579, 272 },
	{ 20572, 277 },
	{ 20578, 286 },
	{ 20674, 279 },
	{ 20675, 238 },
	{ 20511, 259 },
	{ 20583, 291 },
	{ 20604, 264 },
	{ 20676, 294 },
	{ 20619, 269 },
	{ 20628, 284 },
	{ 20617, 295 },
	{ 24786, 256 },
	{ 24787, 256 },
	{ 20672, 265 },
	{ 20696, 265 },
	{ 20697, 265 },
	{ 20715, 265 },
	{ 20711, 219 },
	{ 20712, 219 },
	{ 20713, 219 },
	{ 20725, 268 },
	{ 20726, 270 },
	{ 20728, 219 },
	{ 20741, 254 },
	{ 20743, 279 },
	{ 20752, 279 },
	{ 20886, 243 },
	{ 20874, 0 },
	{ 24592, 306 },
	{ 24634, 306 },
	{ 24594, 278 },
	{ 8272, 0 },
	{ 0, 0 }
};

uint16 Mouse::giveYCood(Compact *itemData, uint32 id) {
	// push objects that are too far to left, or to the right, back onto the screen
	uint16 mid;

	mid = itemData->ycood + ((int16)itemData->mouseRelY) + (itemData->mouseSizeY >> 1);

	if (id == 4112) {
		if (Logic::_scriptVariables[112]) // cupb_flag
			return 0;
		mid = 221;
	}

	if (id == 12630 || id == 12616 || id == 12629 || id == 12628 || id == 12627 || id == 12626) // slot
		mid += 4;

	if (id == 8291 || id == 8290) { // power room switched
		if (!Logic::_scriptVariables[429])
			return 0;
	}

	if (_hotspotYMap.size() == 0) {
		for (MouseYMap *m = mouseYMap; m->id != 0; m++)
			_hotspotYMap[m->id] = m->mid;
	}

	if (_hotspotYMap.contains(id))
		return _hotspotYMap[id];

	return mid;
}

int Mouse::touchingFloor(uint16 xPos, uint16 yPos) {
	uint32 currentListNum = Logic::_scriptVariables[MOUSE_LIST_NO];
	uint16 *currentList;
	Compact *itemData;
	uint16 itemNum;

	// do not detect floors beyond a certain depth
	if (yPos > (HOTSPOT_INVY + 8)) return 0;
	if (yPos < (20 + TOP_LEFT_Y)) return 0;

	do {
		currentList = (uint16 *)_skyCompact->fetchCpt(currentListNum);
		while ((*currentList != 0) && (*currentList != 0xFFFF)) {
			itemNum = *currentList;
			itemData = _skyCompact->fetchCpt(itemNum);
			currentList++;

			// on this screen, active, and a floor
			if ((itemData->screen == Logic::_scriptVariables[SCREEN]) && (!itemData->mouseOn) && (itemData->status & 16)) {
				if (itemData->xcood + ((int16)itemData->mouseRelX) > xPos) continue;
				if (itemData->xcood + ((int16)itemData->mouseRelX) + itemData->mouseSizeX < xPos) continue;
				if (itemData->ycood + ((int16)itemData->mouseRelY) > yPos) continue;
				if (itemData->ycood + ((int16)itemData->mouseRelY) + itemData->mouseSizeY < yPos) continue;

				// hit this floor - we're done here
				return  itemNum;
			}
		}
		if (*currentList == 0xFFFF)
			currentListNum = currentList[1];
	} while (*currentList != 0);

	// found nowt
	return 0;
}

void Mouse::pointerEngine(uint16 xPos, uint16 yPos) {
	uint32 currentListNum = Logic::_scriptVariables[MOUSE_LIST_NO];
	uint16 *currentList;
	do {
		currentList = (uint16 *)_skyCompact->fetchCpt(currentListNum);
		while ((*currentList != 0) && (*currentList != 0xFFFF)) {
			uint16 itemNum = *currentList;
			Compact *itemData = _skyCompact->fetchCpt(itemNum);
			currentList++;
			if ((itemData->screen == Logic::_scriptVariables[SCREEN]) && (itemData->status & 16)) {
				if (itemData->xcood + ((int16)itemData->mouseRelX) > xPos)
					continue;
				if (itemData->xcood + ((int16)itemData->mouseRelX) + itemData->mouseSizeX < xPos)
					continue;
				if (itemData->ycood + ((int16)itemData->mouseRelY) > yPos)
					continue;
				if (itemData->ycood + ((int16)itemData->mouseRelY) + itemData->mouseSizeY < yPos)
					continue;
				// we've hit the item
				if (Logic::_scriptVariables[SPECIAL_ITEM] == itemNum)
					return;
				Logic::_scriptVariables[SPECIAL_ITEM] = itemNum;
				if (Logic::_scriptVariables[GET_OFF])
					_skyLogic->mouseScript(Logic::_scriptVariables[GET_OFF], itemData);
				Logic::_scriptVariables[GET_OFF] = itemData->mouseOff;
				if (itemData->mouseOn)
					_skyLogic->mouseScript(itemData->mouseOn, itemData);
				return;
			}
		}
		if (*currentList == 0xFFFF)
			currentListNum = currentList[1];
	} while (*currentList != 0);
	if (Logic::_scriptVariables[SPECIAL_ITEM] != 0) {
		Logic::_scriptVariables[SPECIAL_ITEM] = 0;

		if (Logic::_scriptVariables[GET_OFF])
			_skyLogic->script((uint16)Logic::_scriptVariables[GET_OFF],(uint16)(Logic::_scriptVariables[GET_OFF] >> 16));
		Logic::_scriptVariables[GET_OFF] = 0;
	}
}

void Mouse::pointerEngineIBASS(uint16 xPos, uint16 yPos) {
	int midx = 0, midy = 0; // open hotspot dimensions
	Compact *itemData;
	int itemProx;
	int d = 0;
	if (yPos < (TOP_LEFT_Y + 5))
		return;
	// mouse must be down to search for stuff
	if (_mouseB) {
		debug(1, "Clicked");
		// what are we near?
		itemProx = doProximityHighlights(xPos, yPos);

		// seems we need to clear this, but only safe to do so when a new click occurs as player is still live when walking to interact after a use-on interaction
		// but not in linc terminal
		if (101 != Logic::_scriptVariables[SCREEN])
			Logic::_scriptVariables[OBJECT_HELD] = 0;
		// need to check if still touching any currently popped up hotspot
		if (_touchId && _touchId != 32767 && _timeOn < CLICK_THRESHOLD) {
			if (_prevMouseOn == false)
				_timeOn = 1;
			// fetch the compact
			itemData = _skyCompact->fetchCpt(_touchId);
			// if not a floor
			if (itemData->mouseOn) {
				_skyScreen->setProximityNotAnimate(_nearestProximityIconId);//we want this one remaining one to not animate

				midx = giveXCood(itemData, _touchId);
				midy = giveYCood(itemData, _touchId);
				// calc dist
				d = abs(xPos - midx) + abs(yPos - midy);
				// some left hand edge adjust for twin-hotspot non-exits
				if (!hasSingleInteractIcon(_touchId) && !_isExit)
					if ((midx - HOTSPOT_DIM) < TOP_LEFT_X)
						midx = TOP_LEFT_X + HOTSPOT_DIM;
				// still touching a normal hotspot?
				if (!_isExit && midx - HOTSPOT_DIM < xPos && midx + HOTSPOT_DIM > xPos && midy - HOTSPOT_YOFF < yPos && midy/*+HOTSPOT_YOFF*/ > yPos) {
					// if newly touching, and same item as last time, then inc the click count
					if (_prevMouseOn == false) {
						_clickedNum++;  // one more click
						// reset fade out timer to max
						_fadeOut = HOTSPOT_FADEOUT;
						// reset hover time
						_timeOn = 1;
					} else
						_timeOn++;

					int icony = midy - HOTSPOT_YOFF;
					icony -= TOP_LEFT_Y; // normalise for renderer
					if (icony < 0)
						icony = 0;

					if (hasSingleInteractIcon(_touchId)) {
						// update the coordinate of the hotspot
						midx -= TOP_LEFT_X;
						midx -= HOTSPOT_EXIT_DIM;

						// draw the icons
						_skyScreen->setIcon(getInteractIcon(_touchId), midx + 4, icony);
						_actionFlashX = midx + 4;
						_actionFlashIcon = getInteractIcon(_touchId);
						_actionFlashY = icony;
					} else { // 2 icons
						midx -= TOP_LEFT_X;
						midx -= HOTSPOT_DIM;

						_actionFlashY = icony;

						if (xPos <= (midx + HOTSPOT_DIM + TOP_LEFT_X)) {
							Logic::_scriptVariables[BUTTON] = 2;
							_actionFlashX = midx;
							_actionFlashIcon = UI_ICON_LOOK;
						} else {
							Logic::_scriptVariables[BUTTON] = 1;
							_actionFlashX = midx + HOTSPOT_DIM + 4;
							_actionFlashIcon = getInteractIcon(_touchId);
						}
					}
					// mouse on
					_prevMouseOn = true;
					// end here, when button held on something
					return;
				} else if (_isExit && midx - HOTSPOT_DIM < xPos && midx + HOTSPOT_DIM > xPos && midy - HOTSPOT_YOFF < yPos && midy/*+HOTSPOT_YOFF*/ > yPos) {
					// still touching the poped up exit
					// which action
					Logic::_scriptVariables[BUTTON] = 1;

					// if newly touching, and same item as last time, then inc the click count
					if (_prevMouseOn == false) {
						_clickedNum++;  // one more click
						// reset fade out timer to max
						_fadeOut = HOTSPOT_FADEOUT;
						// reset hover time
						_timeOn = 0;
					} else {
						_timeOn++; // one more cycle
						// update the coordinate of the hotspot
						int iconx = midx - HOTSPOT_EXIT_DIM;
						iconx -= TOP_LEFT_X; // normalise for renderer
						if (iconx < 0)
							iconx = 0;

						int icony = midy - HOTSPOT_EXIT_YOFF;
						icony -= TOP_LEFT_Y; // normalise for renderer
						if (icony < 0)
							icony = 0;

						// draw the icon
						initExitIcon(_exitType, iconx, icony);

						_actionFlashY = icony;
					}
					// mouse on
					_prevMouseOn = true;
					// end here, when button held on something
					return;
				} else {
					_touchId = 0;
					_clickedNum = 0;
				}
			} else { // else if floor
				// floors just count time - we only want clicks(jabs)
				// still touching it?
				if ((itemData->xcood + ((int16)itemData->mouseRelX) < xPos) && (itemData->xcood + ((int16)itemData->mouseRelX) + itemData->mouseSizeX > xPos) && (itemData->ycood + ((int16)itemData->mouseRelY) < yPos) && (itemData->ycood + ((int16)itemData->mouseRelY) + itemData->mouseSizeY > yPos))
					_timeOn++; // one more cycle
				else {
					_touchId = 0;
					_clickedNum = 0;
				}
			}
		}
		// find if we're touching a floor
		int floor = touchingFloor(xPos, yPos);

		if (itemProx) {
			itemData = _skyCompact->fetchCpt(itemProx);
			midx = giveXCood(itemData, itemProx);
			midy = giveYCood(itemData, itemProx);
			// calc dist
			d = abs(xPos - midx) + abs(yPos - midy);
		}
		// not close enough to real hotspot, and still touching previous floor
		if (floor && (d >= USE_ON_DIST) && floor == _touchId)
			// if we skip floor, and dont hit another hotspot, then we quit after this search, because there is no fadeOut on floors, so the system resets the couter
			return;

		if ((itemProx && d < USE_ON_DIST) || floor) {
			// we've hit a new object
			// floor or object?
			if (itemProx && d < USE_ON_DIST) {
				// record what we're touching
				Logic::_scriptVariables[SPECIAL_ITEM] = itemProx; // put in here now, for mouseOn script
				_touchId = itemProx;
				_touchIdLegacy = itemProx; // remembered when mouse off
			} else {
				// record what we're touching
				Logic::_scriptVariables[SPECIAL_ITEM] = floor; // put in here now, for mouseOn script
				_touchId = floor;
				_touchIdLegacy = floor; // remembered when mouse off
			}
			itemData = _skyCompact->fetchCpt(_touchId);

			// jumping straight from one object to another?
			if (Logic::_scriptVariables[SPECIAL_ITEM]) {
				// remove hotspot icons
				_skyScreen->clearAllIbassIcons(false);
			}
			// reset hover time to max
			_timeOn = 1;
			_clickedNum = 0;

			// run previous items get-off, if there was one (gone straight from one object onto another!)
			if (Logic::_scriptVariables[GET_OFF])
				_skyLogic->mouseScript(Logic::_scriptVariables[GET_OFF], itemData);

			// write mouse off script number
			Logic::_scriptVariables[GET_OFF] = itemData->mouseOff;
			// run the mouse on script
			if (itemData->mouseOn) {
				// disallow sliding back onto floors
				_floorLock = true;
				// reset fade out timer
				_fadeOut = HOTSPOT_FADEOUT;
				debug(1, "Fadeout det to 36, mouseOn = %d, dist = %d", itemData->mouseOn, d);
				_skyScreen->setProximityNotAnimate(_nearestProximityIconId); // we want this one remaining one to not animate

				// assume not
				_isExit = false;
				_isFloor = false;

				_skyLogic->mouseScript(itemData->mouseOn, itemData);

				// exit, or hotspot?
				if (_isExit) {
					int iconx = midx - HOTSPOT_EXIT_DIM;
					iconx -= TOP_LEFT_X; // normalise for renderer
					if (iconx < 0)
						iconx = 0;

					int icony = midy - HOTSPOT_EXIT_YOFF;
					icony -= TOP_LEFT_Y; // normalise for renderer
					if (icony < 0)
						icony = 0;

					// draw the icon
					initExitIcon(_exitType, iconx, icony);
				} else {
					int icony = midy - HOTSPOT_YOFF;
					icony -= TOP_LEFT_Y;// normalise for renderer
					if (icony < 0)
						icony = 0;
					if (hasSingleInteractIcon(_touchId)) {
						// update the coordinate of the hotspot
						int iconx = midx - HOTSPOT_EXIT_DIM;
						iconx -= TOP_LEFT_X;// normalise for renderer
						if (iconx < 0)
							iconx = 0;

						// draw the icons
						_skyScreen->setIcon(getInteractIcon(_touchId), iconx + 4, icony);
						// stairs on screen 1 - normal object turned into exit - need to hack the button -
						// the rest are either talk-to, which doesnt seem to matter, or look-at, like posters on walls
						if (_touchId == 70 || _touchId == 69 || _touchId == 24633
						        || _touchId == 24634 || _touchId == 4119 || _touchId == 8210)
							Logic::_scriptVariables[BUTTON] = 1;
						else
							Logic::_scriptVariables[BUTTON] = 2;

					} else { // 2 icons
						// update the coordinate of the hotspot
						int iconx = midx - HOTSPOT_DIM;
						iconx -= TOP_LEFT_X;// normalise for renderer
						if (iconx < 0)
							iconx = 0;

						// draw the icons
						_skyScreen->setIcon(UI_ICON_LOOK, iconx, icony);
						_skyScreen->setIcon(getInteractIcon(_touchId), iconx + HOTSPOT_DIM + 4, icony);
					}
				}
			} else if (!_floorLock) { // no mouseOn script, which probably always means this is a floor?
				// set action
				Logic::_scriptVariables[BUTTON] = 1;
				// non graphical hotspots - floors - can interact first click
				_clickedNum = 1;
				// reset hover time to max
				_timeOn = 1;
				_isFloor = true;
			} else
				_touchId = 0; // locked floor

			// mouse on
			_prevMouseOn = true;
			// we're done
			return;
		} else {
			debug(1, "nothing");
			// not touching anything, force open hotspot to close
			if (_fadeOut)
				_fadeOut = 0;
		}
		// not touching a game object - but what about HU buttons, such as Inv?
		if (!_prevMouseOn && 101 != Logic::_scriptVariables[SCREEN] && Logic::_scriptVariables[LOGIC_LIST_NO] != 24765) {
			if (xPos < HOTSPOT_INVX && yPos > HOTSPOT_INVY) {
				_skyLogic->startInventory();
				_touchId = 0;
				_holding = false;
				_mMode = PRE_INVENTORY;
				_skyScreen->clearAllProximityIcons(false);
				_skyScreen->clearAllIbassIcons(false);
				return;
			}
			// control panel
			if (xPos < HOTSPOT_OPTIONSX && yPos < HOTSPOT_OPTIONSY) {
				_skyControl->doControlPanel();
				_skyScreen->clearAllProximityIcons(false);
				_skyScreen->clearAllIbassIcons(false);
				_mouseB = 0;
			}
			// help screen
			if (xPos > HOTSPOT_helpx && yPos < HOTSPOT_helpy) {
				_skyScreen->clearAllProximityIcons(false);
				_skyScreen->clearAllIbassIcons(false);
				_mouseB = 0;
			}
		}
		// mouse on
		_prevMouseOn = true;
		debug(1, "floor returned = %d", floor);
	} else { // not touching screen
		if (_prevMouseOn) {
			if (!_touchId || _isFloor)
				_skyScreen->clearAllProximityIcons();
			else if (_touchId) {
				_skyScreen->clearAllProximityIcons();
				// clear all but the one relating to the highlighted hotspot - hmmm
				itemData = _skyCompact->fetchCpt(_touchId);
				midx = giveXCood(itemData, _touchId);
				midy = giveYCood(itemData, _touchId); //itemData->ycood + ((int16)itemData->mouseRelY) + (itemData->mouseSizeY>>1);
				_skyScreen->setProximityIcon(0, midx - TOP_LEFT_X - 4, ((midy - TOP_LEFT_Y) - 4), 1.0, 0);
				_skyScreen->setProximityNotAnimate(0);//we want this one remaining one to not animate
			}
		}
		// let go
		_prevMouseOn = false;
		_floorLock = false;

		// if 2nd or more click on this item
		if (_timeOn && _clickedNum) {
			// faking the click so that the engine scripts execute correctly
			_logicClick = true;
			if (Logic::_scriptVariables[BUTTON] == 0)
				Logic::_scriptVariables[BUTTON] = 1;

			// first, some autosaving incase this is a fatal, yes FATAL, interaction
			if (g_engine->canSaveGameStateCurrently())
				g_engine->saveGameState(0, "Autosave", true);
			_fadeOut = 0;   // force getOff script to run also

			// setup action flash
			if (!_isFloor) {
				_actionFlash = true;
				_actionFlashTime = ACTION_FLASH_TIME;
			}
			// set this again, as there's a chance some other random script may have cleared it since we first touched this hotspot
			Logic::_scriptVariables[SPECIAL_ITEM] = _touchId;
			// over anything?
			Compact *item = _skyCompact->fetchCpt(Logic::_scriptVariables[SPECIAL_ITEM]);
			if (item->mouseClick/* && item->cursorText*/)// not floors
				_skyLogic->mouseScript(item->mouseClick, item);
		}
		// reset ready for next stab
		_timeOn = 0;
	}
	// mouse is not pressed, or not touching anything
	// count down to fade out and run get-off
	if (_fadeOut) {
		_fadeOut--;
		// update coordinate, so hotspot follows walking megas
		updateHotspotCoordinate(xPos);
	}

	if (!_fadeOut) {
		_skyScreen->clearAllProximityIcons();

		// reset number of times we clicked on this
		_clickedNum = 0;
		_touchId = 0;

		// process get-off script, if we were touching, and there is one
		if (Logic::_scriptVariables[SPECIAL_ITEM] != 0) {
			// close the hotspot popup
			// remove hotspot icons
			_skyScreen->clearAllIbassIcons(true);
			Logic::_scriptVariables[SPECIAL_ITEM] = 0;
			// get off
			if (Logic::_scriptVariables[GET_OFF])
				_skyLogic->script((uint16)Logic::_scriptVariables[GET_OFF], (uint16)(Logic::_scriptVariables[GET_OFF] >> 16));

			Logic::_scriptVariables[GET_OFF] = 0;
		}
	}
	return;
}

void Mouse::initExitIcon(uint32 type, int iconx, int icony) {
	switch (type) {
	case MOUSE_LEFT:
		_actionFlashIcon = UI_ICON_LEFT;
		_actionFlashX = iconx + 8;
		_skyScreen->setIcon(UI_ICON_LEFT, iconx + 8, icony);
		break;

	case MOUSE_RIGHT:
		_actionFlashIcon = UI_ICON_RIGHT;
		_actionFlashX = iconx + 4;
		_skyScreen->setIcon(UI_ICON_RIGHT, iconx + 4, icony);
		break;

	case MOUSE_UP:
		_actionFlashIcon = UI_ICON_UP;
		_actionFlashX = iconx + 5;
		_skyScreen->setIcon(UI_ICON_UP, iconx + 5, icony);
		break;

	case MOUSE_DOWN:
		_actionFlashIcon = UI_ICON_DOWN;
		_actionFlashX = iconx + 5;
		_skyScreen->setIcon(UI_ICON_DOWN, iconx + 5, icony);
		break;

	}
}

void Mouse::updateHotspotCoordinate(uint16 xPos) {
	int midx,  midy;

	// fetch the compact
	Compact *itemData = _skyCompact->fetchCpt(_touchIdLegacy);

	if (!itemData)
		return;

	// if not a floor
	if (itemData->mouseOn && !_isExit) {
		midx = giveXCood(itemData, _touchIdLegacy);
		midy = giveYCood(itemData, _touchIdLegacy);

		int icony = midy - HOTSPOT_YOFF;
		icony -= TOP_LEFT_Y; // normalise for renderer
		if (icony < 0)
			icony = 0;

		if (hasSingleInteractIcon(_touchIdLegacy)) {
			// update the coordinate of the hotspot
			int iconx = midx - HOTSPOT_EXIT_DIM;
			iconx -= TOP_LEFT_X;//normalise for renderer
			if (iconx < 0)
				iconx = 0;

			// draw the icons
			_skyScreen->setIcon(getInteractIcon(_touchIdLegacy), iconx + 4, icony);
		} else {
			// update the coordinate of the hotspot
			int iconx = midx - HOTSPOT_DIM;
			iconx -= TOP_LEFT_X;//normalise for renderer
			if (iconx < 0)
				iconx = 0;

			// draw the icons
			_skyScreen->setIcon(UI_ICON_LOOK, iconx, icony);
			_skyScreen->setIcon(getInteractIcon(_touchIdLegacy), iconx + HOTSPOT_DIM + 4, icony);

			// still touching the poped up area
			// which action
			if (xPos <= midx)
				Logic::_scriptVariables[BUTTON] = 2;
			else
				Logic::_scriptVariables[BUTTON] = 1;
		}
	}
}

void Mouse::buttonPressed(uint8 button) {
	_mouseB = button;
}

void Mouse::mouseMoved(uint16 mouseX, uint16 mouseY) {
	_mouseX = mouseX;
	_mouseY = mouseY;
}

void Mouse::buttonEngine1() {
	//checks for clicking on special item
	//"compare the size of this routine to S1 mouse_button"

	if (_mouseB) {	//anything pressed?
		Logic::_scriptVariables[BUTTON] = _mouseB;
		if (Logic::_scriptVariables[SPECIAL_ITEM] && Logic::_scriptVariables[SPECIAL_ITEM] != 0xFFFFFFFF) { // over anything?
			Compact *item = _skyCompact->fetchCpt(Logic::_scriptVariables[SPECIAL_ITEM]);
			if (item->mouseClick)
				_skyLogic->mouseScript(item->mouseClick, item);
		}
	}
}

void Mouse::resetCursor() {
	spriteMouse(_currentCursor, 0, 0);
}

uint16 Mouse::findMouseCursor(uint32 itemNum) {
	uint8 cnt;
	for (cnt = 0; cnt < NO_MAIN_OBJECTS; cnt++) {
		if (itemNum == _mouseMainObjects[cnt]) {
			return cnt;
		}
	}
	for (cnt = 0; cnt < NO_LINC_OBJECTS; cnt++) {
		if (itemNum == _mouseLincObjects[cnt]) {
			return cnt;
		}
	}
	return 0;
}

void Mouse::fnOpenCloseHand(bool open) {
	if ((!open) && (!Logic::_scriptVariables[OBJECT_HELD])) {
		spriteMouse(1, 0, 0);
		return;
	}
	uint16 cursor = findMouseCursor(Logic::_scriptVariables[OBJECT_HELD]) << 1;
	if (open)
		cursor++;

	uint32 size = ((DataFileHeader *)_objectMouseData)->s_sp_size;
	uint8 *srcData;
	uint8 *destData;

	srcData = (uint8 *)_objectMouseData + size * cursor + sizeof(DataFileHeader);
	destData = (uint8 *)_miceData + sizeof(DataFileHeader);
	memcpy(destData, srcData, size);
	spriteMouse(0, 5, 5);
}

bool Mouse::wasClicked() {
	if (_logicClick) {
		_logicClick = false;
		return true;
	} else
		return false;
}

void Mouse::lincInvMouse(uint16 xPos, uint16 yPos) {
	uint32 *objList;
	int j, num;
	Compact *itemData = nullptr;
	bool touched = false;
	bool buttonHeld = (_system->getEventManager()->getButtonState() != 0); // mouseEngine() consumes the click at the end so using _mouseB would increment the timer at each click not on hold

	_logicClick = buttonHeld;

	// get list of compacts in inventory
	objList = _skyLogic->giveInvList();

	// how many
	num = (int)Logic::_scriptVariables[MENU_LENGTH];

	if (_holding) {
		debug(1, "Object held = %d", _touchId);
		// dragged off of inv? Quit inv mode
		if (xPos < _invX || xPos > _invX + _invW || yPos < _invY || yPos > _invY + _invH) {
			// dragged off of the inv
			_mMode = INVENTORY_USE_ON;
			// remove inv items from screen/logic processing
			_skyLogic->killInventory();
			// got to tidy the inv item
			itemData = _skyCompact->fetchCpt(_touchId);
			itemData->frame--;
			itemData->getToFlag = 0;
			// start fresh
			_touchId = 0;

			// highlight off
			_skyScreen->setDragIconHighlight(false);
			return;
		}
		// else, wait for release, and see if release on another item
		if (buttonHeld) {
			// keep scanning objects to run geton/off
			for (j = 0; j < num; j++) {
				// fetch the compact
				itemData = _skyCompact->fetchCpt(objList[j]);

				if (itemData->xcood + ((int16)itemData->mouseRelX) > xPos)
					continue;
				if (itemData->xcood + ((int16)itemData->mouseRelX) + XWIDTH < xPos)
					continue;
				if (itemData->ycood + ((int16)itemData->mouseRelY) > yPos)
					continue;
				if (itemData->ycood + ((int16)itemData->mouseRelY) + YDEPTH < yPos)
					continue;

				touched = true;

				// on previous?
				if (_hoverId == objList[j])
					continue; // still on previous, so skip
				// run previous items get-off, if there was one (gone straight from one object onto another!)
				if (Logic::_scriptVariables[GET_OFF])
					_skyLogic->mouseScript(Logic::_scriptVariables[GET_OFF], _skyCompact->fetchCpt(_hoverId));
				// write new mouse off script number
				Logic::_scriptVariables[GET_OFF] = itemData->mouseOff;
				_hoverId = objList[j];

				// new item
				_skyLogic->startInventory(_hoverId); // highlight touched item

				// run the mouse on script
				if (itemData->mouseOn) {
					uint16 tempY = itemData->ycood;
					pushInvY(tempY);
					itemData->ycood = 136;
					Logic::_scriptVariables[BUTTON] = 3;
					Logic::_scriptVariables[ICON_LIT] = _hoverId;
					Logic::_scriptVariables[SPECIAL_ITEM] = _hoverId;
					_skyLogic->mouseScript(itemData->mouseOn, itemData);
					// and bring back render position
					itemData->ycood = tempY;
				}
			}
			// if we didn't register any hit then run the get off - we're sitting on blank inventory space
			if (!touched) {
				// touching nothing
				_hoverId = 0;
				if (Logic::_scriptVariables[GET_OFF]) {
					_skyLogic->mouseScript(Logic::_scriptVariables[GET_OFF], _skyCompact->fetchCpt(_hoverId));
					Logic::_scriptVariables[GET_OFF] = 0;
				}
			}
			// we're done
			return;
		}

		if (_hoverId == 24584 || _hoverId == 24630 || _hoverId == 24732 || _hoverId == 24643) {
			// fetch the compact
			itemData = _skyCompact->fetchCpt(_touchId);
			// set button
			Logic::_scriptVariables[BUTTON] = 3;
			// record what we're touching
			Logic::_scriptVariables[SPECIAL_ITEM] = _hoverId;
			// fight internal logic's impulse to slide back up
			uint16 tempY = itemData->ycood;
			itemData->ycood = 136;
			uint32 menuRef = _lincMenuRef;
			_skyLogic->mouseScript(itemData->mouseClick, itemData);
			// and bring back render position
			itemData->ycood = tempY; // itemData->invY;

			if (menuRef != _lincMenuRef) {
				itemData = _skyCompact->fetchCpt(_touchId);
				itemData->frame--;
				itemData->getToFlag = 0;
			}
		} else {
			// wont combine
			itemData = _skyCompact->fetchCpt(_touchId);
			itemData->frame--;
			itemData->getToFlag = 0;

		}
		// bring back new inv - have to call again to kick in this frame fix
		_skyLogic->startInventory();
		_holding = false;
		_skyScreen->clearDragIcon(); // clear the dragging icon

		// cancel cursor
		Logic::_scriptVariables[OBJECT_HELD] = 0;

		return;
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------

	// touching screen, but not an object yet
	if (buttonHeld) {
		for (j = 0; j < num; j++) {
			// fetch the compact
			itemData = _skyCompact->fetchCpt(objList[j]);

			if (itemData->xcood + ((int16)itemData->mouseRelX) > xPos) continue;
			if (itemData->xcood + ((uint16)itemData->mouseRelX) + XWIDTH < xPos) continue;
			if (itemData->ycood + ((uint16)itemData->mouseRelY) > yPos) continue;
			if (itemData->ycood + ((uint16)itemData->mouseRelY) + YDEPTH < yPos) continue;

			// record what we're touching
			Logic::_scriptVariables[SPECIAL_ITEM] = objList[j];

			if (_touchId != objList[j] && !_holding) {
				debug(1, "New touch");
				// run previous items get-off, if there was one (gone straight from one object onto another)
				if (Logic::_scriptVariables[GET_OFF])
					_skyLogic->mouseScript(Logic::_scriptVariables[GET_OFF], itemData);

				// write mouse off script number
				Logic::_scriptVariables[GET_OFF] = itemData->mouseOff;

				_touchId = objList[j];
				_hoverId = objList[j];

				Logic::_scriptVariables[ICON_LIT] = objList[j];

				_skyLogic->mouseScript(itemData->mouseOn, itemData);
				// reset hold counter

				// init drag
				_skyScreen->setDragIcon(itemData->frame, true);

				// set button
				Logic::_scriptVariables[BUTTON] = 2;
				// record what we're touching
				Logic::_scriptVariables[SPECIAL_ITEM] = _touchId;
				Logic::_scriptVariables[ICON_LIT] = 0;
				Logic::_scriptVariables[OBJECT_HELD] = 0;

				// fetch the compact
				itemData = _skyCompact->fetchCpt(_touchId);

				// fight internal logic's impulse to slide back up

				uint16 tempY = itemData->ycood;
				itemData->ycood = 136;

				// special stuff for quit-linc
				if (_touchId == 24582) {
					debug(1, "QUIT LINC");
					itemData->frame--;

					// remove inventory items from screen/logic processing
					_skyLogic->killInventory();
					_skyScreen->clearDragIcon();
					_skyLogic->mouseScript(itemData->mouseClick, itemData);
					return;
				}
				_skyLogic->mouseScript(itemData->mouseClick, itemData);
				// and bring back render position
				itemData->ycood = tempY;

				if (Logic::_scriptVariables[OBJECT_HELD]) {
					// just render offset - cursor is still beneath the finger
					_mouseYOff = HOTSPOT_USE_ON_YOFF;
					_holding = true;
					_touchIdLegacy = _touchId;
					_skyLogic->startInventory(_touchId); // highlight touched item
				}
			}
			// touching something - we're done
			return;
		}
	}
	// clicked outside the inventory, close it
	if (!_touchId && buttonHeld) {
		if (xPos < _invX || xPos > _invX + _invW || yPos < _invY || yPos > _invY + _invH) {
			_mMode = MUST_RELEASE;
			_skyLogic->killInventory();
			return;
		}
	}
	_holding = false;
	_touchId = 0;
	_timeOn = 0;

	if (Logic::_scriptVariables[GET_OFF]) {
		_skyLogic->mouseScript(Logic::_scriptVariables[GET_OFF], itemData);
		Logic::_scriptVariables[GET_OFF] = 0;
	}
}

void Mouse::invMouse(uint16 xPos, uint16 yPos) {
	uint32 *objList;
	Compact *itemData = nullptr;
	int j, num;
	bool touched = false;

	debug(1, "xPos = %d, yPos = %d, _invX = %d, _invY = %d, _invW = %d, _invH = %d", xPos, yPos, _invX, _invY, _invW, _invH);

	bool buttonHeld = (_system->getEventManager()->getButtonState() != 0); // mouseEngine() consumes the click at the end so using _mouseB would increment the timer at each click not on hold

	objList = _skyLogic->giveInvList();

	num = (int)Logic::_scriptVariables[MENU_LENGTH];

	// held for more than the threshold time
	if (_holding) {
		debug(1, "Holding an object");

		// if moved(i.e., taken out of the item's original dimension)
		if (xPos < _invX || yPos < _invY || xPos > _invX + _invW || yPos > _invY + _invH) {

			// for invUseOn() to drag object around the screen
			_mMode = INVENTORY_USE_ON;

			// remove all the inv items from the screen when one is being dragged
			_skyLogic->killInventory();

			_touchId = 0;
			_skyScreen->setDragIconHighlight(false);

			return;
		}

		// still holding the button, just update the hover text for the item it is dragging over and stop
		if (buttonHeld) {
			for (j = 0; j < num; j++) {
				itemData = _skyCompact->fetchCpt(objList[j]);
				debug(1, "itemData->xcood = %d, itemData->mouseRelX = %d, itemData->ycood = %d, itemData->mouseRelY = %d", itemData->xcood, itemData->mouseRelX, itemData->ycood, itemData->mouseRelY);

				if (itemData->xcood + (uint16)itemData->mouseRelX > xPos)
					continue;
				if (itemData->xcood + (uint16)itemData->mouseRelX + XWIDTH < xPos)
					continue;
				if (itemData->ycood + (uint16)itemData->mouseRelY > yPos)
					continue;
				if (itemData->ycood + (uint16)itemData->mouseRelY + YDEPTH < yPos)
					continue;

				touched = true;

				if (_hoverId == objList[j]) // still on previous
					continue;

				// run previous item's GET_OFF, if there was one
				if (Logic::_scriptVariables[GET_OFF])
					_skyLogic->mouseScript(Logic::_scriptVariables[GET_OFF], _skyCompact->fetchCpt(_hoverId));

				Logic::_scriptVariables[GET_OFF] = itemData->mouseOff;
				_hoverId = objList[j];

				// run the mouseOn script
				if (itemData->mouseOn) {
					uint16 tempY = itemData->ycood;
					itemData->ycood = 136;
					Logic::_scriptVariables[BUTTON] = 2;

					// record what we are touching
					Logic::_scriptVariables[SPECIAL_ITEM] = _hoverId;
					_skyLogic->mouseScript(itemData->mouseOn, itemData);

					// reset the render position to original
					itemData->ycood = tempY;
				}

				_skyLogic->startInventory(_hoverId);
			}
			if (!touched) {
				// touching nothin
				_hoverId = 0;
				if (Logic::_scriptVariables[GET_OFF]) {
					_skyLogic->mouseScript(Logic::_scriptVariables[GET_OFF], _skyCompact->fetchCpt(_hoverId));
					Logic::_scriptVariables[GET_OFF] = 0;
				}
			}
			return;
		}

		// dropped on another item
		// fetch the compact
		itemData = _skyCompact->fetchCpt(_touchId);
		// cancel cursor
		Logic::_scriptVariables[OBJECT_HELD] = 0;

		// check for the actual two objects that can combine like touhchId == 37 and hoverId == 74 etc.
		if ((_touchId == 74 && _hoverId == 37) || (_touchId == 37 && _hoverId == 74)) {
			Logic::_scriptVariables[83] = 0;
			Logic::_scriptVariables[89] = 0;
			Logic::_scriptVariables[90] = 39;
			debug(1, "Combined");

			_skyLogic->killInventory();
			_skyLogic->startInventory();

			_touchId = 0;
		} else {
			// dropping on an item that can't combine
			_touchId = 0;
			debug(1, "Using on invalid item");
		}

		_skyScreen->clearDragIcon();
		_skyLogic->startInventory();
	}

	if (buttonHeld) {
		for (j = 0; j < num; j++) {
			itemData = _skyCompact->fetchCpt(objList[j]);

			if (itemData->xcood + (int16)itemData->mouseRelX > xPos)
				continue;
			if (itemData->xcood + (int16)itemData->mouseRelX + XWIDTH < xPos)
				continue;
			if (itemData->ycood + (int16)itemData->mouseRelY > yPos)
				continue;
			if (itemData->ycood + (int16)itemData->mouseRelY + YDEPTH < yPos)
				continue;

			// record what we are touching
			Logic::_scriptVariables[SPECIAL_ITEM] = objList[j];

			// if we are on an object that is different from the object we were over just a moment ago
			if (_touchId != objList[j] && !_holding) {
				debug(1, "Entered the if block");
				if (Logic::_scriptVariables[GET_OFF])
					_skyLogic->mouseScript(Logic::_scriptVariables[GET_OFF], itemData);

				// update GET_OFF
				Logic::_scriptVariables[GET_OFF] = itemData->mouseOff;

				_touchId = objList[j];
				_hoverId = objList[j];
				// run the mouseOn script
				_skyLogic->mouseScript(itemData->mouseOn, itemData);
				// reset hold counter
				_timeOn = 0;
				_skyLogic->startInventory(_touchId);
			} else {
				// holding
				_timeOn++;
				debug(1, "Holding item - %d, timer - %d", _touchId, _timeOn);

				// if held long enough
				if (_timeOn == USEON_THRESHOLD) {
					debug(1, "Threshold crossed");
					_skyScreen->setDragIcon(itemData->frame, true);

					// hold the item
					Logic::_scriptVariables[OBJECT_HELD] = _touchId;

					_mouseYOff = HOTSPOT_USE_ON_YOFF;

					_holding = true;
				}
			}
			return;
		}
	}

	// made a quick click and release
	if (!buttonHeld && _touchId && _timeOn < CLICK_THRESHOLD) {
		Logic::_scriptVariables[BUTTON] = 2;
		Logic::_scriptVariables[SPECIAL_ITEM] = _touchId;

		itemData = _skyCompact->fetchCpt(_touchId);

		uint16 tempY = itemData->ycood;
		pushInvY(tempY);
		itemData->ycood = 136;

		_mMode = INV_TEMP_EXAMINE;

		_skyLogic->mouseScript(itemData->mouseClick, itemData);

		_mMode = INVENTORY;

		_skyScreen->clearDragIcon();
		_skyLogic->startInventory();
	}

	// if clicked somewhere outside the inventory, close the inventory
	if (!_touchId && buttonHeld) {
		if (xPos < _invX || xPos > _invX + _invW || yPos < _invY || yPos > _invY + _invH) {

			_mMode = MUST_RELEASE; // MUST_RELEASE because if we go straight to GAMEPLAY, robert will move to the place of clicking
			_skyLogic->killInventory(); // stop rendering the inventory items
			return;
		}
	}

	_holding = false;
	_touchId = false;

	// reset
	if (Logic::_scriptVariables[GET_OFF]) {
		_skyLogic->mouseScript(Logic::_scriptVariables[GET_OFF], itemData);
		Logic::_scriptVariables[GET_OFF] = 0;
	}
}

// dragging the inventory items around the screen and releasing to use
void Mouse::invUseOn(uint16 xPos, uint16 yPos) {
	Compact *itemData;
	uint32 itemNum;
	uint16 midX, midY;
	bool buttonHeld = (_system->getEventManager()->getButtonState() != 0);

	_logicClick = buttonHeld;

	if (buttonHeld) {
		itemNum = doProximityHighlights(xPos, yPos);
		// without this, it would calculate Compact 0 if hovering over a non interactive item
		if (!itemNum)
			return;
		debug(1, "Item %d is being dragged", _touchId);
		// what are we near
		itemNum = doProximityHighlights(xPos, yPos);
		itemData = _skyCompact->fetchCpt(itemNum);
		midX = giveXCood(itemData, itemNum);
		midY = giveYCood(itemData, itemNum);
		int d = abs(xPos - midX) + abs(yPos - midY);
		if (d < USE_ON_DIST) {
			if (_touchId != itemNum) {
				// run previous item's GET_OFF
				if (Logic::_scriptVariables[GET_OFF])
					_skyLogic->mouseScript(Logic::_scriptVariables[GET_OFF], itemData);

				Logic::_scriptVariables[GET_OFF] = itemData->mouseOff;
				// record what we're touching
				Logic::_scriptVariables[SPECIAL_ITEM] = itemNum;
				_touchId = itemNum;
				if (itemData->mouseOn) {
					_skyLogic->mouseScript(itemData->mouseOn, itemData);
					_skyScreen->setDragIconHighlight(true);
				} else {
					_skyScreen->setDragIconHighlight(false);
				}
			}
			// touching something
			_skyScreen->setProximityNotAnimate(_nearestProximityIconId);
			return;
		}
		// touching nothing
		// run previous item's GET_OFF if there was one
		if (Logic::_scriptVariables[GET_OFF]) {
			_skyScreen->setDragIconHighlight(false); // highlight off
			itemData = _skyCompact->fetchCpt(_touchId);
			_skyLogic->mouseScript(Logic::_scriptVariables[GET_OFF], itemData);
			Logic::_scriptVariables[GET_OFF] = 0;
		}
		_touchId = 0; // clear
	} else { // release on another object
		_skyScreen->clearAllProximityIcons();
		// run previous item's GET_OFF if there was one
		if (Logic::_scriptVariables[GET_OFF]) {
			itemData = _skyCompact->fetchCpt(_touchId);
			_skyLogic->mouseScript(Logic::_scriptVariables[GET_OFF], itemData);
			Logic::_scriptVariables[GET_OFF] = 0;
		}
		if (_touchId) {
			if (g_engine->canSaveGameStateCurrently())
				g_engine->saveGameState(0, "Autosave", true);
			Logic::_scriptVariables[BUTTON] = 3;
			itemData = _skyCompact->fetchCpt(_touchId);
			if (itemData->mouseClick && itemData->mouseOn) {
				Logic::_scriptVariables[SAFE_LOGIC_LIST] = Logic::_scriptVariables[LOGIC_LIST_NO];
				_skyLogic->mouseScript(itemData->mouseClick, itemData);
			}
		}
		resetUI();
		_touchId = 0;
		_mMode = GAMEPLAY;
		_skyScreen->clearDragIcon();
	}
}

} // End of namespace Sky
