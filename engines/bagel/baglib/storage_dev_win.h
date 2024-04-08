
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
	const char *m_lpszResourceName;
	uint32 m_nResId;

public:
	CResource() {
		m_lpszResourceName = nullptr;
		m_nResId = 0;
	}

	void SetResource(const char *lpszResourceName) {
		m_lpszResourceName = lpszResourceName;
	}
	void SetResource(uint32 nResId) {
		m_nResId = nResId;
	}
	const char *GetResourceName() {
		return m_lpszResourceName;
	}
	uint32 GetResourceId() {
		return m_nResId;
	}
};

// Filter function function prototype.
//
// for power pc, just pretend our filter function is upp
#if BOF_MAC && __POWERPC__
typedef UniversalProcPtr FilterFunction;
#else
typedef bool (*FilterFunction)(const uint16 nFilterId, CBofBitmap *, CBofRect *);
#endif

// Callbacks are a little tricky for powerpc...
#if BOF_MAC && __POWERPC__
enum {
	uppFilterProcInfo = kRegisterBased | REGISTER_ROUTINE_PARAMETER(1, kRegisterD0, SIZE_CODE(sizeof(uint16))) | REGISTER_ROUTINE_PARAMETER(2, kRegisterA0, SIZE_CODE(sizeof(CBofBitmap *))) | REGISTER_ROUTINE_PARAMETER(3, kRegisterA1, SIZE_CODE(sizeof(CBofRect *)))
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

	uint16 m_nExitOnEdge; // Non zero if SDev should be closed when an outside edge is clicked on, and and prev sdev is set, m_nExitOnEdge is the thickness of the edge

	uint16 m_nFilterId; // Filter id number 0 for normal view
	uint16 m_nFadeId;   // Fade id number 0 for no fade in
	uint16 m_nDiskID;

protected:
	uint16 m_xSDevType;                 // Type of storage device
	static CBofPoint m_xCursorLocation; // Current cursor location in bmp.
	static bool m_bHandledUpEvent;      // Hack to stop
	static bool m_bHidePDA;
	FilterFunction m_pBitmapFilter; // Pointer to the bitmap filter.

	bool m_bForiegnList : 1;   // True if SetObjectList has been called
	bool m_bCloseOnOpen : 1;   // True if other SDev should be closed when this is opened
	bool m_bContainsModal : 1; // True if SDev contains a modal object
	bool m_bCloseup : 1;       // true if is a closeup (includes CIC, or CHAT)
	bool m_bCIC : 1;           // true if is a CIC
	bool m_bCustom : 1;        // true if is a hand coded closeup
	bool m_bFirstPaint : 1;    // run object stuff

	static bool m_bPanPreFiltered;  // Let pda know if screens been prefiltered
	static bool m_bDirtyAllObjects; // Dirty all objects in prefilter?
	static bool m_bPreFilter;

	int m_nFloatPages; // The number of pages required to display all floating objects

public:
	CBagStorageDev();
	virtual ~CBagStorageDev();

	virtual ErrorCode PreFilter(CBofBitmap *pBmp, CBofRect *pRect, CBofList<CBagObject *> *pList = nullptr);

	/**
	 * Make all the objects in a list dirty
	 */
	void MakeListDirty(CBofList<CBagObject *> *pList);

	void SetDiskID(uint16 nDiskID) {
		m_nDiskID = nDiskID;
	}
	uint16 GetDiskID() {
		return m_nDiskID;
	}

	virtual ErrorCode SetLoadFilePos(const CBofPoint) {
		return ERR_NONE;
	}

	bool IsCloseup() {
		return m_bCloseup;
	}
	void SetCloseup(bool b = true) {
		m_bCloseup = b;
	}

	// Set true if sdef is "AS CIC"
	bool IsCIC();
	void SetCIC(bool b = true) {
		m_bCIC = b;
	}

	// Set to true if this is a hand coded closeup
	bool IsCustom() {
		return m_bCustom;
	}
	void SetCustom(bool b = true) {
		m_bCustom = b;
	}

	int GetDeviceType() {
		return m_xSDevType;
	}

	int GetObjectCount();
	CBagObject *GetObjectByPos(int nIndex);
	CBagObject *GetObject(int nRefId, bool bActiveOnly = false);
	CBagObject *GetObject(const CBofPoint &xPoint, bool bActiveOnly = false);
	CBagObject *GetObject(const CBofString &sName, bool bActiveOnly = false);
	CBagObject *GetObjectByType(const CBofString &sName, bool bActiveOnly = false);
	CBofPoint &GetLastCursorLocation() {
		return m_xCursorLocation;
	}
	CBofList<CBagObject *> *
	GetObjectList() {
		return m_pObjectList;
	}
	void SetObjectList(CBofList<CBagObject *> *pList, CBofList<CBagExpression *> *pEList = nullptr);

	bool Contains(CBagObject *pObj, bool bActive = true);

	// virtual CBofRect GetLocation()                       { return CBofRect(); }
	CBofRect GetRect() {
		return m_cDestRect;
	}
	void SetRect(const CBofRect &xRect) {
		m_cDestRect = xRect;
	}
	CBofPoint GetPosition() {
		return m_cDestRect.TopLeft();
	}
	void SetPosition(const CBofPoint &pos);

	virtual ErrorCode AddObject(CBagObject *pObj, int nPos = 0); // Add a new object
	virtual ErrorCode ActivateLocalObject(CBagObject *pObj);
	virtual ErrorCode ActivateLocalObject(const CBofString &sName);
	virtual ErrorCode AttachActiveObjects();

	virtual ErrorCode RemoveObject(CBagObject *pObj);
	virtual ErrorCode DeactivateLocalObject(CBagObject *pObj);
	virtual ErrorCode DeactivateLocalObject(const CBofString &sName);
	virtual ErrorCode DetachActiveObjects();

	virtual void SetHelpFilename(const CBofString &) {}

	const CBofString &GetName() {
		return m_sName;
	}
	void SetName(const CBofString &str) {
		m_sName = str;
	}

	void SetFilterId(uint16 nId) {
		m_nFilterId = nId;
	}
	uint16 GetFilterId() {
		return m_nFilterId;
	}

	void SetFadeId(uint16 nId) {
		m_nFadeId = nId;
	}
	uint16 GetFadeId() {
		return m_nFadeId;
	}

	const CBofString &GetPrevSDev() {
		return m_sPrevSDev;
	}
	const CBofPoint GetPrevLoc() {
		return m_xPrevLocation;
	}
	void SetPrevSDev(const CBofString &str) {
		m_sPrevSDev = str;
	}

	bool GetCloseOnOpen() {
		return m_bCloseOnOpen;
	}
	void SetCloseOnOpen(bool bVal) {
		m_bCloseOnOpen = bVal;
	}

	bool GetContainsModal() {
		return m_bContainsModal;
	}
	void SetContainsModal(bool bVal) {
		m_bContainsModal = bVal;
	}

	uint16 GetExitOnEdge() {
		return m_nExitOnEdge;
	}
	void SetExitOnEdge(uint16 nVal) {
		m_nExitOnEdge = nVal;
	}

	CBagObject *GetLActiveObject() {
		return m_pLActiveObject;
	}
	// CBagObject*      GetRActiveObject()                  { return m_pRActiveObject; }
	ErrorCode SetLActiveObject(CBagObject *pObj) {
		m_pLActiveObject = pObj;
		return ERR_NONE;
	}
	// ErrorCode           SetRActiveObject(CBagObject* pObj)  { m_pRActiveObject = pObj; return 0;}

	virtual ErrorCode OnLActiveObject(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);
	// virtual ErrorCode   OnRActiveObject(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);

	MOUSE_ACTIVITY GetLActivity() {
		return m_nCurrMouseActivity;
	}
	void SetLActivity(MOUSE_ACTIVITY ma) {
		m_nCurrMouseActivity = ma;
	}

	ErrorCode PaintStorageDevice(CBofWindow *pWnd, CBofBitmap *pBmp = nullptr, CBofRect * = nullptr);

	virtual void HandleError(ErrorCode errCode);

	// The associated window describes which window to get screen information from and
	// where to paint objects and most importantly what info to send to the callbacks
	virtual void SetAssociateWnd(CBofWindow *pWnd) {
		m_pAssociateWnd = pWnd;
	}
	virtual CBofWindow *GetAssociateWnd() {
		return m_pAssociateWnd;
	}

	virtual PARSE_CODES SetInfo(bof_ifstream &fpInput); // This function call the pure virt set background

	virtual ErrorCode SetBackground(CBofBitmap *pBmp) = 0; // This could be eliminated but is kept in to insure good class usage
	virtual CBofBitmap *GetBackground() = 0;                // Think about it, you can figure this out
	const CBofString &GetBackgroundName() {
		return m_sBackgroundName;
	}

	virtual ErrorCode Attach(); // This function attachs the background and nessasary bitmaps
	virtual ErrorCode Detach(); // This function attachs the background and nessasary bitmaps

	virtual ErrorCode Close();

	virtual ErrorCode LoadObjects();
	virtual ErrorCode ReleaseObjects();
	// virtual ErrorCode   InsertObjects(CBagObject *);
	virtual ErrorCode NoObjectsUnderMouse();

	virtual const CBofPoint DevPtToViewPort(const CBofPoint &xPoint) {
		return xPoint;
	}
	virtual const CBofPoint ViewPortToDevPt(const CBofPoint &xPoint) {
		return xPoint;
	}

	// virtual ErrorCode   SaveFile(ostream& fpOutput);
	virtual ErrorCode LoadFile(const CBofString &sFile);
	virtual ErrorCode LoadFileFromStream(bof_ifstream &fpInput, const CBofString &sWldName, bool bAttach = true);

	virtual void OnMouseMove(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);
	virtual ErrorCode OnMouseOver(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);
	virtual void OnLButtonDown(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);
	virtual void OnLButtonUp(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);

	virtual ErrorCode OnCursorUpdate(int /*nCurrObj*/) {
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
	void SetNumFloatPages(int nFloatPages) {
		m_nFloatPages = nFloatPages;
	}

	// Correctly set the filter function for the storage device.
	//
	void OnSetFilter(bool (*FilterFxn)(const uint16 nFilterId, CBofBitmap *, CBofRect *));

	// Get a pointer to the filter function
	//
	FilterFunction GetFilter();

	// Predicate to test if this storage device is filtered.
	//
	bool IsFiltered() {
		return m_nFilterId != 0;
	}

	static CBofBitmap *m_pUnderCursorBmp; // For preserving under bitmap.
	static bool m_bDrawCursorBackdrop;    // Draw cursor backdrop

	// Provide a method to get at the above vars
	static void SetDrawCursorBackdrop(bool b) {
		m_bDrawCursorBackdrop = b;
	}
	static bool DrawCursorBackdrop() {
		return m_bDrawCursorBackdrop;
	}

	// Provide a method to get at the above vars
	static void SetDirtyAllObjects(bool b) {
		m_bDirtyAllObjects = b;
	}
	static bool GetDirtyAllObjects() {
		return m_bDirtyAllObjects;
	}

	// Provide a method to let PDA know that it should update everything
	static void SetPreFiltered(bool b = true) {
		m_bPanPreFiltered = b;
	}
	static bool GetPreFiltered() {
		return m_bPanPreFiltered;
	}

	// We won't always call the prefilter, just when explicitly instructed to.
	static bool PreFilterPan() {
		return m_bPreFilter;
	}
	static void SetPreFilterPan(bool b = true) {
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
	bool m_bMadeSelection;
	bool m_bOnUpdate;
	CBofBitmap *m_pWorkBmp;
	CBofString m_sHelpFileName; // Name of the help file for this device

public:
	static CBagEventSDev *m_pEvtSDev; // Pointer to the Event Storage Device

	CBagStorageDevWnd();
	virtual ~CBagStorageDevWnd();
	static void initialize() {
		m_pEvtSDev = nullptr;
	}

	virtual ErrorCode Attach(); // This function attachs the background and nessasary bitmaps
	virtual ErrorCode Detach(); // This function attachs the background and nessasary bitmaps

	virtual ErrorCode Close();
	virtual ErrorCode RunModal(CBagObject *pObj);
	virtual void OnTimer(uint32 nTimerId);

	void SetOnUpdate(bool bVal = true) {
		m_bOnUpdate = bVal;
	}
	bool GetOnUpdate() {
		return m_bOnUpdate;
	}

	virtual ErrorCode PaintScreen(CBofRect *pRect = nullptr, bool bPaintCursor = true);
	ErrorCode PaintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp,
	                        CBofRect &viewOffsetRect, CBofList<CBofRect> * = nullptr, bool tempVar = true);
	ErrorCode PaintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp) {
		CBofRect emptyRect;
		return PaintObjects(list, pBmp, emptyRect);
	}

	ErrorCode PaintWithCursor(CBofBitmap *pBmp, CBofRect *pRect = nullptr);

	virtual CBofRect GetLocation() {
		return GetWindowRect();
	}

	virtual ErrorCode SetBackground(CBofBitmap *pBmp);
	virtual CBofBitmap *GetBackground() {
		return GetBackdrop();
	}
	virtual CBofBitmap *GetWorkBmp() {
		return m_pWorkBmp;
	}

	virtual ErrorCode LoadFile(const CBofString &sWldFile);

	virtual const CBofString &GetHelpFilename() {
		return m_sHelpFileName;
	}
	virtual void SetHelpFilename(const CBofString &s) {
		m_sHelpFileName = s;
	}

	virtual ErrorCode OnRender(CBofBitmap *pBmp, CBofRect *pRect = nullptr);
	virtual void OnPaint(CBofRect *);
	virtual void OnMainLoop();
	void OnClose();
	void OnMouseMove(uint32 nFlags, CBofPoint *, void * = nullptr);
	void OnLButtonDown(uint32 nFlags, CBofPoint *point, void * = nullptr);
	void OnLButtonUp(uint32 nFlags, CBofPoint *point, void * = nullptr);

	void OnKeyHit(uint32 lKey, uint32 nRepCount);

protected:
	virtual ErrorCode SetWorkBmp();
	virtual ErrorCode KillWorkBmp();
};

/**
 * CBagStorageDevDlg is a window that contains a slide bitmap object.
 * It has specialize functions for handling slide bitmaps and slide objects.
 */
class CBagStorageDevDlg : public CBofDialog, public CBagStorageDev {
private:
	bool m_bMadeSelection;
	CBofString m_sHelpFileName; // Name of the help file for this device

public:
	CBagStorageDevDlg();

	virtual ErrorCode PaintScreen(CBofRect *pRect = nullptr, bool bPaintCursor = true);
	ErrorCode PaintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp,
	                        CBofRect &viewOffsetRect, CBofList<CBofRect> * = nullptr, bool tempVar = true);
	ErrorCode PaintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp) {
		CBofRect emptyRect;
		return PaintObjects(list, pBmp, emptyRect);
	}

	virtual CBofRect GetLocation() {
		return GetWindowRect();
	}

	virtual ErrorCode SetBackground(CBofBitmap *pBmp) {
		if (pBmp)
			return SetBackdrop(pBmp);
		else
			KillBackdrop();
		return ERR_NONE;
	}
	virtual CBofBitmap *GetBackground() {
		return GetBackdrop();
	}

	virtual ErrorCode LoadFile(const CBofString &sWldFile);

	ErrorCode Create(const char *pszName, int x = 0, int y = 0, int nWidth = USE_DEFAULT, int nHeight = USE_DEFAULT, CBofWindow *pParent = nullptr, uint32 nControlID = 0);
	ErrorCode Create(const char *pszName, CBofRect *pRect = nullptr, CBofWindow *pParent = nullptr, uint32 nControlID = 0);

	virtual ErrorCode Attach(); // This function attachs the background and nessasary bitmaps

	virtual ErrorCode Close();

	virtual const CBofString &GetHelpFilename() {
		return m_sHelpFileName;
	}
	virtual void SetHelpFilename(const CBofString &s) {
		m_sHelpFileName = s;
	}

	virtual void OnMainLoop();
	virtual ErrorCode OnRender(CBofBitmap *pBmp, CBofRect *pRect = nullptr);

	virtual void OnPaint(CBofRect *);
	virtual void OnClose();
	void OnMouseMove(uint32 nFlags, CBofPoint *, void * = nullptr);
	void OnLButtonDown(uint32 nFlags, CBofPoint *point, void * = nullptr);
	void OnLButtonUp(uint32 nFlags, CBofPoint *point, void * = nullptr);
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

	ErrorCode RegisterStorageDev(CBagStorageDev *pSDev);
	ErrorCode UnRegisterStorageDev(CBagStorageDev *pSDev);
	ErrorCode ReleaseStorageDevices();

	int GetObjectValue(const CBofString &sObject, const CBofString &sProperty);
	void SetObjectValue(const CBofString &sObject, const CBofString &sProperty, int nValue);

	int GetNumStorageDevices() {
		return m_xStorageDeviceList.GetCount();
	}

	CBagStorageDev *GetStorageDevice(int nIndex) {
		return m_xStorageDeviceList[nIndex];
	}
	CBagStorageDev *GetStorageDeviceContaining(const CBofString &sName);
	CBagStorageDev *GetStorageDeviceContaining(CBagObject *pObj);
	CBagStorageDev *GetStorageDevice(const CBofString &sName);
	bool MoveObject(const CBofString &sDstName, const CBofString &sSrcName, const CBofString &sObjName);
	bool AddObject(const CBofString &sDstName, const CBofString &sObjName);
	bool RemoveObject(const CBofString &sSrcName, const CBofString &sObjName);

	void SaveObjList(ST_OBJ *pObjList, int nNumEntries);
	void RestoreObjList(ST_OBJ *pObjList, int nNumEntries);
};

extern bool g_bAllowPaint;
extern CBagStorageDevWnd *g_pLastWindow;

} // namespace Bagel

#endif
