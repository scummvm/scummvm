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

#ifndef SWORD1_CONTROL_H
#define SWORD1_CONTROL_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/str-array.h"
#include "sword1/sworddefs.h"

class OSystem;
namespace Common {
class SaveFileManager;
class MemoryWriteStreamDynamic;
}

namespace Sword1 {

enum SNRStatus {
	SNR_BLANK = 0,
	SNR_MAINPANEL,
	SNR_SAVE,
	SNR_RESTORE,
	SNR_RESTART,
	SNR_QUIT,
	SNR_SPEED,
	SNR_VOLUME,
	SNR_SUBTITLES,
	SNR_DONE,
	SNR_DRIVEFULL = 99
};

enum SaveGameFlags {
	SGF_DONE = 0,
	SGF_SAVE,
	SGF_RESTORE,
	SGF_RESTART,
	SGF_QUIT
};

enum PsxComponents {
	PSX_PANEL = 0,
	PSX_DEATHPANEL,
	PSX_CONFIRM,
	PSX_BUTTON,
	PSX_TEXT,
	PSX_SLAB,
	PSX_SCROLL
};

class SwordEngine;
class ObjectMan;
class ResMan;
class Mouse;
class Sound;
class Screen;
class Logic;

#define SAVEGAME_HEADER MKTAG('B','S','_','1')
#define SAVEGAME_VERSION 2

#define FIRSTFONTCHAR ' '
#define LASTFONTCHAR  (32 + 137)
#define CR             13
#define LF             10
#define ESCAPE         27
#define BACKSPACE      8

#define CONTROL_NOTHING_DONE 0
#define CONTROL_GAME_RESTORED 1
#define CONTROL_RESTART_GAME 2

#define VD1X 139
#define VD2X 273
#define VD3X 404
#define VDY  94

#define SCROLL1X 311
#define SCROLL1Y 124
#define SCROLL2X 311
#define SCROLL2Y 188

#define SAVEBUTTONS  14
#define MAXSAVEGAMES 1000
#define OVERLAP      3
#define SP_OVERLAP   2
#define TEXTBUTTONID 7

#define PSX_CREDITS_SPACING (-3)
#define PSX_CREDITS_MIDDLE  450
#define PSX_CREDITS_OFFSET  150
#define PSX_NUM_CREDITS     14

struct Button {
	int32 x1;
	int32 y1;
	int32 x2;
	int32 y2;
};

class Control {
public:
	Control(SwordEngine *vm, Common::SaveFileManager *saveFileMan, ResMan *pResMan, ObjectMan *pObjMan, OSystem *system, Mouse *pMouse, Sound *pSound, Screen *pScreen, Logic *pLogic);

	void getPlayerOptions();
	void askForCdMessage(uint32 needCD, bool incorrectCDPhase);
	void doRestore();
	bool savegamesExist();
	void saveGameToFile(uint8 slot);
	bool restoreGameFromFile(uint8 slot);
	bool restoreGame();
	void checkForOldSaveGames();
	bool isPanelShown();
	const uint8 *getPauseString();
	void psxEndCredits();

	void setSaveDescription(int slot, const char *desc) {
		Common::strcpy_s((char *)_fileDescriptions[slot], sizeof(_fileDescriptions[slot]), desc);
	}

private:
	void saveRestoreScreen();

	void renderSlab(int32 start, int32 i);
	void renderSlabs();
	void renderText(const uint8 *str, int32 x, int32 y, bool useSpeechFont = false);
	void renderRedText(const uint8 *str, int32 x, int32 y);
	void renderTexts();
	int32 getTextLength(const uint8 *str, bool useSpeechFont = false);

	void putButton(int32 x, int32 y, int32 index);
	void putSpriteButton(Sprite *spr, int32 x, int32 y, int32 index);
	void putTextButton(int32 index);
	int32 getCurrentButton(const Button b[]);

	void initialiseConfirmation(const uint8 *title);
	int32 implementConfirmation();
	void removeConfirmation();

	void volUp(int32 i, int32 j);
	void volDown(int32 i, int32 j);
	void renderVolumeLight(int32 i);
	void renderVolumeDisc(int32 i, int32 j);
	void initialiseVolume();
	void implementVolume();
	void removeVolume();

	void renderScrolls();
	void initialiseSpeed();
	void implementSpeed();
	void removeSpeed();

	int16 readFileDescriptions();
	void setEditDescription(int32 line);
	bool driveSpaceAvailable();
	bool attemptSave();
	bool saveGame();
	void editDescription();
	void restoreSelected();
	void uneditDescription();
	void initialiseSave();
	void implementSave();
	void removeSave();

	void initialiseRestore();
	void implementRestore();
	void removeRestore();

	void initialiseControlPanel();
	void implementControlPanel();
	void removeControlPanel();

	void initialiseResources();
	void releaseResources();

	uint8 *decompressPsxGfx(uint8 *src, FrameHeader *f);
	void drawPsxComponent(int componentType, uint8 *src, uint8 *dst, FrameHeader *f);

	bool convertSaveGame(uint8 slot, char *desc);

	void delay(uint32 msecs);
	bool gameVersionIsAkella();
	bool gameVersionIsMediaHouse();
	bool loadCustomStrings(const char *filename);

	int displayMessage(const char *altButton, MSVC_PRINTF const char *message, ...) GCC_PRINTF(3, 4);

	// PSX Credits functions
	int32 getCreditsFontHeight(uint8 *font);
	int32 getCreditsStringLength(uint8 *str, uint8 *font);
	void renderCreditsTextSprite(uint8 *data, uint8 *dst, int16 x, int16 y, int16 width, int16 height);
	void createCreditsTextSprite(uint8 *data, int32 pitch, uint8 *str, uint8 *font);

	Common::MemoryWriteStreamDynamic *_tempThumbnail;
	static const uint8 _languageStrings[8 * 20][43];
	static const uint8 _akellaLanguageStrings[20][43];
	static const uint8 _mediaHouseLanguageStrings[20][43];
	static const uint8 _polishTranslationLanguageStrings[20][43];
	uint8 _customStrings[20][43];
	const uint8(*_lStrings)[43];
	const uint8 _psxPauseStrings[3][7] = { "Paused", "Pause", "Pausa" };
	SwordEngine *_vm;
	Common::SaveFileManager *_saveFileMan;
	ObjectMan *_objMan;
	ResMan *_resMan;
	OSystem *_system;
	Mouse *_mouse;
	Sound *_sound;
	Screen *_screen;
	Logic *_logic;
	uint8 *_screenBuf;
	Common::KeyState _keyPressed;
	Common::CustomEventType _customType;

	Common::Point _mouseCoord;
	uint16 _mouseState;

	int _oldSnrStatus = SNR_BLANK;
	bool _newPal = false;
	Sprite *_slabs[SAVEBUTTONS - 6];
	int32 _scrollIndex[2] = { 0, 0 };
	int32 _speedFlag = 0;

	int32 _currentButton = 0;
	int32 _buttonPressed = 0;
	int32 _buttonHold = 0;
	int32 _slabSelected = 0;
	int32 _firstDescription = 0;
	byte  _fileDescriptions[MAXSAVEGAMES][40];
	int32 _editingDescription = 0;
	int32 _gamesSaved = 0;
	int32 _textCursor;
	int32 _curCharCount;
	char  _oldString[40];
	int32 _scroll = 0;
	int32 _scrollCount = 0;

	uint8 *_restoreBuf = nullptr;
	uint32 _selectedSavegame = 0;
	uint8 _numButtons = 0;
	uint8 _selectedButton = 0;
	bool _panelShown = false;

	static const Button panelButtons[8];
	// We want the death screen buttons to have
	// the same numbers as the panel buttons
	static const Button deathButtons[8];
	static const Button confirmButtons[2];
	static const Button speedButtons[3];
	static const Button saveButtons[SAVEBUTTONS];
	static const Button restoreButtons[SAVEBUTTONS];
	static const Button volumeButtons[25];
};

} // End of namespace Sword1

#endif //BSCONTROL_H
