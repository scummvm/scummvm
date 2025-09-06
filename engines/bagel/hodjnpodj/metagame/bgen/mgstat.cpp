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

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/hodjnpodj/metagame/bgen/mgstat.h"
#include "bagel/hodjnpodj/archeroids/init.h"
#include "bagel/hodjnpodj/artparts/init.h"
#include "bagel/hodjnpodj/barbershop/init.h"
#include "bagel/hodjnpodj/battlefish/init.h"
#include "bagel/hodjnpodj/beacon/init.h"
#include "bagel/hodjnpodj/crypt/init.h"
#include "bagel/hodjnpodj/dfa/init.h"
#include "bagel/hodjnpodj/fuge/init.h"
#include "bagel/hodjnpodj/garfunkle/init.h"
#include "bagel/hodjnpodj/life/init.h"
#include "bagel/hodjnpodj/mankala/init.h"
#include "bagel/hodjnpodj/mazedoom/init.h"
#include "bagel/hodjnpodj/novacancy/init.h"
#include "bagel/hodjnpodj/packrat/init.h"
#include "bagel/hodjnpodj/pdq/init.h"
#include "bagel/hodjnpodj/peggle/init.h"
#include "bagel/hodjnpodj/poker/init.h"
#include "bagel/hodjnpodj/riddles/init.h"
#include "bagel/hodjnpodj/wordsearch/init.h"
#include "bagel/hodjnpodj/metagame/zoom/init.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

// Game table

const CGameTable CMgStatic::cGameTable[] = {
	{
		MG_GAME_ANAGRAMS, 0,
		"Anagrams", nullptr, nullptr, nullptr
	},
	{
		MG_GAME_ARCHEROIDS, MG_WIN_INFO,
		"Archeroids", "arch", "hnparch.dll", Archeroids::RunArch
	},
	{
		MG_GAME_ARTPARTS, MG_WIN_INFO,
		"Art Parts", "artparts", "hnpartp.dll", ArtParts::RunArtp
	},
	{
		MG_GAME_BARBERSHOP, MG_WIN_OBJECT,
		"Barbershop Quintet", "barb", "hnpbarb.dll", Barbershop::RunBarb
	},
	{
		MG_GAME_BATTLEFISH, MG_WIN_OBJECT,
		"Battlefish", "bfish", "hnpbfish.dll", Battlefish::RunBFish
	},
	{
		MG_GAME_BEACON, MG_WIN_MONEY,
		"Beacon", "beacon", "hnpbeac.dll", Beacon::RunBeac
	},
	{
		MG_GAME_CRYPTOGRAMS, MG_WIN_MONEY,
		"Cryptograms", "crypt", "hnpcrypt.dll", Crypt::RunCrypt
	},
	{
		MG_GAME_DAMFURRY, MG_WIN_MONEY,
		"Dam Furry Animals", "dfa", "hnpdfa.dll", DFA::RunDFA
	},
	{
		MG_GAME_EVERYTHING, MG_WIN_INFO,
		"Everything Under the Sun", nullptr, nullptr, nullptr
	},
	{
		MG_GAME_FUGE, MG_WIN_MONEY,
		"Fuge", "fuge", "hnpfuge.dll", Fuge::RunFuge
	},
	{
		MG_GAME_GARFUNKEL, MG_WIN_INFO,
		"Garfunkel", "garf", "hnpgarf.dll", Garkfunkle::RunGarf
	},
	{
		MG_GAME_LIFE, MG_WIN_INFO,
		"Life", "life", "hnplife.dll", Life::RunLife
	},
	{
		MG_GAME_MANKALA, MG_WIN_INFO,
		"Mankala", "mankala", "hnpmank.dll", Mankala::RunMank
	},
	{
		MG_GAME_MAZEODOOM, MG_WIN_OBJECT,
		"Maze o'Doom", "mazedoom", "hnpmaze.dll", MazeDoom::RunMaze
	},
	{
		MG_GAME_NOVACANCY, MG_WIN_INFO,
		"No Vacancy", "novac", "hnpnova.dll", NoVacancy::RunNoVa
	},
	{
		MG_GAME_PACRAT, MG_WIN_OBJECT,
		"Pac-Rat", "packrat", "hnppkrt.dll", Packrat::RunPackRat
	},
	{
		MG_GAME_PEGGLEBOZ, MG_WIN_MONEY,
		"Peggleboz", "peggle", "hnppggl.dll", Peggle::RunPeggle
	},
	{
		MG_GAME_PINBALL, 0,
		"Pinball", nullptr, nullptr, nullptr
	},
	{
		MG_GAME_RIDDLES, MG_WIN_OBJECT,
		"Riddles", "riddles", "hnpridl.dll", Riddles::RunRiddles
	},
	{
		MG_GAME_ROULETTE, 0,
		"Roulette", nullptr, nullptr, nullptr
	},
	{
		MG_GAME_SHOTMACHINE, 0,
		"Shot Machine", nullptr, nullptr, nullptr
	},
	{
		MG_GAME_SPINBALL, 0,
		"Spinball", nullptr, nullptr, nullptr
	},
	{
		MG_GAME_THGESNGGME, MG_WIN_MONEY,
		"TH GESNG GME", "pdq", "hnptgg.dll", PDQ::RunTGG
	},
	{
		MG_GAME_TRIVIA, 0,
		"Trivia", nullptr, nullptr, nullptr
	},
	{
		MG_GAME_VIDEOPOKER, MG_WIN_MONEY,
		"Video Poker", "poker", "hnppkr.dll", Poker::RunPoker
	},
	{
		MG_GAME_WORDSEARCH, MG_WIN_OBJECT,
		"Word Search", "wordsrch", "hnpws.dll", WordSearch::RunWordSearch
	},
	{
		MG_GAME_CHALLENGE, MG_WIN_MISHMOSH,
		"Challenge", ".", "hnpzm.dll", nullptr /*Metagame::Zoom::RunZoomMap*/
	},
	{ 0, 0, nullptr, nullptr, nullptr, nullptr }
};

// =============================================

// location to game (or other action) table
#ifndef FRAME_EXE
const CLocTable CMgStatic::cLocTable[] = {
	{
		MG_LOC_AERIE, MG_VISIT_MISHMOSH, 0,
		"Aerie", "Aerie"
	},
	{
		MG_LOC_AMPHI, MG_GAME_GARFUNKEL, 0,
		"Amphi", "Amphitheatre"
	},
	{
		MG_LOC_ANCIENTRUIN, MG_GAME_RIDDLES, 0,
		"AncientRuin", "Ancient Ruin"
	},
	{
		MG_LOC_ARTISTCOTT, MG_GAME_ARTPARTS, 0,
		"ArtistCott", "Artist's Cottage"
	},
	{
		MG_LOC_BANDITHIDE, MG_VISIT_BLACKMARKET, 0,  // used to be EVERYTHING
		"BanditHide", "Bandit's Hideout"
	},
	{
		MG_LOC_BARBERSHOP, MG_GAME_BARBERSHOP, 0,
		"BarberShop", "Barber Shop"
	},
	{
		MG_LOC_BARN, MG_GAME_WORDSEARCH, 0,
		"Barn", "Barn"
	},
	{
		MG_LOC_BEARCAVE, MG_VISIT_MISHMOSH, 0,
		"BearCave", "Bear Cave"
	},
	{
		MG_LOC_BEAVER, MG_GAME_DAMFURRY, 0,
		"Beaver", "Beaver Dam"
	},
	{
		MG_LOC_BOARDING, MG_GAME_NOVACANCY, 0,
		"Boarding", "Boarding House"
	},
	{
		MG_LOC_CASTLE, MG_VISIT_CASTLE, 0,
		"Castle", "Castle"
	},
	{
		MG_LOC_CORRAL, MG_GAME_THGESNGGME, 0,
		"Corral", "Corral"
	},
	{
		MG_LOC_CRYPT, MG_GAME_CRYPTOGRAMS, 0,
		"Crypt", "Crypt"
	},
	{
		MG_LOC_FARMHOUSE, MG_VISIT_INFO, 23,
		"Farmhouse", "Farmhouse"
	},
	{
		MG_LOC_FERRISWHEEL, MG_GAME_FUGE, 0,
		"FerrisWheel", "Ferris Wheel"
	},
	{
		MG_LOC_FISHMARKET, MG_GAME_BATTLEFISH, 0,
		"FishMarket", "Fish Market"
	},
	{
		MG_LOC_GENERALSTR, MG_VISIT_GENERALSTR, 0,
		"GeneralStr", "General Store"
	},
	{
		MG_LOC_HEDGEMAZE, MG_GAME_PACRAT, 0,
		"HedgeMaze", "Hedge Maze"
	},
	{
		MG_LOC_HERMITTENT, MG_VISIT_MISHMOSH, 0,
		"HermitTent", "Hermit's Tent"
	},
	{
		MG_LOC_INN, MG_GAME_MANKALA, 0,
		"Inn", "Inn"
	},
	{
		MG_LOC_LIGHTHOUSE, MG_GAME_BEACON, 0,
		"Lighthouse", "Lighthouse"
	},
	{
		MG_LOC_MANSION, MG_VISIT_INFO, 33,
		"CountryMans", "Country Mansion"
	},
	{
		MG_LOC_MERMAIDROCK, MG_VISIT_MISHMOSH, 0,
		"MermaidRock", "Mermaid Rock"
	},
	{
		MG_LOC_MINE, MG_GAME_MAZEODOOM, 0,
		"Mine", "Mine"
	},
	{
		MG_LOC_MTKRAK, MG_VISIT_NONE, 0,
		"MtKrak", "Mount Krakanukkul"
	},
	{
		MG_LOC_OASIS, MG_VISIT_OASIS, 0,
		"Oasis", "Oasis"
	},
	{
		MG_LOC_PIRATECOVE, MG_VISIT_MISHMOSH, 0,
		"PirateCove", "Pirate's Cove"
	},
	{
		MG_LOC_PAWNSHOP, MG_VISIT_PAWNSHOP, 0,
		"PawnShop", "Pawn Shop"
	},
	{
		MG_LOC_POSTOFFICE, MG_VISIT_INFO, 27,
		"PostOffice", "Post Office"
	},
	{
		MG_LOC_SHERIFF, MG_GAME_VIDEOPOKER, 0,
		"Sheriff", "Sheriff's Office"
	},
	{
		MG_LOC_SILO, MG_GAME_ANAGRAMS, 0,
		"Silo", "Silo"
	},
	{
		MG_LOC_TAVERN, MG_GAME_PEGGLEBOZ, 0,
		"Tavern", "Tavern"
	},
	{
		MG_LOC_TEMPLE, MG_GAME_LIFE, 0,
		"Temple", "Temple"
	},
	{
		MG_LOC_TOURNAMENT, MG_GAME_ARCHEROIDS, 0,
		"Tournament", "Tournament Field"
	},
	{
		MG_LOC_TRADINGPOST, MG_VISIT_TRADINGPOST, 0,
		"TradePost", "Trading Post"
	},
	{
		MG_LOC_WARLOCKLAIR, MG_VISIT_MISHMOSH, 0,
		"WarlockLair", "Warlock's Lair"
	},
	{
		MG_LOC_WINDMILL, MG_GAME_SPINBALL, 0,
		"Windmill", "Windmill"
	},
	{
		MG_LOC_WITCHHOVEL, MG_VISIT_MISHMOSH, 0,
		"WitchHovel", "Witch's Hovel"
	},
	{
		MG_LOC_WOODCUTTER, MG_VISIT_INFO, 18,
		"Woodcut", "Woodcutter's Cottage"
	},
	{
		MG_LOC_BOAT1, MG_VISIT_BOAT1, 0,
		"Boat1", "Boat Dock 1"
	},
	{
		MG_LOC_BOAT2, MG_VISIT_BOAT2, 0,
		"Boat2", "Boat Dock 2"
	},
	{
		MG_LOC_BOAT3, MG_VISIT_BOAT3, 0,
		"Boat3", "Boat Dock 3"
	},
	{
		MG_LOC_SKY1, MG_VISIT_SKY1, 5,
		"Sky1", "Sky Port 1"
	},
	{
		MG_LOC_SKY2, MG_VISIT_SKY2, 5,
		"Sky2", "Sky Port 2"
	},
	{
		MG_LOC_CAR1, MG_VISIT_CAR1, 0,
		"st22", "Mining Car"
	},
	{
		MG_LOC_SECRET1, MG_VISIT_SECRET1, 0,
		"Secret1", "Secret Passage 1"
	},
	{
		MG_LOC_SECRET2, MG_VISIT_SECRET2, 0,
		"Secret2", "Secret Passage 2"
	},
	{ 0, 0, 0, nullptr, nullptr }
} ;
#endif

// ==================================================

// game length to number of object and items of information
// For each game length, the following information is provided:
//      (1) The number of objects required
//      (2) The number of pieces of information required
//      (3) A flag that indicates whether one of the objects being
//              money is required (1) or optional (0)


#ifndef FRAME_EXE
const CLengthTable CMgStatic::cLengthTable[] =  {
	{MG_LENGTH_SHORT, 1, 2, 0},
	{MG_LENGTH_MEDIUM, 2, 5, 0},
	{MG_LENGTH_LONG, 3, 8, 0},
	{0, 0, 0, 0}
} ;
#endif

// ==================================================

// Clue tables (Hodj and Podj)

const CClueTable CMgStatic::cHodjClueTable[] = {
	{
		MG_CLUEPIC_BAT, 1,
		MG_CLUE_NEED, MG_OBJ_PADDLE
	},

	{
		MG_CLUEPIC_BAT, 2,
		MG_CLUE_NEED, MG_OBJ_SWATTER
	},

	{
		MG_CLUEPIC_BAT, 3,
		MG_CLUE_CROWNS, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_BAT, 4,
		MG_CLUE_MISHMOSH, MG_LOC_BEARCAVE
	},

	{
		MG_CLUEPIC_BAT, 5,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_BAT, 6,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_BAT, 7,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_BAT, 8,
		MG_CLUE_NEED, MG_OBJ_EGG
	},

	{
		MG_CLUEPIC_BUTTERFLY, 1,
		MG_CLUE_NEED, MG_OBJ_SHELL
	},

	{
		MG_CLUEPIC_BUTTERFLY, 2,
		MG_CLUE_GOTO, MG_LOC_FARMHOUSE
	},

	{
		MG_CLUEPIC_BUTTERFLY, 3,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_BUTTERFLY, 4,
		MG_CLUE_NEED, MG_OBJ_BERET
	},

	{
		MG_CLUEPIC_BUTTERFLY, 5,
		MG_CLUE_NEED, MG_OBJ_HONEY
	},

	{
		MG_CLUEPIC_BUTTERFLY, 6,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_BUTTERFLY, 7,
		MG_CLUE_CROWNS, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_BUTTERFLY, 8,
		MG_CLUE_MISHMOSH, MG_LOC_WARLOCKLAIR
	},

	{
		MG_CLUEPIC_TURTLE, 1,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_TURTLE, 2,
		MG_CLUE_NEED, MG_OBJ_SHOELACE
	},

	{
		MG_CLUEPIC_TURTLE, 3,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_TURTLE, 4,
		MG_CLUE_CROWNS, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_TURTLE, 5,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_TURTLE, 6,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_TURTLE, 7,
		MG_CLUE_NEED, MG_OBJ_CANDLE
	},

	{
		MG_CLUEPIC_TURTLE, 8,
		MG_CLUE_NEED, MG_OBJ_SWAB
	},

	{
		MG_CLUEPIC_SNAIL, 1,
		MG_CLUE_NEED, MG_OBJ_SHARPENER
	},

	{
		MG_CLUEPIC_SNAIL, 2,
		MG_CLUE_NEED, MG_OBJ_SCARAB
	},

	{
		MG_CLUEPIC_SNAIL, 3,
		MG_CLUE_NEED, MG_OBJ_EGGBEATER
	},

	{
		MG_CLUEPIC_SNAIL, 4,
		MG_CLUE_NEED, MG_OBJ_CHISEL
	},

	{
		MG_CLUEPIC_SNAIL, 5,
		MG_CLUE_MISHMOSH, MG_LOC_MERMAIDROCK
	},

	{
		MG_CLUEPIC_SNAIL, 6,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_SNAIL, 7,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_SNAIL, 8,
		MG_CLUE_GOTO, MG_LOC_WOODCUTTER
	},

	{
		MG_CLUEPIC_OWL, 1,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_OWL, 2,
		MG_CLUE_NEED, MG_OBJ_FLASHLIGHT
	},

	{
		MG_CLUEPIC_OWL, 3,
		MG_CLUE_CROWNS, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_OWL, 4,
		MG_CLUE_MISHMOSH, MG_LOC_PIRATECOVE
	},

	{
		MG_CLUEPIC_OWL, 5,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_OWL, 6,
		MG_CLUE_NEED, MG_OBJ_GRAPNEL
	},

	{
		MG_CLUEPIC_OWL, 7,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_OWL, 8,
		MG_CLUE_GOTO, MG_LOC_POSTOFFICE
	},

	{
		MG_CLUEPIC_JELLYFISH, 1,
		MG_CLUE_NEED, MG_OBJ_EYEPATCH
	},

	{
		MG_CLUEPIC_JELLYFISH, 2,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_JELLYFISH, 3,
		MG_CLUE_NEED, MG_OBJ_MIRROR
	},

	{
		MG_CLUEPIC_JELLYFISH, 4,
		MG_CLUE_NEED, MG_OBJ_ROPE
	},

	{
		MG_CLUEPIC_JELLYFISH, 5,
		MG_CLUE_CROWNS, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_JELLYFISH, 6,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_JELLYFISH, 7,
		MG_CLUE_NEED, MG_OBJ_PAN
	},

	{
		MG_CLUEPIC_JELLYFISH, 8,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_ANT, 1,
		MG_CLUE_MISHMOSH, MG_LOC_WITCHHOVEL
	},

	{
		MG_CLUEPIC_ANT, 2,
		MG_CLUE_NEED, MG_OBJ_KUMQUAT
	},

	{
		MG_CLUEPIC_ANT, 3,
		MG_CLUE_NEED, MG_OBJ_BROOMSTICK
	},

	{
		MG_CLUEPIC_ANT, 4,
		MG_CLUE_NEED, MG_OBJ_ANVIL
	},

	{
		MG_CLUEPIC_ANT, 5,
		MG_CLUE_GOTO, MG_LOC_MANSION
	},

	{
		MG_CLUEPIC_ANT, 6,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_ANT, 7,
		MG_CLUE_NEED, MG_OBJ_TICKET
	},

	{
		MG_CLUEPIC_ANT, 8,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_FISH, 1,
		MG_CLUE_NEED, MG_OBJ_HACKSAW
	},

	{
		MG_CLUEPIC_FISH, 2,
		MG_CLUE_NEED, MG_OBJ_CHESTNUT
	},

	{
		MG_CLUEPIC_FISH, 3,
		MG_CLUE_MISHMOSH, MG_LOC_HERMITTENT
	},

	{
		MG_CLUEPIC_FISH, 4,
		MG_CLUE_NEED, MG_OBJ_CRUSOE
	},

	{
		MG_CLUEPIC_FISH, 5,
		MG_CLUE_CROWNS, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_FISH, 6,
		MG_CLUE_NEED, MG_OBJ_STAFF
	},

	{
		MG_CLUEPIC_FISH, 7,
		MG_CLUE_NEED, MG_OBJ_FISHHOOK
	},

	{
		MG_CLUEPIC_FISH, 8,
		MG_CLUE_MISHMOSH,  MG_LOC_HERRING
	},

	{0, 0, 0, 0}
};



const CClueTable CMgStatic::cPodjClueTable[] = {
	{
		MG_CLUEPIC_BAT, 1,
		MG_CLUE_NEED, MG_OBJ_HONEY
	},

	{
		MG_CLUEPIC_BAT, 2,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_BAT, 3,
		MG_CLUE_NEED, MG_OBJ_EGG
	},

	{
		MG_CLUEPIC_BAT, 4,
		MG_CLUE_NEED, MG_OBJ_ROPE
	},

	{
		MG_CLUEPIC_BAT, 5,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_BAT, 6,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_BAT, 7,
		MG_CLUE_CROWNS, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_BAT, 8,
		MG_CLUE_NEED, MG_OBJ_PAN
	},

	{
		MG_CLUEPIC_BUTTERFLY, 1,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_BUTTERFLY, 2,
		MG_CLUE_NEED, MG_OBJ_HACKSAW
	},

	{
		MG_CLUEPIC_BUTTERFLY, 3,
		MG_CLUE_CROWNS, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_BUTTERFLY, 4,
		MG_CLUE_NEED, MG_OBJ_CANDLE
	},

	{
		MG_CLUEPIC_BUTTERFLY, 5,
		MG_CLUE_NEED, MG_OBJ_ANVIL
	},

	{
		MG_CLUEPIC_BUTTERFLY, 6,
		MG_CLUE_GOTO, MG_LOC_POSTOFFICE
	},

	{
		MG_CLUEPIC_BUTTERFLY, 7,
		MG_CLUE_MISHMOSH, MG_LOC_MERMAIDROCK
	},

	{
		MG_CLUEPIC_BUTTERFLY, 8,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_TURTLE, 1,
		MG_CLUE_NEED, MG_OBJ_PADDLE
	},

	{
		MG_CLUEPIC_TURTLE, 2,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_TURTLE, 3,
		MG_CLUE_NEED, MG_OBJ_CRUSOE
	},

	{
		MG_CLUEPIC_TURTLE, 4,
		MG_CLUE_NEED, MG_OBJ_FISHHOOK
	},

	{
		MG_CLUEPIC_TURTLE, 5,
		MG_CLUE_MISHMOSH, MG_LOC_WITCHHOVEL
	},

	{
		MG_CLUEPIC_TURTLE, 6,
		MG_CLUE_CROWNS, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_TURTLE, 7,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_TURTLE, 8,
		MG_CLUE_NEED, MG_OBJ_CHISEL
	},

	{
		MG_CLUEPIC_SNAIL, 1,
		MG_CLUE_NEED, MG_OBJ_TICKET
	},

	{
		MG_CLUEPIC_SNAIL, 2,
		MG_CLUE_MISHMOSH, MG_LOC_BEARCAVE
	},

	{
		MG_CLUEPIC_SNAIL, 3,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_SNAIL, 4,
		MG_CLUE_NEED, MG_OBJ_BROOMSTICK
	},

	{
		MG_CLUEPIC_SNAIL, 5,
		MG_CLUE_GOTO, MG_LOC_MANSION
	},

	{
		MG_CLUEPIC_SNAIL, 6,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_SNAIL, 7,
		MG_CLUE_NEED, MG_OBJ_CHESTNUT
	},

	{
		MG_CLUEPIC_SNAIL, 8,
		MG_CLUE_NEED, MG_OBJ_BERET
	},

	{
		MG_CLUEPIC_OWL, 1,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_OWL, 2,
		MG_CLUE_NEED, MG_OBJ_SHELL
	},

	{
		MG_CLUEPIC_OWL, 3,
		MG_CLUE_NEED, MG_OBJ_FLASHLIGHT
	},

	{
		MG_CLUEPIC_OWL, 4,
		MG_CLUE_NEED, MG_OBJ_KUMQUAT
	},

	{
		MG_CLUEPIC_OWL, 5,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_OWL, 6,
		MG_CLUE_CROWNS, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_OWL, 7,
		MG_CLUE_MISHMOSH, MG_LOC_PIRATECOVE
	},

	{
		MG_CLUEPIC_OWL, 8,
		MG_CLUE_GOTO, MG_LOC_WOODCUTTER
	},

	{
		MG_CLUEPIC_JELLYFISH, 1,
		MG_CLUE_NEED, MG_OBJ_SCARAB
	},

	{
		MG_CLUEPIC_JELLYFISH, 2,
		MG_CLUE_CROWNS, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_JELLYFISH, 3,
		MG_CLUE_NEED, MG_OBJ_SWATTER
	},

	{
		MG_CLUEPIC_JELLYFISH, 4,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_JELLYFISH, 5,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_JELLYFISH, 6,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_JELLYFISH, 7,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_JELLYFISH, 8,
		MG_CLUE_NEED, MG_OBJ_SHARPENER
	},

	{
		MG_CLUEPIC_ANT, 1,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_ANT, 2,
		MG_CLUE_MISHMOSH, MG_LOC_WARLOCKLAIR
	},

	{
		MG_CLUEPIC_ANT, 3,
		MG_CLUE_GOTO, MG_LOC_FARMHOUSE
	},

	{
		MG_CLUEPIC_ANT, 4,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_ANT, 5,
		MG_CLUE_NEED, MG_OBJ_EGGBEATER
	},

	{
		MG_CLUEPIC_ANT, 6,
		MG_CLUE_NEED, MG_OBJ_SWAB
	},

	{
		MG_CLUEPIC_ANT, 7,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_ANT, 8,
		MG_CLUE_NEED, MG_OBJ_GRAPNEL
	},

	{
		MG_CLUEPIC_FISH, 1,
		MG_CLUE_NEED, MG_OBJ_SHOELACE
	},

	{
		MG_CLUEPIC_FISH, 2,
		MG_CLUE_CROWNS, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_FISH, 3,
		MG_CLUE_MISHMOSH, MG_LOC_HERRING
	},

	{
		MG_CLUEPIC_FISH, 4,
		MG_CLUE_NEED, MG_OBJ_HERRING
	},

	{
		MG_CLUEPIC_FISH, 5,
		MG_CLUE_NEED, MG_OBJ_EYEPATCH
	},

	{
		MG_CLUEPIC_FISH, 6,
		MG_CLUE_MISHMOSH, MG_LOC_HERMITTENT
	},

	{
		MG_CLUEPIC_FISH, 7,
		MG_CLUE_NEED, MG_OBJ_STAFF
	},

	{
		MG_CLUEPIC_FISH, 8,
		MG_CLUE_NEED, MG_OBJ_MIRROR
	},

	{0, 0, 0, 0}
};

#ifndef FRAME_EXE
//* CMgStatic::FindClue -- find Hodj/Podj clue
const CClueTable *CMgStatic::FindClue(bool bHodj, int iClue)
// bPodj -- Hodj if false, Podj if true
// returns: pointer to clue table entry
{
	//int iError = 0 ;            // error code
	const CClueTable * xpClueTable = bHodj ? cHodjClueTable : cPodjClueTable ;
	const CClueTable * xpClueEntry = nullptr ;   // return value
	int iK ;    // loop variable

	// this loop verifies that index is not out of range
	for (iK = 0 ; iK < iClue && xpClueTable[iK].m_iCluePicCode ; iK++)
		;       // null loop body

	if (iK == iClue && xpClueTable[iK].m_iCluePicCode)
		xpClueEntry = &xpClueTable[iK] ;
	else {
		//iError = 100 ;          // invalid index iClue
		goto cleanup ;
	}

cleanup:

	return (xpClueEntry) ;
}
#endif

#ifndef FRAME_EXE
const CSectorTable CMgStatic::cSectorTable[] = {
	{MG_SECTOR_BRIDGE, "Bridge", "Bridge"},
	{MG_SECTOR_CHUCKLE, "Chuckle", "Cape Chuckle"},
	{MG_SECTOR_DESERT, "Desert", "Desert"},
	{MG_SECTOR_ESTATE, "Estate", "The Estate"},
	{MG_SECTOR_FARM, "Farm", "Farm"},
	{MG_SECTOR_GLACIER, "Glacier", "Glacier"},
	{MG_SECTOR_HILLSIDE, "Hillside", "Hillside"},
	{MG_SECTOR_LAKE, "LakeVw", "Lake View"},
	{MG_SECTOR_MEADOW, "Meadow", "Meadow"},
	{MG_SECTOR_MLPARK, "MistlePk", "Mistle-Laney Park"},
	{MG_SECTOR_MOUNTAINS, "Mountains", "Mountains"},
	{MG_SECTOR_MOUNTAINVIEW, "MountVw", "Mountain View"},
	{MG_SECTOR_PASTEESH, "Pasteech", "Pasteesh"},
	{MG_SECTOR_PATSHWERQUE, "Patshwerque", "Patshwerque"},
	{MG_SECTOR_PLAINS, "Plains", "Plains"},
	{MG_SECTOR_POPORREE, "PoPoree", "Po-Porree City"},
	{MG_SECTOR_RIVER, "RiverVw", "River View"},
	{MG_SECTOR_SWAMP, "SwampEdge", "Edge of Swamp"},
	{MG_SECTOR_VOLCANO, "VolcanoVw", "Volcano View"},
	{MG_SECTOR_WITTY, "WittyWoods", "Witty Woods"},

	{0, nullptr, nullptr}
} ;
#endif

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
