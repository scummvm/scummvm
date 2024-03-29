
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

#ifndef BAGEL_BAGLIB_STORAGE_DEV_WIN_H
#define BAGEL_BAGLIB_STORAGE_DEV_WIN_H

#include "bagel/boflib/gui/dialog.h"
#include "bagel/boflib/list.h"
#include "bagel/baglib/object.h"
#include "bagel/baglib/var.h"
#include "bagel/baglib/save_game_file.h"

namespace Bagel {

#define SDEV_UNDEF 00
#define SDEV_WND 11
#define SDEV_DLG 22
#define SDEV_BMP 33
#define SDEV_PDA 44
#define SDEV_WIELD 55
#define SDEV_CLOSEP 66
#define SDEV_GAMEWIN 77
#define SDEV_ZOOMPDA 88

/**
 * CResource is an encapsulation of resources.
 */
class CResource {
private:
	LPCTSTR m_lpszResourceName;
	UINT m_nResId;

public:
	CResource() {
		m_lpszResourceName = nullptr;
		m_nResId = 0;
	}

	VOID SetResource(LPCTSTR lpszResourceName) {
		m_lpszResourceName = lpszResourceName;
	}
	VOID SetResource(UINT nResId) {
		m_nResId = nResId;
	}
	LPCTSTR GetResourceName() {
		return m_lpszResourceName;
	}
	UINT GetResourceId() {
		return m_nResId;
	}
};

// Filter function function prototype.
//
// for power pc, just pretend our filter function is upp
#if BOF_MAC && __POWERPC__
typedef UniversalProcPtr FilterFunction;
#else
typedef BOOL (*FilterFunction)(const USHORT nFilterId, CBofBitmap *, CBofRect *);
#endif

// Callbacks are a little tricky for powerpc...
#if BOF_MAC && __POWERPC__
enum {
	uppFilterProcInfo = kRegisterBased | REGISTER_ROUTINE_PARAMETER(1, kRegisterD0, SIZE_CODE(sizeof(USHORT))) | REGISTER_ROUTINE_PARAMETER(2, kRegisterA0, SIZE_CODE(sizeof(CBofBitmap *))) | REGISTER_ROUTINE_PARAMETER(3, kRegisterA1, SIZE_CODE(sizeof(CBofRect *)))
};
#endif

/**
 * CBagPanWindow is a window that contains a slide bitmap object.  It has specialize
 *  functions for handling slide bitmaps and slide objects.
 */
class CBagStorageDev : public CBagParseObject {
public:
	enum MOUSE_ACTIVITY { kMouseNONE = 0x0000, kMouseDRAGGING = 0x0001 };

private:
	CBofString m_sName;           // Name of this storage device
	CBofString m_sBackgroundName; // Name of this storage devices background
	CBofString m_sPrevSDev;       // Name of the previous storage device

	CBofRect m_cDestRect;      // Position of storage device
	CBofPoint m_xPrevLocation; // Previous view location in bmp.

	CBagObject *m_pLActiveObject; // The last object selected on mouse down
	// CBagObject          *m_pRActiveObject;       // The last object selected on mouse down

	CBofList<CBagObject *> *m_pObjectList;         // Objects to be painted to the pan
	CBofList<CBagExpression *> *m_pExpressionList; // Condition for painting to the pan

	CBofWindow *m_pAssociateWnd; // Sounds need a window

	MOUSE_ACTIVITY m_nCurrMouseActivity; // What is up with the mouse currently

	USHORT m_nExitOnEdge; // Non zero if SDev should be closed when an outside edge is clicked on, and and prev sdev is set, m_nExitOnEdge is the thickness of the edge

	USHORT m_nFilterId; // Filter id number 0 for normal view
	USHORT m_nFadeId;   // Fade id number 0 for no fade in
	USHORT m_nDiskID;

protected:
	USHORT m_xSDevType;                 // Type of storage device
	static CBofPoint m_xCursorLocation; // Current cursor location in bmp.
	static BOOL m_bHandledUpEvent;      // Hack to stop
	static BOOL m_bHidePDA;
	FilterFunction m_pBitmapFilter; // Pointer to the bitmap filter.

	BOOL m_bForiegnList : 1;   // True if SetObjectList has been called
	BOOL m_bCloseOnOpen : 1;   // True if other SDev should be closed when this is opened
	BOOL m_bContainsModal : 1; // True if SDev contains a modal object
	BOOL m_bCloseup : 1;       // TRUE if is a closeup (includes CIC, or CHAT)
	BOOL m_bCIC : 1;           // TRUE if is a CIC
	BOOL m_bCustom : 1;        // TRUE if is a hand coded closeup
	BOOL m_bFirstPaint : 1;    // run object stuff

	static BOOL m_bPanPreFiltered;  // Let pda know if screens been prefiltered
	static BOOL m_bDirtyAllObjects; // Dirty all objects in prefilter?
	static BOOL m_bPreFilter;

	INT m_nFloatPages; // The number of pages required to display all floating objects

public:
	CBagStorageDev();
	virtual ~CBagStorageDev();

	virtual ERROR_CODE PreFilter(CBofBitmap *pBmp, CBofRect *pRect, CBofList<CBagObject *> *pList = nullptr);

	/**
	 * Make all the objects in a list dirty
	 */
	VOID MakeListDirty(CBofList<CBagObject *> *pList);

	VOID SetDiskID(USHORT nDiskID) {
		m_nDiskID = nDiskID;
	}
	USHORT GetDiskID() {
		return m_nDiskID;
	}

	virtual ERROR_CODE SetLoadFilePos(const CBofPoint) {
		return ERR_NONE;
	}

	BOOL IsCloseup() {
		return m_bCloseup;
	}
	VOID SetCloseup(BOOL b = TRUE) {
		m_bCloseup = b;
	}

	// Set true if sdef is "AS CIC"
	BOOL IsCIC();
	VOID SetCIC(BOOL b = TRUE) {
		m_bCIC = b;
	}

	// Set to true if this is a hand coded closeup
	BOOL IsCustom() {
		return m_bCustom;
	}
	VOID SetCustom(BOOL b = TRUE) {
		m_bCustom = b;
	}

	INT GetDeviceType() {
		return m_xSDevType;
	}

	INT GetObjectCount();
	CBagObject *GetObjectByPos(INT nIndex);
	CBagObject *GetObject(INT nRefId, BOOL bActiveOnly = FALSE);
	CBagObject *GetObject(const CBofPoint &xPoint, BOOL bActiveOnly = FALSE);
	CBagObject *GetObject(const CBofString &sName, BOOL bActiveOnly = FALSE);
	CBagObject *GetObjectByType(const CBofString &sName, BOOL bActiveOnly = FALSE);
	CBofPoint &GetLastCursorLocation() {
		return m_xCursorLocation;
	}
	CBofList<CBagObject *> *
	GetObjectList() {
		return m_pObjectList;
	}
	VOID SetObjectList(CBofList<CBagObject *> *pList, CBofList<CBagExpression *> *pEList = nullptr);

	BOOL Contains(CBagObject *pObj, BOOL bActive = TRUE);

	// virtual CBofRect GetLocation()                       { return CBofRect(); }
	CBofRect GetRect() {
		return m_cDestRect;
	}
	VOID SetRect(const CBofRect &xRect) {
		m_cDestRect = xRect;
	}
	CBofPoint GetPosition() {
		return m_cDestRect.TopLeft();
	}
	VOID SetPosition(const CBofPoint &pos);

	virtual ERROR_CODE AddObject(CBagObject *pObj, int nPos = 0); // Add a new object
	virtual ERROR_CODE ActivateLocalObject(CBagObject *pObj);
	virtual ERROR_CODE ActivateLocalObject(const CBofString &sName);
	virtual ERROR_CODE AttachActiveObjects();

	virtual ERROR_CODE RemoveObject(CBagObject *pObj);
	virtual ERROR_CODE DeactivateLocalObject(CBagObject *pObj);
	virtual ERROR_CODE DeactivateLocalObject(const CBofString &sName);
	virtual ERROR_CODE DetachActiveObjects();

	virtual VOID SetHelpFilename(const CBofString &) {}

	const CBofString &GetName() {
		return m_sName;
	}
	VOID SetName(const CBofString &str) {
		m_sName = str;
	}

	VOID SetFilterId(USHORT nId) {
		m_nFilterId = nId;
	}
	USHORT GetFilterId() {
		return m_nFilterId;
	}

	VOID SetFadeId(USHORT nId) {
		m_nFadeId = nId;
	}
	USHORT GetFadeId() {
		return m_nFadeId;
	}

	const CBofString &GetPrevSDev() {
		return m_sPrevSDev;
	}
	const CBofPoint GetPrevLoc() {
		return m_xPrevLocation;
	}
	VOID SetPrevSDev(const CBofString &str) {
		m_sPrevSDev = str;
	}

	BOOL GetCloseOnOpen() {
		return m_bCloseOnOpen;
	}
	VOID SetCloseOnOpen(BOOL bVal) {
		m_bCloseOnOpen = bVal;
	}

	BOOL GetContainsModal() {
		return m_bContainsModal;
	}
	VOID SetContainsModal(BOOL bVal) {
		m_bContainsModal = bVal;
	}

	USHORT GetExitOnEdge() {
		return m_nExitOnEdge;
	}
	VOID SetExitOnEdge(USHORT nVal) {
		m_nExitOnEdge = nVal;
	}

	CBagObject *GetLActiveObject() {
		return m_pLActiveObject;
	}
	// CBagObject*      GetRActiveObject()                  { return m_pRActiveObject; }
	ERROR_CODE SetLActiveObject(CBagObject *pObj) {
		m_pLActiveObject = pObj;
		return ERR_NONE;
	}
	// ERROR_CODE           SetRActiveObject(CBagObject* pObj)  { m_pRActiveObject = pObj; return 0;}

	virtual ERROR_CODE OnLActiveObject(UINT /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);
	// virtual ERROR_CODE   OnRActiveObject(UINT /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);

	MOUSE_ACTIVITY GetLActivity() {
		return m_nCurrMouseActivity;
	}
	VOID SetLActivity(MOUSE_ACTIVITY ma) {
		m_nCurrMouseActivity = ma;
	}

	ERROR_CODE PaintStorageDevice(CBofWindow *pWnd, CBofBitmap *pBmp = nullptr, CBofRect * = nullptr);

	virtual VOID HandleError(ERROR_CODE errCode);

	// The associated window describes which window to get screen information from and
	// where to paint objects and most importantly what info to send to the callbacks
	virtual VOID SetAssociateWnd(CBofWindow *pWnd) {
		m_pAssociateWnd = pWnd;
	}
	virtual CBofWindow *GetAssociateWnd() {
		return m_pAssociateWnd;
	}

	virtual PARSE_CODES SetInfo(bof_ifstream &fpInput); // This function call the pure virt set background

	virtual ERROR_CODE SetBackground(CBofBitmap *pBmp) = 0; // This could be eliminated but is kept in to insure good class usage
	virtual CBofBitmap *GetBackground() = 0;                // Think about it, you can figure this out
	const CBofString &GetBackgroundName() {
		return m_sBackgroundName;
	}

	virtual ERROR_CODE Attach(); // This function attachs the background and nessasary bitmaps
	virtual ERROR_CODE Detach(); // This function attachs the background and nessasary bitmaps

	virtual ERROR_CODE Close();

	virtual ERROR_CODE LoadObjects();
	virtual ERROR_CODE ReleaseObjects();
	// virtual ERROR_CODE   InsertObjects(CBagObject *);
	virtual ERROR_CODE NoObjectsUnderMouse();

	virtual const CBofPoint DevPtToViewPort(const CBofPoint &xPoint) {
		return xPoint;
	}
	virtual const CBofPoint ViewPortToDevPt(const CBofPoint &xPoint) {
		return xPoint;
	}

	// virtual ERROR_CODE   SaveFile(ostream& fpOutput);
	virtual ERROR_CODE LoadFile(const CBofString &sFile);
	virtual ERROR_CODE LoadFileFromStream(bof_ifstream &fpInput, const CBofString &sWldName, BOOL bAttach = TRUE);

	virtual void OnMouseMove(UINT /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);
	virtual ERROR_CODE OnMouseOver(UINT /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);
	virtual void OnLButtonDown(UINT /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);
	virtual void OnLButtonUp(UINT /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);

	virtual ERROR_CODE OnCursorUpdate(INT /*nCurrObj*/) {
		return ERR_NONE;
	}
	virtual CBagObject *OnNewSpriteObject(const CBofString &sInit);
	virtual CBagObject *OnNewBitmapObject(const CBofString &sInit);
	virtual CBagObject *OnNewTextObject(const CBofString &sInit);
	virtual CBagObject *OnNewLinkObject(const CBofString &sInit);
	virtual CBagObject *OnNewRPObject(const CBofString &sInit);
	virtual CBagObject *OnNewEDObject(const CBofString &sInit);
	virtual CBagObject *OnNewDosObject(const CBofString &sInit);
	virtual CBagObject *OnNewSoundObject(const CBofString &sInit);
	virtual CBagObject *OnNewButtonObject(const CBofString &sInit);
	virtual CBagObject *OnNewCharacterObject(const CBofString &sInit);
	virtual CBagObject *OnNewMovieObject(const CBofString &);
	virtual CBagObject *OnNewCommandObject(const CBofString &sInit);
	virtual CBagObject *OnNewAreaObject(const CBofString &sInit);
	virtual CBagObject *OnNewExpressionObject(const CBofString &sInit);
	virtual CBagObject *OnNewVariableObject(const CBofString &sInit);
	virtual CBagObject *OnNewUserObject(const CBofString &sInit);
	virtual CBagObject *OnNewThingObject(const CBofString &sInit);

	// Call to arrange floating object, override to customize layout
	virtual CBofPoint ArrangeFloater(CBofPoint nPos, CBagObject *pObj);

	// Set and Get the number of pages required to display all floating objects
	int GetNumFloatPages() {
		return m_nFloatPages;
	}
	VOID SetNumFloatPages(int nFloatPages) {
		m_nFloatPages = nFloatPages;
	}

	// Correctly set the filter function for the storage device.
	//
	VOID OnSetFilter(BOOL (*FilterFxn)(const USHORT nFilterId, CBofBitmap *, CBofRect *));

	// Get a pointer to the filter function
	//
	FilterFunction GetFilter();

	// Predicate to test if this storage device is filtered.
	//
	BOOL IsFiltered() {
		return m_nFilterId != 0;
	}

	static CBofBitmap *m_pUnderCursorBmp; // For preserving under bitmap.
	static BOOL m_bDrawCursorBackdrop;    // Draw cursor backdrop

	// Provide a method to get at the above vars
	static VOID SetDrawCursorBackdrop(BOOL b) {
		m_bDrawCursorBackdrop = b;
	}
	static BOOL DrawCursorBackdrop() {
		return m_bDrawCursorBackdrop;
	}

	// Provide a method to get at the above vars
	static VOID SetDirtyAllObjects(BOOL b) {
		m_bDirtyAllObjects = b;
	}
	static BOOL GetDirtyAllObjects() {
		return m_bDirtyAllObjects;
	}

	// Provide a method to let PDA know that it should update everything
	static VOID SetPreFiltered(BOOL b = TRUE) {
		m_bPanPreFiltered = b;
	}
	static BOOL GetPreFiltered() {
		return m_bPanPreFiltered;
	}

	// We won't always call the prefilter, just when explicitly instructed to.
	static BOOL PreFilterPan() {
		return m_bPreFilter;
	}
	static VOID SetPreFilterPan(BOOL b = TRUE) {
		m_bPreFilter = b;
		m_bDirtyAllObjects = b;
	}
};

class CBagEventSDev;

//
// CBagStorageDevWnd -
//  CBagPanWindow is a window that contains a slide bitmap object.  It has specialize
//  functions for handling slide bitmaps and slide objects.
//
class CBagStorageDevWnd : public CBofWindow, public CBagStorageDev {
private:
	BOOL m_bMadeSelection;
	BOOL m_bOnUpdate;
	CBofBitmap *m_pWorkBmp;
	CBofString m_sHelpFileName; // Name of the help file for this device

protected:
	bool _freezeCursor = false;

public:
	static CBagEventSDev *m_pEvtSDev; // Pointer to the Event Storage Device

	CBagStorageDevWnd();
	virtual ~CBagStorageDevWnd();
	static void initialize() {
		m_pEvtSDev = nullptr;
	}

	virtual ERROR_CODE Attach(); // This function attachs the background and nessasary bitmaps
	virtual ERROR_CODE Detach(); // This function attachs the background and nessasary bitmaps

	virtual ERROR_CODE Close();
	virtual ERROR_CODE RunModal(CBagObject *pObj);
	virtual VOID OnTimer(UINT nTimerId);

	VOID SetOnUpdate(BOOL bVal = TRUE) {
		m_bOnUpdate = bVal;
	}
	BOOL GetOnUpdate() {
		return m_bOnUpdate;
	}

	virtual ERROR_CODE PaintScreen(CBofRect *pRect = nullptr, BOOL bPaintCursor = TRUE);
	ERROR_CODE PaintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp,
	                        CBofRect &viewOffsetRect, CBofList<CBofRect> * = nullptr, BOOL tempVar = TRUE);
	ERROR_CODE PaintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp) {
		CBofRect emptyRect;
		return PaintObjects(list, pBmp, emptyRect);
	}

	ERROR_CODE PaintWithCursor(CBofBitmap *pBmp, CBofRect *pRect = nullptr);

	virtual CBofRect GetLocation() {
		return GetWindowRect();
	}

	virtual ERROR_CODE SetBackground(CBofBitmap *pBmp);
	virtual CBofBitmap *GetBackground() {
		return GetBackdrop();
	}
	virtual CBofBitmap *GetWorkBmp() {
		return m_pWorkBmp;
	}

	virtual ERROR_CODE LoadFile(const CBofString &sWldFile);

	virtual const CBofString &GetHelpFilename() {
		return m_sHelpFileName;
	}
	virtual VOID SetHelpFilename(const CBofString &s) {
		m_sHelpFileName = s;
	}

	virtual ERROR_CODE OnRender(CBofBitmap *pBmp, CBofRect *pRect = nullptr);
	virtual VOID OnPaint(CBofRect *);
	virtual VOID OnMainLoop();
	VOID OnClose();
	void OnMouseMove(UINT nFlags, CBofPoint *, void * = nullptr);
	void OnLButtonDown(UINT nFlags, CBofPoint *point, void * = nullptr);
	void OnLButtonUp(UINT nFlags, CBofPoint *point, void * = nullptr);

	VOID OnKeyHit(ULONG lKey, ULONG nRepCount);

protected:
	virtual ERROR_CODE SetWorkBmp();
	virtual ERROR_CODE KillWorkBmp();
};

/**
 * CBagStorageDevDlg is a window that contains a slide bitmap object.
 * It has specialize functions for handling slide bitmaps and slide objects.
 */
class CBagStorageDevDlg : public CBofDialog, public CBagStorageDev {
private:
	BOOL m_bMadeSelection;
	CBofString m_sHelpFileName; // Name of the help file for this device

public:
	CBagStorageDevDlg();

	virtual ERROR_CODE PaintScreen(CBofRect *pRect = nullptr, BOOL bPaintCursor = TRUE);
	ERROR_CODE PaintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp,
	                        CBofRect &viewOffsetRect, CBofList<CBofRect> * = nullptr, BOOL tempVar = TRUE);
	ERROR_CODE PaintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp) {
		CBofRect emptyRect;
		return PaintObjects(list, pBmp, emptyRect);
	}

	virtual CBofRect GetLocation() {
		return GetWindowRect();
	}

	virtual ERROR_CODE SetBackground(CBofBitmap *pBmp) {
		if (pBmp)
			return SetBackdrop(pBmp);
		else
			KillBackdrop();
		return ERR_NONE;
	}
	virtual CBofBitmap *GetBackground() {
		return GetBackdrop();
	}

	virtual ERROR_CODE LoadFile(const CBofString &sWldFile);

	ERROR_CODE Create(const CHAR *pszName, INT x = 0, INT y = 0, INT nWidth = USE_DEFAULT, INT nHeight = USE_DEFAULT, CBofWindow *pParent = nullptr, UINT nControlID = 0);
	ERROR_CODE Create(const CHAR *pszName, CBofRect *pRect = nullptr, CBofWindow *pParent = nullptr, UINT nControlID = 0);

	virtual ERROR_CODE Attach(); // This function attachs the background and nessasary bitmaps

	virtual ERROR_CODE Close();

	virtual const CBofString &GetHelpFilename() {
		return m_sHelpFileName;
	}
	virtual VOID SetHelpFilename(const CBofString &s) {
		m_sHelpFileName = s;
	}

	virtual VOID OnMainLoop();
	virtual ERROR_CODE OnRender(CBofBitmap *pBmp, CBofRect *pRect = nullptr);

	virtual VOID OnPaint(CBofRect *);
	virtual VOID OnClose();
	void OnMouseMove(UINT nFlags, CBofPoint *, void * = nullptr);
	void OnLButtonDown(UINT nFlags, CBofPoint *point, void * = nullptr);
	void OnLButtonUp(UINT nFlags, CBofPoint *point, void * = nullptr);
};

/**
 * Tracks all the storage devices
 */
class CBagStorageDevManager : public CBofObject {
private:
	static int nSDevMngrs;
	CBofList<CBagStorageDev *> m_xStorageDeviceList;

public:
	CBagStorageDevManager();
	~CBagStorageDevManager();

	ERROR_CODE RegisterStorageDev(CBagStorageDev *pSDev);
	ERROR_CODE UnRegisterStorageDev(CBagStorageDev *pSDev);
	ERROR_CODE ReleaseStorageDevices();

	INT GetObjectValue(const CBofString &sObject, const CBofString &sProperty);
	VOID SetObjectValue(const CBofString &sObject, const CBofString &sProperty, INT nValue);

	INT GetNumStorageDevices() {
		return m_xStorageDeviceList.GetCount();
	}

	CBagStorageDev *GetStorageDevice(INT nIndex) {
		return m_xStorageDeviceList[nIndex];
	}
	CBagStorageDev *GetStorageDeviceContaining(const CBofString &sName);
	CBagStorageDev *GetStorageDeviceContaining(CBagObject *pObj);
	CBagStorageDev *GetStorageDevice(const CBofString &sName);
	BOOL MoveObject(const CBofString &sDstName, const CBofString &sSrcName, const CBofString &sObjName);
	BOOL AddObject(const CBofString &sDstName, const CBofString &sObjName);
	BOOL RemoveObject(const CBofString &sSrcName, const CBofString &sObjName);

	VOID SaveObjList(ST_OBJ *pObjList, INT nNumEntries);
	VOID RestoreObjList(ST_OBJ *pObjList, INT nNumEntries);
};

extern BOOL g_bAllowPaint;
extern CBagStorageDevWnd *g_pLastWindow;

} // namespace Bagel

#endif
