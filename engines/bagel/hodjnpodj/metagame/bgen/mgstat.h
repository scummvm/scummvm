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

#ifndef BAGEL_METAGAME_BGEN_MGSTAT_H
#define BAGEL_METAGAME_BGEN_MGSTAT_H

#include "bagel/hodjnpodj/hnplibs/gamedll.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define BOOBYSOUND(n) nullptr

// character codes
#define MG_CHAR_HODJ            100
#define MG_CHAR_PODJ            101


// game codes

#define MG_GAME_BASE            110 /* Base */

#define MG_GAME_ANAGRAMS        110 /* O. Anagrams */
#define MG_GAME_ARCHEROIDS      111 /* A. Archeroids */
#define MG_GAME_ARTPARTS        112 /* S. Art Parts */
#define MG_GAME_BARBERSHOP      113 /* U. Barbershop Quintet */
#define MG_GAME_BATTLEFISH      114 /* X. Battlefish */
#define MG_GAME_BEACON          115 /* H. Beacon */
#define MG_GAME_CRYPTOGRAMS     116 /* Q. Cryptograms */
#define MG_GAME_DAMFURRY        117 /* G. Dam Furry Animals */
#define MG_GAME_EVERYTHING      118 /* Y. Everything Under the Sun */
#define MG_GAME_FUGE            119 /* D. Fuge */
#define MG_GAME_GARFUNKEL       120 /* Garfunkel */
#define MG_GAME_LIFE            121 /* F. Life */
#define MG_GAME_MANKALA         122 /* W. Mankala */
#define MG_GAME_MAZEODOOM       123 /* L. Maze o'Doom */
#define MG_GAME_NOVACANCY       124 /* M. No Vacancy */
#define MG_GAME_PACRAT          125 /* C. Pac-Rat */
#define MG_GAME_PEGGLEBOZ       126 /* V. Peggleboz */
#define MG_GAME_PINBALL         127 /* B. Pinball */
#define MG_GAME_RIDDLES         128 /* T. Riddles */
#define MG_GAME_ROULETTE        129 /* J. Roulette */
#define MG_GAME_SHOTMACHINE     130 /* I. Shot Machine */
#define MG_GAME_SPINBALL        131 /* E. Spinball */
#define MG_GAME_THGESNGGME      132 /* N. TH GESNG GME */
#define MG_GAME_TRIVIA          133 /* R. Trivia */
#define MG_GAME_VIDEOPOKER      134 /* K. Video Poker */
#define MG_GAME_WORDSEARCH      135 /* P. Word Search */
#define MG_GAME_CHALLENGE       136 /* P. Word Search */

#define MG_GAME_MAX             136 /* max */
#define MG_GAME_COUNT   (MG_GAME_MAX - MG_GAME_BASE + 1) /* Count */

// codes for inventory objects
#define MG_OBJ_BASE             200 /* base number for objects */

#define MG_OBJ_ANVIL            200 /* Radio-Controlled Flying Anvil */
#define MG_OBJ_BERET            201 /* Beret of Invisibilty */
#define MG_OBJ_BROOMSTICK       202 /* Broomstick */
#define MG_OBJ_CANDLE           203 /* Scented Candle */
#define MG_OBJ_CHESTNUT         204 /* Magic Chestnut */
#define MG_OBJ_CHISEL           205 /* Chisel of Death */
#define MG_OBJ_CROWN            206 /* Gold Crown */
#define MG_OBJ_CRUSOE           207 /* Copy of \"Robinson Crusoe\" */
#define MG_OBJ_EGG              208 /* Exploding Eggs */
#define MG_OBJ_EGGBEATER        209 /* Eggbeater */
#define MG_OBJ_EYEPATCH         210 /* Eyepatch */
#define MG_OBJ_FISHHOOK         211 /* Fishhook */
#define MG_OBJ_FLASHLIGHT       212 /* Heavy-Duty Flashlight */
#define MG_OBJ_GRAPNEL          213 /* Grappling Hook */
#define MG_OBJ_HACKSAW          214 /* Killer Hacksaw */
#define MG_OBJ_HERRING          215 /* Red herring object */
#define MG_OBJ_HODJ_NOTEBOOK    216 /* Hodj's Log */
#define MG_OBJ_HONEY            217 /* Pot of Honey */
#define MG_OBJ_KUMQUAT          218 /* Enchanted Kumquat */
#define MG_OBJ_MIRROR           219 /* Mirror of Deflection */
#define MG_OBJ_MISH             220 /* Mish */
#define MG_OBJ_MOSH             221 /* Mosh */
#define MG_OBJ_PADDLE           222 /* Kayak Paddle */
#define MG_OBJ_PAN              223 /* Bundt Pan */
#define MG_OBJ_PODJ_NOTEBOOK    224 /* Podj's Lo */
#define MG_OBJ_ROPE             225 /* Extra-Long Jump Rope */
#define MG_OBJ_SCARAB           226 /* Poison Scarab */
#define MG_OBJ_SHARPENER        227 /* Enchanted Pencil Sharpener */
#define MG_OBJ_SHELL            228 /* Scallop Shell */
#define MG_OBJ_SHOELACE         229 /* Giant Shoelace */
#define MG_OBJ_STAFF            230 /* Magic Staff */
#define MG_OBJ_SWAB             231 /* Killer Cotton Swab */
#define MG_OBJ_SWATTER          232 /* Flyswatter of Death */
#define MG_OBJ_TICKET           233 /* General Admission Ticket */

#define MG_OBJ_MAX              233  /* max */
#define MG_OBJ_COUNT    (MG_OBJ_MAX - MG_OBJ_BASE + 1)
/* total number of defined objects */

// Locations

#define MG_LOC_BASE             300

#define MG_LOC_AERIE            300 /* Aerie  */
#define MG_LOC_AMPHI            301 /* Amphitheatre  */
#define MG_LOC_ANCIENTRUIN      302 /* Ancient Ruin */
#define MG_LOC_ARTISTCOTT       303 /* Artist's Cottage */
#define MG_LOC_BANDITHIDE       304 /* Bandit's Hideout */
#define MG_LOC_BARBERSHOP       305 /* Barber Shop */
#define MG_LOC_BARN             306 /* Barn */
#define MG_LOC_BEARCAVE         307 /* Bear Cave */
#define MG_LOC_BEAVER           308 /* Beaver Dam */
#define MG_LOC_BOARDING         309 /* Boarding House */
#define MG_LOC_CASTLE           310 /* Castle */
#define MG_LOC_CORRAL           311 /* Corral */
#define MG_LOC_CRYPT            312 /* Crypt */
#define MG_LOC_FARMHOUSE        313 /* Farmhouse */
#define MG_LOC_FERRISWHEEL      314 /* Ferris Wheel */
#define MG_LOC_FISHMARKET       315 /* Fish Market */
#define MG_LOC_GENERALSTR       316 /* General Store */
#define MG_LOC_HEDGEMAZE        317 /* Hedge Maze */
#define MG_LOC_HERMITTENT       318 /* Hermit's Tent */
#define MG_LOC_INN              319 /* Inn */
#define MG_LOC_LIGHTHOUSE       320 /* Lighthouse */
#define MG_LOC_MANSION          321 /* Country Mansion */
#define MG_LOC_MERMAIDROCK      322 /* Mermaid Rock */
#define MG_LOC_MINE             323 /* Mine */
#define MG_LOC_MTKRAK           324 /* Mount Krakanukkul */
#define MG_LOC_OASIS            325 /* Oasis */
#define MG_LOC_PAWNSHOP         326 /* Pawn Shop */
#define MG_LOC_PIRATECOVE       327 /* Pirate Cove */
#define MG_LOC_POSTOFFICE       328 /* Post Office */
#define MG_LOC_SHERIFF          329 /* Sheriff's Office */
#define MG_LOC_SILO             330 /* Silo */
#define MG_LOC_TAVERN           331 /* Tavern */
#define MG_LOC_TEMPLE           332 /* Temple */
#define MG_LOC_TOURNAMENT       333 /* Tournament Field */
#define MG_LOC_TRADINGPOST      334 /* Trading Post */
#define MG_LOC_WARLOCKLAIR      335 /* Warlock's Lair */
#define MG_LOC_WINDMILL         336 /* Windmill */
#define MG_LOC_WITCHHOVEL       337 /* Witch's Hovel */
#define MG_LOC_WOODCUTTER       338 /* Woodcutter's Cottage */
#define MG_LOC_BOAT1            339
#define MG_LOC_BOAT2            340
#define MG_LOC_BOAT3            341
#define MG_LOC_SKY1             342
#define MG_LOC_SKY2             343
#define MG_LOC_CAR1             344
#define MG_LOC_SECRET1          345
#define MG_LOC_SECRET2          346
#define MG_LOC_HODJ             347

#define MG_LOC_HERRING          348 /* Red herring location */

#define MG_LOC_MAX              348
#define MG_LOC_COUNT    (MG_LOC_MAX - MG_LOC_BASE + 1)

// Sector definitions

#define MG_SECTOR_BASE          360

#define MG_SECTOR_ANY           360 /* Any sector */
#define MG_SECTOR_BRIDGE        361 /* Bridge */
#define MG_SECTOR_CHUCKLE       362 /* Cape Chuckle */
#define MG_SECTOR_DESERT        363 /* Desert */
#define MG_SECTOR_ESTATE        364 /* The Estate */
#define MG_SECTOR_FARM          365 /* Farm */
#define MG_SECTOR_GLACIER       366 /* Glacier */
#define MG_SECTOR_HILLSIDE      367 /* Hillside */
#define MG_SECTOR_LAKE          368 /* Lake View */
#define MG_SECTOR_MEADOW        369 /* Meadow */
#define MG_SECTOR_MLPARK        370 /* Mistle-Laney Park */
#define MG_SECTOR_MOUNTAINS     371 /* Mountains */
#define MG_SECTOR_MOUNTAINVIEW  372 /* Mountain View */
#define MG_SECTOR_PASTEESH      373 /* Pasteesh */
#define MG_SECTOR_PATSHWERQUE   374 /* Patshwerque */
#define MG_SECTOR_PLAINS        375 /* Plains */
#define MG_SECTOR_POPORREE      376 /* Po-Porree City */
#define MG_SECTOR_RIVER         377 /* River View */
#define MG_SECTOR_SWAMP         378 /* Edge of Swamp */
#define MG_SECTOR_VOLCANO       379 /* Volcano View */
#define MG_SECTOR_WITTY         380 /* Witty Woods */

#define MG_SECTOR_MAX           380
#define MG_SECTOR_COUNT         (MG_SECTOR_MAX - MG_SECTOR_BASE + 1)

// location visit codes -- actions you can take at a location where
// there's no game defined
#define MG_VISIT_NONE           400
#define MG_VISIT_MISHMOSH       401
#define MG_VISIT_CASTLE         402
#define MG_VISIT_INFO           403
#define MG_VISIT_GENERALSTR     404     /* general store: buy objects */
#define MG_VISIT_TRADINGPOST    405     /* trading post exch money/objs */
#define MG_VISIT_BLACKMARKET    406     /* buy objects */
#define MG_VISIT_PAWNSHOP       407     /* sell objects */

#define MG_SPECIAL_VISIT_BASE   410
#define MG_VISIT_BOAT1          410
#define MG_VISIT_BOAT2          411
#define MG_VISIT_BOAT3          412
#define MG_VISIT_BOAT4          413
#define MG_VISIT_SKY1           414
#define MG_VISIT_SKY2           415
#define MG_VISIT_CAR1           416
#define MG_VISIT_OASIS          417
#define MG_VISIT_SECRET1        418
#define MG_VISIT_SECRET2        419

#define MG_SPECIAL_VISIT_MAX    419
#define MG_SPECIAL_VISIT_COUNT  (MG_SPECIAL_VISIT_MAX - MG_SPECIAL_VISIT_BASE + 1)


// Pictures used in clue book

#define MG_CLUEPIC_BASE         440

#define MG_CLUEPIC_BAT          440 /* bat, bats */
#define MG_CLUEPIC_BUTTERFLY    441 /* butterfly, butterflies */
#define MG_CLUEPIC_TURTLE       442 /* turtle, turtles */
#define MG_CLUEPIC_SNAIL        443 /* snail, snails */
#define MG_CLUEPIC_OWL          444 /* owl, owls */
#define MG_CLUEPIC_JELLYFISH    445 /* jellyfish, jellyfish */
#define MG_CLUEPIC_FISH         446 /* fish, fish */
#define MG_CLUEPIC_ANT          447 /* ant, ants */

#define MG_CLUEPIC_MAX          447

// Types of clues
#define MG_CLUE_NEED            470 /* need an object */
#define MG_CLUE_CROWNS          471 /* need some crowns */
#define MG_CLUE_MISHMOSH        472 /* where to find Mish/Mosh */
#define MG_CLUE_GOTO            473 /* go for information */

// Booby trap action codes
#define MG_ACT_OBJECT           500 /* win/lose an object */
#define MG_ACT_TURN             501 /* win/lose a turn */
#define MG_ACT_CROWNS           502 /* win/lose crowns */
#define MG_ACT_GO               503 /* go to sector */
#define MG_ACT_GO2              504 /* go to either of two sectors */
#define MG_ACT_GOCOND           505 /* go to either of two sectors where */
/* opponent isn't */

// DLL exit codes
#define MG_DLLX_BASE            550

#define MG_DLLX_QUIT            550 /* quit game (game is over) */
#define MG_DLLX_SAVE            551 /* save game (game is not over) */
#define MG_DLLX_COLOR           552 /* color narration or booby trap */
#define MG_DLLX_ZOOM            553 /* zoom map */
#define MG_DLLX_INVENTORY       554 /* display inventory */
#define MG_DLLX_SPINNER         557 /* run spinner */
#define MG_DLLX_INFO            558 /* provide a piece of info */
#define MG_DLLX_HODJ_WINS       559 /* game over - Hodj wins */
#define MG_DLLX_PODJ_WINS       560 /* game over - Podj wins */
#define MG_DLLX_SCROLL          561 /* options dialog */
#define MG_DLLX_GENERALSTR      562 /* general store: buy objects */
#define MG_DLLX_TRADINGPOST     563 /* trading post exch money/objs */
#define MG_DLLX_BLACKMARKET     564 /* buy objects */
#define MG_DLLX_PAWNSHOP        565 /* sell objects */
#define MG_DLLX_RESTORE         566 /* restore game */

#define MG_DLLX_MAX             566
#define MG_DLLX_COUNT   (MG_DLLX_MAX - MG_DLLX_BASE + 1)

// game win codes
#define MG_WIN_MONEY            600
#define MG_WIN_OBJECT           601
#define MG_WIN_INFO             602
#define MG_WIN_MISHMOSH         603

// game length codes
#define MG_LENGTH_SHORT         620
#define MG_LENGTH_MEDIUM        621
#define MG_LENGTH_LONG          622

// sound file IDs
#define MG_SOUND_BASE           700
#define MG_SOUND_PK1            700     // lose poker
#define MG_SOUND_PK2            701     // win poker
#define MG_SOUND_PG1            702     // peggleboz
#define MG_SOUND_PG2            703
#define MG_SOUND_PG3            704
#define MG_SOUND_PG4            705
#define MG_SOUND_PG5            706
#define MG_SOUND_PG6            707
#define MG_SOUND_FG1            708     // fuge
#define MG_SOUND_FG2            709
#define MG_SOUND_FG3            710
#define MG_SOUND_FG4            711
#define MG_SOUND_FG5            712
#define MG_SOUND_TG1            713     // the guessing game
#define MG_SOUND_TG2            714
#define MG_SOUND_TG3            715
#define MG_SOUND_TG4            716
#define MG_SOUND_TG5            717
#define MG_SOUND_TG6            718
#define MG_SOUND_DF1            719     // damn furry animals
#define MG_SOUND_DF2            720
#define MG_SOUND_DF3            721
#define MG_SOUND_DF4            722
#define MG_SOUND_DF5            723
#define MG_SOUND_GF1B           724     // garfunkle
#define MG_SOUND_GF2B           725
#define MG_SOUND_GF3B           726
#define MG_SOUND_CR1            727     // cryptograms
#define MG_SOUND_CR2            728
#define MG_SOUND_CR3            729
#define MG_SOUND_CR4            730
#define MG_SOUND_CR5            731
#define MG_SOUND_BC1            732
#define MG_SOUND_BC2            733
#define MG_SOUND_BC3            734
#define MG_SOUND_BC4            735
#define MG_SOUND_BC5            736
#define MG_SOUND_RR1            737     // riddles
#define MG_SOUND_RR2            738
#define MG_SOUND_RR3            739
#define MG_SOUND_RR4            740
#define MG_SOUND_BF1            741     // battlefish
#define MG_SOUND_BF2            742
#define MG_SOUND_BF3            743
#define MG_SOUND_PR1B           744
#define MG_SOUND_PR2B           745
#define MG_SOUND_PR3B           746
#define MG_SOUND_PR4B           747
#define MG_SOUND_MD1            748
#define MG_SOUND_MD2            749
#define MG_SOUND_MD3            750
#define MG_SOUND_MD4            751
#define MG_SOUND_WS1            752     // wordsearch
#define MG_SOUND_WS2            753
#define MG_SOUND_WS3            754
#define MG_SOUND_WS4            755
#define MG_SOUND_BQ1            756     // barbershop quintet
#define MG_SOUND_BQ2            757
#define MG_SOUND_MK1            758     // mankala
#define MG_SOUND_MK6            759
#define MG_SOUND_AR1            760     // archeroids
#define MG_SOUND_AR2            761
#define MG_SOUND_NV1            762     // no vacancy
#define MG_SOUND_NV2            763
#define MG_SOUND_NV3            764
#define MG_SOUND_AP1            765     // artparts
#define MG_SOUND_AP2            766
#define MG_SOUND_LF1            767     // life
#define MG_SOUND_LF2            768
#define MG_SOUND_EU1            769     // everything under the sun
#define MG_SOUND_EU2            770
#define MG_SOUND_OLN1           771
#define MG_SOUND_OLN2           772
#define MG_SOUND_OLN3           773
#define MG_SOUND_OLN4           774
#define MG_SOUND_OLN5           775
#define MG_SOUND_OLN6           776
#define MG_SOUND_OLN7           777
#define MG_SOUND_OLN8           778
#define MG_SOUND_OLN9           779
#define MG_SOUND_GSPS1          780
#define MG_SOUND_GSPS2          781
#define MG_SOUND_GSPS3          782
#define MG_SOUND_GSPS4          783
#define MG_SOUND_GSPS5          784
#define MG_SOUND_GSPS6          785
#define MG_SOUND_GSPS7          786
#define MG_SOUND_GSPS8          787
#define MG_SOUND_UTP1           788
#define MG_SOUND_UTP2           789
#define MG_SOUND_UTP3           790
#define MG_SOUND_UTP4           791
#define MG_SOUND_UTP5           792
#define MG_SOUND_UTP6           793
#define MG_SOUND_UTP7           794
#define MG_SOUND_UTP8           795
#define MG_SOUND_TRAN1          796
#define MG_SOUND_TRAN2          797
#define MG_SOUND_TRAN3          798
#define MG_SOUND_TRAN4          799
#define MG_SOUND_TRAN5          800
#define MG_SOUND_TRAN6          801
#define MG_SOUND_TRAN7          802
#define MG_SOUND_TRAN8          803
#define MG_SOUND_TRAN9          804
#define MG_SOUND_TRAN10         805
#define MG_SOUND_TRAN11         806
#define MG_SOUND_TRAN12         807
#define MG_SOUND_BB61           808
#define MG_SOUND_BB62           809
#define MG_SOUND_BB63           810
#define MG_SOUND_BB64           811
#define MG_SOUND_BB65           812
#define MG_SOUND_BB66           813
#define MG_SOUND_BB67           814
#define MG_SOUND_BB68           815
#define MG_SOUND_BB69           816
#define MG_SOUND_BB70           817
#define MG_SOUND_BB71           818
#define MG_SOUND_FM1            819
#define MG_SOUND_FM2            820
#define MG_SOUND_PM1            821
#define MG_SOUND_PM2            822
#define MG_SOUND_WC1            823
#define MG_SOUND_WC2            824
#define MG_SOUND_AC1            825
#define MG_SOUND_AC2            826

#define MG_SOUND_WT1            827     // start of Win/Lose Mish/Mosh narrations
#define MG_SOUND_WT2            828
#define MG_SOUND_WR1            829
#define MG_SOUND_WR2            830
#define MG_SOUND_BR1            831
#define MG_SOUND_BR2            832
#define MG_SOUND_MM1            833
#define MG_SOUND_MM2            834
#define MG_SOUND_PI1            835
#define MG_SOUND_PI2            836
#define MG_SOUND_PI3            837
#define MG_SOUND_HM1            838
#define MG_SOUND_HM2            839

#define MG_SOUND_MAX            839
#define MG_SOUND_COUNT (MG_SOUND_MAX - MG_SOUND_BASE + 1)


// CLocTable
class CLocTable {
public:
	int m_iLocCode ;            // MG_LOC_xxxx
	int m_iFunctionCode ;       // MG_GAME_xxxx or MG_VISIT_xxxx
	int m_iCost ;               // Amount of money for MG_VISIT_INFO
	const char *m_lpszLabel ;         // node label for location
	const char *m_lpszName ;          // full name of location
} ;

// CSectorTable
class CSectorTable {
public:
	int m_iSectorCode ;         // MG_SECTOR_xxxx
	const char *m_lpszLabel ;         // node label for sector
	const char *m_lpszName ;          // full name of sector
} ;

// CGameTable
typedef HWND(FAR PASCAL *FPDLLFUNCT)(HWND, LPGAMESTRUCT);

class CGameTable {
public:
	int m_iGameCode;		// MG_GAME_xxxx
	int m_iWinCode;			// MG_WIN_xxxx (win money, object, info)
	const char *m_lpszGameName;	// full name of game
	const char *_path;			// game subfolder path
	const char *_dllName;		// dll filename
	FPDLLFUNCT _initFn;
} ;

// CNoteTable
class CNoteTable {
public:
	int m_iCode ;       // game or location code
	int m_iCost ;       // cost of information
	char *m_lpszWavFile ;  // sound file
	char *m_lpszText ; // text of message
} ;

// CClueTable
class CClueTable {
public:
	int m_iCluePicCode ;        // MG_CLUEPIC_xxxx, specifying the
	// picture (ant, bat, fish, etc) for clue
	int m_iCluePicCount ;       // count of # of pictures
	int m_iClueCode ;           // MG_CLUE_xxxx -- type of clue -- (info
	// mishmosh location, etc)
	int m_iArgument ;           // MG_OBJ_xxxx or MG_LOC_xxxx -- argument
	// to clue code
};

// CLengthTable
class CLengthTable {
public:
	int m_iLengthCode ;         // MG_LENGTH_xxxx : length argument
	int m_iNumObjects ;         // number of objects required
	int m_iNumInfo ;            // # pieces of information required
	bool m_bMoneyRequired ;     // if true, one object must be money
} ;

#define FINDS(tab, idx) { const C##tab *lpTab ; \
		for (lpTab = c##tab ;; ++lpTab) \
			if (!lpTab->m_##idx) return(nullptr) ; \
			else if (lpTab->m_##idx == idx) return(lpTab) ;}


#define FINDI(nm, tab, idx) { \
		if (idx >= MG_##nm##_BASE && idx <= MG_##nm##_MAX) \
			return(c##tab+(idx-MG_##nm##_BASE)) ; \
		else return(nullptr) ; }

// CMgStatic -- meta game static class
class CMgStatic {
public:
	#ifndef FRAME_EXE
	static const CLocTable FAR cLocTable[] ; // location to game/action
	static const CSectorTable FAR cSectorTable[] ; // sector table
	static const CLengthTable FAR cLengthTable[] ; // level to # objects
	#endif
	static const CGameTable FAR cGameTable[] ; // game to win result
	static const CClueTable FAR cHodjClueTable[] ; // Hodj clue booklet
	static const CClueTable FAR cPodjClueTable[] ; // Podj clue booklet

	static const CGameTable *FindGame(int iGameCode) FINDS(GameTable, iGameCode)

	#ifndef FRAME_EXE
	static const CLocTable *FindLoc(int iLocCode) FINDS(LocTable, iLocCode)
	static const CSectorTable *FindSector(int iSectorCode) FINDS(SectorTable, iSectorCode)
	#endif

// mgstat.cpp -- meta game static tables

//- MgStatic::FindClue -- find Hodj/Podj clue
	static const CClueTable *FindClue(bool bPodj, int iClue) ;

};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
