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

#ifndef SKY_MOUSE_H
#define SKY_MOUSE_H


#include "common/scummsys.h"
#include "common/hashmap.h"

class OSystem;

namespace Sky {

// max click cycles before interact is not valid upon release
#define CLICK_THRESHOLD 10
// cycles before inv use-on trigger
#define USEON_THRESHOLD 5
// time till hotspot fades out after stopped touching
#define HOTSPOT_FADEOUT 36
// hotspot icon size
#define HOTSPOT_DIM 36
// exit is just one icon, so half width as straddles centre point
#define HOTSPOT_EXIT_DIM (HOTSPOT_DIM >> 1)
// how far the popup icons sit above the midy
#define HOTSPOT_YOFF 48
// differentiate exits, just incase
#define HOTSPOT_EXIT_YOFF 48

// use on cursor offset
#define HOTSPOT_USE_ON_YOFF (-55)

// inv button (bottom left)
#define HOTSPOT_INVX (TOP_LEFT_X + 40)
#define HOTSPOT_INVY (TOP_LEFT_Y + FULL_SCREEN_HEIGHT-35)

// control panel button (top left)
#define HOTSPOT_OPTIONSX (TOP_LEFT_X + 30)
#define HOTSPOT_OPTIONSY (TOP_LEFT_Y + 30)

// help button (top right)
#define HOTSPOT_helpx (TOP_LEFT_X + FULL_SCREEN_WIDTH-30)
#define HOTSPOT_helpy (TOP_LEFT_Y + 30)

// glow proximity
#define GLOW_DIST 150
// drag to use on distance
#define USE_ON_DIST 40

// fix super deep exits
#define TWEEKY_EXIT_ADJUST (FULL_SCREEN_HEIGHT + TOP_LEFT_Y - 40)


// inv item sizes
#define XWIDTH 32
#define YDEPTH 32

// action flash
#define ACTION_FLASH_TIME 7

class Disk;
class Logic;
class SkyCompact;
class Text;
class Screen;
class Control;
struct Compact;
enum UIIcon : uint8;

class Mouse {

public:

	Mouse(OSystem *system, Disk *skyDisk, SkyCompact *skyCompact, Screen *skyScreen);
	~Mouse();

	void mouseEngine();
	void mouseEngineIBASS();
	void replaceMouseCursors(uint16 fileNo);
	bool fnAddHuman();
	void fnSaveCoods();
	void fnOpenCloseHand(bool open);
	uint16 findMouseCursor(uint32 itemNum);
	void lockMouse();
	void unlockMouse();
	void restoreMouseData(uint16 frameNum);
	void drawNewMouse();
	void spriteMouse(uint16 frameNum, uint8 mouseX, uint8 mouseY);
	void useLogicInstance(Logic *skyLogic) { _skyLogic = skyLogic; }
	void useControlInstance(Control *control) { _skyControl = control; }
	void buttonPressed(uint8 button);
	void mouseMoved(uint16 mouseX, uint16 mouseY);
	void waitMouseNotPressed(int minDelay = 0);
	uint16 giveMouseX() { return _mouseX; }
	uint16 giveMouseY() { return _mouseY; }
	uint16 giveCurrentMouseType() { return _currentCursor; }
	bool wasClicked();
	void logicClick() { _logicClick = true; }
	void resetCursor();
	void setInvDims(uint16 x, uint16 y, uint16 w, uint16 h) {
		_invX = x;
		_invY = y;
		_invW = w;
		_invH = h;
	}
	bool isUILive();
	void setTextChooserMode() {
		_mMode = TEXT_CHOOSER;
	}
	void resetUI();
	OSystem *giveSystem(){
		return	_system;
	}
	int	doProximityHighlights(uint16 xPos, uint16 yPos);
	uint16 giveXCood(Compact *itemData, uint32 id);
	uint16 giveYCood(Compact *itemData, uint32 id);
	void setLincInv(bool inv) {
		_isLincInv = inv;
	}
	bool isLincInv() {
		return _isLincInv;
	}
	void incLincMenuRef() {
		_lincMenuRef++	;
	}
	void pushInvY(uint16 y) {
		_invYCoord = y;
	}
	uint16 popInvY() {
		return _invYCoord;
	}
	UIIcon getInteractIcon(uint32 id);
	bool hasSingleInteractIcon(uint32 id);
	void updateHotspotCoordinate(uint16 xPos);
	int	touchingFloor(uint16 xPos, uint16 yPos);

protected:

	void pointerEngine(uint16 xPos, uint16 yPos);
	void pointerEngineIBASS(uint16 xPos, uint16 yPos);
	void buttonEngine1();
	void invMouse(uint16 xPos, uint16 yPos);
	void lincInvMouse(uint16 xPos, uint16 yPos);
	void invUseOn(uint16 xPos, uint16 yPos);
	void textChooser(uint16 xPos, uint16 yPos);

	bool _logicClick;

	uint16 _mouseB;	//mouse button
	uint16 _mouseX;	//actual mouse coordinates
	uint16 _mouseY;

	int16	_mouseXOff;
	int16	_mouseYOff;
	uint16	_timeOn; // how long have we held on this item
	uint16	_clickedNum; // remmebers what we clicked last
	uint16	_fadeOut; // how long before current hotspot fades out when let go
	bool	_prevMouseOn; // touching yes/no last cycle
	uint16	_touchId;
	uint16	_touchIdLegacy;
	uint16	_hoverId; // inv drag over inv
	bool	_holding;
	bool	_isExit; // cur hotspot an exit?
	uint32	_exitType; // what type of exit
	bool	_isFloor;
	bool	_isLincInv;	// current inventory is in LINC which works weirdly differently
	uint32	_lincMenuRef; // used to check if a linc inv menu restarted the inv
	uint16	_invYCoord;	// store inv item y coords ready for patching back in
	int		_nearestProximityIconId;
	bool	_floorLock; // stop moving off hotpots onto floors then releasing to get an interaction

	uint16	_invX; // inventory bounds x
	uint16	_invY; // inventory bounds y
	uint16	_invW; // inventory bounds w
	uint16	_invH; // inventory bounds h

	bool	_actionFlash = false; // clicked on action icon flasher
	int		_actionFlashTime = 0; // counts down
	int		_actionFlashIcon = 0;
	int		_actionFlashX = 0;
	int		_actionFlashY = 0;
	void initExitIcon(uint32 type, int iconx, int icony);


	uint16 _currentCursor;

	byte *_miceData;	//address of mouse sprites
	byte *_objectMouseData;	//address of object mouse sprites
	
	int _proxFrame;
	int _proxFrameSpeed;

	Common::HashMap<int, int> _hotspotXMap;
	Common::HashMap<int, int> _hotspotYMap;

	static uint32 _mouseMainObjects[24];
	static uint32 _mouseLincObjects[21];

	OSystem *_system;
	Disk *_skyDisk;
	Logic *_skyLogic;
	SkyCompact *_skyCompact;
	Screen *_skyScreen;
	Control *_skyControl;

	enum : uint8 {
		GAMEPLAY,
		PRE_INVENTORY,
		INVENTORY,
		INV_TEMP_EXAMINE,
		INVENTORY_USE_ON,
		TEXT_CHOOSER,
		MUST_RELEASE,
		ALERT_TO_GAME, // new game alert box
	} _mMode;
};

} // End of namespace Sky

#endif //SKYMOUSE_H
