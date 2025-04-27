// gtldcp.cpp -- decompiler for meta game
// Written by John J. Xenakis for Boffo Games Inc., 1994

#ifdef NODEEDIT

#include "stdafx.h"

#include <string.h>
#include <stdio.h>

#include "bgen.h"
#include "gtldat.h"


//* CGtlData::Decompile -- output data to .GTL file
int CGtlData::Decompile(const char * xpszPathName)
// xpszPathName -- target save file name
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::Decompile) ;
    int iError = 0 ;		// error code

    int iK, iL ;		// loop variables
    int iLink ;		// link variable
    CMap FAR * lpMap ;	// pointer to bitmap object
    CNode FAR * lpNode ;	// pointer to node object
    char szLabel[MAX_LABEL_LENGTH] ;	// node/bitmap label
    char szOut[200] ;   // output string
    XPSTR xpIn, xpOut ;		// input/output string point
    BOOL bPrevLink, bNextLink, bBackLink, bForwardLink ;

    strncpy(m_szGtlFile, xpszPathName, sizeof(m_szGtlFile)-1) ;

    if ((m_xpGtlFile = fopen(m_szGtlFile, "w")) == NULL) {

        sprintf(szOut, "\n\nCan't open output file %s.\n\n", m_szGtlFile);
        goto cleanup ;
    }

    sprintf(szOut, "// %s\n", m_szGtlFile) ;
    AsciiOutput(0, szOut) ;

    if (m_szBmpDirectory[0])		// if there's a bitmap directory
    {
	strcpy(szOut, "\tDIRECTORY\t\"") ;
	xpIn = m_szBmpDirectory, xpOut = szOut + strlen(szOut) ;
    while ((*xpOut = *xpIn++) != '\0')
	    if (*xpOut++ == '\\')
            *xpOut++ = '\\';
	strcat(szOut, "\"\t// Bitmap directory\n") ;
//	sprintf(szOut, "\tDIRECTORY\t%s	// Bitmap directory\n",
//		m_szBmpDirectory) ;
	AsciiOutput(0, szOut) ;
    }

    AsciiOutput(0, "// Bitmaps") ;

    for (iK = 0 ; iK < m_iMaps ; ++iK)
    {
	lpMap = m_lpMaps + iK ;
	szOut[0] = 0 ;		// initialize output string

	if (lpMap->m_szLabel[0])	// if there's a label
	    sprintf(szOut, "%Fs:", (LPSTR)lpMap->m_szLabel) ;

	sprintf(szOut + strlen(szOut), "\tBITMAP\t\"%Fs\"",
		(LPSTR)lpMap->m_szFilename) ;

    if (lpMap->m_iRelationType) {

	    strcat(szOut, " ") ;
	    strcat(szOut, FindKeyword(lpMap->m_iRelationType)) ;
	    strcat(szOut, " ") ;
	    if (lpMap->m_iRelationType == KT_NODE)
		_fstrcat(szOut,
			(LPSTR)m_lpNodes[lpMap->m_iRelation].m_szLabel) ;

	    else
		_fstrcat(szOut,
		    (lpMap->m_iRelation == iK - 1 ? (LPSTR)"PREVIOUS" :
			(LPSTR)m_lpMaps[lpMap->m_iRelation].m_szLabel)) ;
	}

	if (lpMap->m_bPalette)
	    strcat(szOut, " PALETTE") ;

	if (lpMap->m_bSprite)
	    strcat(szOut, " SPRITE") ;

	if (lpMap->m_bOverlay)
	    strcat(szOut, " OVERLAY") ;

	if (lpMap->m_bMasked)
	    strcat(szOut, " MASKED") ;

	if (lpMap->m_bMetaGame)
	    strcat(szOut, " METAGAME") ;

	if (!lpMap->m_bRelocatable)
	    strcat(szOut, " BORDER") ;

    // BCW: added SIZE(x,y) to .GTL format
    //
    sprintf(szOut + strlen(szOut), " SIZE(%d,%d)", lpMap->m_lpcBgbObject->m_cSize.cx, lpMap->m_lpcBgbObject->m_cSize.cy);

    if (lpMap->m_bPositionDetermined) {

	    strcat(szOut, "\t") ;
	    if (!lpMap->m_bPositionSpecified)
		strcat(szOut, "// ") ;
	    sprintf(szOut + strlen(szOut), "POSITION(%d,%d,%d,%d)",
		lpMap->m_lpcBgbObject->m_crPosition.x,
    		lpMap->m_lpcBgbObject->m_crPosition.y,
		lpMap->m_lpcBgbObject->m_cSize.cx,
    		lpMap->m_lpcBgbObject->m_cSize.cy) ;
	}

	AsciiOutput(0, szOut) ;
    }

    AsciiOutput(0, "\n// Nodes\n") ;

    for (iK = 0 ; iK < m_iNodes ; ++iK)
    if (!(lpNode = m_lpNodes + iK)->m_bDeleted) {

	    // analyze links for this block
	    bPrevLink = bNextLink = bBackLink = bForwardLink = FALSE ;
	    for (iL = 0 ; iL < lpNode->m_iNumLinks ; ++iL)
        if ( (iLink = lpNode->m_iLinks[iL]) != iK) {

		    if (iLink == iK - 1)
                bPrevLink = TRUE ;
		    else if (iLink == iK + 1)
                bNextLink = TRUE ;
		    else if (iLink < iK)
                bBackLink = TRUE ;
		    else if (iLink > iK)
                bForwardLink = TRUE ;
		}

        // if this is a generated label
	    if (lpNode->m_szLabel[0] == CCHDOT)
            lpNode->m_szLabel[0] = 0 ;  // delete it

        // if there's a forward link
        if (!lpNode->m_szLabel[0] && bForwardLink) {

            //sprintf(szLabel, ".%d", m_iNumGenLabels++);
            sprintf(szLabel, ".n%d", iK) ;
			// generate a new label
            _fstrcpy(lpNode->m_szLabel, szLabel) ;
	    }

	    szOut[0] = 0 ;		// init output string
	    if (lpNode->m_szLabel[0])	// if there's a label
            sprintf(szOut, "%Fs:", (LPSTR)lpNode->m_szLabel) ;
	    strcat(szOut, "\tNODE\t") ;

	    if (lpNode->m_bRelative)
            sprintf(szOut + strlen(szOut), "BITMAP %Fs", (LPSTR)m_lpMaps[lpNode->m_iBitmap].m_szLabel) ;
	    else
            strcat(szOut, "POSITION") ;

        sprintf(szOut + strlen(szOut), " (%d,%d)", lpNode->m_iX, lpNode->m_iY);

	    if (bPrevLink)	// if linked to previous node
            strcat(szOut, " LINKPREV") ;

	    if (!lpNode->m_bRelocatable)
            strcat(szOut, " BORDER") ;

        if (lpNode->m_iWeight != 1) {
            if (lpNode->m_iWeight == 0)
                strcat(szOut, " PASSTHRU") ;
            else
                sprintf(szOut + strlen(szOut), " WEIGHT %d", lpNode->m_iWeight) ;
	    }

//	    if (lpNode->m_iSector)
//		sprintf(szOut + strlen(szOut), " SECTOR %d",
//				lpNode->m_iSector) ;

	    if (lpNode->m_bSenSpec)
            sprintf(szOut + strlen(szOut), " SENSITIVITY %d",
				lpNode->m_iSensitivity) ;

	    if (lpNode->m_szSector[0])	// if there's an action
            sprintf(szOut + strlen(szOut),
			" SECTOR %Fs", (LPSTR)lpNode->m_szSector) ;

	    AsciiOutput(0, szOut) ;

	    if (bBackLink)	// if there's a back link to a node
			// prior to the previous node
		for (iL = 0 ; iL < lpNode->m_iNumLinks ; ++iL)
		    if ( (iLink = lpNode->m_iLinks[iL]) >= 0 
				    && iLink < iK-1)
		    {
			sprintf(szOut, "\tLINK\t%Fs",
				(LPSTR)m_lpNodes[iLink].m_szLabel) ;
			AsciiOutput(0, szOut) ;
		    }
	}

    AsciiOutput(0, "\n\tEND") ;

cleanup:

    if (m_xpGtlFile)              // if there's an open output file
        fclose(m_xpGtlFile) ;     // close it

    JXELEAVE(CGtlData::Decompile) ;
    RETURN(iError != 0) ;
}


//* CGtlData::AsciiOutput -- ascii output
int CGtlData::AsciiOutput(int iIndent, XPSTR lpszOut)
// iIndent -- # spaces to indent, or -1 to continue on same line
// lpszOut -- output string
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::AsciiOutput) ;
    int iError = 0 ;		// error code
    char szBlanks[100] ;

    if (iIndent >= 0)           // if we're starting new line
    {
        memset (szBlanks, ' ', sizeof(szBlanks)) ;	// blank it out
        szBlanks[0] = '\n' ;	// new line

        if (iIndent > sizeof(szBlanks) - 1)
            iIndent = sizeof(szBlanks) - 1 ;

        szBlanks[iIndent + 1] = 0 ;	// null out
        fprintf(m_xpGtlFile, szBlanks) ;	// new line, indent
    }

    fprintf(m_xpGtlFile, lpszOut) ;	// output text
    JXELEAVE(CGtlData::AsciiOutput) ;
    RETURN(iError != 0) ;
}
#endif
