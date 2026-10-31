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

#include "common/util.h"

#include "engines/grim/imuse/imuse_tables.h"

namespace Grim {

const IMuseStateTable _grimStateMusicTable[181] = {
	{"",                        0, 1000, "STATE_NULL",           0,  0,   0, 127,  0 }, /* 000 */
	{"",                        0, 1999, "stateNoChange",        0,  0,   0, 127,  0 }, /* 001 */
	{"1001 - Manny's Office",   3, 1001, "stateMO",              0,  0,   0, 127,  0 }, /* 002 */
	{"1002 - Mr. Frustration",  3, 1002, "stateHA_INTLS",        3,  8,   0,  20, 50 }, /* 003 */
	{"1002 - Mr. Frustration",  3, 1003, "stateHA_REVLS",        3,  8,   0,  80, 12 }, /* 004 */
	{"1002 - Mr. Frustration",  3, 1004, "stateHA_ELVOS",        3,  8,   0,  90, 110}, /* 005 */
	{"1002 - Mr. Frustration",  3, 1005, "stateHA_MNYCU",        3,  8,   0,  60, 110}, /* 006 */
	{"1002 - Mr. Frustration",  3, 1006, "stateHA_EVACU",        3,  8,   0,  80, 64 }, /* 007 */
	{"1002 - Mr. Frustration",  3, 1007, "stateHA_EVAWS",        3,  8,   0,  60, 60 }, /* 008 */
	{"1008 - Domino's Office",  3, 1008, "stateDO",              0,  0,   0, 127, 0  }, /* 009 */
	{"1009 - Copal's Office",   3, 1009, "stateCO",              0,  0,   0, 127, 0  }, /* 010 */
	{"1010 - Ledge",            3, 1010, "stateLE",              0,  0,   0, 127, 0  }, /* 011 */
	{"1011 - Roof",             3, 1011, "stateRF",              0,  0,   0, 127, 0  }, /* 012 */
	{"1020 - Tube Room",        3, 1020, "stateTU",              0,  0,   0, 127, 0  }, /* 013 */
	{"1021 - Brennis",          3, 1021, "stateTU_BRENNIS",      0,  0,   0, 127, 0  }, /* 014 */
	{"1022 - Lobby",            3, 1022, "stateLO",              0,  0,   0, 127, 0  }, /* 015 */
	{"1023 - Packing Room",     3, 1023, "statePK",              0,  0,   0, 127, 0  }, /* 016 */
	{"1030 - Garage",           3, 1030, "stateGA",              0,  0,   0, 127, 0  }, /* 017 */
	{"1031 - Glottis' Shop",    3, 1031, "stateGS",              0,  0,   0, 127, 0  }, /* 018 */
	{"1030 - Garage",           3, 1032, "stateGS_JAIL",         0,  0,   0, 127, 0  }, /* 019 */
	{"1040 - Festival Wet",     3, 1040, "stateAL",             21,  4, 500,  60, 0  }, /* 020 */
	{"1041 - Festival Dry",     3, 1041, "stateFE",             21,  4, 200, 127, 0  }, /* 021 */
	{"1041 - Festival Dry",     3, 1042, "stateST",             21,  4, 200,  60, 0  }, /* 022 */
	{"1041 - Festival Dry",     3, 1043, "stateOS",             21,  4, 200,  46, 0  }, /* 023 */
	{"1040 - Festival Wet",     3, 1044, "stateRP",             21,  4, 200,  85, 0  }, /* 024 */
	{"1050 - Headquarters",     3, 1050, "stateHQ",              0,  0,   0, 127, 0  }, /* 025 */
	{"1060 - Real World",       2, 1060, "stateLR",              0,  0, 300,  80, 0  }, /* 026 */
	{"1070 - Stump Room",       2, 1070, "stateSM",              0,  0,   0, 127, 0  }, /* 027 */
	{"1071 - Signpost Room",    2, 1071, "stateSG",              0,  3,   0, 127, 0  }, /* 028 */
	{"1072 - Navigation",       2, 1072, "stateNA",              0,  0,   0, 127, 0  }, /* 029 */
	{"1071 - Signpost Room",    2, 1073, "stateNA_SOLVED",       0,  3,   0, 127, 0  }, /* 030 */
	{"1074 - Bone Wagon",       2, 1074, "stateOO_BONE",         0,  0,   0, 127, 0  }, /* 031 */
	{"1075 - Spider's Eye",     2, 1075, "stateSP_ENTLA",        0,  0,   0, 127, 0  }, /* 032 */
	{"1076 - Spider Room",      2, 1076, "stateSP_WEBWS",        0,  0,   0, 127, 0  }, /* 033 */
	{"1077 - Tree Pump Amb",    2, 1077, "stateTR_PMPWS",        0,  0,   0, 127, 0  }, /* 034 */
	{"1078 - Tree Pump",        2, 1078, "stateTR_ESTHA",        0,  0,   0, 127, 0  }, /* 035 */
	{"1071 - Signpost Room",    2, 1079, "stateTR_SOLVED",       0,  3,   0, 127, 0  }, /* 036 */
	{"1080 - Beaver Room Lbby", 2, 1080, "stateLB",              0,  0,   0, 127, 0  }, /* 037 */
	{"1081 - Beaver Dam",       2, 1081, "stateBD",              0,  0,   0, 127, 0  }, /* 038 */
	{"1083 - Beaver Room",      2, 1082, "stateBD_SOLVED",       0,  0,   0, 127, 0  }, /* 039 */
	{"1083 - Beaver Room",      2, 1083, "stateBV",              0,  0,   0, 127, 0  }, /* 040 */
	{"1084 - Foggy Cactus",     2, 1084, "stateFC",              0,  0,   0,  80, 0  }, /* 041 */
	{"1085 - Rubamat Exterior", 2, 1085, "stateRE",              0,  0,   0, 105, 0  }, /* 042 */
	{"1087 - Blue Hector",      2, 1086, "stateRI",              0,  4,   0,  80, 30 }, /* 043 */
	{"1109 - Cafe Exterior",    2, 1100, "stateCB",              0,  0,   0, 127, 0  }, /* 044 */
	{"1101 - Cafe Office",      3, 1101, "stateCF",             45,  5, 400,  60, 0  }, /* 045 */
	{"1102 - Cafe Intercom",    3, 1102, "stateCF_INTRCM",      45,  0, 400, 127, 0  }, /* 046 */
	{"1103 - Coat Check",       3, 1103, "stateCC",             49,  4, 400,  60, 100}, /* 047 */
	{"1104 - Lupe",             2, 1104, "stateCC_LUPE",         0,  0, 300, 127, 0  }, /* 048 */
	{"1106 - Glottis Noodle",   3, 1105, "stateCI",             49,  4, 400, 127, 75 }, /* 049 */
	{"1106 - Glottis Noodle",   3, 1106, "stateCI_NOODLE",      49,  4, 400, 127, 75 }, /* 050 */
	{"1106 - Glottis Noodle",   3, 1190, "stateCI_WIDHA",       49,  4, 400,  80, 45 }, /* 051 */
	{"1106 - Glottis Noodle",   3, 1191, "stateCI_BARWS",       49,  4, 400, 127, 127}, /* 052 */
	{"1101 - Cafe Office",      3, 1107, "stateCI_EMPTY",       45,  5, 400, 127, 0  }, /* 053 */
	{"1108 - Casino Interior",  3, 1108, "stateCN",             45,  6, 400, 127, 0  }, /* 054 */
	{"1109 - Cafe Exterior",    2, 1109, "stateCE",              0,  0, 400, 127, 0  }, /* 055 */
	{"1110 - Cafe Ledge",       2, 1110, "stateCL",              0,  0, 400, 105, 0  }, /* 056 */
	{"1108 - Casino Interior",  3, 1111, "stateCN_QUIET",        0,  0, 400,  60, 0  }, /* 057 */
	{"1112 - Rusty Sans Vox",   2, 1112, "stateCI_SONG",         0,  0, 400, 127, 68 }, /* 058 */
	{"1120 - Elevator Station", 2, 1120, "stateEV",              0,  0,   0, 127, 0  }, /* 059 */
	{"1122 - Blue Exterior",    3, 1121, "stateBW",              0,  0, 400, 127, 0  }, /* 060 */
	{"1122 - Blue Exterior",    3, 1122, "stateBE",              0,  0, 400, 127, 0  }, /* 061 */
	{"1123 - Blue Casket Ins",  2, 1123, "stateBI",              0,  4, 700, 127, 0  }, /* 062 */
	{"1124 - Blue Casket Amb",  2, 1124, "stateBI_POE",          0,  0,   0, 127, 0  }, /* 063 */
	{"1125 - Smooth Hector",    2, 1125, "stateBK",              0,  4,   0,  80, 75 }, /* 064 */
	{"1122 - Blue Exterior",    3, 1126, "stateHB",              0,  0, 400, 127, 0  }, /* 065 */
	{"1127 - Limbo Dock",       3, 1127, "stateLM",             66,  0,   0, 127, 0  }, /* 066 */
	{"1128 - Limbo Talk",       2, 1128, "stateLM_TALK",         0,  0, 400, 127, 0  }, /* 067 */
	{"1129 - Limbo Poem",       3, 1129, "stateLM_POEM",        66,  0,   0, 127, 0  }, /* 068 */
	{"1130 - Dry Dock",         2, 1130, "stateDD",              0,  0, 400, 127, 0  }, /* 069 */
	{"1131 - Dry Dock Strike",  2, 1131, "stateDD_STRIKE",       0,  0, 400, 127, 0  }, /* 070 */
	{"1132 - Lighthouse Ext",   2, 1132, "stateLX",              0,  0,   0, 127, 0  }, /* 071 */
	{"1133 - Lola's Last",      2, 1133, "stateLX_LOLA",         0,  0,   0, 127, 0  }, /* 072 */
	{"1140 - Police Station",   3, 1140, "statePC",              0,  0,   0, 127, 0  }, /* 073 */
	{"1141 - Police Interior",  2, 1141, "statePI",              0,  0,   0, 105, 0  }, /* 074 */
	{"1141 - Police Interior",  2, 1142, "statePI_MALLOY",       0,  0,   0, 105, 0  }, /* 075 */
	{"1143 - Morgue",           2, 1143, "stateMG",              0,  0,   0, 105, 0  }, /* 076 */
	{"1140 - Police Station",   3, 1144, "stateBP",              0,  0,   0, 105, 0  }, /* 077 */
	{"1145 - Bridge Blimp",     2, 1145, "stateBB",              0,  0,   0, 127, 0  }, /* 078 */
	{"1146 - LOL Security Ext", 2, 1146, "stateGT",              0,  0,   0,  80, 0  }, /* 079 */
	{"1147 - LOL Security Int", 2, 1147, "stateSL",              0,  0,   0, 127, 0  }, /* 080 */
	{"1148 - Carla's Life",     2, 1148, "stateSL_TALK",         0,  0,   0, 127, 0  }, /* 081 */
	{"1149 - Bomb",             2, 1149, "stateSL_BOMB",         0,  0, 400, 127, 0  }, /* 082 */
	{"1152 - Track Stairs",     3, 1150, "stateTB",             83,  0,   0, 105, 0  }, /* 083 */
	{"1152 - Track Stairs",     3, 1151, "stateTW",             83,  0,   0, 105, 0  }, /* 084 */
	{"1152 - Track Stairs",     3, 1152, "stateTS",             83,  0,   0, 105, 0  }, /* 085 */
	{"1153 - Track Base",       2, 1153, "stateTX",              0,  0, 400, 127, 0  }, /* 086 */
	{"1154 - Kitty Hall",       2, 1154, "stateKH",              0,  0,   0, 127, 0  }, /* 087 */
	{"1155 - Sanspoof",         2, 1155, "stateKH_SANS",         0,  0,   0, 127, 0  }, /* 088 */
	{"1156 - Kitty Stables",    2, 1156, "stateKS",              0,  0,   0, 127, 0  }, /* 089 */
	{"1160 - High Roller Hall", 2, 1160, "stateHH",              0,  0,   0, 127, 0  }, /* 090 */
	{"1161 - High Roller Lnge", 3, 1161, "stateHL",              0,  0, 400, 127, 0  }, /* 091 */
	{"1162 - Glottis Gambling", 2, 1162, "stateHL_GLOT",         0,  0,   0, 127, 0  }, /* 092 */
	{"1163 - Max's Office",     3, 1163, "stateMX",              0,  0, 400, 127, 0  }, /* 093 */
	{"1125 - Hector Steps Out", 3, 1164, "stateHK",             94,  4, 200, 127, 80 }, /* 094 */
	{"1125 - Hector Steps Out", 3, 1165, "stateHP",             94,  4, 200,  20, 64 }, /* 095 */
	{"1125 - Hector Steps Out", 3, 1166, "stateHE",             94,  4, 200, 127, 10 }, /* 096 */
	{"1167 - Dillopede Elev",   3, 1167, "stateDE",              0,  0,   0, 127, 0  }, /* 097 */
	{"1167 - Dillopede Elev",   3, 1168, "stateDH",              0,  0,   0, 127, 0  }, /* 098 */
	{"1167 - Dillopede Elev",   3, 1169, "stateWC",              0,  0,   0, 127, 0  }, /* 099 */
	{"1170 - Extendo Bridge",   3, 1170, "stateXB",              0,  0, 400, 105, 0  }, /* 100 */
	{"1170 - Extendo Bridge",   3, 1171, "stateSD",              0,  0, 400, 105, 0  }, /* 101 */
	{"1170 - Extendo Bridge",   3, 1172, "stateSE",              0,  0, 400, 105, 0  }, /* 102 */
	{"1173 - Scrimshaw Int",    2, 1173, "stateSI",              0,  0, 400, 127, 0  }, /* 103 */
	{"1174 - Scrim Sleep",      2, 1174, "stateSI_SLEEP",        0,  0, 400, 127, 0  }, /* 104 */
	{"1180 - Note to Manny",    2, 1180, "stateNT",              0,  0,   0,  72, 0  }, /* 105 */
	{"1155 - Sanspoof",         2, 1181, "statePF",              0,  0,   0, 127, 0  }, /* 106 */
	{"1201 - Lola Zapata",      2, 1201, "stateLZ",              0,  0,   0, 127, 0  }, /* 107 */
	{"1202 - Inside The Lola",  2, 1202, "stateIL",              0,  0,   0, 127, 0  }, /* 108 */
	{"1203 - Engine Room",      2, 1203, "stateEI",              0,  0,   0, 127, 0  }, /* 109 */
	{"1204 - Porthole",         2, 1204, "stateDP",              0,  0,   0, 127, 0  }, /* 110 */
	{"1204 - Porthole",         2, 1205, "stateOP",              0,  0,   0, 127, 0  }, /* 111 */
	{"1210 - Sunken Lola",      2, 1210, "stateSU",              0,  0,   0, 127, 0  }, /* 112 */
	{"1211 - Pearl Crater Sub", 2, 1211, "statePS",              0,  0,   0, 127, 0  }, /* 113 */
	{"1220 - Miner's Room",     3, 1220, "stateMN",              0,  0,   0, 127, 0  }, /* 114 */
	{"1220 - Miner's Room",     3, 1221, "stateMN_CHEPITO",      0,  0,   0, 127, 0  }, /* 115 */
	{"1222 - Exterior Airlock", 2, 1222, "stateEA",              0,  0,   0, 127, 0  }, /* 116 */
	{"1223 - Factory Hub",      2, 1223, "stateFH",              0,  4,   0, 127, 0  }, /* 117 */
	{"1224 - Foreman's Office", 2, 1224, "stateFO",              0,  0,   0, 127, 0  }, /* 118 */
	{"1230 - Vault Door",       2, 1230, "stateVD",              0,  0,   0, 127, 0  }, /* 119 */
	{"1231 - Outer Vault",      2, 1231, "stateVO",              0,  0,   0, 127, 0  }, /* 120 */
	{"1232 - Inner Vault",      2, 1232, "stateVI",              0,  0,   0, 127, 0  }, /* 121 */
	{"1233 - Ashtray Room",     2, 1233, "stateAR",              0,  0,   0, 127, 0  }, /* 122 */
	{"1234 - Ashtray Scary",    2, 1234, "stateAR_SCARY",        0,  0,   0, 127, 0  }, /* 123 */
	{"1235 - Ashtray Pensive",  2, 1235, "stateAR_PENSV",        0,  0,   0, 127, 0  }, /* 124 */
	{"1236 - Domino's Room",    2, 1236, "stateDR",              0,  0,   0, 127, 0  }, /* 125 */
	{"1240 - Conveyor Under",   3, 1240, "stateCY",              0,  4,   0, 127, 0  }, /* 126 */
	{"1240 - Conveyor Under",   3, 1241, "stateCW",              0,  0,   0, 127, 0  }, /* 127 */
	{"1241 - Crane Intro",      2, 1242, "stateCV",              0,  0,   0, 127, 0  }, /* 128 */
	{"1243 - Anchor Room",      3, 1243, "stateAC",              0,  0,   0, 127, 0  }, /* 129 */
	{"1244 - Glottis Hanging",  2, 1244, "stateGH",              0,  0,   0, 127, 0  }, /* 130 */
	{"1245 - End of the World", 2, 1245, "stateEW",              0,  0,   0, 127, 0  }, /* 131 */
	{"1246 - End World Later",  2, 1246, "stateEW_LATER",        0,  0,   0, 127, 0  }, /* 132 */
	{"1241 - Crane Intro",      2, 1247, "stateCU",              0,  0,   0, 127, 0  }, /* 133 */
	{"1250 - Upper Beach",      3, 1250, "stateBU",              0,  0,   0, 127, 0  }, /* 134 */
	{"1250 - Upper Beach",      3, 1251, "stateBL",              0,  0,   0, 127, 0  }, /* 135 */
	{"1252 - Lower Beach Boat", 3, 1252, "stateBL_BOAT",         0,  0,   0, 127, 0  }, /* 136 */
	{"1253 - Lamancha Sub",     2, 1253, "stateLU",              0,  0,   0, 127, 0  }, /* 137 */
	{"1254 - Crane Later",      2, 1254, "stateCUV_LATER",       0,  0,   0, 127, 0  }, /* 138 */
	{"1301 - Temple Gate",      3, 1301, "stateTG",              0,  0,   0, 127, 0  }, /* 139 */
	{"1301 - Temple Gate",      3, 1302, "stateBS",              0,  0,   0, 127, 0  }, /* 140 */
	{"1303 - Truck Depot",      2, 1303, "stateTD",              0,  0,   0, 105, 0  }, /* 141 */
	{"1306 - Mayan Train Pad",  3, 1306, "stateMT_CHEPITO",    143,  0,   0, 127, 0  }, /* 142 */
	{"1304 - Mayan Train Sta",  3, 1304, "stateMT",            143,  0,   0, 127, 0  }, /* 143 */
	{"1305 - Mayan Workshop",   2, 1305, "stateMY",              0,  0,   0, 127, 0  }, /* 144 */
	{"1307 - Mechanic's Kitch", 2, 1307, "stateMK",              0,  4,   0,  40, 64 }, /* 145 */
	{"1310 - Jello Bomb",       3, 1310, "stateJB",              0,  0,   0, 127, 0  }, /* 146 */
	{"1310 - Jello Bomb",       3, 1311, "stateXB_YEAR4",        0,  0,   0, 127, 0  }, /* 147 */
	{"1125 - Smooth Hector",    3, 1312, "stateBI_CLBYR4",     150,  4, 200,  20, 64 }, /* 148 */
	{"1125 - Smooth Hector",    3, 1313, "stateBI_KITYR4",     150,  4, 200,  50, 32 }, /* 149 */
	{"1125 - Smooth Hector",    3, 1314, "stateBK_YR4",        150,  4, 200, 105, 75 }, /* 150 */
	{"1122 - Blue Exterior",    3, 1315, "stateHB_YR4",          0,  0,   0, 127, 0  }, /* 151 */
	{"1122 - Blue Exterior",    3, 1316, "stateBW_YR4",          0,  0,   0, 127, 0  }, /* 152 */
	{"1122 - Blue Exterior",    3, 1317, "stateBE_YR4",          0,  0,   0, 127, 0  }, /* 153 */
	{"1332 - Hector's Foyer",   3, 1318, "stateHN",              0,  0,   0, 127, 0  }, /* 154 */
	{"1319 - Florist Video",    2, 1319, "stateFVID",            0,  0,   0, 127, 0  }, /* 155 */
	{"1320 - New LSA HQ",       2, 1320, "stateNQ",              0,  0,   0, 127, 0  }, /* 156 */
	{"1321 - LSA Sewer",        3, 1321, "stateLW",              0,  0,   0, 127, 0  }, /* 157 */
	{"1321 - LSA Sewer",        3, 1322, "stateSH",              0,  0,   0, 127, 0  }, /* 158 */
	{"1323 - Sewer Maze",       3, 1323, "stateZW",              0,  2,   0, 127, 0  }, /* 159 */
	{"1324 - Albinozod",        3, 1324, "stateAT",              0,  0,   0, 127, 0  }, /* 160 */
	{"1334 - Albino Trap",      3, 1334, "stateAT_TRAP",         0,  0,   0, 127, 0  }, /* 161 */
	{"1325 - Florist Shop",     3, 1325, "stateFP",            162,  0,   0, 127, 0  }, /* 162 */
	{"1326 - Florist Shop Int", 3, 1326, "stateFI",            162,  0,   0, 127, 0  }, /* 163 */
	{"1327 - Florist OK",       2, 1327, "stateFI_OK",           0,  0,   0, 127, 0  }, /* 164 */
	{"1323 - Sewer Maze",       3, 1328, "stateTE",              0,  2,   0, 127, 0  }, /* 165 */
	{"1329 - Theater Backstag", 2, 1329, "stateTH",              0,  4,   0, 127, 0  }, /* 166 */
	{"1330 - Lemans Lobby",     2, 1330, "stateLY",              0,  0,   0, 127, 0  }, /* 167 */
	{"1330 - Lemans Lobby",     2, 1331, "stateLY_CHOW",         0,  0,   0,  60, 0  }, /* 168 */
	{"1333 - Brennis Talk",     2, 1333, "stateLY_BREN",         0,  0,   0, 127, 0  }, /* 169 */
	{"1332 - Hector's Foyer",   3, 1332, "stateHF",              0,  0,   0, 127, 0  }, /* 170 */
	{"1342 - Neon Ledge",       2, 1340, "stateNL",              0,  0,   0, 127, 0  }, /* 171 */
	{"1350 - Meadow Flowers",   2, 1350, "stateMF",              0,  0,   0, 127, 0  }, /* 172 */
	{"1351 - Meadow",           2, 1351, "stateME",              0,  0,   0, 127, 0  }, /* 173 */
	{"1352 - Car Head",         2, 1352, "stateHI",              0,  0,   0, 127, 0  }, /* 174 */
	{"1353 - Greenhouse Appr",  2, 1353, "stateGE",              0,  0,   0, 127, 0  }, /* 175 */
	{"1355 - Shootout",         2, 1355, "stateSHOOT",           0,  0,   0, 127, 0  }, /* 176 */
	{"1354 - Game Ending",      2, 1354, "stateEND",             0,  0,   0, 127, 0  }, /* 177 */
	{"1400 - Start Credits",    2, 1400, "stateOPEN_CRED",       0,  0,   0, 127, 0  }, /* 178 */
	{"1401 - Smooth Hector",    2, 1401, "stateEND_CRED",        0,  0,  20, 105, 0  }, /* 179 */
	{"",                        0, 1500, "stateMenu",            0,  0,   0, 127, 0  }, /* 180 */
};

const IMuseStateTable _grimSeqMusicTable[41] = {
	{"",                        0, 2000, "SEQ_NULL",             0,  0, 300, 127, 0 }, /* 000 */
	{"2001 - Climb Rope",       2, 2001, "seqClimbRope",         0,  0, 300, 127, 0 }, /* 001 */
	{"2010 - Glottis OK",       2, 2010, "seqGlottisOK",         0,  0, 300, 127, 0 }, /* 002 */
	{"2020 - Reap Bruno",       2, 2020, "seqReapBruno",         0,  0, 300, 127, 0 }, /* 003 */
	{"2030 - Ledgepeckers",     2, 2030, "seqLedgepeckers",      0,  0, 300, 127, 0 }, /* 004 */
	{"2050 - Glottis Heart",    2, 2050, "seqGlottisHeart",      0,  0, 300, 105, 0 }, /* 005 */
	{"2055 - Slingshot Bone",   2, 2055, "seqSlingshotBone",     0,  0, 300, 127, 0 }, /* 006 */
	{"2060 - Glott Tree Fall",  2, 2060, "seqGlottTreeFall",     0,  0, 300, 127, 0 }, /* 007 */
	{"2070 - Beaver Fly",       2, 2070, "seqBeaverFly",         0,  0, 300, 127, 0 }, /* 008 */
	{"2071 - Beaver Sink",      2, 2071, "seqBeaverSink",        0,  0, 300, 127, 0 }, /* 009 */
	{"2080 - Meet Velasco",     2, 2080, "seqMeetVelasco",       0,  0, 300, 127, 0 }, /* 010 */
	{"2140 - Ooo Bonewagon",    2, 2140, "seqGlottSingBone",     0,  0, 300, 127, 75}, /* 011 */
	{"2141 - Ooo Meche",        2, 2141, "seqMannySingMeche",    0,  0, 300, 127, 75}, /* 012 */
	{"2155 - Find Detector",    2, 2155, "seqFindDetector",      0,  0, 300, 127, 0 }, /* 013 */
	{"2156 - Glott Drink Wine", 2, 2156, "seqGlottDrinkWine",    0,  0, 300, 127, 0 }, /* 014 */
	{"2157 - Glott No Wine",    2, 2157, "seqGlottNoWine",       0,  0, 300, 127, 0 }, /* 015 */
	{"2161 - Raoul Appears",    2, 2161, "seqRaoulAppears",      0,  0, 300, 127, 0 }, /* 016 */
	{"2162 - Raoul KO",         2, 2162, "seqRaoulKO",           0,  0, 300, 127, 0 }, /* 017 */
	{"2163 - Raoul Dissed",     2, 2163, "seqRaoulDissed",       0,  0, 300, 127, 0 }, /* 018 */
	{"2165 - Fake Tix",         2, 2165, "seqFakeTix",           0,  0, 300, 127, 0 }, /* 019 */
	{"2180 - Befriend Commies", 2, 2180, "seqBefriendCommies",   0,  0, 300, 127, 0 }, /* 020 */
	{"2186 - Nick Punchout",    2, 2186, "seqNickPunchout",      0,  0, 300, 127, 0 }, /* 021 */
	{"2200 - Yr 3 Iris",        2, 2200, "seqYr3Iris",           0,  0, 300, 127, 0 }, /* 022 */
	{"2210 - Hit Men",          2, 2210, "seqHitMen",            0,  0, 700, 127, 0 }, /* 023 */
	{"2230 - Open Vault",       2, 2230, "seqOpenVault",         0,  0, 300, 127, 0 }, /* 024 */
	{"2235 - Dead Tix",         2, 2235, "seqDeadTix",           0,  0, 300, 127, 0 }, /* 025 */
	{"2240 - Sprinkler",        2, 2240, "seqSprinkler",         0,  0, 500, 127, 0 }, /* 026 */
	{"2250 - Crane Track",      2, 2250, "seqCraneTrack",        0,  0, 300, 127, 0 }, /* 027 */
	{"2255 - Crane Fall",       2, 2255, "seqCraneFall",         0,  0, 300, 127, 0 }, /* 028 */
	{"2300 - Yr 4 Iris",        2, 2300, "seqYr4Iris",           0,  0, 300, 127, 0 }, /* 029 */
	{"2301 - Pop Bruno Casket", 2, 2301, "seqPopBrunoCasket",    0,  0, 300, 127, 0 }, /* 030 */
	{"2310 - Rocket Idea",      2, 2310, "seqRocketIdea",        0,  0, 300, 127, 0 }, /* 031 */
	{"2320 - Jello Suspense",   2, 2320, "seqJelloSuspense",     0,  0, 300, 127, 0 }, /* 032 */
	{"2325 - Lumbago Lemo",     2, 2325, "seqLumbagoLemo",       0,  0, 300, 127, 0 }, /* 033 */
	{"2327 - Breath Mint",      2, 2327, "seqBreathMint",        0,  0, 300, 127, 0 }, /* 034 */
	{"2330 - Pigeon Fly",       2, 2330, "seqPigeonFly",         0,  0, 300, 127, 0 }, /* 035 */
	{"2340 - Coffee On Boys",   2, 2340, "seqCoffeeOnBoys",      0,  0, 300, 127, 0 }, /* 036 */
	{"2350 - Sprout Aha",       2, 2350, "seqSproutAha",         0,  0, 300, 127, 0 }, /* 037 */
	{"2360 - Chowchilla Bye",   2, 2360, "seqChowchillaBye",     0,  0, 300, 127, 0 }, /* 038 */
	{"2370 - Salvador Death",   2, 2370, "seqSalvadorDeath",     0,  0, 300, 127, 0 }, /* 039 */
	{"2399 - End Credits",      2, 2399, "seqEndCredits",        0,  0, 200, 127, 0 }, /* 040 */
};

// The demo's tables have no pan field
const IMuseStateTable _grimDemoStateMusicTable[23] = {
	{"",                        0,    0, "STATE_NULL",           0,  0,   0, 127, 0}, /* 000 */
	{"",                        0, 1000, "stateNoChange",        0,  0,   0, 127, 0}, /* 001 */
	{"MO - Manny's Office",     3, 1001, "stateMO",              0,  0,   0, 127, 0}, /* 002 */
	{"MO - Manny's Office",     3, 1002, "stateHA_INTLS",        0,  0,   0, 127, 0}, /* 003 */
	{"MO - Manny's Office",     3, 1003, "stateHA",              0,  0,   0, 127, 0}, /* 004 */
	{"DO - Domino's Office",    3, 1004, "stateDO",              0,  0,   0, 127, 0}, /* 005 */
	{"CO - Copal's Office",     3, 1005, "stateCO",              0,  0,   0, 127, 0}, /* 006 */
	{"LE - Ledge",              3, 1010, "stateLE",              0,  0,   0, 127, 0}, /* 007 */
	{"RF - Roof",               3, 1011, "stateRF",              0,  0,   0, 127, 0}, /* 008 */
	{"TU - Tube Room",          3, 1020, "stateTU",              0,  0,   0, 127, 0}, /* 009 */
	{"LO - Lobby",              3, 1021, "stateLO",              0,  0,   0, 127, 0}, /* 010 */
	{"PK - Packing Room",       3, 1022, "statePK",              0,  0,   0, 127, 0}, /* 011 */
	{"PK - Packing Room",       3, 1030, "stateGA",              0,  0,   0, 127, 0}, /* 012 */
	{"PK - Packing Room",       3, 1031, "stateGS",              0,  0,   0, 127, 0}, /* 013 */
	{"Festival Wet",            3, 1040, "stateAL",              1,  0,  80,  60, 0}, /* 014 */
	{"Festival Dry",            3, 1041, "stateFE",              1,  0,   0, 127, 0}, /* 015 */
	{"Festival Dry",            3, 1042, "stateST",              1,  0,  80,  50, 0}, /* 016 */
	{"Festival Dry",            3, 1043, "stateOS",              1,  0,  80,  64, 0}, /* 017 */
	{"Festival Wet",            3, 1044, "stateRP",              1,  0,  80,  75, 0}, /* 018 */
	{"HQ - Headquarters",       3, 1050, "stateHQ",              0,  0,   0, 127, 0}, /* 019 */
	{"",                        3, 1060, "stateLR",              0,  0,   0, 127, 0}, /* 020 */
	{"",                        0, 5000, "stateCredits",         0,  0,   0, 127, 0}, /* 021 */
	{"",                        0,    1, "stateMenu",            0,  0,   0, 127, 0}, /* 022 */
};

const IMuseStateTable _grimDemoSeqMusicTable[2] = {
	{"",                        0, 2000, "SEQ_NULL",             0,  0,   0, 127, 0}, /* 000 */
	{"Rope Climb",              3, 2100, "seqClimbRope",         0,  0,   0, 127, 0}, /* 001 */
};

const IMuseDemoFileTable _grimDemoMusicFileTable[12] = {
	{"CO - Copal's Office",  "CO - Copal's Office.IMC"},
	{"DO - Domino's Office", "DO - Domino's Office.IMC"},
	{"Festival Dry",         "Festival Dry.IMC"},
	{"Festival Wet",         "Festival Wet.IMC"},
	{"HQ - Headquarters",    "HQ - Headquarters.IMC"},
	{"LE - Ledge",           "LE - Ledge.IMC"},
	{"LO - Lobby",           "LO - Lobby.IMC"},
	{"MO - Manny's Office",  "MO - Manny's Office.IMC"},
	{"PK - Packing Room",    "PK - Packing Room.IMC"},
	{"RF - Roof",            "RF - Roof.IMC"},
	{"Rope Climb",           "Rope Climb.IMC"},
	{"TU - Tube Room",       "TU - Tube Room.IMC"},
};

// The state table of the previous iMUSE implementation had some entries in a different order, so
// we have to account for that when reloading older savegames...
static const struct {
	int tableIndex;
	int soundId;
} kLegacyMovedStates[] = {
	{142, 1304}, {143, 1305}, {144, 1306}, {161, 1325}, {162, 1326}, {163, 1327}, {164, 1328},
	{165, 1329}, {166, 1330}, {167, 1331}, {168, 1332}, {170, 1334}, {176, 1354}, {177, 1355}
};

int grimLegacyStateSoundId(int tableIndex) {
	if (tableIndex < 0 || tableIndex >= ARRAYSIZE(_grimStateMusicTable))
		return -1;

	for (int i = 0; i < ARRAYSIZE(kLegacyMovedStates); i++) {
		if (kLegacyMovedStates[i].tableIndex == tableIndex)
			return kLegacyMovedStates[i].soundId;
	}

	return _grimStateMusicTable[tableIndex].soundId;
}

} // End of namespace Grim
