/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef MOUSE_H
#define MOUSE_H

#define	TOTAL_mouse_list 50

namespace Sword2 {

struct ObjectMouse;
struct BuildUnit;

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct MouseAnim {
	uint8 runTimeComp;	// type of runtime compression used for the
				// frame data
	uint8 noAnimFrames;	// number of frames in the anim
	int8 xHotSpot;		
	int8 yHotSpot;
	uint8 mousew;
	uint8 mouseh;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

// The MOUSE_holding mode is entered when the conversation menu is closed, and
// exited when the mouse cursor moves off that menu area. I don't know why yet.
 
// mouse unit - like ObjectMouse, but with anim resource & pc (needed if
// sprite is to act as mouse detection mask)

struct MouseUnit {
	// Top-left and bottom-right of mouse area. These coords are inclusive
	int32 x1;
	int32 y1;
	int32 x2;
	int32 y2;

	int32 priority;

	// type (or resource id?) of pointer used over this area
	int32 pointer;

	// up to here, this is basically a copy of the ObjectMouse
	// structure, but then we have...

	// object id, used when checking mouse list
	int32 id;

	// resource id of animation file (if sprite to be used as mask) -
	// otherwise 0
	int32 anim_resource;

	// current frame number of animation
	int32 anim_pc;

	// local id of text line to print when pointer highlights an object
	int32 pointer_text;
};

class Mouse {
private:
	Sword2Engine *_vm;

	Common::Point _pos;

	MouseUnit _mouseList[TOTAL_mouse_list];
	uint32 _curMouse;

	MenuObject _tempList[TOTAL_engine_pockets];
	uint32 _totalTemp;

	MenuObject _masterMenuList[TOTAL_engine_pockets];
	uint32 _totalMasters;

	uint8 _menuStatus[2];
	byte *_icons[2][RDMENU_MAXPOCKETS];
	uint8 _pocketStatus[2][RDMENU_MAXPOCKETS];

	uint8 _iconCount;

	// If it's NORMAL_MOUSE_ID (ie. normal pointer) then it's over a floor
	// area (or hidden hot-zone)

	uint32 _mousePointerRes;

	struct MouseAnim *_mouseAnim;
	struct MouseAnim *_luggageAnim;

	uint8 _mouseFrame;
	byte *_mouseSprite;
	int32 *_mouseOffsets;
	int32 *_luggageOffset;

	uint32 _mouseMode;

	bool _mouseStatus;		// Human 0 on/1 off
	bool _mouseModeLocked;		// 0 not !0 mode cannot be changed from
					// normal mouse to top menu (i.e. when
					// carrying big objects)
	uint32 _realLuggageItem;	// Last minute for pause mode
	uint32 _currentLuggageResource;
	uint32 _oldButton;		// For the re-click stuff - must be
					// the same button you see
	uint32 _buttonClick;
	uint32 _pointerTextBlocNo;
	uint32 _playerActivityDelay;	// Player activity delay counter

	bool _examiningMenuIcon;

	// Set by checkMouseList()
	uint32 _mouseTouching;
	uint32 _oldMouseTouching;

	bool _objectLabels;

	uint32 _menuSelectedPos;

	void decompressMouse(byte *decomp, byte *comp, int width, int height, int pitch, int xOff = 0, int yOff = 0);

	int32 setMouseAnim(byte *ma, int32 size, int32 mouseFlash);
	int32 setLuggageAnim(byte *la, int32 size);

	void clearIconArea(int menu, int pocket, Common::Rect *r);

public:
	Mouse(Sword2Engine *vm);
	~Mouse();

	void getPos(int &x, int &y);
	void setPos(int x, int y);

	bool getObjectLabels() { return _objectLabels; }
	void setObjectLabels(bool b) { _objectLabels = b; }

	bool getMouseStatus() { return _mouseStatus; }
	uint32 getMouseTouching() { return _mouseTouching; }
	void setMouseTouching(uint32 touching) { _mouseTouching = touching; }

	void pauseGame();
	void unpauseGame();

	void setMouse(uint32 res);
	void setLuggage(uint32 res);

	void setObjectHeld(uint32 res);

	void resetMouseList();

	void registerMouse(ObjectMouse *ob_mouse, BuildUnit *build_unit);
	void registerPointerText(int32 text_id);

	void createPointerText(uint32 text_id, uint32 pointer_res);
	void clearPointerText();

	void drawMouse();
	int32 animateMouse();

	void processMenu();

	void addMenuObject(MenuObject *obj);
	void buildMenu();
	void buildSystemMenu();

	int32 showMenu(uint8 menu);
	int32 hideMenu(uint8 menu);
	int32 setMenuIcon(uint8 menu, uint8 pocket, byte *icon);

	void closeMenuImmediately();

	void refreshInventory();

	void startConversation();
	void endConversation();

	void hideMouse();
	void noHuman();
	void addHuman();

	void resetPlayerActivityDelay() { _playerActivityDelay = 0; }
	void monitorPlayerActivity();
	void checkPlayerActivity(uint32 seconds);

	void mouseOnOff();
	uint32 checkMouseList();
	void mouseEngine();

	void normalMouse();
	void menuMouse();
	void dragMouse();
	void systemMenuMouse();

	int menuClick(int menu_items);
};

} // End of namespace Sword2

#endif
