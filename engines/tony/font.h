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
 */
/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#ifndef TONY_FONT_H
#define TONY_FONT_H

#include "common/system.h"
#include "common/coroutines.h"
#include "tony/gfxcore.h"
#include "tony/resid.h"

namespace Tony {

class RMInput;
class RMInventory;
class RMItem;
class RMLoc;
class RMLocation;
class RMPointer;

/**
 * Manages a font, in which there is a different surface for each letter
 */
class RMFont : public RMGfxTaskSetPrior {
protected:
	int nLetters;
	RMGfxSourceBuffer8RLEByte *_letter;
public:
	int _fontDimx, _fontDimy;

private:
	int _dimx, _dimy;

	class RMFontPrimitive : public RMGfxPrimitive {
	public:
		RMFontPrimitive() : RMGfxPrimitive() {}
		RMFontPrimitive(RMGfxTask *task) : RMGfxPrimitive(task) {}
		virtual ~RMFontPrimitive() { }
		virtual RMGfxPrimitive *duplicate() {
			return new RMFontPrimitive(*this);
		}

		int _nChar;
	};

protected:
	// Loads the font
	void load(uint32 resID, int nChars, int dimx, int dimy, uint32 palResID = RES_F_PAL);
	void load(const byte *buf, int nChars, int dimx, int dimy, uint32 palResID = RES_F_PAL);

	// Remove the font
	void unload(void);

protected:
	// Conversion form character to font index
	virtual int convertToLetter(byte nChar) = 0;

	// Character width
	virtual int letterLength(int nChar, int nNext = 0) = 0;

public:
	virtual int letterHeight(void) = 0;

public:
	RMFont();
	virtual ~RMFont();

	// Initialisation and closing
	virtual void init(void) = 0;
	virtual void close(void);

	// Drawing
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBug, RMGfxPrimitive *prim);

	// Create a primitive for a letter
	RMGfxPrimitive *makeLetterPrimitive(byte bChar, int &nLength);

	// Length in pixels of a string with the current font
	int stringLen(const RMString &text);
	int stringLen(char bChar, char bNext = 0);
};


class RMFontColor : public virtual RMFont {
private:
	byte m_r, m_g, m_b;

public:
	RMFontColor();
	virtual ~RMFontColor();
	virtual void setBaseColor(byte r, byte g, byte b);
};


class RMFontWithTables : public virtual RMFont {
protected:
	int cTable[256];
	int lTable[256];
	int lDefault;
	int hDefault;
	signed char l2Table[256][256];

protected:
	// Overloaded methods
	int convertToLetter(byte nChar) {
		return cTable[nChar];
	}
	int letterLength(int nChar, int nNext = 0) {
		return (nChar != -1 ? lTable[(byte)nChar] + l2Table[(byte)nChar][(byte)nNext] : lDefault);
	}

public:
	int letterHeight() {
		return hDefault;
	}
	virtual ~RMFontWithTables() {}
};


class RMFontParla : public RMFontColor, public RMFontWithTables {
public:
	void init(void);
	virtual ~RMFontParla() {}
};

class RMFontObj : public RMFontColor, public RMFontWithTables {
private:
	void setBothCase(int nChar, int nNext, signed char spiazz);

public:
	void init(void);
	virtual ~RMFontObj() {}
};

class RMFontMacc : public RMFontColor, public RMFontWithTables {
public:
	void init(void);
	virtual ~RMFontMacc() {}
};

class RMFontCredits : public RMFontColor, public RMFontWithTables {
public:
	void init(void);
	virtual ~RMFontCredits() {}
	virtual void setBaseColor(byte r, byte g, byte b) {}
};

/**
 * Manages writing text onto9 the screen
 */
class RMText : public RMGfxWoodyBuffer {
private:
	static RMFontColor *_fonts[4];
	static RMGfxClearTask _clear;
	int maxLineLength;

public:
	enum HORALIGN {
	    HLEFT,
	    HLEFTPAR,
	    HCENTER,
	    HRIGHT
	};

	enum VERALIGN {
	    VTOP,
	    VCENTER,
	    VBOTTOM
	};

private:
	HORALIGN aHorType;
	VERALIGN aVerType;
	byte m_r, m_g, m_b;

protected:
	virtual void clipOnScreen(RMGfxPrimitive *prim);

public:
	RMText();
	virtual ~RMText();
	static void initStatics();
	static void unload();

	// Set the alignment type
	void setAlignType(HORALIGN aHor, VERALIGN aVer) {
		aHorType = aHor;
		aVerType = aVer;
	}

	// Sets the maximum length of a line in pixels (used to format the text)
	void setMaxLineLength(int max);

	// Write the text
	void writeText(const RMString &text, int font, int *time = NULL);
	void writeText(const RMString &text, RMFontColor *font, int *time = NULL);

	// Overloaded function to decide when you delete the object from the OT list
	virtual void removeThis(CORO_PARAM, bool &result);

	// Overloading of the Draw to center the text, if necessary
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Set the base colour
	void setColor(byte r, byte g, byte b) {
		m_r = r;
		m_g = g;
		m_b = b;
	}
};

/**
 * Manages text in a dialog
 */
class RMTextDialog : public RMText {
protected:
	int _startTime;
	int _time;
	bool _bSkipStatus;
	RMPoint dst;
	uint32 hEndDisplay;
	bool _bShowed;
	bool _bForceTime;
	bool _bForceNoTime;
	uint32 hCustomSkip;
	uint32 hCustomSkip2;
	RMInput *_input;
	bool _bAlwaysDisplay;
	bool _bNoTab;

public:
	RMTextDialog();
	virtual ~RMTextDialog();

	// Write the text
	void writeText(const RMString &text, int font, int *time = NULL);
	void writeText(const RMString &text, RMFontColor *font, int *time = NULL);

	// Overloaded function to decide when you delete the object from the OT list
	virtual void removeThis(CORO_PARAM, bool &result);

	// Overloaded de-registration
	virtual void Unregister(void);

	// Overloading of the Draw to center the text, if necessary
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Set the position
	void setPosition(const RMPoint &pt) {
		dst = pt;
	}

	// Waiting
	void waitForEndDisplay(CORO_PARAM);
	void setCustomSkipHandle(uint32 hCustomSkip);
	void setCustomSkipHandle2(uint32 hCustomSkip);
	void setSkipStatus(bool bEnabled);
	void setForcedTime(uint32 dwTime);
	void setNoTab(void);
	void forceTime(void);
	void forceNoTime(void);
	void setAlwaysDisplay(void);

	// Set the input device, to allow skip from mouse
	void setInput(RMInput *input);

	void show(void);
	void hide(CORO_PARAM);
};

class RMTextDialogScrolling : public RMTextDialog {
protected:
	RMLocation *curLoc;
	RMPoint startScroll;

	virtual void clipOnScreen(RMGfxPrimitive *prim);

public:
	RMTextDialogScrolling();
	RMTextDialogScrolling(RMLocation *loc);
	virtual ~RMTextDialogScrolling();

	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};


/**
 * Manages the name of a selected item on the screen
 */
class RMTextItemName : protected RMText {
protected:
	RMPoint _mpos;
	RMPoint _curscroll;
	RMItem *_item;
	RMString _itemName;

public:
	RMTextItemName();
	virtual ~RMTextItemName();

	void setMouseCoord(const RMPoint &m) {
		_mpos = m;
	}

	void doFrame(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMLocation &loc, RMPointer &ptr, RMInventory &inv);
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	RMPoint getHotspot();
	RMItem *getSelectedItem();
	bool isItemSelected();
	bool isNormalItemSelected();

	virtual void removeThis(CORO_PARAM, bool &result) {
		result = true;
	}
};


/**
 * Manages the selection of screen items in a box
 */
class RMDialogChoice : public RMGfxWoodyBuffer {
private:
	int _curSelection;
	int _numChoices;
	RMText *_drawedStrings;
	RMPoint *_ptDrawStrings;
	int _curAdded;
	bool _bShow;
	RMGfxSourceBuffer8 DlgText;
	RMGfxSourceBuffer8 DlgTextLine;
	RMPoint _ptDrawPos;
	uint32 hUnreg;
	bool bRemoveFromOT;

protected:
	void prepare(CORO_PARAM);
	void setSelected(CORO_PARAM, int pos);

public:
	virtual void removeThis(CORO_PARAM, bool &result);
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	void Unregister(void);

public:
	// Initialisation
	RMDialogChoice();
	virtual ~RMDialogChoice();

	// Initialisation and closure
	void init(void);
	void close(void);

	// Sets the number of possible sentences, which then be added with AddChoice()
	void setNumChoices(int num);

	// Adds a string with the choice
	void addChoice(const RMString &string);

	// Show and hide the selection, with possible animations.
	// NOTE: If no parameter is passed to Show(), it is the obligation of
	// caller to ensure that the class is inserted into OT list
	void show(CORO_PARAM, RMGfxTargetBuffer *bigBuf = NULL);
	void hide(CORO_PARAM);

	// Polling Update
	void doFrame(CORO_PARAM, RMPoint ptMousePos);

	// Returns the currently selected item, or -1 if none is selected
	int getSelection(void);
};

} // End of namespace Tony

#endif
