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
#define _AFX_FUNCNAME(NAME) NAME

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

class CDocManager;
class CCmdTarget;
class CView;
class CWnd;
class CFrameWnd;

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

} // namespace MFC
} // namespace Bagel

#endif
