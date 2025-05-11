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

#ifndef HODJNPODJ_METAGAME_GTL_RESOURCE_H
#define HODJNPODJ_METAGAME_GTL_RESOURCE_H

//#include <sys\timeb.h>
#include "bagel/boflib/sound.h"

#include "bgen.h"
#include "bgenut.h"
#include "bsutl.h"
#include "gtlview.h"

#include "globals.h"
#include "resource.h"
#include "bgb.h"
#include "bbt.h"
#include "bfc.h"
#include "mgstat.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define MAX_NODES 550                   // max # nodes supported
#define MAX_LINKS_PER_NODE 6            // max # links for 1 node
#define MAX_FILENAME_LENGTH 60          // max .p filename length
#define MAX_TEXT_LENGTH 200             // max length of text
#define MAX_HANDLE_LENGTH 200           // max length of handle name
#define MAX_STRINGLIST 512              // max total length of all strings in line
#define MAX_LEXELTS 100                 // max # of CLexElement objects
#define MAX_LABEL_LENGTH 12             // max length of node/bitmap
                                        // label/action
#define MAX_BITMAPS 130

#define NODERADIUS 5            /* display radius of node */
#define NODESENSITIVITY 20      /* default node sensitivity */
#define MAX_GAME_TABLE 12       /* meta game execution table */
#define MAX_CLUES 100

#define NEWSCROLL 1

// defining distance between nodes
inline long SQ(int y) {return((long)((long)y * (long)y)) ;}
inline long SQ(long y) {return(y * y) ;}
inline long LSQ(CPoint p1, CPoint p2)
                {return(SQ(p1.x-p2.x)+SQ(p1.y-p2.y)) ;}

// lexical element types
#define LXT_INTEGER     102
#define LXT_STRING      103
#define LXT_IDENT       104
#define LXT_COMMA       106
#define LXT_EQU         107
#define LXT_LPR         108
#define LXT_RPR         109
#define LXT_PLUS        110
#define LXT_MINUS       111
#define LXT_SEMIC       112
#define LXT_COLON       113
#define LXT_EOX         120


// keyword codes
#define KT_BMP          201
#define KT_NODE         202
#define KT_LINK         203
#define KT_ABOVE        204
#define KT_BELOW        205
#define KT_LEFT         206
#define KT_RIGHT        207
#define KT_PALETTE      208
#define KT_SPRITE       209
#define KT_PASSTHRU     210
#define KT_PREVIOUS     211
#define KT_LINKPREV     212
#define KT_POSITION     213
#define KT_DIRECTORY    214
#define KT_OVERLAY      215
#define KT_MASKED       216
#define KT_METAGAME     217
#define KT_WEIGHT       218
#define KT_ACTION       219
#define KT_BORDER       220
#define KT_SECTOR       221
#define KT_SENSITIVITY  222
#define KT_FOREGROUND   223
#define KT_BACKGROUND   224
#define KT_ANIMATED     225
#define KT_SIZE         226
#define KT_MIDDLE       227
#define KT_FREQUENCY    228
#define KT_EOF          230

// object types
#define OBJ_NONE 0
#define OBJ_BITMAP 301
#define OBJ_NODE 302
#define OBJ_LINK 303

// buttons
#define BUTTON_MINIMAP          1
#define BUTTON_INVENTORY        2
#define BUTTON_SCROLL           3

// node types
#define NT_GAME_MIN 100         /* use BFCG_xxxx game code */
#define NT_GAME_MAX 200
#define NT_MENU 201             /* meta-game menu */
#define NT_OVERLAY 202          /* tied to overlay */

// codes for DumpGameStatus
#define DUMPSTAT_PROBLEM 0x8000
#define DUMPSTAT_CURRENT 0x4000
#define DUMPSTAT_BOTH    0x2000
#define DUMPSTAT_EVERYTHING 0xFFFF

class CMemDC {
    public:
        CMemDC(VOID);
        ~CMemDC(VOID);

        CDC      *m_pDC;
        CPalette *m_pPalOld;
        HBITMAP   m_hBmpOld;
};

// CLexElement -- lexical element class
class CLexElement {
    friend class CGtlData ;
    int m_iType ;         // lexical type -- LXT_xxxx
    int m_iVal ;          // integer or char or keyword code
    int m_iStringListPos ;      // string position
    int m_iLineNumber, m_iColumn ;      // for error messages
} ;

// CKeyTab -- keyword table element
class CKeyTab {
public:
    int m_iKeyValue ;   // keyword value
    XPSTR m_xpszKeyString ;     // string
} ;

// CMap -- bit map
class FAR CMap {
    friend class CGtlData ;

    char m_cStartData ;
    char m_szLabel[MAX_LABEL_LENGTH] ;  // bitmap label
    char m_szFilename[MAX_FILENAME_LENGTH] ;    // file name of BMP file
    BOOL m_bPositionDetermined:1 ;
    BOOL m_bSprite : 1 ;
    BOOL m_bPalette : 1 ;               // game palette from this bitmap
    BOOL m_bOverlay : 1 ;               // overlay onto background map
    BOOL m_bMasked : 1 ;                // bitmap is masked
    BOOL m_bMetaGame : 1 ;              // show bitmap only in metagame
    BOOL m_bRelocatable : 1 ;           // relocatable
    BOOL m_bPositionSpecified : 1 ;     // position specified on input
    BOOL m_bSpecialPaint: 1 ;           // don't automatically paint
    CBgbObject FAR * m_lpcBgbObject ;   // Boffo game object for bitmap file

    int m_iRelationType ;       // KT_ABOVE, KT_BELOW, KT_LEFT, KT_RIGHT,
                                // KT_NODE
    int m_iRelation ;           // index of relation bitmap or node
    char m_cEndData ;

    CMap::CMap(void) {_fmemset(&m_cStartData,
                        0, &m_cEndData - &m_cStartData) ;}
} ;

// CNode -- node on the map
class FAR CNode {

    public:
#ifdef NODEEDIT
        char m_szSector[MAX_LABEL_LENGTH] ; // node sector
#endif
        char m_szLabel[MAX_LABEL_LENGTH] ;  // node label
        int m_iLinks[MAX_LINKS_PER_NODE];   // up to 10 links per node
        int m_iLocationCode ;               // MG_LOC_xxxx to specify location
        int m_iSector ;                     // MG_SECTOR_xxxx to specify sector
        int m_iBitmap ;                     // index of related bitmap
        int m_iX, m_iY ;                    // position
        int m_iNumLinks ;                   // number of links
        BOOL m_bDeleted : 1 ;               // deleted node
        BOOL m_bSelected : 1 ;              // this is selected node
        BOOL m_bRelative : 1 ;              // relative to bitmap
        BOOL m_bRelocatable : 1 ;           // relocatable node
        BOOL m_bWgtSpec : 1 ;               // weight specified
        BOOL m_bSenSpec : 1 ;               // sensitivity specified
        BOOL m_bMenu : 1;                   // TRUE if menu
        UBYTE m_iWeight ;                   // node weight
        UBYTE m_iSensitivity ;              // distance to accept clickb

        CNode(void);

        BOOL IfRelocatable(void) {return (m_bRelocatable != 0) ; }
};

// CStratLocInfo -- strategy location information
class CStratLocInfo {
    friend class CGtlData ;
    friend class CStrategyInfo ;

    char m_cStartData ;
    int m_iLocCode ;            // MG_LOC_xxxx -- location code
    CNode FAR * m_lpNode ;      // node pointer for location
    int m_iValueCode ;          // MG_VISIT_xxxx or MG_WIN_xxxx
                                // or 0 if not eligible
    int m_iDistance ;           // distance to location
    int m_iWeight ;             // weight of location
    int m_iAdjustedWeight ;     // weight adjusted by distance

    char m_cEndData ;
    CStratLocInfo(void) {memset(&m_cStartData,
                        0, &m_cEndData - &m_cStartData) ;}
} ;


// CStrategyInfo -- strategy information for determining best move
class CStrategyInfo {
    friend class CGtlData ;

    char m_cStartData ;
    CStratLocInfo * xpTargetLocInfo ;   // target location (best move)

    int m_iRequiredObjectsCount ;   // total number of objects I need
    int m_iRequiredObjectsTable[MAX_GAME_TABLE] ; // objects I need

    int m_iMaximumDistance ;    // max distance to eligible location
    int m_iMaxAdjustedWeight ;  // maximum adjusted weight
    int m_iTopLocCount ;        // number of top locations
    int m_iTopLocTable[MG_LOC_MAX] ;    // table of top locations

    char m_cEndData ;

    CStratLocInfo m_cStratLocInfo[MG_LOC_MAX + 1] ;

    CStrategyInfo(void) {memset(&m_cStartData,
                        0, &m_cEndData - &m_cStartData) ;}
} ;

// CXodj -- structure for Hodj Podj
class CXodj {
    friend class CGtlData ;

public:

    char m_cStartData ;
    CXodj * m_xpXodjNext ;      // next in chain
    char m_szName[MAX_LABEL_LENGTH] ;   // name of character
    BOOL m_bHodj ;              // char is Hodj (not Podj)
    int m_iFurlongs ;           // number of furlongs available
    int m_iCharNode ;           // node where character is positioned

    int m_nTurns;               // number of turns remaining (not furlongs)

    BOOL m_bComputer ;          // played by computer
    CMap FAR * m_lpMap ;        // bitmap object
    CBgbObject FAR * m_lpcCharSprite ;  // character sprite bgb object
    CBgbObject FAR * m_lpcIcon ;        // icon for character

    CSound *m_pThemeSound;              // this character's MIDI theme song info
    char   *m_pszThemeFile;
    UINT    m_nThemeStart;
    UINT    m_nThemeEnd;

    CHAR   *m_pszStarsFile;             // cel strip for stars animation

    // fields for computing computer play (also used for human player
    // for debugging or giving hints)
    class CStrategyInfo * m_xpStrategyInfo ; // strategy information

    // list of clue numbers for clues given by winning mini-game
    int m_iWinInfoWon ;     // how many clues I've received
    int m_iWinInfoNeed ;    // total number of clues I need
    int m_iWinInfoTable[MAX_GAME_TABLE] ;

    // list of clue numbers for clues given by farmer, etc.
    int m_iSecondaryInfoWon ;   // how many clues I've received
    int m_iSecondaryInfoNeed ;  // total number of clues I need
    int m_iSecondaryInfoTable[MAX_GAME_TABLE] ;

    // list of objects required to get Mish/Mosh
    int m_iRequiredObjectsCount ;   // total number of objects I need
    int m_iRequiredObjectsTable[MAX_GAME_TABLE] ; // objects I need
    int m_iRequiredMoney ;      // money needed for Mish/Mosh

    // list of secondary information location we still have to visit
    int m_iSecondaryLoc[MAX_GAME_TABLE] ;

    int m_iGameHistory[20] ;    // last 20 mini-games played
    int m_iTargetLocation ;     // target location for computer play
    BOOL m_bGatherInformation ; // if FALSE, data from last
                                // GatherInformation is still valid

    CInventory *m_pInventory ;   // items in personal inventory
    CInventory *m_pGenStore ;    // items in general store
    CInventory *m_pBlackMarket ;    // items in pawn shop
    CInventory *m_pTradingPost ;    // items in pawn shop

    char m_cEndData ;

    CXodj(void) {_fmemset(&m_cStartData, 0, &m_cEndData - &m_cStartData);}
    ~CXodj(void);
} ;

// CGtlData -- data class for graphics utility
class CGtlData {
public:
    class CGtlDoc *m_xpcGtlDoc; // document pointer

private:
    CGenUtil m_cGenUtil ;       // general utility object
    CBgbMgr m_cBgbMgr ;         // Boffo Game Object manager
    CBbtMgr m_cBbtMgr ;         // Boffo Games button manager
    CBbutton m_cMiniButton ;    // minimap button
    CBbutton m_cInvButton ;     // inventory button
    CBbutton m_cScrollButton ;  // scroll button

    CMap FAR * m_lpMaps /* [MAX_BITMAPS] */ ; // bitmap array
    CNode FAR * m_lpNodes /* [MAX_NODES] */ ;     // node array

    char m_cStartData ;
    BOOL m_bGtlDll ;            // equals GTLDLL define
    char m_szBmpDirectory[MAX_FILENAME_LENGTH] ; // bitmap file director
    char m_szGtlFile[MAX_FILENAME_LENGTH] ; // gtl file name
    char m_szListFile[MAX_FILENAME_LENGTH] ; // List file name
    FILE * m_xpGtlFile ;        // ptr to file structure for .GTL file
    FILE * m_xpListFile ;       // ptr to file structure for .LST file
    BOOL m_bListing ;           // listing file flag
    int m_iIndent ;             // current indent (decompile only)
    int m_iLineNumber ;         // line number in input file
    int m_iMaps, m_iNodes ;     // # of bitmaps, nodes
    BOOL m_bEof ;               // end of file on input (or END stmt)
    CNode FAR * m_lpLastNode ;  // pointer to last node
    int m_iNumGenLabels ;               // number of generated lables
    CLexElement * m_xpLexLabel ;        // current label lexeme
    CLexElement m_cLexElts[MAX_LEXELTS] ;       // lexical element blocks
    char m_szStringList[MAX_STRINGLIST] ;     // compiler input string
    BOOL m_bSelectedLink ;      // flag: a link is selected
    int m_iSelLinkPt1, m_iSelLinkPt2 ;  // indexes of selected link pts
    struct _timeb m_stAcceptClickActive ;       // prevent recursion
    struct _timeb m_stLDownTime ;       // time of left button down
    CNode FAR * m_lpFoundNode ; // node clicked on
    CNode FAR * m_lpLastSelectedNode ;  // selected on click down

    BOOL m_bJustPlayedMiniGame; // true for Mini-Games in ProcessGameResult

    // fields for Draw routine
public:
    BOOL m_bInhibitDraw ;       // inhibit any actual drawing
    BOOL m_bDrawing ;           // in Draw routine (prevent recursion)
    CGtlView * m_xpGtlView ;    // current view
    BOOL m_bPaint ;             // WM_PAINT flag
    BOOL m_bRelocatable ;       // device context is currently
                                // relocatable
    CRect * m_xpClipRect ;      // relocatable clipping rectangle
    CRect * m_xpDevClipRect ;   // device (non-relocatable)
                                // clipping rectangle
    CMap FAR * m_lpFurlongMaps[25] ; // 0-24 furlong bitmaps

// ---
public:
    int m_iSizeX, m_iSizeY ;    // size of entire client area
                                // (entire bitmap)
    int m_iMargin ;             // size of top margin of view window
    BOOL m_bChangeSize ;        // size of window changed (because
                                // gtl modified the bitmap template)
    int m_iViewSizeX, m_iViewSizeY ;    // size of view

    BOOL m_bPaintBackground ;   // paint background
    BOOL m_bShowNodes ;         // show nodes and links
    BOOL m_bShowSensitivity ;           // show node sensitivities
    BOOL m_bStartMetaGame ;     // start in meta game mode
    BOOL m_bMetaGame ;          // meta game mode
    BOOL m_bInitMetaGame ;      // init sprites at beginning of metagame

    CXodj * m_xpXodjChain ;     // chain of characters
    CXodj * m_xpCurXodj ;       // character currently on the move
    BOOL m_bSamePlace ;         // both characters are in same place
    BOOL m_bGameOver ;          // game is over

    CBitmap  *m_pOffScreenBmp;
    CPalette *m_pPalOld;
    HBITMAP   m_hOldBitmap;

    int m_iMishMoshLoc ;        // MG_LOC_xxxx -- Mish/Mosh location

    COLORREF m_cNodePenColor, m_cNodeBrushColor, m_cLinkPenColor ;
    COLORREF m_cNodeSensitivityColor ;
    COLORREF m_cNodePassThruColor, m_cNodeHeavyColor ;
    COLORREF m_cSelNodePenColor, m_cSelNodeBrushColor ;
    COLORREF m_cSelLinkPenColor ;
    COLORREF m_cBackColor ;

    char m_cEndData ;

    // constructor zeroes out all fields and allocates far arrays
public:
    void ClearData(void) {memset(&m_cStartData,
                        0, &m_cEndData - &m_cStartData) ;}

// gtldcp.cpp -- decompiler for meta game

//- Decompile -- output data to .GTL file
public: int Decompile(const char * xpszPathName) ;
//- AsciiOutput -- ascii output
private: int AsciiOutput(int iIndent, XPSTR lpszOut) ;
//- ListingOutput -- listing output
private: int ListingOutput(int iIndent, XPSTR lpszOut) ;


// gtlcpl.cpp -- compiler for graphics utility

//- Compile -- compile .gtl file to internal objects
public: int Compile(const char * xpszPathName) ;
//- ParseLine -- parse input line
private: BOOL ParseLine(void) ;
//- ParseInteger -- parse integer, store into node structure
private: CLexElement * ParseInteger(CLexElement * xpLxel,
                int iPrevType, int FAR& iValue) ;
//- ParseString -- parse string, store into node structure
//      Note: Accepts identifier as well as string
private: CLexElement * ParseString(CLexElement * xpLxel,
                int iPrevType, LPSTR lpszValue, XPINT xpiValue) ;
//- GetLabel -- get bitmap or node label
private: BOOL GetLabel(CLexElement * xpLxel,
                        BOOL bNode, int FAR& iIndex) ;
//- GetLabel -- get bitmap or node label
private: BOOL GetLabel(LPSTR lpszLabel,
                        BOOL bNode, int FAR& iIndex) ;
//- AddLink -- link together a pair of nodes
private: BOOL AddLink(CNode FAR * lpNode1, CNode FAR * lpNode2) ;
//- AddLink -- add link to one node
private: BOOL AddLink(CNode FAR * lpNode, int iLink) ;



// gtllex -- lexical analysis for graphics utility

//- FindKeyword -- find keyword, given tree node type
private: XPSTR FindKeyword(int iType) ;
//- ReadLine -- read input line
private: BOOL ReadLine(void) ;
//- ErrorMsg -- publish error message
private: BOOL ErrorMsg(CLexElement * xpLxel, XPSTR szMessage) ;



// gtlui.cpp -- data interface to Windows

//- CGtlData -- constructor -- zero out all fields, allocate
//      arrays, and set colors
public: CGtlData(void) ;
//- ~CGtlData -- destructor
public: ~CGtlData(void) ;

    VOID    CreateOffScreenBmp(VOID);
    VOID    DeleteOffScreenBmp(VOID);
    VOID    PaintOffScreenBmp(VOID);
    CMemDC *GetMemDC(VOID);
    VOID    ReleaseMemDC(CMemDC *);


//- SetColors -- set colors of various elements
public: BOOL SetColors(void) ;
//- ClearInhibitDraw -- clear m_bInhibitDraw flag
private: BOOL ClearInhibitDraw(void) ;
//- Draw -- draw data
public: BOOL Draw(CGtlView * xpGtlView, CRect * xpClipRect,
                        CDC* xpDc PDFT(NULL)) ;
//- SwitchDc -- switch between relocatable/nonrelocatable
//              device context
private: BOOL SwitchDc(CDC * xpDc, BOOL bRelocatable) ;
//- DrawBitmaps -- draw bitmaps for Draw routine
private: BOOL DrawBitmaps(CDC *, BOOL);

//- DrawABitmap -- draw a bitmap object for Draw routine
private: BOOL DrawABitmap(CDC *, CBgbObject *, BOOL bPaint PDFT(FALSE));

//- NodeToPoint -- return relocatable coordinates of node
//              adjusted by optional size object
private: CRPoint NodeToPoint(CNode * lpNode,
                        CSize FAR * lpcSize PDFT(NULL)) ;
//- SpecifyUpdate -- specify update rectangle
public: BOOL SpecifyUpdate(CGtlView * xpGtlView) ;
//- UpdateDialogs -- update modeless dialog boxes
public: BOOL UpdateDialogs(BOOL bRetrieve PDFT(FALSE)) ;
//- UpdateMenuDialog -- update control dialog box
private: BOOL UpdateMenuDialog(BOOL bRetrieve) ;

//- UpdateInfoDialog -- update information dialog box
private: BOOL UpdateInfoDialog(BOOL bRetrieve) ;

//- UpdateControlDialog -- update control dialog box
private: BOOL UpdateControlDialog(BOOL bRetrieve) ;

//- UpdateNodeDialog -- update node dialog box
private: BOOL UpdateNodeDialog(BOOL bRetrieve) ;

//- UpdateFocusRect -- update focus rectangle for hiliting
private: BOOL UpdateFocusRect(void) ;

//- AdjustToView -- set up bitmaps for view
private: BOOL AdjustToView(CGtlView *);

//- InitOverlay -- initialize overlay
private: BOOL InitOverlay(CMap FAR * lpMap) ;
//- NormalizeData -- normalize data after loading
private: BOOL NormalizeData(CGtlView * xpGtlView) ;

// gtlmanp.cpp -- manipulate the data

//- AcceptClick -- process a mouse click by user
public: BOOL AcceptClick(CGtlView * xpGtlView,CPoint cClickPoint, int iClickType) ;
//- AcceptDeleteKey -- process depressing delete key
public: BOOL AcceptDeleteKey(CGtlView * xpGtlView) ;
//- PointToNode -- find node closest to point
private: CNode FAR * PointToNode(CRPoint crPoint) ;
//- SelectNode -- algorithm when node is clicked
private: BOOL SelectNode(CNode FAR * lpNode) ;
//- ModifySelectNode -- modify node select/deselect
private: BOOL ModifySelectNode(CNode FAR * lpNode PDFT(NULL), BOOL bSelect PDFT(TRUE)) ;
//- GetSelectedNode -- return selected node, or NULL if none
private: CNode FAR * GetSelectedNode(void) ;
//- SelectLink -- select or deselect a link
private: BOOL SelectLink(CNode FAR * lpNode1 PDFT(NULL), CNode FAR * lpNode2 PDFT(NULL)) ;
//- IfLinked -- test whether two nodes are linked
private: BOOL IfLinked(CNode FAR * lpNode1, CNode FAR * lpNode2) ;
//- DeleteNode -- delete node and all connecting links
private: BOOL DeleteNode(CNode * lpNode) ;
//- DeleteLink -- delete link between two nodes
private: BOOL DeleteLink(CNode FAR * lpNode1 PDFT(NULL), CNode FAR * lpNode2 PDFT(NULL)) ;
//- DeleteLinkIndex -- delete index from array of link
//              indices for a given node, if found
private: BOOL DeleteLinkIndex(CNode FAR * lpNode, int iLink) ;
//- CallUpdate -- call to update all views for this map
private: BOOL CallUpdate(CNode FAR * lpNode1 PDFT(NULL),
        CNode FAR * lpNode2 PDFT(NULL), BOOL bLinks PDFT(FALSE),
        BOOL bWmPaint PDFT(FALSE)) ;
//- CallUpdate -- call to update all views for this map
//              -- version which updates a bitmap object
private: BOOL CallUpdate(CBgbObject * lpcBgbObject) ;
//- CreateNode -- create a new node
private: BOOL CreateNode(CNode FAR *& lpNode, CRPoint crPosition) ;
//- MoveNode -- move node
private: BOOL MoveNode(CNode FAR * lpNode, CRPoint crPosition) ;

    int DoSpecialTravel(int, BOOL);
    VOID LoadCharDirection(CBgbObject *, CPoint, CPoint);

//- CreateLink -- create link between two nodes
private: BOOL CreateLink(CNode FAR * lpNode1, CNode FAR * lpNode2) ;
//- SetNodePosition -- set position relative to bitmap
private: BOOL SetNodePosition(CNode FAR * lpNode, CRPoint crPosition) ;


// gtlmgm.cpp -- meta game

//- SetMetaGame -- set meta game on or off
public: BOOL SetMetaGame(BOOL bOn) ;
//- InitMetaGame -- init or release sprites for Meta Game
public: BOOL InitMetaGame(CGtlView * xpGtlView PDFT(NULL),
        BOOL bInit PDFT(TRUE)) ;
//- ProcessMove -- handle move processing
public: BOOL ProcessMove(CNode FAR * lpTargetNode PDFT(NULL)) ;
//- MoveCharToNode -- move current character to specified node
private: BOOL MoveCharToNode(CNode FAR * lpTargetNode) ;
//- EstimatePathDistance -- estimate distance between nodes
private: int EstimatePathDistance(CNode FAR * lpNode1,
                                CNode FAR * lpNode2) ;
//- FindShortestPath -- between two nodes
private: LPINT FindShortestPath(CNode FAR * lpNode1,
                                CNode FAR * lpNode2) ;
//- PositionCharacters -- set positions for Hodj and Podj
public: BOOL PositionCharacters(void) ;
//- PositionACharacter -- find positions for Hodj and Podj
private: BOOL PositionACharacter(CXodj * xpXodj, int iShift) ;
//- LocationToNode -- find node for game location
private: CNode FAR * LocationToNode(int iLocationCode) ;
public: int FindNodeId(char *pszLabel);


// gtlmve.cpp -- meta game move processing

//- InitProblem -- initialize character problem
private: BOOL InitProblem(void) ;
//- EndMoveProcessing -- handle things at end of a move
private: BOOL EndMoveProcessing(void) ;
//- SwitchPlayers -- switch between Hodj and Podj
public: BOOL SwitchPlayers(void) ;
//- InitInterface -- init data for interface manager
public: BOOL InitInterface(int iCode, BOOL & bExitDll) ;
//- ReturnFromInterface -- reset everything
//              after interface return
public: BOOL ReturnFromInterface(void) ;
//- TakeIneligibleAction -- take action if character is at
//      a location but is not eligible to perform function
private: BOOL TakeIneligibleAction(CXodj * xpXodj,
            int iFunctionCode, int iLocationCode) ;
//- DumpGameStatus -- (debugging) dump status of game
private: BOOL DumpGameStatus(int iOptionFlags
                    PDFT(DUMPSTAT_EVERYTHING)) ;
//- ProcessGameResult -- process result of game,
//              optionally generating a random win
private: BOOL ProcessGameResult(CXodj * xpXodj,
                int iGameCode, LPGAMESTRUCT lpGameStruct PDFT(NULL)) ;

//- GainRandomItem -- player receives a random item from store
public:  BOOL GainRandomItem(CXodj * xpXodj) ;

//- GainMoney -- player has won some money
public:  BOOL GainMoney(CXodj * xpXodj, long lCrowns) ;

//- GainMishMosh -- player gets MishMosh
public:  BOOL GainMishMosh(CXodj * xpXodj, long lCrowns) ;

//- DivulgeInformation -- divulge information to player
public:  BOOL DivulgeInformation(CXodj * xpXodj, BOOL bSecondary) ;

//- DetermineEligibility -- determine whether
//  player has objects/money needed for current location
private: BOOL DetermineEligibility(CXodj *xpXodj, int, int &, BOOL bExecute PDFT(FALSE));

//- DetermineGameEligibility -- determine if eligible to play
//  a game, and update game history if so (and bExecute)
// ***** modify this function to determine how often a user can
//  play the same game over and over
private: BOOL DetermineGameEligibility(CXodj *xpXodj, int iGameCode, BOOL bExecute PDFT(FALSE));

//- GetGameObjectCount -- get the count of the specified
//      object in the player's inventory
private: long GetGameObjectCount(CXodj * xpXodj, int iObjectCode) ;

//- DetermineInfoEligibility -- determine if eligible to
//  get information at this location
private: BOOL DetermineInfoEligibility(CXodj * xpXodj,
            int iLocationCode, BOOL bExecute PDFT(FALSE)) ;

//- DetermineMishMoshEligibility -- determine if eligible to
//  grab Mish and Mosh from current location
private: BOOL DetermineMishMoshEligibility(CXodj *, int);

VOID DoTransport(CXodj *, INT);


// gtlcmp.cpp -- meta game computer play

//- SelectBestMove -- select best move for computer play
//      (can also be used for human player for debugging or giving hints)
private: BOOL SelectBestMove(CXodj * xpXodj) ;
//- GatherInformation -- gather info to decide computer's move
private: BOOL GatherInformation(CXodj * xpXodj) ;
//- DetermineWeights -- determine weight for each
//              location (independent of distance)
private: BOOL DetermineWeights(CXodj * xpXodj) ;
//- DetermineDistances -- determine distance to each
//                      eligible location
private: BOOL DetermineDistances(CXodj * xpXodj) ;
//- AdjustWeightByDistance -- adjust each by the distance
//              of the node
private: BOOL AdjustWeightByDistance(CXodj * xpXodj) ;
//- FindTopLocations -- find locations with top weights
private: BOOL FindTopLocations(CXodj * xpXodj) ;
//- DumpBestMoveData -- dump data used in best move computation
private: BOOL DumpBestMoveData(CXodj * xpXodj) ;

private:
    void CheckForTransport( CXodj *, int);
    void SetFurlongs(CXodj *);
} ;

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
