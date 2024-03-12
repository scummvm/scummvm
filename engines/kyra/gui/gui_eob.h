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

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#ifndef KYRA_GUI_EOB_H
#define KYRA_GUI_EOB_H

#include "kyra/gui/gui.h"

#ifdef ENABLE_EOB

namespace Kyra {

struct EoBRect16 {
	int16 x1;
	int16 y1;
	uint16 x2;
	uint16 y2;
};

class DarkMoonEngine;
class Screen_EoB;

class GUI_EoB : public GUI {
	friend class EoBCoreEngine;
	friend class CharacterGenerator;
public:
	GUI_EoB(EoBCoreEngine *vm);
	~GUI_EoB() override;

	void initStaticData() {}

	// button specific
	void processButton(Button *button) override;
	int processButtonList(Button *buttonList, uint16 inputFlags, int8 mouseWheel) override;

	// Non button based menu handling (main menu, character generation)
	void simpleMenu_setup(int sd, int maxItem, const char *const *strings, int32 menuItemsMask, int itemOffset, int lineSpacing, int textColor, int highlightColor, int shadowColor);
	int simpleMenu_process(int sd, const char *const *strings, void *b, int32 menuItemsMask, int itemOffset);
	void simpleMenu_unselect(int sd, const char *const *strings, void *b, int32 menuItemsMask, int itemOffset);

	// Button based menus (camp menu, load menu)
	virtual void runCampMenu();
	virtual bool runLoadMenu(int x, int y, bool fromMainMenu = false);

	bool confirmDialogue2(int dim, int id, int deflt);
	void messageDialog(int dim, int id, int buttonTextCol);
	void messageDialog2(int dim, int id, int buttonTextCol);

	void updateBoxFrameHighLight(int box);

	int getTextInput(char *dest, int x, int y, int destMaxLen, int textColor1, int textColor2, int cursorColor);

	// Transfer party
	void transferWaitBox();
	Common::String transferTargetMenu(Common::Array<Common::String> &targets);
	bool transferFileMenu(Common::String &targetName, Common::String &selection);

	// utilities for thumbnail creation
	void createScreenThumbnail(Graphics::Surface &dst) override;

protected:
	const char *getMenuString(int id);
	Button *initMenu(int id);
	void releaseButtons(Button *list);
	virtual int mapPointToEntry(const Common::Point &p) const;

	int8 *_numAssignedSpellsOfType;
	char** _saveSlotStringsTemp;
	int16 _saveSlotX;
	int16 _saveSlotY;
	int _menuCur;

	int _clickableCharactersPage;
	char _csjis[3];

	Screen_EoB *_screen;

private:
	int simpleMenu_getMenuItem(int index, int32 menuItemsMask, int itemOffset);
	void simpleMenu_flashSelection(const char *str, int x, int y, int color1, int color2, int color3);
	void simpleMenu_initMenuItemsMask(int menuId, int maxItem, int32 menuItemsMask, int unk);
	void simpleMenu_printButton(int sd, int num, const char *title, bool isHighlight, bool isInitial);
	Common::Point simpleMenu_getTextPoint(int num, int *col = nullptr);
	int simpleMenu_getMouseItem(int sd);

	bool runSaveMenu(int x, int y);
	int selectSaveSlotDialog(int x, int y, int id);
	virtual void drawSaveSlotDialog(int x, int y, int id);
	void runMemorizePrayMenu(int charIndex, int spellType);
	void scribeScrollDialogue();
	bool restParty();

	virtual void drawCampMenu() {}
	virtual void initMemorizePrayMenu(int) {}
	virtual void initScribeScrollMenu() {}
	virtual void printScribeScrollSpellString(const int16 *menuItems, int id, bool highlight);
	virtual bool confirmDialogue(int id);
	int selectCharacterDialogue(int id);
	virtual void displayTextBox(int id, int textColor = 0xFF, bool wait = true);

	virtual void drawMenuButton(Button *b, bool clicked, bool highlight, bool noFill);
	void drawMenuButtonBox(int x, int y, int w, int h, bool clicked, bool noFill);
	void drawTextBox(int dim, int id);
	virtual void drawSaveSlotButton(int slot, int redrawBox, bool highlight);
	virtual void memorizePrayMenuPrintString(int spellId, int bookPageIndex, int spellType, bool noFill, bool highLight);
	virtual void updateOptionsStrings();

	Button *linkButton(Button *list, Button *newbt);

	void setupSaveMenuSlots();
	virtual int getHighlightSlot();
	void sortSaveSlots() override;

	virtual void restParty_updateRestTime(int hours, bool init);

	char **_menuStringsPrefsTemp;
	int16 *_saveSlotIdTemp;
	int _savegameOffset;
	const int _numSlotsVisible;

	EoBCoreEngine *_vm;

	Button *_specialProcessButton;
	Button *_backupButtonList;
	uint16 _flagsMouseLeft;
	uint16 _flagsMouseRight;
	uint16 _flagsModifier;
	uint16 _progress;
	uint16 _prcButtonUnk3;
	uint16 _cflag;

	int _menuLineSpacing;
	int _menuLastInFlags;
	int _menuTextColor;
	int _menuHighlightColor;
	int _menuShadowColor;
	int _menuLines[2];
	int _menuColumnWidth[2];
	int _menuColumnOffset[2];
	bool _menuOverflow[20];
	int _menuColumns;
	Common::Point _menuPoint;

	uint8 _numPages;
	uint8 _numVisPages;
	uint32 _clericSpellAvltyFlags;
	uint32 _paladinSpellAvltyFlags;
	bool _needRest;

	int _menuNumItems;
	bool _charSelectRedraw;

	int _updateBoxIndex;
	int _updateBoxColorIndex;
	const uint8 *_highLightColorTable;
	uint32 _highLightBoxTimer;

	const bool _textInputForceUppercase;
	const int _textInputHeight;
	const int _textInputShadowOffset;

	const Screen::FontId _menuFont;
	const Screen::FontId _menuFont2;
	const int _dlgButtonHeight1;
	const int _dlgButtonHeight2;
	const int _dlgButtonLabelYOffs;

	const EoBRect16 *_highlightFrames;
	static const EoBRect16 _highlightFramesDefault[20];
	static const EoBRect16 _highlightFramesTransferZH[6];
	static const uint8 _highlightColorTableVGA[];
	static const uint8 _highlightColorTableEGA[];
	static const uint8 _highlightColorTableAmiga[];
	static const uint8 _highlightColorTablePC98[];
	static const uint8 _highlightColorTableSegaCD[];

	// FM-Towns / SegaCD specific
	virtual uint16 checkClickableCharactersSelection();
	virtual void printClickableCharacters(int page);
};

} // End of namespace Kyra

#endif // ENABLE_EOB

#endif

#endif // ENABLE_EOB || ENABLE_LOL
