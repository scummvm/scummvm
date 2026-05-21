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
	M_MODE = GAMEPLAY; // master ui mode
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

	if (M_MODE == GAMEPLAY)
		M_MODE = MUST_RELEASE;

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

	if (SkyEngine::isIbass()) {
		debug("ibass cursor path executed\n");
		CursorMan.setDefaultArrowCursor();
		CursorMan.showMouse(true);
		return;
	}

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

	if (SkyEngine::isIbass()) {
		if (_actionFlash) {
			_actionFlashTime--;
			if (!_actionFlashTime)
				_actionFlash = false;

			if (!(_actionFlashTime & 1))
				_skyScreen->clearIbassIcon(_actionFlashIcon, false);
			else
				_skyScreen->setIcon(_actionFlashIcon, _actionFlashX, _actionFlashY);
		}

		switch (M_MODE) {
			case ALERT_TO_GAME:
				if (!_mouseB)
					return;

				_skyScreen->hideInventory();
				M_MODE = MUST_RELEASE;
				break;

			case MUST_RELEASE:
				if (_mouseB)
					return;

				resetUI();
				M_MODE = GAMEPLAY;
				break;

			case GAMEPLAY:
				pointerEngine(_mouseX + TOP_LEFT_X, _mouseY + TOP_LEFT_Y);
				break;

			case PRE_INVENTORY:
				if (_mouseB)
					return;

				M_MODE = INVENTORY;
				break;

			case INVENTORY:
				if (isLincInv())
					lincInvMouse(_mouseX + TOP_LEFT_X, _mouseY + TOP_LEFT_Y);

			case INVENTORY_USE_ON:
			case TEXT_CHOOSER:
				break;
		}

		_mouseB = 0;
		return;
	}

	if (!Logic::_scriptVariables[MOUSE_STOP]) {
		if (Logic::_scriptVariables[MOUSE_STATUS] & (1 << 1)) {
			pointerEngine(_mouseX + TOP_LEFT_X, _mouseY + TOP_LEFT_Y);
			if (Logic::_scriptVariables[MOUSE_STATUS] & (1 << 2)) //buttons enabled?
				buttonEngine1();
		}
	}
	_mouseB = 0;	//don't save up buttons
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

	static int  proxFrame = 0;
	static int  proxFrameSpeed = 0;

	proxFrameSpeed++;
	if (proxFrameSpeed & 1)
		proxFrame++;
	if (proxFrame == 3)
		proxFrame = 0;

	// near inv button?
	if (yPos > HOTSPOT_invy && xPos < HOTSPOT_invx)
		return  0;

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
					midx = giveXcood(itemData, itemNum);
					midy = giveYcood(itemData, itemNum);

					// skip very high hotspots - eg the old inventory catch object
					if (midy < (TOP_LEFT_Y + 5))
						continue;

					int d = abs(xPos - midx) + abs(yPos - midy);
					if (d < GLOW_DIST) {
						float opacity = 1.0 - ((d * 1.0) / GLOW_DIST);
						_skyScreen->setProximityIcon(found++, midx - TOP_LEFT_X - 4, ((midy - TOP_LEFT_Y) - 4), opacity, proxFrame);
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
	}
	while (*currentList != 0);

	return	nearestId;
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

uint16 Mouse::giveXcood(Compact *itemData, uint32 id) {
	uint16 mid, midy;

	mid = itemData->xcood + ((int16)itemData->mouseRelX) + (itemData->mouseSizeX >> 1);
	midy = itemData->ycood + ((int16)itemData->mouseRelY) + (itemData->mouseSizeY >> 1);

	if (mid < HOTSPOT_invx && midy > HOTSPOT_invy)
		return  HOTSPOT_invx + 16;


	// not lamb
	if (id != 16) {
		if (mid > 428)
			return  428;
		if (mid < 142)
			return  142;
	}

	switch (id) {
	case 4112: {
		mid = 258; // cupboard door
		break;
	}
	case 4113: {
		mid = 254; // sandwich
		break;
	}
	case 4114: {
		mid = 254; // spanner
		break;
	}

	case 12358:  {
		mid = 428; // bellevue - missing exit
		break;
	}
	case 12681:  {
		mid = 428; // bellevue
		break;
	}
	case 12349:  {
		mid = 142; // bellevue
		break;
	}
	case 12347:  {
		mid = 428; // bellevue
		break;
	}
	case 90: {
		mid = 404; // first room, door
		break;
	}
	case 97: {
		mid = 190; // ledge room, door
		break;
	}
	case 4116: {
		mid = 269; // elevator
		break;
	}
	case 4119: {
		mid = 269; // hole
		break;
	}
	case 4110: {
		mid = 356; // lathe
		break;
	}
	case 8248: {
		mid = 357; // walkway to security lobby
		break;
	}
	case 8238: {
		mid = 181; // walkway to power
		break;
	}
	case 8317: {
		mid = 426; // walkway to crash
		break;
	}
	case 8341: {
		mid = 173; // factory to walkway
		break;
	}
	case 8344: {
		mid = 433; // factory to factory 2
		break;
	}
	case 8500: {
		mid = 224; // sensors
		break;
	}
	case 8355: {
		mid = 428; // factory 2 to factory 3
		break;
	}
	case 8446: {
		mid = 226; // putty
		break;
	}
	case 8438: {
		mid = 226; // gangway
		break;
	}
	case 12680: {
		mid = 201; // exit
		break;
	}
	case 12679: {
		mid = 307; // exit
		break;
	}
	case 12678: {
		mid = 179; // exit
		break;
	}
	case 12677: {
		mid = 319; // exit
		break;
	}
	case 12324: {
		mid = 191; // exit
		break;
	}
	case 12336: {
		mid = 182; // exit
		break;
	}
	case 12442: {
		mid = 376; // exit
		break;
	}
	case 8483: {
		mid = 225; // console
		break;
	}
	case 12641: {
		mid = 158; // exit
		break;
	}

	// linc
	case 12633: {
		mid = 190; // maze
		break;
	}
	case 24586: {
		mid = 210; // maze
		break;
	}
	case 24592: {
		mid = 288; // maze mid-bot
		break;
	}
	case 24593: {
		mid = 400; // maze mid-bot
		break;
	}
	case 24594: {
		mid = 203; // maze left-mid
		break;
	}
	case 24595: {
		mid = 299; // maze left-mid
		break;
	}


	// hyde
	case 16496: {
		mid = 263; // plant
		break;
	}
	// cathedral
	case 16462: {
		mid = 168; // exit
		break;
	}

	// lockers
	case 16576: {
		mid = 246; // body == 16569
		break;
	}
	case 16577: {
		mid = 271; // body == 16570
		break;
	}
	case 16578: {
		mid = 296; // body == 16571
		break;
	}
	case 16579: {
		mid = 321; // body == 16572
		break;
	}
	case 16580: {
		mid = 346; // body == 16573
		break;
	}

	// underworld
	case 20506: {
		mid = 389; // metal door
		break;
	}
	// pit room
	case 20600: {
		mid = 283; // cover
		break;
	}
	case 20648: {
		mid = 283; // pit
		break;
	}
	case 20570: {
		mid = 247; // exit to medical droid room
		break;
	}

	case 20575: {
		mid = 277; // slot
		break;
	}
	case 20577: {
		mid = 297; // recharge unit
		break;
	}

	case 24786: {
		mid = 288; // linc crystal
		break;
	}
	case 24787: {
		mid = 288; // virus
		break;
	}

	case 20712: {
		mid = 305; // console
		break;
	}
	case 20713: {
		mid = 356; // console
		break;
	}

	case 20728: {
		mid = 388; // exit from door room
		break;
	}

	case 8272: {
		mid = 278; // power room chair
		break;
	}

	case 12390: {
		mid = 187; // burke door
		break;
	}
	case 12541: {
		mid = 187; // burke exit
		break;
	}

	}

	return  mid;
}

uint16 Mouse::giveYcood(Compact *itemData, uint32 id) {
	// push objects that are too far to left, or to the right, back onto the screen
	uint16 mid;

	mid = itemData->ycood + ((int16)itemData->mouseRelY) + (itemData->mouseSizeY >> 1);

	switch (id) {
	case 4112: { // cupboard door - not if open
		if (Logic::_scriptVariables[112]) // cupb_flag
			return  0;
		mid = 221;
		break;
	}
	case 4113: {
		mid = 218; // sandwich
		break;
	}
	case 4114: {
		mid = 226; // spanner
		break;
	}

	case 4108: {
		mid = 0; // remove monitor screen
		break;
	}
	case 4111: {
		mid = 0; // remove lazer
		break;
	}
	case 4115: {
		mid = 0; // remove monitor screen
		break;
	}
	case 4117: {
		mid = 0; // remove monitor screen
		break;
	}
	case 4154: {
		mid = 0; // remove right sign
		break;
	}
	case 12349:
	// fall through
	case 12347:
	case 12358:  {
		mid = TWEEKY_EXIT_ADJUST; // bellevue
		break;
	}
	case 90: {
		mid = 208; // first room, door
		break;
	}
	case 97: {
		mid = 216; // ledge room, door
		break;
	}
	case 4116: {
		mid = 259; // elevator
		break;
	}
	case 4119: {
		mid = 259; // hole
		break;
	}
	case 4315: {
		mid = 261; // exit left from junk room
		break;
	}
	case 4103: {
		mid = 242; // exit right from junk room
		break;
	}
	case 4105: {
		mid = 241; // exit left from hobbins room
		break;
	}
	case 4110: {
		mid = 230; // lathe
		break;
	}
	case 8248: {
		mid = 222;  // walkway to security lobby
		break;
	}
	case 8238: {
		mid = 222; // walkway to power
		break;
	}
	case 8310: {
		mid = 222; // walkway to first room
		break;
	}
	case 8246: {
		mid = 273; // power exit
		break;
	}
	case 8317: {
		mid = 273; // walkway to crash
		break;
	}
	case 8331: {
		mid = 230; // walkway to factory
		break;
	}
	case 8341: {
		mid = 239; // factory to walkway
		break;
	}
	case 8344: {
		mid = 282; // factory to factory 2
		break;
	}
	case 8500: {
		mid = 197; // sensors
		break;
	}
	case 8355: {
		mid = 255; // factory 2 to factory 3
		break;
	}
	case 8366: {
		mid = 227; // factory 2 to storeroom
		break;
	}
	case 8375: {
		mid = 238; // storeroom to factory 2
		break;
	}
	case 8446: {
		mid = 247; // putty
		break;
	}
	case 8438: {
		mid = 247; // gangway
		break;
	}
	case 8364: {
		mid = 252; // exit
		break;
	}
	case 8459: {
		mid = 252; // exit
		break;
	}
	case 8256: {
		mid = 297; // exit
		break;
	}

	// bellevue
	case 12304: {
		mid = 252; // exit
		break;
	}
	case 12338: {
		mid = 286; // exit
		break;
	}
	case 12681: {
		mid = 250; // exit
		break;
	}
	case 12370: {
		mid = 289; // exit
		break;
	}
	case 12361: {
		mid = 267; // exit
		break;
	}
	case 12679: {
		mid = 179; // exit
		break;
	}
	case 12678: {
		mid = 179; // exit
		break;
	}
	case 12677: {
		mid = 179; // exit
		break;
	}
	case 12313: {
		mid = 284; // exit
		break;
	}
	case 12676: {
		mid = 241; // exit
		break;
	}
	case 12327: {
		mid = 244; // exit
		break;
	}
	case 12315: {
		mid = 286; // exit
		break;
	}
	case 12324: {
		mid = 308; // exit
		break;
	}
	case 12336: {
		mid = 306; // exit
		break;
	}
	case 12442: {
		mid = 246; // exit
		break;
	}
	case 12447: {
		mid = 242; // exit
		break;
	}
	case 12680: {
		mid = 179; // exit
		break;
	}
	case 12459: {
		mid = 281; // exit
		break;
	}
	case 12471: {
		mid = 302; // exit
		break;
	}
	case 12399: {
		mid = 257; // exit
		break;
	}
	case 12390: {
		mid = 264; // burke door
		break;
	}
	case 12541: {
		mid = 264; // burke exit
		break;
	}

	case 12630: {
		mid += 4; // slot
		break;
	}
	case 12616: {
		mid += 4; // slot
		break;
	}
	case 12629: {
		mid += 4; // slot
		break;
	}
	case 12628: {
		mid += 4; // slot
		break;
	}
	case 12627: {
		mid += 4; // slot
		break;
	}
	case 12626: {
		mid += 4; // slot
		break;
	}

	case 12620: {
		mid = 220; // locker
		break;
	}
	case 12621: {
		mid = 220; // locker
		break;
	}
	case 12622: {
		mid = 220; // locker
		break;
	}
	case 12623: {
		mid = 220; // locker
		break;
	}
	case 12613: {
		mid = 220; // locker
		break;
	}
	case 12624: {
		mid = 220; // locker
		break;
	}

	case 12631: {
		mid = 273; // exit
		break;
	}
	case 12641: {
		mid = 290; // exit
		break;
	}
	case 12642: {
		mid = 273; // exit
		break;
	}

	case 12474: {
		mid = 282; // lamb door
		break;
	}
	case 12486: {
		mid = 298; // lamb door
		break;
	}

	// security room
	case 8295: {
		mid = 299; // exit
		break;
	}

	// hyde
	case 16415: {
		mid = 261; // exit
		break;
	}
	case 16403: {
		mid = 261; // exit
		break;
	}
	case 16393: {
		mid = 246; // exit
		break;
	}
	case 16487: {
		mid = 256; // exit
		break;
	}
	case 16492: {
		mid = 287; // exit
		break;
	}
	case 16394: {
		mid = 313; // exit
		break;
	}
	case 16412: {
		mid = 276; // exit
		break;
	}
	case 16428: {
		mid = 233; // exit
		break;
	}
	case 16424: {
		mid = 236; // exit
		break;
	}


	// cathedral
	case 16462: {
		mid = 296; // exit
		break;
	}
	case 16464: {
		mid = 250; // exit
		break;
	}
	case 16465: {
		mid = 226; // exit
		break;
	}
	case 16474: {
		mid = 263; // exit
		break;
	}

	case 16576: {
		mid = 229; // body == 16569
		break;
	}
	case 16577: {
		mid = 229; // body == 16570
		break;
	}
	case 16578: {
		mid = 229; // body == 16571
		break;
	}
	case 16579: {
		mid = 229; // body == 16572
		break;
	}
	case 16580: {
		mid = 229; // body == 16573
		break;
	}

	// reactor
	case 8478: {
		mid = 291; // exit
		break;
	}
	case 8481: {
		mid = 253; // exit
		break;
	}
	case 8511: {
		mid = 271; // exit
		break;
	}

	// club
	case 16538: {
		mid = 279; // exit
		break;
	}

	// abandoned subway
	case 16439: {
		mid = 267; // exit
		break;
	}

	// entrance to underworld
	case 16592: {
		mid = 267; // exit
		break;
	}

	// underworld
	case 16649: {
		mid = 291; // exit
		break;
	}
	case 16660: {
		mid = 285; // exit
		break;
	}
	case 16662: {
		mid = 254; // exit
		break;
	}
	case 16661: {
		mid = 291; // exit
		break;
	}
	case 16671: {
		mid = 291; // exit
		break;
	}
	case 16681: {
		mid = 282; // exit
		break;
	}
	case 16682: {
		mid = 282; // exit
		break;
	}
	case 16719: {
		mid = 287; // exit
		break;
	}
	case 16720: {
		mid = 287; // exit
		break;
	}
	case 16729: {
		mid = 291; // exit
		break;
	}
	case 16730: {
		mid = 301; // exit
		break;
	}
	case 20506: {
		mid = 276; // metal door
		break;
	}
	// underworld main
	case 20518: {
		mid = 276; // metal door
		break;
	}
	case 20532: {
		mid = 264; // exit
		break;
	}
	case 20558: {
		mid = 296; // exit
		break;
	}
	case 20598: {
		mid = 237; // exit
		break;
	}
	case 20601: {
		mid = 183; // metal bar
		break;
	}
	case 20600: {
		mid = 237; // cover
		break;
	}
	case 20648: {
		mid = 237; // pit
		break;
	}
	case 20570: {
		mid = 268; // exit to medical droid room
		break;
	}

	case 20573: {
		mid = 290; // droid room exit
		break;
	}
	case 20579: {
		mid = 272; // droid room exit
		break;
	}
	case 20572: {
		mid = 277; // droid room exit
		break;
	}
	case 20578: {
		mid = 286; // console
		break;
	}
	case 20674: {
		mid = 279; // droid room exit
		break;
	}

	case 20675: {
		mid = 238; // robot
		break;
	}
	case 20511: {
		mid = 259; // slot
		break;
	}

	case 20583: {
		mid = 291; // tank room exit
		break;
	}
	case 20604: {
		mid = 264; // tank room exit
		break;
	}
	case 20676: {
		mid = 294; // exit
		break;
	}
	case 20619: {
		mid = 269; // exit
		break;
	}
	case 20628: {
		mid = 284; // exit
		break;
	}
	case 20617: {
		mid = 295; // exit
		break;
	}

	case 24786: {
		mid = 256; // linc crystal
		break;
	}
	case 24787: {
		mid = 256; // linc crystal
		break;
	}

	case 20672: {
		mid = 265; // android room exit
		break;
	}
	case 20696: {
		mid = 265; // android room exit
		break;
	}
	case 20697: {
		mid = 265; // android room exit
		break;
	}
	case 20715: {
		mid = 265; // android room exit
		break;
	}

	case 20711: {
		mid = 219; // console
		break;
	}
	case 20712: {
		mid = 219; // console
		break;
	}
	case 20713: {
		mid = 219; // console
		break;
	}

	case 20725: {
		mid = 268; // exit
		break;
	}

	case 20726: {
		mid = 270; // exit from door room
		break;
	}
	case 20728: {
		mid = 219; // exit from door room
		break;
	}

	case 20741: {
		mid = 254; // exit from first vein room
		break;
	}
	case 20743: {
		mid = 279; // exit from first vein room
		break;
	}
	case 20752: {
		mid = 279; // exit near end
		break;
	}
	case 20886: {
		mid = 243; // exit orifice to linc
		break;
	}

	case 20874: {
		mid = 0; // pipe support - removed as appears to have no interaction
		break;
	}

	// linc
	case 24592: {
		mid = 306; // maze mid-bot
		break;
	}
	case 24634: {
		mid = 306; // maze mid-bot
		break;
	}
	case 24594: {
		mid = 278; // maze left-mid
		break;
	}

	case 8272: {
		mid = 0; // REMOVE power room chair
		break;
	}

	case 8290:
	case 8291: { // power room switches
		if (!Logic::_scriptVariables[429])
			return  0;
	}
	}

	return mid;
}

int Mouse::touchingFloor(uint16 xPos, uint16 yPos) {
	uint32 currentListNum = Logic::_scriptVariables[MOUSE_LIST_NO];
	uint16 *currentList;
	Compact *itemData;
	uint16 itemNum;

	// do not detect floors beyond a certain depth
	if (yPos > (HOTSPOT_invy + 8)) return 0;
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

	if (SkyEngine::isIbass()) {
		int midx, midy; // open hotspot dimensions
		Compact *itemData;
		int itemProx;
		int d = 0;

		if (yPos < (TOP_LEFT_Y + 5))
			return;

		// mouse must be down to search for stuff
		if (_mouseB) {
			// debug("Clicked");
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

					midx = giveXcood(itemData, _touchId);
					midy = giveYcood(itemData, _touchId);
					// calc dist
					d = abs(xPos - midx) + abs(yPos - midy);

					// some left hand edge adjust for twin-hotspot non-exits
					if (!hasSingleInteractIcon(_touchId) && !_isExit)
						if ((midx - HOTSPOT_dim) < TOP_LEFT_X)
							midx = TOP_LEFT_X + HOTSPOT_dim;

					// still touching a normal hotspot?
					if (!_isExit && midx - HOTSPOT_dim < xPos && midx + HOTSPOT_dim > xPos && midy - HOTSPOT_yoff < yPos && midy/*+HOTSPOT_yoff*/ > yPos) {
						// if newly touching, and same item as last time, then inc the click count
						if (_prevMouseOn == false) {

							_clickedNum++;  // one more click
							// reset fade out timer to max
							_fadeOut = HOTSPOT_FADEOUT;
							// reset hover time
							_timeOn = 1;
						} else
							_timeOn++;

						int icony = midy - HOTSPOT_yoff;
						icony -= TOP_LEFT_Y; // normalise for renderer
						if (icony < 0)
							icony = 0;

						if (hasSingleInteractIcon(_touchId)) {
							// update the coordinate of the hotspot
							midx -= TOP_LEFT_X;
							midx -= HOTSPOT_ExitDim;

							// draw the icons
							_skyScreen->setIcon(getInteractIcon(_touchId), midx + 4, icony);
							_actionFlashX = midx + 4;
							_actionFlashIcon = getInteractIcon(_touchId);
							_actionFlashY = icony;

						} else { // 2 icons
							midx -= TOP_LEFT_X;
							midx -= HOTSPOT_dim;

							_actionFlashY = icony;

							if (xPos <= (midx + HOTSPOT_dim + TOP_LEFT_X)) {
								Logic::_scriptVariables[BUTTON] = 2;
								_actionFlashX = midx;
								_actionFlashIcon = UI_ICON_LOOK;
							} else {
								Logic::_scriptVariables[BUTTON] = 1;
								_actionFlashX = midx + HOTSPOT_dim + 4;
								_actionFlashIcon = getInteractIcon(_touchId);
							}
						}

						// mouse on
						_prevMouseOn = true;

						// end here, when button held on something
						return;
					} else if (_isExit && midx - HOTSPOT_dim < xPos && midx + HOTSPOT_dim > xPos && midy - HOTSPOT_yoff < yPos && midy/*+HOTSPOT_yoff*/ > yPos) {
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
							int iconx = midx - HOTSPOT_ExitDim;
							iconx -= TOP_LEFT_X; // normalise for renderer
							if (iconx < 0)
								iconx = 0;

							int icony = midy - HOTSPOT_exit_yoff;
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
				midx = giveXcood(itemData, itemProx);
				midy = giveYcood(itemData, itemProx);
				// calc dist
				d = abs(xPos - midx) + abs(yPos - midy);
			}

			// not close enough to real hotspot, and still touching previous floor
			if (floor && (d >= USE_ON_DIST) && floor == _touchId)
				// if we skip floor, and dont hit another hotspot, then we quit after this search, because there is no fadeOut on floors, so the system resets the couter
				// floorSkip=true;
				// continue;
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
					debug("Fadeout det to 36, mouseOn = %d, dist = %d", itemData->mouseOn, d);

					_skyScreen->setProximityNotAnimate(_nearestProximityIconId); // we want this one remaining one to not animate

					// assume not
					_isExit = false;
					_isFloor = false;

					_skyLogic->mouseScript(itemData->mouseOn, itemData);

					// exit, or hotspot?
					if (_isExit) {

						int iconx = midx - HOTSPOT_ExitDim;
						iconx -= TOP_LEFT_X; // normalise for renderer
						if (iconx < 0)
							iconx = 0;

						int icony = midy - HOTSPOT_exit_yoff;
						icony -= TOP_LEFT_Y; // normalise for renderer
						if (icony < 0)
							icony = 0;

						// draw the icon
						initExitIcon(_exitType, iconx, icony);
					} else {
						int icony = midy - HOTSPOT_yoff;
						icony -= TOP_LEFT_Y;// normalise for renderer
						if (icony < 0)
							icony = 0;

						if (hasSingleInteractIcon(_touchId)) {
							// update the coordinate of the hotspot
							int iconx = midx - HOTSPOT_ExitDim;
							iconx -= TOP_LEFT_X;// normalise for renderer
							if (iconx < 0)
								iconx = 0;

							// draw the icons

							_skyScreen->setIcon(getInteractIcon(_touchId), iconx + 4, icony);

							if (_touchId == 70 || _touchId == 69 || _touchId == 24633 || _touchId == 24634 || _touchId == 4119 || _touchId == 8210) // stairs on screen 1 - normal object turned into exit - need to hack the button - the rest are either talk-to, which doesnt seem to matter, or look-at, like posters on walls
								Logic::_scriptVariables[BUTTON] = 1;
							else
								Logic::_scriptVariables[BUTTON] = 2;

						} else { // 2 icons
							// update the coordinate of the hotspot
							int iconx = midx - HOTSPOT_dim;
							iconx -= TOP_LEFT_X;// normalise for renderer
							if (iconx < 0)
								iconx = 0;

							// draw the icons
							_skyScreen->setIcon(UI_ICON_LOOK, iconx, icony);
							_skyScreen->setIcon(getInteractIcon(_touchId), iconx + HOTSPOT_dim + 4, icony);
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
				debug("nothing");
				// not touching anything, force open hotspot to close
				if (_fadeOut)
					_fadeOut = 0;
			}
			// not touching a game object - but what about HU buttons, such as Inv?
			if (!_prevMouseOn && 101 != Logic::_scriptVariables[SCREEN] && Logic::_scriptVariables[LOGIC_LIST_NO] != 24765) {
				if (xPos < HOTSPOT_invx && yPos > HOTSPOT_invy) {
					_skyLogic->startInventory();
					_touchId = 0;
					_holding = false;
					M_MODE = PRE_INVENTORY;
					_skyScreen->clearAllProximityIcons(false);
					_skyScreen->clearAllIbassIcons(false);
					return;
				}
				// control panel
				if (xPos < HOTSPOT_optionsx && yPos < HOTSPOT_optionsy) {
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


			debug("floor returned = %d", floor);
		} else { // not touching screen
			if (_prevMouseOn) {

				if (!_touchId || _isFloor)
					_skyScreen->clearAllProximityIcons();

				else if (_touchId) {
					_skyScreen->clearAllProximityIcons();
					// clear all but the one relating to the highlighted hotspot - hmmm
					itemData = _skyCompact->fetchCpt(_touchId);
					midx = giveXcood(itemData, _touchId);
					midy = giveYcood(itemData, _touchId); //itemData->ycood + ((int16)itemData->mouseRelY) + (itemData->mouseSizeY>>1);
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

				// an sfx

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

void Mouse::initExitIcon(uint32 type, int iconx, int icony) {
	switch (type) {
	case MOUSE_LEFT: {
		_actionFlashIcon = UI_ICON_LEFT;
		_actionFlashX = iconx + 8;
		_skyScreen->setIcon(UI_ICON_LEFT, iconx + 8, icony);
		break;
	}
	case MOUSE_RIGHT: {
		_actionFlashIcon = UI_ICON_RIGHT;
		_actionFlashX = iconx + 4;
		_skyScreen->setIcon(UI_ICON_RIGHT, iconx + 4, icony);
		break;
	}
	case MOUSE_UP: {
		_actionFlashIcon = UI_ICON_UP;
		_actionFlashX = iconx + 5;
		_skyScreen->setIcon(UI_ICON_UP, iconx + 5, icony);
		break;
	}
	case MOUSE_DOWN: {
		_actionFlashIcon = UI_ICON_DOWN;
		_actionFlashX = iconx + 5;
		_skyScreen->setIcon(UI_ICON_DOWN, iconx + 5, icony);
		break;
	}
	}
}

void Mouse::updateHotspotCoordinate(uint16 xPos) {
	Compact *itemData;
	int midx,  midy;

	// fetch the compact
	itemData = _skyCompact->fetchCpt(_touchIdLegacy);

	if (!itemData)
		return;

	// if not a floor
	if (itemData->mouseOn && !_isExit) {

		midx = giveXcood(itemData, _touchIdLegacy);
		midy = giveYcood(itemData, _touchIdLegacy);

		int icony = midy - HOTSPOT_yoff;
		icony -= TOP_LEFT_Y; // normalise for renderer
		if (icony < 0)
			icony = 0;

		if (hasSingleInteractIcon(_touchIdLegacy)) {
			// update the coordinate of the hotspot
			int iconx = midx - HOTSPOT_ExitDim;
			iconx -= TOP_LEFT_X;//normalise for renderer
			if (iconx < 0)
				iconx = 0;

			// draw the icons
			_skyScreen->setIcon(getInteractIcon(_touchIdLegacy), iconx + 4, icony);
		} else {
			// update the coordinate of the hotspot
			int iconx = midx - HOTSPOT_dim;
			iconx -= TOP_LEFT_X;//normalise for renderer
			if (iconx < 0)
				iconx = 0;

			// draw the icons
			_skyScreen->setIcon(UI_ICON_LOOK, iconx, icony);
			_skyScreen->setIcon(getInteractIcon(_touchIdLegacy), iconx + HOTSPOT_dim + 4, icony);

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
		if (Logic::_scriptVariables[SPECIAL_ITEM] && Logic::_scriptVariables[SPECIAL_ITEM] != 0xFFFFFFFF) { //over anything?
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
	Compact *itemData;
	bool touched = false;

	if (_mouseB)
		_logicClick++;
	else
		_logicClick = 0;

	// get list of compacts in inventory
	objList = _skyLogic->giveInvList();

	// how many
	num = (int)Logic::_scriptVariables[MENU_LENGTH];

	if (_holding) {

		// dragged off of inv? Quit inv mode
		if (xPos < _invX || xPos > _invX + _invW || yPos < _invY || yPos > _invY + _invH) {

			// dragged off of the inv
			M_MODE = INVENTORY_USE_ON;
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
		if (_mouseB) {
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
			} else {
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
	if (_mouseB) {
		for (j = 0; j < num; j++) {
			// fetch the compact
			itemData = _skyCompact->fetchCpt(objList[j]);

			if (itemData->xcood + ((int16)itemData->mouseRelX) > xPos) continue;
			if (itemData->xcood + ((uint16)itemData->mouseRelX) + XWIDTH < xPos) continue;
			if (itemData->ycood + ((uint16)itemData->mouseRelX) > yPos) continue;
			if (itemData->ycood + ((uint16)itemData->mouseRelX) + YDEPTH < yPos) continue;

			// record what we're touching
			Logic::_scriptVariables[SPECIAL_ITEM] = objList[j];

			if (_touchId != objList[j] && !_holding) {
				debug("New touch\n");
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
					debug("QUIT LINC\n");
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
					_mouseYOff = HOTSPOT_useon_yoff;
					_holding = true;
					_touchIdLegacy = _touchId;
					_skyLogic->startInventory(_touchId); // highlight touched item
				}
			}
			// touching something - we're done
			return;
		}
	}

}

} // End of namespace Sky
