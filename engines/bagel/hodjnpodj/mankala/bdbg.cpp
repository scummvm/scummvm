// bdbg.cpp -- Boffo debugging for meta game
// Written by John J. Xenakis for Boffo Games Inc., 1994

#include "stdafx.h"
#include "bdbg.h"

#include "copyrite.h"	// mandatory internal copyright notice

///DEFS bdbg.h

CBdbgMgr FAR * CBdbgMgr::lpBdbgMgr = NULL ;

//* CBdbgMgr::CBdbgMgr -- constructor
PUBLIC CBdbgMgr::CBdbgMgr(void)
{
    JXENTER(CBdbgMgr::CBdbgMgr) ;
    int iError = 0 ;		// error code

    memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;
					// zero out all data
    lpBdbgMgr = this ;
// cleanup:

    JXELEAVE(CBdbgMgr::CBdbgMgr) ;
    RETURN_VOID ;
}

//* CBdbgMgr::~CBdbgMgr -- destructor
PUBLIC CBdbgMgr::~CBdbgMgr(void)
// returns: VOID
{
    JXENTER(CBdbgMgr::~CBdbgMgr) ;
    int iError = 0 ;		// error code
    if (m_lpTraceObjects)
	delete [] m_lpTraceObjects ;
    m_lpTraceObjects = NULL ;

    lpBdbgMgr = NULL ;

// cleanup:

    JXELEAVE(CBdbgMgr::~CBdbgMgr) ;
    RETURN_VOID ;
}

//* CBdbgMgr::DebugInit -- Initialize
PUBLIC BOOL CBdbgMgr::DebugInit(LPSTR lpszIniFilename,
			LPSTR lpszIniSectionname)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CBdbgMgr::DebugInit) ;
    int iError = 0 ;		// error code
    int iK ;		// loop variable
    char szStr[100] ;

    _fstrncpy(m_szIniFilename, lpszIniFilename,
    			sizeof(m_szIniFilename)) ;
    _fstrncpy(m_szIniSectionname, lpszIniSectionname,
    			sizeof(m_szIniSectionname)) ;

    m_bDebug = GetDebugInt("debug") ;
    m_bDebugMessages = GetDebugInt("debugmessages") ;
    m_bTimeMessage = GetDebugInt("timemessage") ;
    m_iConstructorMsgLevel = GetDebugInt("constructormsglevel") ;
    m_bVerifyDc = GetDebugInt("verifydc") ;
    m_bTrack = GetDebugInt("track") ;
    m_bTrace = GetDebugInt("trace") ;
    m_bTraceError = GetDebugInt("traceerror") ;

    dbgtrk = m_bTrack ;
    dbgtrr = m_bTraceError ;
    dbgtrc = m_bTrace ;

    GetDebugString("debugoutput", dbgfile, DBGFILESIZE, NULL) ;
    dbgreopen = GetDebugInt("reopen", MAXPOSINT) ;

    for (iK = 0 ; iK < DIMENSION(m_iDebugValues) ; ++iK)
    {
	_snprintf(szStr, sizeof(szStr)-1, "debug%d", iK) ;
	m_iDebugValues[iK] = GetDebugInt(szStr) ;
    }

    m_iTraceObjectCount = GetDebugInt("traceobjectcount") ;

    if (m_iTraceObjectCount)
    {
	if (m_iTraceObjectCount < 100)
	    m_iTraceObjectCount = 100 ;
	if (!m_lpTraceObjects)
	    m_lpTraceObjects = new LPVOID[m_iTraceObjectCount + 1] ;
    }

    if (m_bDebugMessages || m_bTimeMessage)
	OutputWithTime("\n\nDebug Output File: %s.\n\n") ;

// cleanup:

    JXELEAVE(CBdbgMgr::DebugInit) ;
    RETURN(iError != 0) ;
}

//* CBdbgMgr::GetDebugInt -- get debugging integer
PUBLIC int CBdbgMgr::GetDebugInt(LPSTR lpszOption,
			int iDefault PDFT(0))
// lpszOption -- option name string
// iDefault -- default value
// returns: debugging integer value
{
    JXENTER(CBdbgMgr::GetDebugInt) ;
    int iError = 0 ;		// error code

    int iRetval = ::GetPrivateProfileInt(m_szIniSectionname,
    		lpszOption, iDefault, m_szIniFilename) ;

// cleanup:

    JXELEAVE(CBdbgMgr::GetDebugInt) ;
    RETURN(iRetval) ;
}



//* CBdbgMgr::GetDebugString -- 
PUBLIC BOOL CBdbgMgr::GetDebugString(LPCSTR lpszOption,
    	LPSTR lpszTarget, int iTargetSize,
		LPSTR lpszDefault PDFT(NULL))
// lpszOption -- option name string
// lpszDefault -- default value; if NULL, then default is null string
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CBdbgMgr::GetDebugString) ;
    int iError = 0 ;		// error code

    if (!lpszDefault)
	lpszDefault = "" ;

    ::GetPrivateProfileString(m_szIniSectionname,
    		lpszOption, lpszDefault, lpszTarget, iTargetSize,
    		m_szIniFilename) ;

// cleanup:

    JXELEAVE(CBdbgMgr::GetDebugString) ;
    RETURN(iError != 0) ;
}

//* CBdbgMgr::TraceConstructor -- trace object constructor, if optioned
PUBLIC BOOL CBdbgMgr::TraceConstructor(LPCSTR lpszName, LPVOID lpLoc)
// lpszName -- name of object
// lpLoc -- location of object
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CBdbgMgr::TraceConstructor) ;
    int iError = 0 ;		// error code
    char szStr[100] ;

    if (m_iConstructorMsgLevel)
    {
	_snprintf(szStr, sizeof(szStr) - 1,
			"Constructing %Fs at %#04x:%#04x.\n",
			lpszName, HIWORD(lpLoc), LOWORD(lpLoc)) ;
	JXOutputDebugString(szStr) ;
    }

    iError = AddTraceObject(lpszName, lpLoc) ;

// cleanup:

    JXELEAVE(CBdbgMgr::TraceConstructor) ;
    RETURN(iError != 0) ;
}

//* CBdbgMgr::TraceDestructor -- trace object destructor, if optioned
PUBLIC BOOL CBdbgMgr::TraceDestructor(LPCSTR lpszName, LPVOID lpLoc)
// lpszName -- name of object
// lpLoc -- location of object
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CBdbgMgr::TraceDestructor) ;
    int iError = 0 ;		// error code
    char szStr[100] ;

    if (m_iConstructorMsgLevel)
    {
	_snprintf(szStr, sizeof(szStr)-1,
			"Destructing %Fs at %#04x:%#04x.\n",
			lpszName, HIWORD(lpLoc), LOWORD(lpLoc)) ;
	JXOutputDebugString(szStr) ;
    }

    iError = RemoveTraceObject(lpszName, lpLoc) ;

// cleanup:

    JXELEAVE(CBdbgMgr::TraceDestructor) ;
    RETURN(iError != 0) ;
}

//* CBdbgMgr::DebugMessageBox -- 
PUBLIC BOOL CBdbgMgr::DebugMessageBox(LPCSTR lpszPrompt,
	UINT nType, UINT nIDPrompt)
// lpszPrompt -- message string, starting with '~'
// nType, nIDPrompt -- integer sprintf substitutions
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CBdbgMgr::DebugMessageBox) ;
    int iError = 0 ;		// error code
    BOOL bDone = FALSE ;
    char szStr[100] ;

    if (lpszPrompt[1] == '~')
	switch (lpszPrompt[2])
	{
	    case 'C':
		TraceConstructor(lpszPrompt+3,
			(LPVOID)MAKELONG(nType, nIDPrompt)) ;
		bDone = TRUE ;
		break ;

	    case 'D':
		TraceDestructor(lpszPrompt+3,
			(LPVOID)MAKELONG(nType, nIDPrompt)) ;
		bDone = TRUE ;
		break ;

	    default:
		break ;
	}


    if (!bDone)
    {
	_snprintf(szStr, sizeof(szStr)-1, lpszPrompt + 1,
    		nType, nIDPrompt) ;
	JXOutputDebugString(szStr) ;
    }

// cleanup:

    JXELEAVE(CBdbgMgr::DebugMessageBox) ;
    RETURN(iError != 0) ;
}

//* CBdbgMgr::AddTraceObject -- add object to trace list
PUBLIC BOOL CBdbgMgr::AddTraceObject(LPCSTR lpszName, LPVOID lpPtr)
// lpszName -- name of object
// lpPtr -- pointer to object
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CBdbgMgr::AddTraceObject) ;
    int iError = 0 ;		// error code
    int iK ;		// loop variable
    int iPosition = -1 ;	// free position in array
    char szStr[100] ;

    if (!lpPtr)
    {
	_snprintf(szStr, sizeof(szStr) - 1,
			"AddTraceObject error: %Fs %#04x:%#04x "
			"invalid pointer.\n", lpszName,
			HIWORD(lpPtr), LOWORD(lpPtr)) ;
	JXOutputDebugString(szStr) ;
	++m_iErrorCount ;
	iError = 100 ;
	goto cleanup ;
    }

    if (lpPtr && m_lpTraceObjects)
    {
	for (iK = 0 ; iK < m_iTraceObjectCurrent ; ++iK)
	    if (!m_lpTraceObjects[iK])
		iPosition = iK ;
	    else if (m_lpTraceObjects[iK] == lpPtr)
	    {
		_snprintf(szStr, sizeof(szStr) - 1,
			"AddTraceObject error: %Fs %#04x:%#04x "
			"already in array.\n", lpszName,
			HIWORD(lpPtr), LOWORD(lpPtr)) ;
		JXOutputDebugString(szStr) ;
		++m_iErrorCount ;
		iError = 101 ;
		goto cleanup ;
	    }

	if (iPosition < 0 && m_iTraceObjectCurrent
			>= m_iTraceObjectCount - 1)
	{
	    JXOutputDebugString("AddTraceObject error: "
				"Array overflow.\n") ;
	    ++m_iErrorCount ;
	    iError = 102 ;
	    goto cleanup ;
	}

	if (iPosition < 0)
	    iPosition = m_iTraceObjectCurrent++ ;

	m_lpTraceObjects[iPosition] = lpPtr ;
    }

cleanup:

    JXELEAVE(CBdbgMgr::AddTraceObject) ;
    RETURN(iError != 0) ;
}

//* CBdbgMgr::TestTraceObject -- test 
PUBLIC BOOL CBdbgMgr::TestTraceObject(LPCSTR lpszName, LPVOID lpPtr,
    			BOOL bMissing PDFT(FALSE))
// lpszName -- name of object
// lpPtr -- pointer to object
// bMissing -- if TRUE, then correct if pointer is missing;
//		if FALSE, then correct if pointer is present -- in
//		case of array overflow, this test can't be made
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CBdbgMgr::TestTraceObject) ;
    int iError = 0 ;		// error code
    int iK ;		// loop variable
    int iPosition = -1 ;
    char szStr[100] ;

    if (!lpPtr)
    {
	_snprintf(szStr, sizeof(szStr) - 1,
			"TestTraceObject error: %Fs %#04x:%#04x "
			"invalid pointer.\n", lpszName,
			HIWORD(lpPtr), LOWORD(lpPtr)) ;
	JXOutputDebugString(szStr) ;
	++m_iErrorCount ;
	iError = 100 ;
	goto cleanup ;
    }

    if (lpPtr && m_lpTraceObjects)
    {
	for (iK = 0 ; iPosition < 0
			&& iK < m_iTraceObjectCurrent ; ++iK)
	    if (m_lpTraceObjects[iK] == lpPtr)
		iPosition = iK ;

	if (bMissing && iPosition >= 0)
	{
	    _snprintf(szStr, sizeof(szStr) - 1,
			"TestTraceObject error: %Fs %#04x:%#04x "
			"already in array.\n", lpszName,
			HIWORD(lpPtr), LOWORD(lpPtr)) ;
	    JXOutputDebugString(szStr) ;
	    ++m_iErrorCount ;
	    iError = 101 ;
	    goto cleanup ;
	}

	if (!bMissing && iPosition < 0
		&& m_iTraceObjectCurrent < m_iTraceObjectCount - 1)
	{
	    _snprintf(szStr, sizeof(szStr) - 1,
			"TestTraceObject error: %Fs %#04x:%#04x "
			"is not in array.\n", lpszName,
			HIWORD(lpPtr), LOWORD(lpPtr)) ;
	    JXOutputDebugString(szStr) ;
	    ++m_iErrorCount ;
	    iError = 102 ;
	    goto cleanup ;
	}
    }

cleanup:

    JXELEAVE(CBdbgMgr::TestTraceObject) ;
    RETURN(iError != 0) ;
}

//* CBdbgMgr::RemoveTraceObject -- add object to trace list
PUBLIC BOOL CBdbgMgr::RemoveTraceObject(LPCSTR lpszName, LPVOID lpPtr)
// lpszName -- name of object
// lpPtr -- pointer to object
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CBdbgMgr::RemoveTraceObject) ;
    int iError = 0 ;		// error code
    int iK ;		// loop variable
    int iPosition = -1 ;
    char szStr[100] ;

    if (!lpPtr)
    {
	_snprintf(szStr, sizeof(szStr) - 1,
			"RemoveTraceObject error: %Fs %#04x:%#04x "
			"invalid pointer.\n", lpszName,
			HIWORD(lpPtr), LOWORD(lpPtr)) ;
	JXOutputDebugString(szStr) ;
	++m_iErrorCount ;
	iError = 100 ;
	goto cleanup ;
    }

    if (lpPtr && m_lpTraceObjects)
    {
	for (iK = 0 ; iPosition < 0
			&& iK < m_iTraceObjectCurrent ; ++iK)
	    if (m_lpTraceObjects[iK] == lpPtr)
		iPosition = iK ;

	if (iPosition >= 0)
	    m_lpTraceObjects[iPosition] = NULL ;

	else if (m_iTraceObjectCurrent < m_iTraceObjectCount - 1)
	{
	    _snprintf(szStr, sizeof(szStr) - 1,
			"RemoveTraceObject error: %Fs %#04x:%#04x "
			"is not in array.\n", lpszName,
			HIWORD(lpPtr), LOWORD(lpPtr)) ;
	    JXOutputDebugString(szStr) ;
	    ++m_iErrorCount ;
	    iError = 101 ;
	    goto cleanup ;
	}
    }

cleanup:

// cleanup:

    JXELEAVE(CBdbgMgr::RemoveTraceObject) ;
    RETURN(iError != 0) ;
}

//* CBdbgMgr::ReportTraceObjects -- final report at program end
PUBLIC BOOL CBdbgMgr::ReportTraceObjects(void)
//// int FAR PASCAL CBdbgMgr::ReportTraceObjects(void)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CBdbgMgr::ReportTraceObjects) ;
    int iError = 0 ;		// error code
    int iUnfreed = 0 ;		// number of unfreed objects
    LPVOID lpPtr ;
    int iK ;		// loop variable
    char szStr[100] ;

    if (m_lpTraceObjects)
    {
	JXOutputDebugString("\nCBdbgMgr final report:\n") ;

	if (m_iErrorCount)
	{
	    _snprintf(szStr, sizeof(szStr) - 1,
		"   Previous errors: %d\n", m_iErrorCount) ;
	    JXOutputDebugString(szStr) ;
	}

	for (iK = 0 ; iK < m_iTraceObjectCurrent ; ++iK)
	    if ( (lpPtr = m_lpTraceObjects[iK]) )
	    {
		_snprintf(szStr, sizeof(szStr) - 1,
			"    Unfreed object: %#04x:%#04x.\n",
			HIWORD(lpPtr), LOWORD(lpPtr)) ;
		JXOutputDebugString(szStr) ;
		++iUnfreed ;
	    }

	if (iUnfreed)
	{
	    _snprintf(szStr, sizeof(szStr) - 1,
			"    Total unfreed objects: %d.\n",
    			iUnfreed) ;
	    JXOutputDebugString(szStr) ;
	}

	if (!m_iErrorCount && !iUnfreed)
	    JXOutputDebugString("    No errors encountered.\n") ;
    }

// cleanup:

    JXELEAVE(CBdbgMgr::ReportTraceObjects) ;
    RETURN(iError != 0) ;
}

//* CBdbgMgr::OutputWithTime -- output debugging string with time
PUBLIC STATIC BOOL CBdbgMgr::OutputWithTime(LPSTR lpszPattern)
// lpszPattern -- sprintf string containing %s for time substitution
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CBdbgMgr::OutputWithTime) ;
    int iError = 0 ;		// error code
    char szMsg[100] ;

    struct tm *newtime;
    time_t aclock;

    time( &aclock );                 /* Get time in seconds */
    newtime = localtime( &aclock );

    _snprintf(szMsg, sizeof(szMsg)-1, lpszPattern, asctime(newtime)) ;
    JXOutputDebugString(szMsg) ;

// cleanup:

    JXELEAVE(CBdbgMgr::OutputWithTime) ;
    RETURN(iError != 0) ;
}


//* CBdbgMgr::OutputWithWordWrap -- output debugging string with time
PUBLIC STATIC BOOL CBdbgMgr::OutputWithWordWrap(LPSTR lpStr1,
			LPSTR lpStr2, int iIndent)
// lpStr1, lpStr2 -- strings to be concatenated and outputted, with
//		the second one (only) processed for word wrap
// iIndent -- # blanks to indent after first line
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CBdbgMgr::OutputWithWordWrap) ;
    int iError = 0 ;		// error code
    char szMsg[100] ;
    BOOL bSecond = FALSE ;
    int iK = 0, iBlank ;		// index variables
    XPSTR xpMsg = szMsg ;
    LPSTR lpStr = lpStr1, lpBlank = NULL ;
    char cChar ;

    while ( (cChar = *lpStr++) || !bSecond)
	if (!cChar)
	    lpStr = lpStr2, lpBlank = NULL, bSecond = TRUE ;

	else
	{
	    szMsg[iK++] = cChar ;

	    if (cChar == ' ')
		lpBlank = lpStr, iBlank = iK ;

	    if (iK >= 80)
	    {
		if (lpBlank)
//		    lpStr = lpStr, iK = iBlank - 1 ;
			lpStr = lpBlank, iK = iBlank - 1;
		szMsg[iK++] = '\n', szMsg[iK] = 0 ;
		
		JXOutputDebugString(szMsg) ;
		for (iK = 0 ; iK < iIndent ; )
		    szMsg[iK++] = ' ' ;
	    }
	}

    if (iK > 0)
    {
	szMsg[iK] = 0 ;
	JXOutputDebugString(szMsg) ;
    }

// cleanup:

    JXELEAVE(CBdbgMgr::OutputWithWordWrap) ;
    RETURN(iError != 0) ;
}



