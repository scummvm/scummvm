// gtldoc.cpp -- implementation of the CGtlDoc class
// Written by John J. Xenakis for Boffo Games Inc., 1994

#include "stdafx.h"
#include "gtl.h"

#include "gtldoc.h"

///DEFS gtldoc.h

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGtlDoc

IMPLEMENT_DYNCREATE(CGtlDoc, CDocument)

BEGIN_MESSAGE_MAP(CGtlDoc, CDocument)
	//{{AFX_MSG_MAP(CGtlDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGtlDoc construction/destruction

CGtlDoc::CGtlDoc()
{
    TRACECONSTRUCTOR(CGtlDoc) ;

    // clear all data
    memset(&m_cStartData, 0, &m_cEndData - &m_cStartData);
}

CGtlDoc::~CGtlDoc()
{
    TRACEDESTRUCTOR(CGtlDoc) ;
    DeleteContents() ;
}

BOOL CGtlDoc::OnNewDocument()
{
    CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application
    if (!CDocument::OnNewDocument())
	return FALSE;

    // TODO: add reinitialization code here
    // (SDI documents will reuse this document)

    if (++xpGtlApp->m_iNumOpens == 1 && xpGtlApp->m_szFilename[0])
        InitDocument(xpGtlApp->m_szFilename) ;
    else
        InitDocument("default.gtl");

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////


//* CGtlDoc::DeleteContents() --
void CGtlDoc::DeleteContents()
{
    if (m_xpcInfDlg)
        m_xpcInfDlg->DestroyWindow() ;

    if (m_xpcCtlDlg)
        m_xpcCtlDlg->DestroyWindow() ;

    if (m_xpcNodeDlg)
        m_xpcNodeDlg->DestroyWindow() ;

    if (m_xpcMenuDlg)
        m_xpcMenuDlg->DestroyWindow() ;

    if (m_xpGtlData)
        delete m_xpGtlData ;

    // clear all data
    memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;
}

//* CGtlDoc::InitDocument -- initialize document to specified file
void CGtlDoc::InitDocument(const char *xpszPathName)
// xpszPathName -- filename to be opened
// returns: Void
{
    CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application
    memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;

//  dbgtrc = TRUE ;
    m_xpGtlData = new CGtlData ;
    m_xpGtlData->m_xpcGtlDoc = this ;
    m_xpGtlData->m_bShowNodes = xpGtlApp->m_bShowNodes ;
    m_xpGtlData->m_bPaintBackground  = xpGtlApp->m_bPaintBackground ;

    OnChangedViewList() ;
    FixChecks() ;
    if (xpszPathName && *xpszPathName)
        m_xpGtlData->Compile(xpszPathName) ;

#ifdef _DEBUG
    if (CBdbgMgr::GetPointer()->m_iDebugValues[1])
        dbgtrc = TRUE ; // turn on tracing
#endif

    m_xpGtlData->m_bStartMetaGame = xpGtlApp->m_bStartMetaGame ;
    UpdateAllViews(NULL, 0L, NULL) ;

#ifdef NODEEDIT
    if (xpGtlApp->m_bMenuDialog && m_xpGtlFrame)
        m_xpGtlFrame->ShowMenuDialog() ;
    if (xpGtlApp->m_bControlDialog && m_xpGtlFrame)
        m_xpGtlFrame->ShowControlDialog() ;
#endif
}

//* CGtlDoc::OnOpenDocument --
BOOL CGtlDoc::OnOpenDocument(const char *xpszPathName)
{
    if (!CDocument::OnOpenDocument(xpszPathName))
        return FALSE;

    InitDocument(xpszPathName) ;
    return TRUE;
}

//* CGtlDoc::OnSaveDocument --
BOOL CGtlDoc::OnSaveDocument(const char *xpszPathName)
{
#ifdef NODEEDIT
    m_xpGtlData->Decompile(xpszPathName) ;
#endif

    return(TRUE);
}


//* CGtlDoc::DoOnFileSaveAs -- 
BOOL CGtlDoc::DoOnFileSaveAs(void)
// returns: TRUE if error, FALSE otherwise
{
    CDocument::OnFileSaveAs() ;

    return(FALSE);
}


/////////////////////////////////////////////////////////////////////////////
// CGtlDoc serialization

void CGtlDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
	// TODO: add storing code here
    }
    else
    {
	// TODO: add loading code here
    }
}

/////////////////////////////////////////////////////////////////////////////
// CGtlDoc diagnostics

#ifdef _DEBUG
void CGtlDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGtlDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGtlDoc commands



//* CGtlDoc::OnChangedViewList -- called by MFC when a view is
//		added or deleted
VOID CGtlDoc::OnChangedViewList(void)
// returns: VOID
{
    JXENTER(CGtlDoc::OnChangedViewList) ;
    int iError = 0 ;		// error code
    POSITION nViewPosition = GetFirstViewPosition() ;
    CGtlView *xpGtlView ;

    // if we haven't yet found frame window
    //
    if (!m_xpGtlFrame) {

        if ((xpGtlView = (CGtlView *)GetNextView(nViewPosition)) != NULL) {
            if (!m_xpcLastFocusView)
                m_xpcLastFocusView = xpGtlView ;

            if (!m_xpcLastMouseView)
                m_xpcLastMouseView = xpGtlView ;

            CWnd *pWnd = xpGtlView ;
            while (pWnd->GetParent())
                pWnd = pWnd->GetParent();

            m_xpGtlFrame = (CGtlFrame *)pWnd ;
        }
    }

    if (m_xpGtlFrame) {
        m_xpGtlFrame->m_xpDocument = this ;
        m_xpGtlFrame->m_xpcLastMouseView = m_xpcLastMouseView ;
        m_xpGtlFrame->m_xpcLastFocusView = m_xpcLastFocusView ;
    }

// cleanup:

    JXELEAVE(CGtlDoc::OnChangedViewList) ;
    RETURN_VOID ;
}

//* CGtlDoc::FixChecks -- fix dialog box check marks
BOOL CGtlDoc::FixChecks(void)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlDoc::FixChecks) ;
    int iError = 0 ;		// error code

    if (m_xpGtlFrame && m_xpGtlFrame->GetMenu()) {

        m_xpGtlFrame->GetMenu()->CheckMenuItem(ID_VIEW_INFO_DLG, m_xpcInfDlg ? MF_CHECKED : MF_UNCHECKED);
        m_xpGtlFrame->GetMenu()->CheckMenuItem(ID_VIEW_CTL_DLG, m_xpcCtlDlg ? MF_CHECKED : MF_UNCHECKED);
        m_xpGtlFrame->GetMenu()->CheckMenuItem(ID_VIEW_NODE_DLG, m_xpcNodeDlg ? MF_CHECKED : MF_UNCHECKED);
        m_xpGtlFrame->GetMenu()->CheckMenuItem(ID_VIEW_MENU_DLG, m_xpcMenuDlg ? MF_CHECKED : MF_UNCHECKED);
    }

// cleanup:

    JXELEAVE(CGtlDoc::FixChecks) ;
    RETURN(iError != 0) ;
}
