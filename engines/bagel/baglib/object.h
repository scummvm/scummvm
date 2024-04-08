
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

#include "bagel/boflib/stdinc.h"
#include "bagel/baglib/expression.h"
#include "bagel/baglib/res.h"
#include "bagel/boflib/gfx/bitmap.h"
#include "bagel/boflib/string.h"
#include "bagel/boflib/res.h"
#include "bagel/baglib/parse_object.h"

namespace Bagel {

class CBagObject;
typedef void *(*BAGFUNCPTR)(int, void *);

enum BAG_OBJECT_TYPE {
	BASEOBJ = 0x0000,
	BMPOBJ = BOFBMPOBJ,
	SPRITEOBJ = BOFSPRITEOBJ,
	BUTTONOBJ = BOFBUTTONOBJ,
	SOUNDOBJ = BOFSOUNDOBJ,
	LINKOBJ = BOFLINKOBJ,
	TEXTOBJ = BOFTEXTOBJ,
	CHAROBJ = BOFCHAROBJ,
	VAROBJ = BOFVAROBJ,
	AREAOBJ = BOFAREAOBJ,
	EXPRESSOBJ = BOFEXPRESSOBJ,
	COMMANDOBJ = BOFCOMMOBJ,
	MOVIEOBJ = BOFMOVIEOBJ,
	THNGOBJ = BOFTHINGOBJ,
	RESPRNTOBJ = BOFRESPRNTOBJ,
	DOSSIEROBJ = BOFDOSSIEROBJ,
	USEROBJ = 0x1000
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

CBofString GetStringTypeOfObject(BAG_OBJECT_TYPE n);

#if BOF_MAC && __POWERPC__
enum {
	uppCBagObjectProcInfo = kRegisterBased | REGISTER_ROUTINE_PARAMETER(1, kRegisterD0, SIZE_CODE(sizeof(int))) | REGISTER_ROUTINE_PARAMETER(2, kRegisterA0, SIZE_CODE(sizeof(void *)))
};
#endif

/**
 * CBofBagObject is an object that can be place within the slide window.
 */
class CBagObject : public CBagParseObject, public CBofObject, public CBofError {
private:
	CBofString m_sFileName;         // File name contain object look/feel data
	CBofString *m_psName = nullptr; // Name of this object, needed by movable objects only, it equals file name unless specified.
	CBagMenu *m_pMenu = nullptr;    // Menu for the object
	CBagExpression *m_pEvalExpr = nullptr;  // Pointer to expression to be evaluated by

	int16 m_nState = 0;         // Current state of the object
	uint16 m_nId = 0;           // Ref Id for an object

protected:
	uint16 m_xObjType = AREAOBJ;

private:
	uint16 m_nProperties = 0;   // Properties of object

	int16 m_nX = 0;             // Replaces m_xPosition
	int16 m_nY = 0;

	byte m_nOverCursor = 0;    // Ref Id for the objects over cursor
	byte m_bDirty : 1;         // Object needs redrawing or not?
	byte m_bMsgWaiting : 1;    // Event needing to be played?
	byte m_bAlwaysUpdate : 1;  // For message light
	byte m_bNoMenu : 1;        // Used by AS NOMENU

protected:
	byte m_bInteractive = 0;

	// Object property functionality
	bool IsProperty(BAG_OBJECT_PROPERTIES xProp) {
		return m_nProperties & xProp;
	}

	void SetProperty(BAG_OBJECT_PROPERTIES xProp, bool bVal);

public:
	CBagObject();
	virtual ~CBagObject();

	bool IsInteractive() {
		return m_bInteractive;
	}
	void SetInteractive(bool b) {
		m_bInteractive = (byte)b;
	}

	// Callback function functionality - probably can be phased out
	virtual bool RunCallBack() {
		return FALSE;
	}

#if BOF_MAC && __POWERPC__
	virtual UniversalProcPtr GetCallBack() {
		return nullptr;
	}
#else
	virtual BAGFUNCPTR GetCallBack() {
		return nullptr;
	}
#endif

	// Run Object is called when there is no callback and the item was selected
	virtual bool RunObject();

	void SetExpression(CBagExpression *pExpr) {
		m_pEvalExpr = pExpr;
	}
	CBagExpression *GetExpression() const {
		return m_pEvalExpr;
	}

	// Return true if the Object had members that are properly initialized/de-initialized
	virtual ERROR_CODE Attach();
	virtual ERROR_CODE Detach();

	BAG_OBJECT_TYPE GetType() {
		return (BAG_OBJECT_TYPE)m_xObjType;
	}
	void SetType(BAG_OBJECT_TYPE nType) {
		m_xObjType = (uint16)nType;
	}

	// Object can be moved within a sceene
	virtual bool IsInside(const CBofPoint &xPoint) {
		return GetRect().PtInRect(xPoint);
	}

	// Object can be moved within a sceene
	bool IsMovable() {
		return IsProperty(MOVABLE);
	}
	void SetMovable(bool b = true) {
		SetProperty(MOVABLE, b);
	}
	// Object can be stretched within a sceene
	bool IsStretchable() {
		return IsProperty(STRETCH);
	}
	void SetStretchable(bool b = true) {
		SetProperty(STRETCH, b);
	}
	// Object has exclusive updates to a sceene
	bool IsModal() {
		return IsProperty(MODAL);
	}
	void SetModal(bool b = true) {
		SetProperty(MODAL, b);
	}
	virtual bool IsModalDone() {
		return true;
	}
	//  Is object visible within sceene
	bool IsVisible() {
		return IsProperty(VISIBLE);
	}
	void SetVisible(bool b = true) {
		SetProperty(VISIBLE, b);
	}
	// Sould object be hightlighed when the mouse is over
	bool IsHighlight() {
		return IsProperty(HIGHLIGHT);
	}
	void SetHighlight(bool b = true) {
		SetProperty(HIGHLIGHT, b);
	}
	// Is the object active in this world
	bool IsActive() {
		return IsProperty(ACTIVE);
	}
	void SetActive(bool b = true) {
		SetProperty(ACTIVE, b);
	}
	// Is the object has a trasparent bkground
	bool IsTransparent() {
		return IsProperty(TRANSPAR);
	}
	void SetTransparent(bool b = true) {
		SetProperty(TRANSPAR, b);
	}
	// Should the object be hidden when clicked on
	bool IsHideOnClick() {
		return IsProperty(HIDEONCLK);
	}
	void SetHideOnClick(bool b = true) {
		SetProperty(HIDEONCLK, b);
	}
	// Should the object run and then be destroyed after the attach
	bool IsImmediateRun() {
		return IsProperty(IMRUN);
	}
	void SetImmediateRun(bool b = true) {
		SetProperty(IMRUN, b);
	}
	// Is the object currently local to the object
	bool IsLocal() {
		return IsProperty(LOCAL);
	}
	void SetLocal(bool b = true) {
		SetProperty(LOCAL, b);
	}
	// Is the object expression negative
	bool IsNegative() {
		return IsProperty(NEGATIVE);
	}
	void SetNegative(bool b = true) {
		SetProperty(NEGATIVE, b);
	}
	// Should the object be constantly updated, even when not on screen
	bool IsConstantUpdate() {
		return IsProperty(CONUPDATE);
	}
	void SetConstantUpdate(bool b = true) {
		SetProperty(CONUPDATE, b);
	}
	// Does this objects action take up time
	bool IsTimeless() {
		return IsProperty(TIMELESS);
	}
	void SetTimeless(bool b = true) {
		SetProperty(TIMELESS, b);
	}
	// Does this objects have a set position/or should the sdev provide one when it is attached
	bool IsFloating() {
		return IsProperty(FLOATING);
	}
	void SetFloating(bool b = true) {
		SetProperty(FLOATING, b);
	}
	// Does this objects have a set position/or should the sdev provide one when it is attached
	bool IsPreload() {
		return IsProperty(PRELOAD);
	}
	void SetPreload(bool b = true) {
		SetProperty(PRELOAD, b);
	}
	// Does this objects have a set position/or should the sdev provide one when it is attached
	bool IsForeGround() {
		return IsProperty(FOREGROUND);
	}
	void SetForeGround(bool b = true) {
		SetProperty(FOREGROUND, b);
	}

	int GetProperties() {
		return m_nProperties;
	}
	void SetProperties(int nProperties) {
		m_nProperties = (uint16)nProperties;
	}

	// Init variables
	virtual const CBofString *GetInitInfo() const {
		return nullptr;
	}
	virtual void SetInitInfo(const CBofString &) {}

	virtual int GetProperty(const CBofString &sProp);
	virtual void SetProperty(const CBofString &, int nVal);

	bool IsDirty() {
		return m_bDirty != 0;
	}
	void SetDirty(bool b = true) {
		m_bDirty = (byte)b;
	}

	// If this thing is getting purged but is awaiting playback, then mark it as such.
	bool IsMsgWaiting() {
		return m_bMsgWaiting != 0;
	}
	void SetMsgWaiting(bool b = true) {
		m_bMsgWaiting = (byte)b;
	}

	bool IsAlwaysUpdate() {
		return m_bAlwaysUpdate != 0;
	}
	void SetAlwaysUpdate(bool b = true) {
		m_bAlwaysUpdate = (byte)b;
	}

	bool IsNoMenu() {
		return m_bNoMenu;
	}
	void SetNoMenu(bool b = true) {
		m_bNoMenu = (byte)b;
	}

	virtual CBofPoint GetPosition() {
		return CBofPoint(m_nX, m_nY);
	}
	virtual int GetRefId() {
		return m_nId;
	}
	virtual int GetOverCursor() {
		return m_nOverCursor;
	}
	virtual int GetState() {
		return m_nState;
	}
	virtual CBofRect GetRect() {
		return CBofRect(m_nX, m_nY, m_nX - 1, m_nY - 1);
	}

	virtual const CBofString &
	GetFileName() {
		return m_sFileName;
	}
	CBagMenu *GetMenuPtr() {
		return m_pMenu;
	}
	virtual const CBofString &GetRefName();
	virtual void SetRefName(const CBofString &s);

	virtual void SetFileName(const CBofString &s) {
		m_sFileName = s;
	}
	virtual void SetSize(const CBofSize &) {}
	virtual void SetRefId(int id) {
		Assert(id >= 0 && id <= 0xFFFF);
		m_nId = (uint16)id;
	}
	virtual void SetOverCursor(int curs) {
		m_nOverCursor = (byte)curs;
	}
	virtual void SetState(int state) {
		Assert(ABS(state) < 0x8000);
		m_nState = (int16)state;
	}
	virtual void SetMenuPtr(CBagMenu *pm) {
		m_pMenu = pm;
	}
	virtual void SetPosition(const CBofPoint &pos) {
		m_nX = (int16)pos.x;
		m_nY = (int16)pos.y;
	}

	/**
	 * Takes in info and then removes the relative information and returns
	 * UNKNOWN_TOKEN    if nothing has changed
	 * UPDATED_OBJECT   if something has changed
	 * PARSING_DONE     if done processing
	 */
	virtual PARSE_CODES SetInfo(bof_ifstream &istr);

	virtual int HandleError(int nErrID) {
		Assert(!nErrID);
		return nErrID;
	}

	virtual ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, int /*nMaskColor*/ = -1);

	virtual bool OnObjInteraction(CBagObject * /*pObj*/, CBagStorageDev * /*pSDev*/) {
		return FALSE;
	}

	virtual void OnLButtonDown(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr) {}
	virtual void OnLButtonUp(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr); //{ } run menu if available
	// virtual bool        OnLButtonDblClk(uint32 /*nFlags*/, CBofPoint /*xPoint*/, void * = nullptr);    //{ return FALSE; }
	virtual bool OnMouseMove(uint32 /*nFlags*/, CBofPoint /*xPoint*/, void * = nullptr); //{ return FALSE; }
	virtual bool OnMouseOver(uint32 /*nFlags*/, CBofPoint /*xPoint*/, void * = nullptr) {
		return FALSE;
	}
};

} // namespace Bagel

#endif
