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

#ifndef BAGEL_METAGAME_GTL_GTLDAT_H
#define BAGEL_METAGAME_GTL_GTLDAT_H

#include "common/stream.h"
#include "common/system.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/metagame/bgen/bgen.h"
#include "bagel/hodjnpodj/metagame/bgen/bgenut.h"
#include "bagel/hodjnpodj/metagame/bgen/bsutl.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlview.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/metagame/gtl/resource.h"
#include "bagel/hodjnpodj/metagame/bgen/bgb.h"
#include "bagel/hodjnpodj/metagame/bgen/bbt.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"
#include "bagel/hodjnpodj/metagame/bgen/mgstat.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

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
inline long SQ(int y) {
	return ((long)((long)y * (long)y)) ;
}
inline long SQ(long y) {
	return (y * y) ;
}
inline long LSQ(CPoint p1, CPoint p2) {
	return (SQ(p1.x - p2.x) + SQ(p1.y - p2.y)) ;
}

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

class CGtlView;

class CMemDC {
public:
	CMemDC();
	~CMemDC();

	CDC      *m_pDC = nullptr;
	CPalette *m_pPalOld = nullptr;
	HBITMAP   m_hBmpOld = nullptr;
};

// CLexElement -- lexical element class
class CLexElement {
	friend class CGtlData ;
	int m_iType ;         // lexical type -- LXT_xxxx
	int m_iVal ;          // integer or char or keyword code
	int m_iStringListPos ;      // string position
	int m_iLineNumber, m_iColumn ;      // for error messages
};

// CKeyTab -- keyword table element
class CKeyTab {
public:
	int m_iKeyValue ;   // keyword value
	const char *m_xpszKeyString ;     // string
};

// CMap -- bit map
class CMap {
	friend class CGtlData ;

	char m_szLabel[MAX_LABEL_LENGTH] = {};  // bitmap label
	char m_szFilename[MAX_FILENAME_LENGTH] = {};    // file name of BMP file
	bool m_bPositionDetermined: 1 ;
	bool m_bSprite : 1 ;
	bool m_bPalette : 1 ;               // game palette from this bitmap
	bool m_bOverlay : 1 ;               // overlay onto background map
	bool m_bMasked : 1 ;                // bitmap is masked
	bool m_bMetaGame : 1 ;              // show bitmap only in metagame
	bool m_bRelocatable : 1 ;           // relocatable
	bool m_bPositionSpecified : 1 ;     // position specified on input
	bool m_bSpecialPaint: 1 ;           // don't automatically paint
	CBgbObject FAR *m_lpcBgbObject = nullptr;    // Boffo game object for bitmap file

	int m_iRelationType = 0;       // KT_ABOVE, KT_BELOW, KT_LEFT, KT_RIGHT,
	// KT_NODE
	int m_iRelation = 0;           // index of relation bitmap or node

	CMap() {
		m_bPositionDetermined = false;
		m_bSprite = false;
		m_bPalette = false;
		m_bOverlay = false;
		m_bMasked = false;
		m_bMetaGame = false;
		m_bRelocatable = false;
		m_bPositionSpecified = false;
		m_bSpecialPaint = false;
	}
};

/**
 * Node on the map
 */
class CNode {
public:
	char m_szLabel[MAX_LABEL_LENGTH] = { 0 };  // node label
	int m_iLinks[MAX_LINKS_PER_NODE] = { 0 };   // up to 10 links per node
	int m_iLocationCode = 0;			// MG_LOC_xxxx to specify location
	int m_iSector = 0;					// MG_SECTOR_xxxx to specify sector
	int m_iBitmap = 0;					// index of related bitmap
	int m_iX = 0, m_iY = 0;				// position
	int m_iNumLinks = 0;				// number of links
	bool m_bDeleted : 1;				// deleted node
	bool m_bSelected : 1;				// this is selected node
	bool m_bRelative : 1;				// relative to bitmap
	bool m_bRelocatable : 1;			// relocatable node
	bool m_bWgtSpec : 1;				// weight specified
	bool m_bSenSpec : 1;				// sensitivity specified
	bool m_bMenu : 1;					// true if menu
	byte m_iWeight = 0;				// node weight
	byte m_iSensitivity = 0;			// distance to accept clickb

	CNode();

	bool IfRelocatable() {
		return (m_bRelocatable != 0) ;
	}
};

// CStratLocInfo -- strategy location information
class CStratLocInfo {
	friend class CGtlData ;
	friend class CStrategyInfo ;

	int m_iLocCode = 0;				// MG_LOC_xxxx -- location code
	CNode FAR *m_lpNode = nullptr;	// node pointer for location
	int m_iValueCode = 0;			// MG_VISIT_xxxx or MG_WIN_xxxx
	// or 0 if not eligible
	int m_iDistance = 0;			// distance to location
	int m_iWeight = 0;				// weight of location
	int m_iAdjustedWeight = 0;		// weight adjusted by distance

	void clear() {
		m_iLocCode = 0;
		m_lpNode = nullptr;
		m_iValueCode = 0;
		m_iDistance = 0;
		m_iWeight = 0;
		m_iAdjustedWeight = 0;
	}
};


// CStrategyInfo -- strategy information for determining best move
class CStrategyInfo {
	friend class CGtlData ;

	CStratLocInfo *xpTargetLocInfo = nullptr;		// target location (best move)

	int m_iRequiredObjectsCount = 0;				// total number of objects I need
	int m_iRequiredObjectsTable[MAX_GAME_TABLE] = {};	// objects I need

	int m_iMaximumDistance = 0;						// max distance to eligible location
	int m_iMaxAdjustedWeight = 0;					// maximum adjusted weight
	int m_iTopLocCount = 0;							// number of top locations
	int m_iTopLocTable[MG_LOC_MAX] = {};			// table of top locations

	CStratLocInfo m_cStratLocInfo[MG_LOC_MAX + 1] ;
};

// CXodj -- structure for Hodj Podj
class CXodj {
	friend class CGtlData ;

public:

	CXodj *m_xpXodjNext = nullptr;       // next in chain
	char m_szName[MAX_LABEL_LENGTH] = {};   // name of character
	bool m_bHodj = false;              // char is Hodj (not Podj)
	int m_iFurlongs = 0;           // number of furlongs available
	int m_iCharNode = 0;           // node where character is positioned

	int m_nTurns = 0;               // number of turns remaining (not furlongs)

	bool m_bComputer = false;          // played by computer
	CMap FAR *m_lpMap = nullptr;         // bitmap object
	CBgbObject FAR *m_lpcCharSprite = nullptr;   // character sprite bgb object
	CBgbObject FAR *m_lpcIcon = nullptr;         // icon for character

	CSound *m_pThemeSound = nullptr;              // this character's MIDI theme song info
	const char *m_pszThemeFile = nullptr;
	unsigned int    m_nThemeStart = 0;
	unsigned int    m_nThemeEnd = 0;

	const char *m_pszStarsFile = nullptr;             // cel strip for stars animation

	// fields for computing computer play (also used for human player
	// for debugging or giving hints)
	class CStrategyInfo *m_xpStrategyInfo = nullptr;  // strategy information

	// list of clue numbers for clues given by winning mini-game
	int m_iWinInfoWon = 0;     // how many clues I've received
	int m_iWinInfoNeed = 0;    // total number of clues I need
	int m_iWinInfoTable[MAX_GAME_TABLE] = {};

	// list of clue numbers for clues given by farmer, etc.
	int m_iSecondaryInfoWon = 0;   // how many clues I've received
	int m_iSecondaryInfoNeed = 0;  // total number of clues I need
	int m_iSecondaryInfoTable[MAX_GAME_TABLE] = {};

	// list of objects required to get Mish/Mosh
	int m_iRequiredObjectsCount = {};   // total number of objects I need
	int m_iRequiredObjectsTable[MAX_GAME_TABLE] = {}; // objects I need
	int m_iRequiredMoney = 0;      // money needed for Mish/Mosh

	// list of secondary information location we still have to visit
	int m_iSecondaryLoc[MAX_GAME_TABLE] = {};

	int m_iGameHistory[20] = {};    // last 20 mini-games played
	int m_iTargetLocation = 0;     // target location for computer play
	bool m_bGatherInformation = false; // if false, data from last
	// GatherInformation is still valid

	CInventory *m_pInventory = nullptr;   // items in personal inventory
	CInventory *m_pGenStore = nullptr;    // items in general store
	CInventory *m_pBlackMarket = nullptr;    // items in pawn shop
	CInventory *m_pTradingPost = nullptr;    // items in pawn shop

	~CXodj();
};

// CGtlData -- data class for graphics utility
class CGtlData {
public:
	class CGtlDoc *m_xpcGtlDoc = nullptr; // document pointer

private:
	CGenUtil m_cGenUtil;       // general utility object
	CBgbMgr m_cBgbMgr;         // Boffo Game Object manager
	CBbtMgr m_cBbtMgr;         // Boffo Games button manager
	CBbutton m_cMiniButton;    // minimap button
	CBbutton m_cInvButton;     // inventory button
	CBbutton m_cScrollButton;  // scroll button

	CMap FAR *m_lpMaps = nullptr;  // bitmap array
	CNode FAR *m_lpNodes = nullptr;      // node array

	bool m_bGtlDll = false;            // equals GTLDLL define
	char m_szBmpDirectory[MAX_FILENAME_LENGTH] = {}; // bitmap file director
	char m_szGtlFile[MAX_FILENAME_LENGTH] = {}; // gtl file name
	char m_szListFile[MAX_FILENAME_LENGTH] = {}; // List file name
	Common::SeekableReadStream *m_xpGtlFile = nullptr;         // ptr to file structure for .GTL file
	Common::WriteStream *m_xpListFile = nullptr;        // ptr to file structure for .LST file
	bool m_bListing = false;           // listing file flag
	int m_iIndent = 0;             // current indent (decompile only)
	int m_iLineNumber = 0;         // line number in input file
	int m_iMaps = 0, m_iNodes = 0;     // # of bitmaps, nodes
	bool m_bEof = false;               // end of file on input (or END stmt)
	CNode FAR *m_lpLastNode = nullptr;   // pointer to last node
	int m_iNumGenLabels = 0;               // number of generated lables
	CLexElement *m_xpLexLabel = nullptr;         // current label lexeme
	CLexElement m_cLexElts[MAX_LEXELTS] = {};       // lexical element blocks
	char m_szStringList[MAX_STRINGLIST] = {};     // compiler input string
	bool m_bSelectedLink = false;      // flag: a link is selected
	int m_iSelLinkPt1 = 0, m_iSelLinkPt2 = 0;  // indexes of selected link pts
	uint32 m_stAcceptClickActive = 0;       // prevent recursion
	uint32 m_stLDownTime = 0;       // time of left button down
	CNode FAR *m_lpFoundNode = nullptr;  // node clicked on
	CNode FAR *m_lpLastSelectedNode = nullptr;   // selected on click down

	bool m_bJustPlayedMiniGame = false;	// true for Mini-Games in ProcessGameResult

	// fields for Draw routine
public:
	bool m_bInhibitDraw = false;		// inhibit any actual drawing
	bool m_bDrawing = false;			// in Draw routine (prevent recursion)
	CGtlView *m_xpGtlView = nullptr;	// current view
	bool m_bPaint = false;				// WM_PAINT flag
	bool m_bRelocatable = false;		// device context is currently
	// relocatable
	CRect *m_xpClipRect = nullptr;		// relocatable clipping rectangle
	CRect *m_xpDevClipRect = nullptr;	// device (non-relocatable)
	// clipping rectangle
	CMap FAR *m_lpFurlongMaps[25] = {};	// 0-24 furlong bitmaps

// ---
public:
	int m_iSizeX = 0, m_iSizeY = 0;		// size of entire client area
	// (entire bitmap)
	int m_iMargin = 0;					// size of top margin of view window
	bool m_bChangeSize = false;			// size of window changed (because
	// gtl modified the bitmap template)
	int m_iViewSizeX = 0, m_iViewSizeY = 0;	// size of view

	bool m_bPaintBackground = false;	// paint background
	bool m_bShowNodes = false;			// show nodes and links
	bool m_bShowSensitivity = false;	// show node sensitivities
	bool m_bStartMetaGame = false;		// start in meta game mode
	bool _metaGame = false;				// meta game mode
	bool m_bInitMetaGame = false;		// init sprites at beginning of metagame

	CXodj *m_xpXodjChain = nullptr;		// chain of characters
	CXodj *m_xpCurXodj = nullptr;		// character currently on the move
	bool m_bSamePlace = false;			// both characters are in same place
	bool m_bGameOver = false;			// game is over

	CBitmap  *m_pOffScreenBmp = nullptr;
	CPalette *m_pPalOld = nullptr;
	HBITMAP   m_hOldBitmap = nullptr;

	int m_iMishMoshLoc = 0;				// MG_LOC_xxxx -- Mish/Mosh location

	COLORREF m_cNodePenColor = 0, m_cNodeBrushColor = 0, m_cLinkPenColor = 0;
	COLORREF m_cNodeSensitivityColor = 0;
	COLORREF m_cNodePassThruColor = 0, m_cNodeHeavyColor = 0;
	COLORREF m_cSelNodePenColor = 0, m_cSelNodeBrushColor = 0;
	COLORREF m_cSelLinkPenColor = 0;
	COLORREF m_cBackColor = 0;

	// constructor zeroes out all fields and allocates far arrays
public:
	void ClearData() {
		*this = CGtlData();
	}

	// gtldcp.cpp -- decompiler for meta game

	//- Decompile -- output data to .GTL file
	int Decompile(const char *xpszPathName) ;
	//- AsciiOutput -- ascii output
	int AsciiOutput(int iIndent, XPSTR lpszOut) ;
	//- ListingOutput -- listing output
	int ListingOutput(int iIndent, XPSTR lpszOut) ;


	// gtlcpl.cpp -- compiler for graphics utility

	/**
	 * Compile .gtl file to internal objects.
	 * @param xpszPathName	Input file name
	 * @returns		true if error, false otherwise
	 */
	int Compile(const char *xpszPathName);

	//- ParseLine -- parse input line
	bool ParseLine() ;
	//- ParseInteger -- parse integer, store into node structure
	CLexElement *ParseInteger(CLexElement * xpLxel,
	                          int iPrevType, int FAR& iValue) ;
	//- ParseString -- parse string, store into node structure
	//      Note: Accepts identifier as well as string
	CLexElement *ParseString(CLexElement * xpLxel,
	                         int iPrevType, char *lpszValue, XPINT xpiValue) ;
	//- GetLabel -- get bitmap or node label
	bool GetLabel(CLexElement * xpLxel,
	              bool bNode, int FAR& iIndex) ;
	//- GetLabel -- get bitmap or node label
	bool GetLabel(char *lpszLabel,
	              bool bNode, int FAR& iIndex) ;

private:
	//- AddLink -- link together a pair of nodes
	bool AddLink(CNode FAR * lpNode1, CNode FAR * lpNode2) ;
	//- AddLink -- add link to one node
	bool AddLink(CNode FAR * lpNode, int iLink) ;


	// gtllex -- lexical analysis for graphics utility

	//- FindKeyword -- find keyword, given tree node type
	const char *FindKeyword(int iType) ;
	//- ReadLine -- read input line
	bool ReadLine() ;
	//- ErrorMsg -- publish error message
	bool ErrorMsg(CLexElement * xpLxel, const char *szMessage) ;

	// gtlui.cpp -- data interface to Windows

public:
	//- CGtlData -- constructor -- zero out all fields, allocate
	//      arrays, and set colors
	CGtlData() ;
	~CGtlData() ;

	void    CreateOffScreenBmp();
	void    DeleteOffScreenBmp();
	void    PaintOffScreenBmp();
	CMemDC *GetMemDC();
	void    ReleaseMemDC(CMemDC *);


	bool SetColors() ;
	//- ClearInhibitDraw -- clear m_bInhibitDraw flag
	bool ClearInhibitDraw() ;
	//- Draw -- draw data
	bool Draw(CGtlView * xpGtlView, CRect * xpClipRect,
	          CDC* xpDc PDFT(nullptr)) ;
	//- SwitchDc -- switch between relocatable/nonrelocatable
	//              device context
	bool SwitchDc(CDC * xpDc, bool bRelocatable) ;
	//- DrawBitmaps -- draw bitmaps for Draw routine
	bool DrawBitmaps(CDC *, bool);

	//- DrawABitmap -- draw a bitmap object for Draw routine
	bool DrawABitmap(CDC *, CBgbObject *, bool bPaint PDFT(false));

	//- NodeToPoint -- return relocatable coordinates of node
	//              adjusted by optional size object
	CRPoint NodeToPoint(CNode * lpNode,
	                    CSize FAR * lpcSize PDFT(nullptr)) ;
	//- SpecifyUpdate -- specify update rectangle
	bool SpecifyUpdate(CGtlView * xpGtlView) ;
	//- UpdateDialogs -- update modeless dialog boxes
	bool UpdateDialogs(bool bRetrieve PDFT(false)) ;

private:
	//- UpdateMenuDialog -- update control dialog box
	bool UpdateMenuDialog(bool bRetrieve) ;

	//- UpdateInfoDialog -- update information dialog box
	bool UpdateInfoDialog(bool bRetrieve) ;

	//- UpdateControlDialog -- update control dialog box
	bool UpdateControlDialog(bool bRetrieve) ;

	//- UpdateNodeDialog -- update node dialog box
	bool UpdateNodeDialog(bool bRetrieve) ;

	//- UpdateFocusRect -- update focus rectangle for hiliting
	bool UpdateFocusRect() ;

	//- AdjustToView -- set up bitmaps for view
	bool AdjustToView(CGtlView *);

	//- InitOverlay -- initialize overlay
	bool InitOverlay(CMap FAR * lpMap) ;
	//- NormalizeData -- normalize data after loading
	bool NormalizeData(CGtlView * xpGtlView) ;

	// gtlmanp.cpp -- manipulate the data
public:

	//- AcceptClick -- process a mouse click by user
	bool AcceptClick(CGtlView * xpGtlView, CPoint cClickPoint, int iClickType) ;
	//- AcceptDeleteKey -- process depressing delete key
	bool AcceptDeleteKey(CGtlView * xpGtlView) ;

private:
	//- PointToNode -- find node closest to point
	CNode FAR *PointToNode(CRPoint crPoint) ;
	//- SelectNode -- algorithm when node is clicked
	bool SelectNode(CNode FAR * lpNode) ;
	//- ModifySelectNode -- modify node select/deselect
	bool ModifySelectNode(CNode FAR * lpNode PDFT(nullptr), bool bSelect PDFT(true)) ;
	//- GetSelectedNode -- return selected node, or nullptr if none
	CNode FAR *GetSelectedNode() ;
	//- SelectLink -- select or deselect a link
	bool SelectLink(CNode FAR * lpNode1 PDFT(nullptr), CNode FAR * lpNode2 PDFT(nullptr)) ;
	//- IfLinked -- test whether two nodes are linked
	bool IfLinked(CNode FAR * lpNode1, CNode FAR * lpNode2) ;
	//- DeleteNode -- delete node and all connecting links
	bool DeleteNode(CNode * lpNode) ;
	//- DeleteLink -- delete link between two nodes
	bool DeleteLink(CNode FAR * lpNode1 PDFT(nullptr), CNode FAR * lpNode2 PDFT(nullptr)) ;
	//- DeleteLinkIndex -- delete index from array of link
	//              indices for a given node, if found
	bool DeleteLinkIndex(CNode FAR * lpNode, int iLink) ;
	//- CallUpdate -- call to update all views for this map
	bool CallUpdate(CNode FAR * lpNode1 PDFT(nullptr),
	                CNode FAR * lpNode2 PDFT(nullptr), bool bLinks PDFT(false),
	                bool bWmPaint PDFT(false)) ;
	//- CallUpdate -- call to update all views for this map
	//              -- version which updates a bitmap object
	bool CallUpdate(CBgbObject * lpcBgbObject) ;
	//- CreateNode -- create a new node
	bool CreateNode(CNode FAR * &lpNode, CRPoint crPosition) ;
	//- MoveNode -- move node
	bool MoveNode(CNode FAR * lpNode, CRPoint crPosition) ;

	int DoSpecialTravel(int, bool);
	void LoadCharDirection(CBgbObject *, CPoint, CPoint);

	//- CreateLink -- create link between two nodes
	bool CreateLink(CNode FAR * lpNode1, CNode FAR * lpNode2) ;
	//- SetNodePosition -- set position relative to bitmap
	bool SetNodePosition(CNode FAR * lpNode, CRPoint crPosition) ;


	// gtlmgm.cpp -- meta game
public:

	//- SetMetaGame -- set meta game on or off
	bool SetMetaGame(bool bOn) ;
	//- InitMetaGame -- init or release sprites for Meta Game
	bool InitMetaGame(CGtlView * xpGtlView PDFT(nullptr),
	                  bool bInit PDFT(true)) ;
	//- ProcessMove -- handle move processing
	bool ProcessMove(CNode FAR * lpTargetNode PDFT(nullptr)) ;

private:
	//- MoveCharToNode -- move current character to specified node
	bool MoveCharToNode(CNode FAR * lpTargetNode) ;
	//- EstimatePathDistance -- estimate distance between nodes
	int EstimatePathDistance(CNode FAR * lpNode1,
	                         CNode FAR * lpNode2) ;
	//- FindShortestPath -- between two nodes
	int *FindShortestPath(CNode FAR * lpNode1,
	                       CNode FAR * lpNode2) ;

public:
	//- PositionCharacters -- set positions for Hodj and Podj
	bool PositionCharacters() ;

private:
	//- PositionACharacter -- find positions for Hodj and Podj
	bool PositionACharacter(CXodj * xpXodj, int iShift) ;
	//- LocationToNode -- find node for game location
	CNode FAR *LocationToNode(int iLocationCode) ;

public:
	int FindNodeId(const char *pszLabel);


	// gtlmve.cpp -- meta game move processing

private:
	//- InitProblem -- initialize character problem
	bool InitProblem() ;
	//- EndMoveProcessing -- handle things at end of a move
	bool EndMoveProcessing() ;

public:
	//- SwitchPlayers -- switch between Hodj and Podj
	bool SwitchPlayers() ;
	//- InitInterface -- init data for interface manager
	bool InitInterface(int iCode, bool & bExitDll) ;
	//- ReturnFromInterface -- reset everything
	//              after interface return
	bool ReturnFromInterface() ;

private:
	//- TakeIneligibleAction -- take action if character is at
	//      a location but is not eligible to perform function
	bool TakeIneligibleAction(CXodj * xpXodj,
	                          int iFunctionCode, int iLocationCode) ;
	//- DumpGameStatus -- (debugging) dump status of game
	bool DumpGameStatus(int iOptionFlags
	                    PDFT(DUMPSTAT_EVERYTHING)) ;
	//- ProcessGameResult -- process result of game,
	//              optionally generating a random win
	bool ProcessGameResult(CXodj * xpXodj,
	                       int iGameCode, LPGAMESTRUCT lpGameStruct PDFT(nullptr)) ;

public:
	//- GainRandomItem -- player receives a random item from store
	bool GainRandomItem(CXodj * xpXodj) ;

	//- GainMoney -- player has won some money
	bool GainMoney(CXodj * xpXodj, long lCrowns) ;

	//- GainMishMosh -- player gets MishMosh
	bool GainMishMosh(CXodj * xpXodj, long lCrowns) ;

	//- DivulgeInformation -- divulge information to player
	bool DivulgeInformation(CXodj * xpXodj, bool bSecondary) ;

private:
	//- DetermineEligibility -- determine whether
	//  player has objects/money needed for current location
	bool DetermineEligibility(CXodj *xpXodj, int, int &, bool bExecute PDFT(false));

	//- DetermineGameEligibility -- determine if eligible to play
	//  a game, and update game history if so (and bExecute)
	// ***** modify this function to determine how often a user can
	//  play the same game over and over
	bool DetermineGameEligibility(CXodj *xpXodj, int iGameCode, bool bExecute PDFT(false));

	//- GetGameObjectCount -- get the count of the specified
	//      object in the player's inventory
	long GetGameObjectCount(CXodj * xpXodj, int iObjectCode) ;

	//- DetermineInfoEligibility -- determine if eligible to
	//  get information at this location
	bool DetermineInfoEligibility(CXodj * xpXodj,
	                              int iLocationCode, bool bExecute PDFT(false)) ;

	//- DetermineMishMoshEligibility -- determine if eligible to
	//  grab Mish and Mosh from current location
	bool DetermineMishMoshEligibility(CXodj *, int);

	void DoTransport(CXodj *, int);


	// gtlcmp.cpp -- meta game computer play
private:
	//- SelectBestMove -- select best move for computer play
	//      (can also be used for human player for debugging or giving hints)
	bool SelectBestMove(CXodj * xpXodj) ;
	//- GatherInformation -- gather info to decide computer's move
	bool GatherInformation(CXodj * xpXodj) ;
	//- DetermineWeights -- determine weight for each
	//              location (independent of distance)
	bool DetermineWeights(CXodj * xpXodj) ;
	//- DetermineDistances -- determine distance to each
	//                      eligible location
	bool DetermineDistances(CXodj * xpXodj) ;
	//- AdjustWeightByDistance -- adjust each by the distance
	//              of the node
	bool AdjustWeightByDistance(CXodj * xpXodj) ;
	//- FindTopLocations -- find locations with top weights
	bool FindTopLocations(CXodj * xpXodj) ;
	//- DumpBestMoveData -- dump data used in best move computation
	bool DumpBestMoveData(CXodj * xpXodj) ;

	void CheckForTransport(CXodj *, int);
	void SetFurlongs(CXodj *);
};

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
