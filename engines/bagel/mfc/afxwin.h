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

namespace Bagel {
namespace MFC {

typedef void *LPENUMOLEVERB;
typedef void *LPMSG;
typedef void *LPTYPEINFO;
typedef void *LPTYPELIB;
typedef void AFX_CMDHANDLERINFO;
typedef void CTypeLibCache;
typedef void CVariantBoolConverter;
typedef void AFX_EVENT;

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

class CCmdTarget;
class CView;
class CFrameWnd;

struct tagDISPPARAMS;
typedef tagDISPPARAMS DISPPARAMS;

struct tagVARIANT;
typedef tagVARIANT VARIANT;

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


} // namespace MFC
} // namespace Bagel

#endif
