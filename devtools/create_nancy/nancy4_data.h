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

#ifndef NANCY4DATA_H
#define NANCY4DATA_H

#include "types.h"

const GameConstants _nancy4Constants ={
	28,												// numItems
	504,											// numEventFlags
	{	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,			// genericEventFlags
		11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
		21, 22, 23, 24, 25, 26, 27, 28, 29, 30 },
	12,												// numCursorTypes
	4000,											// logoEndAfter
	32												// wonGameFlagID
};

const Common::Array<GameLanguage> _nancy4LanguagesOrder = {
	GameLanguage::kEnglish,
	GameLanguage::kRussian
};

const Common::Array<Common::Array<ConditionalDialogue>> _nancy4ConditionalDialogue = {
{	// Mr. Egan, 17 responses
	{	0, 1050, "NDE50",
		{ { kEv, 172, true }, { kEv, 249, false } } },
	{	1, 1051, "NDE51",
		{ { kEv, 249, true }, { kEv, 218, false }, { kEv, 246, false } } },
	{	2, 1052, "NDE52",
		{ { kEv, 249, true }, { kEv, 70, false }, { kEv, 218, true } } },
	{	3, 1053, "NDE53",
		{ { kEv, 249, true }, { kEv, 218, true }, { kEv, 116, true }, { kEv, 82, false } } },
	{	4, 1054, "NDE54",
		{ { kEv, 222, true }, { kEv, 77, false } } },
	{	5, 1055, "NDE55",
		{ { kEv, 223, true }, { kEv, 69, false } } },
	{	6, 1057, "NDE57",
		{ { kEv, 290, true }, { kEv, 85, false } } },
	{	7, 1058, "NDE58",
		{ { kEv, 262, true }, { kEv, 72, false } } },
	{	8, 1059, "NDE59",
		{ { kEv, 262, true }, { kEv, 67, false } } },
	{	9, 1061, "NDE61",
		{ { kEv, 275, true }, { kEv, 72, true }, { kEv, 81, true }, { kEv, 69, true }, { kEv, 61, false } } },
	{	10, 1062, "NDE62",
		{ { kEv, 262, true }, { kEv, 60, false }, { kEv, 69, true } } },
	{	11, 1065, "NDE41b",
		{ { kEv, 262, true }, { kEv, 83, true }, { kEv, 60, true }, { kEv, 89, false } } },
	{	12, 1067, "NDE67",
		{ { kEv, 331, true }, { kEv, 278, false }, { kEv, 63, false }, { kEv, 83, true } } },
	{	13, 1068, "NDE68",
		{ { kEv, 335, true }, { kEv, 286, false }, { kEv, 84, false }, { kEv, 83, true } } },
	{	14, 1069, "NDE69",
		{ { kEv, 332, true }, { kEv, 280, false }, { kEv, 65, false }, { kEv, 83, true } } },
	{	15, 1071, "NDE71",
		{ { kEv, 223, false }, { kEv, 172, true }, { kEv, 252, false } } },
	{	16, 1074, "NDE74",
		{ { kEv, 75, true }, { kEv, 66, false } } },
	{	17, 1078, "NDE78",
		{ { kEv, 258, true }, { kEv, 279, false }, { kEv, 64, false }, { kEv, 83, true } } }
},
{	// Lisa, 7 responses
	{	18, 2050, "NLO50",
		{ { kEv, 173, true }, { kEv, 163, false } } },
	{	19, 2051, "NLO51",
		{ { kEv, 256, true }, { kEv, 162, false }, { kEv, 164, true } } },
	{	20, 2052, "NLO52",
		{ { kEv, 256, true }, { kEv, 164, false } } },
	{	21, 2053, "NLO53",
		{ { kEv, 164, true }, { kEv, 105, false } } },
	{	22, 2055, "NLO55",
		{ { kEv, 270, true }, { kEv, 167, false } } },
	{	23, 2056, "NLO56",
		{ { kEv, 60, true }, { kEv, 89, false }, { kEv, 160, false } } },
	{	24, 2057, "NLO57",
		{ { kEv, 338, true }, { kEv, 169, false } } }
},
{	// Jacques, 8 responses + 1 repeat
	{	25, 3050, "NJB50",
		{ { kEv, 173, true }, { kEv, 147, false } } },
	{	26, 3052, "NJB52",
		{ { kEv, 163, true }, { kEv, 150, false } } },
	{	27, 3053, "NJB53",
		{ { kEv, 127, true }, { kEv, 37, false } } },
	{	27, 3053, "NJB53",
		{ { kEv, 338, true }, { kEv, 37, false } } },
	{	28, 3054, "NJB54",
		{ { kEv, 145, false } } },
	{	29, 3056, "NJB56",
		{ { kEv, 255, true }, { kEv, 151, false }, { kEv, 50, false } } },
	{	30, 3062, "NJB62",
		{ { kEv, 69, true }, { kEv, 146, false } } },
	{	31, 3063, "NJB63",
		{ { kEv, 339, true }, { kEv, 334, true }, { kEv, 144, false } } },
	{	32, 3060, "NJB60",
		{ { kEv, 238, false }, { kIn, 5, false } } }
},
{	// Professor Hotchkiss, 8 responses
	{	33, 5050, "NHL50",
		{ { kEv, 272, true }, { kEv, 136, false } } },
	{	34, 5051, "NHL51",
		{ { kEv, 264, true }, { kEv, 131, false } } },
	{	35, 5052, "NHL52",
		{ { kEv, 254, true }, { kEv, 129, false } } },
	{	36, 5053, "NHL53",
		{ { kEv, 176, true }, { kEv, 229, false } } },
	{	37, 5055, "NHL55",
		{ { kEv, 239, true }, { kEv, 38, false } } },
	{	38, 5057, "NHL57",
		{ { kEv, 153, true }, { kEv, 265, true }, { kEv, 225, false }, { kIn, 14, true } } },
	{	39, 5058, "NHL58",
		{ { kEv, 256, true }, { kEv, 326, true }, { kEv, 227, false } } },
	{	40, 5060, "NHL60",
		{ { kEv, 49, false }, { kIn, 6, true } } }
},
{	// Bess and George, 19 responses + 1 repeat
	{	41, 6520, "NBG20A",
		{ { kEv, 145, true }, { kEv, 40, false } } },
	{	42, 6521, "NBG21a",
		{ { kEv, 338, true }, { kEv, 170, false }, { kEv, 122, false } } },
	{	43, 6522, "NBG22a",
		{ { kEv, 175, true }, { kEv, 287, false }, { kEv, 119, false } } },
	{	44, 6523, "NBG23A",
		{ { kEv, 172, true }, { kEv, 70, true }, { kEv, 69, false }, { kEv, 180, false } } },
	{	45, 6524, "NBG24a",
		{ { kEv, 338, true }, { kEv, 50, false }, { kEv, 133, true }, { kEv, 41, false } } },
	{	46, 6525, "NBG25A",
		{ { kEv, 263, true }, { kEv, 177, true }, { kEv, 244, false } } },
	{	47, 6526, "NBG26a",
		{ { kEv, 271, true }, { kEv, 187, false }, { kIn, 1, true } } },
	{	48, 6527, "NBG27a",
		{ { kEv, 338, true }, { kEv, 340, true }, { kEv, 183, false } } },
	{	49, 6528, "NBG28a",
		{ { kEv, 75, true }, { kEv, 66, false }, { kEv, 178, false } } },
	{	50, 6530, "NBG30a",
		{ { kEv, 184, true }, { kEv, 277, true }, { kEv, 190, false } } },
	{	51, 6531, "NBG31a",
		{ { kEv, 239, true }, { kEv, 177, true }, { kEv, 187, true }, { kEv, 189, false } } },
	{	52, 6532, "NBG32a",
		{ { kEv, 166, true }, { kEv, 170, false }, { kEv, 327, false }, { kEv, 185, false } } },
	{	53, 6533, "NBG33a",
		{ { kEv, 184, true }, { kEv, 265, true }, { kEv, 190, false }, { kEv, 328, false }, { kEv, 182, false }, { kIn, 11, false } } },
	{	54, 6534, "NBG34a",
		{ { kEv, 225, true }, { kEv, 179, false }, { kIn, 14, true } } },
	{	55, 6535, "NBG35a",
		{ { kEv, 120, false }, { kIn, 7, true } } },
	{	56, 6536, "NBG36a",
		{ { kEv, 153, true }, { kEv, 266, true }, { kEv, 267, true }, { kEv, 260, true }, { kEv, 121, false } } },
	{	57, 6537, "NBG37a",
		{ { kEv, 189, true }, { kEv, 260, true }, { kEv, 360, false }, { kEv, 277, true } } },
	{	57, 6537, "NBG37a",
		{ { kEv, 189, true }, { kEv, 260, true }, { kEv, 360, false }, { kEv, 159, true } } },
	{	58, 6570, "NBG70",
		{ { kEv, 171, true }, { kEv, 42, false } } },
	{	59, 6571, "NBG71a",
		{ { kEv, 171, true }, { kEv, 42, true } } },
},
{	// Ned, 14 responses + 1 repeat
	{	60, 7020, "NDN20a",
		{ { kEv, 338, true }, { kEv, 257, false }, { kEv, 201, false } } },
	{	61, 7021, "NDN21a",
		{ { kEv, 173, true }, { kEv, 261, true }, { kEv, 181, false } } },
	{	62, 7022, "NDN22a",
		{ { kEv, 255, true }, { kEv, 261, true }, { kEv, 193, false } } },
	{	62, 7022, "NDN22a",
		{ { kEv, 255, true }, { kEv, 274, true }, { kEv, 193, false } } },
	{	63, 7023, "NDN23a",
		{ { kEv, 133, true }, { kEv, 50, false }, { kEv, 197, false } } },
	{	64, 7024, "NDN24a",
		{ { kEv, 273, true }, { kEv, 289, false }, { kEv, 191, false } } },
	{	65, 7025, "NDN25a",
		{ { kEv, 259, true }, { kEv, 117, false }, { kEv, 202, false } } },
	{	66, 7026, "NDN26",
		{ { kEv, 193, true }, { kEv, 195, false }, { kIn, 7, true } } },
	{	67, 7027, "NDN27",
		{ { kEv, 265, true }, { kEv, 198, false } } },
	{	68, 7028, "NDN28",
		{ { kEv, 136, true }, { kEv, 247, false } } },
	{	69, 7029, "NDN29",
		{ { kEv, 131, true }, { kEv, 248, false } } },
	{	70, 7030, "NDN30a",
		{ { kEv, 129, true }, { kEv, 248, true }, { kEv, 49, false }, { kEv, 194, false }, { kIn, 6, false } } },
	{	71, 7031, "NDN31a",
		{ { kEv, 176, true }, { kEv, 138, false }, { kEv, 186, false } } },
	{	72, 7070, "NNN70",
		{ { kEv, 174, true }, { kEv, 199, false } } },
	{	73, 7071, "NNN71",
		{ { kEv, 174, true }, { kEv, 199, true } } },
}
};

const Common::Array<Goodbye> _nancy4Goodbyes = {
    { "NDE90", { { { 1090, 1091, 1092, 1094, 1096 }, {}, NOFLAG } } },	// Mr. Egan
    { "NLO90", { { { 2090, 2091, 2092 }, {}, NOFLAG } } },				// Lisa
    { "NJB90", { { { 3091, 3092, 3093, 3094 }, {}, NOFLAG } } },		// Jacques
    { "NHL90", { { { 5090, 5091, 5092 }, {}, NOFLAG } } },				// Prof. Hotchkiss
    { "NBG90", { { { 6590, 6591, 6592, 6593, 6594 }, {}, NOFLAG } } },	// Bess & George
    { "NDN90", { { { 7090, 7091, 7092, 7093, 7094 }, {}, NOFLAG } } },	// Ned
};

const Common::Array<Common::Array<const char *>> _nancy4ConditionalDialogueTexts {
{	// English
	// 00
	"I think there's something wrong with my radiator. It hisses, and there's a clanging noise, too. Could you check it out for me?<h><n>", // NDE50
	"Can you do something about my radiator?<h><n>", // NDE51
	"About my radiator, Mr. Egan. Do you think you'll be able to fix it any time soon?<h><n>", // NDE52
	"I don't want to pester you, Mr. Egan, but...the radiator?<h><n>", // NDE53
	"The professor says she has a hankering for, ummm, cous cous.<h><n>", // NDE54
	// 05
	"The Professor has changed her order. Seems she's developed an appetite for chicken drumsticks, fifty of them!<h><n>", // NDE55
	"I was in the elevator and it got stuck between floors! I had to climb out the top and I just barely made it up to the floor above. Do you think you'll be able to fix it?<h><n>", // NDE57
	"So, did you grow up around here?<h><n>", // NDE58
	"Did you know the original owner, Ezra Wickford, when he lived here?<h><n>", // NDE59
	"You've been holding out on me, Mr. Egan. I read in an old issue of the Daily Telegraph that you grew up here - right in this castle.<h><n>", // NDE61
	// 10
	"So, do you know if Ezra Wickford had a place where he liked to go and think, some place he thought of as a 'refuge'?<h><n>", // NDE62
	"I don't want to press my luck with you, but I sure would like to see Ezra's private garden. Could you tell me how to find it?<h><n>", // NDE41b
	"I'm trying to get into the tower but I can't get across that big pit.<h><n>", // NDE67
	"I got across the pit but I'm stuck at the bottom of the tower stairs.<h><n>", // NDE68
	"Is that some sort of puzzle on the wall of the Queen's Tower?<h><n>", // NDE69
	// 15
	"So how long have you been working here?<h><n>", // NDE71
	"Mr. Egan, that red dirt you asked me about? It came from the tunnel that leads to the Queen's tower. I should have told you in the first place, but I was afraid you'd be mad.<h><n>", // NDE74
	"Why are those holes in the crest on the floor of the tower room?<h><n>", // NDE78
	"What do you know about Jacques Brunais?<h><n>", // NLO50
	"There was a little mix-up with the lockers and I accidentally opened yours.<h><n>", // NLO51
	// 20
	"So, tell me more about the life of a photojournalist. It must be very glamorous at times.<h><n>", // NLO52
	"Does your job take you to exotic far-off places?<h><n>", // NLO53
	"Did you know Professor Hotchkiss published a book on Marie Antoinette?<h><n>", // NLO55
	"When you researched this place, did you find any references to a secret garden?<h><n>", // NLO56
	"I'm dying to find a way into that tower. Where do they usually hide the secret entrances in weird old Midwestern mansions?<h><n>", // NLO57
	// 25
	"Tell me about these boxes you're making.<h><n>", // NJB50
	"Lisa told me you were in the Olympics. What was that like?<h><n>", // NJB52
	"Do you know what happened in the library? I heard it was vandalized.<h><n>", // NJB53
	"It must be lonely living so far from home. Don't you miss your family?<h><n>", // NJB54
	"Do you know much about the tower that's closed off? I heard the original owner imported it from France.<h><n>", // NJB56
	// 30
	"Dexter needs you to defrost that big bag of chicken legs.<h><n>", // NJB62
	"Dexter told me the combination to my locker, number 310, is 5-1-7. I tried it but the locker won't open.<h><n>", // NJB63
	"I need to bring Professor Hotchkiss her boots. Do you have them?<h><n>", // NJB60
	"What does 'L'espoir a ceux qui cherchent' mean?<h><n>", // NHL50
	"Can you translate this for me? 'Le diamant de misere dans mon journal.'<h><n>", // NHL51
	// 35
	"Can you tell me what this means in English?: 'La solution se trouve dedans.'<h><n>", // NHL52
	"What did you mean when you said Marie Antoinette was misunderstood?<h><n>", // NHL53
	"Do you know anything about a tiara that was given to Marie Antoinette?<h><n>", // NHL55
	"I saw a letter on your desk from your friend, the Baronness von Hanseldorf, and I'm just wondering, did the medallion she gave you have a blue stone in it, by any chance?<h><n>", // NHL57
	"What do you think of Lisa Ostrum?<h><n>", // NHL58
	// 40
	"I found something that I think you might be very interested in. It seems to be some kind of journal. I think it was written by Marie Antoinette, herself!<h><n>", // NHL60
	"I met my ski instructor, Jacques Brunais. He sure is French.<h><n>", // NBG20A
	"I met this nice woman, Lisa Ostrum.<h><n>", // NBG21a
	"I'm trying to meet with Professor Hotchkiss to find out what was stolen from her room, but she won't open her door to discuss it.<h><n>", // NBG22a
	"It seems like every time I talk to Dexter he has a new chore for me to do!<h><n>", // NBG23A
	// 45
	"You guys! I've heard these awful screeching and banging noises at night.<h><n>", // NBG24a
	"Get this: I found a secret room in the library.<h><n>", // NBG25A
	"Wickford wrote Dexter a farewell poem before he died.<h><n>", // NBG26a
	"I stumbled onto Lisa Ostrum's camera bag and found a bunch of fake ID's.<h><n>", // NBG27a
	"After I got knocked out, Dexter called me down to the front desk to ask me where the red dirt on my shoes came from.<h><n>", // NBG28a
	// 50
	"I think Professor Hotchkiss made an important discovery while she was filming that video tape.<h><n>", // NBG30a
	"I've been doing some research on Marie Antoinette.<h><n>", // NBG31a
	"You'll never guess what Lisa told me!<h><n>", // NBG32a
	"I think Hotchkiss has a medallion that belonged to Marie Antoinette, too! It has something to do with her research.<h><n>", // NBG33a
	"This is so weird: I found the blue medallion from Jacques' locker in Hotchkiss' room. But she says her medallion had a green stone!<h><n>", // NBG34a
	// 55
	"I found Ezra Wickford's secret garden! You'll never guess what Dexter's boyhood 'luck charm' was!<h><n>", // NBG35a
	"I went back to check out Jacques' locker, and I found a pamphlet about diamonds, a letter from his fiancee, and a warning notice from immigration!<h><n>", // NBG36a
	"There sure is a lot of talk about diamonds around this place!<h><n>", // NBG37a
	"Ladies, I need inspiration, brain juice, a hint!<h><n>", // NBG70
	"Help! I'm a little stuck!<h><n>", // NBG71a
	// 60
	"I'm trying to get into the library, but the door is locked and I can't find any other way in.<h><n>", // NDN20a
	"Apparently this castle's tower used to be a hang-out for Marie Antoinette.<h><n>", // NDN21a
	"Are you ready for a crazy coincidence?<h><n>", // NDN22a
	"There are these creepy sounds at night! I can't figure out what, or who, could be causing them.<h><n>", // NDN23a
	"How can I get to that secret door when the elevator's always in the way?<h><n>", // NDN24a
	// 65
	"According to the decoder, the symbols in the stained glass window mean 'Purple Rose Holds Diamond Key of Queen.'<h><n>", // NDN25a
	"How do you think Ezra Wickford got his hands on Marie Antoinette's red medallion?<h><n>", // NDN26
	"How do you think Professor Hotchkiss ended up with Marie Antoinette's green medallion?<h><n>", // NDN27
	"What do you think 'hope to those who search' means?<h><n>", // NDN28
	"What do you think 'Diamond of misery in my journal' means?<h><n>", // NDN29
	// 70
	"What do you think this means? 'You will find the decoder within.'<h><n>", // "NDN30a
	"I finally got to meet Professor Hotchkiss in the lobby - at 3 am of all times! It turns out she's a scholar of French History and her specialty is Marie Antoinette!<h><n>", // NDN31a
	"Put on your thinking cap! I need a hint.<h><n>", // NNN70
	"I'm trying to decide what to do next. Any thoughts?<h><n>" // NNN71
},
{	// Russian
	// 00
	"S batareej v moej komnate yto-to ne tak. Ona wipit i stuyit. V% mogete poyinit+ ee?<h><n>",
	"V% mogete otremontirovat+ batare/ v moej komnate?<h><n>",
	"Mister Ihan, v% pomnite o bataree v moej komnate? Moget, v% ee kak-nibud+ poyinite?<h><n>",
	") b% ne xotela nadoedat+ vam s pros+bami... No, mister Ihan, yto s moej batareej?<h><n>",
	"Professor Xoykis skazala, yto na ugin ona b% s=ela kuskus.<h><n>",
	// 05
	"Missis Xoykis izmenila svoe mnenie. Kagets&, u nee prosnuls& appetit k garen%m kurin%m nogkam. Ona zakazala p&t+des&t wtuk!<h><n>",
	") exala na lifte, i on zastr&l megdu ;tagami! Mne priwlos+ v%birat+s& yerez l/k v potolke lifta, i & ele dostala do dveri sledu/qeho ;taga.<h><n>",
	"V% proveli svoe detstvo v ;toj mestnosti?<h><n>",
	"V% znali vladel+ca zamka Yzru Uikforda?<h><n>",
	"Mister Ihan, mne vse izvestno. V starom nomere 'Dnevnoho obozreni&' & proyitala, yto v% v%rosli v ;tom zamke.<h><n>",
	// 10
	"V% ne znaete, b%lo li u Yzr% mesto, hde on l/bil spokojno porazm%wl&t+? Mesto, hde on yuvstvoval seb& v bezopasnosti.<h><n>",
	") ne xoyu vas bespokoit+, no mne nugno najti sad Yzr% Uikforda. Ne znaete, hde on?<h><n>",
	") p%ta/s+ popast+ v bawn/, no ne mohu projti mimo kolodca.<h><n>",
	") prowla mimo kolodca, no ne mohu podn&t+s& po stupen+kam v bawn/.<h><n>",
	"Po-moemu, na stene korolevskoj bawni est+ kaka&-to holovolomka. Vam tak ne kagets&?<h><n>",
	// 15
	"V% davno zdes+ rabotaete?<h><n>",
	"Mister Ihan, & dolgna vam koe v yem priznat+s&. Krasna& p%l+ na moix botinkax - iz tunnel&, kotor%j vedet v korolevsku/ bawn/. Mne nugno b%lo srazu skazat+ vam ob ;tom, no & prosto ispuhalas+.<h><n>",
	"V polu komnat% bawni est+ kakie-to v%emki. V% znaete, zayem oni?<h><n>",
	"Jto v% znaete o Gake Brun;?<h><n>",
	"So wkafyikami v%wla putanica, i & sluyajno otkr%la vaw.<h><n>",
	// 20
	"Rasskagite mne o gizni fotogurnalista. Navernoe, ona oyen+ interesna&.<h><n>",
	"V% yasto ezdite v komandirovki v ;kzotiyeskie stran%?<h><n>",
	"V% znaete, yto professor Xoykis napisala knihu o Marii-Antuanette?<h><n>",
	"Kohda v% osmatrivali zamok, v% nawli kakie-nibud+ upominani& o sade?<h><n>",
	") oyen+ xoyu popast+ v bawn/. Hde ob%yno dela/t sekretn%j xod v osobn&kax na Srednem Zapade?<h><n>",
	// 25
	"Rasskagite mne o korobkax, kotor%e v% delaete.<h><n>",
	"Liza skazala, yto v% uyastvovali v Olimpijskix ihrax. I kak uspexi?<h><n>",
	"V% znaete, yto sluyilos+ v biblioteke? Mne skazali, yto tam vse razhromleno.<h><n>",
	"Navernoe, inohda vam odinoko. V% ne skuyaete po svoej sem+e?<h><n>",
	"Jto v% znaete o zakr%toj bawne? Hovor&t, yto xoz&in zamka privez ee iz Francii.<h><n>",
	// 30
	"Deksteru nugna vawa pomoq+, ytob% razmorozit+ bol+woj paket kurin%x nogek.<h><n>",
	"Dekster skazal, yto moj wkafyik nomer 310 i kod k nemu - 5-1-7. ) probovala eho otkr%t+, no u men& ne poluyilos+.<h><n>",
	"Mne nugno otnesti professoru Xoykis ee botinki. Oni u vas?<h><n>",
	"Jto znayit 'Lespuar a se ki werw'?<h><n>",
	"V% mogete perevesti ;tu frazu: 'Le diamon de mizer dan mon gernal+'?<h><n>",
	// 35
	"Pogalujsta, pomohite mne s perevodom: 'L& sol/s+on se truv dedon'.<h><n>",
	"Jto v% imeli v vidu, kohda skazali, yto Mari&-Antuanetta b%la ne pon&ta?<h><n>",
	"Jto v% znaete o tiare, kotoru/ podarili Marii-Antuanette?<h><n>",
	"Na vawem stole & videla pis+mo ot baroness% fon Xansel+dorf. Medal+on, kotor%j ona vam podarila, sluyajno, ne s holub%m kamnem?<h><n>",
	"Jto v% dumaete o Lize Ostrum?<h><n>",
	// 40
	") nawla to, yto dolgno vas zainteresovat+. Duma/, ;to dnevnik Marii-Antuanett%!<h><n>",
	") poznakomilas+ s l%gn%m instruktorom, Gakom Brun;. On nasto&qij francuz.<h><n>",
	") poznakomilas+ s Lizoj Ostrum. Mila& genqina.<h><n>",
	") xoyu uznat+, yto ukrali iz nomera professora Xoykis, no ona ne otkr%vaet dver+.<h><n>",
	"Kagd%j raz, kohda & razhovariva/ s Deksterom, on daet mne novoe zadanie!<h><n>",
	// 45
	"Predstavl&ete, & vs/ noy+ sl%wala kakoj-to skrip i stuk.<h><n>",
	") nawla sekretnu/ komnatu v biblioteke.<h><n>",
	"Pered smert+/ Uikford napisal Deksteru stixotvorenie.<h><n>",
	") sluyajno otkr%la wkafyik Liz% i obnarugila tam neskol+ko fal+wiv%x pasportov.<h><n>",
	"Kohda & oynulas+, Dekster pozval men& i sprosil, otkuda na moix botinkax vz&las+ krasna& p%l+.<h><n>",
	// 50
	"Duma/, vo vrem& s=emki professor Xoykis sdelala vagnoe otkr%tie.<h><n>",
	") zdes+ poyitala knihu o Marii-Antuanette.<h><n>",
	"Nikohda ne dohadaetes+, yto mne skazala Liza!<h><n>",
	"Duma/, u Xoykis toge est+ medal+on Marii-Antuanett%! On kak-to sv&zan s ee rabotoj.<h><n>",
	"Tak stranno: v komnate Xoykis & nawla medal+on s holub%m kamnem iz wkafyika Gaka. No ona hovorit, yto ee medal+on b%l s zelen%m kamnem!<h><n>",
	// 55
	") videla sad Yzr% Uikforda. Uhadajte, yto & tam nawla?!<h><n>",
	"V wkafyike Gaka & nawla brow/ru o brilliantax, pis+mo eho nevest% i predupregdenie slugb% immihracii!<h><n>",
	"Zdes+ oyen+ mnoho hovor&t o brilliantax!<h><n>",
	"Podruhi, mne sroyno nugna podskazka!<h><n>",
	"Pomohite! ) ne zna/, yto delat+ dal+we.<h><n>",
	// 60
	"Mne nugno popast+ v biblioteku, no dver+ zakr%ta, a druhoho vxoda & ne mohu najti.<h><n>",
	"Vero&tno, v bawne zamka dergali Mari/-Antuanettu.<h><n>",
	"T% hotov k oweloml&/qej novosti?<h><n>",
	"Noy+/ & sl%wala kakie-to strann%e zvuki! ) ne zna/, kto ili yto ix proizvodit.<h><n>",
	"Kak mne dobrat+s& do potajnoj dveri, esli ee zakr%vaet lift?<h><n>",
	// 65
	") raswifrovala simvol% na vitrage. Vot yto poluyilos+: 'V lilovoj roze kl/y k brilliantu korolev%'.<h><n>",
	"Kakim obrazom k Yzre Uikfordu popal medal+on Marii-Antuanett% s krasn%m kamnem?<h><n>",
	"Kakim obrazom k professoru Xoykis popal medal+on Marii-Antuanett% s zelen%m kamnem?<h><n>",
	"Jto oznayaet 'nade/s+ na tex, kto iqet'?<h><n>",
	"Jto oboznayaet fraza: 'brilliant stradanij v moem dnevnike'?<h><n>",
	// 70
	"Jto znayit 'otvet najdets& vnutri'?<h><n>",
	") nakonec-to vstretila professora Xoykis v xolle. V tri yasa noyi! Okaz%vaets&, ona prepodaet istori/ Francii i piwet rabotu o Marii-Antuanette.<h><n>",
	"Mne nugna podskazka.<h><n>",
	"Mne nugno rewit+, yto delat+ dal+we. Est+ kakie-nibud+ predlogeni&?<h><n>"
}
};

const Common::Array<Common::Array<const char *>> _nancy4GoodbyeTexts = {
{	// English
	"Okay, see ya, Mr. Egan!<h>", // NDE90
	"I'll let you get back to your magazine.<h>", // NLO90
	"I'll talk to you later.<h>", // NJB90
	"See you soon.<h>", // NHL90
	"OK, you two. I'll talk to you soon!<h>", // NBG90
	"Bye, Ned.<h>" // NDN90
},
{	// Russian
	"Uvidims&, mister Ihan!<h>",
	"Pohovorim pozge!<h>",
	"Ne budu vam mewat+.<h>",
	"Do skoroho.<h>",
	"Do vstreyi.<h>",
	"Poka, N;d.<h>"
}
};

const Common::Array<const char *> _nancy4TelephoneRinging = {
    "ringing...<n><e>", // English
    "Hudki...  <n><e>"  // Russian
};

const Common::Array<const char *> _nancy4EmptySaveStrings = {
	"Nothing Saved Here",	// English
	"- - - - -"				// Russian
};

const Common::Array<const char *> _nancy4EventFlagNames = {
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
	"EV_Accepted_Riddle",
	"EV_Alarm_Reset",
	"EV_Asked_JB_Lib",
	"EV_Asked_PH_Tiara",
	"EV_B_Said_Brush",
	"EV_B_Said_Illegal",
	"EV_B_Said_Indigestion",
	"EV_B_Said_Silver_Platter",
	"EV_B_Said_Switchboard",
	"EV_Batteries_Out",
	"EV_Batteries_Recharged",
	"EV_Blue_In_Blue_Crest",
	"EV_Blue_In_Green_Crest",
	"EV_Blue_In_Red_Crest",
	"EV_Brought_PH_Journal",
	"EV_Busted_JB",
	"EV_Call_Button",
	"EV_Candle_Lit",
	"EV_Caught_JB",
	"EV_DE_At_Library_Door",
	"EV_DE_Available",
	"EV_DE_BustI",
	"EV_DE_Hang_Up",
	"EV_DE_In_Library",
	"EV_DE_Mad1",
	"EV_DE_Mentions_SecretGarden",
	"EV_DE_Said_Adopted",
	"EV_DE_Said_Cando",
	"EV_DE_Said_Chains",
	"EV_DE_Said_Crest",
	"EV_DE_Said_Diaper",
	"EV_DE_Said_Dirt",
	"EV_DE_Said_EW",
	"EV_DE_Said_Flowers",
	"EV_DE_Said_Freezer",
	"EV_DE_Said_Fuse",
	"EV_DE_Said_Gatekey",
	"EV_DE_Said_Growup",
	"EV_DE_Said_Hang_Up",
	"EV_DE_Said_I_Fixed",
	"EV_DE_Said_Knockout",
	"EV_DE_Said_Malfunction",
	"EV_DE_Said_Menu",
	"EV_DE_Said_More_Snow",
	"EV_DE_Said_NoLib",
	"EV_DE_Said_Not_Here",
	"EV_DE_Said_Past",
	"EV_DE_Said_PH_Dinner",
	"EV_DE_Said_RedDirt",
	"EV_DE_Said_Ricochet",
	"EV_DE_Said_Switch",
	"EV_DE_Said_Thinking",
	"EV_DE_Said_Weeds",
	"EV_DE_Took_Envelope",
	"EV_DE_Where_SecretGarden",
	"EV_Diamond_Key_In_Crest",
	"EV_Elevator_Back",
	"EV_Elevator_In_Use",
	"EV_Elevator_Moved",
	"EV_Elevator_Off",
	"EV_Empty1333",
	"EV_Event01",
	"EV_Event02",
	"EV_Event03",
	"EV_Event04",
	"EV_Event05",
	"EV_Event06",
	"EV_Event07",
	"EV_Event08",
	"EV_Event09",
	"EV_Event10",
	"EV_Event11",
	"EV_Event12",
	"EV_Event13",
	"EV_Event14",
	"EV_Event15",
	"EV_Event16",
	"EV_Event17",
	"EV_Event18",
	"EV_Event19",
	"EV_Event20",
	"EV_Fixed_Fuse",
	"EV_Found_4th_Medallion",
	"EV_Found_Blue_in_PH",
	"EV_G_Said_Elephant",
	"EV_G_Said_Leprechaun",
	"EV_G_Said_Pressure",
	"EV_G_Said_PrimeMinister",
	"EV_Got_Blue_Medal",
	"EV_Got_Stuck",
	"EV_Green_In_Blue_Crest",
	"EV_Green_In_Green_Crest",
	"EV_Green_In_Red_Crest",
	"EV_Heard_Argument",
	"EV_Heard_Blue_Translation",
	"EV_Heard_DE_Msg",
	"EV_Heard_Green_Translation",
	"EV_Heard_LO_Msg",
	"EV_Heard_Nightsounds",
	"EV_Heard_PH_Msg",
	"EV_Heard_Phone",
	"EV_Heard_Red_Translation",
	"EV_In_Elevator",
	"EV_In_PH_Room",
	"EV_In_Secret_Room",
	"EV_In_Tower",
	"EV_JB_Available",
	"EV_JB_Gave_Combo",
	"EV_JB_Phone",
	"EV_JB_Said_Combo",
	"EV_JB_Said_Fiancee",
	"EV_JB_Said_Freezer",
	"EV_JB_Said_HopeBox",
	"EV_JB_Said_MA",
	"EV_JB_Said_Nightmare",
	"EV_JB_Said_Olympics",
	"EV_JB_Said_Tower",
	"EV_JB_Said_Whereisit",
	"EV_Knocked_Out",
	"EV_Library_Door_Open",
	"EV_Lights_Out",
	"EV_Lit_Candle",
	"EV_LO_Angry",
	"EV_LO_Available",
	"EV_LO_Said_Fat_Diamond",
	"EV_LO_Said_Garden",
	"EV_LO_Said_Helper",
	"EV_LO_Said_ID",
	"EV_LO_Said_JB",
	"EV_LO_Said_Job",
	"EV_LO_Said_Lockout",
	"EV_LO_Said_Medallion",
	"EV_LO_Said_Review",
	"EV_LO_Said_Theory",
	"EV_LO_Said_Weird",
	"EV_Locked_Out",
	"EV_Met_BG",
	"EV_Met_DE",
	"EV_Met_JB",
	"EV_Met_NN",
	"EV_Met_PH_at_Door",
	"EV_Met_PH_in_Lobby",
	"EV_N_Said_Cinematic",
	"EV_N_Said_Dirt",
	"EV_N_Said_Found_Blue",
	"EV_N_Said_Garbage",
	"EV_N_Said_Hangout",
	"EV_N_Said_Helga_Letter",
	"EV_N_Said_IDs",
	"EV_N_Said_JB_Suspect",
	"EV_N_Said_Lisa_Tip",
	"EV_N_Said_PH_Key",
	"EV_N_Said_Poem",
	"EV_N_Said_Stuck",
	"EV_N_Said_Tiara",
	"EV_N_Said_VideoII",
	"EV_NN_Said_Alarm",
	"EV_NN_Said_Alvin",
	"EV_NN_Said_Boeuf",
	"EV_NN_Said_Connected",
	"EV_NN_Said_Cow",
	"EV_NN_Said_Disappear",
	"EV_NN_Said_Ghosts",
	"EV_NN_Said_Helga",
	"EV_NN_Said_Hint",
	"EV_NN_Said_Hotline",
	"EV_NN_Said_Portal",
	"EV_NN_Said_Purple_Rose",
	"EV_NN_Said_Secrets",
	"EV_Oil_Comment",
	"EV_On_Floor1",
	"EV_On_Floor2",
	"EV_On_FloorB",
	"EV_On_Phone",
	"EV_On_Stuck",
	"EV_Opened_EW_Head",
	"EV_PH_Asked_Riddle",
	"EV_PH_Asked_Riddle1",
	"EV_PH_Asked_Riddle2",
	"EV_PH_Asked_Riddle3",
	"EV_PH_Asked_Riddle4",
	"EV_PH_Asked_Riddle5",
	"EV_PH_Available",
	"EV_PH_Got_Boots",
	"EV_PH_Said_Beep",
	"EV_PH_Said_Blue_Medal_Hint",
	"EV_PH_Said_Both",
	"EV_PH_Said_Couscous",
	"EV_PH_Said_Drumsticks",
	"EV_PH_Said_Hint",
	"EV_PH_Said_Imposter",
	"EV_PH_Said_Leave_Boots",
	"EV_PH_Said_LO",
	"EV_PH_Said_Menu",
	"EV_PH_Said_Myth",
	"EV_PH_Said_Protein",
	"EV_PH_Said_Recharged",
	"EV_PH_Said_Theory",
	"EV_PH_Said_Translated",
	"EV_PH_Said_Translation",
	"EV_PH_Takes_Boots",
	"EV_PH_Takes_Menu",
	"EV_Placed_Menu",
	"EV_Placed_PH_Boots",
	"EV_Read_Painting_Book",
	"EV_Red_In_Blue_Crest",
	"EV_Red_In_Green_Crest",
	"EV_Red_In_Red_Crest",
	"EV_Rescued",
	"EV_Said_Adopted",
	"EV_Said_Close_Call",
	"EV_Said_DoSomething",
	"EV_Said_Hope",
	"EV_Said_Misery",
	"EV_Said_Radi",
	"EV_Said_Stairs",
	"EV_Said_Stuck",
	"EV_Said_Working",
	"EV_Said_Your_Room",
	"EV_Saw_Blue_Message",
	"EV_Saw_Brochure",
	"EV_Saw_Camerabag",
	"EV_Saw_Cinematic",
	"EV_Saw_Crest",
	"EV_Saw_Decoder",
	"EV_Saw_Diamond_Book",
	"EV_Saw_Encyclopedia",
	"EV_Saw_Ezra_Poem",
	"EV_Saw_GoodBad_Book",
	"EV_Saw_Green_Message",
	"EV_Saw_Helga_Letter",
	"EV_Saw_INS",
	"EV_Saw_Isabelle",
	"EV_Saw_Marie_Painting",
	"EV_Saw_Nightmare",
	"EV_Saw_PH_Book",
	"EV_Saw_Poem",
	"EV_Saw_Red_Message",
	"EV_Saw_Shaft_Door",
	"EV_Saw_SpyDiary",
	"EV_Saw_Telegraph",
	"EV_Saw_VideoI",
	"EV_Saw_VideoII",
	"EV_Solved_Chains",
	"EV_Solved_Crest",
	"EV_Solved_Diaper",
	"EV_Solved_Door_Puzzle",
	"EV_Solved_Garden_Gate",
	"EV_Solved_Globe",
	"EV_Solved_Keypad",
	"EV_Solved_Lever",
	"EV_Solved_Ricochet",
	"EV_Solved_Riddle",
	"EV_Solved_Sextant",
	"EV_Solved_Shaft_Door",
	"EV_Stuck",
	"EV_Sun_East",
	"EV_Sun_North",
	"EV_Sun_South",
	"EV_Sun_West",
	"EV_Switch01",
	"EV_Switch02",
	"EV_Switch03",
	"EV_Switch04",
	"EV_Switch05",
	"EV_Switch06",
	"EV_Switch07",
	"EV_Switch08",
	"EV_Switch09",
	"EV_Switch10",
	"EV_Switch11",
	"EV_Switch12",
	"EV_Switch13",
	"EV_Switch14",
	"EV_Switch15",
	"EV_Switch16",
	"EV_Switch17",
	"EV_Switch18",
	"EV_Switch19",
	"EV_Switch20",
	"EV_Tile01_Off",
	"EV_Tile02_Off",
	"EV_Tile03_Off",
	"EV_Tile04_Off",
	"EV_Tile05_Off",
	"EV_Tile06_Off",
	"EV_Tile07_Off",
	"EV_Tile08_Off",
	"EV_Tile09_Off",
	"EV_Tile10_Off",
	"EV_Took_Journal",
	"EV_Took_Medallion_Blue",
	"EV_Took_Medallion_Green",
	"EV_Took_Medallion_Red",
	"EV_Took_Medallion_Square",
	"EV_Took_Paintbrush",
	"EV_Tried_Chains",
	"EV_Tried_Diaper",
	"EV_Tried_Gate",
	"EV_Tried_Locker",
	"EV_Tried_Ricochet",
	"EV_Turn_On",
	"EV_Went_Outside",
	"EV_Met_LO",
	"EV_Saw_Combo",
	"EV_Saw_IDs",
	"EV_Empty1341",
	"EV_Empty1342",
	"EV_Empty1343",
	"EV_Empty1344",
	"EV_Empty1345",
	"EV_Empty1346",
	"EV_Empty1347",
	"EV_Empty1348",
	"EV_Empty1349",
	"EV_Empty1350",
	"EV_Empty1351",
	"EV_Empty1352",
	"EV_Empty1353",
	"EV_Empty1354",
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
	"EV_Empty1383",
	"EV_Empty1384",
	"EV_Empty1385",
	"EV_Empty1386",
	"EV_Empty1387",
	"EV_Empty1388",
	"EV_Empty1389",
	"EV_Empty1390",
	"EV_Empty1391",
	"EV_Empty1392",
	"EV_Empty1393",
	"EV_Empty1394",
	"EV_Empty1395",
	"EV_Empty1396",
	"EV_Empty1397",
	"EV_Empty1398",
	"EV_Empty1399",
	"EV_Empty1400",
	"EV_Empty1401",
	"EV_Empty1402",
	"EV_Empty1403",
	"EV_Empty1404",
	"EV_Empty1405",
	"EV_Empty1406",
	"EV_Empty1407",
	"EV_Empty1408",
	"EV_Empty1409",
	"EV_Empty1410",
	"EV_Empty1411",
	"EV_Empty1412",
	"EV_Empty1413",
	"EV_Empty1414",
	"EV_Empty1415",
	"EV_Empty1416",
	"EV_Empty1417",
	"EV_Empty1418",
	"EV_Empty1419",
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
	"EV_Empty1455",
	"EV_Empty1456",
	"EV_Empty1457",
	"EV_Empty1458",
	"EV_Empty1459",
	"EV_Empty1460",
	"EV_Empty1461",
	"EV_Empty1462",
	"EV_Empty1463",
	"EV_Empty1464",
	"EV_Empty1465",
	"EV_Empty1466",
	"EV_Empty1467",
	"EV_Empty1468",
	"EV_Empty1469",
	"EV_Empty1470",
	"EV_Empty1471",
	"EV_Empty1472",
	"EV_Empty1473",
	"EV_Empty1474",
	"EV_Empty1475",
	"EV_Empty1476",
	"EV_Empty1477",
	"EV_Empty1478",
	"EV_Empty1479",
	"EV_Empty1480",
	"EV_Empty1481",
	"EV_Empty1482",
	"EV_Empty1483",
	"EV_Empty1484",
	"EV_Empty1485",
	"EV_Empty1486",
	"EV_Empty1487",
	"EV_Empty1488",
	"EV_Empty1489",
	"EV_Empty1490",
	"EV_Empty1491",
	"EV_Empty1492",
	"EV_Empty1493",
	"EV_Empty1494",
	"EV_Empty1495",
	"EV_Empty1496",
	"EV_Empty1497",
	"EV_Empty1498",
	"EV_Empty1499",
	"EV_Empty1500",
	"EV_Empty1501",
	"EV_Empty1502",
	"EV_Empty1503"
};

#endif // NANCY4DATA_H
