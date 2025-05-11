// gtllex -- lexical analysis for graphics utility
// Written by John J. Xenakis for Boffo Games Inc., 1994

#include "stdafx.h"

#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys\timeb.h>

#include "bgen.h"
#include "gtldat.h"

///DEFS gtldat.h

static char st_szIdentChars[] =
	    "abcdefghijklmnopqrstuvwxyz"
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	    "0123456789_." ;

static CKeyTab st_cKytbs[] = {
	{KT_BMP, "BITMAP"},
	{KT_NODE, "NODE"},
	{KT_LINK, "LINK"},
	{KT_ABOVE, "ABOVE"},
	{KT_BELOW, "BELOW"},
	{KT_LEFT, "LEFT"},
	{KT_RIGHT, "RIGHT"},
	{KT_PALETTE, "PALETTE"},
	{KT_OVERLAY, "OVERLAY"},
	{KT_MASKED, "MASKED"},
	{KT_METAGAME, "METAGAME"},
	{KT_SPRITE, "SPRITE"},
	{KT_PASSTHRU, "PASSTHRU"},
	{KT_WEIGHT, "WEIGHT"},
	{KT_SENSITIVITY, "SENSITIVITY"},
	{KT_SECTOR, "SECTOR"},
	{KT_BORDER, "BORDER"},
	{KT_PREVIOUS, "PREVIOUS"},
	{KT_LINKPREV, "LINKPREV"},
	{KT_POSITION, "POSITION"},
	{KT_DIRECTORY, "DIRECTORY"},
    {KT_FOREGROUND, "FOREGROUND"},
    {KT_BACKGROUND, "BACKGROUND"},
    {KT_ANIMATED, "ANIMATED"},
    {KT_SIZE, "SIZE"},
    {KT_MIDDLE, "MIDDLE"},
    {KT_FREQUENCY, "FREQ"},
	{KT_EOF, "END"},
	{0, "????"}} ;

//* CGtlData::FindKeyword -- find keyword, given tree node type
XPSTR CGtlData::FindKeyword(int iType)
// iType -- KT_xxxx -- tree node type
// returns: string pointer to keyword, or to "????"
{
    int iError = 0 ;		// error code
    CKeyTab * xpKytb ;	// ptr to keyword table entry

    JXENTER(CGtlData::FindKeyword) ;
    for (xpKytb = &st_cKytbs[0] ; xpKytb->m_iKeyValue &&
    			xpKytb->m_iKeyValue != iType ; ++xpKytb)
	;

    JXELEAVE(CGtlData::FindKeyword) ;
    return(xpKytb->m_xpszKeyString) ;	// return string point
}

//* CGtlData::ReadLine -- read input line
BOOL CGtlData::ReadLine(void)
// returns: TRUE if error, FALSE otherwise
{
    int iError = 0 ;		// error code
    char szLine[512] ;		// input line
    XPSTR xpStart ;       // ptr to first char of element
    int iElementLength ;	// length of current lexical element
    int iStringListPos = 0 ;	// current position in string list
    int iNumLxels = 0 ;		// number of lexical elements so far
    XPSTR xpStr ;		// string pointer
    XPSTR xpszIdent ;		// identifier string pointer
    CKeyTab * xpKytb ;		// pointer to keyword table
    CLexElement * xpLxel, * xpLastLxel = NULL ;	// last lexical element on line
    BOOL bDone ;	// loop variable

    JXENTER(CGtlData::ReadLine) ;

    // zero out the lexical blocks
    memset((void *)&m_cLexElts[0], 0, MAX_LEXELTS * sizeof (class CLexElement));

    bDone = FALSE;
    while (!bDone) {

        // read line and test for EOF
        if (!fgets(szLine, sizeof(szLine), m_xpGtlFile)) {
            bDone = TRUE ;  // last loop iteration
            iError = 100 ;
            m_bEof = TRUE ; // end of file
            szLine[0] = 0 ; // process null line
        }

        ++m_iLineNumber ;   // increment line number

        if (m_xpListFile)       // if there's a listing file
            fputs(szLine, m_xpListFile) ;   // list input

        xpStart = szLine;
        for (xpStr = szLine ; *xpStr ; ) {

            xpLxel = &m_cLexElts[iNumLxels] ;   // point to current lexical block
            xpLxel->m_iLineNumber = m_iLineNumber ;
            xpLxel->m_iColumn = xpStr - xpStart;

            // test for blank, tab, line feed (newline)
            //  or carriage return
            if ((*xpStr == CCHBL) || (*xpStr == CCHTAB)
                    || (*xpStr == CCHLF) || (*xpStr == CCHCR))
            ++xpStr ;       // just skip it

            else if ((*xpStr >= '0') && (*xpStr <= '9')) {
                xpLxel->m_iType = LXT_INTEGER ;

                // loop thru digits, adding to value
                while (*xpStr >= '0' && *xpStr <= '9')
                    xpLxel->m_iVal = 10 * xpLxel->m_iVal + *xpStr++ - '0' ;

            // it's an identifier
            } else if ( (iElementLength = strspn(xpStr, st_szIdentChars)) > 0) {

                xpLxel->m_iType = LXT_IDENT ;
                xpLxel->m_iStringListPos = iStringListPos ;

                // store position in string list
                if (iStringListPos + iElementLength + 2 > MAX_STRINGLIST) {
                    iError = 100 ;  // string list overflow
                    CGtlData::ErrorMsg(xpLxel, "ReadLine - String List overflow");
                    goto cleanup ;
                }

                // copy string into string list
                strncpy( (xpszIdent = &m_szStringList[iStringListPos]), xpStr, iElementLength) ;

                iStringListPos += iElementLength ;  // new position

                xpStr += iElementLength ;           // skip input string

                // null terminator on string
                m_szStringList[iStringListPos++] = 0 ;

                for (xpKytb = &st_cKytbs[0] ; xpKytb->m_iKeyValue &&
                    stricmp(xpKytb->m_xpszKeyString, xpszIdent)
                            != 0 ;  ++xpKytb)
                    ;   // null loop body

                // copy keyword value (zero if none)
                xpLxel->m_iVal = xpKytb->m_iKeyValue ;

            // check for string beginning with quote char
            //
            } else if (*xpStr == CCHQUO) {

                // store position in string list
                xpLxel->m_iType = LXT_STRING ;
                xpLxel->m_iStringListPos = iStringListPos ;

                // find closing quote
                while (*++xpStr && *xpStr != CCHQUO) {
                    if (*xpStr == '\\')
                        ++xpStr ;
                    m_szStringList[iStringListPos++] = *xpStr ;
                    if (iStringListPos + 5 > MAX_STRINGLIST) {
                        iError = 101 ;  // string list overflow
                        CGtlData::ErrorMsg(xpLxel, "ReadLine - String List overflow");
                        goto cleanup ;
                    }
                }
                // if no closing quote
                if (*xpStr++ != CCHQUO) {
                    iError = 102 ;  // unterminated quote
                    CGtlData::ErrorMsg(xpLxel, "ReadLine - Unterminated quoted string");
                    goto cleanup ;
                }
                m_szStringList[iStringListPos++] = 0 ;

            // comment
            //
            } else if (*xpStr == '/' && *(xpStr+1) == '/') {
                *xpStr = 0 ;    // insert null, loop will terminate

            } else if (strchr(",()=+-;:", *xpStr)) {

                xpLxel->m_iType = (*xpStr == ',') ? LXT_COMMA :
                    (*xpStr == '(') ? LXT_LPR :
                    (*xpStr == ')') ? LXT_RPR :
                    (*xpStr == '=') ? LXT_EQU :
                    (*xpStr == '+') ? LXT_PLUS :
                    (*xpStr == '-') ? LXT_MINUS :
                    (*xpStr == ';') ? LXT_SEMIC :
                    (*xpStr == ':') ? LXT_COLON : 0 ;
                xpLxel->m_iVal = *xpStr++ ; // save char as val

            // char not recognized
            //
            } else {

                iError = 103 ;
                CGtlData::ErrorMsg(xpLxel, "Invalid char in input") ;
                goto cleanup ;
            }

            if (xpLxel->m_iType)        // if we used this lex element
                xpLastLxel = xpLxel, ++iNumLxels ; // skip to next one
        }

        // we're done if there was at least one lexical element,
        //  and the last one is not a comma
        if (xpLastLxel && xpLastLxel->m_iType != LXT_COMMA)
            bDone = TRUE ;
    }

    // end of statement
    m_cLexElts[iNumLxels].m_iType = LXT_EOX ;

cleanup:

    JXELEAVE(CGtlData::ReadLine) ;
    RETURN(iError != 0) ;
}


//* CGtlData::ErrorMsg -- publish error message
BOOL CGtlData::ErrorMsg(CLexElement * xpLxel, XPSTR szMessage)
// returns: TRUE if error, FALSE otherwise
{
    int iError = 0 ;		// error code
    char szLine[200] ;		// output line
    char szTitle[40] ;		// message box title

    JXENTER(CGtlData::ErrorMsg) ;
    Common::sprintf_s(szLine, "Error: %s.", szMessage) ;
    //printf(szLine) ;

    if (xpLxel)
    {
	Common::sprintf_s(szTitle, "Error in line %i, column %i.",
		    xpLxel->m_iLineNumber, xpLxel->m_iColumn) ;
	// printf(szLine) ;
    }
    else
	strcpy(szTitle, "Error message") ;
    ::MessageBox(NULL, szLine, szTitle, MB_OK) ;

// cleanup:

    JXELEAVE(CGtlData::ErrorMsg) ;
    RETURN(iError != 0) ;
}
