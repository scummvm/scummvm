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

#ifndef HODJNPODJ_METAGAME_BOARDGAME_DATA_H
#define HODJNPODJ_METAGAME_BOARDGAME_DATA_H

#include "common/file.h"
#include "common/savefile.h"
#include "bagel/hodjnpodj/views/view.h"
#include "bagel/hodjnpodj/gfx/button.h"
#include "bagel/hodjnpodj/metagame/bgen/bbt.h"
#include "bagel/hodjnpodj/metagame/bgen/bgen_util.h"
#include "bagel/hodjnpodj/metagame/bgen/bs_util.h"
#include "bagel/hodjnpodj/metagame/bgen/bgb.h"
#include "bagel/hodjnpodj/metagame/boardgame/gtl_view.h"
#include "bagel/boflib/sound.h"

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
#define MAX_LABEL_LENGTH 32             // max length of node/bitmap label
// label/action
#define MAX_BITMAPS 130

#define NODERADIUS 5            /* display radius of node */
#define NODESENSITIVITY 20      /* default node sensitivity */
#define MAX_GAME_TABLE 12       /* meta game execution table */
#define MAX_CLUES 100

#define NEWSCROLL 1

// defining distance between nodes
inline long SQ(int y) {
	return (long)((long)y * (long)y);
}
inline long SQ(long y) {
	return y * y;
}
inline long LSQ(const Common::Point &p1, const Common::Point &p2) {
	return SQ(p1.x - p2.x) + SQ(p1.y - p2.y);
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

class CMemDC {
public:
	CMemDC();
	~CMemDC();

	void *m_pDC = nullptr;	// CDC
	Graphics::Palette *m_pPalOld = nullptr;	// CPalette
	int m_hBmpOld = 0;		// HBITMAP
};

// CLexElement -- lexical element class
class CLexElement {
	friend class CGtlData;
	int m_iType = 0;         // lexical type -- LXT_xxxx
	int m_iVal = 0;          // integer or char or keyword code
	int m_iStringListPos = 0;      // string position
	int m_iLineNumber = 0, m_iColumn = 0;      // for error messages

	void clear() {
		m_iType = m_iVal = 0;
		m_iStringListPos = 0;
		m_iLineNumber = m_iColumn = 0;
	}
};

// CKeyTab -- keyword table element
class CKeyTab {
public:
	int m_iKeyValue;   // keyword value
	const char *m_xpszKeyString;     // string
};

// CMap -- bit map
class CMap {
	friend class CGtlData;

	char m_szLabel[MAX_LABEL_LENGTH] = { 0 };	// bitmap label
	char m_szFilename[MAX_FILENAME_LENGTH] = { 0 };	// file name of BMP file
	bool m_bPositionDetermined : 1;
	bool m_bSprite : 1;
	bool m_bPalette : 1;			// game palette from this bitmap
	bool m_bOverlay : 1;			// overlay onto background map
	bool m_bMasked : 1;				// bitmap is masked
	bool m_bMetaGame : 1;			// show bitmap only in metagame
	bool m_bRelocatable : 1;		// relocatable
	bool m_bPositionSpecified : 1;	// position specified on input
	bool m_bSpecialPaint : 1;		// don't automatically paint
	CBgbObject *m_lpcBgbObject = nullptr;		// Boffo game object for bitmap file

	int m_iRelationType = 0;       // KT_ABOVE, KT_BELOW, KT_LEFT, KT_RIGHT,
	// KT_NODE
	int m_iRelation = 0;           // index of relation bitmap or node

	CMap();
};

// CNode -- node on the map
class CNode {
public:
#ifdef NODEEDIT
	char m_szSector[MAX_LABEL_LENGTH]; // node sector
#endif
	char m_szLabel[MAX_LABEL_LENGTH];  // node label
	int m_iLinks[MAX_LINKS_PER_NODE];   // up to 10 links per node
	int m_iLocationCode;               // MG_LOC_xxxx to specify location
	int m_iSector;                     // MG_SECTOR_xxxx to specify sector
	int m_iBitmap;                     // index of related bitmap
	int m_iX, m_iY;                    // position
	int m_iNumLinks;                   // number of links
	bool m_bDeleted : 1;               // deleted node
	bool m_bSelected : 1;              // this is selected node
	bool m_bRelative : 1;              // relative to bitmap
	bool m_bRelocatable : 1;           // relocatable node
	bool m_bWgtSpec : 1;               // weight specified
	bool m_bSenSpec : 1;               // sensitivity specified
	bool m_bMenu : 1;                   // true if menu
	byte m_iWeight;                   // node weight
	byte m_iSensitivity;              // distance to accept clickb

	CNode();

	bool IfRelocatable() const {
		return m_bRelocatable;
	}
};

// CStratLocInfo -- strategy location information
class CStratLocInfo {
	friend class CGtlData;
	friend class CStrategyInfo;

	int m_iLocCode = 0;            // MG_LOC_xxxx -- location code
	CNode *m_lpNode = nullptr;      // node pointer for location
	int m_iValueCode = 0;          // MG_VISIT_xxxx or MG_WIN_xxxx
	// or 0 if not eligible
	int m_iDistance = 0;           // distance to location
	int m_iWeight = 0;             // weight of location
	int m_iAdjustedWeight = 0;     // weight adjusted by distance
};


// CStrategyInfo -- strategy information for determining best move
class CStrategyInfo {
	friend class CGtlData;

	char m_cStartData;
	CStratLocInfo *xpTargetLocInfo;   // target location (best move)

	int m_iRequiredObjectsCount;   // total number of objects I need
	int m_iRequiredObjectsTable[MAX_GAME_TABLE]; // objects I need

	int m_iMaximumDistance;    // max distance to eligible location
	int m_iMaxAdjustedWeight;  // maximum adjusted weight
	int m_iTopLocCount;        // number of top locations
	int m_iTopLocTable[MG_LOC_MAX];    // table of top locations

	char m_cEndData;

	CStratLocInfo m_cStratLocInfo[MG_LOC_MAX + 1];

	CStrategyInfo(void) {
		memset(&m_cStartData,
			0, &m_cEndData - &m_cStartData);
	}
};

// CXodj -- structure for Hodj Podj
class CXodj {
	friend class CGtlData;

public:
	CXodj *m_xpXodjNext = nullptr;      // next in chain
	char m_szName[MAX_LABEL_LENGTH] = { 0 };   // name of character
	bool m_bHodj = false;              // char is Hodj (not Podj)
	int m_iFurlongs = 0;           // number of furlongs available
	int m_iCharNode = 0;           // node where character is positioned

	int m_nTurns = 0;               // number of turns remaining (not furlongs)

	bool m_bComputer = false;          // played by computer
	CMap *m_lpMap = nullptr;        // bitmap object
	CBgbObject *m_lpcCharSprite = nullptr;  // character sprite bgb object
	CBgbObject *m_lpcIcon = nullptr;        // icon for character

	CBofSound *m_pThemeSound = nullptr;              // this character's MIDI theme song info
	const char *m_pszThemeFile = nullptr;
	uint    m_nThemeStart = 0;
	uint    m_nThemeEnd = 0;

	const char *m_pszStarsFile = nullptr;             // cel strip for stars animation

	// fields for computing computer play (also used for human player
	// for debugging or giving hints)
	class CStrategyInfo *m_xpStrategyInfo = nullptr; // strategy information

	// list of clue numbers for clues given by winning mini-game
	int m_iWinInfoWon = 0;     // how many clues I've received
	int m_iWinInfoNeed = 0;    // total number of clues I need
	int m_iWinInfoTable[MAX_GAME_TABLE] = { 0 };

	// list of clue numbers for clues given by farmer, etc.
	int m_iSecondaryInfoWon = 0;   // how many clues I've received
	int m_iSecondaryInfoNeed = 0;  // total number of clues I need
	int m_iSecondaryInfoTable[MAX_GAME_TABLE] = { 0 };

	// list of objects required to get Mish/Mosh
	int m_iRequiredObjectsCount = 0;   // total number of objects I need
	int m_iRequiredObjectsTable[MAX_GAME_TABLE] = { 0 }; // objects I need
	int m_iRequiredMoney = 0;      // money needed for Mish/Mosh

	// list of secondary information location we still have to visit
	int m_iSecondaryLoc[MAX_GAME_TABLE] = { 0 };

	int m_iGameHistory[20] = { 0 };    // last 20 mini-games played
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
private:
	/**
	 * Parse input line
	 * @returns		TRUE if error, FALSE otherwise

	 */
	bool parseLine();

	/**
	 * Parse integer, store into node structure
	 * @param xpLxel	Pointer to previous lexeme
	 * @param iPrevType	Type of previous lexeme
	 * @param iValue	Where integer value is to be stored
	 * @returns			Pointer to next lexeme
	 */
	CLexElement *ParseInteger(CLexElement *xpLxel,
		int iPrevType, int &iValue);

	/**
	 * Parse string, store into node structure
	 * Note: Accepts identifier as well as string
	 * @param xpLxel        Pointer to previous lexeme
	 * @param iPrevType     Type of previous lexeme
	 * @param lpszValue     Where string is to be stored, or NULL if not stored
	 * @param xpiValue      Where to store keyword value, or NULL if not stored
	 * @returns             Pointer to next lexeme
	 */
	CLexElement *ParseString(CLexElement *xpLxel,
		int iPrevType, char *lpszValue, int *xpiValue);

	/**
	 * Get bitmap or node label
	 */
	bool GetLabel(CLexElement *xpLxel,
		bool bNode, int &iIndex);

	/**
	 * Get bitmap or node label
	 * @param lpszLabel		Pointer to label string for bitmap/node being sought
	 * @param bNode		False for bitmaps, true for nodes
	 * @param iIndex	Output: index of bitmap or node for label
	 * @returns			true if error (label not found), false otherwise
	 */
	bool GetLabel(char *lpszLabel,
		bool bNode, int &iIndex);

	/**
	 * Link together a pair of nodes
	 * @param lpNode1	Node to be linked
	 * @param lpNode2	Node to be linked
	 * @returns			True if error, false otherwise
	 */
	bool AddLink(CNode *lpNode1, CNode *lpNode2);

	/**
	 * Add link to one node
	 * @param lpNode	node to add link to
	 * @param iLink		Index of linked node
	 * @returns			True if error, false otherwise
	 */
	bool AddLink(CNode *lpNode, int iLink);

	/**
	 * Find keyword, given tree node type
	 * @param iType		KT_xxxx -- tree node type
	 * @returns			String pointer to keyword, or to "????"
	 */
	const char *FindKeyword(int iType);

	/**
	 * Read input line
	 * true if error, false otherwise
	 */
	bool ReadLine(void);

	/**
	 * Publish error message
	 * @returns		true if error, false otherwise
	 */
	bool ErrorMsg(const CLexElement *xpLxel, const char *szMessage);

public:
	class CGtlDoc *m_xpcGtlDoc = nullptr; // document pointer

private:
	CGenUtil m_cGenUtil;       // general utility object
	CBgbMgr m_cBgbMgr;         // Boffo Game Object manager
	CBbtMgr m_cBbtMgr;         // Boffo Games button manager
	CBbutton m_cMiniButton;    // minimap button
	CBbutton m_cInvButton;     // inventory button
	CBbutton m_cScrollButton;  // scroll button

	CMap *m_lpMaps /* [MAX_BITMAPS] */ = nullptr; // bitmap array
	CNode *m_lpNodes /* [MAX_NODES] */ = nullptr;     // node array

	bool m_bGtlDll = false;            // equals GTLDLL define
	char m_szBmpDirectory[MAX_FILENAME_LENGTH] = { 0 }; // bitmap file director
	char m_szGtlFile[MAX_FILENAME_LENGTH] = { 0 }; // gtl file name
	char m_szListFile[MAX_FILENAME_LENGTH] = { 0 }; // List file name
	Common::File m_xpGtlFile;        // ptr to file structure for .GTL file
	Common::OutSaveFile *m_xpListFile = nullptr;       // ptr to file structure for .LST file
	bool m_bListing = false;           // listing file flag
	int m_iIndent = 0;             // current indent (decompile only)
	int m_iLineNumber = 0;         // line number in input file
	int m_iMaps = 0, m_iNodes = 0;     // # of bitmaps, nodes
	bool m_bEof = false;               // end of file on input (or END stmt)
	CNode *m_lpLastNode = nullptr;  // pointer to last node
	int m_iNumGenLabels = 0;               // number of generated lables
	CLexElement *m_xpLexLabel = nullptr;        // current label lexeme
	CLexElement m_cLexElts[MAX_LEXELTS];       // lexical element blocks
	char m_szStringList[MAX_STRINGLIST] = { 0 };     // compiler input string
	bool m_bSelectedLink = false;      // flag: a link is selected
	int m_iSelLinkPt1 = 0, m_iSelLinkPt2 = 0;  // indexes of selected link pts
	TimeDate m_stAcceptClickActive;       // prevent recursion
	TimeDate m_stLDownTime;       // time of left button down
	CNode *m_lpFoundNode = nullptr; // node clicked on
	CNode *m_lpLastSelectedNode = nullptr;  // selected on click down

	bool m_bJustPlayedMiniGame; // true for Mini-Games in ProcessGameResult

	// fields for Draw routine
public:
	bool m_bInhibitDraw = false;       // inhibit any actual drawing
	bool m_bDrawing = false;           // in Draw routine (prevent recursion)
	CGtlView *m_xpGtlView = nullptr;    // current view
	bool m_bPaint = false;             // WM_PAINT flag
	bool m_bRelocatable = false;       // device context is currently
	// relocatable
	Common::Rect *m_xpClipRect = nullptr;      // relocatable clipping rectangle
	Common::Rect *m_xpDevClipRect = nullptr;   // device (non-relocatable)
	// clipping rectangle
	CMap *m_lpFurlongMaps[25] = { nullptr }; // 0-24 furlong bitmaps

	// ---
public:
	int m_iSizeX = 0, m_iSizeY = 0;    // size of entire client area
	// (entire bitmap)
	int m_iMargin = 0;             // size of top margin of view window
	bool m_bChangeSize = false;        // size of window changed (because
	// gtl modified the bitmap template)
	int m_iViewSizeX = 0, m_iViewSizeY = 0;    // size of view

	bool m_bPaintBackground = false;   // paint background
	bool m_bShowNodes = false;         // show nodes and links
	bool m_bShowSensitivity = false;           // show node sensitivities
	bool m_bStartMetaGame = false;     // start in meta game mode
	bool m_bMetaGame = false;          // meta game mode
	bool m_bInitMetaGame = false;      // init sprites at beginning of metagame

	CXodj *m_xpXodjChain = nullptr;     // chain of characters
	CXodj *m_xpCurXodj = nullptr;       // character currently on the move
	bool m_bSamePlace = false;         // both characters are in same place
	bool m_bGameOver = false;          // game is over

	GfxSurface *m_pOffScreenBmp = nullptr;
	CPalette *m_pPalOld = nullptr;
#ifdef TODO
	HBITMAP   m_hOldBitmap = nullptr;
#endif
	int m_iMishMoshLoc = 0;        // MG_LOC_xxxx -- Mish/Mosh location

	COLORREF m_cNodePenColor = 0, m_cNodeBrushColor = 0, m_cLinkPenColor;
	COLORREF m_cNodeSensitivityColor = 0;
	COLORREF m_cNodePassThruColor = 0, m_cNodeHeavyColor = 0;
	COLORREF m_cSelNodePenColor = 0, m_cSelNodeBrushColor = 0;
	COLORREF m_cSelLinkPenColor = 0;
	COLORREF m_cBackColor = 0;

	// constructor zeroes out all fields and allocates arrays
public:
	void ClearData(void) {
		error("TODO: cleardata");
	}

	// gtldcp.cpp -- decompiler for meta game

	//- Decompile -- output data to .GTL file
public: int Decompile(const char *xpszPathName);
	  //- AsciiOutput -- ascii output
private: int AsciiOutput(int iIndent, const char *lpszOut);
	   //- ListingOutput -- listing output
private: int ListingOutput(int iIndent, const char *lpszOut);


	   // gtlcpl.cpp -- compiler for graphics utility

public:
	/**
	 * Compile .gtl file to internal objects
	 */
	int compile(const char *xpszPathName);


	   // gtlui.cpp -- data interface to Windows

	   //- CGtlData -- constructor -- zero out all fields, allocate
	   //      arrays, and set colors
public:
	CGtlData(void);
	  //- ~CGtlData -- destructor
public:
	~CGtlData(void);

	void    CreateOffScreenBmp(void);
	void    DeleteOffScreenBmp(void);
	void    PaintOffScreenBmp(void);
	CMemDC *GetMemDC(void);
	void    ReleaseMemDC(CMemDC *);


	  //- SetColors -- set colors of various elements
public: bool SetColors(void);
	  //- ClearInhibitDraw -- clear m_bInhibitDraw flag
private: bool ClearInhibitDraw(void);
	   //- Draw -- draw data
public: bool Draw(CGtlView *xpGtlView, Common::Rect *xpClipRect,
	CDC *xpDc PDFT(NULL));
	  //- SwitchDc -- switch between relocatable/nonrelocatable
	  //              device context
private: bool SwitchDc(CDC *xpDc, bool bRelocatable);
	   //- DrawBitmaps -- draw bitmaps for Draw routine
private: bool DrawBitmaps(CDC *, bool);

	   //- DrawABitmap -- draw a bitmap object for Draw routine
private: bool DrawABitmap(CDC *, CBgbObject *, bool bPaint PDFT(false));

	   //- NodeToPoint -- return relocatable coordinates of node
	   //              adjusted by optional size object
private: CRPoint NodeToPoint(CNode *lpNode,
	CSize *lpcSize PDFT(NULL));
	   //- SpecifyUpdate -- specify update rectangle
public: bool SpecifyUpdate(CGtlView *xpGtlView);
	  //- UpdateDialogs -- update modeless dialog boxes
public: bool UpdateDialogs(bool bRetrieve PDFT(false));
	  //- UpdateMenuDialog -- update control dialog box
private: bool UpdateMenuDialog(bool bRetrieve);

	   //- UpdateInfoDialog -- update information dialog box
private: bool UpdateInfoDialog(bool bRetrieve);

	   //- UpdateControlDialog -- update control dialog box
private: bool UpdateControlDialog(bool bRetrieve);

	   //- UpdateNodeDialog -- update node dialog box
private: bool UpdateNodeDialog(bool bRetrieve);

	   //- UpdateFocusRect -- update focus rectangle for hiliting
private: bool UpdateFocusRect(void);

	   //- AdjustToView -- set up bitmaps for view
private: bool AdjustToView(CGtlView *);

	   //- InitOverlay -- initialize overlay
private: bool InitOverlay(CMap *lpMap);
	   //- NormalizeData -- normalize data after loading
private: bool NormalizeData(CGtlView *xpGtlView);

	   // gtlmanp.cpp -- manipulate the data

	   //- AcceptClick -- process a mouse click by user
public: bool AcceptClick(CGtlView *xpGtlView, const Common::Point &cClickPoint, int iClickType);
	  //- AcceptDeleteKey -- process depressing delete key
public: bool AcceptDeleteKey(CGtlView *xpGtlView);
	  //- PointToNode -- find node closest to point
private: CNode *PointToNode(CRPoint crPoint);
	   //- SelectNode -- algorithm when node is clicked
private: bool SelectNode(CNode *lpNode);
	   //- ModifySelectNode -- modify node select/deselect
private: bool ModifySelectNode(CNode *lpNode PDFT(NULL), bool bSelect PDFT(true));
	   //- GetSelectedNode -- return selected node, or NULL if none
private: CNode *GetSelectedNode(void);
	   //- SelectLink -- select or deselect a link
private: bool SelectLink(CNode *lpNode1 PDFT(NULL), CNode *lpNode2 PDFT(NULL));
	   //- IfLinked -- test whether two nodes are linked
private: bool IfLinked(CNode *lpNode1, CNode *lpNode2);
	   //- DeleteNode -- delete node and all connecting links
private: bool DeleteNode(CNode *lpNode);
	   //- DeleteLink -- delete link between two nodes
private: bool DeleteLink(CNode *lpNode1 PDFT(NULL), CNode *lpNode2 PDFT(NULL));
	   //- DeleteLinkIndex -- delete index from array of link
	   //              indices for a given node, if found
private: bool DeleteLinkIndex(CNode *lpNode, int iLink);
	   //- CallUpdate -- call to update all views for this map
private: bool CallUpdate(CNode *lpNode1 PDFT(NULL),
	CNode *lpNode2 PDFT(NULL), bool bLinks PDFT(false),
	bool bWmPaint PDFT(false));
	   //- CallUpdate -- call to update all views for this map
	   //              -- version which updates a bitmap object
private: bool CallUpdate(CBgbObject *lpcBgbObject);
	   //- CreateNode -- create a new node
private: bool CreateNode(CNode *&lpNode, CRPoint crPosition);
	   //- MoveNode -- move node
private: bool MoveNode(CNode *lpNode, CRPoint crPosition);

	   int DoSpecialTravel(int, bool);
	   void LoadCharDirection(CBgbObject *, const Common::Point &, const Common::Point &);

	   //- CreateLink -- create link between two nodes
private: bool CreateLink(CNode *lpNode1, CNode *lpNode2);
	   //- SetNodePosition -- set position relative to bitmap
private: bool SetNodePosition(CNode *lpNode, CRPoint crPosition);


	   // gtlmgm.cpp -- meta game

	   //- SetMetaGame -- set meta game on or off
public: bool SetMetaGame(bool bOn);
	  //- InitMetaGame -- init or release sprites for Meta Game
public: bool InitMetaGame(CGtlView *xpGtlView PDFT(NULL),
	bool bInit PDFT(true));
	  //- ProcessMove -- handle move processing
public: bool ProcessMove(CNode *lpTargetNode PDFT(NULL));
	  //- MoveCharToNode -- move current character to specified node
private: bool MoveCharToNode(CNode *lpTargetNode);
	   //- EstimatePathDistance -- estimate distance between nodes
private: int EstimatePathDistance(CNode *lpNode1,
	CNode *lpNode2);
	   //- FindShortestPath -- between two nodes
private:
	int *FindShortestPath(CNode *lpNode1, CNode *lpNode2);
	   //- PositionCharacters -- set positions for Hodj and Podj
public: bool PositionCharacters(void);
	  //- PositionACharacter -- find positions for Hodj and Podj
private: bool PositionACharacter(CXodj *xpXodj, int iShift);
	   //- LocationToNode -- find node for game location
private: CNode *LocationToNode(int iLocationCode);
public: int FindNodeId(char *pszLabel);


	  // gtlmve.cpp -- meta game move processing

	  //- InitProblem -- initialize character problem
private: bool InitProblem(void);
	   //- EndMoveProcessing -- handle things at end of a move
private: bool EndMoveProcessing(void);
	   //- SwitchPlayers -- switch between Hodj and Podj
public: bool SwitchPlayers(void);
	  //- InitInterface -- init data for interface manager
public: bool InitInterface(int iCode, bool &bExitDll);
	  //- ReturnFromInterface -- reset everything
	  //              after interface return
public: bool ReturnFromInterface(void);
	  //- TakeIneligibleAction -- take action if character is at
	  //      a location but is not eligible to perform function
private: bool TakeIneligibleAction(CXodj *xpXodj,
	int iFunctionCode, int iLocationCode);
	   //- DumpGameStatus -- (debugging) dump status of game
private: bool DumpGameStatus(int iOptionFlags
	PDFT(DUMPSTAT_EVERYTHING));
	   //- ProcessGameResult -- process result of game,
	   //              optionally generating a random win
private: bool ProcessGameResult(CXodj *xpXodj,
	int iGameCode, LPGAMESTRUCT lpGameStruct PDFT(NULL));

	   //- GainRandomItem -- player receives a random item from store
public:  bool GainRandomItem(CXodj *xpXodj);

	  //- GainMoney -- player has won some money
public:  bool GainMoney(CXodj *xpXodj, long lCrowns);

	  //- GainMishMosh -- player gets MishMosh
public:  bool GainMishMosh(CXodj *xpXodj, long lCrowns);

	  //- DivulgeInformation -- divulge information to player
public:  bool DivulgeInformation(CXodj *xpXodj, bool bSecondary);

	  //- DetermineEligibility -- determine whether
	  //  player has objects/money needed for current location
private: bool DetermineEligibility(CXodj *xpXodj, int, int &, bool bExecute PDFT(false));

	   //- DetermineGameEligibility -- determine if eligible to play
	   //  a game, and update game history if so (and bExecute)
	   // ***** modify this function to determine how often a user can
	   //  play the same game over and over
private: bool DetermineGameEligibility(CXodj *xpXodj, int iGameCode, bool bExecute PDFT(false));

	   //- GetGameObjectCount -- get the count of the specified
	   //      object in the player's inventory
private: long GetGameObjectCount(CXodj *xpXodj, int iObjectCode);

	   //- DetermineInfoEligibility -- determine if eligible to
	   //  get information at this location
private: bool DetermineInfoEligibility(CXodj *xpXodj,
	int iLocationCode, bool bExecute PDFT(false));

	   //- DetermineMishMoshEligibility -- determine if eligible to
	   //  grab Mish and Mosh from current location
private: bool DetermineMishMoshEligibility(CXodj *, int);

	   void DoTransport(CXodj *, int);


	   // gtlcmp.cpp -- meta game computer play

	   //- SelectBestMove -- select best move for computer play
	   //      (can also be used for human player for debugging or giving hints)
private: bool SelectBestMove(CXodj *xpXodj);
	   //- GatherInformation -- gather info to decide computer's move
private: bool GatherInformation(CXodj *xpXodj);
	   //- DetermineWeights -- determine weight for each
	   //              location (independent of distance)
private: bool DetermineWeights(CXodj *xpXodj);
	   //- DetermineDistances -- determine distance to each
	   //                      eligible location
private: bool DetermineDistances(CXodj *xpXodj);
	   //- AdjustWeightByDistance -- adjust each by the distance
	   //              of the node
private: bool AdjustWeightByDistance(CXodj *xpXodj);
	   //- FindTopLocations -- find locations with top weights
private: bool FindTopLocations(CXodj *xpXodj);
	   //- DumpBestMoveData -- dump data used in best move computation
private: bool DumpBestMoveData(CXodj *xpXodj);

private:
	void CheckForTransport(CXodj *, int);
	void SetFurlongs(CXodj *);
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
