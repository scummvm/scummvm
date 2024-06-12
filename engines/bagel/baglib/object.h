
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

#ifndef BAGEL_BAGLIB_OBJECT_H
#define BAGEL_BAGLIB_OBJECT_H

#include "bagel/baglib/expression.h"
#include "bagel/baglib/res.h"
#include "bagel/boflib/gfx/bitmap.h"
#include "bagel/boflib/string.h"
#include "bagel/baglib/parse_object.h"

namespace Bagel {

class CBagObject;
typedef void *(*BagFuncPtr)(int, void *);

enum BagObjectType {
	BASE_OBJ = 0x0000,
	BMP_OBJ = BOF_BMP_OBJ,
	SPRITE_OBJ = BOF_SPRITE_OBJ,
	BUTTON_OBJ = BOF_BUTTON_OBJ,
	SOUND_OBJ = BOF_SOUND_OBJ,
	LINK_OBJ = BOF_LINK_OBJ,
	TEXT_OBJ = BOF_TEXT_OBJ,
	CHAR_OBJ = BOF_CHAR_OBJ,
	VAR_OBJ = BOF_VAR_OBJ,
	AREA_OBJ = BOF_AREA_OBJ,
	EXPRESS_OBJ = BOF_EXPRESS_OBJ,
	COMMAND_OBJ = BOF_COMM_OBJ,
	MOVIE_OBJ = BOF_MOVIE_OBJ,
	THING_OBJ = BOF_THING_OBJ,
	RESPRNT_OBJ = BOF_RESPRNT_OBJ,
	DOSSIER_OBJ = BOF_DOSSIER_OBJ,
	USER_OBJ = 0x1000
};

enum BAG_OBJECT_PROPERTIES {
	NONE = 0x0000,
	MOVABLE = 0x0001,
	MODAL = 0x0002,
	VISIBLE = 0x0004,
	HIGHLIGHT = 0x0008,
	ACTIVE = 0x0010,
	TRANSPAR = 0x0020,
	HIDEONCLK = 0x0040,
	IMRUN = 0x0080,
	LOCAL = 0x0100,
	NEGATIVE = 0x0200,
	CONUPDATE = 0x0400,
	STRETCH = 0x0800,
	TIMELESS = 0x1000,
	FLOATING = 0x2000,
	PRELOAD = 0x4000,
	FOREGROUND = 0x8000
};

class CBagMenu;
class CBagStorageDev;

CBofString getStringTypeOfObject(BagObjectType n);

/**
 * CBofBagObject is an object that can be place within the slide window.
 */
class CBagObject : public CBagParseObject, public CBofObject, public CBofError {
private:
	CBofString _sFileName;         // File name contain object look/feel data
	CBofString *_psName = nullptr; // Name of this object, needed by movable objects only, it equals file name unless specified.
	CBagMenu *_pMenu = nullptr;    // Menu for the object
	CBagExpression *_pEvalExpr = nullptr;  // Pointer to expression to be evaluated by
	CBofString _emptyString;

	int16 _nState = 0;         // Current state of the object
	uint16 _nId = 0;           // Ref Id for an object

protected:
	uint16 _xObjType = AREA_OBJ;

private:
	uint16 _nProperties = 0;   // Properties of object

	int16 _nX = 0;             // Replaces _xPosition
	int16 _nY = 0;

	byte _nOverCursor = 0;    // Ref Id for the objects over cursor
	byte _bDirty : 1;         // Object needs redrawing or not?
	byte _bMsgWaiting : 1;    // Event needing to be played?
	byte _bAlwaysUpdate : 1;  // For message light
	byte _bNoMenu : 1;        // Used by AS NOMENU

protected:
	byte _bInteractive = 0;

	// Object property functionality
	bool isProperty(BAG_OBJECT_PROPERTIES xProp) {
		return _nProperties & xProp;
	}

	void setProperty(BAG_OBJECT_PROPERTIES xProp, bool bVal);

public:
	CBagObject();
	virtual ~CBagObject();

	bool isInteractive() {
		return _bInteractive;
	}
	void setInteractive(bool b) {
		_bInteractive = (byte)b;
	}

	// Callback function functionality - probably can be phased out
	virtual bool runCallBack() {
		return false;
	}

	virtual BagFuncPtr getCallBack() {
		return nullptr;
	}

	// Run Object is called when there is no callback and the item was selected
	virtual bool runObject();

	void setExpression(CBagExpression *pExpr) {
		_pEvalExpr = pExpr;
	}
	CBagExpression *getExpression() const {
		return _pEvalExpr;
	}

	// Return true if the Object had members that are properly initialized/de-initialized
	ErrorCode attach() override;
	ErrorCode detach() override;

	BagObjectType getType() {
		return (BagObjectType)_xObjType;
	}
	void setType(BagObjectType nType) {
		_xObjType = (uint16)nType;
	}

	// Object can be moved within a screen
	virtual bool isInside(const CBofPoint &xPoint) {
		return getRect().ptInRect(xPoint);
	}

	// Object can be moved within a screen
	bool isMovable() {
		return isProperty(MOVABLE);
	}
	void setMovable(bool b = true) {
		setProperty(MOVABLE, b);
	}
	// Object can be stretched within a screen
	bool isStretchable() {
		return isProperty(STRETCH);
	}
	void setStretchable(bool b = true) {
		setProperty(STRETCH, b);
	}
	// Object has exclusive updates to a screen
	bool isModal() {
		return isProperty(MODAL);
	}
	void setModal(bool b = true) {
		setProperty(MODAL, b);
	}
	virtual bool isModalDone() {
		return true;
	}
	//  Is object visible within scene
	bool isVisible() {
		return isProperty(VISIBLE);
	}
	void setVisible(bool b = true) {
		setProperty(VISIBLE, b);
	}
	// Should object be highlighted when the mouse is over
	bool isHighlight() {
		return isProperty(HIGHLIGHT);
	}
	void setHighlight(bool b = true) {
		setProperty(HIGHLIGHT, b);
	}
	// Is the object active in this world
	bool isActive() {
		return isProperty(ACTIVE);
	}
	void setActive(bool b = true) {
		setProperty(ACTIVE, b);
	}
	// Is the object has a transparent background
	bool isTransparent() {
		return isProperty(TRANSPAR);
	}

	virtual void setTransparent(bool b = true) {
		setProperty(TRANSPAR, b);
	}
	// Should the object be hidden when clicked on
	bool isHideOnClick() {
		return isProperty(HIDEONCLK);
	}
	void setHideOnClick(bool b = true) {
		setProperty(HIDEONCLK, b);
	}
	// Should the object run and then be destroyed after the attach
	bool isImmediateRun() {
		return isProperty(IMRUN);
	}
	void setImmediateRun(bool b = true) {
		setProperty(IMRUN, b);
	}
	// Is the object currently local to the object
	bool isLocal() {
		return isProperty(LOCAL);
	}
	void setLocal(bool b = true) {
		setProperty(LOCAL, b);
	}
	// Is the object expression negative
	bool isNegative() {
		return isProperty(NEGATIVE);
	}
	void setNegative(bool b = true) {
		setProperty(NEGATIVE, b);
	}
	// Should the object be constantly updated, even when not on screen
	bool isConstantUpdate() {
		return isProperty(CONUPDATE);
	}
	void setConstantUpdate(bool b = true) {
		setProperty(CONUPDATE, b);
	}
	// Does this objects action take up time
	bool isTimeless() {
		return isProperty(TIMELESS);
	}

	void setTimeless(bool b = true);
	// Does this objects have a set position/or should the sdev provide one when it is attached
	bool isFloating() {
		return isProperty(FLOATING);
	}
	void setFloating(bool b = true) {
		setProperty(FLOATING, b);
	}
	// Does this objects have a set position/or should the sdev provide one when it is attached
	bool isPreload() {
		return isProperty(PRELOAD);
	}
	void setPreload(bool b = true) {
		setProperty(PRELOAD, b);
	}
	// Does this objects have a set position/or should the sdev provide one when it is attached
	bool isForeGround();
	void setForeGround(bool b = true);
	int getProperties();
	void setProperties(int nProperties);

	// Init variables
	virtual const CBofString *getInitInfo() const;
	virtual void setInitInfo(const CBofString &) {}
	virtual int getProperty(const CBofString &sProp);
	virtual void setProperty(const CBofString &, int nVal);

	bool isDirty() {
		return _bDirty != 0;
	}
	
	void setDirty(bool b = true) {
		_bDirty = (byte)b;
	}

	// If this thing is getting purged but is awaiting playback, then mark it as such.
	bool isMsgWaiting() {
		return _bMsgWaiting != 0;
	}
	
	void setMsgWaiting(bool b = true) {
		_bMsgWaiting = (byte)b;
	}

	bool isAlwaysUpdate() {
		return _bAlwaysUpdate != 0;
	}
	
	void setAlwaysUpdate(bool b = true) {
		_bAlwaysUpdate = (byte)b;
	}

	bool isNoMenu() {
		return _bNoMenu;
	}
	
	void setNoMenu(bool b = true) {
		_bNoMenu = (byte)b;
	}

	virtual CBofPoint getPosition();
	virtual int getRefId();
	virtual int getOverCursor();
	virtual int getState();
	virtual CBofRect getRect();
	virtual const CBofString &getFileName();
	CBagMenu *getMenuPtr();
	virtual const CBofString &getRefName();
	virtual void setRefName(const CBofString &s);
	virtual void setFileName(const CBofString &s);
	virtual void setSize(const CBofSize &) {}
	virtual void setRefId(int id);
	virtual void setOverCursor(int curs);
	virtual void setState(int state);
	virtual void setMenuPtr(CBagMenu *pm);
	virtual void setPosition(const CBofPoint &pos);

	/**
	 * Takes in info and then removes the relative information and returns
	 * UNKNOWN_TOKEN    if nothing has changed
	 * UPDATED_OBJECT   if something has changed
	 * PARSING_DONE     if done processing
	 */
	ParseCodes setInfo(CBagIfstream &istr) override;

	virtual ErrorCode update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, int /*nMaskColor*/ = -1);
	virtual bool onObjInteraction(CBagObject * /*pObj*/, CBagStorageDev * /*pSDev*/);
	virtual void onLButtonDown(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr) {}
	virtual void onLButtonUp(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr); // run menu if available
	virtual bool onMouseMove(uint32 /*nFlags*/, CBofPoint /*xPoint*/, void * = nullptr);
	virtual bool onMouseOver(uint32 /*nFlags*/, CBofPoint /*xPoint*/, void * = nullptr);
};

inline void CBagObject::setFileName(const CBofString &s) {
	_sFileName = s;
}

inline void CBagObject::setMenuPtr(CBagMenu *pm) {
	_pMenu = pm;
}

} // namespace Bagel

#endif
