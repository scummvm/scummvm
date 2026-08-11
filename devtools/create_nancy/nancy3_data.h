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

#ifndef NANCY3DATA_H
#define NANCY3DATA_H

#include "types.h"

const GameConstants _nancy3Constants = {
	18,												// numItems
	336,											// numEventFlags
	{	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,			// genericEventFlags
		11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
		21, 22, 23, 24, 25, 26, 27, 28, 29, 30 },
	8,												// numCursorTypes
	4000,											// logoEndAfter
	32												// wonGameFlagID
};

const SoundChannelInfo _nancy3andUpSoundChannelInfo = {
	32, 14,
	{ 12, 13, 30 },
	{ 0, 1, 2, 3, 19, 26, 27, 29 },
	{ 4, 5, 6, 7, 8, 9, 10, 11, 17, 18, 20, 21, 22, 23, 24, 25, 31 }
};

const Common::Array<GameLanguage> _nancy3LanguagesOrder = {
	GameLanguage::kEnglish,
	GameLanguage::kRussian
};

const Common::Array<Common::Array<ConditionalDialogue>> _nancy3ConditionalDialogue = {
{	// Abby, 13 responses
	{	0, 1050, "NAS50",
		{ { kEv, 215, true }, { kEv, 49, false }, { kEv, 218, false } } },
	{	1, 1053, "NAS53",
		{ { kEv, 169, true }, { kEv, 37, false } } },
	{	2, 1054, "NAS54",
		{ { kEv, 213, true }, { kEv, 46, false } } },
	{	3, 1055, "NAS55",
		{ { kEv, 171, true }, { kEv, 44, false } } },
	{	4, 1056, "NAS56",
		{ { kEv, 173, true }, { kEv, 50, false } } },
	{	5, 1057, "NAS57",
		{ { kEv, 211, true }, { kEv, 45, false }, { kEv, 109, false } } },
	{	6, 1059, "NAS59",
		{ { kEv, 146, true }, { kEv, 39, false }, { kEv, 218, false } } },
	{	7, 1062, "NLC60",
		{ { kEv, 109, true }, { kEv, 43, false } } },
	{	8, 1064, "NAS64",
		{ { kEv, 218, true }, { kEv, 225, true }, { kEv, 209, true }, { kEv, 38, false } } },
	{	9, 1067, "NAS67",
		{ { kEv, 223, true }, { kEv, 37, true }, { kEv, 36, false } } },
	{	10, 1070, "NAS70",
		{ { kEv, 37, true }, { kEv, 53, false } } },
	{	11, 1071, "NAS71",
		{ { kEv, 46, true }, { kEv, 41, false } } },
	{	12, 1074, "NAS74",
		{ { kEv, 191, true }, { kEv, 37, true }, { kEv, 218, false }, { kEv, 294, false }, { kIn, 13, false } } },
	{	13, 1075, "NAS75",
		{ { kEv, 220, true }, { kEv, 51, false }, { kEv, 45, false }, { kEv, 218, false } } }
},
{	// Bess & George, 11 responses
	{	14, 1120, "NBG20",
		{ { kEv, 169, true }, { kEv, 72, false } } },
	{	15, 1121, "NBG21",
		{ { kEv, 171, true }, { kEv, 77, false }, { kEv, 208, false } } },
	{	16, 1122, "NBG22",
		{ { kEv, 197, true }, { kEv, 73, false }, { kEv, 248, false } } },
	{	17, 1124, "NBG24",
		{ { kEv, 73, true }, { kEv, 75, false }, { kEv, 202, true }, { kEv, 246, false } } },
	{	18, 1125, "NBG25",
		{ { kEv, 222, true }, { kEv, 79, false }, { kEv, 218, false } } },
	{	19, 1126, "NBG26",
		{ { kEv, 241, true }, { kEv, 74, false } } },
	{	20, 1127, "NBG27",
		{ { kEv, 223, true }, { kEv, 80, false }, { kEv, 72, true } } },
	{	21, 1130, "NBG30",
		{ { kEv, 208, true }, { kEv, 77, true }, { kEv, 203, false }, { kEv, 295, false } } },
	{	22, 1131, "NBG31",
		{ { kEv, 203, true }, { kEv, 76, false } } },
	{	23, 1132, "NBG32",
		{ { kEv, 297, true }, { kEv, 71, false }, { kEv, 208, false } } },
	{	24, 1133, "NBG33",
		{ { kEv, 168, true } } }
},
{	// Charlie, 13 responses
	{	25, 1250, "NCM50",
		{ { kEv, 171, true }, { kEv, 93, false }, { kEv, 298, false } } },
	{	26, 1253, "NCM53",
		{ { kEv, 173, true }, { kEv, 84, false }, { kEv, 223, false } } },
	{	27, 1254, "NCM54",
		{ { kEv, 228, true }, { kEv, 100, false }, { kEv, 219, false } } },
	{	28, 1255, "NCM55",
		{ { kEv, 223, true }, { kEv, 299, true }, { kEv, 97, false } } },
	{	29, 1257, "NCM57",
		{ { kEv, 167, true }, { kEv, 83, false } } },
	{	30, 1260, "NCM60",
		{ { kEv, 219, true }, { kEv, 87, false } } },
	{	31, 1263, "NEF40",
		{ { kEv, 109, true }, { kEv, 91, false } } },
	{	32, 1265, "NCM65",
		{ { kEv, 183, true }, { kEv, 94, false }, { kIn, 13, false } } },
	{	33, 1267, "NCM67",
		{ { kEv, 145, true }, { kEv, 300, false }, { kEv, 122, false }, { kEv, 218, false } } },
	{	34, 1269, "NCM69",
		{ { kEv, 241, true }, { kEv, 301, false } } },
	{	35, 1270, "NCM70",
		{ { kEv, 297, true }, { kEv, 302, false } } },
	{	36, 1273, "NCM73",
		{ { kEv, 213, true }, { kEv, 303, false } } },
	{	37, 1274, "NCM74",
		{ { kEv, 304, true }, { kEv, 240, false }, { kEv, 305, false } } }
},
{	// Emily, 10 responses
	{	38, 1330, "NEF30",
		{ { kEv, 204, true }, { kEv, 110, false } } },
	{	39, 1331, "NEF31",
		{ { kEv, 200, true }, { kEv, 108, false } } },
	{	40, 1332, "NEF32",
		{ { kEv, 205, true }, { kEv, 157, false }, { kEv, 111, false } } },
	{	41, 1333, "NEF33",
		{ { kEv, 197, true }, { kEv, 107, false } } },
	{	42, 1334, "NEF34",
		{ { kEv, 171, true }, { kEv, 113, false } } },
	{	43, 1335, "NEF35",
		{ { kEv, 206, true }, { kEv, 112, false } } },
	{	44, 1338, "NEF38",
		{ { kEv, 234, true }, { kEv, 106, false } } },
	{	45, 1340, "NEF40",
		{ { kEv, 203, true }, { kEv, 109, false } } },
	{	46, 1341, "NEF41",
		{ { kEv, 307, true }, { kEv, 118, false } } },
	{	47, 1344, "NEF44",
		{ { kEv, 167, true }, { kEv, 116, false }, { kEv, 107, false } } }
},
{	// Hannah, 12 responses
	{	48, 1420, "NHG20",
		{ { kEv, 167, true }, { kEv, 218, false }, { kEv, 129, false } } },
	{	49, 1423, "NHG23",
		{ { kEv, 169, true }, { kEv, 131, false } } },
	{	50, 1426, "NHG26",
		{ { kEv, 171, true }, { kEv, 136, false } } },
	{	51, 1429, "NHG29",
		{ { kEv, 222, true }, { kEv, 140, false } } },
	{	52, 1433, "NHG33",
		{ { kEv, 140, true }, { kEv, 215, false }, { kEv, 139, false } } },
	{	53, 1434, "NHG34",
		{ { kEv, 234, true }, { kEv, 130, false } } },
	{	54, 1437, "NHG37",
		{ { kEv, 241, true }, { kEv, 133, false } } },
	{	55, 1438, "NHG38",
		{ { kEv, 133, true }, { kEv, 214, false }, { kEv, 138, false } } },
	{	56, 1439, "NHG39",
		{ { kEv, 131, true }, { kEv, 223, true }, { kEv, 141, false }, { kEv, 97, false } } },
	{	57, 1440, "NHG40",
		{ { kEv, 208, true }, { kEv, 143, false } } },
	{	58, 1441, "NHG41",
		{ { kEv, 143, true }, { kEv, 203, true }, { kEv, 134, false }, { kEv, 109, false } } },
	{	59, 1442, "NHG42",
		{ { kEv, 97, true }, { kEv, 141, true }, { kEv, 132, false } } }
},
{	// Louis, 15 responses
	{	60, 1550, "NLC50",
		{ { kEv, 211, true }, { kEv, 165, false } } },
	{	61, 1551, "NLC51",
		{ { kEv, 287, true }, { kEv, 151, false }, { kEv, 241, false} } },
	{	62, 1552, "NLC52",
		{ { kEv, 171, true }, { kEv, 148, false }, { kEv, 98, false } } },
	{	63, 1553, "NLC53",
		{ { kEv, 173, true }, { kEv, 154, false }, { kEv, 148, false } } },
	{	64, 1554, "NLC54",
		{ { kEv, 191, true }, { kEv, 161, false }, { kIn, 13, false } } },
	{	65, 1555, "NLC55",
		{ { kEv, 213, true }, { kEv, 159, false }, { kEv, 160, true } } },
	{	66, 1557, "NLC57",
		{ { kEv, 95, true }, { kEv, 162, false } } },
	{	67, 1558, "NLC58",
		{ { kEv, 98, true }, { kEv, 164, false } } },
	{	68, 1559, "NLC59",
		{ { kEv, 190, true }, { kEv, 163, false } } },
	{	69, 1560, "NLC60",
		{ { kEv, 109, true }, { kEv, 153, false } } },
	{	70, 1562, "NLC62",
		{ { kEv, 206, true }, { kEv, 157, false } } },
	{	71, 1565, "NLC65",
		{ { kEv, 205, true }, { kEv, 156, false }, { kEv, 165, true } } },
	{	72, 1566, "NAS54",
		{ { kEv, 213, true }, { kEv, 160, false } } },
	{	73, 1567, "NLC67",
		{ { kEv, 169, true }, { kEv, 37, true }, { kEv, 150, false } } },
	{	74, 1568, "NLC68",
		{ { kEv, 288, true }, { kEv, 153, false }, { kEv, 123, false }, { kEv, 157, true } } },
},
{	// Ned, 0 responses
},
{	// Rose,
	{	75, 1750, "NRM17",
		{ { kEv, 211, true }, { kEv, 186, false } } },
	{	76, 1751, "NRM18",
		{ { kEv, 169, true }, { kEv, 179, false } } },
	{	77, 1752, "NRM19",
		{ { kEv, 171, true }, { kEv, 185, false } } },
	{	78, 1753, "NRM20",
		{ { kEv, 167, true }, { kEv, 177, false } } },
	{	79, 1754, "NRM21",
		{ { kEv, 241, true }, { kEv, 214, true }, { kEv, 187, false } } },
	{	80, 1755, "NRM55",
		{ { kEv, 196, true }, { kEv, 178, false } } },
	{	81, 1757, "NRM22",
		{ { kEv, 206, true }, { kEv, 184, false } } },
	{	82, 1758, "NRM23",
		{ { kEv, 167, true }, { kEv, 222, false }, { kEv, 192, false } } },
	{	83, 1759, "NRM24",
		{ { kEv, 213, true }, { kEv, 227, true }, { kEv, 188, false } } },
	{	84, 1761, "NRM25",
		{ { kEv, 84, true }, { kEv, 308, false }, { kEv, 179, true } } },
	{	85, 1762, "NAS67",
		{ { kEv, 84, true }, { kEv, 97, false }, { kEv, 299, false }, { kEv, 179, true } } },
	{	86, 1763, "NRM27",
		{ { kEv, 225, true }, { kEv, 38, false }, { kEv, 309, false }, { kEv, 310, true } } },
	{	87, 1764, "NRM28",
		{ { kEv, 311, true }, { kEv, 312, false }, { kEv, 171, false } } },
	{	88, 1765, "NRM29",
		{ { kEv, 220, true }, { kEv, 38, false }, { kEv, 310, false } } },
	{	89, 1756, "NLC60",
		{ { kEv, 203, true }, { kEv, 124, false } } },
}
};

const Common::Array<Goodbye> _nancy3Goodbyes = {
	{ "NAS90", { { { 1090, 1091, 1092, 1093, 1094, 1095, 1096 }, {}, NOFLAG } } },				// Abby
	{ "NBG90", { { { 1190, 1191, 1192, 1193, 1194 }, {}, NOFLAG } } },							// Bess & George
	{ "NCM90", { { { 1290, 1291, 1292, 1293, 1294, 1295, 1296, 1297, 1298 }, {}, NOFLAG } } },	// Charlie
	{ "NEF90", { { { 1390, 1391, 1392, 1393, 1394 }, {}, NOFLAG } } },							// Emily
	{ "NHG90", { { { 1490, 1491, 1492, 1493, 1494 }, {}, NOFLAG } } },							// Hannah
	{ "NLC90", { { { 1590, 1591, 1592, 1593, 1594 }, {}, NOFLAG } } },							// Louis
	{ "NNN90", { { { 1690, 1691, 1692, 1693, 1694 }, {}, NOFLAG } } },							// Ned
	{ "NRG90", { { { 1790, 1791, 1792, 1793, 1795, 1796 }, {}, NOFLAG } } },					// Rose
};

const Common::Array<Common::Array<const char *>> _nancy3ConditionalDialogueTexts {
{	// English
	// 00
	"I found out how you rigged the seance table with the projector. That was a pretty good show you gave.<h><n>", // NAS50
	"What kind of person is Charlie? He seems to feel bad about all of these mishaps.<h><n>", // NAS53
	"Have you seen the poem in my room?<h><n>", // NAS54
	"Do you know much about Louis? It must be great having your own expert on Victorians.<h><n>", // NAS55
	"How long have you known Rose?<h><n>", // NAS56
	// 05
	"I saw those papers in the parlor. Where did you find them?<h><n>", // NAS57
	"I heard someone crying in the hallway. Was that you?<h><n>", // NAS59
	"Do you know what 'Gum Bo Fu' means?<h><n>", // NLC60
	"I found out how you wired the house to make it 'haunted'. The only ghost who walks these halls is you, Abby.<h><n>", // NAS64
	"Does Charlie live around here?<h><n>", // NAS67
	// 10
	"You mentioned Charlie was 'suspicious'. How so?<h><n>", // NAS70
	"Did the house come with a lot of furniture?<h><n>", // NAS71
	"Do you know where I can find a paintscraper?<h><n>", // NAS74
	"Have you noticed the dead flowers in the parlor?<h><n>", // NAS75
	"I met the 'resident handyman', Charlie. He's pretty young and I don't think he has much experience.<h><n>", // NBG20
	// 15
	"Rose is really lucky. This antique dealer, Louis Chandler, is helping her out - although he doesn't know very much about the house's history.<h><n>", // NBG21
	"Listen to this. I found a secret attic and an old desk. It looks like no one's been in there for years.<h><n>", // NBG22
	"I found some letters written by E. Valdez. I guess he was the owner of a hotel named 'The Golden Gardenia'.<h><n>", // NBG24
	"Abby hosted a seance and contacted the spirit who's haunting the house.<h><n>", // NBG25
	"There was a small fire in the house but luckily I put it out. But the old papers that Abby found were destroyed.<h><n>", // NBG26
	// 20
	"I found a secret room in the basement and it looks like someone is living there.<h><n>", // NBG27
	"I was spying on Louis and saw him take a book from the library. He put it in his briefcase.<h><n>", // NBG30
	"Do either of you know what 'gum bo fu' means?<h><n>", // NBG31
	"I found a hidden passageway in the library.<h><n>", // NBG32
	"Can you guys give me a clue? I'm not sure what to do next.<h><n>", // NGB33
	// 25
	"Have you met Louis Chandler? What do you know about him?<h><n>", // NCM50
	"Can you tell me more about the accidents?<h><n>", // NCM53
	"Have you ever heard of someone named, Valdez?<h><n>", // NCM54
	"Charlie, I know your secret. I know you're living in the hidden room behind the saloon.<h><n>", // NCM55
	"How do you like working for Abby?<h><n>", // NCM57
	// 30
	"Charlie, I found this diskette. I think it's yours.<h><n>", // NCM60
	"Do you know what the words, 'gum bo fu' mean?<h><n>", // NEF40
	"Charlie, do you know where I can find a paint scraper?<h><n>", // NCM65
	"Do you know how I can get into Abby's room?<h><n>", // NCM67
	"Who was the last person you saw in the parlor before the fire?<h><n>", // NCM69
	// 35
	"Have you come across any hidden passageways down here?<h><n>", // NCM70
	"Have you seen the poem in the Chinese room?<h><n>", // NCM73
	"What is that small closet in the hallway for?<h><n>", // NCM74
	"Can you tell me about the Chinese writing system? I seem to come across a lot of Chinese symbols.<h><n>", // NEF30
	"I found some old papers in the house, plus a page from a phone directory dated 1894.<h><n>", // NEF31
	// 40
	"Have you ever heard of the Ladies Protection Society?<h><n>", // NEF32
	"What do you know about 'The Bandit's Treasure'?<h><n>", // NEF33
	"Have you heard of an antique dealer named Louis Chandler?<h><n>", // NEF34
	"Have you heard of an actress, Lizzie Applegate?<h><n>", // NEF35
	"Have you ever come across hidden rooms in Victorian mansions?<h><n>", // NEF38
	// 45
	"Do you know what the words, 'gum bo fu' mean?<h><n>", // NEF40
	"Do you know where Yerba Buena town is?<h><n>", // NEF41
	"Do you know anything about 'Valdez'?<h><n>", // NEF44
	"Abby is very strange. She really is convinced there's a ghost somewhere in the house.<h><n>", // NHG20
	"I met Rose's handyman, Charlie. He seems nice.<h><n>", // NHG23
	// 50
	"Did you know Rose has a resident expert on Victorians?<h><n>", // NHG26
	"Have you heard about the seance Abby hosted for Rose and me?<h><n>", // NHG29
	"Abby faked the seance. She rigged a table with a projector.<h><n>", // NHG33
	"I just found a hidden attic. I wonder if it has anything to do with all these accidents.<h><n>", // NHG34
	"There was a fire in the parlor but I put it out in time.<h><n>", // NHG37
	// 55
	"Hannah, do you think Rose could have started the fire to collect the insurance on the house?<h><n>", // NHG38
	"This house is full of surprises. I found a secret room in the basement where someone's been living.<h><n>", // NHG39
	"Louis is up to something. I saw him take a book from the library.<h><n>", // NHG40
	"Louis's book mentioned that this house was once called 'gum bo fu' in the 1800's.<h><n>", // NHG41
	"It turns out Charlie is the one living in the basement.<h><n>", // NHG42
	// 60
	"I was wondering whether you knew anything about someone named 'E. Valdez'?<h><n>", // NLC50
	"Have you ever heard of the Great Christmas gold robbery?<h><n>", // NLC51
	"What kind of antique store do you own?<h><n>", // NLC52
	"Was this house once a hotel?<h><n>", // NLC53
	"Have you seen a paint scraper anywhere?<h><n>", // NLC54
	// 65
	"Do you know what a phoenix is?<h><n>", // NLC55
	"Do you know why the fireplace in the parlor didn't have a screen?<h><n>", // NLC57
	"Have you found any secret passage ways in this house?<h><n>", // NLC58
	"Do you think Rose should sell the house?<h><n>", // NLC59
	"Do you know what 'gum bo fu' means?<h><n>", // NLC60
	// 70
	"Do you know who Lizzie Applegate was?<h><n>", // NLC62
	"What was the Ladies Protection Society?<h><n>", // NLC65
	"Have you seen the poem in my room?<h><n>", // NAS54
	"Do you think Charlie is doing a good job?<h><n>", // NLC67
	"Did Lizzie ever wear men's clothing?<h><n>", // NLC68
	// 75
	"Where did Abby find those papers that are in the parlor?<h><n>", // NRM17
	"How did you find Charlie?<h><n>", // NRM18
	"How do you know Louis?<h><n>", // NRM19
	"How did you meet Abby?<h><n>", // NRM20
	"Excuse me for prying, but why did you spend so much money to insure the house against fire?<h><n>", // NRM21
	// 80
	"Are you missing any papers?<h><n>", // NRM55
	"Have you heard of someone named Lizzie Applegate?<h><n>", // NRM22
	"What is Abby planning for tonight?<h><n>", // NRM23
	"Have you found any rainbow designs in the house?<h><n>", // NRM24
	"Do you think Charlie is responsible for these accidents?<h><n>", // NRM25
	// 85
	"Does Charlie live around here?<h><n>", // NAS67
	"Do you know why there's a speaker in the air vent?<h><n>", // NRM27
	"Whose laptop is that in the library?<h><n>", // NRM28
	"Why are there dead roses in the parlor?<h><n>", // NRM29
	"Do you know what 'gum bo fu' means?<h><n>", // NLC60
},
{	// Russian
	// 00
	"Z pyf-, xnj ds gjlcnhjbkb ctfyc. Kjdrj ghblevfyj.<h><n>", // NAS50
	"Ds vj&tnt hfccrfpfnm j XfhkbM Rf&tncz, jy jxtym hfccnhjty bpSpf dctuj ghjbc(jlzotuj.<h><n>", // NAS53
	"Ds dbltkb gj?ve d vjtq rjvyfntM<h><n>", // NAS54
	"Pljhjdj, yfdthyjt, xnj dfv gjvjuftn ?rcgthn gj Dbrnjhbfycrjq ?gj(t.<h><n>", // NAS55
	"Rfr lfdyj ds pyfrjvs c HjepM<h><n>", // NAS56
	// 05
	"Z ghjcvjnhtkf ljrevtyns d ujcnbyjq. Ult ds b( yfikbM<h><n>", // NAS57
	"Z cksifkf d rjhbljht xtqSnj gkfx. Rnj ?nj vju ;snmM<h><n>", // NAS59
	"Ds pyftnt, xnj nfrjt 'Ufv /j Ae'M<h><n>", // NLC60
	"Z yfikf fggfhfnehe, c gjvjom- rjnjhjq ds j;vfysdfkb yfc. Tckb d ?njv ljvt b tcnm ghbphfr, nfr ?nj S ds, =;;b.<h><n>", // NAS64
	"Xfhkb ytlfktrj &bdtnM<h><n>", // NAS67
	// 10
	"Ds ujdjhbkb, xnj d Xfhkb tcnm xnjSnj gjljphbntkmyjt. GjxtveM<h><n>", // NAS70
	"Ds regbkb ljv dvtcnt c vt;tkm-M<h><n>", // NAS71
	"Ds yt gjlcrf&tnt, ult vj&yj yfqnb igfntkmM<h><n>", // NAS74
	"Ds pfvtnbkb, xnj wdtns d ujcnbyjq lfdyj pfdzkbM<h><n>", // NAS75
	"Z gjpyfrjvbkfcm c hf;jxbv, Xfhkb. Jy ljdjkmyj vjkjl. Yj vyt rf&tncz, xnj tve yt (dfnftn jgsnf.<h><n>", // NBG20
	// 15
	"Hjep jxtym gjdtpkj. Tq gjvjuftn njhujdtw fynbrdfhbfnjv, Kebc Xtylkth. +jnz jy b yt jxtym (jhjij pyftn bcnjhb- jcj;yzrf.<h><n>", // NBG21
	"Pyftnt, z yfikf xthlfr b cnfhbyysq ctrhtnth. Rf&tncz, nelf ybrnj yt pfukzlsdfk djn e&t ytcrjkmrj ktn.<h><n>", // NBG22
	"Z yfikf gbcmvf c gjlgbcm- =. Dfkmltp. Levf-, xnj ?njn xtkjdtr ;sk dkfltkmwtv jntkz 'Pjkjnfz ufhltybz'.<h><n>", // NBG24
	"=;;b ecnhjbkf cgbhbnbxtcrbq ctfyc b ghbpdfkf le(f, rjnjhsq &bdtn d jcj;yzrt.<h><n>", // NBG25
	"Pltcm ghjbpjitk yt;jkmijq gj&fh, yj vyt elfkjcm tuj gjneibnm. Ghfdlf, ljrevtyns, rjnjhst yfikf =;;b, ;skb eybxnj&tys.<h><n>", // NBG26
	// 20
	"..Z yfikf gjnfqye- rjvyfne d gjldfkt. Rf&tncz, nfv rnjSnj &bdtn.<h><n>", // NBG27
	"Z dbltkf, rfr Kebc dpzk jlye bp rybu d ;b;kbjntrt. Jy gjkj&bk tt d cdjq lbgkjvfn.<h><n>", // NBG30
	"Ds pyftnt, xnj nfrjt 'Ufv /j Ae'M<h><n>", // NBG31
	"Z yfikf gjnfqye- rjvyfne d ;b;kbjntrt.<h><n>", // NBG32
	"Ds yt vjukb ;s lfnm vyt gjlcrfpreM<h><n>", // NBG33
	// 25
	"Ns e&t dcnhtxfk Kebcf XtylkthfM Ns xnjSyb;elm j ytv pyftimM<h><n>", // NCM50
	"Ns vj&tim hfccrfpfnm vyt ;jkmit j ytcxfcnys( ckexfz(M<h><n>", // NCM53
	"Ns pyftim xnjSyb;elm j xtkjdtrt gj bvtyb DfkmltpM<h><n>", // NCM54
	"Xfhkb, z pyf- ndjq ctrhtn. Z pyf-, xnj ns &bdtim d gjnfqyjq rjvyfnt d gjldfkt.<h><n>", // NCM55
	"Nt;t yhfdbncz hf;jnfnm yf =;;bM<h><n>", // NCM57
	// 30
	"Xfhkb, z yfikf ?ne lbcrtne. Levf-, jyf ndjz.<h><n>", // NCM60
	"Ns pyftim, xnj nfrjt 'Ufv /j Ae'M<h><n>", // NEF40
	"Xfhkb, ns pyftim, ult vj&yj dpznm igfntkmM<h><n>", // NCM65
	"Ns pyftim, rfr gjgfcnm d rjvyfne =;;bM<h><n>", // NCM67
	"Ns dbltk rjujSyb;elm d ujcnbyjq gthtl gj&fhjvM<h><n>", // NCM69
	// 35
	"Ns yf(jlbk d jcj;yzrt nfqyst (jlsM<h><n>", // NCM70
	"Ns dbltk gj?ve d rbnfqcrjq rjvyfntM<h><n>", // NCM73
	"Ns pyftim, xnj crhsdftncz pf yt;jkmijq ldthwtq d rjhbljhtM<h><n>", // NCM74
	"Ns yt vjukf ;s hfccrfpfnm j rbnfqcrjq cbcntvt gbcmvfM Ytlfdyj vyt pltcm gjgfkbcm bthjukbas.<h><n>", // NEF30
	"Z yfikf cnfhst ljrevtyns d ljvt b cnhfybwe ntktajyyjuj cghfdjxybrf 1894 u.<h><n>", // NEF31
	// 40
	"Ns cksifkf j; J;otcndt pfobns &tyobyM<h><n>", // NEF32
	"Xnj nt;t bpdtcnyj j 'Cjrhjdbot ;fylbnf'M<h><n>", // NEF33
	"Ns cksifkf j njhujdwt fynbrdfhbfnjv Kebct XtylkthtM<h><n>", // NEF34
	"Nt;t pyfrjvf frnhbcf Kbppb =ggkutqnM<h><n>", // NEF35
	"Ns cksifkf j gjnfqys( rjvyfnf( d dbrnjhbfycrb( jcj;yzrf(M<h><n>", // NEF38
	// 45
	"Ns pyftim, xnj pyfxbn ckjdj 'Ufv /j Ae'M<h><n>", // NEF40
	"Ns cksifkf j ujhjlt Qth;f /e?yfM<h><n>", // NEF41
	"Nt;t xnjSyb;elm bpdtcnyj j<n>DfkmltptM<h><n>", // NEF44
	"=;;b nfrfz cnhfyyfz. Jyf ltqcndbntkmyj dthbn d nj, xnj d ljvt &bdtn ghbphfr.<h><n>", // NHG20
	"E Hjep tcnm jlby hf;jnybr. Tuj pjden Xfhkb.<h><n>", // NHG23
	// 50
	"Ns pyfkf, xnj e Hjep tcnm ?rcgthn gj Dbrnjhbfycrjq ?gj(tM<h><n>", // NHG26
	"=;;b ghjdtkf lkz yfc c Hjep ctfyc dspsdfybz le(jd.<h><n>", // NHG29
	"Ctfyc =;;b jrfpfkcz abrwbtq. Gjl cnjkjv cnjzk ghjtrnjh.<h><n>", // NHG33
	"Z yfikf rjvyfne yf xthlfrt. Bynthtcyj, cdzpfyf kb jyf c gjcktlybvb cj;snbzvbM<h><n>", // NHG34
	"Ujcnbyfz xenm yt cujhtkf, yj z gjneibkf gj&fh.<h><n>", // NHG37
	// 55
	"+fyyf, Hjep vjukf ecnhjbnm gj&fh, xnj;s gjkexbnm cnhf(jdreM<h><n>", // NHG38
	"=njn ljv gjkjy c-hghbpjd. Z yfikf d gjldfkt nfqye- rjvyfne, d rjnjhjq rnjSnj &bdtn.<h><n>", // NHG39
	"Kebc xnjSnj pfvsikztn. Z dbltkf, rfr jy ;hfk rybue d ;b;kbjntrt.<h><n>", // NHG40
	"D rybut Kebcf yfgbcfyj, xnj d 19 dtrt ?njn ljv yfpsdfkcz 'Ufv /j Ae'.<h><n>", // NHG41
	"Jrfpfkjcm, xnj Xfhkb &bdtn d gjldfkt ljvf.<h><n>", // NHG42
	// 60
	"Z (jntkf cghjcbnm, ds pyftnt xnjSyb;elm j xtkjdtrt gj bvtyb =. DfkmltpM<h><n>", // NLC50
	"Ds cksifkb j Dtkbrjv hj&ltcndtycrjv juhf;ktybbM<h><n>", // NLC51
	"E dfc tcnm cj;cndtyysq fynbrdfhysq vfufpbyM<h><n>", // NLC52
	"Ds pyftnt, hfymit pltcm ;skf ujcnbybwfM<h><n>", // NLC53
	"Ds dbltkb ultSyb;elm igfntkmM<h><n>", // NLC54
	// 65
	"Ds pyftnt, xnj nfrjt atybrcM<h><n>", // NLC55
	"Ds pyftnt, gjxtve e rfvbyf d ujcnbyjq ytn pfobnyjuj ?rhfyfM<h><n>", // NLC57
	"Ds dbltkb d ?njv ljvt gjnfqyst rjvyfnsM<h><n>", // NLC58
	"Ds levftnt, xnj Hjep kexit ghjlfnm ljvM<h><n>", // NLC59
	"Ds pyftnt, xnj nfrjt 'Ufv /j Ae'M<h><n>", // NLC60
	// 70
	"Ds pyftnt, rnj nfrfz Kbppb =ggkutqnM<h><n>", // NLC62
	"Ds pyftnt j; J;otcndt pfobns &tyobyM<h><n>", // NLC65
	"Ds dbltkb gj?ve d vjtq rjvyfntM<h><n>", // NAS54
	"GjSdfitve, Xfhkb (jhjij hf;jnftnM<h><n>", // NLC67
	"Kbppb yjcbkf ve&cre- jlt&leM<h><n>", // NLC68
	// 75
	"Ult =;;b yfikf ;evfub, rjnjhst kt&fn d ujcnbyjqM<h><n>", // NRM17
	"Rfr ds yfikb XfhkbM<h><n>", // NRM18
	"Jnrelf ds pyftnt KebcfM<h><n>", // NRM19
	"Ult ds dcnhtnbkb =;;bM<h><n>", // NRM20
	"Ghjcnbnt pf k-;jgsncndj, yj gjxtve ds gjnhfnbkb nfr vyjuj ltytu yf cnhf(jdre jn gj&fhfM<h><n>", // NRM21
	// 80
	"E dfc yt ghjgflfkb ;evfubM<h><n>", // NRM55
	"Ds cksifkb j Kbppb =ggkutqnM<h><n>", // NRM22
	"Xnj =;;b gkfybhetn yf dtxthM<h><n>", // NRM23
	"Ds dbltkb d ljvt bpj;hf&tybz hfleubM<h><n>", // NRM24
	"Ds levftnt, xnj d gjcktlyb( cj;snbz( dbyjdfn XfhkbM<h><n>", // NRM25
	// 85
	"Xfhkb ytlfktrj &bdtnM<h><n>", // NAS67
	"Pfxtv d rjhbljht ecnfyjdkty lbyfvbrM<h><n>", // NRM27
	"Xtq yjen;er cnjbn d ;b;kbjntrtM<h><n>", // NRM28
	"Gjxtve d ujcnbyjq cnjzn edzlibt hjpsM<h><n>", // NRM29
	"Ds pyftnt, xnj nfrjt 'Ufv /j Ae'M<h><n>" // NLC60
}
};

const Common::Array<Common::Array<const char *>> _nancy3GoodbyeTexts = {
{	// English
	"I'll let you get back to what you were doing.<h>", // NAS90
	"I should get going. Talk to you later.<h>", // NBG90
	"I'll let you get back to your renovation.<h>", // NCM90
	"I should get going. Goodbye, Emily.<h>", // NEF90
	"I should get back to work. Goodbye.<h>", // NHG90
	"I won't keep you any longer.<h>", // NLC90
	"Goodbye, Ned.<h>", // NNN90
	"I can see you're busy - I'll let you go.<h>" // NRG90
},
{	// Russian
	"Yt ;ele dfv ;jkmit vtifnm.<h>", // NAS90
	"Vyt gjhf blnb. Tot edblbvcz.<h>", // NBG90
	"Vyt gjhf blnb. Tot edblbvcz.<h>", // NCM90
	"Vyt ye&yj blnb. Lj cdblfybz, =vbkb.<h>", // NEF90
	"Vyt e&t gjhf. Lj cdblfybz.<h>", // NHG90
	"/jkmit yt ;ele vtifnm.<h>", // NLC90
	"Gjrf, Y?l.<h>", // NNN90
	"Ye, yt ;ele ;jkmit vtifnm.<h>" // NRG90
}
};

const Common::Array<const char *> _nancy3TelephoneRinging = {
	"ringing...<n><e>", // English
	"Cjtlbytybt...<n><e>"  // Russian
};

const Common::Array<const char *> _nancy3EmptySaveStrings = {
	"Nothing Saved Here",	// English
	"- - - - -"				// Russian
};

const Common::Array<const char *> _nancy3EventFlagNames = {
	"Generic 0",
	"Generic 1",
	"Generic 2",
	"Generic 3",
	"Generic 4",
	"Generic 5",
	"Generic 6",
	"Generic 7",
	"Generic 8",
	"Generic 9",
	"Generic 10",
	"Generic 11",
	"Generic 12",
	"Generic 13",
	"Generic 14",
	"Generic 15",
	"Generic 16",
	"Generic 17",
	"Generic 18",
	"Generic 19",
	"Generic 20",
	"Generic 21",
	"Generic 22",
	"Generic 23",
	"Generic 24",
	"Generic 25",
	"Generic 26",
	"Generic 27",
	"Generic 28",
	"Generic 29",
	"Generic 30",
	"time for end game",
	"player won game",
	"stop player scrolling",
	"easter eggs",
	"A said accident",
	"A said address",
	"A said Charlie",
	"A said confess",
	"A said crying",
	"A said fire",
	"A said furniture",
	"A said ghost",
	"A said gumbo ",
	"A said Louis",
	"A said papers",
	"A said poem",
	"A said publicity",
	"A said rents",
	"A said rig",
	"A said rose",
	"A said roses",
	"A said seance",
	"A said sneaky",
	"Abby available",
	"attic rope broke",
	"attic tile01 off",
	"attic tile02 off",
	"attic tile03 off",
	"attic tile04 off",
	"attic tile05 off",
	"attic tile06 off",
	"attic tile07 off",
	"attic tile08 off",
	"attic tile09 off",
	"attic tile10 off",
	"attic tile11 off",
	"attic tile12 off",
	"attic tile13 off",
	"attic tile14 off",
	"attic tile15 off",
	"BG said bookcase",
	"BG said Charlie",
	"BG said desk",
	"BG said fire",
	"BG said gardenia",
	"BG said gumbo",
	"BG said Louis",
	"BG said policy",
	"BG said seance",
	"BG said secret",
	"BG said steal",
	"bookshelf open",
	"C said Abby",
	"C said accidents",
	"C said apology",
	"C said Diablo",
	"C said diskette",
	"C said ditch",
	"C said fire",
	"C said gold",
	"C said gumbo",
	"C said haunted",
	"C said Louis",
	"C said scraper",
	"C said screen",
	"C said seance",
	"C said secret",
	"C said thump",
	"C said upset",
	"C said Valdez",
	"C upset",
	"chain1 off",
	"chain2 off",
	"Charlie available",
	"diskette in laptop",
	"E said attic",
	"E said bandit",
	"E said directory",
	"E said gumbo",
	"E said Hanzi",
	"E said ladies",
	"E said Lizzie",
	"E said Louis",
	"E said Mexico",
	"E said recluse",
	"E said Valdez",
	"E said Victoria",
	"E said Yerba",
	"event01",
	"event02",
	"event03",
	"event04",
	"event05",
	"event06",
	"event07",
	"event08",
	"event09",
	"event10",
	"H said Abby",
	"H said attic",
	"H said Charlie",
	"H said confession",
	"H said fire",
	"H said gumbo",
	"H said haunted",
	"H said Louis",
	"H said owner",
	"H said policy",
	"H said projector",
	"H said seance",
	"H said secret",
	"H said spindle",
	"H said steal",
	"H said Valdez",
	"heard argument",
	"heard crying",
	"heard door shut",
	"L said antique",
	"L said busy",
	"L said Charlie",
	"L said christmas",
	"L said fire",
	"L said gumbo",
	"L said hotel",
	"L said insulate",
	"L said ladies",
	"L said Lizzie",
	"L said papers",
	"L said Phoenix",
	"L said poem",
	"L said scraper",
	"L said screen",
	"L said sellout",
	"L said thump",
	"L said Valdez",
	"Louis available",
	"met Abby",
	"met BG",
	"met Charlie",
	"met Emily",
	"met Louis",
	"met Ned",
	"met Rose",
	"N said haunt",
	"N said treasure",
	"opened attic",
	"R said Abby",
	"R said ashes",
	"R said Charlie",
	"R said dumbwaiter",
	"R said fire",
	"R said Hx",
	"R said inlay",
	"R said Lizzie",
	"R said Louis",
	"R said papers",
	"R said policy",
	"R said rainbow",
	"R said seance",
	"R said sellout",
	"R said tiles",
	"R said tonight",
	"R said winter",
	"Rose available",
	"said stunt",
	"saw ashes",
	"saw bandits",
	"saw blink",
	"saw desk lock",
	"saw directory",
	"saw E note",
	"saw gardenia",
	"saw gumbo",
	"saw Hanzi",
	"saw Kehne letter",
	"saw Lizzie book",
	"saw locked Armoire",
	"saw Louis steal",
	"saw mirror",
	"saw music",
	"saw papers",
	"saw passage",
	"saw poem",
	"saw policy",
	"saw projector",
	"saw pyramid",
	"saw rainbow",
	"saw recorder",
	"saw report",
	"saw roses",
	"saw safe",
	"saw seance",
	"saw secret room",
	"saw shadow",
	"saw speaker",
	"saw threat",
	"saw treasure map",
	"saw Valdez",
	"saw zodiac",
	"screw01 off",
	"screw02 off",
	"screw03 off",
	"screw04 off",
	"solved attic",
	"solved brief",
	"solved brief left",
	"solved brief right",
	"solved charm",
	"solved desk",
	"solved dumbwaiter",
	"solved fire",
	"solved fiver",
	"solved inlay",
	"solved mantle",
	"solved piano",
	"solved safe",
	"solved slider",
	"solved spindle",
	"solved template",
	"solved zodiac",
	"stop spooky01",
	"stop spooky02",
	"stop spooky 03",
	"stop spooky04",
	"stop spooky05",
	"stop spooky06",
	"stop spooky07",
	"stop spooky08",
	"stop spooky09",
	"stop spooky10",
	"tape in deck",
	"tile01 off",
	"tile02 off",
	"tile03 off",
	"tile04 off",
	"tile05 off",
	"tile06 off",
	"tile07 off",
	"tile08 off",
	"tile09 off",
	"tile1 - pos1",
	"tile1 pos2",
	"tile1 pos3",
	"tile10 off",
	"tile11 off",
	"tile12 off",
	"tile13 off",
	"tile14 off",
	"tile15 off",
	"tile2 pos1",
	"tile2 pos2",
	"tile2 pos3",
	"tile3 pos1",
	"tile3 pos2",
	"tile3 pos3",
	"saw gardenia letter",
	"saw robbery book",
	"A said men",
	"sound 0",
	"sound 1",
	"sound 2",
	"sound 3",
	"sound 4",
	"A said scraper",
	"BG said Louis Steal",
	"empty",
	"saw bookcase",
	"solved tiles",
	"R said Iowa",
	"C said Knox",
	"C said last ",
	"C said trap",
	"C said poem",
	"saw dumbwaiter",
	"C said dumb",
	"Louis says Lizzie",
	"saw Yerba",
	"R said trust",
	"R said PA",
	"R said roses",
	"saw laptop",
	"R said laptop",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty"
};

const Common::Array<const char *> nancy3PatchSrcFiles {
	"han92b.his"
};

// Patch notes:
// - The missing sound file is a patch from the original devs. Should only be enabled in the English version
const Common::Array<PatchAssociation> nancy3PatchAssociations {
	{ { "language", "en" }, { "han92b.his" } }
};

#endif // NANCY3DATA_H
