/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef BSCONTROL_H
#define BSCONTROL_H

#include "scummsys.h"
#include "sworddefs.h"

class ObjectMan;
class ResMan;
class OSystem;
class SwordMouse;
class SwordMusic;

#define MAX_BUTTONS 16

#define CONTROL_NOTHING_DONE 0
#define CONTROL_GAME_RESTORED 1
#define CONTROL_RESTART_GAME 2

class ControlButton {
public:
	ControlButton(uint16 x, uint16 y, uint32 resId, uint8 id, ResMan *pResMan, uint8 *screenBuf, OSystem *system);
	~ControlButton(void);
	void draw(void);
	bool wasClicked(uint16 mouseX, uint16 mouseY);
	void setSelected(uint8 selected);
	bool isSaveslot(void);
	uint8 _id;
private:
	int _frameIdx;
	uint16 _x, _y;
	uint16 _width, _height;
	uint32 _resId;
	ResMan *_resMan;
	uint8 *_dstBuf;
	OSystem *_system;
};

struct ButtonInfo {
	uint16 x, y;
	uint32 resId, id;
};

class SwordControl {
public:
	SwordControl(ResMan *pResMan, ObjectMan *pObjMan, OSystem *system, SwordMouse *pMouse, SwordMusic *pMusic, const char *savePath);
	~SwordControl(void);
	uint8 runPanel(void);
	void doRestore(void);
	void askForCd(void);
	bool savegamesExist(void);
private:
	void initData(void);
	void closeData(void);

	void saveGameToFile(uint8 slot);
	bool restoreGameFromFile(uint8 slot);
	void readSavegameDescriptions(void);
	void writeSavegameDescriptions(void);
	void showSavegameNames(void);
	void deselectSaveslots(void);
	uint8 *_restoreBuf;
	uint8 _saveFiles;
	uint8 _saveScrollPos;
	uint8 _selectedSavegame;
	uint8 _saveNames[64][32];
	uint8 _oldName[32];

	uint8 getClicks(uint8 mode, uint8 *retVal);
	uint8 handleButtonClick(uint8 id, uint8 mode, uint8 *retVal);

	void setupMainPanel(void);
	void setupSaveRestorePanel(bool saving);
	void setupVolumePanel(void);

	void saveNameScroll(uint8 scroll, bool saving);
	void saveNameSelect(uint8 id, bool saving);
	bool saveToFile(void);
	bool restoreFromFile(void);
	bool keyAccepted(uint8 key);
	void handleSaveKey(uint8 key);

	void renderVolumeBar(uint8 id);
	uint16 getTextWidth(const uint8 *str);
	void renderText(const uint8 *str, uint16 x, uint16 y, uint8 mode);
	uint8 _numButtons;
	uint8 _selectedButton;
	void createButtons(const ButtonInfo *buttons, uint8 num);
	void destroyButtons(void);
	ControlButton *_buttons[MAX_BUTTONS];
	static const ButtonInfo _deathButtons[3], _panelButtons[7], _saveButtons[16], _volumeButtons[1];
	static const uint8 _languageStrings[8 * 20][43];
	const uint8 (*_lStrings)[43];
	ObjectMan *_objMan;
	ResMan *_resMan;
	OSystem *_system;
	SwordMouse *_mouse;
	SwordMusic *_music;
	char _savePath[256];
	uint8 *_font, *_redFont;
	uint8 *_screenBuf;
	uint8 _keyPressed;
	void delay(uint32 msecs);
	uint16 _mouseX, _mouseY, _mouseState;
};


#endif //BSCONTROL_H

