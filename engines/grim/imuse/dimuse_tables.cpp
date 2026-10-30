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

#include "engines/grim/imuse/dimuse_tables.h"

namespace Grim {

const imuseComiTable _comiStateMusicTable[] = {
	{0, 1000, "STATE_NULL",         0, 0,    0, ""            }, /* 00 */
	{0, 1001, "stateNoChange",      0, 0,    0, ""            }, /* 01 */
	{3, 1098, "stateCredits1",      0, 0,   60, "1098-C~1.IMX"}, /* 02 */
	{3, 1099, "stateMenu",          0, 0,   60, "1099-M~1.IMX"}, /* 03 */
	{3, 1100, "stateHold1",         4, 0,   60, "1100-H~1.IMX"}, /* 04 */
	{3, 1101, "stateWaterline1",    4, 0,   60, "1101-W~1.IMX"}, /* 05 */
	{3, 1102, "stateHold2",         6, 1,   60, "1102-H~1.IMX"}, /* 06 */
	{3, 1103, "stateWaterline2",    6, 0,   60, "1103-W~1.IMX"}, /* 07 */
	{3, 1104, "stateCannon",        0, 0,   60, "1104-C~1.IMX"}, /* 08 */
	{3, 1105, "stateTreasure",      0, 0,   60, "1105-T~1.IMX"}, /* 09 */
	{3, 1200, "stateFortBase",     10, 1,   60, "1200-F~1.IMX"}, /* 10 */
	{3, 1201, "statePreFort",      10, 1,   60, "1201-P~1.IMX"}, /* 11 */
	{3, 1202, "statePreVooOut",    12, 0,   60, "1202-P~1.IMX"}, /* 12 */
	{3, 1203, "statePreVooIn",     12, 0,   60, "1203-P~1.IMX"}, /* 13 */
	{3, 1204, "statePreVooLady",   12, 0,   60, "1204-P~1.IMX"}, /* 14 */
	{3, 1205, "stateVoodooOut",     0, 0,   60, "1205-V~1.IMX"}, /* 15 */
	{3, 1210, "stateVoodooIn",      0, 0,   60, "1210-V~1.IMX"}, /* 16 */
	{12,1212, "stateVoodooInAlt",   0, 1,   42, "1210-V~1.IMX"}, /* 17 */
	{3, 1215, "stateVoodooLady",    0, 0,   60, "1215-V~1.IMX"}, /* 18 */
	{3, 1219, "statePrePlundermap", 0, 0,   60, "1219-P~1.IMX"}, /* 19 */
	{3, 1220, "statePlundermap",    0, 0,   60, "1220-P~1.IMX"}, /* 20 */
	{3, 1222, "statePreCabana",     0, 0,   60, "1222-P~1.IMX"}, /* 21 */
	{3, 1223, "stateCabana",        0, 0,   60, "1223-C~1.IMX"}, /* 22 */
	{3, 1224, "statePostCabana",   23, 0,   60, "1224-P~1.IMX"}, /* 23 */
	{3, 1225, "stateBeachClub",    23, 0,   60, "1225-B~1.IMX"}, /* 24 */
	{3, 1230, "stateCliff",         0, 0,   60, "1230-C~1.IMX"}, /* 25 */
	{3, 1232, "stateBelly",         0, 0,   48, "1232-B~1.IMX"}, /* 26 */
	{3, 1235, "stateQuicksand",     0, 0,   60, "1235-Q~1.IMX"}, /* 27 */
	{3, 1240, "stateDangerBeach",   0, 0,   48, "1240-D~1.IMX"}, /* 28 */
	{12,1241, "stateDangerBeachAlt",0, 2,   48, "1240-D~1.IMX"}, /* 29 */
	{3, 1245, "stateRowBoat",       0, 0,   60, "1245-R~1.IMX"}, /* 30 */
	{3, 1247, "stateAlongside",     0, 0,   48, "1247-A~1.IMX"}, /* 31 */
	{12,1248, "stateAlongsideAlt",  0, 1,   48, "1247-A~1.IMX"}, /* 32 */
	{3, 1250, "stateChimpBoat",     0, 0,   30, "1250-C~1.IMX"}, /* 33 */
	{3, 1255, "stateMrFossey",      0, 0,   48, "1255-M~1.IMX"}, /* 34 */
	{3, 1259, "statePreTown",       0, 0,   60, "1259-P~1.IMX"}, /* 35 */
	{3, 1260, "stateTown",          0, 0,   60, "1260-T~1.IMX"}, /* 36 */
	{3, 1264, "statePreMeadow",     0, 0,   60, "1264-P~1.IMX"}, /* 37 */
	{3, 1265, "stateMeadow",        0, 0,   60, "1265-M~1.IMX"}, /* 38 */
	{3, 1266, "stateMeadowAmb",     0, 0,   60, "1266-M~1.IMX"}, /* 39 */
	{3, 1270, "stateWardrobePre",  40, 0,   60, "1270-W~1.IMX"}, /* 40 */
	{3, 1272, "statePreShow",      40, 0,   60, "1272-P~1.IMX"}, /* 41 */
	{3, 1274, "stateWardrobeShow", 42, 0,   60, "1274-W~1.IMX"}, /* 42 */
	{3, 1276, "stateShow",         42, 0,   60, "1276-S~1.IMX"}, /* 43 */
	{3, 1277, "stateWardrobeJug",  44, 0,   60, "1277-W~1.IMX"}, /* 44 */
	{3, 1278, "stateJuggling",     44, 0,   60, "1278-J~1.IMX"}, /* 45 */
	{3, 1279, "statePostShow",      0, 0,   60, "1279-P~1.IMX"}, /* 46 */
	{3, 1280, "stateChickenShop",   0, 0,   60, "1280-C~1.IMX"}, /* 47 */
	{3, 1285, "stateBarberShop",   48, 0,   60, "1285-B~1.IMX"}, /* 48 */
	{3, 1286, "stateVanHelgen",    48, 0,   60, "1286-V~1.IMX"}, /* 49 */
	{3, 1287, "stateBill",         48, 0,   60, "1287-B~1.IMX"}, /* 50 */
	{3, 1288, "stateHaggis",       48, 0,   60, "1288-H~1.IMX"}, /* 51 */
	{3, 1289, "stateRottingham",   48, 0,   60, "1289-R~1.IMX"}, /* 52 */
	{3, 1305, "stateDeck",          0, 0,   60, "1305-D~1.IMX"}, /* 53 */
	{3, 1310, "stateCombatMap",     0, 0,   60, "1310-C~1.IMX"}, /* 54 */
	{3, 1320, "stateShipCombat",    0, 0,   60, "1320-S~1.IMX"}, /* 55 */
	{3, 1325, "stateSwordfight",    0, 0,   60, "1325-S~1.IMX"}, /* 56 */
	{3, 1327, "stateSwordRott",     0, 0,   60, "1327-S~1.IMX"}, /* 57 */
	{3, 1330, "stateTownEdge",      0, 0,   60, "1330-T~1.IMX"}, /* 58 */
	{3, 1335, "stateSwordLose",     0, 0,   60, "1335-S~1.IMX"}, /* 59 */
	{3, 1340, "stateSwordWin",      0, 0,   60, "1340-S~1.IMX"}, /* 60 */
	{3, 1345, "stateGetMap",        0, 0,   60, "1345-G~1.IMX"}, /* 61 */
	{3, 1400, "stateWreckBeach",    0, 0,   60, "1400-W~1.IMX"}, /* 62 */
	{3, 1405, "stateBloodMap",     63, 0,   60, "1405-B~1.IMX"}, /* 63 */
	{3, 1410, "stateClearing",      0, 0,   60, "1410-C~1.IMX"}, /* 64 */
	{3, 1415, "stateLighthouse",   63, 0,   60, "1415-L~1.IMX"}, /* 65 */
	{3, 1420, "stateVillage",      66, 0,   60, "1420-V~1.IMX"}, /* 66 */
	{3, 1423, "stateVolcano",      66, 0,   60, "1423-V~1.IMX"}, /* 67 */
	{3, 1425, "stateAltar",        66, 0,   60, "1425-A~1.IMX"}, /* 68 */
	{3, 1430, "stateHotelOut",      0, 0,   60, "1430-H~1.IMX"}, /* 69 */
	{3, 1435, "stateHotelBar",     70, 0,   60, "1435-H~1.IMX"}, /* 70 */
	{3, 1440, "stateHotelIn",      70, 0,   60, "1440-H~1.IMX"}, /* 71 */
	{3, 1445, "stateTarotLady",    70, 0,   60, "1445-T~1.IMX"}, /* 72 */
	{3, 1447, "stateGoodsoup",     70, 0,   60, "1447-G~1.IMX"}, /* 73 */
	{3, 1448, "stateGuestRoom",     0, 0,   60, "1448-G~1.IMX"}, /* 74 */
	{3, 1450, "stateWindmill",     63, 0,   60, "1450-W~1.IMX"}, /* 75 */
	{3, 1455, "stateCemetary",      0, 0,   60, "1455-C~1.IMX"}, /* 76 */
	{3, 1460, "stateCrypt",        77, 0,   60, "1460-C~1.IMX"}, /* 77 */
	{3, 1463, "stateGraveDigger",  77, 0,   60, "1463-G~1.IMX"}, /* 78 */
	{3, 1465, "stateMonkey1",       0, 0,   60, "1465-M~1.IMX"}, /* 79 */
	{3, 1475, "stateStanDark",      0, 0,   60, "1475-S~1.IMX"}, /* 80 */
	{3, 1477, "stateStanLight",     0, 0,   60, "1477-S~1.IMX"}, /* 81 */
	{3, 1480, "stateEggBeach",     63, 0,   60, "1480-E~1.IMX"}, /* 82 */
	{3, 1485, "stateSkullIsland",   0, 0,   60, "1485-S~1.IMX"}, /* 83 */
	{3, 1490, "stateSmugglersCave", 0, 0,   60, "1490-S~1.IMX"}, /* 84 */
	{3, 1500, "stateLeChuckTalk",   0, 0,   60, "1500-L~1.IMX"}, /* 85 */
	{3, 1505, "stateCarnival",      0, 0,   60, "1505-C~1.IMX"}, /* 86 */
	{3, 1511, "stateHang",         87, 0,   60, "1511-H~1.IMX"}, /* 87 */
	{3, 1512, "stateRum",          87, 0,   60, "1512-RUM.IMX"}, /* 88 */
	{3, 1513, "stateTorture",      87, 0,   60, "1513-T~1.IMX"}, /* 89 */
	{3, 1514, "stateSnow",         87, 0,   60, "1514-S~1.IMX"}, /* 90 */
	{3, 1515, "stateCredits",       0, 0,   60, "1515-C~1.IMX"}, /* 91 */
	{3, 1520, "stateCarnAmb",       0, 0,   60, "1520-C~1.IMX"}, /* 92 */
	{0,   -1, "",                   0, 0,    0, "",           }
};

const imuseComiTable _comiSeqMusicTable[] = {
	{0, 2000, "SEQ_NULL",        0, 0,    0, ""            },
	{0, 2100, "seqINTRO",        0, 0,    0, ""            },
	{3, 2105, "seqInterlude1",   0, 0,   60, "2105-I~1.IMX"},
	{8, 2110, "seqLastBoat",     0, 1,    0, ""            },
	{0, 2115, "seqSINK_SHIP",    0, 0,    0, ""            },
	{0, 2120, "seqCURSED_RING",  0, 0,   60, ""            },
	{3, 2200, "seqInterlude2",   0, 0,   60, "2200-I~1.IMX"},
	{3, 2210, "seqKidnapped",    0, 0,   60, "2210-K~1.IMX"},
	{8, 2220, "seqSnakeVomits",  0, 1,    0, ""            },
	{8, 2222, "seqPopBalloon",   0, 1,    0, ""            },
	{3, 2225, "seqDropBalls",    0, 0,   60, "2225-D~1.IMX"},
	{4, 2232, "seqArriveBarber", 0, 0,   60, "2232-A~1.IMX"},
	{3, 2233, "seqAtonal",       0, 0,   60, "2233-A~1.IMX"},
	{3, 2235, "seqShaveHead1",   0, 0,   60, "2235-S~1.IMX"},
	{2, 2236, "seqShaveHead2",   0, 2,   60, "2235-S~1.IMX"},
	{3, 2245, "seqCaberLose",    0, 0,   60, "2245-C~1.IMX"},
	{3, 2250, "seqCaberWin",     0, 0,   60, "2250-C~1.IMX"},
	{3, 2255, "seqDuel1",        0, 0,   60, "2255-D~1.IMX"},
	{2, 2256, "seqDuel2",        0, 2,   60, "2255-D~1.IMX"},
	{2, 2257, "seqDuel3",        0, 3,   60, "2255-D~1.IMX"},
	{3, 2260, "seqBlowUpTree1",  0, 0,   60, "2260-B~1.IMX"},
	{2, 2261, "seqBlowUpTree2",  0, 2,   60, "2260-B~1.IMX"},
	{3, 2275, "seqMonkeys",      0, 0,   60, "2275-M~1.IMX"},
	{9, 2277, "seqAttack",       0, 1,    0, ""            },
	{3, 2285, "seqSharks",       0, 0,   60, "2285-S~1.IMX"},
	{3, 2287, "seqTowelWalk",    0, 0,   60, "2287-T~1.IMX"},
	{0, 2293, "seqNICE_BOOTS",   0, 0,    0, ""            },
	{0, 2295, "seqBIG_BONED",    0, 0,    0, ""            },
	{3, 2300, "seqToBlood",      0, 0,   60, "2300-T~1.IMX"},
	{3, 2301, "seqInterlude3",   0, 0,   60, "2301-I~1.IMX"},
	{3, 2302, "seqRott1",        0, 0,   60, "2302-R~1.IMX"},
	{2, 2304, "seqRott2",        0, 2,   60, "2302-R~1.IMX"},
	{2, 2305, "seqRott2b",       0,21,   60, "2302-R~1.IMX"},
	{2, 2306, "seqRott3",        0, 3,   60, "2302-R~1.IMX"},
	{2, 2308, "seqRott4",        0, 4,   60, "2302-R~1.IMX"},
	{2, 2309, "seqRott5",        0, 5,   60, "2302-R~1.IMX"},
	{3, 2311, "seqVerse1",       0, 0,   60, "2311-S~1.IMX"},
	{2, 2312, "seqVerse2",       0, 2,   60, "2311-S~1.IMX"},
	{2, 2313, "seqVerse3",       0, 3,   60, "2311-S~1.IMX"},
	{2, 2314, "seqVerse4",       0, 4,   60, "2311-S~1.IMX"},
	{2, 2315, "seqVerse5",       0, 5,   60, "2311-S~1.IMX"},
	{2, 2316, "seqVerse6",       0, 6,   60, "2311-S~1.IMX"},
	{2, 2317, "seqVerse7",       0, 7,   60, "2311-S~1.IMX"},
	{2, 2318, "seqVerse8",       0, 8,   60, "2311-S~1.IMX"},
	{2, 2319, "seqSongEnd",      0, 9,   60, "2311-S~1.IMX"},
	{2, 2336, "seqRiposteLose",  0, 0,   60, "2336-R~1.IMX"},
	{2, 2337, "seqRiposteWin",   0, 0,   60, "2337-R~1.IMX"},
	{2, 2338, "seqInsultLose",   0, 0,   60, "2338-I~1.IMX"},
	{2, 2339, "seqInsultWin",    0, 0,   60, "2339-I~1.IMX"},
	{3, 2340, "seqSwordLose",    0, 0,   60, "1335-S~1.IMX"},
	{3, 2345, "seqSwordWin",     0, 0,   60, "1340-S~1.IMX"},
	{3, 2347, "seqGetMap",       0, 0,   60, "1345-G~1.IMX"},
	{3, 2400, "seqInterlude4",   0, 0,   60, "2400-I~1.IMX"},
	{0, 2405, "seqSHIPWRECK",    0, 0,    0, ""            },
	{3, 2408, "seqFakeCredits",  0, 0,   60, "2408-F~1.IMX"},
	{3, 2410, "seqPassOut",      0, 0,   60, "2410-P~1.IMX"},
	{3, 2414, "seqGhostTalk",    0, 0,   60, "2414-G~1.IMX"},
	{2, 2415, "seqGhostWedding", 0, 1,   60, "2414-G~1.IMX"},
	{3, 2420, "seqEruption",     0, 0,   60, "2420-E~1.IMX"},
	{3, 2425, "seqSacrifice",    0, 0,   60, "2425-S~1.IMX"},
	{2, 2426, "seqSacrificeEnd", 0, 1,   60, "2425-S~1.IMX"},
	{3, 2430, "seqScareDigger",  0, 0,   60, "2430-S~1.IMX"},
	{3, 2445, "seqSkullArrive",  0, 0,   60, "2445-S~1.IMX"},
	{3, 2450, "seqFloat",        0, 0,   60, "2450-C~1.IMX"},
	{2, 2451, "seqFall",         0, 1,   60, "2450-C~1.IMX"},
	{2, 2452, "seqUmbrella",     0, 2,   60, "2450-C~1.IMX"},
	{3, 2460, "seqFight",        0, 0,   60, "2460-F~1.IMX"},
	{0, 2465, "seqLAVE_RIDE",    0, 0,    0, ""            },
	{0, 2470, "seqMORE_SLAW",    0, 0,    0, ""            },
	{0, 2475, "seqLIFT_CURSE",   0, 0,    0, ""            },
	{3, 2500, "seqInterlude5",   0, 0,   60, "2500-I~1.IMX"},
	{3, 2502, "seqExitSkycar",   0, 0,   60, "2502-E~1.IMX"},
	{3, 2504, "seqGrow1",        0, 0,   60, "2504-G~1.IMX"},
	{2, 2505, "seqGrow2",        0, 1,   60, "2504-G~1.IMX"},
	{3, 2508, "seqInterlude6",   0, 0,   60, "2508-I~1.IMX"},
	{0, 2515, "seqFINALE",       0, 0,    0, ""            },
	{3, 2520, "seqOut",          0, 0,   60, "2520-OUT.IMX"},
	{3, 2530, "seqZap1a",        0, 0,   60, "2530-Z~1.IMX"},
	{2, 2531, "seqZap1b",        0, 1,   60, "2530-Z~1.IMX"},
	{2, 2532, "seqZap1c",        0, 2,   60, "2530-Z~1.IMX"},
	{2, 2540, "seqZap2a",        0, 0,   60, "2540-Z~1.IMX"},
	{2, 2541, "seqZap2b",        0, 1,   60, "2540-Z~1.IMX"},
	{2, 2542, "seqZap2c",        0, 2,   60, "2540-Z~1.IMX"},
	{3, 2550, "seqZap3a",        0, 0,   60, "2550-Z~1.IMX"},
	{2, 2551, "seqZap3b",        0, 1,   60, "2550-Z~1.IMX"},
	{2, 2552, "seqZap3c",        0, 2,   60, "2550-Z~1.IMX"},
	{3, 2560, "seqZap4a",        0, 0,   60, "2560-Z~1.IMX"},
	{2, 2561, "seqZap4b",        0, 1,   60, "2560-Z~1.IMX"},
	{2, 2562, "seqZap4c",        0, 2,   60, "2560-Z~1.IMX"},
	{0,   -1, "",                0, 0,    0, ""            }
};

} // End of namespace Grim
