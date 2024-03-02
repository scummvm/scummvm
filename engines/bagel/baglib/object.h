
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
	CBofString m_sFileName;			// File name contain object look/feel data
	CBofString *m_psName = nullptr;	// Name of this object, needed by movable objects only, it equals file name unless specified.
	CBagMenu *m_pMenu = nullptr;	// Menu for the object
	CBagExpression *m_pEvalExpr = nullptr;	// Pointer to expression to be evaluated by

	SHORT m_nState = 0;			// Current state of the object
	USHORT m_nId = 0;			// Ref Id for an object

protected:
	USHORT m_xObjType = AREAOBJ;

private:
	USHORT m_nProperties = 0;	// Properties of object

	SHORT m_nX = 0;				// Replaces m_xPosition
	SHORT m_nY = 0;

	UBYTE m_nOverCursor = 0;	// Ref Id for the objects over cursor
	UBYTE m_bDirty : 1;			// Object needs redrawing or not?
	UBYTE m_bMsgWaiting : 1;	// Event needing to be played?
	UBYTE m_bAlwaysUpdate : 1;	// For message light
	UBYTE m_bNoMenu : 1;		// Used by AS NOMENU

protected:
	UBYTE m_bInteractive = 0;

	// Object property functionality
	BOOL IsProperty(BAG_OBJECT_PROPERTIES xProp) { return m_nProperties & xProp; }

	VOID SetProperty(BAG_OBJECT_PROPERTIES xProp, BOOL bVal);

public:
	CBagObject();
	virtual ~CBagObject();

	BOOL IsInteractive() { return m_bInteractive; }
	VOID SetInteractive(BOOL b) { m_bInteractive = (UBYTE)b; }

	// Callback function functionality - probably can be phased out
	virtual BOOL RunCallBack() { return FALSE; }

#if BOF_MAC && __POWERPC__
	virtual UniversalProcPtr GetCallBack() { return nullptr; }
#else
	virtual BAGFUNCPTR GetCallBack() { return nullptr; }
#endif

	// Run Object is called when there is no callback and the item was selected
	virtual BOOL RunObject();

	VOID SetExpression(CBagExpression *pExpr) { m_pEvalExpr = pExpr; }
	CBagExpression *GetExpression() const { return m_pEvalExpr; }

	// Return TRUE if the Object had members that are properly initialized/de-initialized
	virtual ERROR_CODE Attach();
	virtual ERROR_CODE Detach();

	BAG_OBJECT_TYPE GetType() { return (BAG_OBJECT_TYPE)m_xObjType; }
	VOID SetType(BAG_OBJECT_TYPE nType) { m_xObjType = (USHORT)nType; }

	// Object can be moved within a sceene
	virtual BOOL IsInside(const CBofPoint &xPoint) { return GetRect().PtInRect(xPoint); }

	// Object can be moved within a sceene
	BOOL IsMovable() { return IsProperty(MOVABLE); }
	VOID SetMovable(BOOL b = TRUE) { SetProperty(MOVABLE, b); }
	// Object can be stretched within a sceene
	BOOL IsStretchable() { return IsProperty(STRETCH); }
	VOID SetStretchable(BOOL b = TRUE) { SetProperty(STRETCH, b); }
	// Object has exclusive updates to a sceene
	BOOL IsModal() { return IsProperty(MODAL); }
	VOID SetModal(BOOL b = TRUE) { SetProperty(MODAL, b); }
	virtual BOOL IsModalDone() { return TRUE; }
	//  Is object visible within sceene
	BOOL IsVisible() { return IsProperty(VISIBLE); }
	VOID SetVisible(BOOL b = TRUE) { SetProperty(VISIBLE, b); }
	// Sould object be hightlighed when the mouse is over
	BOOL IsHighlight() { return IsProperty(HIGHLIGHT); }
	VOID SetHighlight(BOOL b = TRUE) { SetProperty(HIGHLIGHT, b); }
	// Is the object active in this world
	BOOL IsActive() { return IsProperty(ACTIVE); }
	VOID SetActive(BOOL b = TRUE) { SetProperty(ACTIVE, b); }
	// Is the object has a trasparent bkground
	BOOL IsTransparent() { return IsProperty(TRANSPAR); }
	VOID SetTransparent(BOOL b = TRUE) { SetProperty(TRANSPAR, b); }
	// Should the object be hidden when clicked on
	BOOL IsHideOnClick() { return IsProperty(HIDEONCLK); }
	VOID SetHideOnClick(BOOL b = TRUE) { SetProperty(HIDEONCLK, b); }
	// Should the object run and then be destroyed after the attach
	BOOL IsImmediateRun() { return IsProperty(IMRUN); }
	VOID SetImmediateRun(BOOL b = TRUE) { SetProperty(IMRUN, b); }
	// Is the object currently local to the object
	BOOL IsLocal() { return IsProperty(LOCAL); }
	VOID SetLocal(BOOL b = TRUE) { SetProperty(LOCAL, b); }
	// Is the object expression negative
	BOOL IsNegative() { return IsProperty(NEGATIVE); }
	VOID SetNegative(BOOL b = TRUE) { SetProperty(NEGATIVE, b); }
	// Should the object be constantly updated, even when not on screen
	BOOL IsConstantUpdate() { return IsProperty(CONUPDATE); }
	VOID SetConstantUpdate(BOOL b = TRUE) { SetProperty(CONUPDATE, b); }
	// Does this objects action take up time
	BOOL IsTimeless() { return IsProperty(TIMELESS); }
	VOID SetTimeless(BOOL b = TRUE) { SetProperty(TIMELESS, b); }
	// Does this objects have a set position/or should the sdev provide one when it is attached
	BOOL IsFloating() { return IsProperty(FLOATING); }
	VOID SetFloating(BOOL b = TRUE) { SetProperty(FLOATING, b); }
	// Does this objects have a set position/or should the sdev provide one when it is attached
	BOOL IsPreload() { return IsProperty(PRELOAD); }
	VOID SetPreload(BOOL b = TRUE) { SetProperty(PRELOAD, b); }
	// Does this objects have a set position/or should the sdev provide one when it is attached
	BOOL IsForeGround() { return IsProperty(FOREGROUND); }
	VOID SetForeGround(BOOL b = TRUE) { SetProperty(FOREGROUND, b); }

	INT GetProperties() { return m_nProperties; }
	VOID SetProperties(INT nProperties) { m_nProperties = (USHORT)nProperties; }

	// Init variables
	virtual const CBofString *GetInitInfo() { return nullptr; }
	virtual VOID SetInitInfo(const CBofString &) {}

	virtual INT GetProperty(const CBofString &sProp);
	virtual VOID SetProperty(const CBofString &, int nVal);

	BOOL IsDirty() { return m_bDirty != 0; }
	VOID SetDirty(BOOL b = TRUE) { m_bDirty = (UBYTE)b; }

	// If this thing is getting purged but is awaiting playback, then mark it as such.
	BOOL IsMsgWaiting() { return m_bMsgWaiting != 0; }
	VOID SetMsgWaiting(BOOL b = TRUE) { m_bMsgWaiting = (UBYTE)b; }

	BOOL IsAlwaysUpdate() { return m_bAlwaysUpdate != 0; }
	VOID SetAlwaysUpdate(BOOL b = TRUE) { m_bAlwaysUpdate = (UBYTE)b; }

	BOOL IsNoMenu() { return m_bNoMenu; }
	VOID SetNoMenu(BOOL b = TRUE) { m_bNoMenu = (UBYTE)b; }

	virtual CBofPoint GetPosition() { return CBofPoint(m_nX, m_nY); }
	virtual INT GetRefId() { return m_nId; }
	virtual INT GetOverCursor() { return m_nOverCursor; }
	virtual INT GetState() { return m_nState; }
	virtual CBofRect GetRect() { return CBofRect(m_nX, m_nY, m_nX - 1, m_nY - 1); }

	virtual const CBofString &
	GetFileName() { return m_sFileName; }
	CBagMenu *GetMenuPtr() { return m_pMenu; }
	virtual const CBofString &
	GetRefName();
	virtual VOID SetRefName(const CBofString &s);

	virtual VOID SetFileName(const CBofString &s) { m_sFileName = s; }
	virtual VOID SetSize(const CBofSize &) {}
	virtual VOID SetRefId(INT id) {
		Assert(id >= 0 && id <= 0xFFFF);
		m_nId = (USHORT)id;
	}
	virtual VOID SetOverCursor(INT curs) { m_nOverCursor = (UBYTE)curs; }
	virtual VOID SetState(INT state) {
		Assert(ABS(state) < 0x8000);
		m_nState = (SHORT)state;
	}
	virtual VOID SetMenuPtr(CBagMenu *pm) { m_pMenu = pm; }
	virtual VOID SetPosition(const CBofPoint &pos) {
		m_nX = (SHORT)pos.x;
		m_nY = (SHORT)pos.y;
	}

	virtual PARSE_CODES SetInfo(bof_ifstream &istr);

	virtual INT HandleError(int nErrID) {
		Assert(!nErrID);
		return nErrID;
	}

	virtual ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, INT /*nMaskColor*/ = -1);

	// virtual ERROR_CODE  DoModal(CBofBitmap * pBmp, CBofPoint pt, CBofRect * pSrcRect = nullptr, INT /*nMaskColor*/ = -1);

	virtual BOOL OnObjInteraction(CBagObject * /*pObj*/, CBagStorageDev * /*pSDev*/) { return FALSE; }

	virtual BOOL OnLButtonDown(UINT /*nFlags*/, CBofPoint /*xPoint*/, void * = nullptr) { return FALSE; }
	virtual BOOL OnLButtonUp(UINT /*nFlags*/, CBofPoint /*xPoint*/, void * = nullptr); //{ return FALSE; } run menu if availible
	// virtual BOOL        OnLButtonDblClk(UINT /*nFlags*/, CBofPoint /*xPoint*/, void * = nullptr);	//{ return FALSE; }
	virtual BOOL OnMouseMove(UINT /*nFlags*/, CBofPoint /*xPoint*/, void * = nullptr); //{ return FALSE; }
	virtual BOOL OnMouseOver(UINT /*nFlags*/, CBofPoint /*xPoint*/, void * = nullptr) { return FALSE; }
};

} // namespace Bagel

#endif
