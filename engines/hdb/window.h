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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HDB_WINDOW_H
#define HDB_WINDOW_H

namespace HDB {

enum {
	kMaxMsgQueue = 10,
	kPanicZoneFaceY = 5,
	kNumCrazy = 37
};

enum PZValue {
	PANICZONE_TIMER,
	PANICZONE_START,
	PANICZONE_TITLESTOP,
	PANICZONE_BLASTOFF,
	PANICZONE_COUNTDOWN,

	PANICZONE_END
};

struct DialogInfo {
	char		title[64];				// TITLE string
	int			tileIndex;					// this is for a character picture
	char		string[160];			// actual text in the dialog

	bool		active;					// is it drawing or not?
	int			x, y;					// where to draw dialog
	int			width, height;			// size of the dialog itself
	int			titleWidth;
	Picture		*gfx;					// character tile (picture)
	int			more;					// whether we want to draw the MORE icon or not
	int			el, er, et, eb;			// saves the text edges
	char		luaMore[64];			// the name of the function to call after clicking the MORE button

	DialogInfo() : tileIndex(0), active(false), x(0), y(0),
		width(0), height(0), titleWidth(0), gfx(NULL), more(0), el(0), er(0), et(0),
		eb(0) {
			title[0] = 0;
			string[0] = 0;
			luaMore[0] = 0;
		}
};

struct DialogChoiceInfo {
	char		title[64];				// TITLE string
	char		text[160];				// actual text in the dialog
	char		func[64];				// function to call with result

	bool		active;					// is it drawing or not?
	int			x, y;					// where to draw dialog
	int			width, height;			// size of the dialog itself
	int			textHeight;				// height of everything above choices
	int			titleWidth;
	int			el, er, et, eb;			// saves the text edges
	uint32		timeout;				// timeout value!

	int			selection;				// which choice we've made
	int			numChoices;			// how many choices possible
	char		choices[10][64];		// ptrs to choice text

	DialogChoiceInfo() : active(false), x(0), y(0),
		width(0), height(0), textHeight(0), titleWidth(0), el(0), er(0), et(0),
		eb(0), timeout(0), selection(0), numChoices(0) {
		title[0] = 0;
		text[0] = 0;
		func[0] = 0;
		for (int i = 0; i < 10; i++)
			choices[i][0] = 0;
	}
};

struct MessageInfo {
	bool		active;
	char		title[128];
	int			timer;
	int			x, y;
	int			width, height;

	MessageInfo() : active(false), timer(0), x(0), y(0), width(0), height(0) {
		title[0] = 0;
	}
};

struct InvWinInfo {
	int x, y;
	int width, height;
	int selection;
	bool active;

	InvWinInfo() : x(0), y(0), width(0), height(0), selection(0), active(false) {}
};

struct DlvsInfo {
	int x, y;
	int width, height;
	bool active;
	int selected;
	bool animate;
	uint32 delay1, delay2, delay3;
	bool go1, go2, go3;

	DlvsInfo() : x(0), y(0), width(0), height(0), active(false), selected(0),
		animate(false), delay1(0), delay2(0), delay3(0), go1(false), go2(false), go3(false) {}
};

struct PanicZone {
	bool active;
	int sequence;
	int timer;
	int x1, y1;
	int x2, y2;
	int xv, yv;		// for both
	int numberTime;
	int numberTimeMaster;
	int numberValue;
	Picture *gfxPanic, *gfxZone;
	Picture *gfxFace[2];
	Picture *gfxNumber[10];

	PanicZone() : active(false), sequence(0), timer(0), x1(0), y1(0), x2(0), y2(0), xv(0), yv(0),
		numberTime(0), numberTimeMaster(0), numberValue(0), gfxPanic(NULL), gfxZone(NULL) {
		memset(&gfxFace, 0, sizeof(gfxFace));
		memset(&gfxNumber, 0, sizeof(gfxNumber));
	}
};

struct TryAgainInfo {
	double y1, y2;
	double yv1, yv2;
	double yv1v, yv2v;
	double x1, x2;

	TryAgainInfo() : y1(0), y2(0), yv1(0), yv2(0), yv1v(0), yv2v(0), x1(0), x2(0) {}
};

struct TOut {
	char text[128];
	int x, y;
	uint32 timer;

	TOut() : x(0), y(0), timer(0) {
		text[0] = 0;
	}
};

class Window {
public:
	Window();
	~Window();

	void init();
	void save(Common::OutSaveFile *out);
	void loadSaveFile(Common::InSaveFile *in);
	void restartSystem();
	void setInfobarDark(int value);

	void closeAll();

	// Pause Functions
	void drawPause();
	void checkPause(int x, int y);

	// Weapon Functions
	void drawWeapon();
	void chooseWeapon(AIType wType);

	// Dialog Functions

	void openDialog(const char *title, int tileIndex, const char *string, int more, const char *luaMore);
	void drawDialog();
	void closeDialog();
	bool checkDialogClose(int x, int y);
	void drawBorder(int x, int y, int width, int height, bool guyTalking);
	void setDialogDelay(int delay);
	uint32 getDialogDelay() {
		return _dialogDelay;
	}
	bool dialogActive() {
		return _dialogInfo.active;
	}

	// Dialog Choice Functions

	void openDialogChoice(const char *title, const char *text, const char *func, int numChoices, const char *choices[10]);
	void drawDialogChoice();
	void closeDialogChoice();
	bool checkDialogChoiceClose(int x, int y);
	void dialogChoiceMoveup();
	void dialogChoiceMovedown();
	bool dialogChoiceActive() {
		return _dialogChoiceInfo.active;
	}

	// MessageBar Functions
	void openMessageBar(const char *title, int time);
	void drawMessageBar();
	bool checkMsgClose(int x, int y);
	void nextMsgQueued();
	void closeMsg();
	bool msgBarActive() {
		return _msgInfo.active;
	}

	// Inventory Functions
	void drawInventory();
	void setInvSelect(int status) {
		_invWinInfo.selection = status;
	}
	int getInvSelect() {
		return _invWinInfo.selection;
	}
	void checkInvSelect(int x, int y);
	bool inventoryActive() {
		return _invWinInfo.active;
	}
	// PPC Inventory
	void openInventory();
	bool checkInvClose(int x, int y);
	void closeInv();

	// Deliveries Functions
	void openDeliveries(bool animate);
	void drawDeliveries();
	void setSelectedDelivery(int which);
	int getSelectedDelivery() {
		return _dlvsInfo.selected;
	}
	bool animatingDelivery() {
		return _dlvsInfo.animate;
	}
	void checkDlvSelect(int x, int y);
	// PPC Deliveries
	bool checkDlvsClose(int x, int y);
	void closeDlvs();
	bool deliveriesActive() {
		return _dlvsInfo.active;
	}

	// Try Again Functions
	void drawTryAgain();
	void clearTryAgain();

	// Panic Zone Functions
	void loadPanicZoneGfx();
	void drawPanicZone();
	void startPanicZone();
	void stopPanicZone();
	bool inPanicZone() {
		return _pzInfo.active;
	}

	// TextOut functions
	void textOut(const char *text, int x, int y, int timer);
	void centerTextOut(const char *text, int y, int timer);
	void drawTextOut();
	int textOutActive() {
		return (_textOutList.size());
	}
	void closeTextOut();

	// Platform-specific Constants
	int _weaponX, _weaponY;
	int _invItemSpace, _invItemSpaceX, _invItemSpaceY;
	int _invItemPerLine;
	int _dlvItemSpaceX;
	int _dlvItemSpaceY;
	int _dlvItemPerLine;
	int _dlvItemTextY;
	int _dialogTextLeft;
	int _dialogTextRight;
	int _openDialogTextLeft;
	int _openDialogTextRight;
	int _textOutCenterX;
	int _pauseY;
	int _tryY1;	// TRY
	int _tryY2;			// AGAIN
	int _tryRestartY;	// (ok)
	int _panicXStop;
	int _panicZoneFaceX;

	Tile *getGemGfx() {
		return _gemGfx;
	}

	PanicZone _pzInfo, _tempPzInfo;

private:

	DialogInfo _dialogInfo;
	uint32 _dialogDelay;	// Used for Cinematics

	DialogChoiceInfo _dialogChoiceInfo;

	MessageInfo _msgInfo;

	InvWinInfo _invWinInfo;
	Common::Array<TOut *> _textOutList;
	DlvsInfo _dlvsInfo;

	TryAgainInfo _tryAgainInfo;

	char _msgQueueStr[kMaxMsgQueue][128];
	int _msgQueueWait[kMaxMsgQueue];
	int _numMsgQueue;

	// Windows GFX
	Picture *_gfxTL, *_gfxTM, *_gfxTR;
	Picture *_gfxL, *_gfxM, *_gfxR;
	Picture *_gfxBL, *_gfxBM, *_gfxBR;
	Picture *_gfxTitleL, *_gfxTitleM, *_gfxTitleR;
	Picture *_gGfxTL, *_gGfxTM, *_gGfxTR;
	Picture *_gGfxL, *_gGfxM, *_gGfxR;
	Picture *_gGfxBL, *_gGfxBM, *_gGfxBR;
	Picture *_gGfxTitleL, *_gGfxTitleM, *_gGfxTitleR;
	Picture *_gfxIndent, *_gfxArrowTo, *_gfxHandright;
	Picture *_gfxTry, *_gfxAgain, *_gfxInvSelect;
	Picture *_gfxLevelRestart, *_gfxPausePlaque;
	Tile *_gemGfx;
	Picture *_mstoneGfx;

	// Info Bar
	Picture *_gfxResources, *_gfxDeliveries;
	Picture *_gfxInfobar, *_gfxDarken;
	int _infobarDimmed;
};

} // End of Namespace

#endif // !HDB_WINDOW_H
