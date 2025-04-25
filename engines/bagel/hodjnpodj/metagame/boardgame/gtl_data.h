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

// Defining distance between nodes
inline long SQ(int y) {
	return (long)((long)y * (long)y);
}
inline long SQ(long y) {
	return y * y;
}
inline long LSQ(const Common::Point &p1, const Common::Point &p2) {
	return SQ(p1.x - p2.x) + SQ(p1.y - p2.y);
}

// Lexical element types
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


// Keyword codes
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

// Object types
#define OBJ_NONE 0
#define OBJ_BITMAP 301
#define OBJ_NODE 302
#define OBJ_LINK 303

// Buttons
#define BUTTON_MINIMAP          1
#define BUTTON_INVENTORY        2
#define BUTTON_SCROLL           3

// Node types
#define NT_GAME_MIN 100         /* use BFCG_xxxx game code */
#define NT_GAME_MAX 200
#define NT_MENU 201             /* meta-game menu */
#define NT_OVERLAY 202          /* tied to overlay */

// Codes for DumpGameStatus
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

/**
 * Lexical element class
 */
class CLexElement {
	friend class CGtlData;
	int m_iType = 0;			// Lexical type -- LXT_xxxx
	int m_iVal = 0;				// Integer or char or keyword code
	int m_iStringListPos = 0;	// String position
	int m_iLineNumber = 0, m_iColumn = 0;      // for error messages

	void clear() {
		m_iType = m_iVal = 0;
		m_iStringListPos = 0;
		m_iLineNumber = m_iColumn = 0;
	}
};

/**
 * Keyword table element
 */
class CKeyTab {
public:
	int m_iKeyValue;				// Keyword value
	const char *m_xpszKeyString;	// String
};

/**
 * Bit map
 */
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

/**
 * Node on the map
 */
class CNode {
public:
#ifdef NODEEDIT
	char m_szSector[MAX_LABEL_LENGTH]; // node sector
#endif
	char m_szLabel[MAX_LABEL_LENGTH] = { 0 };	// Node label
	int m_iLinks[MAX_LINKS_PER_NODE] = { 0 };	// Up to 10 links per node
	int m_iLocationCode = 0;	// MG_LOC_xxxx to specify location
	int m_iSector = 0;			// MG_SECTOR_xxxx to specify sector
	int m_iBitmap = 0;			// Index of related bitmap
	int m_iX = 0, m_iY = 0;		// Position
	int m_iNumLinks = 0;		// Number of links
	bool m_bDeleted : 1;		// Deleted node
	bool m_bSelected : 1;		// This is selected node
	bool m_bRelative : 1;		// Relative to bitmap
	bool m_bRelocatable : 1;	// Relocatable node
	bool m_bWgtSpec : 1;		// Weight specified
	bool m_bSenSpec : 1;		// Sensitivity specified
	bool m_bMenu : 1;			// true if menu
	byte m_iWeight = 0;			// Node weight
	byte m_iSensitivity = 0;	// Distance to accept clickb

	CNode();

	bool IfRelocatable() const {
		return m_bRelocatable;
	}
};

/**
 * Strategy location information
 */
class CStratLocInfo {
	friend class CGtlData;
	friend class CStrategyInfo;

	int m_iLocCode = 0;			// MG_LOC_xxxx -- location code
	CNode *m_lpNode = nullptr;	// Node pointer for location
	int m_iValueCode = 0;		// MG_VISIT_xxxx or MG_WIN_xxxx
								// or 0 if not eligible
	int m_iDistance = 0;		// Distance to location
	int m_iWeight = 0;			// Weight of location
	int m_iAdjustedWeight = 0;	// Weight adjusted by distance
};

/**
 * Strategy information for determining best move
 */
class CStrategyInfo {
	friend class CGtlData;

	CStratLocInfo *xpTargetLocInfo = nullptr;	// Target location (best move)

	int m_iRequiredObjectsCount = 0;	// Total number of objects I need
	int m_iRequiredObjectsTable[MAX_GAME_TABLE] = { 0 }; // Objects I need

	int m_iMaximumDistance = 0;			// Max distance to eligible location
	int m_iMaxAdjustedWeight = 0;		// Maximum adjusted weight
	int m_iTopLocCount = 0;				// Number of top locations
	int m_iTopLocTable[MG_LOC_MAX] = { 0 };	// Table of top locations

	CStratLocInfo m_cStratLocInfo[MG_LOC_MAX + 1];

	CStrategyInfo() {}
};

/**
 * Structure for Hodj Podj
 */
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

/**
 * Data class for graphics utility
 */
class CGtlData {
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
	 * @param lpszValue     Where string is to be stored, or nullptr if not stored
	 * @param xpiValue      Where to store keyword value, or nullptr if not stored
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
	bool ReadLine();

	/**
	 * Publish error message
	 * @returns		true if error, false otherwise
	 */
	bool ErrorMsg(const CLexElement *xpLxel, const char *szMessage);

	/**
	 * Clear m_bInhibitDraw flag
	 */
	bool ClearInhibitDraw();

	/**
	 * Switch between relocatable/nonrelocatable device context
	 */
	bool SwitchDc(CDC *xpDc, bool bRelocatable);

	/**
	 * Draw bitmaps for Draw routine
	 */
	bool DrawBitmaps(CDC *, bool);

	/**
	 * Draw a bitmap object for Draw routine
	 */
	bool DrawABitmap(CDC *, CBgbObject *, bool bPaint PDFT(false));

	/**
	 * Return relocatable coordinates of node
	 * adjusted by optional size object
	 * @param lpNode	Pointer to node
	 * @param lpcSize	Pointer to size of bitmap sitting on node
	 * @returns			true if error, false otherwise
	 */
	CRPoint NodeToPoint(CNode *lpNode,
		CSize *lpcSize = nullptr);

	/**
	 * Update control dialog box
	 */
	bool UpdateMenuDialog(bool bRetrieve);

	/**
	 * Update information dialog box
	 */
	bool UpdateInfoDialog(bool bRetrieve);

	/**
	 * Update control dialog box
	 */
	bool UpdateControlDialog(bool bRetrieve);

	/**
	 * Update node dialog box
	 */
	bool UpdateNodeDialog(bool bRetrieve);

	/**
	 * Update focus rectangle for hiliting
	 */
	bool UpdateFocusRect();

	/**
	 * Set up bitmaps for view
	 */
	bool AdjustToView(CGtlView *);

	/**
	 * Initialize overlay
	 */
	bool InitOverlay(CMap *lpMap);

	/**
	 * Normalize data after loading
	 */
	bool NormalizeData(CGtlView *xpGtlView);

	/**
	 * Find node closest to point
	 */
	CNode *PointToNode(CRPoint crPoint);

	/**
	 * Algorithm when node is clicked
	 */
	bool SelectNode(CNode *lpNode);

	/**
	 * Modify node select/deselect
	 */
	bool ModifySelectNode(CNode *lpNode PDFT(nullptr), bool bSelect PDFT(true));

	/**
	 * Return selected node, or nullptr if none
	 */
	CNode *GetSelectedNode();

	/**
	 * Select or deselect a link
	 */
	bool SelectLink(CNode *lpNode1 PDFT(nullptr), CNode *lpNode2 PDFT(nullptr));

	/**
	 * Test whether two nodes are linked
	 */
	bool IfLinked(CNode *lpNode1, CNode *lpNode2);

	/**
	 * Delete node and all connecting links
	 */
	bool DeleteNode(CNode *lpNode);

	/**
	 * Delete link between two nodes
	 */
	bool DeleteLink(CNode *lpNode1 PDFT(nullptr), CNode *lpNode2 PDFT(nullptr));

	/**
	 * Delete index from array of link
	 * indices for a given node, if found
	 */
	bool DeleteLinkIndex(CNode *lpNode, int iLink);

	/**
	 * Call to update all views for this map
	 */
	bool CallUpdate(CNode *lpNode1 PDFT(nullptr),
		CNode *lpNode2 PDFT(nullptr), bool bLinks PDFT(false),
		bool bWmPaint PDFT(false));

	/**
	 * Call to update all views for this map
	 * -- version which updates a bitmap object
	 */
	bool CallUpdate(CBgbObject *lpcBgbObject);

	/**
	 * Create a new node
	 */
	bool CreateNode(CNode *&lpNode, CRPoint crPosition);

	/**
	 * Move node
	 */
	bool MoveNode(CNode *lpNode, CRPoint crPosition);

	int DoSpecialTravel(int, bool);
	void LoadCharDirection(CBgbObject *, const CPoint &,
		const CPoint &);

	/**
	 * Create link between two nodes
	 */
	bool CreateLink(CNode *lpNode1, CNode *lpNode2);

	/**
	 * Set position relative to bitmap
	 */
	bool SetNodePosition(CNode *lpNode, CRPoint crPosition);

	/**
	 * Move current character to specified node
	 */
	bool MoveCharToNode(CNode *lpTargetNode);

	/**
	 * Estimate distance between nodes
	 */
	int EstimatePathDistance(CNode *lpNode1,
		CNode *lpNode2);

	/**
	 * Find the shortest path between two nodes
	 */
	int *FindShortestPath(CNode *lpNode1, CNode *lpNode2);

	/**
	 * Find positions for Hodj and Podj
	 */
	bool PositionACharacter(CXodj *xpXodj, int iShift);

	/**
	 * Find node for game location
	 */
	CNode *LocationToNode(int iLocationCode);

	/**
	 * Initialize character problem
	 */
	bool InitProblem();

	/**
	 * Handle things at end of a move
	 */
	bool EndMoveProcessing();

	/**
	 * Take action if character is at
	 * a location but is not eligible to perform function
	 */
	bool TakeIneligibleAction(CXodj *xpXodj,
		int iFunctionCode, int iLocationCode);

	/**
	 * (Debugging) dump status of game
	 */
	bool DumpGameStatus(int iOptionFlags
		PDFT(DUMPSTAT_EVERYTHING));

	/**
	 * Process result of game, optionally generating
	 * a random win
	 */
	bool ProcessGameResult(CXodj *xpXodj,
		int iGameCode, LPGAMESTRUCT lpGameStruct PDFT(nullptr));

	/**
	 * Determine whether player has objects/money
	 * needed for current location
	 */
	bool DetermineEligibility(CXodj *xpXodj, int, int &, bool bExecute PDFT(false));

	/**
	 * Determine if eligible to play a game,
	 * and update game history if so (and bExecute)
	 * **** modify this function to determine how often a user can
	 * play the same game over and over
	 */
	bool DetermineGameEligibility(CXodj *xpXodj, int iGameCode, bool bExecute PDFT(false));

	/**
	 * Get the count of the specified object in
	 * the player's inventory
	 */
	long GetGameObjectCount(CXodj *xpXodj, int iObjectCode);

	/**
	 * Determine if eligible to get information
	 * at this location
	 */
	bool DetermineInfoEligibility(CXodj *xpXodj,
		int iLocationCode, bool bExecute PDFT(false));

	/**
	 * Determine if eligible to grab Mish and Mosh
	 * from current location
	 */
	bool DetermineMishMoshEligibility(CXodj *, int);

	void DoTransport(CXodj *, int);

	/**
	 * Select best move for computer play (can also be used
	 * for human player for debugging or giving hints)
	 */
	bool SelectBestMove(CXodj *xpXodj);

	/**
	 * Gather info to decide computer's move
	 */
	bool GatherInformation(CXodj *xpXodj);

	/**
	 * Determine weight for each location
	 * (independent of distance)
	 */
	bool DetermineWeights(CXodj *xpXodj);

	/**
	 * Determine distance to each eligible location
	 */
	bool DetermineDistances(CXodj *xpXodj);

	/**
	 * Adjust each by the distance of the node
	 */
	bool AdjustWeightByDistance(CXodj *xpXodj);

	/**
	 * Find locations with top weights
	 */
	bool FindTopLocations(CXodj *xpXodj);

	/**
	 * Dump data used in best move computation
	 */
	bool DumpBestMoveData(CXodj *xpXodj);

	void CheckForTransport(CXodj *, int);
	void SetFurlongs(CXodj *);

public:
	class CGtlDoc *m_xpcGtlDoc = nullptr;	// Document pointer

	// Fields for Draw routine
	bool m_bInhibitDraw = false;		// Inhibit any actual drawing
	bool m_bDrawing = false;			// In Draw routine (prevent recursion)
	CGtlView *m_xpGtlView = nullptr;	// Current view
	bool m_bPaint = false;				// WM_PAINT flag
	bool m_bRelocatable = false;		// device context is currently relocatable
	Common::Rect *m_xpClipRect = nullptr;      // Relocatable clipping rectangle
	Common::Rect *m_xpDevClipRect = nullptr;   // Device (non-relocatable)
	// Clipping rectangle
	CMap *m_lpFurlongMaps[25] = { nullptr }; // 0-24 furlong bitmaps

	// ---
public:
	int m_iSizeX = 0, m_iSizeY = 0;		// Size of entire client area
	// (entire bitmap)
	int m_iMargin = 0;					// Size of top margin of view window
	bool m_bChangeSize = false;			// Size of window changed (because
	// gtl modified the bitmap template)
	int m_iViewSizeX = 0, m_iViewSizeY = 0;    // size of view

	bool m_bPaintBackground = false;	// Paint background
	bool m_bShowNodes = false;			// Show nodes and links
	bool m_bShowSensitivity = false;	// Show node sensitivities
	bool m_bStartMetaGame = false;		// Start in meta game mode
	bool m_bMetaGame = false;			// Meta game mode
	bool m_bInitMetaGame = false;		// Init sprites at beginning of metagame

	CXodj *m_xpXodjChain = nullptr;		// Chain of characters
	CXodj *m_xpCurXodj = nullptr;		// Character currently on the move
	bool m_bSamePlace = false;			// Both characters are in same place
	bool m_bGameOver = false;			// Game is over

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
	CGtlData();
	~CGtlData();

	void ClearData() {
		error("TODO: cleardata");
	}

	/**
	 * Compile .gtl file to internal objects
	 */
	int compile(const char *xpszPathName);

	void CreateOffScreenBmp();
	void DeleteOffScreenBmp();
	void PaintOffScreenBmp();
	CMemDC *GetMemDC();
	void ReleaseMemDC(CMemDC *);

	/**
	 * Set colors of various elements
	 */
	bool SetColors();

	/**
	 * Draw data
	 */
	bool Draw(CGtlView *xpGtlView, Common::Rect *xpClipRect,
		CDC *xpDc PDFT(nullptr));

	/**
	 * Specify update rectangle
	 */
	bool SpecifyUpdate(CGtlView *xpGtlView);

	/**
	 * update modeless dialog boxes
	 */
	bool UpdateDialogs(bool bRetrieve PDFT(false));

	/**
	 * Process a mouse click by user
	 */
	bool AcceptClick(CGtlView *xpGtlView, const Common::Point &cClickPoint, int iClickType);

	/**
	 * Process depressing delete key
	 */
	bool AcceptDeleteKey(CGtlView *xpGtlView);

	// gtlmgm.cpp -- meta game
	/**
	 * Set meta game on or off
	 */
	bool SetMetaGame(bool bOn);

	/**
	 * Init or release sprites for Meta Game
	 */
	bool InitMetaGame(CGtlView *xpGtlView PDFT(nullptr),
		bool bInit PDFT(true));

	/**
	 * Handle move processing
	 */
	bool ProcessMove(CNode *lpTargetNode PDFT(nullptr));

	int FindNodeId(char *pszLabel);

	/**
	 * Switch between Hodj and Podj
	 */
	bool SwitchPlayers();

	/**
	 * Init data for interface manager
	 */
	bool InitInterface(int iCode, bool &bExitDll);

	/**
	 * Reset everything
	 * After interface return
	 */
	bool ReturnFromInterface();

	/**
	 * Player receives a random item from store
	 */
	bool GainRandomItem(CXodj *xpXodj);

	/**
	 * Player has won some money
	 */
	bool GainMoney(CXodj *xpXodj, long lCrowns);

	/**
	 * Player gets MishMosh
	 */
	bool GainMishMosh(CXodj *xpXodj, long lCrowns);

	/**
	 * Divulge information to player
	 */
	bool DivulgeInformation(CXodj *xpXodj, bool bSecondary);

	/**
	 * Set positions for Hodj and Podj
	 */
	bool PositionCharacters();
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
