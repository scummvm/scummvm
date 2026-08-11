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

#ifndef NANCY5DATA_H
#define NANCY5DATA_H

#include "types.h"

const GameConstants _nancy5Constants ={
	33,												// numItems
	456,											// numEventFlags
	{	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,			// genericEventFlags
		11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
		21, 22, 23, 24, 25, 26, 27, 28, 29, 30 },
	12,												// numCursorTypes
	4000,											// logoEndAfter
	32												// wonGameFlagID
};

const Common::Array<GameLanguage> _nancy5LanguagesOrder = {
	GameLanguage::kEnglish,
	GameLanguage::kRussian
};

const Common::Array<Common::Array<ConditionalDialogue>> _nancy5ConditionalDialogue = {
{	// Brady, 7 responses + 1 repeat
	{	0, 1050, "NBA50",
		{ { kEv, 142, true }, { kEv, 47, false } } },
	{	1, 1051, "NBA051",
		{ { kEv, 116, true }, { kEv, 46, false } } },
	{	1, 1051, "NBA051",
		{ { kEv, 138, true }, { kEv, 46, false } } },
	{	2, 1052, "NBA052",
		{ { kEv, 129, true }, { kEv, 40, false } } },
	{	3, 1053, "NBA053",
		{ { kEv, 129, true }, { kEv, 42, false } } },
	{	4, 1054, "NNF652",
		{ { kEv, 129, true }, { kEv, 119, true }, { kEv, 37, false } } },
	{	5, 1060, "NBA061",
		{ { kEv, 43, true }, { kEv, 45, false } } },
	{	6, 1061, "NNF654",
		{ { kEv, 58, true }, { kEv, 44, false } } },
},
{	// Simone, 7 responses
	{	7, 1250, "NSM250",
		{ { kEv, 138, true }, { kEv, 46, true }, { kEv, 213, false } } },
	{	8, 1251, "NSM251",
		{ { kEv, 183, true }, { kEv, 223, false } } },
	{	9, 1252, "NSM252",
		{ { kEv, 119, true }, { kEv, 222, false } } },
	{	6, 1253, "NNF654",
		{ { kEv, 58, true }, { kEv, 133, true }, { kEv, 218, false } } },
	{	10, 1254, "NSM254",
		{ { kEv, 202, true }, { kEv, 215, false } } },
	{	4, 1255, "NNF652",
		{ { kEv, 138, true }, { kEv, 119, true }, { kEv, 212, false } } },
	{	11, 1256, "NSM256",
		{ { kEv, 49, true }, { kEv, 216, false } } },
},
{	// Joseph, 9 responses + 2 repeat
	{	12, 1450, "NJH450",
		{ { kEv, 132, true }, { kEv, 164, true }, { kEv, 105, false } } },
	{	13, 1452, "NJH452",
		{ { kEv, 270, true }, { kEv, 59, false }, { kEv, 117, false } } },
	{	14, 1453, "NJH453",
		{ { kEv, 107, true }, { kEv, 120, false } } },
	{	14, 1453, "NJH453",
		{ { kEv, 152, true }, { kEv, 120, false } } },
	{	15, 1455, "NJH455",
		{ { kEv, 152, true }, { kEv, 108, false } } },
	{	15, 1455, "NJH455",
		{ { kEv, 155, true }, { kEv, 108, false } } },
	{	16, 1456, "NJH456",
		{ { kEv, 166, true }, { kEv, 111, false } } },
	{	17, 1457, "NJH457",
		{ { kEv, 238, true }, { kEv, 118, false } } },
	{	18, 1459, "NJH459",
		{ { kEv, 58, true }, { kEv, 81, true }, { kEv, 101, false } } },
	{	19, 1460, "NJH460",
		{ { kEv, 105, true }, { kEv, 187, true }, { kEv, 112, false } } },
	{	20, 1465, "NJH465",
		{ { kEv, 59, true }, { kEv, 102, false } } },
},
{	// Nicholas, 7 responses + 1 repeat
	{	21, 1650, "NNF650",
		{ { kEv, 159, true }, { kEv, 156, false } } },
	{	21, 1650, "NNF650",
		{ { kEv, 166, true }, { kEv, 156, false } } },
	{	22, 1651, "NNF651",
		{ { kEv, 135, true }, { kEv, 155, false } } },
	{	4, 1652, "NNF652",
		{ { kEv, 135, true }, { kEv, 119, true }, { kEv, 147, false } } },
	{	6, 1654, "NNF654",
		{ { kEv, 58, true }, { kEv, 133, true }, { kEv, 154, false } } },
	{	23, 1655, "NNF655",
		{ { kEv, 58, true }, { kEv, 219, true }, { kEv, 169, false }, { kEv, 150, false } } },
	{	24, 1658, "NNF658",
		{ { kEv, 155, true }, { kEv, 148, false } } },
	{	25, 1659, "NNF659",
		{ { kEv, 108, true }, { kEv, 153, false } } },
},
{	// Bess and George, 14 responses
	{	26, 1820, "NBG20",
		{ { kEv, 105, true }, { kEv, 187, true }, { kEv, 386, false } } },
	{	27, 1821, "NBG21",
		{ { kEv, 54, true }, { kEv, 383, false } } },
	{	28, 1822, "NBG22",
		{ { kEv, 166, true }, { kEv, 399, false } } },
	{	29, 1823, "NBG23",
		{ { kEv, 214, true }, { kEv, 385, false } } },
	{	30, 1824, "NBG24",
		{ { kEv, 219, true }, { kEv, 168, false }, { kEv, 390, false } } },
	{	31, 1825, "NBG25",
		{ { kEv, 46, true }, { kEv, 393, false } } },
	{	32, 1826, "NBG26",
		{ { kEv, 58, true }, { kEv, 207, false }, { kEv, 398, false } } },
	{	33, 1827, "NBG27",
		{ { kEv, 135, true }, { kEv, 395, false } } },
	{	34, 1828, "NBG28",
		{ { kEv, 152, true }, { kEv, 397, false } } },
	{	35, 1830, "NBG30",
		{ { kEv, 201, true }, { kEv, 234, false }, { kEv, 387, false } } },
	{	36, 1831, "NBG31",
		{ { kEv, 55, true }, { kEv, 51, true }, { kEv, 52, false }, { kEv, 384, false } } },
	{	37, 1832, "NBG32",
		{ { kEv, 81, true }, { kEv, 392, false } } },
	{	38, 1870, "NBG70",
		{ { kEv, 401, true }, { kEv, 389, false }, { kDi, 2, true } } },
	{	39, 1871, "NBG71",
		{ { kEv, 389, true }, { kDi, 2, true } } },
},
{	// Ned, 14 responses + 3 repeats
	{	40, 1920, "NDN20",
		{ { kEv, 138, true }, { kEv, 415, false } } },
	{	41, 1921, "NDN21",
		{ { kEv, 108, true }, { kEv, 153, true }, { kEv, 406, false } } },
	{	42, 1922, "NDN22",
		{ { kEv, 324, true }, { kEv, 413, false } } },
	{	43, 1923, "NDN23",
		{ { kEv, 169, true }, { kEv, 168, false }, { kEv, 184, true }, { kEv, 85, false }, { kEv, 410, false } } },
	{	44, 1924, "NDN24",
		{ { kEv, 232, true }, { kEv, 408, false }, { kIn, 4, true } } },
	{	45, 1925, "NDN25",
		{ { kEv, 202, true }, { kEv, 405, false } } },
	{	45, 1925, "NDN25",
		{ { kEv, 167, true }, { kEv, 405, false } } },
	{	45, 1925, "NDN25",
		{ { kEv, 104, true }, { kEv, 405, false } } },
	{	46, 1926, "NDN26",
		{ { kEv, 211, true }, { kEv, 419, false } } },
	{	47, 1927, "NDN27",
		{ { kEv, 148, true }, { kEv, 50, false }, { kEv, 414, false } } },
	{	48, 1928, "NDN28",
		{ { kEv, 117, true }, { kEv, 93, false }, { kEv, 411, false } } },
	{	49, 1929, "NDN29",
		{ { kEv, 346, true } } },
	{	49, 1929, "NDN29",
		{ { kEv, 171, true }, { kEv, 412, false } } },
	{	50, 1930, "NDN30",
		{ { kEv, 268, true }, { kEv, 208, false }, { kEv, 409, false } } },
	{	51, 1931, "NDNj",
		{ { kEv, 238, true }, { kEv, 403, false } } },
	{	52, 1970, "NDN70",
		{ { kEv, 402, true }, { kEv, 422, false }, { kDi, 2, true } } },
	{	53, 1970, "NDN71",
		{ { kEv, 402, true }, { kDi, 0, true } } },
}
};

const Common::Array<Goodbye> _nancy5Goodbyes = {
	{ "NBA090", { { { 1090, 1091, 1092, 1094 }, {}, NOFLAG } } },					// Brady
	{ "NSM290", { { { 1290, 1291, 1292, 1293 }, {}, NOFLAG } } },					// Simone
	{ "NJH90", { { { 1490, 1491, 1492, 1493 }, {}, NOFLAG } } },					// Joseph
	{ "NNF690", { { { 1690, 1691, 1692 }, {}, NOFLAG } } },							// Nicholas
	{ "NBG90", { { { 1890, 1891, 1892, 1893, 1894 }, {}, NOFLAG } } },				// Bess & George
	{ "NDN90", { { { 1990, 1991, 1992, 1993, 1994, 1995, 1996 }, {}, NOFLAG } } },	// Ned
};

const Common::Array<Common::Array<const char *>> _nancy5ConditionalDialogueTexts {
{	// English
	// 00
	"Don't you want to know what Maya's article was about?<h><n>", // NBA50
	"Who's this 'Simone' and how do you know her?<h><n>", // NBA051
	"How do you keep up with all of your fans, Brady? A big star like you must get hundreds of emails.<h><n>", // NBA052
	"Do you mind if I take a look around in here?<h><n>", // NBA053
	"I'm asking everyone. Where were you when the kidnapping happened?<h><n>", // NNF652
	// 05
	"Why are you interested in helping me?<h><n>", // NBA061
	"Have you seen Joseph?<h><n>", // NNF654
	"How long have you been Brady's agent?<h><n>", // NSM250
	"What do you think about Brady's new project?<h><n>", // NSM251
	"Joseph says the building was probably locked when the kidnapping happened and that the kidnapper had to be someone who was inside the theater. Care to comment?<h><n>", // NSM252
	// 10
	"Have you heard about this ransom demand?<h><n>", // NSM254
	"Brady seems uncomfortable with some of your, uh, 'tactics.'<h><n>", // NSM256
	"So, where will you go after the demolition? Are you planning to retire?<h><n>", // NJH450
	"Tell me about that old key making machine in the lobby. Has it been out of order long?<h><n>", // NJH452
	"Who was this JJ Thompson character, anyway?<h><n>", // NJH453
	// 15
	"The history of this theater is so rich. I'm surprised the city of St. Louis isn't more interested in preserving it.<h><n>", // NJH455
	"The police told me Nicholas Falcone is known for using 'extreme tactics' to further his political causes. Why are you letting him use the lobby as his headquarters?<h><n>", // NJH456
	"I was backstage earlier and one of the stage lights came crashing down. It almost hit me!<h><n>", // NJH457
	"I got another threat from the kidnapper! That creepy voice came on the PA and told me to stop searching for Maya!<h><n>", // NJH459
	"Joseph - I did some checking. You don't really have any family in Greasewood, do you?<h><n>", // NJH460
	// 20
	"It's only a matter of hours until the demolition! How are you holding up, Joseph?<h><n>", // NJH465
	"Tell me about your relationship with the police.<h><n>", // NNF650
	"So, 'Humans Against the Destruction of Illustrious Theaters.' Tell me about it.<h><n>", // NNF651
	"Can you believe Brady's agent has called this press conference to trump up his heroics? She can barely even remember Maya's name.<h><n>", // NNF655
	"Do you know what they plan to build on this spot after the theater is gone?<h><n>", // NNF658
	// 25
	"Joseph told me that the St. Louis Historical Society is in the process of trying to declare this building a historical landmark.<h><n>", // NNF659
	"I asked Joseph where he would go after the theater is demolished, and he said he was moving to Arizona to start a movie theater with his brother.<h><n>", // NBG20
	"I took Joseph's advice and called County Administration to see about the blueprints for the theater.<h><n>", // NBG21
	"When I spoke to the police about Maya's press pass, they said that Nicholas is known for using extreme tactics and that he might even have been involved in a staged kidnapping last year in Nashville.<h><n>", // NBG22
	"I think Simone Mueller would do just about anything for publicity.<h><n>", // NBG23
	// 30
	"Simone has called a press conference.  I'd like to hear what's being said, but I get the feeling I'm not welcome.<h><n>", // NBG24
	"Brady said Simone wants him to appear 'wholesomely smoldering' to his public.  What in the world do you think that means?<h><n>", // NBG25
	"When I arrived at the theater this morning, someone had delivered a funeral wreath!<h><n>", // NBG26
	"I met Nicholas Falcone, the leader of HAD IT.  His organization is camped out in front of the theater, demonstrating against the demolition.<h><n>", // NBG27
	"Nicholas told me that his grandmother did a lot of the artwork inside the theater, but that JJ Thompson, the original owner, never paid her or even gave her credit for it!<h><n>", // NBG28
	// 35
	"I came across this old book that explains how to do a bunch of magic tricks.<h><n>", // NBG30
	"I spoke to Sherman Trout at the Library of Congress.  He said he'll search Houdini's letters for some indication of what happened to Houdini's half of the theater.<h><n>", // NBG31
	"I received another threat!  This time the kidnapper addressed me by name!<h><n>", // NBG32
	"Help! I'm a little stuck!<h><n>", // NBG70
	"Ladies, I need a hint!<h><n>", // NBG71
	// 40
	"By the time I finally got to talk to Simone Mueller, she had heard all about the kidnapping and cancelled the premiere of Vanishing Destiny.<h><n>", // NDN20
	"According to Joseph, the St. Louis Historical society has been scrambling to get the Royal Palladium declared a historical landmark.<h><n>", // NDN21
	"I listened in on a call between Brady and Simone.  It was weird.  Brady sounds like he's cracking up.<h><n>", // NDN22
	"I found an old stage technician's guide that explains how some of the illusions were done in the old days when the theater was still used for stage performances.<h><n>", // NDN23
	"I outsmarted the Amazing Monty, now why can't I catch this kidnapper?<h><n>", // NDN24
	// 45
	"I can't understand how the police could've received a ransom call from Granite City!  And why would the kidnapper suddenly demand cash, without even mentioning the demolition!<h><n>", // NDN25
	"I confronted Simone about the receipt for the funeral wreath, and she claimed she just needed something for a photo shoot.  Would you buy that?<h><n>", // NDN26
	"I've got to think of a way to get this demolition stopped!<h><n>", // NDN27
	"There's this cool machine in the lobby that makes souvenir keys.  You can make a key in any shape and call it the key to your heart.  I wanted to make one for you -- not that this is any time to think of souvenirs.<h><n>", // NDN28
	"Harry Houdini must've been quite a character!<h><n>", // NDN29
	// 50
	"Nicholas says that the police are about to start clearing the building.<h><n>", // NDN30
	"I was backstage earlier and I almost got hit with by a falling stage light!<h><n>", // NDNj
	"I could use a hint.<h><n>", // NDN70
	"I'm trying to decide what to do next. Any thoughts?<h><n>" // NDN71
},
{	// Russian
	// Unlike previous titles which just mapped Russian letters to ASCII,
	// nancy5's Russian translation introduces strings in Windows-1251 encoding
	// These are escaped here for portability's sake

	// 00
	"\xc2\xfb \xed\xe5 \xf5\xee\xf2\xe8\xf2\xe5 \xf3\xe7\xed\xe0\xf2\xfc, \xee \xf7\xe5\xec \xf1\xee\xe1\xe8\xf0\xe0\xeb\xe0\xf1\xfc \xef\xe8\xf1\xe0\xf2\xfc \xcc\xe0\xe9\xff?<h><n>", // NBA50
	"\xca\xf2\xee \xf2\xe0\xea\xe0\xff \xd1\xe8\xec\xee\xed\xe0 \xe8 \xee\xf2\xea\xf3\xe4\xe0 \xe2\xfb \xe5\xe5 \xe7\xed\xe0\xe5\xf2\xe5?<h><n>", // NBA051
	"\xc1\xf0\xfd\xe4\xe8, \xe2\xfb \xef\xee\xe4\xe4\xe5\xf0\xe6\xe8\xe2\xe0\xe5\xf2\xe5 \xef\xe5\xf0\xe5\xef\xe8\xf1\xea\xf3 \xf1\xee \xf1\xe2\xee\xe8\xec\xe8 \xef\xee\xea\xeb\xee\xed\xed\xe8\xea\xe0\xec\xe8? \xd2\xe0\xea\xe0\xff \xe7\xe2\xe5\xe7\xe4\xe0, \xea\xe0\xea \xe2\xfb, \xed\xe0\xe2\xe5\xf0\xed\xee\xe5, \xef\xee\xeb\xf3\xf7\xe0\xe5\xf2 \xf1\xee\xf2\xed\xe8 \xef\xe8\xf1\xe5\xec.<h><n>", // NBA052
	"\xcc\xee\xe6\xed\xee \xee\xf1\xec\xee\xf2\xf0\xe5\xf2\xfc \xe2\xe0\xf8\xf3 \xe3\xf0\xe8\xec\xe5\xf0\xed\xf3\xfe?<h><n>", // NBA053
	"\xdf \xe2\xf1\xe5\xec \xe7\xe0\xe4\xe0\xfe \xfd\xf2\xee\xf2 \xe2\xee\xef\xf0\xee\xf1: \xe3\xe4\xe5 \xe2\xfb \xe1\xfb\xeb\xe8 \xe2\xee \xe2\xf0\xe5\xec\xff \xef\xee\xf5\xe8\xf9\xe5\xed\xe8\xff?<h><n>", // NNF652
	// 05
	"\xcf\xee\xf7\xe5\xec\xf3 \xe2\xfb \xf5\xee\xf2\xe8\xf2\xe5 \xef\xee\xec\xee\xf7\xfc \xec\xed\xe5?<h><n>", // NBA061
	"\xc2\xfb \xe2\xe8\xe4\xe5\xeb\xe8 \xc4\xe6\xee\xe7\xe5\xf4\xe0?<h><n>", // NNF654
	"\xca\xe0\xea \xe4\xee\xeb\xe3\xee \xe2\xfb \xf0\xe0\xe1\xee\xf2\xe0\xe5\xf2\xe5 \xe0\xe3\xe5\xed\xf2\xee\xec \xc1\xf0\xfd\xe4\xe8?<h><n>", // NSM250
	"\xd7\xf2\xee \xe2\xfb \xe4\xf3\xec\xe0\xe5\xf2\xe5 \xee \xed\xee\xe2\xee\xec \xef\xf0\xee\xe5\xea\xf2\xe5 \xc1\xf0\xfd\xe4\xe8?<h><n>", // NSM251
	"\xc4\xe6\xee\xe7\xe5\xf4 \xf1\xea\xe0\xe7\xe0\xeb, \xf7\xf2\xee \xf2\xe5\xe0\xf2\xf0, \xe2\xe5\xf0\xee\xff\xf2\xed\xee, \xe7\xe0\xea\xf0\xfb\xeb\xe8 \xef\xe5\xf0\xe5\xe4 \xef\xee\xf5\xe8\xf9\xe5\xed\xe8\xe5\xec. \xc0 \xe7\xed\xe0\xf7\xe8\xf2, \xef\xf0\xe5\xf1\xf2\xf3\xef\xed\xe8\xea \xed\xe0\xf5\xee\xe4\xe8\xf2\xf1\xff \xe2\xed\xf3\xf2\xf0\xe8.<h><n>", // NSM252
	// 10
	"\xc2\xfb \xf1\xeb\xfb\xf8\xe0\xeb\xe8 \xee \xf2\xf0\xe5\xe1\xee\xe2\xe0\xed\xe8\xe8 \xe2\xfb\xea\xf3\xef\xe0?<h><n>", // NSM254
	"\xca\xe0\xe6\xe5\xf2\xf1\xff, \xc1\xf0\xfd\xe4\xe8 \xed\xe5 \xed\xf0\xe0\xe2\xe8\xf2\xf1\xff \xe2\xe0\xf8\xe0 \xf2\xe0\xea\xf2\xe8\xea\xe0.<h><n>", // NSM256
	"\xd7\xf2\xee \xe2\xfb \xe1\xf3\xe4\xe5\xf2\xe5 \xe4\xe5\xeb\xe0\xf2\xfc \xef\xee\xf1\xeb\xe5 \xf2\xee\xe3\xee, \xea\xe0\xea \xf2\xe5\xe0\xf2\xf0 \xf1\xed\xe5\xf1\xf3\xf2? \xca\xf3\xe4\xe0-\xed\xe8\xe1\xf3\xe4\xfc \xf3\xe5\xe4\xe5\xf2\xe5?<h><n>", // NJH450
	"\xc2\xfb \xec\xee\xe6\xe5\xf2\xe5 \xf0\xe0\xf1\xf1\xea\xe0\xe7\xe0\xf2\xfc \xf7\xf2\xee-\xed\xe8\xe1\xf3\xe4\xfc \xee\xe1 \xe0\xe2\xf2\xee\xec\xe0\xf2\xe5 \xef\xee \xf1\xee\xe7\xe4\xe0\xed\xe8\xfe \xea\xeb\xfe\xf7\xe5\xe9, \xea\xee\xf2\xee\xf0\xfb\xe9 \xf1\xf2\xee\xe8\xf2 \xe2 \xf5\xee\xeb\xeb\xe5? \xce\xed \xe4\xe0\xe2\xed\xee \xf1\xeb\xee\xec\xe0\xed?<h><n>", // NJH452
	"\xd7\xf2\xee \xe2\xfb \xe7\xed\xe0\xe5\xf2\xe5 \xee \xc4\xe6\xee\xed\xe0\xf2\xe0\xed\xe5 \xd2\xee\xec\xef\xf1\xee\xed\xe5?<h><n>", // NJH453
	// 15
	"\xd3 \xfd\xf2\xee\xe3\xee \xf2\xe5\xe0\xf2\xf0\xe0 \xf2\xe0\xea\xe0\xff \xe1\xee\xe3\xe0\xf2\xe0\xff \xe8\xf1\xf2\xee\xf0\xe8\xff. \xd1\xf2\xf0\xe0\xed\xed\xee, \xf7\xf2\xee \xe6\xe8\xf2\xe5\xeb\xe8 \xd1\xe5\xed\xf2-\xcb\xf3\xe8\xf1\xe0 \xed\xe5 \xe7\xe0\xe8\xed\xf2\xe5\xf0\xe5\xf1\xee\xe2\xe0\xed\xfb \xe2 \xe5\xe3\xee \xf1\xee\xf5\xf0\xe0\xed\xe5\xed\xe8\xe8.<h><n>", // NJH455
	"\xce\xf4\xe8\xf6\xe5\xf0 \xef\xee\xeb\xe8\xf6\xe8\xe8 \xf1\xea\xe0\xe7\xe0\xeb, \xf7\xf2\xee \xcd\xe8\xea\xee\xeb\xe0\xf1 \xd4\xe0\xeb\xfc\xea\xee\xed\xe5 \xec\xee\xe6\xe5\xf2 \xef\xee\xe9\xf2\xe8 \xed\xe0 \'\xea\xf0\xe0\xe9\xed\xe8\xe5 \xec\xe5\xf0\xfb\', \xf7\xf2\xee\xe1\xfb \xe4\xee\xe1\xe8\xf2\xfc\xf1\xff \xf1\xe2\xee\xe5\xe9 \xf6\xe5\xeb\xe8. \xcf\xee\xf7\xe5\xec\xf3 \xe2\xfb \xf0\xe0\xe7\xf0\xe5\xf8\xe8\xeb\xe8 \xe5\xec\xf3 \xf3\xf1\xf2\xf0\xee\xe8\xf2\xfc \xf1\xe2\xee\xe9 \xf8\xf2\xe0\xe1 \xe2 \xf5\xee\xeb\xeb\xe5?<h><n>", // NJH456
	"\xcd\xe5\xe4\xe0\xe2\xed\xee \xff \xe1\xfb\xeb\xe0 \xe7\xe0 \xea\xf3\xeb\xe8\xf1\xe0\xec\xe8, \xe8 \xee\xe4\xe8\xed \xe8\xe7 \xef\xf0\xee\xe6\xe5\xea\xf2\xee\xf0\xee\xe2 \xf1\xe2\xe0\xeb\xe8\xeb\xf1\xff \xf1 \xf0\xe0\xec\xef\xfb \xef\xf0\xff\xec\xee \xef\xe5\xf0\xe5\xe4\xee \xec\xed\xee\xe9!<h><n>", // NJH457
	"\xdf \xef\xee\xeb\xf3\xf7\xe8\xeb\xe0 \xe5\xf9\xe5 \xee\xe4\xed\xee \xf1\xee\xee\xe1\xf9\xe5\xed\xe8\xe5 \xee\xf2 \xef\xee\xf5\xe8\xf2\xe8\xf2\xe5\xeb\xff! \xc6\xf3\xf2\xea\xe8\xe9 \xe3\xee\xeb\xee\xf1 \xf1\xea\xe0\xe7\xe0\xeb \xe8\xe7 \xe3\xf0\xee\xec\xea\xee\xe3\xee\xe2\xee\xf0\xe8\xf2\xe5\xeb\xff, \xf7\xf2\xee \xed\xf3\xe6\xed\xee \xee\xf1\xf2\xe0\xed\xee\xe2\xe8\xf2\xfc \xf1\xed\xee\xf1 \xf2\xe5\xe0\xf2\xf0\xe0.<h><n>", // NJH459
	"\xc4\xe6\xee\xe7\xe5\xf4, \xff \xea\xee\xe5-\xf7\xf2\xee \xef\xf0\xee\xe2\xe5\xf0\xe8\xeb\xe0. \xcd\xe0 \xf1\xe0\xec\xee\xec \xe4\xe5\xeb\xe5 \xf3 \xe2\xe0\xf1 \xf3\xe6\xe5 \xed\xe5\xf2 \xe1\xf0\xe0\xf2\xe0 \xe2 \xc3\xf0\xe8\xe7\xe2\xf3\xe4\xe5.<h><n>", // NJH460
	// 20
	"\xd2\xe5\xe0\xf2\xf0 \xf1\xea\xee\xf0\xee \xe1\xf3\xe4\xe5\xf2 \xf0\xe0\xe7\xf0\xf3\xf8\xe5\xed. \xc2\xfb \xe2\xf1\xe5 \xe5\xf9\xe5 \xe4\xe5\xf0\xe6\xe8\xf2\xe5\xf1\xfc?<h><n>", // NJH465
	"\xd0\xe0\xf1\xf1\xea\xe0\xe6\xe8\xf2\xe5, \xef\xee\xe6\xe0\xeb\xf3\xe9\xf1\xf2\xe0, \xee \xf1\xe2\xee\xe8\xf5 \xee\xf2\xed\xee\xf8\xe5\xed\xe8\xff\xf5 \xf1 \xef\xee\xeb\xe8\xf6\xe8\xe5\xe9.<h><n>", // NNF650
	"\xc2\xfb \xec\xee\xe6\xe5\xf2\xe5 \xf0\xe0\xf1\xf1\xea\xe0\xe7\xe0\xf2\xfc \xee \xcc\xee\xeb\xee\xe4\xe5\xe6\xed\xee\xec \xee\xe1\xf9\xe5\xf1\xf2\xe2\xe5 \xf1\xef\xe0\xf1\xe5\xed\xe8\xff \xf2\xe5\xe0\xf2\xf0\xee\xe2?<h><n>", // NNF651
	"\xd1\xe8\xec\xee\xed\xe0 \xef\xee\xe7\xe2\xe0\xeb\xe0 \xe6\xf3\xf0\xed\xe0\xeb\xe8\xf1\xf2\xee\xe2, \xf7\xf2\xee\xe1\xfb \xf0\xe0\xf1\xf1\xea\xe0\xe7\xe0\xf2\xfc \xee \xf2\xe0\xea \xed\xe0\xe7\xfb\xe2\xe0\xe5\xec\xee\xec \xe3\xe5\xf0\xee\xe9\xf1\xf2\xe2\xe5 \xc1\xf0\xfd\xe4\xe8! \xce\xed\xe0 \xe4\xe0\xe6\xe5 \xed\xe5 \xe2\xf1\xef\xee\xec\xe8\xed\xe0\xe5\xf2 \xee \xcc\xe0\xe9\xe5!<h><n>", // NNF655
	"\xc2\xfb \xe7\xed\xe0\xe5\xf2\xe5, \xf7\xf2\xee \xf1\xee\xe1\xe8\xf0\xe0\xfe\xf2\xf1\xff \xef\xee\xf1\xf2\xf0\xee\xe8\xf2\xfc \xed\xe0 \xec\xe5\xf1\xf2\xe5 \xf2\xe5\xe0\xf2\xf0\xe0?<h><n>", // NNF658
	// 25
	"\xc4\xe6\xee\xe7\xe5\xf4 \xf1\xea\xe0\xe7\xe0\xeb, \xf7\xf2\xee \xe8\xf1\xf2\xee\xf0\xe8\xf7\xe5\xf1\xea\xee\xe5 \xee\xe1\xf9\xe5\xf1\xf2\xe2\xee \xd1\xe5\xed\xf2-\xcb\xf3\xe8\xf1\xe0 \xf5\xee\xf7\xe5\xf2 \xef\xf0\xe8\xe7\xed\xe0\xf2\xfc \xfd\xf2\xee\xf2 \xf2\xe5\xe0\xf2\xf0 \xed\xe0\xf6\xe8\xee\xed\xe0\xeb\xfc\xed\xfb\xec \xef\xe0\xec\xff\xf2\xed\xe8\xea\xee\xec.<h><n>", // NNF659
	"\xdf \xe2\xfb\xff\xf1\xed\xe8\xeb\xe0, \xf7\xf2\xee \xef\xee\xf1\xeb\xe5 \xf1\xed\xee\xf1\xe0 \xf2\xe5\xe0\xf2\xf0\xe0 \xc4\xe6\xee\xe7\xe5\xf4 \xf1\xee\xe1\xe8\xf0\xe0\xe5\xf2\xf1\xff \xef\xe5\xf0\xe5\xe5\xf5\xe0\xf2\xfc \xe2 \xc0\xf0\xe8\xe7\xee\xed\xf3. \xd2\xe0\xec \xee\xed\xe8 \xf1 \xe1\xf0\xe0\xf2\xee\xec \xf5\xee\xf2\xff\xf2 \xee\xf2\xea\xf0\xfb\xf2\xfc \xf1\xe2\xee\xe9 \xea\xe8\xed\xee\xf2\xe5\xe0\xf2\xf0.<h><n>", // NBG20
	"\xcf\xee \xf1\xee\xe2\xe5\xf2\xf3 \xc4\xe6\xee\xe7\xe5\xf4\xe0 \xff \xef\xee\xe7\xe2\xee\xed\xe8\xeb\xe0 \xe2 \xe3\xee\xf0\xee\xe4\xf1\xea\xf3\xfe \xe0\xe4\xec\xe8\xed\xe8\xf1\xf2\xf0\xe0\xf6\xe8\xfe \xe8 \xf1\xef\xf0\xee\xf1\xe8\xeb\xe0 \xee \xef\xeb\xe0\xed\xe5 \xf2\xe5\xe0\xf2\xf0\xe0.<h><n>", // NBG21
	"\xce\xf4\xe8\xf6\xe5\xf0 \xef\xee\xeb\xe8\xf6\xe8\xe8 \xf1\xea\xe0\xe7\xe0\xeb, \xf7\xf2\xee \xe8\xed\xee\xe3\xe4\xe0 \xcd\xe8\xea\xee\xeb\xe0\xf1 \xe4\xee\xe1\xe8\xe2\xe0\xe5\xf2\xf1\xff \xf1\xe2\xee\xe8\xf5 \xf6\xe5\xeb\xe5\xe9 \xed\xe5\xe7\xe0\xea\xee\xed\xed\xfb\xec \xef\xf3\xf2\xe5\xec. \xcf\xee\xeb\xe8\xf6\xe8\xff \xf1\xf7\xe8\xf2\xe0\xe5\xf2, \xf7\xf2\xee \xee\xed \xef\xf0\xe8\xf7\xe0\xf1\xf2\xe5\xed \xea \xef\xee\xf5\xe8\xf9\xe5\xed\xe8\xfe \xe4\xe5\xe2\xf3\xf8\xea\xe8 \xe8\xe7 \xf2\xe5\xe0\xf2\xf0\xe0 \xe2 \xcd\xfd\xf8\xe2\xe8\xeb\xeb\xe5.<h><n>", // NBG22
	"\xcc\xed\xe5 \xea\xe0\xe6\xe5\xf2\xf1\xff, \xf7\xf2\xee \xd1\xe8\xec\xee\xed\xe0 \xcc\xfe\xeb\xeb\xe5\xf0 \xe3\xee\xf2\xee\xe2\xe0 \xed\xe0 \xe2\xf1\xe5 \xf0\xe0\xe4\xe8 \xf1\xeb\xe0\xe2\xfb.<h><n>", // NBG23
	// 30
	"\xd1\xe8\xec\xee\xed\xe0 \xee\xf0\xe3\xe0\xed\xe8\xe7\xee\xe2\xe0\xeb\xe0 \xef\xf0\xe5\xf1\xf1-\xea\xee\xed\xf4\xe5\xf0\xe5\xed\xf6\xe8\xfe. \xdf \xee\xf7\xe5\xed\xfc \xf5\xee\xf7\xf3 \xf3\xe7\xed\xe0\xf2\xfc, \xee \xf7\xe5\xec \xf2\xe0\xec \xf0\xe0\xe7\xe3\xee\xe2\xe0\xf0\xe8\xe2\xe0\xfe\xf2, \xed\xee \xe2\xe5\xe4\xfc \xec\xe5\xed\xff \xed\xe5 \xef\xf0\xe8\xe3\xeb\xe0\xf8\xe0\xeb\xe8.<h><n>", // NBG24
	"\xd1\xe8\xec\xee\xed\xe0 \xf5\xee\xf7\xe5\xf2, \xf7\xf2\xee\xe1\xfb \xe4\xeb\xff \xf1\xe2\xee\xe8\xf5 \xe7\xf0\xe8\xf2\xe5\xeb\xe5\xe9 \xc1\xf0\xfd\xe4\xe8 \xe1\xfb\xeb \xe2\xf1\xe5\xe3\xe4\xe0 \'\xf3\xec\xee\xef\xee\xec\xf0\xe0\xf7\xe8\xf2\xe5\xeb\xfc\xed\xee \xef\xf0\xe8\xe2\xeb\xe5\xea\xe0\xf2\xe5\xeb\xfc\xed\xfb\xec\'. \xd7\xf2\xee \xfd\xf2\xee \xe7\xed\xe0\xf7\xe8\xf2?<h><n>", // NBG25
	"\xd1\xe5\xe3\xee\xe4\xed\xff \xf3\xf2\xf0\xee\xec \xea\xf2\xee-\xf2\xee \xef\xf0\xe8\xf1\xeb\xe0\xeb \xe2 \xf2\xe5\xe0\xf2\xf0 \xef\xee\xf5\xee\xf0\xee\xed\xed\xfb\xe9 \xe2\xe5\xed\xee\xea!<h><n>", // NBG26
	"\xdf \xef\xee\xe7\xed\xe0\xea\xee\xec\xe8\xeb\xe0\xf1\xfc \xf1 \xcd\xe8\xea\xee\xeb\xe0\xf1\xee\xec \xd4\xe0\xeb\xfc\xea\xee\xed\xe5, \xeb\xe8\xe4\xe5\xf0\xee\xec \xee\xf0\xe3\xe0\xed\xe8\xe7\xe0\xf6\xe8\xe8 \xcc\xce\xd1\xd2. \xc5\xe3\xee \xee\xf0\xe3\xe0\xed\xe8\xe7\xe0\xf6\xe8\xff \xf0\xe0\xe7\xe1\xe8\xeb\xe0 \xef\xe0\xeb\xe0\xf2\xea\xe8 \xe2\xee\xe7\xeb\xe5 \xf2\xe5\xe0\xf2\xf0\xe0, \xe2\xfb\xf0\xe0\xe6\xe0\xff \xf1\xe2\xee\xe9 \xef\xf0\xee\xf2\xe5\xf1\xf2 \xef\xf0\xee\xf2\xe8\xe2 \xf1\xed\xee\xf1\xe0.<h><n>", // NBG27
	"\xcd\xe8\xea\xee\xeb\xe0\xf1 \xf1\xea\xe0\xe7\xe0\xeb, \xf7\xf2\xee \xe5\xe3\xee \xe1\xe0\xe1\xf3\xf8\xea\xe0 \xe2\xfb\xef\xee\xeb\xed\xff\xeb\xe0 \xe2\xed\xf3\xf2\xf0\xe5\xed\xed\xfe\xfe \xee\xf2\xe4\xe5\xeb\xea\xf3 \xf2\xe5\xe0\xf2\xf0\xe0. \xcd\xee \xc4\xe6\xee\xed\xe0\xf2\xe0\xed \xd2\xee\xec\xef\xf1\xee\xed, \xef\xe5\xf0\xe2\xfb\xe9 \xe2\xeb\xe0\xe4\xe5\xeb\xe5\xf6 \'\xd0\xee\xe9\xff\xeb \xcf\xe0\xeb\xeb\xe0\xe4\xe8\xf3\xec\xe0\', \xf2\xe0\xea \xe8 \xed\xe5 \xe7\xe0\xef\xeb\xe0\xf2\xe8\xeb \xe5\xe9 \xe7\xe0 \xf0\xe0\xe1\xee\xf2\xf3!<h><n>", // NBG28
	// 35
	"\xdf \xed\xe0\xf8\xeb\xe0 \xea\xed\xe8\xe3\xf3, \xe2 \xea\xee\xf2\xee\xf0\xee\xe9 \xee\xef\xe8\xf1\xfb\xe2\xe0\xe5\xf2\xf1\xff \xe2\xfb\xef\xee\xeb\xed\xe5\xed\xe8\xe5 \xf0\xe0\xe7\xeb\xe8\xf7\xed\xfb\xf5 \xf4\xee\xea\xf3\xf1\xee\xe2.<h><n>", // NBG30
	"\xdf \xf0\xe0\xe7\xe3\xee\xe2\xe0\xf0\xe8\xe2\xe0\xeb\xe0 \xf1 \xd8\xe5\xf0\xec\xe0\xed\xee\xec \xd2\xf0\xe0\xf3\xf2\xee\xec \xe8\xe7 \xe1\xe8\xe1\xeb\xe8\xee\xf2\xe5\xea\xe8 \xea\xee\xed\xe3\xf0\xe5\xf1\xf1\xe0. \xce\xed \xef\xee\xee\xe1\xe5\xf9\xe0\xeb \xed\xe0\xe9\xf2\xe8 \xef\xe8\xf1\xfc\xec\xe0 \xee \xf2\xee\xec, \xea\xe0\xea \xc3\xf3\xe4\xe8\xed\xe8 \xf0\xe0\xf1\xef\xee\xf0\xff\xe4\xe8\xeb\xf1\xff \xf1\xe2\xee\xe5\xe9 \xef\xee\xeb\xee\xe2\xe8\xed\xee\xe9 \xf2\xe5\xe0\xf2\xf0\xe0.<h><n>", // NBG31
	"\xdf \xef\xee\xeb\xf3\xf7\xe8\xeb\xe0 \xe5\xf9\xe5 \xee\xe4\xed\xf3 \xf3\xe3\xf0\xee\xe7\xf3! \xcd\xe0 \xfd\xf2\xee\xf2 \xf0\xe0\xe7 \xef\xee\xf5\xe8\xf2\xe8\xf2\xe5\xeb\xfc \xee\xe1\xf0\xe0\xf2\xe8\xeb\xf1\xff \xea\xee \xec\xed\xe5 \xef\xee \xe8\xec\xe5\xed\xe8!<h><n>", // NBG32
	"\xcf\xee\xec\xee\xe3\xe8\xf2\xe5! \xdf \xed\xe5 \xe7\xed\xe0\xfe, \xf7\xf2\xee \xe4\xe5\xeb\xe0\xf2\xfc \xe4\xe0\xeb\xfc\xf8\xe5!<h><n>", // NBG70
	"\xc4\xe5\xe2\xf7\xee\xed\xea\xe8, \xec\xed\xe5 \xed\xf3\xe6\xed\xe0 \xef\xee\xe4\xf1\xea\xe0\xe7\xea\xe0!<h><n>", // NBG71
	// 40
	"\xca \xf2\xee\xec\xf3 \xe2\xf0\xe5\xec\xe5\xed\xe8, \xea\xe0\xea \xec\xed\xe5 \xf3\xe4\xe0\xeb\xee\xf1\xfc \xef\xee\xe3\xee\xe2\xee\xf0\xe8\xf2\xfc \xf1 \xd1\xe8\xec\xee\xed\xee\xe9 \xcc\xfe\xeb\xeb\xe5\xf0, \xee\xed\xe0 \xf3\xe6\xe5 \xe7\xed\xe0\xeb\xe0 \xee \xef\xee\xf5\xe8\xf9\xe5\xed\xe8\xe8 \xe8 \xee\xf2\xec\xe5\xed\xe8\xeb\xe0 \xef\xf0\xe5\xec\xfc\xe5\xf0\xf3.<h><n>", // NDN20
	"\xc4\xe6\xee\xe7\xe5\xf4 \xe3\xee\xe2\xee\xf0\xe8\xf2, \xf7\xf2\xee \xe8\xf1\xf2\xee\xf0\xe8\xf7\xe5\xf1\xea\xee\xe5 \xee\xe1\xf9\xe5\xf1\xf2\xe2\xee \xd1\xe5\xed\xf2-\xcb\xf3\xe8\xf1\xe0 \xe4\xee\xe1\xe8\xe2\xe0\xe5\xf2\xf1\xff \xef\xf0\xe8\xe7\xed\xe0\xed\xe8\xff \xf2\xe5\xe0\xf2\xf0\xe0 \xed\xe0\xf6\xe8\xee\xed\xe0\xeb\xfc\xed\xfb\xec \xef\xe0\xec\xff\xf2\xed\xe8\xea\xee\xec.<h><n>", // NDN21
	"\xdf \xf1\xeb\xf3\xf7\xe0\xe9\xed\xee \xf3\xf1\xeb\xfb\xf8\xe0\xeb\xe0 \xf2\xe5\xeb\xe5\xf4\xee\xed\xed\xfb\xe9 \xf0\xe0\xe7\xe3\xee\xe2\xee\xf0 \xc1\xf0\xfd\xe4\xe8 \xe8 \xd1\xe8\xec\xee\xed\xfb. \xce\xf7\xe5\xed\xfc \xf1\xf2\xf0\xe0\xed\xed\xee. \xcf\xee\xf5\xee\xe6\xe5, \xf3 \xc1\xf0\xfd\xe4\xe8 \xe4\xe5\xef\xf0\xe5\xf1\xf1\xe8\xff.<h><n>", // NDN22
	"\xdf \xed\xe0\xf8\xeb\xe0 \xf2\xe5\xf5\xed\xe8\xf7\xe5\xf1\xea\xe8\xe9 \xf1\xef\xf0\xe0\xe2\xee\xf7\xed\xe8\xea. \xc2 \xed\xe5\xec \xee\xef\xe8\xf1\xfb\xe2\xe0\xfe\xf2\xf1\xff \xed\xe5\xea\xee\xf2\xee\xf0\xfb\xe5 \xf2\xf0\xfe\xea\xe8 \xe8\xeb\xeb\xfe\xe7\xe8\xee\xed\xe8\xf1\xf2\xee\xe2, \xe2\xfb\xf1\xf2\xf3\xef\xe0\xe2\xf8\xe8\xf5 \xed\xe0 \xf1\xf6\xe5\xed\xe5 \'\xd0\xee\xe9\xff\xeb \xcf\xe0\xeb\xeb\xe0\xe4\xe8\xf3\xec\xe0\'.<h><n>", // NDN23
	"\xdf \xe2\xfb\xe8\xe3\xf0\xe0\xeb\xe0 \xe2 \xe8\xe3\xf0\xe5 \'\xd3\xe3\xe0\xe4\xe0\xe9 \xea\xe0\xf0\xf2\xf3\'. \xc8 \xff \xed\xe5\xef\xf0\xe5\xec\xe5\xed\xed\xee \xed\xe0\xe9\xe4\xf3 \xef\xee\xf5\xe8\xf2\xe8\xf2\xe5\xeb\xff.<h><n>", // NDN24
	// 45
	"\xdf \xed\xe5 \xec\xee\xe3\xf3 \xef\xee\xed\xff\xf2\xfc, \xef\xee\xf7\xe5\xec\xf3 \xef\xee\xf5\xe8\xf2\xe8\xf2\xe5\xeb\xfc \xe7\xe2\xee\xed\xe8\xeb \xe8\xe7 \xc3\xf0\xe0\xed\xe8\xf2-\xf1\xe8\xf2\xe8 \xe8 \xef\xee\xf7\xe5\xec\xf3 \xee\xed \xef\xee\xf2\xf0\xe5\xe1\xee\xe2\xe0\xeb \xe2\xfb\xea\xf3\xef, \xe4\xe0\xe6\xe5 \xed\xe5 \xf3\xef\xee\xec\xff\xed\xf3\xe2 \xee \xf1\xed\xee\xf1\xe5 \xf2\xe5\xe0\xf2\xf0\xe0!<h><n>", // NDN25
	"\xdf \xf1\xef\xf0\xee\xf1\xe8\xeb\xe0 \xd1\xe8\xec\xee\xed\xf3 \xee \xea\xe2\xe8\xf2\xe0\xed\xf6\xe8\xe8 \xed\xe0 \xef\xee\xf5\xee\xf0\xee\xed\xed\xfb\xe9 \xe2\xe5\xed\xee\xea. \xce\xea\xe0\xe7\xe0\xeb\xee\xf1\xfc, \xf7\xf2\xee \xee\xed\xe0 \xe7\xe0\xea\xe0\xe7\xe0\xeb\xe0 \xe5\xe3\xee \xe4\xeb\xff \xee\xf7\xe5\xf0\xe5\xe4\xed\xee\xe9 \xf4\xee\xf2\xee\xf1\xe5\xf1\xf1\xe8\xe8. \xcf\xf0\xe5\xe4\xf1\xf2\xe0\xe2\xeb\xff\xe5\xf2\xe5?<h><n>", // NDN26
	"\xcc\xed\xe5 \xed\xe0\xe4\xee \xef\xf0\xe8\xe4\xf3\xec\xe0\xf2\xfc, \xea\xe0\xea \xee\xf1\xf2\xe0\xed\xee\xe2\xe8\xf2\xfc \xf1\xed\xee\xf1 \xf2\xe5\xe0\xf2\xf0\xe0!<h><n>", // NDN27
	"\xc2 \xf5\xee\xeb\xeb\xe5 \xe5\xf1\xf2\xfc \xe0\xe2\xf2\xee\xec\xe0\xf2 \xe4\xeb\xff \xe8\xe7\xe3\xee\xf2\xee\xe2\xeb\xe5\xed\xe8\xff \xea\xeb\xfe\xf7\xe5\xe9. \xc2 \xed\xe5\xec \xec\xee\xe6\xed\xee \xf1\xe4\xe5\xeb\xe0\xf2\xfc \xea\xeb\xfe\xf7 \xeb\xfe\xe1\xee\xe9 \xf4\xee\xf0\xec\xfb. \xc8 \xe4\xe0\xe6\xe5 \xea\xeb\xfe\xf7 \xea \xf1\xe5\xf0\xe4\xf6\xf3. \xdf \xf5\xee\xf2\xe5\xeb\xe0 \xf1\xe4\xe5\xeb\xe0\xf2\xfc \xf2\xe0\xea\xee\xe9 \xe4\xeb\xff \xf2\xe5\xe1\xff, \xed\xee \xe2\xe5\xe4\xfc \xf1\xe5\xe9\xf7\xe0\xf1 \xed\xe5 \xe2\xf0\xe5\xec\xff \xe4\xf3\xec\xe0\xf2\xfc \xee \xf1\xf3\xe2\xe5\xed\xe8\xf0\xe0\xf5.<h><n>", // NDN28
	"\xca\xe0\xea \xff \xf3\xe7\xed\xe0\xeb\xe0, \xc3\xe0\xf0\xf0\xe8 \xc3\xf3\xe4\xe8\xed\xe8 \xe2\xeb\xe0\xe4\xe5\xeb \xef\xee\xeb\xee\xe2\xe8\xed\xee\xe9 \xf2\xe5\xe0\xf2\xf0\xe0 \'\xd0\xee\xe9\xff\xeb \xcf\xe0\xeb\xeb\xe0\xe4\xe8\xf3\xec\'. \xca\xe0\xe6\xe5\xf2\xf1\xff, \xee\xed \xe1\xfb\xeb \xe1\xee\xeb\xfc\xf8\xe8\xec \xee\xf0\xe8\xe3\xe8\xed\xe0\xeb\xee\xec!<h><n>", // NDN29
	// 50
	"\xcd\xe8\xea\xee\xeb\xe0\xf1 \xe3\xee\xe2\xee\xf0\xe8\xf2, \xf7\xf2\xee \xef\xee\xeb\xe8\xf6\xe8\xff \xf3\xe6\xe5 \xee\xe1\xfb\xf1\xea\xe8\xe2\xe0\xe5\xf2 \xe7\xe4\xe0\xed\xe8\xe5 \xef\xe5\xf0\xe5\xe4 \xf1\xed\xee\xf1\xee\xec.<h><n>", // NDN30
	"\xcd\xe5\xe4\xe0\xe2\xed\xee \xff \xe1\xfb\xeb\xe0 \xe7\xe0 \xea\xf3\xeb\xe8\xf1\xe0\xec\xe8, \xe8 \xee\xe4\xe8\xed \xe8\xe7 \xef\xf0\xee\xe6\xe5\xea\xf2\xee\xf0\xee\xe2 \xf1\xe2\xe0\xeb\xe8\xeb\xf1\xff \xf1 \xf0\xe0\xec\xef\xfb \xef\xf0\xff\xec\xee \xef\xe5\xf0\xe5\xe4\xee \xec\xed\xee\xe9!<h><n>", // NDNj
	"\xdf \xed\xe5 \xee\xf2\xea\xe0\xe6\xf3\xf1\xfc \xee\xf2 \xef\xee\xe4\xf1\xea\xe0\xe7\xea\xe8.<h><n>", // NDN70
	"\xdf \xed\xe5 \xe7\xed\xe0\xfe, \xf7\xf2\xee \xe4\xe5\xeb\xe0\xf2\xfc \xe4\xe0\xeb\xfc\xf8\xe5. \xc5\xf1\xf2\xfc \xe8\xe4\xe5\xe8?<h><n>" // NDN71
}
};

const Common::Array<Common::Array<const char *>> _nancy5GoodbyeTexts = {
{	// English
	"I'll talk to you later, Brady.<h>", // NBA090
	"I think your phone's about to ring.<h>", // NSM290
	"Gotta go now!<h>", // NJH90
	"Catch ya later.<h>", // NNF690
	"OK, you two. I'll talk to you soon!<h>", // NBG90
	"Bye, Ned.<h>" // NDN90
},
{	// Russian
	"\xcf\xee\xe3\xee\xe2\xee\xf0\xe8\xec \xef\xee\xe7\xe6\xe5, \xc1\xf0\xfd\xe4\xe8.<h>", // NBA090
	"\xca\xe0\xe6\xe5\xf2\xf1\xff, \xf3 \xe2\xe0\xf1 \xec\xed\xee\xe3\xee \xe4\xe5\xeb. \xcd\xe5 \xe1\xf3\xe4\xf3 \xec\xe5\xf8\xe0\xf2\xfc.<h>", // NSM290
	"\xcc\xed\xe5 \xef\xee\xf0\xe0!<h>", // NJH90
	"\xcf\xee\xea\xe0.<h>", // NNF690
	"\xcc\xed\xe5 \xef\xee\xf0\xe0. \xdf \xe2\xe0\xec \xef\xe5\xf0\xe5\xe7\xe2\xee\xed\xfe!<h>", // NBG90
	"\xcf\xee\xea\xe0, \xcd\xfd\xe4.<h>", // NDN90
}
};

const Common::Array<const char *> _nancy5TelephoneRinging = {
	"ringing...<n><e>", // English
	"\xc3\xf3\xe4\xea\xe8...  <n><e>" // Russian
};

const Common::Array<const char *> _nancy5EmptySaveStrings = {
	"Nothing Saved Here",	// English
	"\xc3\xf3\xf1\xf2\xee"	// Russian
};

const Common::Array<const char *> _nancy5EventFlagNames = {
	"EV_Generic0",
	"EV_Generic1",
	"EV_Generic2",
	"EV_Generic3",
	"EV_Generic4",
	"EV_Generic5",
	"EV_Generic6",
	"EV_Generic7",
	"EV_Generic8",
	"EV_Generic9",
	"EV_Generic10",
	"EV_Generic11",
	"EV_Generic12",
	"EV_Generic13",
	"EV_Generic14",
	"EV_Generic15",
	"EV_Generic16",
	"EV_Generic17",
	"EV_Generic18",
	"EV_Generic19",
	"EV_Generic20",
	"EV_Generic21",
	"EV_Generic22",
	"EV_Generic23",
	"EV_Generic24",
	"EV_Generic25",
	"EV_Generic26",
	"EV_Generic27",
	"EV_Generic28",
	"EV_Generic29",
	"EV_Generic30",
	"EV_TimeForEndgame",
	"EV_PlayerWonGame",
	"EV_StopPlayerScrolling",
	"EV_Easter_Eggs",
	"EV_BA_Available",
	"EV_BA_Bust",
	"EV_BA_Said_Alibi",
	"EV_BA_Said_Balcony",
	"EV_BA_Said_Complicated",
	"EV_BA_Said_Email",
	"EV_BA_Said_Falcone",
	"EV_BA_Said_Guest",
	"EV_BA_Said_Help",
	"EV_BA_Said_Joseph",
	"EV_BA_Said_Kidnap_Me",
	"EV_BA_Said_My_Agent",
	"EV_BA_Said_Politician",
	"EV_BA_Said_Radicals",
	"EV_BA_Said_RunawayBus",
	"EV_BA_Said_Tinseltown",
	"EV_Bulb_Popped",
	"EV_Bulb_Replaced",
	"EV_CageLock_On",
	"EV_Called_CntyAdmin",
	"EV_Called_LibCong",
	"EV_Called_Widow",
	"EV_Day1",
	"EV_Day2",
	"EV_Day3",
	"EV_FaderLft_On",
	"EV_FaderLft01",
	"EV_FaderLft02",
	"EV_FaderLft03",
	"EV_FaderLft04",
	"EV_FaderLft05",
	"EV_FaderRt_On",
	"EV_FaderRt01",
	"EV_FaderRt02",
	"EV_FaderRt03",
	"EV_FaderRt04",
	"EV_FaderRt05",
	"EV_Flashed_Joseph",
	"EV_Focused_Slide",
	"EV_Found_Secret_Passage",
	"EV_Got_CntyAdmin_Number",
	"EV_Greased_TileMove",
	"EV_Heard_JH_Call01",
	"EV_Heard_MicTest",
	"EV_Heard_Scream01",
	"EV_Heard_Threat01",
	"EV_Heard_Threat02",
	"EV_Hooked_Crate",
	"EV_In_Basement",
	"EV_In_Closet",
	"EV_In_SwitcherooBox",
	"EV_Insert_Blueprint",
	"EV_Insert_Houdini",
	"EV_Insert_Intermission",
	"EV_Iron_Security_Removed",
	"EV_JH_Available",
	"EV_JH_Bust_Depo",
	"EV_JH_Bust_EOD1",
	"EV_JH_Fixed_Keymaker",
	"EV_JH_Page_EOD1",
	"EV_JH_Said_Alibi",
	"EV_JH_Said_AllOver",
	"EV_JH_Said_Blaze",
	"EV_JH_Said_Blueprints",
	"EV_JH_Said_CallPolice_EOD2",
	"EV_JH_Said_CantLetYou",
	"EV_JH_Said_CrowdControl",
	"EV_JH_Said_Fix_Keymaker",
	"EV_JH_Said_GoHome02",
	"EV_JH_Said_GraniteCity",
	"EV_JH_Said_Greasewood",
	"EV_JH_Said_Inside_Line",
	"EV_JH_Said_JJ",
	"EV_JH_Said_Landmark",
	"EV_JH_Said_LastCase",
	"EV_JH_Said_NeverMeant",
	"EV_JH_Said_NF_Kid",
	"EV_JH_Said_Not_Desperate",
	"EV_JH_Said_NotSoFast",
	"EV_JH_Said_Ransom",
	"EV_JH_Said_Secret_Passage",
	"EV_JH_Said_Simone",
	"EV_JH_Said_Souvenir",
	"EV_JH_Said_StageLight",
	"EV_JH_Said_Suspects",
	"EV_JH_Said_Wheelerdealer",
	"EV_JH_Said_Wreath",
	"EV_JH_Showed_PressPass",
	"EV_JH_Suggested_Police",
	"EV_Knob_Lost",
	"EV_Knob_Replaced",
	"EV_Lowered_CageLft",
	"EV_Lowered_CageRt",
	"EV_Magnet_On",
	"EV_Met_BA01",
	"EV_Met_BA02",
	"EV_Met_BA03",
	"EV_Met_JH01",
	"EV_Met_JH02",
	"EV_Met_JH03",
	"EV_Met_NF01",
	"EV_Met_NF02",
	"EV_Met_NF03",
	"EV_Met_SM01",
	"EV_Met_SM02",
	"EV_Met_SM03",
	"EV_Mic_On",
	"EV_ND_Said_Article",
	"EV_ND_Said_Fingers",
	"EV_NF_Available",
	"EV_NF_Gave_Number",
	"EV_NF_Said_411",
	"EV_NF_Said_Alibi",
	"EV_NF_Said_Boloney",
	"EV_NF_Said_Booth",
	"EV_NF_Said_Cowboy",
	"EV_NF_Said_Generica",
	"EV_NF_Said_Grandmother",
	"EV_NF_Said_HocusPocus",
	"EV_NF_Said_Joseph",
	"EV_NF_Said_Megaplex",
	"EV_NF_Said_Most_Wanted",
	"EV_NF_Said_No_EcoTerror",
	"EV_NF_Said_Package",
	"EV_NF_Said_Police",
	"EV_NF_Said_Wrecking_Crew",
	"EV_Placed_Coin_Balcony",
	"EV_Placed_Coin_Souvenir",
	"EV_Police_Bust",
	"EV_PoliceCall_Intro",
	"EV_PoliceCall_PizzaBox",
	"EV_PoliceCall_PressPass",
	"EV_PoliceCall_Ransom",
	"EV_Press_Conf_Over",
	"EV_Press_Conf_Started",
	"EV_Projector_On",
	"EV_Reported_JJ_Docs",
	"EV_Ripped_ManEnvelope",
	"EV_Rope_Fasten01",
	"EV_Rope_Fasten02",
	"EV_Rope_Fasten03",
	"EV_Rope_Fasten04",
	"EV_Rope_Fasten05",
	"EV_Rope_Untied01",
	"EV_Rope_Untied02",
	"EV_Rope_Untied03",
	"EV_Rope_Untied04",
	"EV_Rope_Untied05",
	"EV_Saw_BA_Project",
	"EV_Saw_CageTrick_Book",
	"EV_Saw_CageTrick_Review",
	"EV_Saw_DigiCam",
	"EV_Saw_Greasewood_Evidence",
	"EV_Saw_Houdini_ChllngePost",
	"EV_Saw_JJ_Docs",
	"EV_Saw_LouisaDoc",
	"EV_Saw_ManEnvelope",
	"EV_Saw_Manual_Bulb",
	"EV_Saw_Manual_Focus",
	"EV_Saw_Maya_Depo",
	"EV_Saw_Maya_Email",
	"EV_Saw_Maya_Magician",
	"EV_Saw_Maya_Notebook",
	"EV_Saw_Maya_Pizza",
	"EV_Saw_Maya_Shoe",
	"EV_Saw_Missing_Poster",
	"EV_Saw_PencilTrick_Book",
	"EV_Saw_Ransom",
	"EV_Saw_Slide_Blueprint",
	"EV_Saw_VoiceDistort",
	"EV_Saw_Wreath",
	"EV_Saw_Wreath_Note",
	"EV_Saw_Wreath_Receipt",
	"EV_ShowDown",
	"EV_SM_Available",
	"EV_SM_Dis",
	"EV_SM_Fessed_Wreath",
	"EV_SM_Said_Alibi",
	"EV_SM_Said_Coneydog",
	"EV_SM_Said_FancyJackson",
	"EV_SM_Said_Fifty_Grand",
	"EV_SM_Said_Ponytail",
	"EV_SM_Said_Posters",
	"EV_SM_Said_PotatoTruck",
	"EV_SM_Said_Press_Conference",
	"EV_SM_Said_SamQuick",
	"EV_SM_Said_StageName",
	"EV_SM_Said_Troll",
	"EV_SM_Said_What_Project",
	"EV_Solved_TileMove",
	"EV_Solved_Cntrl_Keypad",
	"EV_Solved_Coin_Souvenir",
	"EV_Solved_DepoDoor",
	"EV_Solved_JJBox_Coin",
	"EV_Solved_JJBox_Lid",
	"EV_Solved_JJPanel_Puzzle",
	"EV_Solved_Keymaker",
	"EV_Solved_Monty",
	"EV_Solved_Palm_Pilot",
	"EV_Solved_PencilTrick",
	"EV_Solved_Ropes",
	"EV_Solved_Safe",
	"EV_Solved_SparkGate",
	"EV_Stagelight_Fell",
	"EV_Stairs_Down",
	"EV_Through_CeilingHatch",
	"EV_Time_For_Endgame",
	"EV_Took_Slide_Houdini",
	"EV_Trap_Lft_Opn",
	"EV_Trap_Rt_Opn",
	"EV_TrickSeat_Unstuck",
	"EV_Tried_TileMove",
	"EV_Tried_Cntrl_Keypad",
	"EV_Tried_Coin_Souvenir",
	"EV_Tried_DepoDoor",
	"EV_Tried_JJBox_Coin",
	"EV_Tried_JJBox_Lid",
	"EV_Tried_JJPanel_Puzzle",
	"EV_Tried_Keymaker",
	"EV_Tried_Monty",
	"EV_Tried_Palm_Pilot",
	"EV_Tried_PencilTrick",
	"EV_Tried_Ropes",
	"EV_Tried_Safe",
	"EV_Tried_SparkGate",
	"EV_BA_Said_Dazed",
	"EV_BA_Said_Outlaw",
	"EV_BA_Said_Verify",
	"EV_ND_Said_Bobby_Comment",
	"EV_ND_Said_Electrified",
	"EV_ND_Said_Hack_Useless",
	"EV_ND_Said_Hercules",
	"EV_ND_Said_Stick",
	"EV_NF_Said_GO",
	"EV_Passage_Opn",
	"EV_Saw_Keymaker",
	"EV_Saw_TrapDr_Anim",
	"EV_Thru_Mag_Exit",
	"EV_Took_Wand",
	"EV_Tried_Coin_Balcony",
	"EV_Tried_Drawer",
	"EV_Tried_Stuck_Seat",
	"EV_Tried_TileMove_Stuck",
	"EV_Unlocked_Drawer",
	"EV_Saw_Intermission",
	"EV_Tried_CageTrick",
	"EV_Solved_CageTrick",
	"EV_ND_Said_Testing",
	"EV_ND_Said_Attention",
	"EV_ND_Said_Outwitted",
	"EV_NF_Said_Spill",
	"EV_Empty1286",
	"EV_Empty1287",
	"EV_Empty1288",
	"EV_Empty1289",
	"EV_Empty1290",
	"EV_Empty1291",
	"EV_Empty1292",
	"EV_Empty1293",
	"EV_Empty1294",
	"EV_Empty1295",
	"EV_Empty1296",
	"EV_Empty1297",
	"EV_Empty1298",
	"EV_Empty1299",
	"EV_Empty1300",
	"EV_Empty1301",
	"EV_Empty1302",
	"EV_Empty1303",
	"EV_Empty1304",
	"EV_Empty1305",
	"EV_Empty1306",
	"EV_Empty1307",
	"EV_Empty1308",
	"EV_Empty1309",
	"EV_Empty1310",
	"EV_Empty1311",
	"EV_Empty1312",
	"EV_Empty1313",
	"EV_Empty1314",
	"EV_Empty1315",
	"EV_Empty1316",
	"EV_Empty1317",
	"EV_Empty1318",
	"EV_Empty1319",
	"EV_StartEndGameTimer",
	"EV_BA_CrackUp01",
	"EV_BA_CrackUp02",
	"EV_BA_CrackUp03",
	"EV_BA_CrackUp04",
	"EV_Called_CntyAdmin01",
	"EV_Called_CntyAdmin02",
	"EV_Called_CntyAdmin03",
	"EV_Called_CntyAdmin04",
	"EV_Called_CntyAdmin05",
	"EV_Called_LibCong01",
	"EV_Called_LibCong02",
	"EV_Called_LibCong03",
	"EV_Called_LibCong04",
	"EV_Called_LibCong05",
	"EV_Called_LibCong06",
	"EV_Called_Widow11",
	"EV_Called_Widow12",
	"EV_Called_Widow13",
	"EV_Called_Widow14",
	"EV_Called_Widow15",
	"EV_JH_Said_Arcade57",
	"EV_JH_Said_Arcade58",
	"EV_JH_Said_Arcade59",
	"EV_JH_Said_Houdini54",
	"EV_JH_Said_Houdini55",
	"EV_JH_Said_Houdini56",
	"EV_NF_Said_Asbestos",
	"EV_NF_Said_Mustache",
	"EV_JH_Said_Rendezvous52",
	"EV_JH_Said_Rendezvous53",
	"EV_SM_Said_Hydrate20",
	"EV_SM_Said_Hydrate21",
	"EV_SM_Said_Hydrate22",
	"EV_SM_Said_Sniveller",
	"EV_Empty1355",
	"EV_Empty1356",
	"EV_Empty1357",
	"EV_Empty1358",
	"EV_Empty1359",
	"EV_Empty1360",
	"EV_Empty1361",
	"EV_Empty1362",
	"EV_Empty1363",
	"EV_Empty1364",
	"EV_Empty1365",
	"EV_Empty1366",
	"EV_Empty1367",
	"EV_Empty1368",
	"EV_Empty1369",
	"EV_Empty1370",
	"EV_Empty1371",
	"EV_Empty1372",
	"EV_Empty1373",
	"EV_Empty1374",
	"EV_Empty1375",
	"EV_Empty1376",
	"EV_Empty1377",
	"EV_Empty1378",
	"EV_Empty1379",
	"EV_Empty1380",
	"EV_Empty1381",
	"EV_Empty1382",
	"EV_B_Said_Blueberry",
	"EV_B_Said_Decipher",
	"EV_B_Said_Disco",
	"EV_B_Said_Fresh",
	"EV_B_Said_Impressionable",
	"EV_B_Said_NoBrainer",
	"EV_B_Said_Silver_Platter",
	"EV_B_Said_Snoop",
	"EV_B_Said_Standard",
	"EV_B_Said_WideOpen",
	"EV_G_Said_ApplePie",
	"EV_G_Said_Broadcast",
	"EV_G_Said_Generic",
	"EV_G_Said_Hi",
	"EV_G_Said_JJ",
	"EV_G_Said_Limelight",
	"EV_G_Said_Sword",
	"EV_Heard_Press_Conf",
	"EV_Met_BG",
	"EV_Met_NN",
	"EV_NN_Said_Accident",
	"EV_NN_Said_Anchovies",
	"EV_NN_Said_Bandwagon",
	"EV_NN_Said_Cynicism",
	"EV_NN_Said_Drawer",
	"EV_NN_Said_Flash",
	"EV_NN_Said_Instincts",
	"EV_NN_Said_Jump",
	"EV_NN_Said_KeyChain",
	"EV_NN_Said_Lapse",
	"EV_NN_Said_Overboard",
	"EV_NN_Said_Postpone",
	"EV_NN_Said_Profit",
	"EV_NN_Said_Projection",
	"EV_NN_Said_Snoop",
	"EV_NN_Said_Sparks",
	"EV_NN_Said_Twisted",
	"EV_Empty1420",
	"EV_Empty1421",
	"EV_Empty1422",
	"EV_Empty1423",
	"EV_Empty1424",
	"EV_Empty1425",
	"EV_Empty1426",
	"EV_Empty1427",
	"EV_Empty1428",
	"EV_Empty1429",
	"EV_Empty1430",
	"EV_Empty1431",
	"EV_Empty1432",
	"EV_Empty1433",
	"EV_Empty1434",
	"EV_Empty1435",
	"EV_Empty1436",
	"EV_Empty1437",
	"EV_Empty1438",
	"EV_Empty1439",
	"EV_Empty1440",
	"EV_Empty1441",
	"EV_Empty1442",
	"EV_Empty1443",
	"EV_Empty1444",
	"EV_Empty1445",
	"EV_Empty1446",
	"EV_Empty1447",
	"EV_Empty1448",
	"EV_Empty1449",
	"EV_Empty1450",
	"EV_Empty1451",
	"EV_Empty1452",
	"EV_Empty1453",
	"EV_Empty1454",
	"EV_TBD"
};

// Patch notes:
// Both of the softlock patch files are described on HeR Interactive's website,
// but not actually present in the file they distribute. Thus, the fixes are ours.
// - S3503.cif blocks the transition between day 2 and 3 until the player obtains both the flash paper
// 	 and the safe key. This is not the most graceful way to handle the endgame softlocks, as both items
//   would technically be obtainable on day 3 as well; however, it is the least intrusive one. Moreover,
//   it seems to be what the original devs were planning to do; there is a check for the flash paper in
//   the exact scene that we patch. However, failing that check simply locks you in the room forever,
//   because they seem to have forgotten to finish the softlock fix.
// - S3218 removes the check for close captioning which would result in the player being unable to
//   close Simone's purse without having subtitles on. Technically not a softlock but just a plain bug,
//   so it is not hidden behind a setting, and is always enabled.

const Common::Array<const char *> nancy5PatchSrcFiles {
	"S3503.cif",
	"S3218.cif"
};

const Common::Array<PatchAssociation> nancy5PatchAssociations {
	{ { "softlocks_fix", "true" }, { "S3503" } }
};

#endif // NANCY5DATA_H
