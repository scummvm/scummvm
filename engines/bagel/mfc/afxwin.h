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

#ifndef BAGEL_MFC_AFXWIN_H
#define BAGEL_MFC_AFXWIN_H

#include "bagel/mfc/minwindef.h"
#include "bagel/mfc/afx.h"
#include "bagel/mfc/atltypes.h"

namespace Bagel {
namespace MFC {

#define RECOVERY_DEFAULT_PING_INTERVAL 1
#define HELP_CONTEXT 1
#define AFX_DEPRECATED(MSG)
#define _AFX_FUNCNAME(_Name) _Name##A

typedef void *LPENUMOLEVERB;
typedef void *LPMSG;
typedef void *LPTYPEINFO;
typedef void *LPTYPELIB;
typedef void *LPSECURITY_ATTRIBUTES;
typedef void *LPCOLORADJUSTMENT;
typedef void *COLORADJUSTMENT;
typedef void *LPDRAWTEXTPARAMS;
typedef void *LPABC;
typedef void *LPKERNINGPAIR;
typedef void *LPOUTLINETEXTMETRIC;
typedef void *LPGLYPHMETRICS;
struct ABCFLOAT {};
typedef ABCFLOAT *LPABCFLOAT;
typedef void *LPDOCINFO;
struct CHwndRenderTarget {};
struct WINDOWINFO {};
typedef WINDOWINFO *PWINDOWINFO;
struct TITLEBARINFO {};
typedef TITLEBARINFO *PTITLEBARINFO;
struct CCreateContext {};
struct CControlCreationInfo {};
struct CREATESTRUCT {};
typedef CREATESTRUCT *LPCREATESTRUCT;
struct SCROLLINFO {};
typedef SCROLLINFO *LPSCROLLINFO;
struct SCROLLBARINFO {};
typedef SCROLLBARINFO *PSCROLLBARINFO;
typedef SCROLLBARINFO *LPSCROLLBARINFO;
struct COPYDATASTRUCT {};
struct MINMAXINFO {};
struct HELPINFO {};
struct WINDOWPOS {};
struct NCCALCSIZE_PARAMS {};
struct COMPAREITEMSTRUCT {};
typedef COMPAREITEMSTRUCT *LPCOMPAREITEMSTRUCT;
struct DELETEITEMSTRUCT {};
typedef DELETEITEMSTRUCT *LPDELETEITEMSTRUCT;
struct DRAWITEMSTRUCT {};
typedef DRAWITEMSTRUCT *LPDRAWITEMSTRUCT;
struct MEASUREITEMSTRUCT {};
typedef MEASUREITEMSTRUCT *LPMEASUREITEMSTRUCT;
struct MENUGETOBJECTINFO {};
struct MDINEXTMENU {};
typedef MDINEXTMENU *LPMDINEXTMENU;
struct STYLESTRUCT {};
typedef STYLESTRUCT *LPSTYLESTRUCT;
struct CDataExchange {};
struct CGestureConfig {};
struct GESTUREINFO {};
typedef GESTUREINFO *PGESTUREINFO;
struct CMFCDynamicLayout {};
struct TOUCHINPUT {};
typedef TOUCHINPUT *PTOUCHINPUT;
struct CDLGTEMPLATE {};
typedef CDLGTEMPLATE *LPCDLGTEMPLATE;
struct CHandleMap {};
struct CMFCControlContainer {};
struct CDCRenderTarget {};
struct CRenderTarget {};

typedef void (*AFX_THREADPROC)();
typedef void (*DRAWSTATEPROC)();
typedef void (*BLENDFUNCTION)();

typedef void AFX_CMDHANDLERINFO;
typedef void AFX_EVENT;
typedef void MSG;
typedef void DEVMODE;
typedef void COleMessageFilter;
typedef void D2D1_FACTORY_TYPE;
typedef void XFORM;
typedef void TRIVERTEX;
typedef void MAT2;
typedef void PRINTDLG;
typedef void ITaskbarList;
typedef void ITaskbarList3;
typedef void WINDOWPLACEMENT;
typedef void TOOLINFO;
typedef void COleControlSiteOrWnd;
typedef void IAccessible;
typedef void IAccessibleProxy;

class CTypeLibCache {};
class CVariantBoolConverter {};
class CDocTemplate {};
class CPen {};
class CBrush {};
class CPalette {};
class CFont {};
class CBitmap {};
class CGdiObject {};
class CRgn {};
class CCommandLineInfo {};
class CRecentFileList {};
class CDataRecoveryHandler {};
class CPrintDialog {};
class CCmdUI {};

typedef long DISPID;
typedef unsigned short VARTYPE;
typedef long SCODE;

struct IUnknown;
typedef IUnknown *LPUNKNOWN;

struct IDispatch;
typedef IDispatch *LPDISPATCH;

struct IConnectionPoint;
typedef IConnectionPoint *LPCONNECTIONPOINT;

typedef struct _GUID GUID;
typedef GUID IID;
typedef GUID CLSID;

struct tagDISPPARAMS;
typedef tagDISPPARAMS DISPPARAMS;

struct tagVARIANT;
typedef tagVARIANT VARIANT;

typedef struct _GUID GUID;
typedef GUID IID;
typedef GUID CLSID;
#define REFCLSID const CLSID &

class CDocManager;
class CCmdTarget;
class CView;
class CWnd;
class CFrameWnd;
class CDialog;
class CListBox;
class CMenu;
class CScrollBar;


/*============================================================================*/
// Window message map handling

struct AFX_MSGMAP_ENTRY;       // declared below after CWnd

struct AFX_MSGMAP {
	const AFX_MSGMAP *(PASCAL *pfnGetBaseMap)();
	const AFX_MSGMAP_ENTRY *lpEntries;
};

#ifndef AFX_MSG_CALL
#define AFX_MSG_CALL
#endif
typedef void (AFX_MSG_CALL CCmdTarget:: *AFX_PMSG)(void);

enum AFX_DISPMAP_FLAGS {
	afxDispCustom = 0,
	afxDispStock = 1
};

struct AFX_MSGMAP_ENTRY {
	AFX_PMSG pfn;    // routine to call (or special value)
	UINT nMessage;   // windows message
	UINT nCode;      // control code or WM_NOTIFY code
	UINT nID;        // control ID (or 0 for windows messages)
	UINT nLastID;    // used for entries specifying a range of control id's
	UINT_PTR nSig;       // signature type (action) or pointer to message #
};


#define DECLARE_MESSAGE_MAP() \
protected: \
	static const AFX_MSGMAP* PASCAL GetThisMessageMap(); \
	const AFX_MSGMAP* GetMessageMap() const override; \

#define BEGIN_TEMPLATE_MESSAGE_MAP(theClass, type_name, baseClass)			\
	PTM_WARNING_DISABLE														\
	template < typename type_name >											\
	const AFX_MSGMAP* theClass< type_name >::GetMessageMap() const			\
		{ return GetThisMessageMap(); }										\
	template < typename type_name >											\
	const AFX_MSGMAP* PASCAL theClass< type_name >::GetThisMessageMap()		\
	{																		\
		typedef theClass< type_name > ThisClass;							\
		typedef baseClass TheBaseClass;										\
		__pragma(warning(push))												\
		__pragma(warning(disable: 4640)) /* message maps can only be called by single threaded message pump */ \
		static const AFX_MSGMAP_ENTRY _messageEntries[] =					\
		{

#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
	PTM_WARNING_DISABLE \
	const AFX_MSGMAP* theClass::GetMessageMap() const \
		{ return GetThisMessageMap(); } \
	const AFX_MSGMAP* PASCAL theClass::GetThisMessageMap() \
	{ \
		typedef theClass ThisClass;						   \
		typedef baseClass TheBaseClass;					   \
		__pragma(warning(push))							   \
		__pragma(warning(disable: 4640)) /* message maps can only be called by single threaded message pump */ \
		static const AFX_MSGMAP_ENTRY _messageEntries[] =  \
		{

#define END_MESSAGE_MAP() \
		{0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0 } \
	}; \
		__pragma(warning(pop))	\
		static const AFX_MSGMAP messageMap = \
		{ &TheBaseClass::GetThisMessageMap, &_messageEntries[0] }; \
		return &messageMap; \
	}								  \
	PTM_WARNING_RESTORE


/*============================================================================*/
// OLE dispatch map handling (more in AFXDISP.H)

struct AFX_DISPMAP_ENTRY;

struct AFX_DISPMAP
{
#ifdef _AFXDLL
	const AFX_DISPMAP *(PASCAL *pfnGetBaseMap)();
#else
	const AFX_DISPMAP *pBaseMap;
#endif
	const AFX_DISPMAP_ENTRY *lpEntries;
	UINT *lpEntryCount;
	DWORD *lpStockPropMask;
};

#define DECLARE_DISPATCH_MAP() \
private: \
	static const AFX_DISPMAP_ENTRY _dispatchEntries[]; \
	static UINT _dispatchEntryCount; \
	static DWORD _dwStockPropMask; \
protected: \
	static const AFX_DISPMAP dispatchMap; \
	virtual const AFX_DISPMAP *GetDispatchMap() const; \

#define DECLARE_OLECMD_MAP()
#define DECLARE_CONNECTION_MAP()
#define DECLARE_INTERFACE_MAP()
#define DECLARE_EVENTSINK_MAP()

/*============================================================================*/

enum AFX_HELP_TYPE {
	afxWinHelp = 0,
	afxHTMLHelp = 1
};

// Type modifier for message handlers
#ifndef afx_msg
#define afx_msg         // intentional placeholder
#endif

/*============================================================================*/

class AFX_NOVTABLE CCmdTarget : public CObject {
	DECLARE_DYNAMIC(CCmdTarget)
protected:

public:
	// Constructors
	CCmdTarget();

	// Attributes
	LPDISPATCH GetIDispatch(BOOL bAddRef);
	// retrieve IDispatch part of CCmdTarget
	static CCmdTarget *PASCAL FromIDispatch(LPDISPATCH lpDispatch);
	// map LPDISPATCH back to CCmdTarget* (inverse of GetIDispatch)
	BOOL IsResultExpected();
	// returns TRUE if automation function should return a value

// Operations
	void EnableAutomation();
	// call in constructor to wire up IDispatch
	void EnableConnections();
	// call in constructor to wire up IConnectionPointContainer

	void BeginWaitCursor();
	void EndWaitCursor();
	void RestoreWaitCursor();       // call after messagebox

	// dispatch OLE verbs through the message map
	BOOL EnumOleVerbs(LPENUMOLEVERB *ppenumOleVerb);
	BOOL DoOleVerb(LONG iVerb, LPMSG lpMsg, HWND hWndParent, LPCRECT lpRect);

	// Overridables
		// route and dispatch standard command message types
		//   (more sophisticated than OnCommand)
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void *pExtra,
		AFX_CMDHANDLERINFO *pHandlerInfo);

	// called when last OLE reference is released
	virtual void OnFinalRelease();

	// called before dispatching to an automation handler function
	virtual BOOL IsInvokeAllowed(DISPID dispid);

	// support for OLE type libraries
	void EnableTypeLib();
	HRESULT GetTypeInfoOfGuid(LCID lcid, const GUID &guid,
		LPTYPEINFO *ppTypeInfo);
	virtual BOOL GetDispatchIID(IID *pIID);
	virtual UINT GetTypeInfoCount();
	virtual CTypeLibCache *GetTypeLibCache();
	virtual HRESULT GetTypeLib(LCID lcid, LPTYPELIB *ppTypeLib);

	// Implementation
public:
	virtual ~CCmdTarget() = 0;
#ifdef _DEBUG
	void Dump(CDumpContext &dc) const override;
	void AssertValid() const override;
#endif

	void GetNotSupported();
	void SetNotSupported();

protected:
	friend class CView;

	CView *GetRoutingView();
	CFrameWnd *GetRoutingFrame();
	static CView *PASCAL GetRoutingView_();
	static CFrameWnd *PASCAL GetRoutingFrame_();

	static const AFX_MSGMAP *PASCAL GetThisMessageMap();
	virtual const AFX_MSGMAP *GetMessageMap() const;

	DECLARE_OLECMD_MAP()
//	friend class COleCmdUI;

	DECLARE_DISPATCH_MAP()
	DECLARE_CONNECTION_MAP()
	DECLARE_INTERFACE_MAP()
	DECLARE_EVENTSINK_MAP()

	// OLE interface map implementation
public:
	// data used when CCmdTarget is made OLE aware
	long m_dwRef;
	LPUNKNOWN m_pOuterUnknown;  // external controlling unknown if != NULL
	DWORD_PTR m_xInnerUnknown;  // place-holder for inner controlling unknown

public:
	// advanced operations
	void EnableAggregation();       // call to enable aggregation
	void ExternalDisconnect();      // forcibly disconnect
	LPUNKNOWN GetControllingUnknown();
	// get controlling IUnknown for aggregate creation

// these versions do not delegate to m_pOuterUnknown
	DWORD InternalQueryInterface(const void *, LPVOID *ppvObj);
	DWORD InternalAddRef();
	DWORD InternalRelease();
	// these versions delegate to m_pOuterUnknown
	DWORD ExternalQueryInterface(const void *, LPVOID *ppvObj);
	DWORD ExternalAddRef();
	DWORD ExternalRelease();

	// implementation helpers
	LPUNKNOWN GetInterface(const void *);
	LPUNKNOWN QueryAggregates(const void *);

	// advanced overrideables for implementation
	virtual BOOL OnCreateAggregates();
	virtual LPUNKNOWN GetInterfaceHook(const void *);

	// OLE automation implementation
protected:
	struct XDispatch
	{
		DWORD_PTR m_vtbl;   // place-holder for IDispatch vtable
	} m_xDispatch;
	BOOL m_bResultExpected;

	// member variable-based properties
	void GetStandardProp(const AFX_DISPMAP_ENTRY *pEntry,
		VARIANT *pvarResult, UINT *puArgErr);
	SCODE SetStandardProp(const AFX_DISPMAP_ENTRY *pEntry,
		DISPPARAMS *pDispParams, UINT *puArgErr);

	// DISPID to dispatch map lookup
	static UINT PASCAL GetEntryCount(const AFX_DISPMAP *pDispMap);
	const AFX_DISPMAP_ENTRY *PASCAL GetDispEntry(LONG memid);
	static LONG PASCAL MemberIDFromName(const AFX_DISPMAP *pDispMap, LPCTSTR lpszName);

	// helpers for member function calling implementation
	static UINT PASCAL GetStackSize(const BYTE *pbParams, VARTYPE vtResult);
	SCODE PushStackArgs(BYTE *pStack, const BYTE *pbParams,
		void *pResult, VARTYPE vtResult, DISPPARAMS *pDispParams,
		UINT *puArgErr, VARIANT *rgTempVars, CVariantBoolConverter *pTempStackArgs = NULL);
	SCODE CallMemberFunc(const AFX_DISPMAP_ENTRY *pEntry, WORD wFlags,
		VARIANT *pvarResult, DISPPARAMS *pDispParams, UINT *puArgErr);

	friend class COleDispatchImpl;

public:
	// OLE event sink implementation
	BOOL OnEvent(UINT idCtrl, AFX_EVENT *pEvent,
		AFX_CMDHANDLERINFO *pHandlerInfo);

protected:
//	const AFX_EVENTSINKMAP_ENTRY *PASCAL GetEventSinkEntry(UINT idCtrl AFX_EVENT *pEvent);

	// OLE connection implementation
	struct XConnPtContainer
	{
		DWORD_PTR m_vtbl;   // place-holder for IConnectionPointContainer vtable
	} m_xConnPtContainer;

	//AFX_MODULE_STATE *m_pModuleState;
	friend class CInnerUnknown;
	friend UINT APIENTRY _AfxThreadEntry(void *pParam);

	virtual BOOL GetExtraConnectionPoints(CPtrArray *pConnPoints);
	virtual LPCONNECTIONPOINT GetConnectionHook(const IID &iid);

	friend class COleConnPtContainer;
};

/*============================================================================*/

class AFX_NOVTABLE CView : public CWnd
{
	friend class CWinAppEx;

	DECLARE_DYNAMIC(CView)

	// Constructors
protected:
	CView();

	// Attributes
public:
	CDocument *GetDocument() const;

	// Operations
public:
	// for standard printing setup (override OnPreparePrinting)
	BOOL DoPreparePrinting(CPrintInfo *pInfo);

	// Overridables
public:
	virtual BOOL IsSelected(const CObject *pDocItem) const; // support for OLE

	// OLE scrolling support (used for drag/drop as well)
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);

	// OLE drag/drop support
	virtual DROPEFFECT OnDragEnter(COleDataObject *pDataObject,
		DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(COleDataObject *pDataObject,
		DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave();
	virtual BOOL OnDrop(COleDataObject *pDataObject,
		DROPEFFECT dropEffect, CPoint point);
	virtual DROPEFFECT OnDropEx(COleDataObject *pDataObject,
		DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);
	virtual DROPEFFECT OnDragScroll(DWORD dwKeyState, CPoint point);

	virtual void OnPrepareDC(CDC *pDC, CPrintInfo *pInfo = NULL);

	virtual void OnInitialUpdate(); // called first time after construct

protected:
	// Activation
	virtual void OnActivateView(BOOL bActivate, CView *pActivateView,
		CView *pDeactiveView);
	virtual void OnActivateFrame(UINT nState, CFrameWnd *pFrameWnd);

	// General drawing/updating
	virtual void OnUpdate(CView *pSender, LPARAM lHint, CObject *pHint);
	virtual void OnDraw(CDC *pDC) = 0;

	// Printing support
	virtual BOOL OnPreparePrinting(CPrintInfo *pInfo);
	// must override to enable printing and print preview

	virtual void OnBeginPrinting(CDC *pDC, CPrintInfo *pInfo);
	virtual void OnPrint(CDC *pDC, CPrintInfo *pInfo);
	virtual void OnEndPrinting(CDC *pDC, CPrintInfo *pInfo);

	// Advanced: end print preview mode, move to point
	virtual void OnEndPrintPreview(CDC *pDC, CPrintInfo *pInfo, POINT point,
		CPreviewView *pView);

	// Implementation
public:
	virtual ~CView() = 0;
#ifdef _DEBUG
	void Dump(CDumpContext &) const override;
	void AssertValid() const override;
#endif //_DEBUG

	// Advanced: for implementing custom print preview
	BOOL DoPrintPreview(UINT nIDResource, CView *pPrintView,
		CRuntimeClass *pPreviewViewClass, CPrintPreviewState *pState);

	void CalcWindowRect(LPRECT lpClientRect,
		UINT nAdjustType = adjustBorder) override;
	CScrollBar *GetScrollBarCtrl(int nBar) const override;
	static CSplitterWnd *PASCAL GetParentSplitter(
		const CWnd *pWnd, BOOL bAnyState);

protected:
	CDocument *m_pDocument;
	BOOL m_bInitialRedraw;

public:
	BOOL OnCmdMsg(UINT nID, int nCode, void *pExtra,
		AFX_CMDHANDLERINFO *pHandlerInfo) override;
protected:
	BOOL PreCreateWindow(CREATESTRUCT &cs) override;
	void PostNcDestroy() override;

	// friend classes that call protected CView overridables
	friend class CDocument;
	friend class CDocTemplate;
	friend class CPreviewView;
	friend class CFrameWnd;
	friend class CMDIFrameWnd;
	friend class CMDIChildWnd;
	friend class CSplitterWnd;
	friend class COleServerDoc;
	friend class CDocObjectServer;

	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg int OnMouseActivate(CWnd *pDesktopWnd, UINT nHitTest, UINT message);

	// Print client support: used for interaction with Windows task bar
	afx_msg LRESULT OnPrintClient(CDC *pDC, UINT nFlags);

	// commands
	afx_msg void OnUpdateSplitCmd(CCmdUI *pCmdUI);
	afx_msg BOOL OnSplitCmd(UINT nID);
	afx_msg void OnUpdateNextPaneMenu(CCmdUI *pCmdUI);
	afx_msg BOOL OnNextPaneCmd(UINT nID);

	// not mapped commands - must be mapped in derived class
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintPreview();

	DECLARE_MESSAGE_MAP()
};

/*============================================================================*/
// class CScrollView supports simple scrolling and scaling

class _AFX_MOUSEANCHORWND;

class CScrollView : public CView {
	DECLARE_DYNAMIC(CScrollView)

	// Constructors
protected:
	CScrollView();

public:
	static AFX_DATA const SIZE sizeDefault;
	// used to specify default calculated page and line sizes

// in logical units - call one of the following Set routines
	void SetScaleToFitSize(SIZE sizeTotal);
	void SetScrollSizes(int nMapMode, SIZE sizeTotal,
		const SIZE &sizePage = sizeDefault,
		const SIZE &sizeLine = sizeDefault);

	// Attributes
public:
	CPoint GetScrollPosition() const;       // upper corner of scrolling
	CSize GetTotalSize() const;             // logical size

	void CheckScrollBars(BOOL &bHasHorzBar, BOOL &bHasVertBar) const;

	// for device units
	CPoint GetDeviceScrollPosition() const;
	void GetDeviceScrollSizes(int &nMapMode, SIZE &sizeTotal,
		SIZE &sizePage, SIZE &sizeLine) const;

	// Operations
public:
	void ScrollToPosition(POINT pt);    // set upper left position
	void FillOutsideRect(CDC *pDC, CBrush *pBrush);
	void ResizeParentToFit(BOOL bShrinkOnly = TRUE);
	BOOL DoMouseWheel(UINT fFlags, short zDelta, CPoint point);

	// Implementation
protected:
	_AFX_MOUSEANCHORWND *m_pAnchorWindow; // window for wheel mouse anchor
	friend class _AFX_MOUSEANCHORWND;
	int m_nMapMode;				 // mapping mode for window creation
	CSize m_totalLog;           // total size in logical units (no rounding)
	CSize m_totalDev;           // total size in device units
	CSize m_pageDev;            // per page scroll size in device units
	CSize m_lineDev;            // per line scroll size in device units

	BOOL m_bCenter;             // Center output if larger than total size
	BOOL m_bInsideUpdate;       // internal state for OnSize callback
	void CenterOnPoint(CPoint ptCenter);
	void ScrollToDevicePosition(POINT ptDev); // explicit scrolling no checking

protected:
	void OnDraw(CDC *pDC) override = 0;      // pass on pure virtual

	void UpdateBars();          // adjust scrollbars etc
	BOOL GetTrueClientSize(CSize &size, CSize &sizeSb);
	// size with no bars
	void GetScrollBarSizes(CSize &sizeSb);
	void GetScrollBarState(CSize sizeClient, CSize &needSb,
		CSize &sizeRange, CPoint &ptMove, BOOL bInsideClient);

public:
	virtual ~CScrollView();
#ifdef _DEBUG
	void Dump(CDumpContext &) const override;
	void AssertValid() const override;
#endif //_DEBUG
	void CalcWindowRect(LPRECT lpClientRect,
		UINT nAdjustType = adjustBorder) override;
	void OnPrepareDC(CDC *pDC, CPrintInfo *pInfo = NULL) override;

	virtual CSize GetWheelScrollDistance(CSize sizeDistance,
		BOOL bHorz, BOOL bVert);

	// scrolling implementation support for OLE
	BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE) override;
	BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE) override;

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT fFlags, short zDelta, CPoint point);
	afx_msg LRESULT HandleMButtonDown(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPrintClient(CDC *pDC, UINT nFlags);

	DECLARE_MESSAGE_MAP()
};

class CWinThread : public CCmdTarget {
	DECLARE_DYNAMIC(CWinThread)

	friend BOOL AfxInternalPreTranslateMessage(MSG *pMsg);

public:
	// Constructors
	CWinThread();
	BOOL CreateThread(DWORD dwCreateFlags = 0, UINT nStackSize = 0,
		LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

	// Attributes
	CWnd *m_pMainWnd;       // main window (usually same AfxGetApp()->m_pMainWnd)
	CWnd *m_pActiveWnd;     // active main window (may not be m_pMainWnd)
	BOOL m_bAutoDelete;     // enables 'delete this' after thread termination

	// only valid while running
	HANDLE m_hThread;       // this thread's HANDLE
	operator HANDLE() const;
	DWORD m_nThreadID;      // this thread's ID

	int GetThreadPriority();
	BOOL SetThreadPriority(int nPriority);

	// Operations
	DWORD SuspendThread();
	DWORD ResumeThread();
	BOOL PostThreadMessage(UINT message, WPARAM wParam, LPARAM lParam);

	// Overridables
		// thread initialization
	virtual BOOL InitInstance();

	// running and idle processing
	virtual int Run();
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	virtual BOOL PumpMessage();     // low level message pump
	virtual BOOL OnIdle(LONG lCount); // return TRUE if more idle processing
	virtual BOOL IsIdleMessage(MSG *pMsg);  // checks for special messages

	// thread termination
	virtual int ExitInstance(); // default will 'delete this'

	// Advanced: exception handling
	virtual LRESULT ProcessWndProcException(CException *e, const MSG *pMsg);

	// Advanced: handling messages sent to message filter hook
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);

	// Advanced: virtual access to m_pMainWnd
	virtual CWnd *GetMainWnd();

	// Implementation
public:
	virtual ~CWinThread();
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext &dc) const override;
#endif
	void CommonConstruct();
	virtual void Delete();
	// 'delete this' only if m_bAutoDelete == TRUE

public:
	// constructor used by implementation of AfxBeginThread
	CWinThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam);

	// valid after construction
	LPVOID m_pThreadParams; // generic parameters passed to starting function
	AFX_THREADPROC m_pfnThreadProc;

	// set after OLE is initialized
	void (AFXAPI *m_lpfnOleTermOrFreeLib)(BOOL, BOOL);
	COleMessageFilter *m_pMessageFilter;

protected:
	BOOL DispatchThreadMessageEx(MSG *msg);  // helper
	void DispatchThreadMessage(MSG *msg);  // obsolete
};

/*============================================================================*/
// The device context

class CDC : public CObject {
	DECLARE_DYNCREATE(CDC)
public:

	// Attributes
	HDC m_hDC;          // The output DC (must be first data member)
	HDC m_hAttribDC;    // The Attribute DC
	operator HDC() const;
	HDC GetSafeHdc() const; // Always returns the Output DC
	CWnd *GetWindow() const;

	static CDC *PASCAL FromHandle(HDC hDC);
	static void PASCAL DeleteTempMap();
	BOOL Attach(HDC hDC);   // Attach/Detach affects only the Output DC
	HDC Detach();

	virtual void SetAttribDC(HDC hDC);  // Set the Attribute DC
	virtual void SetOutputDC(HDC hDC);  // Set the Output DC
	virtual void ReleaseAttribDC();     // Release the Attribute DC
	virtual void ReleaseOutputDC();     // Release the Output DC

	BOOL IsPrinting() const;            // TRUE if being used for printing

	CPen *GetCurrentPen() const;
	CBrush *GetCurrentBrush() const;
	CPalette *GetCurrentPalette() const;
	CFont *GetCurrentFont() const;
	CBitmap *GetCurrentBitmap() const;

	// for bidi and mirrored localization
	DWORD GetLayout() const;
	DWORD SetLayout(DWORD dwLayout);

	// Constructors
	CDC();
	BOOL CreateDC(LPCTSTR lpszDriverName, LPCTSTR lpszDeviceName,
		LPCTSTR lpszOutput, const void *lpInitData);
	BOOL CreateIC(LPCTSTR lpszDriverName, LPCTSTR lpszDeviceName,
		LPCTSTR lpszOutput, const void *lpInitData);
	BOOL CreateCompatibleDC(CDC *pDC);

	BOOL DeleteDC();

	// Device-Context Functions
	virtual int SaveDC();
	virtual BOOL RestoreDC(int nSavedDC);
	int GetDeviceCaps(int nIndex) const;
	UINT SetBoundsRect(LPCRECT lpRectBounds, UINT flags);
	UINT GetBoundsRect(LPRECT lpRectBounds, UINT flags);
	BOOL ResetDC(const DEVMODE *lpDevMode);

	// Drawing-Tool Functions
	CPoint GetBrushOrg() const;
	CPoint SetBrushOrg(int x, int y);
	CPoint SetBrushOrg(POINT point);
	int EnumObjects(int nObjectType,
		int (CALLBACK *lpfn)(LPVOID, LPARAM), LPARAM lpData);

	// Type-safe selection helpers
public:
	virtual CGdiObject *SelectStockObject(int nIndex);
	CPen *SelectObject(CPen *pPen);
	CBrush *SelectObject(CBrush *pBrush);
	virtual CFont *SelectObject(CFont *pFont);
	CBitmap *SelectObject(CBitmap *pBitmap);
	int SelectObject(CRgn *pRgn);       // special return for regions
	CGdiObject *SelectObject(CGdiObject *pObject);
	// CGdiObject* provided so compiler doesn't use SelectObject(HGDIOBJ)

// Color and Color Palette Functions
	COLORREF GetNearestColor(COLORREF crColor) const;
	CPalette *SelectPalette(CPalette *pPalette, BOOL bForceBackground);
	UINT RealizePalette();
	void UpdateColors();

	// Drawing-Attribute Functions
	COLORREF GetBkColor() const;
	int GetBkMode() const;
	int GetPolyFillMode() const;
	int GetROP2() const;
	int GetStretchBltMode() const;
	COLORREF GetTextColor() const;

	virtual COLORREF SetBkColor(COLORREF crColor);
	int SetBkMode(int nBkMode);
	int SetPolyFillMode(int nPolyFillMode);
	int SetROP2(int nDrawMode);
	int SetStretchBltMode(int nStretchMode);
	virtual COLORREF SetTextColor(COLORREF crColor);

	BOOL GetColorAdjustment(LPCOLORADJUSTMENT lpColorAdjust) const;
	BOOL SetColorAdjustment(const COLORADJUSTMENT *lpColorAdjust);

	COLORREF GetDCBrushColor() const;
	COLORREF SetDCBrushColor(COLORREF crColor);

	COLORREF GetDCPenColor() const;
	COLORREF SetDCPenColor(COLORREF crColor);

	// Graphics mode
	int SetGraphicsMode(int iMode);
	int GetGraphicsMode() const;

	// World transform
	BOOL SetWorldTransform(const XFORM *pXform);
	BOOL ModifyWorldTransform(const XFORM *pXform, DWORD iMode);
	BOOL GetWorldTransform(XFORM *pXform) const;

	// Mapping Functions
	int GetMapMode() const;
	CPoint GetViewportOrg() const;
	virtual int SetMapMode(int nMapMode);
	// Viewport Origin
	virtual CPoint SetViewportOrg(int x, int y);
	CPoint SetViewportOrg(POINT point);
	virtual CPoint OffsetViewportOrg(int nWidth, int nHeight);

	// Viewport Extent
	CSize GetViewportExt() const;
	virtual CSize SetViewportExt(int cx, int cy);
	CSize SetViewportExt(SIZE size);
	virtual CSize ScaleViewportExt(int xNum, int xDenom, int yNum, int yDenom);

	// Window Origin
	CPoint GetWindowOrg() const;
	CPoint SetWindowOrg(int x, int y);
	CPoint SetWindowOrg(POINT point);
	CPoint OffsetWindowOrg(int nWidth, int nHeight);

	// Window extent
	CSize GetWindowExt() const;
	virtual CSize SetWindowExt(int cx, int cy);
	CSize SetWindowExt(SIZE size);
	virtual CSize ScaleWindowExt(int xNum, int xDenom, int yNum, int yDenom);

	// Coordinate Functions
	void DPtoLP(LPPOINT lpPoints, int nCount = 1) const;
	void DPtoLP(LPRECT lpRect) const;
	void DPtoLP(LPSIZE lpSize) const;
	void LPtoDP(LPPOINT lpPoints, int nCount = 1) const;
	void LPtoDP(LPRECT lpRect) const;
	void LPtoDP(LPSIZE lpSize) const;

	// Special Coordinate Functions (useful for dealing with metafiles and OLE)
	void DPtoHIMETRIC(LPSIZE lpSize) const;
	void LPtoHIMETRIC(LPSIZE lpSize) const;
	void HIMETRICtoDP(LPSIZE lpSize) const;
	void HIMETRICtoLP(LPSIZE lpSize) const;

	// Region Functions
	BOOL FillRgn(CRgn *pRgn, CBrush *pBrush);
	BOOL FrameRgn(CRgn *pRgn, CBrush *pBrush, int nWidth, int nHeight);
	BOOL InvertRgn(CRgn *pRgn);
	BOOL PaintRgn(CRgn *pRgn);

	// Clipping Functions
	virtual int GetClipBox(LPRECT lpRect) const;
	virtual BOOL PtVisible(int x, int y) const;
	BOOL PtVisible(POINT point) const;
	virtual BOOL RectVisible(LPCRECT lpRect) const;
	int SelectClipRgn(CRgn *pRgn);
	int ExcludeClipRect(int x1, int y1, int x2, int y2);
	int ExcludeClipRect(LPCRECT lpRect);
	int ExcludeUpdateRgn(CWnd *pWnd);
	int IntersectClipRect(int x1, int y1, int x2, int y2);
	int IntersectClipRect(LPCRECT lpRect);
	int OffsetClipRgn(int x, int y);
	int OffsetClipRgn(SIZE size);
	int SelectClipRgn(CRgn *pRgn, int nMode);

	// Line-Output Functions
	CPoint GetCurrentPosition() const;
	CPoint MoveTo(int x, int y);
	CPoint MoveTo(POINT point);
	BOOL LineTo(int x, int y);
	BOOL LineTo(POINT point);
	BOOL Arc(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
	BOOL Arc(LPCRECT lpRect, POINT ptStart, POINT ptEnd);
	BOOL Polyline(const POINT *lpPoints, int nCount);

	BOOL AngleArc(int x, int y, int nRadius, float fStartAngle, float fSweepAngle);
	BOOL ArcTo(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
	BOOL ArcTo(LPCRECT lpRect, POINT ptStart, POINT ptEnd);
	int GetArcDirection() const;
	int SetArcDirection(int nArcDirection);

	BOOL PolyDraw(const POINT *lpPoints, const BYTE *lpTypes, int nCount);
	BOOL PolylineTo(const POINT *lpPoints, int nCount);
	BOOL PolyPolyline(const POINT *lpPoints,
		const DWORD *lpPolyPoints, int nCount);

	BOOL PolyBezier(const POINT *lpPoints, int nCount);
	BOOL PolyBezierTo(const POINT *lpPoints, int nCount);

	// Simple Drawing Functions
	void FillRect(LPCRECT lpRect, CBrush *pBrush);
	void FrameRect(LPCRECT lpRect, CBrush *pBrush);
	void InvertRect(LPCRECT lpRect);
	BOOL DrawIcon(int x, int y, HICON hIcon);
	BOOL DrawIcon(POINT point, HICON hIcon);
	BOOL DrawState(CPoint pt, CSize size, HBITMAP hBitmap, UINT nFlags,
		HBRUSH hBrush = NULL);
	BOOL DrawState(CPoint pt, CSize size, CBitmap *pBitmap, UINT nFlags,
		CBrush *pBrush = NULL);
	BOOL DrawState(CPoint pt, CSize size, HICON hIcon, UINT nFlags,
		HBRUSH hBrush = NULL);
	BOOL DrawState(CPoint pt, CSize size, HICON hIcon, UINT nFlags,
		CBrush *pBrush = NULL);
	BOOL DrawState(CPoint pt, CSize size, LPCTSTR lpszText, UINT nFlags,
		BOOL bPrefixText = TRUE, int nTextLen = 0, HBRUSH hBrush = NULL);
	BOOL DrawState(CPoint pt, CSize size, LPCTSTR lpszText, UINT nFlags,
		BOOL bPrefixText = TRUE, int nTextLen = 0, CBrush *pBrush = NULL);
	BOOL DrawState(CPoint pt, CSize size, DRAWSTATEPROC lpDrawProc,
		LPARAM lData, UINT nFlags, HBRUSH hBrush = NULL);
	BOOL DrawState(CPoint pt, CSize size, DRAWSTATEPROC lpDrawProc,
		LPARAM lData, UINT nFlags, CBrush *pBrush = NULL);

	// Ellipse and Polygon Functions
	BOOL Chord(int x1, int y1, int x2, int y2, int x3, int y3,
		int x4, int y4);
	BOOL Chord(LPCRECT lpRect, POINT ptStart, POINT ptEnd);
	void DrawFocusRect(LPCRECT lpRect);
	BOOL Ellipse(int x1, int y1, int x2, int y2);
	BOOL Ellipse(LPCRECT lpRect);
	BOOL Pie(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
	BOOL Pie(LPCRECT lpRect, POINT ptStart, POINT ptEnd);
	BOOL Polygon(const POINT *lpPoints, int nCount);
	BOOL PolyPolygon(const POINT *lpPoints, const INT *lpPolyCounts, int nCount);
	BOOL Rectangle(int x1, int y1, int x2, int y2);
	BOOL Rectangle(LPCRECT lpRect);
	BOOL RoundRect(int x1, int y1, int x2, int y2, int x3, int y3);
	BOOL RoundRect(LPCRECT lpRect, POINT point);

	// Bitmap Functions
	BOOL PatBlt(int x, int y, int nWidth, int nHeight, DWORD dwRop);
	BOOL BitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
		int xSrc, int ySrc, DWORD dwRop);
	BOOL StretchBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
		int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop);
	COLORREF GetPixel(int x, int y) const;
	COLORREF GetPixel(POINT point) const;
	COLORREF SetPixel(int x, int y, COLORREF crColor);
	COLORREF SetPixel(POINT point, COLORREF crColor);
	BOOL FloodFill(int x, int y, COLORREF crColor);
	BOOL ExtFloodFill(int x, int y, COLORREF crColor, UINT nFillType);
	BOOL MaskBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
		int xSrc, int ySrc, CBitmap &maskBitmap, int xMask, int yMask,
		DWORD dwRop);
	BOOL PlgBlt(LPPOINT lpPoint, CDC *pSrcDC, int xSrc, int ySrc,
		int nWidth, int nHeight, CBitmap &maskBitmap, int xMask, int yMask);
	BOOL SetPixelV(int x, int y, COLORREF crColor);
	BOOL SetPixelV(POINT point, COLORREF crColor);
	BOOL GradientFill(TRIVERTEX *pVertices, ULONG nVertices,
		void *pMesh, ULONG nMeshElements, DWORD dwMode);
	BOOL TransparentBlt(int xDest, int yDest, int nDestWidth, int nDestHeight,
		CDC *pSrcDC, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight,
		UINT clrTransparent);
	BOOL AlphaBlend(int xDest, int yDest, int nDestWidth, int nDestHeight,
		CDC *pSrcDC, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight,
		BLENDFUNCTION blend);

	// Text Functions
	virtual BOOL TextOut(int x, int y, LPCTSTR lpszString, int nCount);
	BOOL TextOut(int x, int y, const CString &str);
	virtual BOOL ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
		LPCTSTR lpszString, UINT nCount, LPINT lpDxWidths);
	BOOL ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
		const CString &str, LPINT lpDxWidths);
	virtual CSize TabbedTextOut(int x, int y, LPCTSTR lpszString, int nCount,
		int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin);
	CSize TabbedTextOut(int x, int y, const CString &str,
		int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin);

#pragma push_macro("DrawText")
#pragma push_macro("DrawTextEx")
#undef DrawText
#undef DrawTextEx
	virtual int _AFX_FUNCNAME(DrawText)(LPCTSTR lpszString, int nCount, LPRECT lpRect,
		UINT nFormat);
	int _AFX_FUNCNAME(DrawText)(const CString &str, LPRECT lpRect, UINT nFormat);

	virtual int _AFX_FUNCNAME(DrawTextEx)(LPTSTR lpszString, int nCount, LPRECT lpRect,
		UINT nFormat, LPDRAWTEXTPARAMS lpDTParams);
	int _AFX_FUNCNAME(DrawTextEx)(const CString &str, LPRECT lpRect, UINT nFormat, LPDRAWTEXTPARAMS lpDTParams);

#if 0
	int DrawText(LPCTSTR lpszString, int nCount, LPRECT lpRect, UINT nFormat);
	int DrawText(const CString &str, LPRECT lpRect, UINT nFormat);

	int DrawTextEx(LPTSTR lpszString, int nCount, LPRECT lpRect,
		UINT nFormat, LPDRAWTEXTPARAMS lpDTParams);
	int DrawTextEx(const CString &str, LPRECT lpRect, UINT nFormat, LPDRAWTEXTPARAMS lpDTParams);
#endif
	CSize GetTextExtent(LPCTSTR lpszString, int nCount) const;
	CSize GetTextExtent(const CString &str) const;
	CSize GetOutputTextExtent(LPCTSTR lpszString, int nCount) const;
	CSize GetOutputTextExtent(const CString &str) const;
	CSize GetTabbedTextExtent(LPCTSTR lpszString, int nCount,
		int nTabPositions, LPINT lpnTabStopPositions) const;
	CSize GetTabbedTextExtent(const CString &str,
		int nTabPositions, LPINT lpnTabStopPositions) const;
	CSize GetOutputTabbedTextExtent(LPCTSTR lpszString, int nCount,
		int nTabPositions, LPINT lpnTabStopPositions) const;
	CSize GetOutputTabbedTextExtent(const CString &str,
		int nTabPositions, LPINT lpnTabStopPositions) const;
	virtual BOOL GrayString(CBrush *pBrush,
		BOOL(CALLBACK *lpfnOutput)(HDC, LPARAM, int), LPARAM lpData,
		int nCount, int x, int y, int nWidth, int nHeight);
	UINT GetTextAlign() const;
	UINT SetTextAlign(UINT nFlags);
	int GetTextFace(_In_ int nCount, _Out_writes_to_(nCount, return) LPTSTR lpszFacename) const;
	int GetTextFace(CString &rString) const;
	BOOL GetTextMetrics(LPTEXTMETRIC lpMetrics) const;
	BOOL GetOutputTextMetrics(LPTEXTMETRIC lpMetrics) const;
	int SetTextJustification(int nBreakExtra, int nBreakCount);
	int GetTextCharacterExtra() const;
	int SetTextCharacterExtra(int nCharExtra);

	DWORD GetCharacterPlacement(LPCTSTR lpString, int nCount, int nMaxExtent, LPGCP_RESULTS lpResults, DWORD dwFlags) const;
	DWORD GetCharacterPlacement(CString &str, int nMaxExtent, LPGCP_RESULTS lpResults, DWORD dwFlags) const;

	BOOL GetTextExtentExPointI(LPWORD pgiIn, int cgi, int nMaxExtent, LPINT lpnFit, LPINT alpDx, _Out_opt_ LPSIZE lpSize) const;
	BOOL GetTextExtentPointI(LPWORD pgiIn, int cgi, _Out_opt_ LPSIZE lpSize) const;

	// Advanced Drawing
	BOOL DrawEdge(LPRECT lpRect, UINT nEdge, UINT nFlags);
	BOOL DrawFrameControl(LPRECT lpRect, UINT nType, UINT nState);

	// Scrolling Functions
	BOOL ScrollDC(int dx, int dy, LPCRECT lpRectScroll, LPCRECT lpRectClip,
		CRgn *pRgnUpdate, LPRECT lpRectUpdate);

	// Font Functions
	BOOL GetCharWidth(UINT nFirstChar, UINT nLastChar, LPINT lpBuffer) const;
	BOOL GetOutputCharWidth(UINT nFirstChar, UINT nLastChar, LPINT lpBuffer) const;
	DWORD SetMapperFlags(DWORD dwFlag);
	CSize GetAspectRatioFilter() const;

	BOOL GetCharABCWidths(UINT nFirstChar, UINT nLastChar, LPABC lpabc) const;
	DWORD GetFontData(DWORD dwTable, DWORD dwOffset, LPVOID lpData, DWORD cbData) const;
	int GetKerningPairs(int nPairs, LPKERNINGPAIR lpkrnpair) const;
	UINT GetOutlineTextMetrics(UINT cbData, LPOUTLINETEXTMETRIC lpotm) const;
	DWORD GetGlyphOutline(UINT nChar, UINT nFormat, LPGLYPHMETRICS lpgm,
		DWORD cbBuffer, LPVOID lpBuffer, const MAT2 *lpmat2) const;

	BOOL GetCharABCWidths(UINT nFirstChar, UINT nLastChar,
		LPABCFLOAT lpABCF) const;
	BOOL GetCharWidth(UINT nFirstChar, UINT nLastChar,
		float *lpFloatBuffer) const;

	DWORD GetFontLanguageInfo() const;

	BOOL GetCharABCWidthsI(UINT giFirst, UINT cgi, LPWORD pgi, LPABC lpabc) const;
	BOOL GetCharWidthI(UINT giFirst, UINT cgi, LPWORD pgi, LPINT lpBuffer) const;

	// Printer/Device Escape Functions
	virtual int Escape(_In_ int nEscape, _In_ int nCount,
		_In_reads_bytes_(nCount) LPCSTR lpszInData, _In_ LPVOID lpOutData);
	int Escape(_In_ int nEscape, _In_ int nInputSize, _In_reads_bytes_(nInputSize) LPCSTR lpszInputData,
		_In_ int nOutputSize, _Out_writes_bytes_(nOutputSize) LPSTR lpszOutputData);
	int DrawEscape(int nEscape, int nInputSize, LPCSTR lpszInputData);

	// Escape helpers
	int StartDoc(LPCTSTR lpszDocName);  // old Win3.0 version
	int StartDoc(LPDOCINFO lpDocInfo);
	int StartPage();
	int EndPage();
	int SetAbortProc(BOOL(CALLBACK *lpfn)(HDC, int));
	int AbortDoc();
	int EndDoc();

	// MetaFile Functions
	BOOL PlayMetaFile(HMETAFILE hMF);
	BOOL PlayMetaFile(HENHMETAFILE hEnhMetaFile, LPCRECT lpBounds);
	BOOL AddMetaFileComment(UINT nDataSize, const BYTE *pCommentData);
	// can be used for enhanced metafiles only

// Path Functions
	BOOL AbortPath();
	BOOL BeginPath();
	BOOL CloseFigure();
	BOOL EndPath();
	BOOL FillPath();
	BOOL FlattenPath();
	BOOL StrokeAndFillPath();
	BOOL StrokePath();
	BOOL WidenPath();
	float GetMiterLimit() const;
	BOOL SetMiterLimit(float fMiterLimit);
	int GetPath(LPPOINT lpPoints, LPBYTE lpTypes, int nCount) const;
	BOOL SelectClipPath(int nMode);

	// Misc Helper Functions
	static CBrush *PASCAL GetHalftoneBrush();
	void DrawDragRect(LPCRECT lpRect, SIZE size,
		LPCRECT lpRectLast, SIZE sizeLast,
		CBrush *pBrush = NULL, CBrush *pBrushLast = NULL);
	void FillSolidRect(LPCRECT lpRect, COLORREF clr);
	void FillSolidRect(int x, int y, int cx, int cy, COLORREF clr);
	void Draw3dRect(LPCRECT lpRect, COLORREF clrTopLeft, COLORREF clrBottomRight);
	void Draw3dRect(int x, int y, int cx, int cy,
		COLORREF clrTopLeft, COLORREF clrBottomRight);

	// Implementation
public:
	virtual ~CDC();
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext &dc) const override;
#endif

	// advanced use and implementation
	BOOL m_bPrinting;
	HGDIOBJ SelectObject(HGDIOBJ);      // do not use for regions

protected:
	// used for implementation of non-virtual SelectObject calls
	static CGdiObject *PASCAL SelectGdiObject(HDC hDC, HGDIOBJ h);
};

/*============================================================================*/
// CDC Helpers

class CPaintDC : public CDC
{
	DECLARE_DYNAMIC(CPaintDC)

	// Constructors
public:
	explicit CPaintDC(CWnd *pWnd);   // BeginPaint

	// Attributes
protected:
	HWND m_hWnd;
public:
	PAINTSTRUCT m_ps;       // actual paint struct!

	// Implementation
public:
	virtual ~CPaintDC();
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext &dc) const override;
#endif
};

class CClientDC : public CDC
{
	DECLARE_DYNAMIC(CClientDC)

	// Constructors
public:
	explicit CClientDC(CWnd *pWnd);

	// Attributes
protected:
	HWND m_hWnd;

	// Implementation
public:
	virtual ~CClientDC();
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext &dc) const override;
#endif
};

class CWindowDC : public CDC
{
	DECLARE_DYNAMIC(CWindowDC)

	// Constructors
public:
	explicit CWindowDC(CWnd *pWnd);

	// Attributes
protected:
	HWND m_hWnd;

	// Implementation
public:
	virtual ~CWindowDC();
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext &dc) const override;
#endif
};

/*============================================================================*/
// CWinApp - the root of all Windows applications

#define _AFX_MRU_COUNT   4      // default support for 4 entries in file MRU
#define _AFX_MRU_MAX_COUNT 16   // currently allocated id range supports 16

#define _AFX_SYSPOLICY_NOTINITIALIZED			0
#define _AFX_SYSPOLICY_NORUN					1
#define _AFX_SYSPOLICY_NODRIVES					2
#define _AFX_SYSPOLICY_RESTRICTRUN				4
#define _AFX_SYSPOLICY_NONETCONNECTDISCONNECTD	8
#define _AFX_SYSPOLICY_NOENTIRENETWORK			16
#define _AFX_SYSPOLICY_NORECENTDOCHISTORY		32
#define _AFX_SYSPOLICY_NOCLOSE					64
#define _AFX_SYSPOLICY_NOPLACESBAR				128
#define _AFX_SYSPOLICY_NOBACKBUTTON				256
#define _AFX_SYSPOLICY_NOFILEMRU				512

// Restart Manager support flags
#define AFX_RESTART_MANAGER_SUPPORT_RESTART				0x01  // restart support, means application is registered via RegisterApplicationRestart
#define AFX_RESTART_MANAGER_SUPPORT_RECOVERY			0x02  // recovery support, means application is registered via RegisterApplicationRecoveryCallback
#define AFX_RESTART_MANAGER_AUTOSAVE_AT_RESTART			0x04  // auto-save support is enabled, documents will be autosaved at restart by restart manager
#define AFX_RESTART_MANAGER_AUTOSAVE_AT_INTERVAL		0x08  // auto-save support is enabled, documents will be autosaved periodically for crash recovery
#define AFX_RESTART_MANAGER_REOPEN_PREVIOUS_FILES		0x10  // reopen of previously opened documents is enabled, on restart all previous documents will be opened
#define AFX_RESTART_MANAGER_RESTORE_AUTOSAVED_FILES		0x20  // restoration of auto-saved documents is enabled, on restart user will be prompted to open auto-saved documents intead of last saved
#define AFX_RESTART_MANAGER_SUPPORT_NO_AUTOSAVE			AFX_RESTART_MANAGER_SUPPORT_RESTART | AFX_RESTART_MANAGER_SUPPORT_RECOVERY | AFX_RESTART_MANAGER_REOPEN_PREVIOUS_FILES
#define AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS			AFX_RESTART_MANAGER_SUPPORT_NO_AUTOSAVE | AFX_RESTART_MANAGER_AUTOSAVE_AT_RESTART | AFX_RESTART_MANAGER_AUTOSAVE_AT_INTERVAL | AFX_RESTART_MANAGER_RESTORE_AUTOSAVED_FILES
#define AFX_RESTART_MANAGER_SUPPORT_RESTART_ASPECTS		AFX_RESTART_MANAGER_SUPPORT_RESTART | AFX_RESTART_MANAGER_AUTOSAVE_AT_RESTART | AFX_RESTART_MANAGER_REOPEN_PREVIOUS_FILES | AFX_RESTART_MANAGER_RESTORE_AUTOSAVED_FILES
#define AFX_RESTART_MANAGER_SUPPORT_RECOVERY_ASPECTS	AFX_RESTART_MANAGER_SUPPORT_RECOVERY | AFX_RESTART_MANAGER_AUTOSAVE_AT_INTERVAL | AFX_RESTART_MANAGER_REOPEN_PREVIOUS_FILES | AFX_RESTART_MANAGER_RESTORE_AUTOSAVED_FILES

class CWinApp : public CWinThread {
	DECLARE_DYNAMIC(CWinApp)
public:

	// Constructor
	explicit CWinApp(LPCTSTR lpszAppName = NULL);     // app name defaults to EXE name

	// Attributes
		// Startup args (do not change)

		// This module's hInstance.
	HINSTANCE m_hInstance;

	// Pointer to the command-line.
	LPTSTR m_lpCmdLine;

	// Initial state of the application's window; normally,
	// this is an argument to ShowWindow().
	int m_nCmdShow;

	// Running args (can be changed in InitInstance)

	// Human-redable name of the application. Normally set in
	// constructor or retreived from AFX_IDS_APP_TITLE.
	LPCTSTR m_pszAppName;

	/// <summary>
	/// Application User Model ID.</summary>
	LPCTSTR m_pszAppID;

	// Name of registry key for this application. See
	// SetRegistryKey() member function.
	LPCTSTR m_pszRegistryKey;

	// Pointer to CDocManager used to manage document templates
	// for this application instance.
	CDocManager *m_pDocManager;

	// Support for Shift+F1 help mode.

	// TRUE if we're in SHIFT+F1 mode.
	BOOL m_bHelpMode;

public:
	// set in constructor to override default

	// Executable name (no spaces).
	LPCTSTR m_pszExeName;

	// Default based on this module's path.
	LPCTSTR m_pszHelpFilePath;

	// Default based on this application's name.
	LPCTSTR m_pszProfileName;

	// Sets and initializes usage of HtmlHelp instead of WinHelp.
	void EnableHtmlHelp();

	// Sets and initializes usage of HtmlHelp instead of WinHelp.
	void SetHelpMode(AFX_HELP_TYPE eHelpType);
	AFX_HELP_TYPE GetHelpMode();

	// help mode used by the app
	AFX_HELP_TYPE m_eHelpType;

	// Returns TRUE is application is running under Windows 7 or higher
	BOOL IsWindows7();

	// Initialization Operations - should be done in InitInstance
protected:
	// Load MRU file list and last preview state.
	void LoadStdProfileSettings(UINT nMaxMRU = _AFX_MRU_COUNT);

	void EnableShellOpen();

	// SetDialogBkColor is no longer supported.
	// To change dialog background and text color, handle WM_CTLCOLORDLG instead.
	AFX_DEPRECATED("CWinApp::SetDialogBkColor is no longer supported. Instead, handle WM_CTLCOLORDLG in your dialog")
		void SetDialogBkColor(COLORREF clrCtlBk = RGB(192, 192, 192), COLORREF clrCtlText = RGB(0, 0, 0));

	// Set registry key name to be used by CWinApp's
	// profile member functions; prevents writing to an INI file.
	void SetRegistryKey(LPCTSTR lpszRegistryKey);
	void SetRegistryKey(UINT nIDRegistryKey);

	/// <summary>
	/// Explicitly sets Application User Model ID for the application. This method should be called before any user interface
	/// is presented to user (the best place is the application constructor).</summary>
	/// <param name="lpcszAppID">Specifies the Application User Model ID.</param>
	void SetAppID(LPCTSTR lpcszAppID);

	// Enable3dControls and Enable3dControlsStatic are no longer necessary.
	AFX_DEPRECATED("CWinApp::Enable3dControls is no longer needed. You should remove this call.")
		BOOL Enable3dControls();
#ifndef _AFXDLL
	AFX_DEPRECATED("CWinApp::Enable3dControlsStatic is no longer needed. You should remove this call.")
		BOOL Enable3dControlsStatic();
#endif

	void RegisterShellFileTypes(BOOL bCompat = FALSE);

	// call after all doc templates are registered
	void UnregisterShellFileTypes();

public:
	// Loads a cursor resource.
	HCURSOR LoadCursor(LPCTSTR lpszResourceName) const;
	HCURSOR LoadCursor(UINT nIDResource) const;

	// Loads a stock cursor resource; for for IDC_* values.
	HCURSOR LoadStandardCursor(LPCTSTR lpszCursorName) const;

	// Loads an OEM cursor; for all OCR_* values.
	HCURSOR LoadOEMCursor(UINT nIDCursor) const;

	// Loads an icon resource.
	HICON LoadIcon(LPCTSTR lpszResourceName) const;
	HICON LoadIcon(UINT nIDResource) const;

	// Loads an icon resource; for stock IDI_ values.
	HICON LoadStandardIcon(LPCTSTR lpszIconName) const;

	// Loads an OEM icon resource; for all OIC_* values.
	HICON LoadOEMIcon(UINT nIDIcon) const;

	// Retrieve an integer value from INI file or registry.
	virtual UINT GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault);

	// Sets an integer value to INI file or registry.
	virtual BOOL WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);

	// Retrieve a string value from INI file or registry.
	virtual CString GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL);

	// Sets a string value to INI file or registry.
	virtual BOOL WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);

	// Retrieve an arbitrary binary value from INI file or registry.
	virtual BOOL GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE *ppData, UINT *pBytes);

	// Sets an arbitrary binary value to INI file or registry.
	virtual BOOL WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes);

	// Override in derived class.
	virtual void InitLibId();

	// Register
	virtual BOOL Register();

	// Unregisters everything this app was known to register.
	virtual BOOL Unregister();

	/// <summary>
	/// Deletes the subkeys and values of the specified key recursively.</summary>
	/// <returns>
	/// If the function succeeds, the return value is ERROR_SUCCESS. If the function fails, the return value is a nonzero error code defined in Winerror.h</returns>
	/// <param name="hParentKey">A handle to an open registry key.</param>
	/// <param name="strKeyName">The name of the key to be deleted.</param>
	/// <param name="pTM">Pointer to CAtlTransactionManager object</param>
	LONG DelRegTree(HKEY hParentKey, const CString &strKeyName, CAtlTransactionManager *pTM = NULL);

	// Running Operations - to be done on a running application
		// Dealing with document templates
	void AddDocTemplate(CDocTemplate *pTemplate);
	POSITION GetFirstDocTemplatePosition() const;
	CDocTemplate *GetNextDocTemplate(POSITION &pos) const;

	// Open named file, trying to match a regsitered
	// document template to it.
	virtual CDocument *OpenDocumentFile(LPCTSTR lpszFileName);
	virtual CDocument *OpenDocumentFile(LPCTSTR lpszFileName, BOOL bAddToMRU);

	// Add a string to the recent file list. Remove oldest string,
	// if no space left.
	virtual void AddToRecentFileList(LPCTSTR lpszPathName);

	// Printer DC Setup routine, 'struct tagPD' is a PRINTDLG structure.
	void SelectPrinter(HANDLE hDevNames, HANDLE hDevMode,
		BOOL bFreeOld = TRUE);

	// Create a DC for the system default printer.
	BOOL CreatePrinterDC(CDC &dc);


	BOOL GetPrinterDeviceDefaults(PRINTDLG *pPrintDlg);

	// Run this app as an embedded object.
	BOOL RunEmbedded();

	// Run this app as an OLE automation server.
	BOOL RunAutomated();

	// Parse the command line for stock options and commands.
	void ParseCommandLine(CCommandLineInfo &rCmdInfo);

	// React to a shell-issued command line directive.
	BOOL ProcessShellCommand(CCommandLineInfo &rCmdInfo);

	// Overridables

		// Hooks for your initialization code
	virtual BOOL InitApplication();

	// exiting
	virtual BOOL SaveAllModified(); // save before exit
	void HideApplication();
	void CloseAllDocuments(BOOL bEndSession); // close documents before exiting

	// Advanced: to override message boxes and other hooks
	virtual int DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt);
	virtual void DoWaitCursor(int nCode); // 0 => restore, 1=> begin, -1=> end

	// Advanced: process async DDE request
	virtual BOOL OnDDECommand(_In_z_ LPTSTR lpszCommand);

	// Advanced: Help support
	virtual void WinHelp(DWORD_PTR dwData, UINT nCmd = HELP_CONTEXT);
	virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd = 0x000F);
	virtual void WinHelpInternal(DWORD_PTR dwData, UINT nCmd = HELP_CONTEXT);

	// Restart Manager support
	virtual HRESULT RegisterWithRestartManager(BOOL bRegisterRecoveryCallback, const CString &strRestartIdentifier);
	virtual HRESULT RegisterWithRestartManager(LPCWSTR pwzCommandLineArgs, DWORD dwRestartFlags, APPLICATION_RECOVERY_CALLBACK pRecoveryCallback, LPVOID lpvParam, DWORD dwPingInterval, DWORD dwCallbackFlags);
	virtual DWORD ApplicationRecoveryCallback(LPVOID lpvParam);

	virtual BOOL SupportsRestartManager() const {
		return m_dwRestartManagerSupportFlags & AFX_RESTART_MANAGER_SUPPORT_RESTART;
	}
	virtual BOOL SupportsApplicationRecovery() const {
		return m_dwRestartManagerSupportFlags & AFX_RESTART_MANAGER_SUPPORT_RECOVERY;
	}
	virtual BOOL SupportsAutosaveAtRestart() const {
		return m_dwRestartManagerSupportFlags & AFX_RESTART_MANAGER_AUTOSAVE_AT_RESTART;
	}
	virtual BOOL SupportsAutosaveAtInterval() const {
		return m_dwRestartManagerSupportFlags & AFX_RESTART_MANAGER_AUTOSAVE_AT_INTERVAL;
	}
	virtual BOOL ReopenPreviousFilesAtRestart() const {
		return m_dwRestartManagerSupportFlags & AFX_RESTART_MANAGER_REOPEN_PREVIOUS_FILES;
	}
	virtual BOOL RestoreAutosavedFilesAtRestart() const {
		return m_dwRestartManagerSupportFlags & AFX_RESTART_MANAGER_RESTORE_AUTOSAVED_FILES;
	}

	virtual DWORD GetApplicationRestartFlags() {
		return 0;
	}  // Flags for RegisterApplicationRestart: default is none of RESTART_NO_CRASH/RESTART_NO_HANG/RESTART_NO_PATCH/RESTART_NO_REBOOT
	virtual LPVOID GetApplicationRecoveryParameter() {
		return NULL;
	}  // Parameter to be passed along to RegisterApplicationRecoveryCallback
	virtual DWORD GetApplicationRecoveryPingInterval() {
		return RECOVERY_DEFAULT_PING_INTERVAL;
	}  // Ping interval for RegisterApplicationRecoveryCallback

// Command Handlers
protected:
	// map to the following for file new/open
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();

	// map to the following to enable print setup
	afx_msg void OnFilePrintSetup();

	// map to the following to enable help
	afx_msg void OnContextHelp();   // shift-F1
	afx_msg void OnHelp();          // F1 (uses current context)
	afx_msg void OnHelpIndex();     // ID_HELP_INDEX
	afx_msg void OnHelpFinder();    // ID_HELP_FINDER, ID_DEFAULT_HELP
	afx_msg void OnHelpUsing();     // ID_HELP_USING

	// Implementation
protected:
	HGLOBAL m_hDevMode;             // printer Dev Mode
	HGLOBAL m_hDevNames;            // printer Device Names
	DWORD m_dwPromptContext;        // help context override for message box

	HINSTANCE m_hLangResourceDLL;  // Satellite resource DLL

	int m_nWaitCursorCount;         // for wait cursor (>0 => waiting)
	HCURSOR m_hcurWaitCursorRestore; // old cursor to restore after wait cursor

	CRecentFileList *m_pRecentFileList;
	CDataRecoveryHandler *m_pDataRecoveryHandler;

	void UpdatePrinterSelection(BOOL bForceDefaults);
	void SaveStdProfileSettings();  // save options to .INI file

public: // public for implementation access
	CCommandLineInfo *m_pCmdInfo;

	ATOM m_atomApp, m_atomSystemTopic;   // for DDE open
	UINT m_nNumPreviewPages;        // number of default printed pages

	size_t  m_nSafetyPoolSize;      // ideal size

	void (AFXAPI *m_lpfnDaoTerm)();

	void DevModeChange(_In_z_ LPTSTR lpDeviceName);
	void SetCurrentHandles();

	// Finds number of opened CDocument items owned by templates
	// registered with the doc manager.
	int GetOpenDocumentCount();

	virtual CDataRecoveryHandler *GetDataRecoveryHandler();
	/// <summary>
	/// Tells whether Windows 7 Taskbar interaction is enabled.</summary>
	/// <returns>
	/// Returns TRUE if EnableTaskbarInteraction has been called and Operation System is
	/// Windows 7 or higher.</returns>
	/// <remarks> Taskbar interaction means that MDI application displays the content of MDI children
	/// in separate tabbed thumbnails that appear when mouse pointer is over application taskbar button.</remarks>
	virtual BOOL IsTaskbarInteractionEnabled();

	/// <summary>
	/// Enables Taskbar interaction</summary>
	/// <returns>
	/// Returns TRUE if taskbar interaction can be enabled or disabled (e.g. this method was called before creation of main window).</returns>
	/// <remarks>
	/// This method must be called before creation of main window, otherwise it asserts and returns FALSE.</remarks>
	/// <param name="bEnable"> Specifies whether interaction with Windows 7 taskbar should be enabled (TRUE), or disabled (FALSE).</param>
	BOOL EnableTaskbarInteraction(BOOL bEnable = TRUE);

#if (NTDDI_VERSION >= NTDDI_WIN7)
	/// <summary>
	/// Creates and stores in the global data a pointer to ITaskBarList interface.</summary>
	/// <returns>A pointer to ITaskbarList interface if creation of a task bar list object succeeds, or NULL if creation fails or current
	/// Operation System is less than Windows 7.</returns>
	ITaskbarList *GetITaskbarList();

	/// <summary>
	/// Creates and stores in the global data a pointer to ITaskBarList3 interface.</summary>
	/// <returns>A pointer to ITaskbarList3 interface if creation creation of a task bar list object succeeds, or NULL if creation fails or current
	/// Operation System is less than Windows 7.</returns>
	ITaskbarList3 *GetITaskbarList3();

	/// <summary>
	/// Releases interfaces obtained through GetITaskbarList and GetITaskbarList3 methods.</summary>
	void ReleaseTaskBarRefs();
#endif

	// helpers for standard commdlg dialogs
	BOOL DoPromptFileName(CString &fileName, UINT nIDSTitle,
		DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate *pTemplate);
	INT_PTR DoPrintDialog(CPrintDialog *pPD);

	void EnableModeless(BOOL bEnable); // to disable OLE in-place dialogs

	// overrides for implementation
	BOOL InitInstance() override;
	int ExitInstance() override; // return app exit code
	virtual BOOL RestartInstance(); // handle restart by Restart Manager
	int Run() override;
	BOOL OnIdle(LONG lCount) override; // return TRUE if more idle processing
	LRESULT ProcessWndProcException(CException *e, const MSG *pMsg) override;
	virtual HINSTANCE LoadAppLangResourceDLL();

	// Helper for message boxes; can work when no CWinApp can be found
	static int ShowAppMessageBox(CWinApp *pApp, LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt);
	static void DoEnableModeless(BOOL bEnable); // to disable OLE in-place dialogs

public:
	virtual ~CWinApp();
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext &dc) const override;
#endif

	// helpers for registration

	/// <summary>
	/// returns key for HKEY_CURRENT_USER\"Software"\RegistryKey\AppName\lpszSection.</summary>
	/// <returns>
	/// Section key if the function succeeds, NULL - otherwise</returns>
	/// <param name="lpszSection">The name of the key to be obtained.</param>
	/// <param name="pTM">Pointer to CAtlTransactionManager object</param>
	HKEY GetSectionKey(LPCTSTR lpszSection, CAtlTransactionManager *pTM = NULL);

	/// <summary>
	/// returns key for HKEY_CURRENT_USER\"Software"\RegistryKey\ProfileName.</summary>
	/// <returns>
	/// Applixation key if the function succeeds, NULL - otherwise</returns>
	/// <param name="pTM">Pointer to CAtlTransactionManager object</param>
	HKEY GetAppRegistryKey(CAtlTransactionManager *pTM = NULL);

protected:
	afx_msg void OnAppExit();
	afx_msg void OnUpdateRecentFileMenu(CCmdUI *pCmdUI);
	afx_msg BOOL OnOpenRecentFile(UINT nID);

	DECLARE_MESSAGE_MAP()
public:
	// System Policy Settings
	virtual BOOL LoadSysPolicies(); // Override to load policies other than the system policies that MFC loads.
	BOOL GetSysPolicyValue(DWORD dwPolicyID, BOOL *pbValue); // returns the policy's setting in the out parameter
protected:
	BOOL _LoadSysPolicies() throw(); // Implementation helper
	DWORD m_dwPolicies;				// block for storing boolean system policies

	// Restart Manager support
	DWORD m_dwRestartManagerSupportFlags; // What aspects of restart/recovery does the application support?
	int   m_nAutosaveInterval;            // How frequently are documents autosaved? (value in milliseconds)

	BOOL m_bTaskbarInteractionEnabled;
	BOOL m_bIsWindows7;

	BOOL m_bComInitialized;
	BOOL m_bTaskBarInterfacesAvailable;
	IUnknown *m_pTaskbarList;
	IUnknown *m_pTaskbarList3;
};

/*============================================================================*/
// CWnd - a Microsoft Windows application window

class COleDropTarget;   // for more information see AFXOLE.H
class COleControlContainer;
class COleControlSite;

// CWnd::m_nFlags (generic to CWnd)
#define WF_TOOLTIPS         0x0001  // window is enabled for tooltips
#define WF_TEMPHIDE         0x0002  // window is temporarily hidden
#define WF_STAYDISABLED     0x0004  // window should stay disabled
#define WF_MODALLOOP        0x0008  // currently in modal loop
#define WF_CONTINUEMODAL    0x0010  // modal loop should continue running
#define WF_OLECTLCONTAINER  0x0100  // some descendant is an OLE control
#define WF_TRACKINGTOOLTIPS 0x0400  // window is enabled for tracking tooltips

// CWnd::m_nFlags (specific to CFrameWnd)
#define WF_STAYACTIVE       0x0020  // look active even though not active
#define WF_NOPOPMSG         0x0040  // ignore WM_POPMESSAGESTRING calls
#define WF_MODALDISABLE     0x0080  // window is disabled
#define WF_KEEPMINIACTIVE   0x0200  // stay activate even though you are deactivated


#define WF_NOWIN32ISDIALOGMSG   0x0800
#define WF_ISWINFORMSVIEWWND    0x1000

// flags for CWnd::RunModalLoop
#define MLF_NOIDLEMSG       0x0001  // don't send WM_ENTERIDLE messages
#define MLF_NOKICKIDLE      0x0002  // don't send WM_KICKIDLE messages
#define MLF_SHOWONIDLE      0x0004  // show window if not visible at idle time

// extra MFC defined TTF_ flags for TOOLINFO::uFlags
#define TTF_NOTBUTTON       0x80000000L // no status help on buttondown
#define TTF_ALWAYSTIP       0x40000000L // always show the tip even if not active

#if (NTDDI_VERSION < NTDDI_WIN7)
typedef struct tagTOUCHINPUT {
} TOUCHINPUT, *PTOUCHINPUT;
typedef struct tagGESTUREINFO {
} GESTUREINFO, *PGESTUREINFO;
#endif

class CWnd : public CCmdTarget {
	DECLARE_DYNCREATE(CWnd)
protected:
	static const MSG *PASCAL GetCurrentMessage();

	// Attributes
public:
	HWND m_hWnd;            // must be first data member
	operator HWND() const;
	BOOL operator==(const CWnd &wnd) const;
	BOOL operator!=(const CWnd &wnd) const;

	HWND GetSafeHwnd() const;
	DWORD GetStyle() const;
	DWORD GetExStyle() const;
	BOOL ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0);
	BOOL ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0);

	CWnd *GetOwner() const;
	void SetOwner(_In_ CWnd *pOwnerWnd);


	BOOL GetWindowInfo(PWINDOWINFO pwi) const;
	BOOL GetTitleBarInfo(PTITLEBARINFO pti) const;

	// Constructors and other creation
	CWnd();

	static CWnd *PASCAL FromHandle(HWND hWnd);
	static CWnd *PASCAL FromHandlePermanent(HWND hWnd);
	static void PASCAL DeleteTempMap();
	BOOL Attach(HWND hWndNew);
	HWND Detach();

	// subclassing/unsubclassing functions
	virtual void PreSubclassWindow();
	BOOL SubclassWindow(HWND hWnd);
	BOOL SubclassDlgItem(UINT nID, CWnd *pParent);
	HWND UnsubclassWindow();

	// handling of RT_DLGINIT resource (extension to RT_DIALOG)
	BOOL ExecuteDlgInit(LPCTSTR lpszResourceName);
	BOOL ExecuteDlgInit(LPVOID lpResource);

public:
	// for child windows, views, panes etc
	virtual BOOL Create(LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT &rect,
		CWnd *pParentWnd, UINT nID,
		CCreateContext *pContext = NULL);

	// advanced creation (allows access to extended styles)
	virtual BOOL CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		int x, int y, int nWidth, int nHeight,
		HWND hWndParent, HMENU nIDorHMenu, LPVOID lpParam = NULL);

	virtual BOOL CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT &rect,
		CWnd *pParentWnd, UINT nID,
		LPVOID lpParam = NULL);

	// for wrapping OLE controls
	BOOL CreateControl(REFCLSID clsid, LPCTSTR pszWindowName, DWORD dwStyle,
		const RECT &rect, CWnd *pParentWnd, UINT nID, CFile *pPersist = NULL,
		BOOL bStorage = FALSE, BSTR bstrLicKey = NULL);

	BOOL CreateControl(LPCTSTR pszClass, LPCTSTR pszWindowName, DWORD dwStyle,
		const RECT &rect, CWnd *pParentWnd, UINT nID, CFile *pPersist = NULL,
		BOOL bStorage = FALSE, BSTR bstrLicKey = NULL);

	// Another overload for creating controls that use default extents.
	BOOL CreateControl(REFCLSID clsid, LPCTSTR pszWindowName, DWORD dwStyle,
		const POINT *ppt, const SIZE *psize, CWnd *pParentWnd, UINT nID,
		CFile *pPersist = NULL, BOOL bStorage = FALSE, BSTR bstrLicKey = NULL);


	//Overload for special controls (WinForms), that require more than CLSID.
	BOOL CreateControl(const CControlCreationInfo &creationInfo, DWORD dwStyle,
		const POINT *ppt, const SIZE *psize, CWnd *pParentWnd, UINT nID);

	LPUNKNOWN GetControlUnknown();
	BOOL PaintWindowlessControls(CDC *pDC);

	virtual BOOL DestroyWindow();

	// special pre-creation and window rect adjustment hooks
	virtual BOOL PreCreateWindow(CREATESTRUCT &cs);

	// Advanced: virtual AdjustWindowRect
	enum AdjustType {
		adjustBorder = 0, adjustOutside = 1
	};
	virtual void CalcWindowRect(LPRECT lpClientRect,
		UINT nAdjustType = adjustBorder);

	// Window tree access
	int GetDlgCtrlID() const;
	int SetDlgCtrlID(int nID);
	// get and set window ID, for child windows only
	CWnd *GetDlgItem(int nID) const;
	// get immediate child with given ID
	void GetDlgItem(int nID, HWND *phWnd) const;
	// as above, but returns HWND
	CWnd *GetDescendantWindow(int nID, BOOL bOnlyPerm = FALSE) const;
	// like GetDlgItem but recursive
	void SendMessageToDescendants(UINT message, WPARAM wParam = 0,
		LPARAM lParam = 0, BOOL bDeep = TRUE, BOOL bOnlyPerm = FALSE);
	CFrameWnd *GetParentFrame() const;
	CFrameWnd *EnsureParentFrame() const;
	CWnd *GetTopLevelParent() const;
	CWnd *EnsureTopLevelParent() const;
	CWnd *GetTopLevelOwner() const;
	CWnd *GetParentOwner() const;
	CFrameWnd *GetTopLevelFrame() const;
	static CWnd *PASCAL GetSafeOwner(CWnd *pParent = NULL, HWND *pWndTop = NULL);

	CWnd *GetAncestor(UINT gaFlags) const;

	// Message Functions
#pragma push_macro("SendMessage")
#undef SendMessage
	LRESULT _AFX_FUNCNAME(SendMessage)(UINT message, WPARAM wParam = 0, LPARAM lParam = 0) const;
	LRESULT SendMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0) const;
#pragma pop_macro("SendMessage")
	BOOL PostMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);

	BOOL SendNotifyMessage(UINT message, WPARAM wParam, LPARAM lParam);
	BOOL SendChildNotifyLastMsg(LRESULT *pResult = NULL);

	BOOL DragDetect(POINT pt) const;


	// Message processing for modeless dialog-like windows
	BOOL IsDialogMessage(LPMSG lpMsg);

	// Window Text Functions
	void SetWindowText(LPCTSTR lpszString);
	int GetWindowText(_Out_writes_to_(nMaxCount, return +1) LPTSTR lpszStringBuf, _In_ int nMaxCount) const;
	void GetWindowText(CString &rString) const;
	int GetWindowTextLength() const;
	void SetFont(CFont *pFont, BOOL bRedraw = TRUE);
	CFont *GetFont() const;

	// CMenu Functions - non-Child windows only
	virtual CMenu *GetMenu() const;
	virtual BOOL SetMenu(CMenu *pMenu);
	void DrawMenuBar();
	CMenu *GetSystemMenu(BOOL bRevert) const;
	BOOL HiliteMenuItem(CMenu *pMenu, UINT nIDHiliteItem, UINT nHilite);

	// Window Size and Position Functions
	BOOL IsIconic() const;
	BOOL IsZoomed() const;
	void MoveWindow(int x, int y, int nWidth, int nHeight,
		BOOL bRepaint = TRUE);
	void MoveWindow(LPCRECT lpRect, BOOL bRepaint = TRUE);
	int SetWindowRgn(HRGN hRgn, BOOL bRedraw);
	int GetWindowRgn(HRGN hRgn) const;

	static AFX_DATA const CWnd wndTop; // SetWindowPos's pWndInsertAfter
	static AFX_DATA const CWnd wndBottom; // SetWindowPos's pWndInsertAfter
	static AFX_DATA const CWnd wndTopMost; // SetWindowPos pWndInsertAfter
	static AFX_DATA const CWnd wndNoTopMost; // SetWindowPos pWndInsertAfter

	BOOL SetWindowPos(const CWnd *pWndInsertAfter, int x, int y,
		int cx, int cy, UINT nFlags);
	UINT ArrangeIconicWindows();
	void BringWindowToTop();
	void GetWindowRect(LPRECT lpRect) const;
	void GetClientRect(LPRECT lpRect) const;

	BOOL GetWindowPlacement(WINDOWPLACEMENT *lpwndpl) const;
	BOOL SetWindowPlacement(const WINDOWPLACEMENT *lpwndpl);

	// Coordinate Mapping Functions
	void ClientToScreen(LPPOINT lpPoint) const;
	void ClientToScreen(LPRECT lpRect) const;
	void ScreenToClient(LPPOINT lpPoint) const;
	void ScreenToClient(LPRECT lpRect) const;
	void MapWindowPoints(CWnd *pwndTo, LPPOINT lpPoint, UINT nCount) const;
	void MapWindowPoints(CWnd *pwndTo, LPRECT lpRect) const;

	// Update/Painting Functions
	CDC *BeginPaint(LPPAINTSTRUCT lpPaint);
	void EndPaint(LPPAINTSTRUCT lpPaint);
	CDC *GetDC();
	CDC *GetWindowDC();
	int ReleaseDC(CDC *pDC);
	void Print(CDC *pDC, DWORD dwFlags) const;
	void PrintClient(CDC *pDC, DWORD dwFlags) const;

	void UpdateWindow();
	void SetRedraw(BOOL bRedraw = TRUE);
	BOOL GetUpdateRect(LPRECT lpRect, BOOL bErase = FALSE);
	int GetUpdateRgn(CRgn *pRgn, BOOL bErase = FALSE);
	void Invalidate(BOOL bErase = TRUE);
	void InvalidateRect(LPCRECT lpRect, BOOL bErase = TRUE);
	void InvalidateRgn(CRgn *pRgn, BOOL bErase = TRUE);
	void ValidateRect(LPCRECT lpRect);
	void ValidateRgn(CRgn *pRgn);
	BOOL ShowWindow(int nCmdShow);
	BOOL IsWindowVisible() const;
	void ShowOwnedPopups(BOOL bShow = TRUE);

	CDC *GetDCEx(CRgn *prgnClip, DWORD flags);
	BOOL LockWindowUpdate();
	void UnlockWindowUpdate();
	BOOL RedrawWindow(LPCRECT lpRectUpdate = NULL,
		CRgn *prgnUpdate = NULL,
		UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	BOOL EnableScrollBar(int nSBFlags, UINT nArrowFlags = ESB_ENABLE_BOTH);

	BOOL DrawAnimatedRects(int idAni, CONST RECT *lprcFrom, CONST RECT *lprcTo);
	BOOL DrawCaption(CDC *pDC, LPCRECT lprc, UINT uFlags);

	BOOL AnimateWindow(DWORD dwTime, DWORD dwFlags);
	BOOL PrintWindow(CDC *pDC, UINT nFlags) const;

public:
	// Layered Window

	BOOL SetLayeredWindowAttributes(COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
	BOOL UpdateLayeredWindow(CDC *pDCDst, POINT *pptDst, SIZE *psize,
		CDC *pDCSrc, POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags);
	BOOL GetLayeredWindowAttributes(COLORREF *pcrKey, BYTE *pbAlpha, DWORD *pdwFlags) const;

	// Timer Functions
	UINT_PTR SetTimer(UINT_PTR nIDEvent, UINT nElapse,
		void (CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD));
	BOOL KillTimer(UINT_PTR nIDEvent);

	// ToolTip Functions
	BOOL EnableToolTips(BOOL bEnable = TRUE);
	BOOL EnableTrackingToolTips(BOOL bEnable = TRUE);
	static void PASCAL CancelToolTips(BOOL bKeys = FALSE);
	void FilterToolTipMessage(MSG *pMsg);

	// for command hit testing (used for automatic tooltips)
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO *pTI) const;

	// Window State Functions
	BOOL IsWindowEnabled() const;
	BOOL EnableWindow(BOOL bEnable = TRUE);

	// the active window applies only to top-level (frame windows)
	static CWnd *PASCAL GetActiveWindow();
	CWnd *SetActiveWindow();

	// the foreground window applies only to top-level windows (frame windows)
	BOOL SetForegroundWindow();
	static CWnd *PASCAL GetForegroundWindow();

	// capture and focus apply to all windows
	static CWnd *PASCAL GetCapture();
	CWnd *SetCapture();
	static CWnd *PASCAL GetFocus();
	CWnd *SetFocus();

	static CWnd *PASCAL GetDesktopWindow();

	// Obsolete and non-portable APIs - not recommended for new code
	void CloseWindow();
	BOOL OpenIcon();

	// Dialog-Box Item Functions
	// (NOTE: Dialog-Box Items/Controls are not necessarily in dialog boxes!)
	void CheckDlgButton(int nIDButton, UINT nCheck);
	void CheckRadioButton(int nIDFirstButton, int nIDLastButton,
		int nIDCheckButton);
	int GetCheckedRadioButton(int nIDFirstButton, int nIDLastButton) const;
	int DlgDirList(_Inout_z_ LPTSTR lpPathSpec, _In_ int nIDListBox,
		_In_ int nIDStaticPath, _In_ UINT nFileType);
	int DlgDirListComboBox(_Inout_z_ LPTSTR lpPathSpec, _In_ int nIDComboBox,
		_In_ int nIDStaticPath, _In_ UINT nFileType);
	AFX_DEPRECATED("CWnd::DlgDirSelect(lpszOut, nControlId) is no longer supported. Instead, use CWnd::DlgDirSelect(lpszOut, nSize, nControlId)")
		BOOL DlgDirSelect(_Out_writes_z_(_MAX_PATH) LPTSTR lpString, _In_ int nIDListBox);
	BOOL DlgDirSelect(_Out_writes_z_(nSize) LPTSTR lpString, _In_ int nSize, _In_ int nIDListBox);
	AFX_DEPRECATED("CWnd::DlgDirSelectComboBox(lpszOut, nControlId) is no longer supported. Instead, use CWnd::DlgDirSelectComboBox(lpszOut, nSize, nControlId)")
		BOOL DlgDirSelectComboBox(_Out_writes_z_(_MAX_PATH) LPTSTR lpString, _In_ int nIDComboBox);
	BOOL DlgDirSelectComboBox(_Out_writes_z_(nSize) LPTSTR lpString, _In_ int nSize, _In_ int nIDComboBox);

	UINT GetDlgItemInt(int nID, BOOL *lpTrans = NULL,
		BOOL bSigned = TRUE) const;
	int GetDlgItemText(_In_ int nID, _Out_writes_to_(nMaxCount, return +1) LPTSTR lpStr, _In_ int nMaxCount) const;
	int GetDlgItemText(int nID, CString &rString) const;
	CWnd *GetNextDlgGroupItem(CWnd *pWndCtl, BOOL bPrevious = FALSE) const;
	COleControlSiteOrWnd *GetNextDlgGroupItem(COleControlSiteOrWnd *pCurSiteOrWnd = NULL) const;
	COleControlSiteOrWnd *GetPrevDlgGroupItem(COleControlSiteOrWnd *pCurSiteOrWnd = NULL) const;
	void RemoveRadioCheckFromGroup(const COleControlSiteOrWnd *pSiteOrWnd) const;
	CWnd *GetNextDlgTabItem(CWnd *pWndCtl, BOOL bPrevious = FALSE) const;
	COleControlSiteOrWnd *GetNextDlgTabItem(COleControlSiteOrWnd *pCurSiteOrWnd, BOOL bPrevious) const;
	UINT IsDlgButtonChecked(int nIDButton) const;
	LRESULT SendDlgItemMessage(int nID, UINT message,
		WPARAM wParam = 0, LPARAM lParam = 0);
	void SetDlgItemInt(int nID, UINT nValue, BOOL bSigned = TRUE);
	void SetDlgItemText(int nID, LPCTSTR lpszString);
	POSITION FindSiteOrWnd(const COleControlSiteOrWnd *pSiteOrWnd) const;
	POSITION FindSiteOrWndWithFocus() const;

	// Scrolling Functions
	int GetScrollPos(int nBar) const;
	void GetScrollRange(int nBar, LPINT lpMinPos, LPINT lpMaxPos) const;
	void ScrollWindow(int xAmount, int yAmount,
		LPCRECT lpRect = NULL,
		LPCRECT lpClipRect = NULL);
	int SetScrollPos(int nBar, int nPos, BOOL bRedraw = TRUE);
	void SetScrollRange(int nBar, int nMinPos, int nMaxPos,
		BOOL bRedraw = TRUE);
	void ShowScrollBar(UINT nBar, BOOL bShow = TRUE);
	void EnableScrollBarCtrl(int nBar, BOOL bEnable = TRUE);
	virtual CScrollBar *GetScrollBarCtrl(int nBar) const;
	// return sibling scrollbar control (or NULL if none)

	int ScrollWindowEx(int dx, int dy,
		LPCRECT lpRectScroll, LPCRECT lpRectClip,
		CRgn *prgnUpdate, LPRECT lpRectUpdate, UINT flags);
	BOOL SetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo,
		BOOL bRedraw = TRUE);
	BOOL GetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo, UINT nMask = SIF_ALL);
	int GetScrollLimit(int nBar);

	BOOL GetScrollBarInfo(LONG idObject, PSCROLLBARINFO psbi) const;

	// Window Access Functions
	CWnd *ChildWindowFromPoint(POINT point) const;
	CWnd *ChildWindowFromPoint(POINT point, UINT nFlags) const;
	static CWnd *PASCAL FindWindow(LPCTSTR lpszClassName, LPCTSTR lpszWindowName);
	static CWnd *PASCAL FindWindowEx(HWND hwndParent, HWND hwndChildAfter, LPCTSTR lpszClass, LPCTSTR lpszWindow);

	CWnd *GetNextWindow(UINT nFlag = GW_HWNDNEXT) const;
	CWnd *GetTopWindow() const;

	CWnd *GetWindow(UINT nCmd) const;
	CWnd *GetLastActivePopup() const;

	BOOL IsChild(const CWnd *pWnd) const;
	CWnd *GetParent() const;
	CWnd *SetParent(CWnd *pWndNewParent);
	static CWnd *PASCAL WindowFromPoint(POINT point);

	// Alert Functions
	BOOL FlashWindow(BOOL bInvert);
#pragma push_macro("MessageBox")
#undef MessageBox
	int _AFX_FUNCNAME(MessageBox)(LPCTSTR lpszText, LPCTSTR lpszCaption = NULL,
		UINT nType = MB_OK);
	int MessageBox(LPCTSTR lpszText, LPCTSTR lpszCaption = NULL,
		UINT nType = MB_OK);
#pragma pop_macro("MessageBox")

	BOOL FlashWindowEx(DWORD dwFlags, UINT  uCount, DWORD dwTimeout);

	// Clipboard Functions
	BOOL ChangeClipboardChain(HWND hWndNext);
	HWND SetClipboardViewer();
	BOOL OpenClipboard();
	static CWnd *PASCAL GetClipboardOwner();
	static CWnd *PASCAL GetClipboardViewer();
	static CWnd *PASCAL GetOpenClipboardWindow();

	// Caret Functions
	void CreateCaret(CBitmap *pBitmap);
	void CreateSolidCaret(int nWidth, int nHeight);
	void CreateGrayCaret(int nWidth, int nHeight);
	static CPoint PASCAL GetCaretPos();
	static void PASCAL SetCaretPos(POINT point);
	void HideCaret();
	void ShowCaret();

	// Shell Interaction Functions
	void DragAcceptFiles(BOOL bAccept = TRUE);

	// Icon Functions
	HICON SetIcon(HICON hIcon, BOOL bBigIcon);
	HICON GetIcon(_In_ BOOL bBigIcon) const;

	// Context Help Functions
	BOOL SetWindowContextHelpId(DWORD dwContextHelpId);
	DWORD GetWindowContextHelpId() const;

	// Dialog Data support
public:
	BOOL UpdateData(BOOL bSaveAndValidate = TRUE);
	// data wnd must be same type as this

// Help Command Handlers
	afx_msg void OnHelp();          // F1 (uses current context)
	afx_msg void OnHelpIndex();     // ID_HELP_INDEX
	afx_msg void OnHelpFinder();    // ID_HELP_FINDER, ID_DEFAULT_HELP
	afx_msg void OnHelpUsing();     // ID_HELP_USING
	virtual void WinHelp(DWORD_PTR dwData, UINT nCmd = HELP_CONTEXT);
	virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd = 0x000F);
	virtual void WinHelpInternal(DWORD_PTR dwData, UINT nCmd = HELP_CONTEXT);

	// Layout and other functions
public:
	enum RepositionFlags
	{
		reposDefault = 0, reposQuery = 1, reposExtra = 2, reposNoPosLeftOver = 0x8000
	};
	void RepositionBars(UINT nIDFirst, UINT nIDLast, UINT nIDLeftOver,
		UINT nFlag = reposDefault, LPRECT lpRectParam = NULL,
		LPCRECT lpRectClient = NULL, BOOL bStretch = TRUE);

	// dialog support
	void UpdateDialogControls(CCmdTarget *pTarget, BOOL bDisableIfNoHndler);
	void CenterWindow(CWnd *pAlternateOwner = NULL);
	int RunModalLoop(DWORD dwFlags = 0);
	virtual BOOL ContinueModal();
	virtual void EndModalLoop(int nResult);

	// OLE control wrapper functions
	COleControlSite *GetOleControlSite(UINT idControl) const;
	void AFX_CDECL InvokeHelper(DISPID dwDispID, WORD wFlags,
		VARTYPE vtRet, void *pvRet, const BYTE *pbParamInfo, ...);
	void AFX_CDECL SetProperty(DISPID dwDispID, VARTYPE vtProp, ...);
	void GetProperty(DISPID dwDispID, VARTYPE vtProp, void *pvProp) const;
	IUnknown *GetDSCCursor();
	void BindDefaultProperty(DISPID dwDispID, VARTYPE vtProp, LPCTSTR szFieldName, CWnd *pDSCWnd);
	void BindProperty(DISPID dwDispId, CWnd *pWndDSC);

	// Accessibility Support
public:
	void EnableActiveAccessibility();
	void NotifyWinEvent(DWORD event, LONG idObjectType, LONG idObject);

public:
	// Windows 7 taskbar Tabs support

	/// <summary>
	/// Called by the framework when it needs to obtain a bitmap to be displayed on Windows 7 tab thumbnail,
	/// or on the client for application peek. </summary>
	/// <remarks>
	/// Override this method in a derived class and draw on the specified device context in order to customize thumbnail and peek.
	/// If bThumbnail is TRUE, szRequiredThumbnailSize can be ignored. In this case you should be aware
	/// that you draw full sized bitmap (e.g. a bitmap that cover the whole client area). The device context (dc) comes with selected 32 bits bitmap.
	/// The default implementation sends WM_PRINT to this window with PRF_CLIENT, PRF_CHILDREN and PRF_NONCLIENT flags.</remarks>
	/// <param name="dc"> Specifies the device context.</param>
	/// <param name="rect"> Specifies the bounding rectangle of area to render.</param>
	/// <param name="szRequiredThumbnailSize"> Specifies the size of target thumbnail. Should be ignored if bIsThumbnail is FALSE.</param>
	/// <param name="bIsThumbnail"> Specifies whether this method is called for iconic thumbnail or live preview (peek).</param>
	/// <param name="bAlphaChannelSet"> Output parameter. Set it to TRUE if your implementation initializes alpha channel of a bitmap
	/// selected in dc.</param>
	virtual void OnDrawIconicThumbnailOrLivePreview(CDC &dc, CRect rect, CSize szRequiredThumbnailSize, BOOL bIsThumbnail, BOOL &bAlphaChannelSet);

protected:
	bool m_bEnableActiveAccessibility;
	IAccessible *m_pStdObject;
	friend BOOL AFXAPI AfxWinInit(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
		_In_z_ LPTSTR lpCmdLine, _In_ int nCmdShow);

protected:
	IAccessibleProxy *m_pProxy;
	afx_msg LRESULT OnGetObject(WPARAM, LPARAM);

	DECLARE_INTERFACE_MAP()
#if 0
	class XAccessible //: public IAccessible
	{
	public:
		virtual ULONG __stdcall AddRef();
		virtual ULONG __stdcall Release();
		virtual HRESULT __stdcall QueryInterface(REFIID iid, LPVOID *ppvObj);
		virtual HRESULT __stdcall Invoke(DISPID, REFIID, LCID, WORD, DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT *);
		virtual HRESULT __stdcall GetIDsOfNames(REFIID, LPOLESTR *, UINT, LCID, DISPID *);
		virtual HRESULT __stdcall GetTypeInfoCount(unsigned int *);
		virtual HRESULT __stdcall GetTypeInfo(unsigned int, LCID, ITypeInfo **);
		virtual HRESULT __stdcall get_accParent(IDispatch **ppdispParent);
		virtual HRESULT __stdcall get_accChildCount(long *pcountChildren);
		virtual HRESULT __stdcall get_accChild(VARIANT varChild, IDispatch **ppdispChild);
		virtual HRESULT __stdcall get_accName(VARIANT varChild, BSTR *pszName);
		virtual HRESULT __stdcall get_accValue(VARIANT varChild, BSTR *pszValue);
		virtual HRESULT __stdcall get_accDescription(VARIANT varChild, BSTR *pszDescription);
		virtual HRESULT __stdcall get_accRole(VARIANT varChild, VARIANT *pvarRole);
		virtual HRESULT __stdcall get_accState(VARIANT varChild, VARIANT *pvarState);
		virtual HRESULT __stdcall get_accHelp(VARIANT varChild, BSTR *pszHelp);
		virtual HRESULT __stdcall get_accHelpTopic(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic);
		virtual HRESULT __stdcall get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut);
		virtual HRESULT __stdcall get_accFocus(VARIANT *pvarChild);
		virtual HRESULT __stdcall get_accSelection(VARIANT *pvarChildren);
		virtual HRESULT __stdcall get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction);
		virtual HRESULT __stdcall accSelect(long flagsSelect, VARIANT varChild);
		virtual HRESULT __stdcall accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);
		virtual HRESULT __stdcall accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);
		virtual HRESULT __stdcall accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
		virtual HRESULT __stdcall accDoDefaultAction(VARIANT varChild);
		virtual HRESULT __stdcall put_accName(VARIANT varChild, BSTR szName);
		virtual HRESULT __stdcall put_accValue(VARIANT varChild, BSTR szValue);
	} m_xAccessible;
	friend class XAccessible;

	class XAccessibleServer //: public IAccessibleServer
	{
	public:
		virtual ULONG __stdcall AddRef();
		virtual ULONG __stdcall Release();
		virtual HRESULT __stdcall QueryInterface(REFIID iid, LPVOID *ppvObj);
		virtual HRESULT __stdcall SetProxy(IAccessibleProxy *pProxy);
		virtual HRESULT __stdcall GetHWND(HWND *phWnd);
		virtual HRESULT __stdcall GetEnumVariant(IEnumVARIANT **ppEnumVariant);
	} m_xAccessibleServer;
	friend class XAccessibleServer;
#endif
public:
	virtual HRESULT EnsureStdObj();

	virtual HRESULT get_accParent(IDispatch **ppdispParent);
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT get_accChild(VARIANT varChild, IDispatch **ppdispChild);
	virtual HRESULT get_accName(VARIANT varChild, BSTR *pszName);
	virtual HRESULT get_accValue(VARIANT varChild, BSTR *pszValue);
	virtual HRESULT get_accDescription(VARIANT varChild, BSTR *pszDescription);
	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole);
	virtual HRESULT get_accState(VARIANT varChild, VARIANT *pvarState);
	virtual HRESULT get_accHelp(VARIANT varChild, BSTR *pszHelp);
	virtual HRESULT get_accHelpTopic(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic);
	virtual HRESULT get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut);
	virtual HRESULT get_accFocus(VARIANT *pvarChild);
	virtual HRESULT get_accSelection(VARIANT *pvarChildren);
	virtual HRESULT get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction);
	virtual HRESULT accSelect(long flagsSelect, VARIANT varChild);
	virtual HRESULT accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);
	virtual HRESULT accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
	virtual HRESULT accDoDefaultAction(VARIANT varChild);
	//Obsolete
	virtual HRESULT put_accName(VARIANT varChild, BSTR szName);
	//Obsolete
	virtual HRESULT put_accValue(VARIANT varChild, BSTR szValue);
	virtual HRESULT SetProxy(IAccessibleProxy *pProxy);
	virtual HRESULT CreateAccessibleProxy(WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	// Helpers for windows that contain windowless controls
	long GetWindowLessChildCount();
	long GetWindowedChildCount();
	long GetAccessibleChildCount();
	HRESULT GetAccessibleChild(VARIANT varChild, IDispatch **ppdispChild);
	HRESULT GetAccessibleName(VARIANT varChild, BSTR *pszName);
	HRESULT GetAccessibilityLocation(VARIANT varChild, long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight);
	HRESULT GetAccessibilityHitTest(long xLeft, long yTop, VARIANT *pvarChild);


	// Window-Management message handler member functions
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg LRESULT OnActivateTopLevel(WPARAM, LPARAM);
	afx_msg void OnCancelMode();
	afx_msg void OnChildActivate();
	afx_msg void OnClose();
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint pos);
	afx_msg BOOL OnCopyData(CWnd *pWnd, COPYDATASTRUCT *pCopyDataStruct);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);

	afx_msg void OnDestroy();
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg void OnEnterIdle(UINT nWhy, CWnd *pWho);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO *lpMMI);
	afx_msg BOOL OnHelpInfo(HELPINFO *lpHelpInfo);
	afx_msg void OnIconEraseBkgnd(CDC *pDC);
	afx_msg void OnKillFocus(CWnd *pNewWnd);
	afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu *pMenu);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnPaint();
	afx_msg void OnSyncPaint();
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	afx_msg UINT OnNotifyFormat(CWnd *pWnd, UINT nCommand);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnQueryEndSession();
	afx_msg BOOL OnQueryNewPalette();
	afx_msg BOOL OnQueryOpen();
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTCard(UINT idAction, DWORD dwActionData);
	afx_msg void OnWindowPosChanging(WINDOWPOS *lpwndpos);
	afx_msg void OnWindowPosChanged(WINDOWPOS *lpwndpos);
	afx_msg void OnSessionChange(UINT nSessionState, UINT nId);

	afx_msg void OnChangeUIState(UINT nAction, UINT nUIElement);
	afx_msg void OnUpdateUIState(UINT nAction, UINT nUIElement);
	afx_msg UINT OnQueryUIState();

	// Nonclient-Area message handler member functions
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS *lpncsp);
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcDestroy();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcMButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcMButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcMButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseHover(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseLeave();
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnNcRButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcRButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcXButtonDown(short zHitTest, UINT nButton, CPoint point);
	afx_msg void OnNcXButtonUp(short zHitTest, UINT nButton, CPoint point);
	afx_msg void OnNcXButtonDblClk(short zHitTest, UINT nButton, CPoint point);

	// System message handler member functions
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnPaletteIsChanging(CWnd *pRealizeWnd);
	afx_msg void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSysDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnAppCommand(CWnd *pWnd, UINT nCmd, UINT nDevice, UINT nKey);
	afx_msg void OnRawInput(UINT nInputCode, HRAWINPUT hRawInput);
	afx_msg void OnCompacting(UINT nCpuTime);
	afx_msg void OnDevModeChange(_In_z_ LPTSTR lpDeviceName);
	afx_msg void OnFontChange();
	afx_msg void OnPaletteChanged(CWnd *pFocusWnd);
	afx_msg void OnSpoolerStatus(UINT nStatus, UINT nJobs);
	afx_msg void OnSysColorChange();
	afx_msg void OnTimeChange();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnWinIniChange(LPCTSTR lpszSection);
	afx_msg UINT OnPowerBroadcast(UINT nPowerEvent, LPARAM lEventData);
	afx_msg void OnUserChanged();
	afx_msg void OnInputLangChange(UINT nCharSet, UINT nLocaleId);
	afx_msg void OnInputLangChangeRequest(UINT nFlags, UINT nLocaleId);
	afx_msg void OnInputDeviceChange(unsigned short nFlags, HANDLE hDevice);

	// Input message handler member functions
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnUniChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnXButtonDblClk(UINT nFlags, UINT nButton, CPoint point);
	afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);
	afx_msg void OnXButtonUp(UINT nFlags, UINT nButton, CPoint point);
	afx_msg int OnMouseActivate(CWnd *pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	// Initialization message handler member functions
	afx_msg void OnInitMenu(CMenu *pMenu);
	afx_msg void OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnUnInitMenuPopup(CMenu *pPopupMenu, UINT nFlags);

	// Clipboard message handler member functions
	afx_msg void OnAskCbFormatName(_In_ UINT nMaxCount, _Out_writes_z_(nMaxCount) LPTSTR lpszString);
	afx_msg void OnChangeCbChain(HWND hWndRemove, HWND hWndAfter);
	afx_msg void OnDestroyClipboard();
	afx_msg void OnDrawClipboard();
	afx_msg void OnHScrollClipboard(CWnd *pClipAppWnd, UINT nSBCode, UINT nPos);
	afx_msg void OnPaintClipboard(CWnd *pClipAppWnd, HGLOBAL hPaintStruct);
	afx_msg void OnRenderAllFormats();
	afx_msg void OnRenderFormat(UINT nFormat);
	afx_msg void OnSizeClipboard(CWnd *pClipAppWnd, HGLOBAL hRect);
	afx_msg void OnVScrollClipboard(CWnd *pClipAppWnd, UINT nSBCode, UINT nPos);
	afx_msg void OnClipboardUpdate();

	// Control message handler member functions
	afx_msg int OnCompareItem(int nIDCtl, LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	afx_msg void OnDeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg int OnCharToItem(UINT nChar, CListBox *pListBox, UINT nIndex);
	afx_msg int OnVKeyToItem(UINT nKey, CListBox *pListBox, UINT nIndex);

	// MDI message handler member functions
	afx_msg void OnMDIActivate(BOOL bActivate,
		CWnd *pActivateWnd, CWnd *pDeactivateWnd);

	// Menu loop notification messages
	afx_msg void OnEnterMenuLoop(BOOL bIsTrackPopupMenu);
	afx_msg void OnExitMenuLoop(BOOL bIsTrackPopupMenu);
	afx_msg void OnMenuRButtonUp(UINT nPos, CMenu *pMenu);
	afx_msg UINT OnMenuDrag(UINT nPos, CMenu *pMenu);
	afx_msg UINT OnMenuGetObject(MENUGETOBJECTINFO *pMenuGetObjectInfo);
	afx_msg void OnMenuCommand(UINT nPos, CMenu *pMenu);
	afx_msg void OnNextMenu(UINT nKey, LPMDINEXTMENU lpMdiNextMenu);

	// Win4 messages
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	afx_msg void OnStyleChanging(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	afx_msg void OnSizing(UINT nSide, LPRECT lpRect);
	afx_msg void OnMoving(UINT nSide, LPRECT lpRect);
	afx_msg void OnEnterSizeMove();
	afx_msg void OnExitSizeMove();
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);

	// Desktop Windows Manager messages
	afx_msg void OnCompositionChanged();
	afx_msg void OnNcRenderingChanged(BOOL bIsRendering);
	afx_msg void OnColorizationColorChanged(DWORD dwColorizationColor, BOOL bOpacity);
	afx_msg void OnWindowMaximizedChange(BOOL bIsMaximized);

	// touch and gesture messages:
#if (NTDDI_VERSION >= NTDDI_WIN7)
	afx_msg LRESULT OnTouchMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTabletQuerySystemGestureStatus(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGesture(WPARAM wParam, LPARAM lParam);
#endif

	// Overridables and other helpers (for implementation of derived classes)
protected:
	// for deriving from a standard control
	virtual WNDPROC *GetSuperWndProcAddr();

	// for dialog data exchange and validation
	virtual void DoDataExchange(CDataExchange *pDX);

public:
	// for modality
	virtual void BeginModalState();
	virtual void EndModalState();

	// for translating Windows messages in main message pump
	virtual BOOL PreTranslateMessage(MSG *pMsg);

	// for ambient properties exposed to contained OLE controls
	virtual BOOL OnAmbientProperty(COleControlSite *pSite, DISPID dispid,
		VARIANT *pvar);

	// for touch:

	/// <summary>
	/// Register/Unregister window Windows touch support</summary>
	/// <returns>
	/// TRUE if succeeds; otherwise FALSE.</returns>
	/// <param name="bRegister">TRUE - register Windows touch support; FALSE - otherwise.</param>
	/// <param name="ulFlags">A set of bit flags that specify optional modifications. This field may contain 0 or one of the following values: TWF_FINETOUCH; TWF_WANTPALM</param>
	BOOL RegisterTouchWindow(BOOL bRegister = TRUE, ULONG ulFlags = 0);

	/// <summary>
	/// Specifies whether CWnd has touch support</summary>
	/// <returns>
	/// TRUE if CWnd has touch support; otherwise FALSE.</returns>
	BOOL IsTouchWindow() const;

	// gesture:
#if (NTDDI_VERSION >= NTDDI_WIN7)
	/// <summary>
	/// Set gesture touch parameters</summary>
	/// <returns>
	/// TRUE if succeeds; otherwise FALSE.</returns>
	/// <param name="pConfig">Pointer to CGestureConfig. Cannot be NULL.</param>
	BOOL SetGestureConfig(CGestureConfig *pConfig);

	/// <summary>
	/// Get gesture touch parameters</summary>
	/// <returns>
	/// TRUE if succeeds; otherwise FALSE.</returns>
	/// <param name="pConfig">Pointer to CGestureConfig. Cannot be NULL.</param>
	BOOL GetGestureConfig(CGestureConfig *pConfig);

	/// <summary>
	/// Returns the current gesture information (PGESTUREINFO)</summary>
	/// <returns>
	/// Pointer to the current gesture info.</returns>
	PGESTUREINFO GetCurrentGestureInfo() const {
		return m_pCurrentGestureInfo;
	}
#endif

	// controls dynamic layout:

	/// <summary>
	/// Enables or disables layout manager for a window.</summary>
	/// <param name="bEnable"> TRUE - enable layout management, FALSE - disable layout management.</param>
	void EnableDynamicLayout(BOOL bEnable = TRUE);

	/// <summary>
	/// This function returns TRUE, if layout management is enabled for a window; otherwise FALSE.</summary>
	/// <remarks>
	/// Call EnableDynamicLayout in order to enable or disable layout management for a window.</remarks>
	/// <returns>
	/// TRUE, if layout management is enabled for a window; otherwise FALSE.</returns>
	BOOL IsDynamicLayoutEnabled() const {
		return m_pDynamicLayout != NULL;
	}

	/// <summary>
	/// Call this function to retrieve a pointer to layout manager.</summary>
	/// <remarks>
	/// Call EnableDynamicLayout in order to enable or disable layout management for a window.</remarks>
	/// <returns>
	/// Returns a pointer to the window layout manager or NULL if layout wasn't enabled.</returns>
	CMFCDynamicLayout *GetDynamicLayout() {
		return m_pDynamicLayout;
	}

protected:
	// for processing Windows messages
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	// for handling default processing
	LRESULT Default();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	// for custom cleanup after WM_NCDESTROY
	virtual void PostNcDestroy();

	// for notifications from parent
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	// return TRUE if parent should not process this message
	BOOL ReflectChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	static BOOL PASCAL ReflectLastMsg(HWND hWndChild, LRESULT *pResult = NULL);

	// for touch:
	BOOL m_bIsTouchWindowRegistered;

	/// <summary>
	/// Process inputs from Windows touch</summary>
	/// <returns>
	/// TRUE if application processes Windows touch inputs; otherwise FALSE.</returns>
	/// <param name="nInputsCount">total number of Windows touch inputs.</param>
	/// <param name="pInputs">array of TOUCHINPUT.</param>
	virtual BOOL OnTouchInputs(UINT nInputsCount, PTOUCHINPUT pInputs);

	/// <summary>
	/// Process single input from Windows touch</summary>
	/// <returns>
	/// TRUE if application processes Windows touch input; otherwise FALSE.</returns>
	/// <param name="pt">point where screen has been tocuhed (in the client coordinates).</param>
	/// <param name="nInputNumber">number of touch input.</param>
	/// <param name="nInputsCount">total number of touch inputs.</param>
	/// <param name="pInput">pointer to TOUCHINPUT structure.</param>
	virtual BOOL OnTouchInput(CPoint pt, int nInputNumber, int nInputsCount, PTOUCHINPUT pInput);

	/// <summary>
	/// The methods is called when the system asks a window which system gestures it would like to receive</summary>
	/// <returns>
	/// A value indicating which system gestures the window would like to receive (TABLET_* flags, see WM_TABLET_QUERYSYSTEMGESTURESTATUS message).</returns>
	/// <param name="ptTouch">point where screen has been tocuhed (in the client coordinates).</param>
	virtual ULONG GetGestureStatus(CPoint ptTouch);

	// for gesture:
	CPoint		 m_ptGestureFrom;
	ULONGLONG	 m_ulGestureArg;
	BOOL		 m_bGestureInited;
	PGESTUREINFO m_pCurrentGestureInfo;

	/// <summary>
	/// The method is called upon gesture zoom event</summary>
	/// <returns>
	/// TRUE if application processes this event; otherwise FALSE.</returns>
	/// <param name="ptCenter">Zoom center point. In client coordinates</param>
	/// <param name="lDelta">The distance from the center point. In pixels</param>
	virtual BOOL OnGestureZoom(CPoint ptCenter, long lDelta);

	/// <summary>
	/// The method is called upon gesture pan event</summary>
	/// <returns>
	/// TRUE if application processes this event; otherwise FALSE.</returns>
	/// <param name="ptFrom">Pan starting point. In client coordinates</param>
	/// <param name="ptTo">Pan current point. In client coordinates</param>
	virtual BOOL OnGesturePan(CPoint ptFrom, CPoint ptTo);

	/// <summary>
	/// The method is called upon gesture rotate event</summary>
	/// <returns>
	/// TRUE if application processes this event; otherwise FALSE.</returns>
	/// <param name="ptCenter">Rotation center point. In client coordinates</param>
	/// <param name="dblAngle">Rotation angle. In radians</param>
	virtual BOOL OnGestureRotate(CPoint ptCenter, double dblAngle);

	/// <summary>
	/// The method is called upon gesture 2 finger tap event</summary>
	/// <returns>
	/// TRUE if application processes this event; otherwise FALSE.</returns>
	/// <param name="ptCenter">Center point between 2 fingers. In client coordinates</param>
	virtual BOOL OnGestureTwoFingerTap(CPoint ptCenter);

	/// <summary>
	/// The method is called upon gesture press and tap event</summary>
	/// <returns>
	/// TRUE if application processes this event; otherwise FALSE.</returns>
	/// <param name="ptPress">"Pressed" point. In client coordinates</param>
	/// <param name="lDelta">The distance from the "pressed" point. In pixels</param>
	virtual BOOL OnGesturePressAndTap(CPoint ptPress, long lDelta);

	// controls layouts:
	CMFCDynamicLayout *m_pDynamicLayout;

	/// <summary>
	/// The method is called to adjust positions of child controls.
	/// It recalculates positions of child controls if layout management is enabled for a window.</summary>
	virtual void ResizeDynamicLayout();
	void InitDynamicLayout();
	BOOL LoadDynamicLayoutResource(LPCTSTR lpszResourceName);

	// Implementation
public:
	virtual ~CWnd();
	virtual BOOL CheckAutoCenter();
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext &dc) const override;
#endif
	static BOOL PASCAL GrayCtlColor(HDC hDC, HWND hWnd, UINT nCtlColor,
		HBRUSH hbrGray, COLORREF clrText);

	// helper routines for implementation
	BOOL HandleFloatingSysCommand(UINT nID, LPARAM lParam);
	BOOL IsTopParentActive() const;
	void ActivateTopParent();
	static BOOL PASCAL WalkPreTranslateTree(HWND hWndStop, MSG *pMsg);
	static CWnd *PASCAL GetDescendantWindow(HWND hWnd, int nID,
		BOOL bOnlyPerm);
	static void PASCAL SendMessageToDescendants(HWND hWnd, UINT message,
		WPARAM wParam, LPARAM lParam, BOOL bDeep, BOOL bOnlyPerm);
	virtual BOOL IsFrameWnd() const; // IsKindOf(RUNTIME_CLASS(CFrameWnd)))
	void OnFinalRelease() override;
	BOOL PreTranslateInput(LPMSG lpMsg);
	static BOOL PASCAL ModifyStyle(HWND hWnd, DWORD dwRemove, DWORD dwAdd,
		UINT nFlags);
	static BOOL PASCAL ModifyStyleEx(HWND hWnd, DWORD dwRemove, DWORD dwAdd,
		UINT nFlags);
	static void PASCAL _FilterToolTipMessage(MSG *pMsg, CWnd *pWnd);
	BOOL _EnableToolTips(BOOL bEnable, UINT nFlag);
	static HWND PASCAL GetSafeOwner_(HWND hWnd, HWND *pWndTop);
	void PrepareForHelp();

	COleControlContainer *GetControlContainer();

public:
	HWND m_hWndOwner;   // implementation of SetOwner and GetOwner
	UINT m_nFlags;      // see WF_ flags above

protected:
	WNDPROC m_pfnSuper; // for subclassing of controls
	static const UINT m_nMsgDragList;
	int m_nModalResult; // for return values from CWnd::RunModalLoop

	COleDropTarget *m_pDropTarget;  // for automatic cleanup of drop target
	friend class COleDropTarget;
	friend class CFrameWnd;

	// for creating dialogs and dialog-like windows
	BOOL CreateDlg(LPCTSTR lpszTemplateName, CWnd *pParentWnd);
	BOOL CreateDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd *pParentWnd, HINSTANCE hInst);
	BOOL CreateRunDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd *pParentWnd, HINSTANCE hInst);

	COleControlContainer *m_pCtrlCont;  // for containing OLE controls
	COleControlSite *m_pCtrlSite;       // for wrapping an OLE control
	friend class COccManager;
	friend class COleControlSite;
	friend class COleControlContainer;
	BOOL InitControlContainer(BOOL bCreateFromResource = FALSE);
	virtual BOOL CreateControlContainer(COleControlContainer **ppContainer);
	virtual BOOL CreateControlSite(COleControlContainer *pContainer,
		COleControlSite **ppSite, UINT nID, REFCLSID clsid);
	virtual BOOL SetOccDialogInfo(struct _AFX_OCC_DIALOG_INFO *pOccDialogInfo);
	virtual _AFX_OCC_DIALOG_INFO *GetOccDialogInfo();
	void AttachControlSite(CHandleMap *pMap);
public:
	void AttachControlSite(CWnd *pWndParent, UINT nIDC = 0);
	COleControlContainer *GetControlContainer() const {
		return m_pCtrlCont;
	}
	COleControlSite *GetControlSite() const {
		return m_pCtrlSite;
	}

public:
	CMFCControlContainer *GetMFCControlContainer() const {
		return m_pMFCCtrlContainer;
	}

protected:
	CMFCControlContainer *m_pMFCCtrlContainer;  // for containing MFC Feature Pack controls

protected:
	// implementation of message dispatch/hooking
	friend LRESULT CALLBACK _AfxSendMsgHook(int, WPARAM, LPARAM);
	friend void AFXAPI _AfxStandardSubclass(HWND);
	friend LRESULT CALLBACK _AfxCbtFilterHook(int, WPARAM, LPARAM);
	friend LRESULT AFXAPI AfxCallWndProc(CWnd *, HWND, UINT, WPARAM, LPARAM);

	// standard message implementation
	afx_msg LRESULT OnNTCtlColor(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDisplayChange(UINT nImageDepth, int cxScreen, int cyScreen);
	afx_msg LRESULT OnDragList(WPARAM, LPARAM);

	// Helper functions for retrieving Text from windows message / structure
	// -----------------------------------------------------------------------
	// errCode  - the errCode for the window message, uMsg
	// pszText  - buffer to grow and retrieve the text (do not allocate when calling, the function will allocate)
	// cch      - size of the buffer in TCHAR to be pass to the windows message, uMsg
	// cchBegin - initial size to allocate
	// cchEnd   - maximum size to allocate
	// uMsg     - window message
	// lParam   - the LPARAM of the message.  This is pass by reference because it could potentially be alias of pszText/cch for some messages.
	// wParam   - the WPARAM of the message.  This is pass by reference because it could potentially be alias of pszText/cch for some messages.
	// strOut   - the CString containing the received text

	template <class TReturnType, class TCchType >
	TReturnType EnlargeBufferGetText(_In_ TReturnType errCode, LPTSTR &pszText, TCchType &cch, TCchType cchBegin, TCchType cchEnd, UINT uMsg, WPARAM &wParam, LPARAM &lParam, CString &strOut) const {
		ENSURE(::IsWindow(m_hWnd));
		ENSURE(cchBegin < cchEnd);
		ENSURE(cchEnd <= INT_MAX); // CString only support up to INT_MAX
		TReturnType retCode = errCode;
		strOut = CString();
		cch = cchBegin;
		do
		{
			pszText = strOut.GetBufferSetLength(cch);
			retCode = static_cast<TReturnType>(this->SendMessage(uMsg, wParam, lParam));
			strOut.ReleaseBuffer();
			pszText = NULL;

			if (retCode == errCode)
			{
				// error clear the string and return error
				strOut = CString();
				cch = 0;
				break;
			}
			if (static_cast<TCchType>(strOut.GetLength()) < cch - 1)
			{
				cch = strOut.GetLength();
				break;
			}
		} while ((::ATL::AtlMultiply(&cch, cch, 2) == S_OK) && (cch < cchEnd));
		return retCode;
	}


	template <class TReturnType>
	inline TReturnType EnlargeBufferGetText(TReturnType errCode, LPTSTR &pszText, int &pcch, UINT uMsg, WPARAM &wParam, LPARAM &lParam, CString &strOut) const {
		return EnlargeBufferGetText<TReturnType, int>(errCode, pszText, pcch, 256, INT_MAX, uMsg, wParam, lParam, strOut);
	}

	template <class TReturnType>
	inline TReturnType EnlargeBufferGetText(TReturnType errCode, LPTSTR &pszText, UINT &pcch, UINT uMsg, WPARAM &wParam, LPARAM &lParam, CString &strOut) const {
		// using INT_MAX instead of UINT_MAX here because CString has a INT_MAX limit
		return EnlargeBufferGetText<TReturnType, UINT>(errCode, pszText, pcch, 256, INT_MAX, uMsg, wParam, lParam, strOut);
	}

	DECLARE_MESSAGE_MAP()

private:
	CWnd(HWND hWnd);    // just for special initialization

	// Support for D2D:
public:
	/// <summary>
	/// Enable/disable window D2D support This method should be called prior to initialization of the main window .</summary>
	/// <param name="bEnable">Specifies whether to turn on, or off D2D support.</param>
	/// <param name="bUseDCRenderTarget">Specifies whether to use CHwndRenderTarget or CDCRenderTarget.</param>
	void EnableD2DSupport(BOOL bEnable = TRUE, BOOL bUseDCRenderTarget = FALSE);

	/// <summary>
	/// Determines whether the D2D support is enabled</summary>
	/// <returns>
	/// TRUE if the feature is enabled; otherwise FALSE.</returns>
	BOOL IsD2DSupportEnabled();

	/// <summary>
	/// Get HWND render target associated with this window</summary>
	/// <returns>
	/// Pointer to the render target or NULL.</returns>
	CHwndRenderTarget *GetRenderTarget();

	/// <summary>
	/// Get DC render target associated with this window</summary>
	/// <returns>
	/// Pointer to the render target or NULL.</returns>
	CDCRenderTarget *GetDCRenderTarget();

	// Obsolete D2D methods:
	CHwndRenderTarget *LockRenderTarget() {
		return GetRenderTarget();
	}
	void UnlockRenderTarget() {
	}

protected:
	CRenderTarget *m_pRenderTarget;
	BOOL DoD2DPaint();
};

/*============================================================================*/
// CDialog - a modal or modeless dialog

class CDialog : public CWnd {
	DECLARE_DYNAMIC(CDialog)

	// Modeless construct
public:
	CDialog();
	void Initialize();

	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd *pParentWnd = NULL);
	virtual BOOL Create(UINT nIDTemplate, CWnd *pParentWnd = NULL);
	virtual BOOL CreateIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd *pParentWnd = NULL,
		void *lpDialogInit = NULL);
	virtual BOOL CreateIndirect(HGLOBAL hDialogTemplate, CWnd *pParentWnd = NULL);

	// Modal construct
public:
	explicit CDialog(LPCTSTR lpszTemplateName, CWnd *pParentWnd = NULL);
	explicit CDialog(UINT nIDTemplate, CWnd *pParentWnd = NULL);
	BOOL InitModalIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd *pParentWnd = NULL,
		void *lpDialogInit = NULL);
	BOOL InitModalIndirect(HGLOBAL hDialogTemplate, CWnd *pParentWnd = NULL);

	// Attributes
public:
	void MapDialogRect(LPRECT lpRect) const;
	void SetHelpID(UINT nIDR);

	// Operations
public:
	// modal processing
	virtual INT_PTR DoModal();

	// support for passing on tab control - use 'PostMessage' if needed
	void NextDlgCtrl() const;
	void PrevDlgCtrl() const;
	void GotoDlgCtrl(CWnd *pWndCtrl);

	// default button access
	void SetDefID(UINT nID);
	DWORD GetDefID() const;

	// termination
	void EndDialog(int nResult);

	// Overridables (special message map entries)
	virtual BOOL OnInitDialog();
	virtual void OnSetFont(CFont *pFont);
protected:
	virtual void OnOK();
	virtual void OnCancel();

	// Implementation
public:
	virtual ~CDialog();
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext &dc) const override;
#endif
	BOOL PreTranslateMessage(MSG *pMsg) override;
	BOOL OnCmdMsg(UINT nID, int nCode, void *pExtra,
		AFX_CMDHANDLERINFO *pHandlerInfo) override;
	BOOL CheckAutoCenter() override;

protected:
	UINT m_nIDHelp;                 // Help ID (0 for none, see HID_BASE_RESOURCE)

	// parameters for 'DoModal'
	LPCTSTR m_lpszTemplateName;     // name or MAKEINTRESOURCE
	HGLOBAL m_hDialogTemplate;      // indirect (m_lpDialogTemplate == NULL)
	LPCDLGTEMPLATE m_lpDialogTemplate;  // indirect if (m_lpszTemplateName == NULL)
	void *m_lpDialogInit;           // DLGINIT resource data
	CWnd *m_pParentWnd;             // parent/owner window
	HWND m_hWndTop;                 // top level parent window (may be disabled)
	BOOL m_bClosedByEndDialog;		// indicates that the dialog was closed by calling EndDialog method

	_AFX_OCC_DIALOG_INFO *m_pOccDialogInfo;
	BOOL SetOccDialogInfo(_AFX_OCC_DIALOG_INFO *pOccDialogInfo) override;
	_AFX_OCC_DIALOG_INFO *GetOccDialogInfo() override;
	virtual void PreInitDialog();

	// implementation helpers
	HWND PreModal();
	void PostModal();

	BOOL CreateIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd *pParentWnd,
		void *lpDialogInit, HINSTANCE hInst);
	BOOL CreateIndirect(HGLOBAL hDialogTemplate, CWnd *pParentWnd,
		HINSTANCE hInst);

protected:
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM);
	afx_msg void OnSetFont(CFont *pFont, BOOL bRedraw);
	afx_msg void OnPaint();
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnEndSession(BOOL bEnding);

	DECLARE_MESSAGE_MAP()
};

// all CModalDialog functionality is now in CDialog
#define CModalDialog    CDialog

/*============================================================================*/

class CScrollBar : public CWnd {
	DECLARE_DYNAMIC(CScrollBar)

	// Constructors
public:
	CScrollBar();
	virtual BOOL Create(DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID);

	// Attributes
	int GetScrollPos() const;
	int SetScrollPos(int nPos, BOOL bRedraw = TRUE);
	void GetScrollRange(LPINT lpMinPos, LPINT lpMaxPos) const;
	void SetScrollRange(int nMinPos, int nMaxPos, BOOL bRedraw = TRUE);
	void ShowScrollBar(BOOL bShow = TRUE);

	BOOL EnableScrollBar(UINT nArrowFlags = ESB_ENABLE_BOTH);

	BOOL SetScrollInfo(LPSCROLLINFO lpScrollInfo, BOOL bRedraw = TRUE);
	BOOL GetScrollInfo(LPSCROLLINFO lpScrollInfo, UINT nMask = SIF_ALL);
	int GetScrollLimit();
	BOOL GetScrollBarInfo(PSCROLLBARINFO pScrollInfo) const;

	// Implementation
public:
	virtual ~CScrollBar();
};

} // namespace MFC
} // namespace Bagel

#endif
