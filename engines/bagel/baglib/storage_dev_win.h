
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
	const char *_lpszResourceName;
	uint32 _nResId;

public:
	CResource() {
		_lpszResourceName = nullptr;
		_nResId = 0;
	}

	void setResource(const char *lpszResourceName) {
		_lpszResourceName = lpszResourceName;
	}
	void setResource(uint32 nResId) {
		_nResId = nResId;
	}
	const char *getResourceName() {
		return _lpszResourceName;
	}
	uint32 getResourceId() {
		return _nResId;
	}
};

// Filter function function prototype.
typedef bool (*FilterFunction)(const uint16 nFilterId, CBofBitmap *, CBofRect *);

/**
 * CBagPanWindow is a window that contains a slide bitmap object.  It has specialize
 *  functions for handling slide bitmaps and slide objects.
 */
class CBagStorageDev : public CBagParseObject {
public:
	enum MouseActivity { kMouseNONE = 0x0000, kMouseDRAGGING = 0x0001 };

private:
	CBofString _sName;           // Name of this storage device
	CBofString _sBackgroundName; // Name of this storage devices background
	CBofString _sPrevSDev;       // Name of the previous storage device

	CBofRect _cDestRect;      // Position of storage device
	CBofPoint _xPrevLocation; // Previous view location in bmp.

	CBagObject *_pLActiveObject; // The last object selected on mouse down
	// CBagObject          *_pRActiveObject;       // The last object selected on mouse down

	CBofList<CBagObject *> *_pObjectList;         // Objects to be painted to the pan
	CBofList<CBagExpression *> *_pExpressionList; // Condition for painting to the pan

	CBofWindow *_pAssociateWnd; // Sounds need a window

	MouseActivity _nCurrMouseActivity; // What is up with the mouse currently

	uint16 _nExitOnEdge; // Non zero if SDev should be closed when an outside edge is clicked on, and and prev sdev is set, _nExitOnEdge is the thickness of the edge

	uint16 _nFilterId; // Filter id number 0 for normal view
	uint16 _nFadeId;   // Fade id number 0 for no fade in
	uint16 _nDiskID;

protected:
	uint16 _xSDevType;                 // Type of storage device
	static CBofPoint *_xCursorLocation; // Current cursor location in bmp.
	static CBofRect *gRepaintRect;
	static bool _bHandledUpEvent;      // Hack to stop
	static bool _bHidePDA;
	FilterFunction _pBitmapFilter; // Pointer to the bitmap filter.

	bool _bForiegnList : 1;   // True if setObjectList has been called
	bool _bCloseOnOpen : 1;   // True if other SDev should be closed when this is opened
	bool _bContainsModal : 1; // True if SDev contains a modal object
	bool _bCloseup : 1;       // true if is a closeup (includes CIC, or CHAT)
	bool _bCIC : 1;           // true if is a CIC
	bool _bCustom : 1;        // true if is a hand coded closeup
	bool _bFirstPaint : 1;    // run object stuff

	static bool _bPanPreFiltered;  // Let pda know if screens been prefiltered
	static bool _bDirtyAllObjects; // Dirty all objects in prefilter?
	static bool _bPreFilter;

	int _nFloatPages; // The number of pages required to display all floating objects

public:
	CBagStorageDev();
	virtual ~CBagStorageDev();

	virtual ErrorCode preFilter(CBofBitmap *pBmp, CBofRect *pRect, CBofList<CBagObject *> *pList = nullptr);

	static void initialize();
	static void shutdown();

	/**
	 * Make all the objects in a list dirty
	 */
	void makeListDirty(CBofList<CBagObject *> *pList);

	void setDiskID(uint16 nDiskID) {
		_nDiskID = nDiskID;
	}
	uint16 getDiskID() {
		return _nDiskID;
	}

	virtual ErrorCode setloadFilePos(const CBofPoint) {
		return ERR_NONE;
	}

	bool isCloseup() {
		return _bCloseup;
	}
	void setCloseup(bool b = true) {
		_bCloseup = b;
	}

	// Set true if sdef is "AS CIC"
	bool isCIC();
	void setCIC(bool b = true) {
		_bCIC = b;
	}

	// Set to true if this is a hand coded closeup
	bool isCustom() {
		return _bCustom;
	}
	void setCustom(bool b = true) {
		_bCustom = b;
	}

	int getDeviceType() {
		return _xSDevType;
	}

	int getObjectCount();
	CBagObject *getObjectByPos(int nIndex);
	CBagObject *getObject(int nRefId, bool bActiveOnly = false);
	CBagObject *getObject(const CBofPoint &xPoint, bool bActiveOnly = false);
	CBagObject *getObject(const CBofString &sName, bool bActiveOnly = false);
	CBagObject *getObjectByType(const CBofString &sName, bool bActiveOnly = false);
	CBofPoint &getLastCursorLocation() {
		return *_xCursorLocation;
	}
	CBofList<CBagObject *> *getObjectList() {
		return _pObjectList;
	}
	void setObjectList(CBofList<CBagObject *> *pList, CBofList<CBagExpression *> *pEList = nullptr);

	bool contains(CBagObject *pObj, bool bActive = true);

	// virtual CBofRect GetLocation()                       { return CBofRect(); }
	CBofRect getRect() {
		return _cDestRect;
	}
	void setRect(const CBofRect &xRect) {
		_cDestRect = xRect;
	}
	CBofPoint getPosition() {
		return _cDestRect.topLeft();
	}
	void setPosition(const CBofPoint &pos);

	virtual ErrorCode addObject(CBagObject *pObj, int nPos = 0); // Add a new object
	virtual ErrorCode activateLocalObject(CBagObject *pObj);
	virtual ErrorCode activateLocalObject(const CBofString &sName);
	virtual ErrorCode attachActiveObjects();

	virtual ErrorCode removeObject(CBagObject *pObj);
	virtual ErrorCode deactivateLocalObject(CBagObject *pObj);
	virtual ErrorCode deactivateLocalObject(const CBofString &sName);
	virtual ErrorCode detachActiveObjects();

	virtual void setHelpFilename(const CBofString &) {}

	const CBofString &getName() {
		return _sName;
	}
	void setName(const CBofString &str) {
		_sName = str;
	}

	void setFilterId(uint16 nId) {
		_nFilterId = nId;
	}
	uint16 getFilterId() {
		return _nFilterId;
	}

	void setFadeId(uint16 nId) {
		_nFadeId = nId;
	}
	uint16 getFadeId() {
		return _nFadeId;
	}

	const CBofString &getPrevSDev() {
		return _sPrevSDev;
	}
	const CBofPoint getPrevLoc() {
		return _xPrevLocation;
	}
	void setPrevSDev(const CBofString &str) {
		_sPrevSDev = str;
	}

	bool getCloseOnOpen() {
		return _bCloseOnOpen;
	}
	void setCloseOnOpen(bool bVal) {
		_bCloseOnOpen = bVal;
	}

	bool getContainsModal() {
		return _bContainsModal;
	}
	void setContainsModal(bool bVal) {
		_bContainsModal = bVal;
	}

	uint16 getExitOnEdge() {
		return _nExitOnEdge;
	}
	void setExitOnEdge(uint16 nVal) {
		_nExitOnEdge = nVal;
	}

	CBagObject *getLActiveObject() {
		return _pLActiveObject;
	}
	// CBagObject*      GetRActiveObject()                  { return _pRActiveObject; }
	ErrorCode setLActiveObject(CBagObject *pObj) {
		_pLActiveObject = pObj;
		return ERR_NONE;
	}

	virtual ErrorCode onLActiveObject(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);

	MouseActivity getLActivity() {
		return _nCurrMouseActivity;
	}
	void setLActivity(MouseActivity ma) {
		_nCurrMouseActivity = ma;
	}

	ErrorCode paintStorageDevice(CBofWindow *pWnd, CBofBitmap *pBmp = nullptr, CBofRect * = nullptr);

	virtual void handleError(ErrorCode errCode);

	// The associated window describes which window to get screen information from and
	// where to paint objects and most importantly what info to send to the callbacks
	virtual void setAssociateWnd(CBofWindow *pWnd) {
		_pAssociateWnd = pWnd;
	}
	virtual CBofWindow *getAssociateWnd() {
		return _pAssociateWnd;
	}

	virtual ParseCodes setInfo(CBagIfstream &fpInput); // This function call the pure virt set background

	virtual ErrorCode setBackground(CBofBitmap *pBmp) = 0; // This could be eliminated but is kept in to insure good class usage
	virtual CBofBitmap *getBackground() = 0;                // Think about it, you can figure this out
	const CBofString &getBackgroundName() {
		return _sBackgroundName;
	}

	virtual ErrorCode attach(); // This function attaches the background and necessary bitmaps
	virtual ErrorCode detach(); // This function attaches the background and necessary bitmaps

	virtual ErrorCode close();

	virtual ErrorCode loadObjects();
	virtual ErrorCode releaseObjects();
	// virtual ErrorCode   InsertObjects(CBagObject *);
	virtual ErrorCode noObjectsUnderMouse();

	virtual const CBofPoint devPtToViewPort(const CBofPoint &xPoint) {
		return xPoint;
	}
	virtual const CBofPoint viewPortToDevPt(const CBofPoint &xPoint) {
		return xPoint;
	}

	virtual ErrorCode loadFile(const CBofString &sFile);
	virtual ErrorCode loadFileFromStream(CBagIfstream &fpInput, const CBofString &sWldName, bool bAttach = true);

	virtual void onMouseMove(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);
	virtual ErrorCode onMouseOver(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);
	virtual void onLButtonDown(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);
	virtual void onLButtonUp(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);

	virtual ErrorCode onCursorUpdate(int /*nCurrObj*/) {
		return ERR_NONE;
	}
	virtual CBagObject *onNewSpriteObject(const CBofString &sInit);
	virtual CBagObject *onNewBitmapObject(const CBofString &sInit);
	virtual CBagObject *onNewTextObject(const CBofString &sInit);
	virtual CBagObject *onNewLinkObject(const CBofString &sInit);
	virtual CBagObject *onNewRPObject(const CBofString &sInit);
	virtual CBagObject *onNewEDObject(const CBofString &sInit);
	virtual CBagObject *onNewDosObject(const CBofString &sInit);
	virtual CBagObject *onNewSoundObject(const CBofString &sInit);
	virtual CBagObject *onNewButtonObject(const CBofString &sInit);
	virtual CBagObject *onNewCharacterObject(const CBofString &sInit);
	virtual CBagObject *onNewMovieObject(const CBofString &);
	virtual CBagObject *onNewCommandObject(const CBofString &sInit);
	virtual CBagObject *onNewAreaObject(const CBofString &sInit);
	virtual CBagObject *onNewExpressionObject(const CBofString &sInit);
	virtual CBagObject *onNewVariableObject(const CBofString &sInit);
	virtual CBagObject *onNewUserObject(const CBofString &sInit);
	virtual CBagObject *onNewThingObject(const CBofString &sInit);

	// Call to arrange floating object, override to customize layout
	virtual CBofPoint arrangeFloater(CBofPoint nPos, CBagObject *pObj);

	// Set and Get the number of pages required to display all floating objects
	int getNumFloatPages() {
		return _nFloatPages;
	}
	void setNumFloatPages(int nFloatPages) {
		_nFloatPages = nFloatPages;
	}

	// Correctly set the filter function for the storage device.
	//
	void onSetFilter(bool (*FilterFxn)(const uint16 nFilterId, CBofBitmap *, CBofRect *));

	// Get a pointer to the filter function
	//
	FilterFunction getFilter();

	// Predicate to test if this storage device is filtered.
	//
	bool isFiltered() {
		return _nFilterId != 0;
	}

	// Provide a method to get at the above vars
	static void setDirtyAllObjects(bool b) {
		_bDirtyAllObjects = b;
	}
	static bool getDirtyAllObjects() {
		return _bDirtyAllObjects;
	}

	// Provide a method to let PDA know that it should update everything
	static void setPreFiltered(bool b = true) {
		_bPanPreFiltered = b;
	}
	static bool getPreFiltered() {
		return _bPanPreFiltered;
	}

	// We won't always call the prefilter, just when explicitly instructed to.
	static bool preFilterPan() {
		return _bPreFilter;
	}
	static void setPreFilterPan(bool b = true) {
		_bPreFilter = b;
		_bDirtyAllObjects = b;
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
	//bool _bMadeSelection;
	bool _bOnUpdate;
	CBofBitmap *_pWorkBmp;
	CBofString _sHelpFileName; // Name of the help file for this device

public:
	static CBagEventSDev *_pEvtSDev; // Pointer to the Event Storage Device

	CBagStorageDevWnd();
	virtual ~CBagStorageDevWnd();
	static void initialize() {
		_pEvtSDev = nullptr;
	}

	ErrorCode attach() override; // This function attaches the background and necessary bitmaps
	ErrorCode detach() override; // This function attaches the background and necessary bitmaps

	ErrorCode close() override;
	virtual ErrorCode runModal(CBagObject *pObj);
	void onTimer(uint32 nTimerId) override;

	void setOnUpdate(bool bVal = true) {
		_bOnUpdate = bVal;
	}
	bool getOnUpdate() {
		return _bOnUpdate;
	}

	virtual ErrorCode paintScreen(CBofRect *pRect = nullptr);
	ErrorCode paintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp,
	                        CBofRect &viewOffsetRect, CBofList<CBofRect> * = nullptr, bool tempVar = true);
	ErrorCode paintWithCursor(CBofBitmap *pBmp, CBofRect *pRect = nullptr);

	virtual CBofRect GetLocation() {
		return getWindowRect();
	}

	ErrorCode setBackground(CBofBitmap *pBmp) override;

	CBofBitmap *getBackground() override {
		return getBackdrop();
	}
	virtual CBofBitmap *getWorkBmp() {
		return _pWorkBmp;
	}

	ErrorCode loadFile(const CBofString &sWldFile) override;

	virtual const CBofString &GetHelpFilename() {
		return _sHelpFileName;
	}

	void setHelpFilename(const CBofString &s) override {
		_sHelpFileName = s;
	}

	virtual ErrorCode onRender(CBofBitmap *pBmp, CBofRect *pRect = nullptr);
	void onPaint(CBofRect *) override;
	void onMainLoop() override;
	void onClose() override;
	void onMouseMove(uint32 nFlags, CBofPoint *, void * = nullptr) override;
	void onLButtonDown(uint32 nFlags, CBofPoint *point, void * = nullptr) override;
	void onLButtonUp(uint32 nFlags, CBofPoint *point, void * = nullptr) override;

	void onKeyHit(uint32 lKey, uint32 nRepCount) override;

protected:
	virtual ErrorCode setWorkBmp();
	virtual ErrorCode killWorkBmp();
};

/**
 * CBagStorageDevDlg is a window that contains a slide bitmap object.
 * It has specialize functions for handling slide bitmaps and slide objects.
 */
class CBagStorageDevDlg : public CBofDialog, public CBagStorageDev {
private:
	//bool _bMadeSelection;
	CBofString _sHelpFileName; // Name of the help file for this device

public:
	CBagStorageDevDlg();

	virtual ErrorCode paintScreen(CBofRect *pRect = nullptr);
	ErrorCode paintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp,
	                        CBofRect &viewOffsetRect, CBofList<CBofRect> * = nullptr, bool tempVar = true);
	ErrorCode paintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp) {
		CBofRect emptyRect;
		return paintObjects(list, pBmp, emptyRect);
	}

	virtual CBofRect GetLocation() {
		return getWindowRect();
	}

	virtual ErrorCode setBackground(CBofBitmap *pBmp) override {
		if (pBmp)
			return setBackdrop(pBmp);

		killBackdrop();
		return ERR_NONE;
	}
	virtual CBofBitmap *getBackground() override {
		return getBackdrop();
	}

	virtual ErrorCode loadFile(const CBofString &sWldFile) override;

	ErrorCode create(const char *pszName, CBofRect *pRect = nullptr, CBofWindow *pParent = nullptr, uint32 nControlID = 0);

	ErrorCode attach() override; // This function attaches the background and necessary bitmaps

	ErrorCode close() override;

	virtual const CBofString &GetHelpFilename() {
		return _sHelpFileName;
	}

	void setHelpFilename(const CBofString &s) override {
		_sHelpFileName = s;
	}

	void onMainLoop() override;
	virtual ErrorCode onRender(CBofBitmap *pBmp, CBofRect *pRect = nullptr);

	void onPaint(CBofRect *) override;
	void onClose() override;
	void onMouseMove(uint32 nFlags, CBofPoint *, void * = nullptr) override;
	void onLButtonDown(uint32 nFlags, CBofPoint *point, void * = nullptr) override;
	void onLButtonUp(uint32 nFlags, CBofPoint *point, void * = nullptr) override;
};

/**
 * Tracks all the storage devices
 */
class CBagStorageDevManager : public CBofObject {
private:
	static int nSDevMngrs;
	CBofList<CBagStorageDev *> _xStorageDeviceList;

public:
	CBagStorageDevManager();
	~CBagStorageDevManager();

	ErrorCode registerStorageDev(CBagStorageDev *pSDev);
	ErrorCode unregisterStorageDev(CBagStorageDev *pSDev);
	ErrorCode releaseStorageDevices();

	int getObjectValue(const CBofString &sObject, const CBofString &sProperty);
	void SetObjectValue(const CBofString &sObject, const CBofString &sProperty, int nValue);

	int getNumStorageDevices() {
		return _xStorageDeviceList.getCount();
	}

	CBagStorageDev *getStorageDevice(int nIndex) {
		return _xStorageDeviceList[nIndex];
	}
	CBagStorageDev *getStorageDeviceContaining(const CBofString &sName);
	CBagStorageDev *getStorageDeviceContaining(CBagObject *pObj);
	CBagStorageDev *getStorageDevice(const CBofString &sName);
	bool moveObject(const CBofString &sDstName, const CBofString &sSrcName, const CBofString &sObjName);
	bool addObject(const CBofString &sDstName, const CBofString &sObjName);
	bool removeObject(const CBofString &sSrcName, const CBofString &sObjName);

	void saveObjList(StObj *pObjList, int nNumEntries);
	void restoreObjList(StObj *pObjList, int nNumEntries);
};

extern bool g_allowPaintFl;
extern CBagStorageDevWnd *g_lastWindow;

} // namespace Bagel

#endif
