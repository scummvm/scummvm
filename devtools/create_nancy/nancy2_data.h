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

#ifndef NANCY2DATA_H
#define NANCY2DATA_H

#include "types.h"

const GameConstants _nancy2Constants ={
	18,												// numItems
	240,											// numEventFlags
	{	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,			// genericEventFlags
		11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
		21, 22, 23, 24, 25, 26, 27, 28, 29, 30 },
	5,												// numCursorTypes
	7000,											// logoEndAfter
	32												// wonGameFlagID
};

const Common::Array<GameLanguage> _nancy2LanguagesOrder = {
	GameLanguage::kEnglish,
	GameLanguage::kRussian
};

const Common::Array<Common::Array<ConditionalDialogue>> _nancy2ConditionalDialogue = {
{	// Dwayne, 7 responses + 2 repeats
	{	0, 816, "nda33",
		{ { kEv, 50, kTrue }, { kEv, 118, kFalse } } },
	{	1, 817, "nda34",
		{ { kEv, 49, kTrue }, { kEv, 119, kFalse } } },
	{	2, 820, "nda35",
		{ { kEv, 55, kTrue }, { kEv, 120, kFalse }, { kEv, 76, kFalse } } },
	{	3, 821, "nda15",
		{ { kEv, 67, kTrue }, { kEv, 121, kFalse }, { kEv, 80, kFalse } } },
	{	4, 823, "nda19",
		{ { kEv, 123, kTrue }, { kEv, 122, kFalse } } },
	{	5, 824, "nda22",
		{ { kEv, 124, kFalse }, { kEv, 68, kTrue } } },
	{	5, 824, "nda22",
		{ { kEv, 124, kFalse }, { kEv, 64, kTrue } } },
	{	6, 826, "nda27",
		{ { kEv, 126, kFalse }, { kEv, 56, kTrue } } },
	{	6, 826, "nda27",
		{ { kEv, 126, kFalse }, { kEv, 58, kTrue } } },
	{	7, 829, "nda28",
		{ { kEv, 39, kTrue }, { kEv, 127, kFalse } } }
},
{	// Rick, 4 responses + 1 repeat
	{	8, 729, "NRD29",
		{ { kEv, 115, kFalse }, { kEv, 68, kTrue } } },
	{	8, 729, "NRD29",
		{ { kEv, 115, kFalse }, { kEv, 64, kTrue } } },
	{	9, 728, "NRD28",
		{ { kEv, 50, kTrue }, { kEv, 114, kFalse } } },
	{	10, 717, "NRD18",
		{ { kEv, 48, kTrue }, { kEv, 112, kFalse } } },
	{	11, 721, "NRD21",
		{ { kEv, 151, kTrue }, { kEv, 113, kFalse } } }
},
{	// Millie, 2 responses
	{	12, 317, "NPR12",
		{ { kEv, 52, kTrue }, { kEv, 133, kFalse } } },
	{	13, 321, "NPR15",
		{ { kEv, 64, kTrue }, { kEv, 134, kFalse } } }
},
{	// Lillian, 4 responses + 1 repeat
	{	14, 503, "NLR07",
		{ { kEv, 65, kTrue }, { kEv, 129, kFalse } } },
	{	15, 504, "NLR08",
		{ { kEv, 151, kTrue }, { kEv, 130, kFalse } } },
	{	16, 510, "NLR15",
		{ { kEv, 131, kFalse }, { kEv, 68, kTrue } } },
	{	16, 510, "NLR15",
		{ { kEv, 131, kFalse }, { kEv, 64, kTrue } } },
	{	17, 512, "NLR17",
		{ { kEv, 132, kTrue }, { kEv, 53, kTrue }, { kEv, 131, kTrue }, { kEv, 123, kFalse } } }
},
{	// Ned, 9 responses
	{	18, 3007, "NNP08",
		{ { kEv, 52, kTrue }, { kEv, 101, kFalse }, { kEv, 73, kFalse } } },
	{	19, 3010, "NNP11",
		{ { kEv, 41, kTrue }, { kEv, 79, kFalse }, { kEv, 102, kFalse } } },
	{	20, 3013, "NNP14",
		{ { kEv, 103, kFalse }, { kIn, 15, kTrue } } },
	{	21, 3014, "NNP15",
		{ { kEv, 105, kTrue }, { kEv, 45, kFalse }, { kEv, 92, kFalse } } },
	{	22, 3015, "NNP16",
		{ { kEv, 68, kTrue }, { kEv, 64, kTrue }, { kEv, 59, kFalse }, { kEv, 106, kFalse } } },
	{	23, 3016, "NNP17",
		{ { kEv, 56, kTrue }, { kEv, 57, kFalse }, { kEv, 107, kFalse }, { kEv, 58, kTrue } } },
	{	24, 3017, "NNP28",
		{ { kEv, 57, kTrue }, { kEv, 107, kTrue }, { kEv, 108, kFalse } } },
	{	25, 3019, "NNP20",
		{ { kEv, 67, kTrue }, { kEv, 80, kFalse }, { kEv, 109, kFalse } } },
	{	26, 3020, "NNP21",
		{ { kEv, 67, kTrue }, { kEv, 80, kFalse }, { kEv, 110, kFalse } } }
},
{	// Bess, 18 responses
	{	27, 3123, "NBES32g",
		{ { kEv, 80, kTrue }, { kEv, 44, kFalse }, { kEv, 155, kFalse } } },
	{	28, 3124, "NBES35",
		{ { kEv, 85, kTrue }, { kEv, 62, kFalse }, { kEv, 156, kFalse } } },
	{	29, 3125, "NBES36",
		{ { kEv, 86, kTrue }, { kEv, 76, kFalse }, { kEv, 157, kFalse } } },
	{	30, 3127, "NBES38",
		{ { kEv, 87, kTrue }, { kEv, 59, kFalse }, { kEv, 159, kFalse } } },
	{	31, 3128, "NBES39g",
		{ { kEv, 132, kTrue }, { kEv, 53, kFalse }, { kEv, 160, kFalse } } },
	{	32, 3129, "NBES41g",
		{ { kEv, 132, kTrue }, { kEv, 35, kTrue }, { kEv, 161, kFalse } } },
	{	33, 3130, "NBES43",
		{ { kEv, 123, kTrue }, { kEv, 162, kFalse } } },
	{	34, 3131, "NBES46",
		{ { kEv, 88, kTrue }, { kEv, 163, kFalse } } },
	{	35, 3133, "NBES48",
		{ { kEv, 61, kTrue }, { kEv, 166, kFalse }, { kIn, 12, kFalse } } },
	{	36, 3136, "NBES53",
		{ { kEv, 64, kTrue }, { kEv, 168, kFalse } } },
	{	37, 3137, "NBES55",
		{ { kEv, 85, kFalse }, { kEv, 62, kTrue }, { kEv, 169, kFalse } } },
	{	38, 3138, "NBES61g",
		{ { kEv, 64, kTrue }, { kEv, 68, kFalse }, { kEv, 170, kFalse } } },
	{	39, 3139, "NBES65",
		{ { kEv, 42, kTrue }, { kEv, 43, kFalse }, { kEv, 155, kFalse } } },
	{	40, 3140, "NBES66",
		{ { kEv, 83, kTrue }, { kEv, 74, kFalse }, { kEv, 171, kFalse } } },
	{	41, 3141, "NBES67g",
		{ { kEv, 72, kTrue }, { kEv, 172, kFalse } } },
	{	42, 3142, "NBES72",
		{ { kEv, 168, kTrue }, { kEv, 68, kTrue }, { kEv, 173, kFalse } } },
	{	43, 3144, "NBES76",
		{ { kEv, 36, kTrue }, { kEv, 174, kFalse } } },
	{	44, 3145, "NBES79g",
		{ { kEv, 31, kTrue }, { kEv, 175, kFalse } } }
},
{	// George, 6 responses
	{	45, 3207, "NGEO14",
		{ { kEv, 104, kTrue }, { kEv, 177, kFalse }, { kEv, 61, kFalse }, { kIn, 10, kFalse } } },
	{	46, 3209, "NGEO16",
		{ { kEv, 61, kFalse }, { kEv, 179, kFalse }, { kIn, 10, kTrue } } },
	{	47, 3210, "NGEO17",
		{ { kEv, 61, kTrue }, { kEv, 180, kFalse } } },
	{	48, 3213, "NGEO18",
		{ { kEv, 181, kFalse }, { kIn, 4, kTrue } } },
	{	49, 3214, "NGEO19",
		{ { kEv, 41, kTrue }, { kEv, 154, kFalse }, { kEv, 79, kFalse } } },
	{	50, 3215, "NGEO20",
		{ { kEv, 182, kFalse }, { kEv, 73, kFalse }, { kIn, 14, kTrue } } }
},
{	// Security guard, 3 responses + 1 repeat
	{	51, 401, "NG01",
		{ { kEv, 61, kFalse }, { kIn, 10, kFalse }, { kIn, 4, kFalse } } },
	{	52, 403, "NG02",
		{ { kEv, 117, kTrue }, { kIn, 0x4, kFalse } } },
	{	53, 409, "NG04",
		{ { kIn, 10, kTrue } } },
	{	53, 409, "NG04",
		{ { kIn, 4, kTrue } } }
},
{	// Mattie, 2 responses
	{	54, 215, "NMD20",
		{ { kEv, 151, kTrue }, { kEv, 137, kFalse } } },
	{	55, 230, "NMD31",
		{ { kEv, 64, kTrue }, { kEv, 138, kFalse } } }
}
};

const Common::Array<Goodbye> _nancy2Goodbyes = {
	{ "NDA29", { { { 890, 891, 892, 893 }, {}, NOFLAG } } },	// Dwayne
	{ "NRD35", { { { 791, 792, 793, 794 }, {}, NOFLAG } } },	// Rick
	{ "NPR16", { { { 391, 392, 394 }, {}, NOFLAG } } },			// Millie
	{ "NLR18", { { { 590, 591, 593 }, {}, NOFLAG } } },			// Lillian
	{ "NPR16", { { { 3090, 3092, 3093 }, {}, NOFLAG } } },		// Ned
	{ "NBES86", { { { 3190 }, {}, NOFLAG } } },					// Bess
	{ "NGEO90", { { { 3290 }, {}, NOFLAG } } },					// George
	{ "", { { {}, {}, NOFLAG } } },								// Security guard, no goodbye
	{ "NMD32", { { { 290, 291, 292, 293 }, {}, NOFLAG } } },	// Mattie
};

const Common::Array<Common::Array<const char *>> _nancy2ConditionalDialogueTexts {
{	// English
	// 00
	"Have you met the prop master at Worldwide? She seems...rather strange.<h><n>", // nda33
	"I'm afraid that I'm not making a very good impression on Lillian. She doesn't seem to like me very much.<h><n>", // nda34
	"Do you know how I could get into the control room at the studio?<h><n>", // nda35
	"Can I get a pass that let's me get into the studio during the night?<h><n>", // ..nda15
	"I'm afraid I've upset Lillian - did she call about terminating my employment with the studio?<h><n>", // nda19
	// 05
	"Do you have many employees working for you at Worldwide?<h><n>", // nda22
	"What do you make of these threats against Rick? Mattie's very concerned about them.<h><n>", // nda27
	"The producer seems pretty upset lately - he's always yelling.<h><n>", // nda28
	"Tell me Rick, do you know a guy by the name of Owen Spayder?<h><n>", // NRD29
	"What's the story with the prop master?<h><n>", // NRD28
	// 10
	"Can I ask your advice? Dwayne Powers is my agent - he's pretty good isn't he?<h><n>", // NRD18
	"So tell me Rick, who haven't you dated on 'Light Of Our Love'? You've got quite a reputation on the set.<h><n>", // NRD21
	"It must be wonderful to work with Rick Arlen. Is he really that exciting in real life as he is on stage?<h><n>", // NPR12
	"Do you know where I can find Owen Spayder? He's a stage hand, I believe.<h><n>", // NPR15
	"I thought you might be interested to know that I found a light clamp on the set. It looked as if it had been sawed off. That was no accident on the set - it was a deliberate attempt on Rick's life.<h><n>", // NLR07
	// 15
	"Can I ask your advice on something? Rick's really been flirty with me - should I take him seriously?<h><n>", // NLR08
	"Can you tell me something about Owen Spayder?<h><n>", // NLR15
	"Lillian, I have reason to believe you're the one threatening Rick. I know for a fact you sent him those chocolates. <h><n>", // NLR17
	"I finally met the Rick Arlen. That man has an ego the size of Texas - he's worse than Daryl Gray!<h><n>", // NNP08
	"Ned, are you very good at riddles?<h><n>", // NNP11
	// 20
	"What do you think I should look for on that death threat tape?<h><n>", // NNP14
	"There's a locked area of the prop room. I wonder what the prop master is hiding there.<h><n>", // NNP15
	"I got into the locked area of the prop room and found an employee ID for one of Dwayne's contract workers, Owen Spayder.<h><n>", // NNP16
	"Guess what, I got a look at the letters Rick has been getting. Some of them have the letters cut out of magazines and some of them are typewritten.  But get this, the 'Y' is dropped on the typewritten letters.<h><n>", // NNP17
	"This case is getting stranger by the minute. Now I found out that the prop master has a typewriter and guess what? The Y's on her machine are dropped!<h><n>", // NNP28
	// 25
	"I'd like to get into the studio at night, but it's locked. Any ideas?<h><n>", // .NNP20
	"I found a side entrance to the studio, but there's a keypad lock on it.<h><n>", // NNP21
	"I need to get into Lillian's office at night. I think there's more to her than meets the eye.<h><n>", // NBES32g
	"Now I need to find an access code to the system computers.<h><n>", // NBES35
	"If only I could find the password into the control room.<h><n>", // NBES36
	// 30
	"I can't get the employee log to print.<h><n>", // NBES38
	"You'll never guess what I found in Lillian's office. A bottle of castor oil.<h><n>", // NBES39g
	"Lillian must've been the one who sent Rick those threats. I found a bottle of castor oil and the number of a chocolate shop in her drawer.<h><n>", // NBES41g
	"Lillian just kicked me off the set!<h><n>", // NBES43
	"I found the sound mixer but am not sure what I am looking for.<h><n>", // NBES46
	// 35
	"If only I could find a surveillance video.<h><n>", // NBES48
	"I found an employee badge for an Owen Spayder in the lost and found.<h><n>", // NBES53
	"I found Millie's computer login.<h><n>", // NBES55
	"I really need to get into Dwayne's office. I need to find more information on this Owen Spayder guy. Do you think I should sneak into Dwayne's office?<h><n>", // NBES61g
	"I can't get into Dwayne's office.<h><n>", // NBES65
	// 40
	"I can't get into Dwayne's briefcase.<h><n>", // NBES66
	"Dwayne's agency is not doing so well. I found all of these outstanding bills. I also found several checks that Mattie wrote to Dwayne.<h><n>", // NBES67g
	"I found out that Owen Spayder worked at the same theater where Dwayne and Mattie met.<h><n>", // NBES72
	"Oh Bess, this is awful. I just got a phony bomb threat in the mail!<h><n>", // NBES76
	"Lillian just called me. She wants me to meet her at the studio. Do you think I should go?<h><n>", // NBES79g
	// 45
	"Mattie got me a visitor's pass, but I don't see it anywhere.<h><n>", // NGEO14
	"I wonder how I can get into the sound stage.<h><n>", // NGEO16
	"Rick was almost killed by a falling klieg light!<h><n>", // NGEO17
	"I'm officially an extra on the set, but there's not much to do.<h><n>", // NGEO18
	"Are you any good at riddles?<h><n>", // NGEO19
	// 50
	"I found a pair of wire cutters!<h><n>", // NGEO20
	"Yes, Mattie Jensen left a visitor's pass for me. It should be listed under Nancy Drew.<h><n>", // NG01
	"Hi, I've been hired as an extra by the Powers Agency.<h><n>", // NG02
	"Hello...here is my pass.<h><n>", // NG04
	"Did Lillian and Rick date after you both broke up?<h><n>", // NMD20
	// 55
	"Tell me, do you know someone by the name of Owen Spayder?<h><n>" // NMD31
},
{	// Russian
	// 00
	"V= znakom= s zaveduyQej rekvizitom? Kagets*, ona nemnoho... stranna*.<h><n>",
	"Po-moemu, * proizvela ploxoe vpeqatlenie na Lilian.<h><n>",
	"Kak popast% v apparatnuy telestudii?<h><n>",
	"Mogno poluqit% propusk, qtob= zaxodit% v telestudiy noq%y?<h><n>",
	"Lilian rassergena na men*. Ona zvonila po povodu moeho uvol%neni*?<h><n>",
	// 05
	"Skol%ko qelovek rabotaet na studii?<h><n>",
	"Wto v= dumaete ob uhrozax Riku? M&tti oqen% obespokoena po &tomu povodu.<h><n>",
	"Kagets*, prodyser qem-to rasstroen. On posto*nno kriqit.<h><n>",
	"V= znaete qeloveka po imeni Ou&n Sp&jder?<h><n>",
	"Rasskagite, pogalujsta, o zaveduyQej rekvizitom.<h><n>",
	// 10
	"Dilan Pau&rz - moj ahent. Wto v= o nem dumaete?<h><n>",
	"Kagets*, v= vstreqalis% so vsemi aktrisami iz 'Ohn* lybvi'. Zdes% u vas reputaci* Don-Guana.<h><n>",
	"Navernoe, zdorovo rabotat% s Rikom Arlenom? V gizni on takoj ge, kak i na &krane?<h><n>",
	"Hde * mohu najti Ou&na Sp&jdera? Kagets*, on zdes% pomoQnikom rabotaet.<h><n>",
	"Y nawla klemmu ot progektora na s+emoqnoj ploQadke. Po-moemu, ee otpilili. Cto b=l ne nesqastn=j sluqaj - Rika xoteli ubit%.<h><n>",
	// 15
	"Rik so mnoj flirtuet. Kak v= dumaete, stoit prinimat% eho slova vser%ez?<h><n>",
	"Wto v= znaete ob Ou&ne Sp&jdere?<h><n>",
	"Y dumay, &to v= pos=lali Riku zapiski s uhrozami. Korobka s isporqenn=mi konfetami - vawix ruk delo.<h><n>",
	"Y nakonec-to poznakomilas% s Rikom Arlenom. Da, &tot paren% sebe cen= ne slogit.<h><n>",
	"N&d, mne nugna pomoQ% s zahadkami.<h><n>",
	// 20
	"Wto mne delat% s kassetoj iz hrimernoj Rika?<h><n>",
	"V rekvizitorskoj est% zakr=ta* komnata. Y xoqu uznat%, qto tam spr*tano.<h><n>",
	"Mne udalos% popast% v zakr=tuy komnatu rekvizitorskoj. Y nawla tam propusk Ou&na Sp&jdera.<h><n>",
	"Y izuqila pis%ma s uhrozami, kotor=e poluqil Rik. Nekotor=e sostavlen= iz v=rezann=x iz gurnalov bukv, a nekotor=e napeqatan= na mawinke, i na nix 'pr=haet' bukva 'n'!<h><n>",
	"U zaveduyQej rekvizitom est% peqatna* mawinka. I znaew%, qto * v=*snila? Tam 'pr=haet' bukva 'n'!<h><n>",
	// 25
	"Mne nugno proniknut% v telestudiy noq%y. Kak &to sdelat%?<h><n>",
	"Y obnarugila bokovoj vxod v telestudiy, no na dveri kodovoj zamok.<h><n>",
	"Y xoqu osmotret% kabinet Lilian noq%y. Dumay, tam est% mnoho interesnoho.<h><n>",
	"Mne nugno uznat% parol% komp%ytera.<h><n>",
	"Mne nugno uznat% kod dl* vxoda v apparatnuy.<h><n>",
	// 30
	"Mne nugno vvesti nomer propuska rabotnika telestudii.<h><n>",
	"Nikohda ne dohadaew%s*, qto * nawla v ofise Lilian. Kastorovoe maslo!<h><n>",
	"Y dumay, qto &to Lilian pos=lala Riku zapiski s uhrozami. U nee v ofise * nawla kastorovoe maslo i adresa konditerskix.<h><n>",
	"Lilian men* uvolila!<h><n>",
	"Y nawla mikwer, no * ne znay, qto s nim delat%.<h><n>",
	// 35
	"Mne nugno najti zapisi videonablydeni*.<h><n>",
	"Y nawla bedg Ou&na Sp&jdera.<h><n>",
	"Y uznala parol% Milli dl* vxoda v sistemu.<h><n>",
	"Mne nugno popast% v ofis Dilana. Y xoqu najti kakuy-nibud% informaciy ob Ou&ne Sp&jdere.<h><n>",
	"Y ne mohu popast% v ofis Dilana.<h><n>",
	// 40
	"Y ne mohu otkr=t% diplomat Dilana.<h><n>",
	"V ahentstve Dilana dela idut ne oqen% xorowo. Y nawla kuqu neoplaqenn=x sqetov i neskol%ko qekov, kotor=e M&tti v=pisala Pau&rzu.<h><n>",
	"Y v=*snila, qto Sp&jder rabotal v tom teatre, hde vstretilis% Dilan i M&tti.<h><n>",
	"Mne prislali pos=lku s qasami. Y podumala, qto tam bomba. K sqast%y, vse obowlos%.<h><n>",
	"Tol%ko qto mne zvonila Lilian. Ona xoqet vstretit%s* so mnoj v studii.<h><n>",
	// 45
	"M&tti v=pisala mne propusk, no * ne mohu eho najti.<h><n>",
	"Kak mne popast% v pavil%on zvukozapisi?<h><n>",
	"Rika qut% ne ubilo progektorom!<h><n>",
	"Teper% * oficial%no rabotay na telestudii.<h><n>",
	"Pomohi mne razhadat% zahadku.<h><n>",
	// 50
	"Y nawla ploskohubc=!<h><n>",
	"Da. M&tti Dgensen ostavila mne propusk na im* N&nsi Dry.<h><n>",
	"Zdravstvujte. Y nova* rabotnica telestudii ot ahentstva Pau&rza.<h><n>",
	"Zdravstvujte. Vot moj propusk.<h><n>",
	"Lilian vstreqalas% s Rikom posle toho, kak v= razowlis%?<h><n>",
	// 55
	"V= znaete Ou&na Sp&jdera?<h><n>"
}
};

const Common::Array<Common::Array<const char *>> _nancy2GoodbyeTexts = {
{	// English
	"Well, I should get back to the set. Thanks for your help.<h>", // NDA29
	"Listen, I gotta' go, Rick. Be careful, Okay?<h>", // NRD35
	"I should get back to the set. Goodbye!<h>", //  NPR16
	"Well, I'll let you get back to your business.<h>", // NLR18
	"I should get back to the set. Goodbye!<h>", //  NPR16
	"I'll talk to you later. Bye!<h>", // NBES86
	"Talk to you later.<h>", // NGEO90
	"", // No goodbye, empty string
	"Well, I'll see you later, Mattie.<h>", // NMD32
},
{	// Russian
	"Nu, mne pora. Spasibo za pomoQ%.<h>",
	"Rik, mne nugno idti. Bud%te ostorogn=.<h>",
	"Mne pora. Poka!<h>",
	"Ne budu vas zadergivat%.<h>",
	"Mne pora. Poka!<h>",
	"Pohovorim pozge. Poka!<h>",
	"Y pozvony pozge.<h>",
	"", // No goodbye, empty string
	"Do vstreqi, M&tti.<h>"
}
};

const Common::Array<const char *> _nancy2TelephoneRinging = {
	"ringing...<n><e>", // English
	"Hudki...  <n><e>"  // Russian
};

const Common::Array<const char *> _nancy2EmptySaveStrings = {
	"Nothing Saved Here",	// English
	"- - - - -"				// Russian
};

const Common::Array<const char *> _nancy2EventFlagNames = {
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
	"called choco shop",
	"clock threat",
	"death threat",
	"found millie login",
	"heard pappas1",
	"head pappas2",
	"heard riddle",
	"in dwayne bldg",
	"in dwayne office",
	"in lillian office",
	"in lost and found",
	"in night studio",
	"mattie spazz",
	"met dwayne",
	"met lillian",
	"met millie",
	"met pappas",
	"52 - met rick",
	"saw choco num",
	"saw control code",
	"saw control room door",
	"saw cut letters",
	"saw dropped Y on typewriter",
	"saw dropped Y letters",
	"saw employee log",
	"saw grandfather clock",
	"saw klieg cinematic",
	"saw log book",
	"saw memo",
	"saw owen spayder ID",
	"saw the cut c-clamp",
	"saw security log",
	"saw side entrance",
	"saw spayder file",
	"saw trap door box",
	"saw interview tape",
	"saw broken watch",
	"saw checks from mattie",
	"solved bomb puzzle",
	"solved briefcase puzzle",
	"solved clock puzzle",
	"solved control room lock puzzle",
	"solved ripped letter puzzle",
	"solved lever puzzle",
	"solved riddle puzzle",
	"solved side entry puzzle",
	"solved tower puzzle",
	"trap door control light",
	"tried briefcase puzzle",
	"tried clock puzzle",
	"tried computer login",
	"tried control room lock",
	"tried employee log",
	"tried equalizer puzzle",
	"tried letter puzzle",
	"tried lever puzzle",
	"tried security log",
	"ned talked about lost and found",
	"prop master talked about bomb",
	"mattie said chocolates",
	"mattie said watch",
	"mattie said teleprompter",
	"mattie said police",
	"tried tower puzzle",
	"met ned",
	"ned talked about mattie's job",
	"ned told rick",
	"ned told riddle",
	"net told tape",
	"met mattie apartment",
	"saw lock prop area",
	"ned told owen",
	"ned told dropped",
	"ned told prop master  typewriter",
	"ned told side entrance",
	"ned told side entrance keypad",
	"rick told bomb",
	"rick told dwayne",
	"rick said misunderstood",
	"rick told prop master",
	"rick told owen",
	"dwayne told dwayne",
	"dwayne set employee pass",
	"dwayne told prop master",
	"dwayne told lillian",
	"dwayne told control room lock",
	"dwayne told side entrance lock",
	"dwayne told lillian upset",
	"lillian upset",
	"dwayne told owen",
	"dwayne told mattie leaving",
	"dwayne told rick letters",
	"dwayne told pappas",
	"lillian told bomb",
	"lillian told C clamp",
	"lillian told date",
	"lillian talked owen",
	"saw castor oil",
	"prop master told rick",
	"prop master told owen",
	"met mattie studio",
	"mattie told bomb",
	"mattie told about lillian dating rick",
	"mattie told owen",
	"pappas told electric",
	"pappas told studio closed",
	"pappas told video",
	"pappas told key",
	"pappas told night",
	"pappas told teleprompter",
	"pappas told sound",
	"pappas told french",
	"bess told rick",
	"bess told dwayne",
	"bess told millie",
	"bess told mattie",
	"bess told lillian",
	"bess told pappas",
	"entered lillian office",
	"george told riddles",
	"bess told credit card trick",
	"bess told login hiding place",
	"bess told control room password",
	"bess told security log",
	"bess told employee log",
	"bess told castor oil",
	"bess told choco phone number",
	"bess told lillian angry",
	"bess told equalizer",
	"dwayne told bomb",
	"bess told security video",
	"bess told security video hiding place",
	"bess told owen video",
	"bess told owen badge",
	"169 - bess told lillian computer",
	"bess told dwayne office",
	"bess told dwayne briefcase",
	"bess told dwayne checks",
	"bess told owen theater",
	"bess told bomb threat",
	"bess told lillian call",
	"bess told mattie keys",
	"george told visitor pass",
	"george told bess",
	"george told sound stage",
	"george told klieg",
	"george told pass",
	"george told wirecutter",
	"george told taxi",
	"george told mattie",
	"george told lillian",
	"george told dwayne",
	"george told rick",
	"george told bomb",
	"george told memo",
	"george told tower",
	"george told clock hand",
	"george told trap door control key",
	"george told trap door  power",
	"george told owen ID",
	"george told employee report",
	"bomb timer started",
	"197 - bomb ten second mark",
	"bomb screw 1 off",
	"bomb screw 2 off",
	"bomb screw 3 off",
	"bomb screw 4 off",
	"clock hour 1",
	"clock hour 2",
	"clock hour 3",
	"clock hour 4",
	"clock hour 5",
	"clock hour 6",
	"clock hour 7",
	"clock hour 8",
	"clock hour 9",
	"clock hour 10",
	"clock hour 12",
	"lillian told dwayne",
	"mattie said letter",
	"mattie said dwayne",
	"Rick recorder tape started",
	"Saw Surveillance Video",
	"Met George",
	"Met Bess",
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

const Common::Array<const char *> nancy2PatchSrcFiles {
	"S1160.cif",
	"S1563.cif",
	"S1564.cif",
	"S1565.cif"
};

// Patch notes:
// - The patch that extends the final timer was originally distributed by HeR.
// - The softlock fix is custom, and works by adding a second inventory dependency
// to the last two ARs in scene S1160. This allows the player to re-enter the
// prop room when they've collected the door knob, but not the wire clippers.
const Common::Array<PatchAssociation> nancy2PatchAssociations {
	{ { "softlocks_fix", "true" }, { "S1160" } },
	{ { "final_timer", "true" }, { "S1563", "S1564", "S1565" } }
};

#endif // NANCY2DATA
