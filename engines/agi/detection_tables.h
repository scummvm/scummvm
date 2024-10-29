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

#include "common/translation.h"

namespace Agi {

#define GAMEOPTIONS_DEFAULT                   GUIO5(GAMEOPTION_ORIGINAL_SAVELOAD,GAMEOPTION_ENABLE_MOUSE,GAMEOPTION_ENABLE_PREDICTIVE_FOR_MOUSE,GAMEOPTION_USE_HERCULES_FONT,GAMEOPTION_COMMAND_PROMPT_WINDOW)
#define GAMEOPTIONS_DEFAULT_CP                GUIO6(GAMEOPTION_ORIGINAL_SAVELOAD,GAMEOPTION_ENABLE_MOUSE,GAMEOPTION_ENABLE_PREDICTIVE_FOR_MOUSE,GAMEOPTION_USE_HERCULES_FONT,GAMEOPTION_COMMAND_PROMPT_WINDOW,GAMEOPTION_COPY_PROTECTION)
#define GAMEOPTIONS_AMIGA                     GUIO5(GAMEOPTION_ORIGINAL_SAVELOAD,GAMEOPTION_ENABLE_PREDICTIVE_FOR_MOUSE,GAMEOPTION_AMIGA_ALTERNATIVE_PALETTE,GAMEOPTION_USE_HERCULES_FONT,GAMEOPTION_COMMAND_PROMPT_WINDOW)
#define GAMEOPTIONS_AMIGA_CP                  GUIO6(GAMEOPTION_ORIGINAL_SAVELOAD,GAMEOPTION_ENABLE_PREDICTIVE_FOR_MOUSE,GAMEOPTION_AMIGA_ALTERNATIVE_PALETTE,GAMEOPTION_USE_HERCULES_FONT,GAMEOPTION_COMMAND_PROMPT_WINDOW,GAMEOPTION_COPY_PROTECTION)
#define GAMEOPTIONS_APPLE2GS                  GUIO6(GAMEOPTION_ORIGINAL_SAVELOAD,GAMEOPTION_ENABLE_MOUSE,GAMEOPTION_ENABLE_PREDICTIVE_FOR_MOUSE,GAMEOPTION_USE_HERCULES_FONT,GAMEOPTION_COMMAND_PROMPT_WINDOW, GAMEOPTION_APPLE2GS_ADD_SPEED_MENU)
#define GAMEOPTIONS_APPLE2GS_CP               GUIO7(GAMEOPTION_ORIGINAL_SAVELOAD,GAMEOPTION_ENABLE_MOUSE,GAMEOPTION_ENABLE_PREDICTIVE_FOR_MOUSE,GAMEOPTION_USE_HERCULES_FONT,GAMEOPTION_COMMAND_PROMPT_WINDOW, GAMEOPTION_APPLE2GS_ADD_SPEED_MENU, GAMEOPTION_COPY_PROTECTION)
#define GAMEOPTIONS_VGA                       GUIO6(GAMEOPTION_ORIGINAL_SAVELOAD,GAMEOPTION_ENABLE_MOUSE,GAMEOPTION_ENABLE_PREDICTIVE_FOR_MOUSE,GAMEOPTION_USE_HERCULES_FONT,GAMEOPTION_COMMAND_PROMPT_WINDOW,GUIO_RENDERVGA)
#define GAMEOPTIONS_FANMADE_MOUSE             GUIO4(GAMEOPTION_ORIGINAL_SAVELOAD,GAMEOPTION_ENABLE_PREDICTIVE_FOR_MOUSE,GAMEOPTION_USE_HERCULES_FONT,GAMEOPTION_COMMAND_PROMPT_WINDOW)
#define GAMEOPTIONS_FANMADE_MOUSE_VGA         GUIO5(GAMEOPTION_ORIGINAL_SAVELOAD,GAMEOPTION_ENABLE_PREDICTIVE_FOR_MOUSE,GAMEOPTION_USE_HERCULES_FONT,GAMEOPTION_COMMAND_PROMPT_WINDOW,GUIO_RENDERVGA)

#define GAME_LVFPN(id,extra,fname,md5,size,lang,ver,features,gid,platform,interp,guioptions) { \
		{ \
			id, \
			extra, \
			AD_ENTRY1s(fname,md5,size), \
			lang, \
			platform, \
			ADGF_NO_FLAGS, \
			guioptions \
		}, \
		gid, \
		interp, \
		features, \
		ver \
	}

#define GAME_LVFPN2(id,extra,fname_1,md5_1,size_1,fname_2,md5_2,size_2,lang,ver,features,gid,platform,interp,guioptions) { \
		{ \
			id, \
			extra, \
			AD_ENTRY2s(fname_1,md5_1,size_1,fname_2,md5_2,size_2), \
			lang, \
			platform, \
			ADGF_NO_FLAGS, \
			guioptions \
		}, \
		gid, \
		interp, \
		features, \
		ver \
	}

#define GAME_LVFPNF(id,name,fname,md5,size,lang,ver,features,gid,platform,interp,guioptions) { \
		{ \
			id, \
			name, \
			AD_ENTRY1s(fname,md5,size), \
			lang, \
			platform, \
			ADGF_USEEXTRAASTITLE | ADGF_AUTOGENTARGET, \
			guioptions \
		}, \
		gid, \
		interp, \
		features, \
		ver \
	}

#define GAME_LVFPN_FLAGS(id,msg,fname,md5,size,lang,ver,features,gid,platform,interp,guioptions,flags) { \
		{ \
			id, \
			msg, \
			AD_ENTRY1s(fname,md5,size), \
			lang, \
			platform, \
			flags, \
			guioptions \
		}, \
		gid, \
		interp, \
		features, \
		ver \
	}

#define GAME_LVFPN2_FLAGS(id,msg,fname_1,md5_1,size_1,fname_2,md5_2,size_2,lang,ver,features,gid,platform,interp,guioptions,flags) { \
		{ \
			id, \
			msg, \
			AD_ENTRY2s(fname_1,md5_1,size_1,fname_2,md5_2,size_2), \
			lang, \
			platform, \
			flags, \
			guioptions \
		}, \
		gid, \
		interp, \
		features, \
		ver \
	}

#define A2(id,extra,md5,ver,gid) GAME_LVFPN_FLAGS(id,extra,"*",md5,AD_NO_SIZE,Common::EN_ANY,ver,0,gid,Common::kPlatformApple2,GType_A2,GAMEOPTIONS_DEFAULT,ADGF_UNSTABLE)
#define A2_CP(id,extra,md5,ver,gid) GAME_LVFPN_FLAGS(id,extra,"*",md5,AD_NO_SIZE,Common::EN_ANY,ver,0,gid,Common::kPlatformApple2,GType_A2,GAMEOPTIONS_DEFAULT_CP,ADGF_UNSTABLE)
#define BOOTER(id,extra,md5,ver,gid) GAME_LVFPN(id,extra,"*",md5,AD_NO_SIZE,Common::EN_ANY,ver,0,gid,Common::kPlatformDOS,GType_V1,GAMEOPTIONS_DEFAULT)
#define BOOTER_UNSTABLE(id,extra,md5,ver,gid) GAME_LVFPN_FLAGS(id,extra,"*",md5,AD_NO_SIZE,Common::EN_ANY,ver,0,gid,Common::kPlatformDOS,GType_V1,GAMEOPTIONS_DEFAULT,ADGF_UNSTABLE)
#define BOOTER_UNSUPPORTED(id,msg,fname,md5,size,ver,gid) GAME_LVFPN_FLAGS(id,msg,fname,md5,size,Common::EN_ANY,ver,0,gid,Common::kPlatformDOS,GType_V1,GAMEOPTIONS_DEFAULT,ADGF_UNSUPPORTED)
#define GAME(id,extra,md5,ver,gid) GAME_LVFPN(id,extra,"logdir",md5,AD_NO_SIZE,Common::EN_ANY,ver,0,gid,Common::kPlatformDOS,GType_V2,GAMEOPTIONS_DEFAULT)
#define GAME3(id,extra,fname,md5,ver,gid) GAME_LVFPN(id,extra,fname,md5,AD_NO_SIZE,Common::EN_ANY,ver,0,gid,Common::kPlatformDOS,GType_V3,GAMEOPTIONS_DEFAULT)
#define GAME3_CP(id,extra,fname,md5,ver,gid) GAME_LVFPN(id,extra,fname,md5,AD_NO_SIZE,Common::EN_ANY,ver,0,gid,Common::kPlatformDOS,GType_V3,GAMEOPTIONS_DEFAULT_CP)
#define GAME3_PIRATED(id,extra,fname,md5,ver,gid) GAME_LVFPN_FLAGS(id,extra,fname,md5,AD_NO_SIZE,Common::EN_ANY,ver,0,gid,Common::kPlatformDOS,GType_V3,GAMEOPTIONS_DEFAULT,ADGF_PIRATED)

#define GAME_P(id,extra,md5,ver,gid,platform) GAME_LVFPN(id,extra,"logdir",md5,AD_NO_SIZE,Common::EN_ANY,ver,0,gid,platform,GType_V2,GAMEOPTIONS_DEFAULT)
#define GAME_P_CP(id,extra,md5,ver,gid,platform) GAME_LVFPN(id,extra,"logdir",md5,AD_NO_SIZE,Common::EN_ANY,ver,0,gid,platform,GType_V2,GAMEOPTIONS_DEFAULT_CP)
#define GAME_PO(id,extra,md5,ver,gid,platform,guioptions) GAME_LVFPN(id,extra,"logdir",md5,AD_NO_SIZE,Common::EN_ANY,ver,0,gid,platform,GType_V2,guioptions)

#define GAME_FP(id,extra,md5,ver,flags,gid,platform) GAME_LVFPN(id,extra,"logdir",md5,AD_NO_SIZE,Common::EN_ANY,ver,flags,gid,platform,GType_V2,GAMEOPTIONS_DEFAULT)
#define GAME_FPO(id,extra,md5,ver,flags,gid,platform,guioptions) GAME_LVFPN(id,extra,"logdir",md5,AD_NO_SIZE,Common::EN_ANY,ver,flags,gid,platform,GType_V2,guioptions)
#define GAME_F(id,extra,md5,ver,flags,gid) GAME_FP(id,extra,md5,ver,flags,gid,Common::kPlatformDOS)
#define GAME_FO(id,extra,md5,ver,flags,gid,guioptions) GAME_FPO(id,extra,md5,ver,flags,gid,Common::kPlatformDOS,guioptions)
#define GAME_FSO(id,extra,fname,md5,size,ver,flags,gid,guioptions) GAME_LVFPN(id,extra,fname,md5,size,Common::EN_ANY,ver,flags,gid,Common::kPlatformDOS,GType_V2,guioptions)

#define GAME_PS(id,extra,md5,size,ver,gid,platform) GAME_LVFPN(id,extra,"logdir",md5,size,Common::EN_ANY,ver,0,gid,platform,GType_V2,GAMEOPTIONS_DEFAULT)
#define GAME_PS_CP(id,extra,md5,size,ver,gid,platform) GAME_LVFPN(id,extra,"logdir",md5,size,Common::EN_ANY,ver,0,gid,platform,GType_V2,GAMEOPTIONS_DEFAULT_CP)

#define GAME_LPS(id,extra,md5,size,lang,ver,gid,platform) GAME_LVFPN(id,extra,"logdir",md5,size,lang,ver,0,gid,platform,GType_V2,GAMEOPTIONS_DEFAULT)

#define GAME_LFPS(id,extra,md5,size,lang,ver,flags,gid,platform) GAME_LVFPN(id,extra,"logdir",md5,size,lang,ver,flags,gid,platform,GType_V2,GAMEOPTIONS_DEFAULT)

#define GAME3_P(id,extra,fname,md5,ver,flags,gid,platform) GAME_LVFPN(id,extra,fname,md5,AD_NO_SIZE,Common::EN_ANY,ver,flags,gid,platform,GType_V3,GAMEOPTIONS_DEFAULT)
#define GAME3_P_CP(id,extra,fname,md5,ver,flags,gid,platform) GAME_LVFPN(id,extra,fname,md5,AD_NO_SIZE,Common::EN_ANY,ver,flags,gid,platform,GType_V3,GAMEOPTIONS_DEFAULT_CP)
#define GAME3_PO(id,extra,fname,md5,ver,flags,gid,platform,guioptions) GAME_LVFPN(id,extra,fname,md5,AD_NO_SIZE,Common::EN_ANY,ver,flags,gid,platform,GType_V3,guioptions)

#define GAMEpre_P(id,extra,fname_1,md5_1,size_1,fname_2,md5_2,size_2,ver,gid,platform) GAME_LVFPN2(id,extra,fname_1,md5_1,size_1,fname_2,md5_2,size_2,Common::EN_ANY,ver,0,gid,platform,GType_PreAGI,GAMEOPTIONS_DEFAULT)
#define GAMEpre_PU(id,msg,fname_1,md5_1,size_1,fname_2,md5_2,size_2,ver,gid,platform) GAME_LVFPN2_FLAGS(id,msg,fname_1,md5_1,size_1,fname_2,md5_2,size_2,Common::EN_ANY,ver,0,gid,platform,GType_PreAGI,GAMEOPTIONS_DEFAULT,ADGF_UNSUPPORTED)
#define GAMEpre_PO(id,extra,fname_1,md5_1,size_1,fname_2,md5_2,size_2,ver,gid,platform,guioptions) GAME_LVFPN2(id,extra,fname_1,md5_1,size_1,fname_2,md5_2,size_2,Common::EN_ANY,ver,0,gid,platform,GType_PreAGI,guioptions)
#define GAMEpre_PS(id,extra,fname,md5,size,ver,gid,platform) GAME_LVFPN(id,extra,fname,md5,size,Common::EN_ANY,ver,0,gid,platform,GType_PreAGI,GAMEOPTIONS_DEFAULT)
#define GAMEpre_PSU(id,msg,fname,md5,size,ver,gid,platform) GAME_LVFPN_FLAGS(id,msg,fname,md5,size,Common::EN_ANY,ver,0,gid,platform,GType_PreAGI,GAMEOPTIONS_DEFAULT,ADGF_UNSUPPORTED)

#define GAME3_PS(id,extra,fname,md5,size,ver,flags,gid,platform) GAME_LVFPN(id,extra,fname,md5,size,Common::EN_ANY,ver,flags,gid,platform,GType_V3,GAMEOPTIONS_DEFAULT)
#define GAME3_PSO(id,extra,fname,md5,size,ver,flags,gid,platform,guioptions) GAME_LVFPN(id,extra,fname,md5,size,Common::EN_ANY,ver,flags,gid,platform,GType_V3,guioptions)

#define FANMADE_ILVFO(id,name,md5,lang,ver,features,guioptions) GAME_LVFPNF(id,name,"logdir",md5,AD_NO_SIZE,lang,ver,(GF_FANMADE|features),GID_FANMADE,Common::kPlatformDOS,GType_V2,guioptions)

#define FANMADE_ISVPO(id,name,md5,size,ver,platform,guioptions) GAME_LVFPNF(id,name,"logdir",md5,size,Common::EN_ANY,ver,GF_FANMADE,GID_FANMADE,platform,GType_V2,guioptions)
#define FANMADE_SVP(name,md5,size,ver,platform) FANMADE_ISVPO("agi-fanmade",name,md5,size,ver,platform,GAMEOPTIONS_DEFAULT)

#define FANMADE_LVFO(name,md5,lang,ver,features,guioptions) FANMADE_ILVFO("agi-fanmade",name,md5,lang,ver,features,guioptions)

#define FANMADE_LF(name,md5,lang,features) FANMADE_LVFO(name,md5,lang,0x2917,features,GAMEOPTIONS_DEFAULT)
#define FANMADE_LFO(name,md5,lang,features,guioptions) FANMADE_LVFO(name,md5,lang,0x2917,features,guioptions)
#define FANMADE_IF(id,name,md5,features) FANMADE_ILVF(id,name,md5,Common::EN_ANY,0x2917,features)

#define FANMADE_V(name,md5,ver) FANMADE_LVFO(name,md5,Common::EN_ANY,ver,0,GAMEOPTIONS_DEFAULT)
#define FANMADE_F(name,md5,features) FANMADE_LF(name,md5,Common::EN_ANY,features)
#define FANMADE_FO(name,md5,features,guioptions) FANMADE_LFO(name,md5,Common::EN_ANY,features,guioptions)
#define FANMADE_L(name,md5,lang) FANMADE_LF(name,md5,lang,0)
#define FANMADE_I(id,name,md5) FANMADE_IF(id,name,md5,0)
#define FANMADE_O(name,md5,guioptions) FANMADE_FO(name,md5,0,guioptions)
#define FANMADE_P(name,md5,platform) FANMADE_SVP(name,md5,AD_NO_SIZE,0x2917,platform)
#define FANMADE_S(name,fname,md5,size) GAME_LVFPNF("agi-fanmade",name,fname,md5,size,Common::EN_ANY,0x2917,GF_FANMADE,GID_FANMADE,Common::kPlatformDOS,GType_V2,GAMEOPTIONS_DEFAULT)

#define FANMADE(name,md5) FANMADE_F(name,md5,0)

static const AGIGameDescription gameDescriptions[] = {

	// AGI Demo 1 (PC) 05/87 [AGI 2.425]
	GAME("agidemo", "Demo 1 1987-05-20", "9c4a5b09cc3564bc48b4766e679ea332", 0x2440, GID_AGIDEMO),

	// AGI Demo (Apple II) 1987 (A2 Int. 0.048)
	A2("agidemo", "Demo 1987", "1abef8018f42dc21a59e03f3d227024f", 0x2917, GID_AGIDEMO),

	// AGI Demo 2 (IIgs) 1.0C (Censored)
	GAME_P("agidemo", "Demo 2 1987-11-24 1.0C", "580ffdc569ff158f56fb92761604f70e", 0x2440, GID_AGIDEMO, Common::kPlatformApple2GS),

	// AGI Demo 2 (PC 3.5") 11/87 [AGI 2.915]
	GAME("agidemo", "Demo 2 1987-11-24 3.5\"", "e8ebeb0bbe978172fe166f91f51598c7", 0x2917, GID_AGIDEMO),

	// AGI Demo 2 (PC 5.25") 11/87 [v1] [AGI 2.915]
	GAME("agidemo", "Demo 2 1987-11-24 [version 1] 5.25\"", "852ac303a374df62571642ca1e2d1f0a", 0x2917, GID_AGIDEMO),

	// AGI Demo 2 (PC 5.25") 01/88 [v2] [AGI 2.917]
	GAME("agidemo", "Demo 2 1987-11-25 [version 2] 5.25\"", "1503f02086ea9f388e7e041c039eaa69", 0x2917, GID_AGIDEMO),

	// AGI Demo 2 Tandy
	GAME("agidemo", "Demo 2 Tandy", "94eca021fe7da8f8572c2edcc631bbc6", 0x2917, GID_AGIDEMO),

	// AGI Demo 3 (PC) 09/88 [AGI 3.002.102]
	GAME3("agidemo", "Demo 3 1988-09-13", "dmdir", "289c7a2c881f1d973661e961ced77d74", 0x3149, GID_AGIDEMO),

	// AGI Demo for Kings Quest III and Space Quest I
	GAME("agidemo", "Demo Kings Quest III and Space Quest I", "502e6bf96827b6c4d3e67c9cdccd1033", 0x2272, GID_AGIDEMO),

	// Black Cauldron (PC 5.25" booter) 1.1J [AGI 1.12]
	BOOTER_UNSTABLE("bc", "1.1J 5.25\" Booter", "0f69951170868481acebf831dd743b21", 0x1120, GID_BC),

	// Black Cauldron (PC 5.25" booter) 1.1K [AGI 1.12]
	// This also matches against Tandy version 01.00.00. The game resources are identical
	// except for the copyright/version text on the title screen and three pics.
	BOOTER_UNSTABLE("bc", "1.1K 3.5\" Booter", "297a586027a5eba60219b339ebe53443", 0x1120, GID_BC),

	// Black Cauldron (PC 5.25" booter) 1.1M [AGI 1.12]
	BOOTER_UNSTABLE("bc", "1.1M 3.5\" Booter", "29bc82f2acfd0c7deeb7941cafd745d2", 0x1120, GID_BC),

	// Black Cauldron (Amiga) 2.00 6/14/87
	GAME_PO("bc", "2.00 1987-06-14", "7b01694af21213b4727bb94476f64eb5", 0x2440, GID_BC, Common::kPlatformAmiga, GAMEOPTIONS_AMIGA),

	// Black Cauldron (Apple II) 1.1H [AGI 1.20]
	A2("bc", "1.1H", "80c7d0af6c89bf28ae44d2aa5ca83dc1", 0x1120, GID_BC),

	// Black Cauldron (Apple IIgs) 1.0O 2/24/89 (CE)
	GAME3_PO("bc", "1.0O 1989-02-24 (CE)", "bcdir", "dc09d30b147242692f4f85b9811962db", 0x3149, 0, GID_BC, Common::kPlatformApple2GS, GAMEOPTIONS_APPLE2GS),

	// Black Cauldron (PC) 2.00 6/14/87 [AGI 2.439]
	GAME("bc", "2.00 1987-06-14", "7f598d4712319b09d7bd5b3be10a2e4a", 0x2440, GID_BC),

	// Black Cauldron (Russian)
	GAME_LPS("bc", "",  "b7de782dfdf8ea7dde8064f09804bcf5", 357, Common::RU_RUS, 0x2440, GID_BC, Common::kPlatformDOS),

	// Black Cauldron (PC 5.25") 2.10 11/10/88 [AGI 3.002.098]
	GAME3("bc", "2.10 1988-11-10 5.25\"", "bcdir", "0c5a9acbcc7e51127c34818e75806df6", 0x3149, GID_BC),

	// Black Cauldron (PC 3.5") 2.10 11/10/88 [AGI 3.002.098]
	// BC.QA says "Interpreter: 3.002.097", while AGIDATA.OVL is "3.002.098"
	GAME3("bc", "2.10 1988-11-10 3.5\"", "bcdir", "0de3953c9225009dc91e5b0d1692967b", 0x3149, GID_BC),

	// Black Cauldron (CoCo3 360k) [AGI 2.023]
	// Unofficial port by Guillaume Major
	GAME_PS("bc", "", "51212c54808ade96176f201ae0ac7a6f", 357, 0x2440, GID_BC, Common::kPlatformCoCo3),

	// Black Cauldron (CoCo3 360k) [AGI 2.072]
	// Unofficial port by Guillaume Major
	GAME_PS("bc", "updated", "c4e1937f74e8100cd0152b904434d8b4", 357, 0x2440, GID_BC, Common::kPlatformCoCo3),

	// Donald Duck's Playground (PC 5.25" Booter) 1.0Q 06/09/1986
	BOOTER("ddp", "1.0Q 1986-06-09 5.25\" Booter", "f0f35d60e3e3303480a6bd109d54248d", 0x2001, GID_DDP),

	// Donald Duck's Playground (Amiga) 1.0C
	GAME_PO("ddp", "1.0C 1987-04-27", "550971d196f65190a5c760d2479406ef", 0x2272, GID_DDP, Common::kPlatformAmiga, GAMEOPTIONS_AMIGA),

	// Donald Duck's Playground (ST) 1.0A 8/8/86
	GAME("ddp", "1.0A 1986-08-08", "64388812e25dbd75f7af1103bc348596", 0x2272, GID_DDP),

	{
		// Donald Duck's Playground (CoCo)
		{
			"ddp",
			_s("Early AGI games for Tandy Color Computer are not yet supported."),
			AD_ENTRY1s("ITEM.DAT", "6cc7d630c2771ade7058460a765e0cb9", 4290),
			Common::EN_ANY,
			Common::kPlatformCoCo,
			ADGF_UNSUPPORTED,
			GAMEOPTIONS_DEFAULT
		},
		GID_DDP,
		GType_V1,
		0,
		0x2001 // unknown
	},

	// reported by Filippos (thebluegr) in bugreport #3048
	GAME_PS("ddp", "1.0C 1986-06-09", "550971d196f65190a5c760d2479406ef", 132, 0x2272, GID_DDP, Common::kPlatformDOS),

	// Gold Rush! (Amiga) 1.01 1/13/89 aka 2.05 3/9/89  # 2.316
	GAME3_PSO("goldrush", "1.01 1989-01-13 aka 2.05 1989-03-09", "dirs", "a1d4de3e75c2688c1e2ca2634ffc3bd8", 2399, 0x3149, 0, GID_GOLDRUSH, Common::kPlatformAmiga, GAMEOPTIONS_AMIGA_CP),

	// Gold Rush! (Apple II) 1.0M 11/16/1989 (A2 Int. 0.144)
	A2_CP("goldrush", "1.0M 1989-11-16", "a0bf4d801eaf1af4728ea85d6dedf8a6", 0x3149, GID_GOLDRUSH),

	// Gold Rush! (Apple IIgs) 1.0M 2/28/89 (CE) aka 2.01 12/22/88
	GAME3_PO("goldrush", "1.0M 1989-02-28 (CE) aka 2.01 1988-12-22", "grdir", "3f7b9ce62631434389f85371b11921d6", 0x3149, GF_2GSOLDSOUND, GID_GOLDRUSH, Common::kPlatformApple2GS, GAMEOPTIONS_APPLE2GS_CP),

	// Gold Rush! (ST) 1.01 1/13/89 aka 2.01 12/22/88
	GAME3_P_CP("goldrush", "1.01 1989-01-13 aka 2.01 1988-12-22", "grdir", "4dd4d50480a3d6c206fa227ce8142735", 0x3149, 0, GID_GOLDRUSH, Common::kPlatformAtariST),

	// Gold Rush! (PC 5.25") 2.01 12/22/88 [AGI 3.002.149]
	GAME3_CP("goldrush", "2.01 1988-12-22 5.25\"", "grdir", "db733d199238d4009a9e95f11ece34e9", 0x3149, GID_GOLDRUSH),

	// Gold Rush! (PC 3.5") 2.01 12/22/88 [AGI 3.002.149]
	GAME3_CP("goldrush", "2.01 1988-12-22 3.5\"", "grdir", "6a285235745f69b4b421403659497216", 0x3149, GID_GOLDRUSH),

	// Gold Rush! (PC 3.5") 2.01 12/22/88 [AGI 3.002.149] with fanmade Spanish translation
	GAME_LVFPN("goldrush", "2.01 1988-12-22 3.5\"", "grdir", "64ef3b1949c262b92694381cb36d79b9", 3080, Common::ES_ESP, 0x3149, 0, GID_GOLDRUSH, Common::kPlatformDOS, GType_V2, GAMEOPTIONS_DEFAULT_CP),

	// Gold Rush!  2.01 12/22/88 - pirated copy, according to https://bugs.scummvm.org/ticket/3220
	GAME3_PIRATED("goldrush", "2.01 1988-12-22", "grdir", "3ae052117feb483f01a9017025fbb366", 2399, GID_GOLDRUSH),
	GAME3_PIRATED("goldrush", "2.01 1988-12-22", "grdir", "1ef85c37fcf7224f9731f20f169c8c53", 2399, GID_GOLDRUSH),

	// Gold Rush! (PC 3.5", bought from The Software Farm) 3.0 1998-12-22 [AGI 3.002.149]
	GAME3_CP("goldrush", "3.0 1998-12-22 3.5\"", "grdir", "6882b6090473209da4cd78bb59f78dbe", 0x3149, GID_GOLDRUSH),

	{
		// Gold Rush! (Mac) 2.01 12/22/88 [AGI 3.002.149]
		{
			"goldrush",
			"2.01 1988-12-22",
			AD_ENTRY2s("grdir", "db733d199238d4009a9e95f11ece34e9", 2399,
					   "vol.0", "4b6423d143674d3757ab1b875d25951d", 25070),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GAMEOPTIONS_DEFAULT_CP
		},
		GID_GOLDRUSH,
		GType_V3,
		0,
		0x3149
	},

	// Gold Rush! (CoCo3 720k) [AGI 2.023]
	// Unofficial port by Guillaume Major
	GAME_PS_CP("goldrush", "", "0a41b65efc0cd6c4271e957e6ffbbd8e", 744, 0x2440, GID_GOLDRUSH, Common::kPlatformCoCo3),

	// Gold Rush! (CoCo3 360k/720k) [AGI 2.072]
	// Unofficial port by Guillaume Major
	GAME_PS_CP("goldrush", "updated", "c49bf56bf91e31a4601a604e51ef8bfb", 744, 0x2440, GID_GOLDRUSH, Common::kPlatformCoCo3),

	// King's Quest 1 (DOS) 1.0U [AGI 2.272]
	GAME("kq1", "1.0U 1986", "f3464778c9ae1a9e1fbed566f917b3d7", 0x2272, GID_KQ1),

	// King's Quest 1 (Amiga) 1.0U      # 2.082
	GAME_PO("kq1", "1.0U 1986", "246c695324f1c514aee2b904fa352fad", 0x2440, GID_KQ1, Common::kPlatformAmiga, GAMEOPTIONS_AMIGA),

	// King's Quest 1 (ST) 1.0V
	GAME_P("kq1", "1.0V 1986", "c3a017e556c4b0eece366a4cd9abb657", 0x2272, GID_KQ1, Common::kPlatformAtariST),

	// King's Quest 1 (IIgs) 1.0S-88223
	GAME_PO("kq1", "1.0S 1988-02-23", "f4277aa34b43d37382bc424c81627617", 0x2272, GID_KQ1, Common::kPlatformApple2GS, GAMEOPTIONS_APPLE2GS),

	// King's Quest 1 (Mac) 2.0C 3/26/87
	GAME_P("kq1", "2.0C 1987-03-26", "d4c4739d4ac63f7dbd29255425077d48", 0x2440, GID_KQ1, Common::kPlatformMacintosh),

	// King's Quest 1 (IBM PCjr) 1.00 1502265 5/10/84
	BOOTER_UNSUPPORTED("kq1", "Early King\'s Quest releases are not currently supported.",
		"kq1.img", "127675735f9d2c148738c1e96ea9d2cf", 368640, 0x1120, GID_KQ1),

	// King's Quest 1 (Tandy 1000) 01.01.00 5/24/84
	BOOTER_UNSUPPORTED("kq1", "Early King\'s Quest releases are not currently supported.",
		"kq1.img", "0a22131d0eaf66d955afecfdc83ef9d6", 368640, 0x1120, GID_KQ1),

	// King's Quest 1 (PC 5.25"/3.5") 2.0F [AGI 2.917]
	GAME("kq1", "2.0F 1987-05-05 5.25\"/3.5\"", "10ad66e2ecbd66951534a50aedcd0128", 0x2917, GID_KQ1),

	// King's Quest 1 (CoCo3 360k) [AGI 2.023]
	// Unofficial port by Guillaume Major
	GAME_PS("kq1", "", "10ad66e2ecbd66951534a50aedcd0128", 315, 0x2440, GID_KQ1, Common::kPlatformCoCo3),

	// King's Quest 1 (CoCo3 360k) [AGI 2.023]
	// Unofficial port by Guillaume Major
	GAME_PS("kq1", "fixed", "4c8ef8b5d2f1b6c1a93e456d1f1ffc74", 768, 0x2440, GID_KQ1, Common::kPlatformCoCo3),

	// King's Quest 1 (CoCo3 360k) [AGI 2.072]
	// Unofficial port by Guillaume Major
	GAME_PS("kq1", "updated", "94087178c78933a4af3cd24d1c8dd7b2", 315, 0x2440, GID_KQ1, Common::kPlatformCoCo3),

	// King's Quest 1 (Russian)
	GAME_LPS("kq1", "", "973f5830ed5e1c919354dfbcd5036c53", 315, Common::RU_RUS, 0x2440, GID_KQ1, Common::kPlatformDOS),

	// King's Quest 2 (Apple II) 1.0G [AGI 1.08]
	A2("kq2", "1.0G", "8e8d562e50233c939112c89bba55d249", 0x1120, GID_KQ2),

	// King's Quest 2 (Apple II) 1.0H [AGI 1.10]
	A2("kq2", "1.0H", "3a25cb0a87316f449d559ceb93d349e9", 0x1120, GID_KQ2),

	// King's Quest 2 (IIgs) 2.0A 6/16/88 (CE)
	GAME_PO("kq2", "2.0A 1988-06-16 (CE)", "5203c8b95250a2ecfee93ddb99414753", 0x2917, GID_KQ2, Common::kPlatformApple2GS, GAMEOPTIONS_APPLE2GS),

	// King's Quest 2 (Amiga) 2.0J # 2.107
	GAME_PO("kq2", "2.0J 1987-01-29", "b866f0fab2fad91433a637a828cfa410", 0x2440, GID_KQ2, Common::kPlatformAmiga, GAMEOPTIONS_AMIGA),

	// King's Quest 2 (Mac) 2.0R 3/23/88
	GAME_P("kq2", "2.0R 1988-03-23", "cbdb0083317c8e7cfb7ac35da4bc7fdc", 0x2440, GID_KQ2, Common::kPlatformMacintosh),

	// King's Quest 2 (Tandy 5.25" booter) 01.00.00 [AGI 1.12]
	BOOTER_UNSTABLE("kq2", "01.00.00 5.25\" Booter", "59119ff7a21965c0fb5f001f0d049765", 0x1120, GID_KQ2),

	// King's Quest 2 (PC 5.25" booter) 1.0W [AGI 1.12]
	BOOTER_UNSTABLE("kq2", "1.0W 5.25\" Booter", "0b4172d13b0fb7e5b83244a964e52ece", 0x1120, GID_KQ2),

	// King's Quest 2 (PC 5.25" booter) 1.1H [AGI 1.12]
	BOOTER_UNSTABLE("kq2", "1.1H 5.25\" Booter", "4924e12c90f883b81db426e11e091beb", 0x1120, GID_KQ2),

	// King's Quest 2 (PC) 2.1 [AGI 2.411]; entry from DAGII, but missing from Sarien?
	// XXX: any major differences from 2.411 to 2.440?
	GAME("kq2", "2.1 1987-04-10", "759e39f891a0e1d86dd29d7de485c6ac", 0x2440, GID_KQ2),

	// King's Quest 2 (PC 5.25"/3.5") 2.2 [AGI 2.426]
	GAME("kq2", "2.2 1987-05-07 5.25\"/3.5\"", "b944c4ff18fb8867362dc21cc688a283", 0x2917, GID_KQ2),

	// King's Quest 2 (PC 5.25"/3.5") 2.2 [AGI 2.426] with fanmade Spanish translation
	// TRAC #14604
	GAME_LPS("kq2", "2.2 1987-05-07 5.25\"/3.5\"", "252354b72d7062228cfe1b6f8211c761", 543, Common::ES_ESP, 0x2917, GID_KQ2, Common::kPlatformDOS),

	// King's Quest 2 (Russian)
	GAME_LPS("kq2", "", "35211c574ececebdc723b23e35f99275", 543, Common::RU_RUS, 0x2917, GID_KQ2, Common::kPlatformDOS),

	// King's Quest 2 (CoCo3 360k) [AGI 2.023]
	// Unofficial port by Guillaume Major
	GAME_PS("kq2", "", "b944c4ff18fb8867362dc21cc688a283", 543, 0x2440, GID_KQ2, Common::kPlatformCoCo3),

	// King's Quest 2 (CoCo3 360k) [AGI 2.072]
	// Unofficial port by Guillaume Major
	GAME_PS("kq2", "updated", "f64a606de740a5348f3d125c03e989fe", 543, 0x2440, GID_KQ2, Common::kPlatformCoCo3),

	// King's Quest 2 (CoCo3 360k) [AGI 2.023]
	// Unofficial port by Guillaume Major
	GAME_PS("kq2", "fixed", "fb33ac2768a94a89117a270771db465c", 768, 0x2440, GID_KQ2, Common::kPlatformCoCo3),

	// King's Quest 2 (CoCo3 720k)
	// Unofficial port by Guillaume Major
	GAME_PS("kq2", "", "bdb10876cd4cb20eabd88778c40b4075", 543, 0x2440, GID_KQ2, Common::kPlatformCoCo3),

	// King's Quest 3 (Amiga) 1.01 11/8/86
	GAME_PO("kq3", "1.01 1986-11-08", "8ab343306df0e2d98f136be4e8cfd0ef", 0x2440, GID_KQ3, Common::kPlatformAmiga, GAMEOPTIONS_AMIGA),

	// King's Quest 3 (ST) 1.02 11/18/86
	GAME_P("kq3", "1.02 1986-11-18", "8846df2654302b623217ba8bd6d657a9", 0x2272, GID_KQ3, Common::kPlatformAtariST),

	// King's Quest 3 (Mac) 2.14 3/15/88
	GAME_P("kq3", "2.14 1988-03-15", "7639c0da5ce94848227d409351fabda2", 0x2440, GID_KQ3, Common::kPlatformMacintosh),

	// King's Quest 3 (Apple II) 2.0A 3/13/88 (A2 Int. 0.101)
	A2("kq3", "2.0A 1988-03-13", "6d3982705071a59b65fe0953333074f0", 0x2440, GID_KQ3),

	// King's Quest 3 (IIgs) 2.0A 8/28/88 (CE)
	GAME_PO("kq3", "2.0A 1988-08-28 (CE)", "ac30b7ca5a089b5e642fbcdcbe872c12", 0x2917, GID_KQ3, Common::kPlatformApple2GS, GAMEOPTIONS_APPLE2GS),

	// King's Quest 3 (Amiga) 2.15 11/15/89 # 2.333
	GAME3_PSO("kq3", "2.15 1989-11-15", "dirs", "8e35bded2bc5cf20f5eec2b15523b155", 1805, 0x3149, 0, GID_KQ3, Common::kPlatformAmiga, GAMEOPTIONS_AMIGA),

	// King's Quest 3 (PC) 1.01 11/08/86 [AGI 2.272]
	GAME_P("kq3", "1.01 1986-11-08", "9c2b34e7ffaa89c8e2ecfeb3695d444b", 0x2272, GID_KQ3, Common::kPlatformDOS),

	// King's Quest 3 (Russian)
	GAME_LPS("kq3", "", "5856dec6ccb9c4b70aee21044a19270a", 390, Common::RU_RUS, 0x2272, GID_KQ3, Common::kPlatformDOS),

	// King's Quest 3 (PC 5.25") 2.00 5/25/87 [AGI 2.435]
	GAME("kq3", "2.00 1987-05-25 5.25\"", "18aad8f7acaaff760720c5c6885b6bab", 0x2440, GID_KQ3),

	// King's Quest 3 (PC 5.25") 2.00 5/28/87 [AGI 2.435]
	// Bugreport #10646
	GAME("kq3", "2.00 1987-05-25 5.25\"", "b46dc63d6272fb6ed24a004ad580a033", 0x2440, GID_KQ3),

	// King's Quest 3 (PC 3.5") 2.14 3/15/88 [AGI 2.936]
	GAME("kq3", "2.14 1988-03-15 3.5\"", "d3d17b77b3b3cd13246749231d9473cd", 0x2936, GID_KQ3),

	// King's Quest 3 (PC 3.5") 2.14 3/15/88 [AGI 2.936] "For MCGA"
	GAME("kq3", "2.14 1988-03-15 3.5\" MCGA", "7650e659c7bc0f1e9f8a410b7a2e9de6", 0x2936, GID_KQ3),

	// King's Quest 3 (PC 3.5") 2.14 3/15/88 [AGI 2.936] with fanmade Spanish translation
	// TRAC #13494
	GAME_LPS("kq3", "2.14 1988-03-15 3.5\"", "87956c92d23f53d81bf2ee9e08fdc64c", 390, Common::ES_ESP, 0x2936, GID_KQ3, Common::kPlatformDOS),

	// King's Quest 3 (CoCo3 158k/360k) 1.0C 6/27/88 [AGI 2.023]
	// Official port by Sierra
	GAME_PS("kq3", "", "5a6be7d16b1c742c369ef5cc64fefdd2", 429, 0x2440, GID_KQ3, Common::kPlatformCoCo3),

	// King's Quest 4 (PC 5.25") 2.0 7/27/88 [AGI 3.002.086]
	GAME3("kq4", "2.0 1988-07-27", "kq4dir", "f50f7f997208ca0e35b2650baec43a2d", 0x3086, GID_KQ4),

	// King's Quest 4 (PC 3.5") 2.0 7/27/88 [AGI 3.002.086]
	GAME3("kq4", "2.0 1988-07-27 3.5\"", "kq4dir", "fe44655c42f16c6f81046fdf169b6337", 0x3086, GID_KQ4),

	// King's Quest 4 (PC 3.5") 2.2 9/27/88 [AGI 3.002.086]
	GAME3("kq4", "2.2 1988-09-27 3.5\"", "kq4dir", "7470b3aeb49d867541fc66cc8454fb7d", 0x3086, GID_KQ4),

	// King's Quest 4 (PC 5.25") 2.2 9/27/88 [AGI 3.002.086]
	// TRAC #13734
	GAME3_PS("kq4", "2.2 1988-09-27 5.25\"", "kq4dir", "106219d71140823f6bec1d9747128796", 2786, 0x3086, 0, GID_KQ4, Common::kPlatformDOS),

	// King's Quest 4 (PC 5.25") 2.3 9/27/88 [AGI 3.002.086]
	GAME3("kq4", "2.3 1988-09-27", "kq4dir", "6d7714b8b61466a5f5981242b993498f", 0x3086, GID_KQ4),

	// King's Quest 4 (PC 3.5") 2.3 9/27/88 [AGI 3.002.086]
	GAME3("kq4", "2.3 1988-09-27 3.5\"", "kq4dir", "82a0d39af891042e99ac1bd6e0b29046", 0x3086, GID_KQ4),

	// King's Quest 4 (Apple II) 1.0W 3/16/89 (A2 Int. 0.144)
	A2("kq4", "1.0W 1989-03-16", "e5c6f8f0b5db09b00477012fc57fe775", 0x3086, GID_KQ4),

	// King's Quest 4 (IIgs) 1.0K 11/22/88 (CE)
	GAME3_PO("kq4", "1.0K 1988-11-22", "kq4dir", "8536859331159f15012e35dc82cb154e", 0x3086, 0, GID_KQ4, Common::kPlatformApple2GS, GAMEOPTIONS_APPLE2GS),

	// King's Quest 4 (IIgs) 2.0H 12/20/88 (CE)
	GAME3_PO("kq4", "2.0H 1988-12-20", "kq4dir", "c8ccc3964ee405aa56fd20b16c5afd95", 0x3086, 0, GID_KQ4, Common::kPlatformApple2GS, GAMEOPTIONS_APPLE2GS),

	// King's Quest 4 demo (PC) [AGI 3.002.102]
	GAME3("kq4", "Demo 1988-12-20", "dmdir", "a3332d70170a878469d870b14863d0bf", 0x3149, GID_KQ4),

	// King's Quest 4 (CoCo3 720k) [AGI 2.023]
	// Unofficial port by Guillaume Major
	GAME_PS("kq4", "", "9e7729a28e749ca241d2bf71b9b2dbde", 741, 0x2440, GID_KQ4, Common::kPlatformCoCo3),

	// King's Quest 4 (CoCo3 360k/720k) [AGI 2.072]
	// Unofficial port by Guillaume Major
	GAME_PS("kq4", "updated", "1959ca10739edb34069bb504dbd74805", 741, 0x2440, GID_KQ4, Common::kPlatformCoCo3),

	// King's Quest 4 (CoCo3 360k/720k)
	// Unofficial port by Guillaume Major
	GAME_PS("kq4", "", "ef1c0d2d14fe643929a092621c7459cc", 741, 0x2440, GID_KQ4, Common::kPlatformCoCo3),

	// Leisure Suit Larry 1 (PC 5.25"/3.5") 1.00 6/1/87 [AGI 2.440]
	GAME("lsl1", "1.00 1987-06-01 5.25\"/3.5\"", "1fe764e66857e7f305a5f03ca3f4971d", 0x2440, GID_LSL1),

	// Leisure Suit Larry 1 Polish
	GAME_LPS("lsl1", "2.00 2001-12-11", "7ba1fccc46d27c141e704706c1d0a85f", 303, Common::PL_POL, 0x2440, GID_LSL1, Common::kPlatformDOS),

	// Leisure Suit Larry 1 Polish - Demo
	GAME_LPS("lsl1", "Demo", "3b2f564306c401dff6334441df967ddd", 666, Common::PL_POL, 0x2917, GID_LSL1, Common::kPlatformDOS),

	// Leisure Suit Larry 1 (ST) 1.04 6/18/87
	GAME_P("lsl1", "1.04 1987-06-18", "8b579f8673fe9448c2538f5ed9887cf0", 0x2440, GID_LSL1, Common::kPlatformAtariST),

	// Leisure Suit Larry 1 (Amiga) 1.05 6/26/87    # x.yyy
	GAME_PO("lsl1", "1.05 1987-06-26", "3f5d26d8834ca49c147fb60936869d56", 0x2440, GID_LSL1, Common::kPlatformAmiga, GAMEOPTIONS_AMIGA),

	// Leisure Suit Larry 1 (Apple II) 1.0L (A2 Int. 0.080)
	A2("lsl1", "1.0L", "d1d4204485c2735f343ab54ff609631f", 0x2440, GID_LSL1),

	// Leisure Suit Larry 1 (Apple II) 1.0M (A2 Int. 0.080)
	A2("lsl1", "1.0M", "cf5452e0e36d0c0bd86dea9ad630e001", 0x2440, GID_LSL1),

	// Leisure Suit Larry 1 (IIgs) 1.0E
	GAME_PO("lsl1", "1.0E 1987", "5f9e1dd68d626c6d303131c119582ad4", 0x2440, GID_LSL1, Common::kPlatformApple2GS, GAMEOPTIONS_APPLE2GS),

	// Leisure Suit Larry 1 (Mac) 1.05 6/26/87
	GAME_P("lsl1", "1.05 1987-06-26", "8a0076429890531832f0dc113285e31e", 0x2440, GID_LSL1, Common::kPlatformMacintosh),

	// Leisure Suit Larry 1 (CoCo3 158k/360k) 1.0C [AGI 2.072]
	// Official port by Sierra
	GAME_PS("lsl1", "", "a2de1fe76565c3e8b40c9d036b5e5612", 198, 0x2440, GID_LSL1, Common::kPlatformCoCo3),

	// Manhunter NY (ST) 1.03 10/20/88
	GAME3_P("mh1", "1.03 1988-10-20", "mhdir", "f2d58056ad802452d60776ee920a52a6", 0x3149, 0, GID_MH1, Common::kPlatformAtariST),

	// Manhunter NY (Apple II) 1.0I 4/19/90 (AGI Int. 1.050)
	A2("mh1", "1.0I 1990-04-19", "a197817633e17cec3407ea194da4a372", 0x3149, GID_MH1),

	// Manhunter NY (IIgs) 2.0E 10/05/88 (CE)
	GAME3_P("mh1", "2.0E 1988-10-05 (CE)", "mhdir", "2f1509f76f24e6e7d213f2dadebbf156", 0x3149, 0, GID_MH1, Common::kPlatformApple2GS),

	// Manhunter NY (Amiga) 1.06 3/18/89 # 2.328
	GAME3_PO("mh1", "1.06 1989-03-18", "dirs", "92c6183042d1c2bb76236236a7d7a847", 0x3149, 0, GID_MH1, Common::kPlatformAmiga, GAMEOPTIONS_AMIGA),

	// Manhunter NY (PC 5.25") 1.22 8/31/88 [AGI 3.002.102 or 3.002.107]
	// There are two 5.25" versions with the same resources but different interpreters.
	GAME3_PS("mh1", "1.22 1988-08-31", "mhdir", "0c7b86f05fe02c2e26cff1b07450b82a", 2123, 0x3149, 0, GID_MH1, Common::kPlatformDOS),

	// Manhunter NY (PC 3.5") 1.22 8/31/88 [AGI 3.002.102]
	// About box says 8/30/88, QA file says 8/31/88
	// This version has different resources than the three other PC 1.22 versions.
	GAME3_PS("mh1", "1.22 1988-08-31", "mhdir", "f96e31316eab8fc5b3b7928611354541", 2141, 0x3149, 0, GID_MH1, Common::kPlatformDOS),

	// Manhunter NY (PC 3.5") 1.22 8/31/88 [AGI 3.002.107]
	GAME3_PS("mh1", "1.22 1988-08-31", "mhdir", "5b625329021ad49fd0c1d6f2d6f54bba", 2141, 0x3149, 0, GID_MH1, Common::kPlatformDOS),

	// Manhunter NY (CoCo3 720k) [AGI 2.023]
	// Unofficial port by Guillaume Major
	GAME_PS("mh1", "", "b968285caf2f591c78dd9c9e26ab8974", 495, 0x2440, GID_MH1, Common::kPlatformCoCo3),

	// Manhunter NY (CoCo3 360k/720k) [AGI 2.072]
	// Unofficial port by Guillaume Major
	GAME_PS("mh1", "updated", "d47da950c62289f8d4ccf36af73365f2", 495, 0x2440, GID_MH1, Common::kPlatformCoCo3),

	{
		// Manhunter NY (Mac) 1.22 8/31/88
		{
			"mh1",
			"1.22 1988-08-31",
			AD_ENTRY2s("mhdir",	"0c7b86f05fe02c2e26cff1b07450b82a", 2123,
					   "vol.0", "338d7053d8cf08b517edebad2807975d", 115078),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GAMEOPTIONS_DEFAULT
		},
		GID_MH1,
		GType_V3,
		0,
		0x3149
	},

	// Manhunter SF (ST) 1.0 7/29/89
	GAME3_P("mh2", "1.0 1989-07-29", "mh2dir", "5e3581495708b952fea24438a6c7e040", 0x3149, 0, GID_MH2, Common::kPlatformAtariST),

	// Manhunter SF (Amiga) 3.06 8/17/89        # 2.333
	GAME3_PSO("mh2", "3.06 1989-08-17", "dirs", "b412e8a126368b76696696f7632d4c16", 2573, 0x3149, 0, GID_MH2, Common::kPlatformAmiga, GAMEOPTIONS_AMIGA),

	// Manhunter SF (PC 5.25") 3.02 7/26/89 [AGI 3.002.149]
	GAME3("mh2", "3.02 1989-07-26 5.25\"", "mh2dir", "bbb2c2f88d5740f7437fb7aa6f080b7b", 0x3149, GID_MH2),

	// Manhunter SF (PC 3.5") 3.02 7/26/89 [AGI 3.002.149]
	GAME3("mh2", "3.02 1989-07-26 3.5\"", "mh2dir", "6fb6f0ee2437704c409cf17e081ba152", 0x3149, GID_MH2),

	// Manhunter SF (PC 5.25") 3.03 8/17/89 [AGI 3.002.149]
	GAME3("mh2", "3.03 1989-08-17 5.25\"", "mh2dir", "b90e4795413c43de469a715fb3c1fa93", 0x3149, GID_MH2),

	{
		// Manhunter SF (Mac) 3.03 10/23/89
		{
			"mh2",
			"3.03 1989-10-23",
			AD_ENTRY2s("mh2dir", "b90e4795413c43de469a715fb3c1fa93", 2588,
					   "vol.0", "b174bcf485bc348eae77782f9da4143e", 115338),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GAMEOPTIONS_DEFAULT
		},
		GID_MH1,
		GType_V3,
		0,
		0x3149
	},

	// Manhunter SF (CoCo3 720k) [AGI 2.023]
	// Unofficial port by Guillaume Major
	GAME_PS("mh2", "", "acaaa577e10d1753c5a74f6ae1d858d4", 591, 0x2440, GID_MH2, Common::kPlatformCoCo3),

	// Manhunter SF (CoCo3 720k) [AGI 2.072]
	// Unofficial port by Guillaume Major
	GAME_PS("mh2", "updated", "c64875766700196e72a92359f70f45a9", 591, 0x2440, GID_MH2, Common::kPlatformCoCo3),

	// Mickey's Space Adventure
	// Preagi game
	GAMEpre_P("mickey", "", "1.pic", "b6ec04c91a05df374792872c4d4ce66d", 2489,
							"OBJ/FLASHLIT.OOO", "d60a7b6ff96720037f7e88863f48c5d4", 97, 0x0000, GID_MICKEY, Common::kPlatformDOS),

	// Mixed-Up Mother Goose (Amiga) 1.1 # 2.328
	// Files are timestamped 1986-12-10, but this is a 1989 AGI 3 interpreter.
	GAME3_PSO("mixedup", "1.1", "dirs", "5c1295fe6daaf95831195ba12894dbd9", 2021, 0x3149, 0, GID_MIXEDUP, Common::kPlatformAmiga, GAMEOPTIONS_AMIGA),

	// Mixed Up Mother Goose (Apple II) 1.0I (A2 Int. 0.100)
	A2("mixedup", "1.0I", "ba33c035fa9f9bfb5655f59e677cabed", 0x2917, GID_MIXEDUP),

	// Mixed Up Mother Goose (IIgs)
	GAME_PO("mixedup", "1987", "3541954a7303467c6df87665312ffb6a", 0x2917, GID_MIXEDUP, Common::kPlatformApple2GS, GAMEOPTIONS_APPLE2GS),

	// Mixed-Up Mother Goose (PC) [AGI 2.915]
	GAME("mixedup", "1987-11-10", "e524655abf9b96a3b179ffcd1d0f79af", 0x2917, GID_MIXEDUP),

	// Mixed-Up Mother Goose (Mac)
	GAME_P("mixedup", "", "45bd58c59a744f61a435287f96142f7e", 0x2917, GID_MIXEDUP, Common::kPlatformMacintosh),

	// Mixed-Up Mother Goose (CoCo3 360k) [AGI 2.072]
	// Unofficial port by Guillaume Major
	GAME_PS("mixedup", "", "44e63e9b4d4822a31edea0e8a7e7eac4", 606, 0x2440, GID_MIXEDUP, Common::kPlatformCoCo3),

	// Police Quest 1 (Mac) 2.0G 12/3/87
	GAME_P("pq1", "2.0G 1987-12-03", "805750b66c1c5b88a214e67bfdca17a1", 0x2440, GID_PQ1, Common::kPlatformMacintosh),

	// Police Quest 1 (Apple II) 1.0I 11/23/88 (A2 Int. 0.099)
	A2("pq1", "1.0I 1988-11-23", "581e54c4d89bd53e775482cee9cd3ea0", 0x2440, GID_PQ1),

	// Police Quest 1 (IIgs) 2.0B-88421
	GAME_PO("pq1", "2.0B 1988-04-21", "e7c175918372336461e3811d594f482f", 0x2917, GID_PQ1, Common::kPlatformApple2GS, GAMEOPTIONS_APPLE2GS),

	// Police Quest 1 (Amiga) 2.0B 2/22/89  # 2.310
	GAME3_PSO("pq1", "2.0B 1989-02-22", "dirs", "cfa93e5f2aa7378bddd10ad6746a2ffb", 1613, 0x3149, 0, GID_PQ1, Common::kPlatformAmiga, GAMEOPTIONS_AMIGA),

	// Police Quest 1 (IIgs) 2.0A-88318
	GAME_PO("pq1", "2.0A 1988-03-18", "8994e39d0901de3d07cecfb954075bb5", 0x2917, GID_PQ1, Common::kPlatformApple2GS, GAMEOPTIONS_APPLE2GS),

	// Police Quest 1 (PC) 2.0A 10/23/87 [AGI 2.903/2.911]
	GAME("pq1", "2.0A 1987-10-23", "b9dbb305092851da5e34d6a9f00240b1", 0x2917, GID_PQ1),

	// Police Quest 1 (PC) 2.0E 11/17/87 [AGI 2.915]
	GAME("pq1", "2.0E 1987-11-17", "2fd992a92df6ab0461d5a2cd83c72139", 0x2917, GID_PQ1),

	// Police Quest 1 (PC 720k) 2.0E 11/17/87 [AGI 2.915]
	GAME("pq1", "2.0E 1987-11-17", "e2f94216357d07ecd93d75f1ff4a3338", 0x2917, GID_PQ1),

	// Police Quest 1 2.0G 12/3/87
	GAME("pq1", "2.0G 1987-12-03 5.25\"/ST", "231f3e28170d6e982fc0ced4c98c5c1c", 0x2440, GID_PQ1),

	// Police Quest 1 (PC) 2.0G 12/3/87; entry from DAGII, but missing from Sarien?
	// not sure about disk format -- dsymonds
	GAME("pq1", "2.0G 1987-12-03", "d194e5d88363095f55d5096b8e32fbbb", 0x2917, GID_PQ1),

	// Police Quest 1 (PC) 2.0G 12/3/87; with Hebrew translation
	GAME_LVFPN("pq1", "2.0G 1987-12-03", "PQ1.WAG", "59e1b2fb6d025968b8ed7388f107c7b5", AD_NO_SIZE, Common::HE_ISR, 0x2917, GF_EXTCHAR, GID_PQ1, Common::kPlatformDOS, GType_V2, GAMEOPTIONS_DEFAULT),

	// Police Quest 1 (PC) 2.0G 12/3/87; with Spanish translation (TRAC #14369)
	GAME_LPS("pq1", "2.0G 1987-12-03", "5d151f2f4c4e0675534d49b13529da3f", 360, Common::ES_ESP, 0x2917, GID_PQ1, Common::kPlatformDOS),

	// Police Quest 1 (Russian)
	GAME_LPS("pq1", "", "604cc8041d24c4c7e5fa8baf386ef76e", 360, Common::RU_RUS, 0x2917, GID_PQ1, Common::kPlatformDOS),

	// Police Quest 1 (CoCo3 360k) [AGI 2.023]
	// Unofficial port by Guillaume Major
	GAME_PS("pq1", "", "28a077041f75aab78f66804800940085", 375, 0x2440, GID_PQ1, Common::kPlatformCoCo3),

	// Police Quest 1 (CoCo3 360k) [AGI 2.072]
	// Unofficial port by Guillaume Major
	GAME_PS("pq1", "updated", "63b9a9c6eec154751dd446cd3693e0e2", 768, 0x2440, GID_PQ1, Common::kPlatformCoCo3),

	// Space Quest 1 (ST) 1.1A
	GAME_P("sq1", "1.1A 1986-02-06", "6421fb64b0e6604c9dd065975d9279e9", 0x2440, GID_SQ1, Common::kPlatformAtariST),

	// Space Quest 1 (PC 360k) 1.1A [AGI 2.272]
	GAME_P("sq1", "1.1A 1986-11-13", "8d8c20ab9f4b6e4817698637174a1cb6", 0x2272, GID_SQ1, Common::kPlatformDOS),

	// Space Quest 1 (PC 720k) 1.1A [AGI 2.272]
	GAME_P("sq1", "1.1A 720kb", "0a92b1be7daf3bb98caad3f849868aeb", 0x2272, GID_SQ1, Common::kPlatformDOS),

	// Space Quest 1 (Amiga) 1.2            # 2.082
	GAME_FPO("sq1", "1.2 1986", "0b216d931e95750f1f4837d6a4b821e5", 0x2440, 0, GID_SQ1, Common::kPlatformAmiga, GAMEOPTIONS_AMIGA),

	// Space Quest 1 (Mac) 1.5D
	GAME_P("sq1", "1.5D 1987-04-02", "ce88419aadd073d1c6682d859b3d8aa2", 0x2440, GID_SQ1, Common::kPlatformMacintosh),

	// Space Quest 1 (Apple II) 1.0P (A2 Int. 0.073)
	// also matches:
	// Space Quest 1 (Apple II) 1.0Q (A2 Int. 0.073)
	A2("sq1", "1.0P", "2a738214e1d89bb7f810bcceb32828a0", 0x2272, GID_SQ1),

	// Space Quest 1 (IIgs) 2.2
	GAME_PO("sq1", "2.2 1987", "64b9b3d04c1066d36e6a6e56187a83f7", 0x2917, GID_SQ1, Common::kPlatformApple2GS, GAMEOPTIONS_APPLE2GS),

	// Space Quest 1 (PC) 1.0X [AGI 2.089]
	GAME_P("sq1", "1.0X 1986-09-24", "af93941b6c51460790a9efa0e8cb7122", 0x2089, GID_SQ1, Common::kPlatformDOS),

	// Space Quest 1 (Russian)
	GAME_LPS("sq1", "", "a279eb8ddbdefdb1ea6adc827a1d632a", 372, Common::RU_RUS, 0x2089, GID_SQ1, Common::kPlatformDOS),

	// Space Quest 1 (PC 5.25"/3.5") 2.2 [AGI 2.426/2.917]
	GAME("sq1", "2.2 1987-05-07 5.25\"/3.5\"", "5d67630aba008ec5f7f9a6d0a00582f4", 0x2440, GID_SQ1),

	// Space Quest 1 (PC 5.25"/3.5") 2.2 [AGI 2.917]; French Translation
	GAME_LVFPN("sq1", "2.2 1987-05-07 5.25\"/3.5\"", "words.tok.extended", "3f1730f3c9d4622a986f735af0f8734a", 12665, Common::FR_FRA, 0x2917, GF_EXTCHAR, GID_SQ1, Common::kPlatformDOS, GType_V2, GAMEOPTIONS_DEFAULT),

	// Space Quest 1 (CoCo3 360k) [AGI 2.072]
	// Unofficial port by Guillaume Major
	GAME_PS("sq1", "", "5d67630aba008ec5f7f9a6d0a00582f4", 372, 0x2440, GID_SQ1, Common::kPlatformCoCo3),

	// Space Quest 1 (CoCo3 360k) [AGI 2.023]
	// Unofficial port by Guillaume Major
	GAME_PS("sq1", "fixed", "ca822b768b6462e410423ea7f498daee", 768, 0x2440, GID_SQ1, Common::kPlatformCoCo3),

	// Space Quest 1 (CoCo3 360k) [AGI 2.072]
	// Unofficial port by Guillaume Major
	GAME_PS("sq1", "updated", "7fa54e6bb7ffeb4cf20eca39d86f5fb2", 387, 0x2440, GID_SQ1, Common::kPlatformCoCo3),

	// Space Quest 2 (Apple II) 2.0A (A2 Int. 0.089)
	A2("sq2", "2.0A", "5b15026eee7a3a9e36e645feb026d931", 0x2917, GID_SQ2),

	// Space Quest 2 (Apple II) 2.0F (A2 Int. 0.099)
	A2("sq2", "2.0F", "5ca2c0e49918acb2517742922717201c", 0x2917, GID_SQ2),

	// Space Quest 2 (IIgs) 2.0A 7/25/88 (CE)
	GAME_PO("sq2", "2.0A 1988-07-25 (CE)", "5dfdac98dd3c01fcfb166529f917e911", 0x2936, GID_SQ2, Common::kPlatformApple2GS, GAMEOPTIONS_APPLE2GS),

	{
		// Space Quest 2 (Amiga) 2.0F
		{
			"sq2",
			"2.0F 1986-12-09 [VOL.2->PICTURE.16 broken]",
			AD_ENTRY2s("logdir", "28add5125484302d213911df60d2aded", 426,
					   "object", "5dc52be721257719f4b311a84ce22b16", 372),
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GAMEOPTIONS_AMIGA
		},
		GID_SQ2,
		GType_V2,
		0,
		0x2936
	},

	// Space Quest 2 (Mac) 2.0D
	GAME_P("sq2", "2.0D 1988-04-04", "bfbebe0b59d83f931f2e1c62ce9484a7", 0x2936, GID_SQ2, Common::kPlatformMacintosh),

	// reported by Filippos (thebluegr) in bugreport #3048
	// Space Quest 2 (PC 5.25") 2.0A [AGI 2.912]
	GAME_PS("sq2", "2.0A 1987-11-06 5.25\"", "ad7ce8f800581ecc536f3e8021d7a74d", 423, 0x2917, GID_SQ2, Common::kPlatformDOS),

	// Space Quest 2 (PC 3.5") 2.0A [AGI 2.912]
	GAME_PS("sq2", "2.0A 1987-11-06 3.5\"", "6c25e33d23b8bed42a5c7fa63d588e5c", 423, 0x2917, GID_SQ2, Common::kPlatformDOS),

	// Space Quest 2 (PC 5.25"/ST) 2.0C/A [AGI 2.915]
	GAME("sq2", "2.0C/A 5.25\"/ST", "bd71fe54869e86945041700f1804a651", 0x2917, GID_SQ2),

	// Space Quest 2 (PC 3.5") 2.0C [AGI 2.917]
	// TRAC #14287
	GAME_PS("sq2", "2.0C 3.5\"", "b394eaae1dfe4203378e02641178959c", 426, 0x2917, GID_SQ2, Common::kPlatformDOS),

	// Space Quest 2 (PC 5.25") 2.0C [AGI 2.917]
	// TRAC #14286
	GAME_PS("sq2", "2.0C 5.25\"", "7d29fccae8765ae298cfe720a6d771b1", 426, 0x2917, GID_SQ2, Common::kPlatformDOS),

	// Space Quest 2 (PC 3.5") 2.0D [AGI 2.936]
	GAME("sq2", "2.0D 1988-03-14 3.5\"", "85390bde8958c39830e1adbe9fff87f3", 0x2936, GID_SQ2),

	// Space Quest 2 (PC 5.25") 2.0D [AGI 2.936]
	// TRAC #13274
	GAME_PS("sq2", "2.0D 1988-03-14 5.25\"", "0884ea4e127e333986568775fb21600b", 426, 0x2936, GID_SQ2, Common::kPlatformDOS),

	// Space Quest 2 (PC 3.5") 2.0F [AGI 2.936]
	GAME("sq2", "2.0F 1989-01-05 3.5\"", "28add5125484302d213911df60d2aded", 0x2936, GID_SQ2),

	// Space Quest 2 (PC 5.25") 2.0F [AGI 2.936]
	GAME("sq2", "2.0F 1989-01-05 5.25\"", "bb5a44d0bea416f2cd4c3385eaa21af4", 0x2936, GID_SQ2),

	// Space Quest 2 (Spanish)
	// reported by RadG (radg123) in bug report #5617
	GAME_LPS("sq2", "", "1ae7640dd4d253c3ac2d708d61a35379", 426, Common::ES_ESP, 0x2917, GID_SQ2, Common::kPlatformDOS),

	// Space Quest 2 (Russian)
	GAME_LPS("sq2", "", "ba21c8934caf28e3ba45ce7d1cd6b041", 423, Common::RU_RUS, 0x2917, GID_SQ2, Common::kPlatformDOS),

	// Space Quest 2 (CoCo3 360k) [AGI 2.023]
	// Unofficial port by Guillaume Major
	GAME_PS("sq2", "", "12973d39b892dc9d280257fd271e9597", 768, 0x2440, GID_SQ2, Common::kPlatformCoCo3),

	// Space Quest 2 (CoCo3 360k) [AGI 2.072]
	// Unofficial port by Guillaume Major
	GAME_PS("sq2", "updated", "d24f19b047e65e1763eff4b46f3d50df", 768, 0x2440, GID_SQ2, Common::kPlatformCoCo3),

	// Troll's Tale (PC Booter)
	GAMEpre_PS("troll", "", "troll.img", "62903f264b3d849be4214b3a5c42a2fa", 184320, 0x0000, GID_TROLL, Common::kPlatformDOS),

	// Troll's Tale (PC Booter)
	GAMEpre_PS("troll", "", "troll.img", "2b7f6da1b1008bfb22750e6ab7359942", 184320, 0x0000, GID_TROLL, Common::kPlatformDOS),

	// Troll's Tale (DOS)
	GAMEpre_PSU("troll", "MS-DOS versions of Troll's Tale are not currently supported.",
		"game.dat", "62903f264b3d849be4214b3a5c42a2fa", 184320, 0x0000, GID_TROLL, Common::kPlatformDOS),

	// Troll's Tale (DOS)
	GAMEpre_PSU("troll", "MS-DOS versions of Troll's Tale are not currently supported.",
		"troll.exe", "9cd0dda6e2e7bc78a053f881f3a3176b", 59120, 0x0000, GID_TROLL, Common::kPlatformDOS),

	// Winnie the Pooh in the Hundred Acre Wood
	GAMEpre_P("winnie", "", "title.pic", "2e7900c1ccaa7671d65405f6d1efed30", 1334,
							"obj.40",    "188636a4a340fcfd8e7fab1e8e0ac53b", 657, 0x0000, GID_WINNIE, Common::kPlatformDOS),

	// Winnie the Pooh in the Hundred Acre Wood (Amiga)
	GAMEpre_PO("winnie", "", "misc/title",    "2e7900c1ccaa7671d65405f6d1efed30", 1334,
							 "rooms/room.62", "d56297aa9a1714dc654034f8db9f05c9", 1319, 0x0000, GID_WINNIE, Common::kPlatformAmiga, GAMEOPTIONS_AMIGA),

	// Winnie the Pooh in the Hundred Acre Wood (C64)
	GAMEpre_P("winnie", "", "title.pic", "d4eb97cffc866110f71e1ec9f84fe643", 4097,
							"room62",    "f0e1c2de0ac257dfaa905eaa6de0ca8b", 1378, 0x0000, GID_WINNIE, Common::kPlatformC64),

	// Winnie the Pooh in the Hundred Acre Wood (Apple ][)
	GAMEpre_P("winnie", "", "title.pic",  "45e06010a3c61d78f4661103c901ae11", 8190,
							"room62.obj", "d0711928e7a3ccfa4850fade052c6761", 1313, 0x0000, GID_WINNIE, Common::kPlatformApple2),

	// Winnie the Pooh in the Hundred Acre Wood (CoCo)
	GAMEpre_PU("winnie", "Early AGI games for Tandy Color Computer are not yet supported.",
		"wintitle.pic", "cc8d2ae52e18700843f466a23d62c773", 6144,
		"room62",		"813de04fd57063a402272c603be1188a", 1338, 0x0000, GID_WINNIE, Common::kPlatformCoCo),

	// Xmas Card 1986 (PC) [AGI 2.272]
	GAME("xmascard", "1986-11-13 [version 1]", "3067b8d5957e2861e069c3c0011bd43d", 0x2272, GID_XMASCARD),

	// Xmas Card 1986 (CoCo3 360k) [AGI 2.072]
	// Appears to be an unofficial port
	GAME_PS("xmascard", "", "25ad35e9628fc77e5e0dd35852a272b6", 768, 0x2440, GID_XMASCARD, Common::kPlatformCoCo3),

	FANMADE_FO("2 Player Demo", "4279f46b3cebd855132496476b1d2cca", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),
	FANMADE("Advanced Epic Fighting", "6454e8c82a7351c8eef5927ad306af4f"),
	FANMADE("AGI Combat", "0be6a8a9e19203dcca0067d280798871"),
	FANMADE("AGI Combat (Beta)", "341a47d07be8490a488d0c709578dd10"),
	FANMADE("AGI Contest 1 Template", "d879aed25da6fc655564b29567358ae2"),
	FANMADE("AGI Contest 2 Template", "5a2fb2894207eff36c72f5c1b08bcc07"),
	FANMADE_O("AGI Mouse Demo 0.60 demo 1", "c07e2519de674c67386cb2cc6f2e3904", GAMEOPTIONS_FANMADE_MOUSE),
	FANMADE_O("AGI Mouse Demo 0.60 demo 2", "cc49d8b88ed6faf4f53ce92c84e0fe1b", GAMEOPTIONS_FANMADE_MOUSE),
	FANMADE_O("AGI Mouse Demo 0.70", "3497c291e4afb6f758e61740678a2aec", GAMEOPTIONS_FANMADE_MOUSE),
	FANMADE_FO("AGI Mouse Demo 1.00", "20397f0bf0ef936f416bb321fb768fc7", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),
	FANMADE_FO("AGI Mouse Demo 1.10", "f4ad396b496d6167635ad0b410312ab8", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),	// AGIPAL
	FANMADE("AGI Piano (v1.0)", "8778b3d89eb93c1d50a70ef06ef10310"),
	FANMADE("AGI Quest (v1.46-TJ0)", "1cf1a5307c1a0a405f5039354f679814"),
	GAME("tetris", "", "7a874e2db2162e7a4ce31c9130248d8a", 0x2917, GID_FANMADE),
	FANMADE("AGI Tetris (1998)", "1afcbc25bfafded2d5fb82de9da0bd9a"),
	FANMADE_V("AGI Trek (Demo)", "c02882b8a8245b629c91caf7eb78eafe", 0x2440),
	FANMADE_FO("AGI256 Demo", "79261ac143b2e2773b2753674733b0d5", GF_AGI256, GAMEOPTIONS_VGA),
	FANMADE_FO("AGI256-2 Demo", "3cad9b3aff1467cebf0c5c5b110985c5", GF_AGI256, GAMEOPTIONS_VGA),
	FANMADE_F("Abrah: L'orphelin de l'espace (v1.2)", "b7b6d1539e14d5a26fa3088288e1badc", Common::FR_FRA),	// AGIPAL
	FANMADE("Acidopolis", "7017db1a4b726d0d59e65e9020f7d9f7"),
	FANMADE("Agent 0055 (v1.0)", "c2b34a0c77acb05482781dda32895f24"),
	FANMADE("Agent 06 vs. The Super Nazi", "136f89ca9f117c617e88a85119777529"),
	FANMADE("Agent Quest", "59e49e8f72058a33c00d60ee1097e631"),
	FANMADE("Al Pond - On Holiday (v1.0)", "a84975496b42d485920e886e92eed68b"),
	FANMADE("Al Pond - On Holiday (v1.1)", "7c95ac4689d0c3bfec61e935f3093634"),
	FANMADE("Al Pond - On Holiday (v1.3)", "8f30c260de9e1dd3d8b8f89cc19d2633"),
	FANMADE("Al Pond 1 - Al Lives Forever (v1.0)", "e8921c3043b749b056ff51f56d1b451b"),
	FANMADE("Al Pond 1 - Al Lives Forever (v1.3)", "fb4699474054962e0dbfb4cf12ca52f6"),
	FANMADE("Apocalyptic Quest (v0.03 Teaser)", "42ced528b67965d3bc3b52c635f94a57"),
	FANMADE("Apocalyptic Quest Demo 2003-06-24", "c68c49a37eaac73e5aa80ce7f05bbd72"),
	FANMADE("Apocalyptic Quest 4.00 Alpha 2", "30c74d194840abc3fb1341b567743ac3"),
	FANMADE_FO("Apocalyptic Quest (v4.00 Alpha 1)", "e15581628d84949b8d352d224ec3184b", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),
	FANMADE_FO("Apocalyptic Quest (v4.00 Alpha 2)", "0eee850005860e46345b38fea093d194", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),
	FANMADE_FO("Band Quest (Demo)", "7326abefd793571cc17ed0db647bdf34", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),
	FANMADE_FO("Band Quest (Early Demo)", "de4758dd34676b248c8301b32d93bc6f", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),
	FANMADE("Beyond the Titanic 2", "9b8de38dc64ffb3f52b7877ea3ebcef9"),
	FANMADE("Biri Quest 1", "1b08f34f2c43e626c775c9d6649e2f17"),
	FANMADE("Bob The Farmboy", "e4b7df9d0830addee5af946d380e66d7"),
	FANMADE_FO("Boring Man 1: The Toad to Robinland", "d74481cbd227f67ace37ce6a5493039f", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),
	FANMADE_FO("Boring Man 2: Ho Man! This Game Sucks!", "250032ba105bdf7c1bc4fed767c2d37e", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),
	FANMADE("Botz", "a8fabe4e807adfe5ec02bfec6d983695"),
	FANMADE("Brian's Quest (v1.0)", "0964aa79b9cdcff7f33a12b1d7e04b9c"),
	FANMADE("CPU-21 (v1.0)", "35b7cdb4d17e890e4c52018d96e9cbf4"),
	GAME("caitlyn", "Demo", "5b8a3cdb2fc05469f8119d49f50fbe98", 0x2917, GID_FANMADE),
	GAME("caitlyn", "", "818469c484cae6dad6f0e9a353f68bf8", 0x2917, GID_FANMADE),
	FANMADE("Car Driver (v1.1)", "2311611d2d36d20ccc9da806e6cba157"),
	FANMADE("Cloak of Darkness (v1.0)", "5ba6e18bf0b53be10db8f2f3831ee3e5"),
	FANMADE("Coco Coq (English) - Coco Coq In Grostesteing's Base (v.1.0.3)", "97631f8e710544a58bd6da9e780f9320"),
	FANMADE_L("Coco Coq (French) - Coco Coq Dans la Base de Grostesteing (v1.0.2)", "ef579ebccfe5e356f9a557eb3b2d8649", Common::FR_FRA),
	FANMADE("Corby's Murder Mystery (v1.0)", "4ebe62ac24c5a8c7b7898c8eb070efe5"),
	FANMADE_FO("DG: The AGIMouse Adventure (English v1.1)", "efe453b92bc1487ea69fbebede4d5f26", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),	// AGIPAL
	FANMADE_LFO("DG: The AGIMouse Adventure (French v1.1)", "eb3d17ca466d672cbb95947e8d6e846a", Common::FR_FRA, GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),	// AGIPAL
	FANMADE("DG: The Adventure Game (v1.1)", "0d6376d493fa7a21ec4da1a063e12b25"),
	FANMADE_L("DG: The Adventure Game (v1.1)", "258bdb3bb8e61c92b71f2f456cc69e23", Common::FR_FRA),
	FANMADE("Dashiki (16 Colors)", "9b2c7b9b0283ab9f12bedc0cb6770a07"),
	FANMADE_FO("Dashiki (256 Colors)", "c68052bb209e23b39b55ff3d759958e6", GF_AGIMOUSE|GF_AGI256, GAMEOPTIONS_FANMADE_MOUSE_VGA),
	FANMADE("Date Quest 1 (v1.0)", "ba3dcb2600645be53a13170aa1a12e69"),
	FANMADE("Date Quest 2 (v1.0 Demo)", "1602d6a2874856e928d9a8c8d2d166e9"),
	FANMADE("Date Quest 2 (v1.0)", "f13f6fc85aa3e6e02b0c20408fb63b47"),
	FANMADE("Dave's Quest (v0.07)", "f29c3660de37bacc1d23547a167f27c9"),
	FANMADE("Dave's Quest (v0.17)", "da3772624cc4a86f7137db812f6d7c39"),
	FANMADE("Disco Nights (Demo)", "dc5a2b21182ba38bdcd992a3a978e690"),
	FANMADE("Dogs Quest - The Quest for the Golden Bone (v1.0)", "f197357edaaea0ff70880602d2f09b3e"),
	FANMADE("Dr. Jummybummy's Space Adventure", "988bd81785f8a452440a2a8ac67f96aa"),
	FANMADE("Ed Ward", "98be839b9f30cbedea4c9cee5442d827"),
	FANMADE("Elfintard", "c3b847e9e9e978af9708df76a0751dc2"),
	FANMADE("Enclosure (v1.01)", "f08e66fee9ecdde77db7ee9a10c96ba2"),
	FANMADE("Enclosure (v1.03)", "e4a0613ed02401502e506ba3565a8c40"),
	FANMADE("Enclosure (v1.5)", "3aca812211f32a802cb2d7b81d8cba65"),
	FANMADE_SVP("Enclosure", "fe98e6126db74c6cc6fd8fe395cc6e8c", 345, 0x2440, Common::kPlatformCoCo3),
	FANMADE("Epic Fighting (v0.1)", "aff24a1b3bdd676187685c4d95ba4294"),
	FANMADE("Escape Quest (v0.0.3)", "2346b65619b1da0298b715b06d1a45a1"),
	FANMADE("Escape from the Desert (beta 1)", "dfdc634d340854bd6ece28024010758d"),
	FANMADE("Escape from the Salesman", "e723ca4fe0f6f56affe039fbb4dbeb6c"),
	FANMADE("Flag Quest", "ca062c27e6f932661cc06ed986663861"),
	FANMADE_S("Fu$k Quest 1", "vol.0", "e62f1de7732b354fa4ff813d3a8dda65", 96856),
	FANMADE_S("Fu$k Quest 1 (final)", "vol.0", "e62f1de7732b354fa4ff813d3a8dda65", 97127),
	FANMADE("Fu$k Quest 2 - Romancing the Bone (Teaser)", "d288355d71d9bb1639260ccaa3b2fbfe"),
	FANMADE("Fu$k Quest 2 - Romancing the Bone", "294beeb7765c7ea6b05ed7b9bf7bff4f"),
	FANMADE("Fro Quest", "17c4e9dd6a7b6cdf2e7a799fa294bf8e"),
	FANMADE("Gennadi Tahab Autot - Mission Pack 1 - Kuressaare", "bfa5fe71978e6ccf3d4eedd430124015"),

	GAME_LVFPNF("agi-fanmade", "Get Outta Space Quest", "logdir", "aaea5b4a348acb669d13b0e6f22d4dc9", AD_NO_SIZE,
	            Common::EN_ANY, 0x2440, GF_FANMADE, GID_GETOUTTASQ, Common::kPlatformDOS, GType_V2, GAMEOPTIONS_DEFAULT),

	FANMADE("Go West, Young Hippie", "ff31484ea465441cb5f3a0f8e956b716"),
	FANMADE("Good Man (demo v3.41)", "3facd8a8f856b7b6e0f6c3200274d88c"),
	FANMADE("Good Man (demo v4.0)", "d36f5d98cfcfd28cf7d4103906c59a77"),
	FANMADE("Good Man (demo v4.0T)", "8184f70a5a33d4f407dfc8e9ddab99e9"),

	GAME_LVFPNF("agi-fanmade", "Groza [AGDS sample]", "logdir", "421da3a18004122a966d64ab6bd86d2e", AD_NO_SIZE,
	            Common::RU_RUS, 0x2440, GF_AGDS, GID_FANMADE, Common::kPlatformDOS, GType_V2, GAMEOPTIONS_DEFAULT), // Гроза

	FANMADE_FO("Half-Death - Terror At White-Mesa", "b62c05d0ace878261392073f57ae788c", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),
	FANMADE("Hank's Quest (v1.0 English) - Victim of Society", "64c15b3d0483d17888129100dc5af213"),
	FANMADE("Hank's Quest (v1.1 English) - Victim of Society", "86d1f1dd9b0c4858d096e2a60cca8a14"),
	FANMADE_L("Hank's Quest (v1.81 Dutch) - Slachtoffer Van Het Gebeuren", "41e53972d55ff3dff9e90d15fe1b659f", Common::NL_NLD),
	FANMADE("Hank's Quest (v1.81 English) - Victim of Society", "7a776383282f62a57c3a960dafca62d1"),
	FANMADE("Herbao (v0.2)", "6a5186fc8383a9060517403e85214fc2"),
	FANMADE_FO("Hitler's Legacy (v.0004q)", "a412881269ba34584bd0a3268e5a9863", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),
	FANMADE("Hobbits", "4a1c1ef3a7901baf0ab45fde0cfadd89"),
	FANMADE_FO("Isabella Coq - A Present For My Dad", "55c6819f2330c4d5d6459874c9f123d9", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),
	FANMADE("Jack & Julia - VAMPYR", "8aa0b9a26f8d5a4421067ab8cc3706f6"),
	FANMADE("Jeff's Quest (v.5 alpha Jun 1)", "10f1720eed40c12b02a0f32df3e72ded"),
	FANMADE("Jeff's Quest (v.5 alpha May 31)", "51ff71c0ed90db4e987a488ed3bf0551"),
	FANMADE("Jen's Quest (Demo 1)", "361afb5bdb6160213a1857245e711939"),
	FANMADE("Jen's Quest (Demo 2)", "3c321eee33013b289ab8775449df7df2"),
	FANMADE("Jiggy Jiggy Uh! Uh!", "bc331588a71e7a1c8840f6cc9b9487e4"),
	FANMADE("Jimmy In: The Alien Attack (v0.1)", "a4e9db0564a494728de7873684a4307c"),
	FANMADE("Joe McMuffin In \"What's Cooking, Doc\" (v1.0)", "8a3de7e61a99cb605fa6d233dd91c8e1"),
	FANMADE_LVFO("Jolimie, le Village Maudit (v0.5)", "21818501636b3cb8ad5de5c1a66de5c2", Common::FR_FRA, 0x2936, GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),	// AGIPAL
	FANMADE_LVFO("Jolimie, le Village Maudit (v1.1)", "68d7aef1161bb5972fe03efdf29ccb7f", Common::FR_FRA, 0x2936, GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),	// AGIPAL
	FANMADE("Journey Of Chef", "aa0a0b5a6364801ae65fdb96d6741df5"),
	FANMADE("Jukebox (v1.0)", "c4b9c5528cc67f6ba777033830de7751"),
	FANMADE("Justin Quest (v1.0 in development)", "103050989da7e0ffdc1c5e1793a4e1ec"),
	FANMADE_L("Jõulumaa (v0.05)", "53982ecbfb907e41392b3961ad1c3475", Common::ET_EST),
	FANMADE("Kings Quest 2  - Breast Intentions (v2.0 Mar 26)", "a25d7379d281b1b296d4785df90a8e78"),
	FANMADE("Kings Quest 2  - Breast Intentions (v2.0 Aug 16)", "6b4f796d0421d2e12e501b511962e03a"),
	FANMADE("Lasse Holm: The Quest for Revenge (v1.0)", "f9fbcc8a4ef510bfbb92423296ff4abb"),
	FANMADE("Lawman for Hire", "c78b28bfd3767dd455b992cd8b7854fa"),
	FANMADE("Lefty Goes on Vacation (Not in The Right Place)", "ccdc49a33870310b01f2c48b8a1f3c34"),
	FANMADE("Les Inseparables (v1.0)", "4b780887cab0ecabc5eca319acb3acf2"),
	FANMADE("Little Pirate (Demo 2 v0.6)", "437068efe4ec32d436da09d6f2ea56e1"),
	FANMADE("Lost Eternity (v1.0)", "95f15c5632feb8a39e9ca3d9af35fcc9"),
	FANMADE("MD Quest - The Search for Michiel (v0.10)", "2a6fcb21d2b5e4144c38ed817fabe8ee"),
	FANMADE("Maale Adummin Quest", "ddfbeb33feb7cf78504fe4dba14ec63b"),
	FANMADE("Monkey Man", "2322d03f997e8cc235d4578efff69cfa"),
	FANMADE_FO("Napalm Quest (v0.5)", "b659afb491d967bb34810d1c6ce22093", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),
	FANMADE("Naturette 1 (English v1.2)", "0a75884e7f010974a230bdf269651117"),
	FANMADE("Naturette 1 (English v1.3)", "f15bbf999ac55ebd404aa1eb84f7c1d9"),
	FANMADE_L("Naturette 1 (French v1.2)", "d3665622cc41aeb9c7ecf4fa43f20e53", Common::FR_FRA),
	FANMADE_FO("Naturette 2: Daughter of the Moon (v1.0)", "bdf76a45621c7f56d1c9d40292c6137a", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),	// AGIPAL
	FANMADE_FO("Naturette 3: Adventure in Treeworld (v1.0a)", "6dbb0e7fc75fec442e6d9e5a06f1530e", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),	// AGIPAL
	FANMADE_FO("Naturette 3: Adventure in Treeworld (v1.0c)", "f6d4950e22fac1951b221df15d3b1e9a", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),	// AGIPAL
	FANMADE_FO("Naturette 4: From a Planet to Another Planet (Not Finished)", "13be8cd9cf35aeff0a39b8757057fbc8", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),
	// FIXME: Actually Naturette 4 has both English and French language support built into it. How to add that information?
	FANMADE_FO("Naturette 4: From a Planet to Another Planet (2007-10-05)", "8253706b6ef5423a79413b216760297c", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),	// AGIPAL
	FANMADE("New AGI Hangman Test", "d69c0e9050ccc29fd662b74d9fc73a15"),
	FANMADE("Nick's Quest - In Pursuit of QuakeMovie (v2.1 Gold)", "e29cbf9222551aee40397fabc83eeca0"),
	FANMADE_FO("Open Mic Night (v0.1)", "70000a2f67aac27d1133d019df70246d", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),	// AGIPAL
	FANMADE("Operation: Recon", "0679ce8405411866ccffc8a6743370d0"),
	FANMADE_SVP("Operation: Recon (Demo)", "c52cc1ab8450a153e9d1474ce7636a13", 642, 0x2440, Common::kPlatformCoCo3),
	FANMADE("Patrick's Quest (Demo v1.0)", "f254f5b894b98fec5f92acc07fb62841"),
	FANMADE("Phantasmagoria", "87d20c1c11aee99a4baad3797b63146b"),
	FANMADE("Pharaoh Quest (v0.0)", "51c630899d076cf799e573dadaa2276d"),
	FANMADE("Phil's Quest - the Search for Tolbaga", "5e7ca45c360e03164b8358e49900c588"),
	FANMADE("Pinkun Maze Quest (v0.1)", "148ff0843af389928b3939f463bfd20d"),
	FANMADE("Pirate Quest", "bb612a919ed2b9ea23bbf03ce69fed42"),
	FANMADE("Pothead Quest (v0.1)", "d181101385d3a45082f418cd4b3c5b01"),
	FANMADE("President's Quest", "4937d0e8ecadb7888faeb347799b0388"),
	FANMADE("Prince Quest", "266248d75c3130c8ccc9c9bf2ad30a0d"),
	FANMADE("Professor (English) - The Professor is Missing (Mar 17)", "6232de31cc204affdf2e92dfe3dc0e4d"),
	FANMADE("Professor (English) - The Professor is Missing (Mar 22)", "b5fcf0ca2f0d1c073be82f01e2170961"),
	FANMADE_L("Professor (French) - Le Professeur a Disparu", "7d9f8a4d4610bb9b0b97caa17590c2d3", Common::FR_FRA),
	FANMADE("Quest for Glory VI - Hero's Adventure", "d26765c3075064c80d284c5e06e33a7e"),
	FANMADE("Quest for Home", "d2895dc1cd3930f2489af0f843b144b3"),
	FANMADE("Quest for Ladies (demo v1.1 Apr 1)", "3f6e02f16e1154a0daf296c8895edd97"),
	FANMADE("Quest for Ladies (demo v1.1 Apr 6)", "f75e7b6a0769a3fa926eea0854711591"),
	FANMADE("Quest for Piracy 1 - Enter the Silver Pirate (v0.15)", "d23f5c2a26f6dc60c686f8a2436ea4a6"),
	FANMADE_L("Quest for the Toilet", "402d71a73f5eb236ad569f80695d3db2", Common::IT_ITA),
	FANMADE("Quest for a Record Deal", "f4fbd7abf056d2d3204f790da5ac89ab"),
	FANMADE("Ralph's Quest (v0.1)", "5cf56378aa01a26ec30f25295f0750ca"),
	FANMADE_L("Residence 44 Quest (v0.99)", "7c5cc64200660c70240053b33d379d7d", Common::NL_NLD),
	FANMADE("Residence 44 Quest (v0.99)", "fe507851fddc863d540f2bec67cc67fd"),
	FANMADE("Residence 44 Quest (v1.0a)", "f99e3f69dc8c77a45399da9472ef5801"),
	FANMADE("SQ2Eye (v0.3)", "2be2519401d38ad9ce8f43b948d093a3"),
	//FANMADE("SQ2Eye (v0.4)", "2be2519401d38ad9ce8f43b948d093a3"),
	FANMADE("SQ2Eye (v0.41)", "f0e82c55f10eb3542d7cd96c107ae113"),
	FANMADE("SQ2Eye (v0.42)", "d7beae55f6328ef8b2da47b1aafea40c"),
	FANMADE("SQ2Eye (v0.43)", "2a895f06e45de153bb4b77c982009e06"),
	FANMADE("SQ2Eye (v0.44)", "5174fc4b6d8a477ba0ff0575cd64e0aa"),
	FANMADE("SQ2Eye (v0.45)", "6e06f8bb7b90ce6f6aabf1a0e620159c"),
	FANMADE("SQ2Eye (v0.46)", "bf0ad7a035ff9113951d09d1efe380c4"),
	FANMADE("SQ2Eye (v0.47)", "85dc3be1d33ff932c292b74f9037abaa"),
	FANMADE("SQ2Eye (v0.48)", "587574252972a5b5c070a647973a9b4a"),
	FANMADE("SQ2Eye (v0.481)", "fc9234beb49804ae869696ce5af8ef30"),
	FANMADE("SQ2Eye (v0.482)", "3ed84b7b87fa6840f25c15f250a11ffb"),
	FANMADE("SQ2Eye (v0.483)", "647c31298d3f9cda641231b893e347c0"),
	FANMADE("SQ2Eye (v0.484)", "f2c86fae7b9046d408c62c8c49a4b882"),
	FANMADE("SQ2Eye (v0.485)", "af59e36bc28f44545458b68a93e91e67"),
	FANMADE("SQ2Eye (v0.486)", "3fd86436e93456770dbdd4593eded70a"),
	FANMADE("Sarien", "314e5fdef17b803226d1de3af2e997ea"),
	FANMADE("Save Santa (v1.0)", "4644f6beb5802081772f14be56ae196c"),
	FANMADE("Save Santa (v1.3)", "f8afdb6efc5af5e7c0228b44633066af"),
	FANMADE("Schiller (preview 1)", "ade39dea968c959cfebe1cf935d653e9"),
	FANMADE("Schiller (preview 2)", "62cd1f8fc758bf6b4aa334e553624cef"),
	GAME_FO("serguei1", "v1.0", "b86725f067e456e10cdbdf5f58e01dec", 0x2917, GF_FANMADE|GF_AGIMOUSE, GID_FANMADE, GAMEOPTIONS_FANMADE_MOUSE),	// AGIPAL
	GAME_FSO("serguei1", "v1.1 2002 Sep 5", "vol.0", "c518cc64fb731de406cb96f95534a388", 209980, 0x2917, GF_FANMADE|GF_AGIMOUSE, GID_FANMADE, GAMEOPTIONS_FANMADE_MOUSE),	// AGIPAL
	GAME_FSO("serguei1", "v1.1 2003 Apr 10", "vol.0", "c518cc64fb731de406cb96f95534a388", 211301, 0x2917, GF_FANMADE|GF_AGIMOUSE, GID_FANMADE, GAMEOPTIONS_FANMADE_MOUSE),	// AGIPAL
	GAME_FO("serguei2", "v0.1.1 Demo", "906ccbc2ddedb29b63141acc6d10cd28", 0x2917, GF_FANMADE|GF_AGIMOUSE, GID_FANMADE, GAMEOPTIONS_FANMADE_MOUSE),
	GAME_FO("serguei2", "v1.3.1 Demo (March 22nd 2008)", "ad1308fcb8f48723cd388e012ebf5e20", 0x2917, GF_FANMADE|GF_AGIMOUSE, GID_FANMADE, GAMEOPTIONS_FANMADE_MOUSE),	// AGIPAL
	FANMADE("Shifty (v1.0)", "2a07984d27b938364bf6bd243ac75080"),
	FANMADE_FO("Sliding Tile Game (v1.00)", "949bfff5d8a81c3139152eed4d84ca75", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),
	FANMADE("Snowboarding Demo (v1.0)", "24bb8f29f1eddb5c0a099705267c86e4"),
	FANMADE("Solar System Tour", "b5a3d0f392dfd76a6aa63f3d5f578403"),
	FANMADE("Sorceror's Appraisal", "fe62615557b3cb7b08dd60c9d35efef1"),
	GAME_FO("sq0", "v1.03", "d2fd6f7404e86182458494e64375e590", 0x2917, GF_FANMADE, GID_FANMADE, GAMEOPTIONS_DEFAULT),
	GAME_FO("sq0", "v1.04", "2ad9d1a4624a98571ee77dcc83f231b6", 0x2917, GF_FANMADE, GID_FANMADE, GAMEOPTIONS_DEFAULT),
	GAME_PS("sq0", "", "e1a8e4efcce86e1efcaa14633b9eb986", 762, 0x2440, GID_FANMADE, Common::kPlatformCoCo3),
	GAME_FO("sqx", "v10.0 Feb 05", "c992ae2f8ab18360404efdf16fa9edd1", 0x2917, GF_FANMADE, GID_FANMADE, GAMEOPTIONS_DEFAULT),
	GAME_FO("sqx", "v10.0 Jul 18", "812edec45cefad559d190ffde2f9c910", 0x2917, GF_FANMADE, GID_FANMADE, GAMEOPTIONS_DEFAULT),
	GAME_PS("sqx", "v10.0", "f0a59044475a5fa37c055d8c3eb4d1a7", 768, 0x2440, GID_FANMADE, Common::kPlatformCoCo3),
	GAME_FO("sqx", "v1.0 March 12, 2001", "ee0f10a44f94ab091455e643cd547e03", 0x2917, GF_FANMADE|GF_AGIMOUSE, GID_FANMADE, GAMEOPTIONS_DEFAULT), // AGIPAL
	FANMADE_FO("Space Quest 3.5", "c077bc28d7b36213dd99dc9ecb0147fc", GF_AGIMOUSE, GAMEOPTIONS_FANMADE_MOUSE),	// AGIPAL
	FANMADE("Space Trek (v1.0)", "807a1aeadb2ace6968831d36ab5ea37a"),
	FANMADE("Special Delivery", "88764dfe61126b8e73612c851b510a33"),
	FANMADE("Speeder Bike Challenge (v1.0)", "2deb25bab379285ca955df398d96c1e7"),
	FANMADE("Star Commander 1 - The Escape (v1.0)", "a7806f01e6fa14ebc029faa58f263750"),
	FANMADE_SVP("Star Commander 1 - The Escape (v1.0)", "a7806f01e6fa14ebc029faa58f263750", 768, 0x2440, Common::kPlatformCoCo3),
	FANMADE("Star Pilot: Bigger Fish", "8cb26f8e1c045b75c6576c839d4a0172"),
	FANMADE("Street Quest (Demo)", "cf2aa94a7eb78dce6892c37f03e310d6"),	// AGIPAL
	FANMADE("Tales of the Tiki", "8103c9c87e3964690a14a3d0d83f7ddc"),
	FANMADE("Tex McPhilip 1 - Quest For The Papacy", "3c74b9a24b51aa8020ac82bee3132266"),
	FANMADE("Tex McPhilip 2 - Road To Divinity (v1.5)", "7387e8df854440bc26620ca0ea43af9a"),
	FANMADE("Tex McPhilip 3 - A Destiny of Sin (Demo v0.25)", "992d12031a486ad84e592ff5d7c9d782"),
	FANMADE("Tex McPhilip 3 - A Destiny of Sin (v1.02)", "587d15e1106e59c33053c01b301ffe05"),
	FANMADE("The 13th Disciple (v1.00)", "887719ad59afce9a41ec057dbb73ad73"),
	FANMADE("The 13th Disciple (v1.01)", "58e3ec1b9ac1a79901c472aaa59db832"),
	FANMADE("The Adventures of a Crazed Hermit", "6e3086cbb794d3299a9c5a9792295511"),
	FANMADE("The Gourd of the Beans", "246f4d94946afb547482d44a53616d06"),
	FANMADE_P("The Gourd of the Beans: An Unplanned Adventure", "5fe4b1fdcccd6b6a7c87d852a67c7f1e", Common::kPlatformWindows), // NAGI
	FANMADE("The Grateful Dead", "c2146631afacf8cb455ce24f3d2d46e7"),
	FANMADE("The Legend of Shay-Larah 1 - The Lost Prince", "04e720c8e30c9cf12db22ea14a24a3dd"),
	FANMADE("The Legend of Zelda: The Fungus of Time (Demo v1.00)", "dcaf8166ceb62a3d9b9aea7f3b197c09"),
	FANMADE("The Legendary Harry Soupsmith (Demo 1998 Apr 2)", "64c46b0d6fc135c9835afa80980d2831"),
	FANMADE("The Legendary Harry Soupsmith (Demo 1998 Aug 19)", "8d06d82970f2c591d880a95476efbcf0"),
	FANMADE("The Long Haired Dude: Encounter of the 18-th Kind", "86ea17b9fc2f3e537a7e40863d352c29"),
	FANMADE("The Lost Planet (v0.9)", "590dffcbd932a9fbe554be13b769cac0"),
	FANMADE("The Lost Planet (v1.0)", "58564df8b6394612dd4b6f5c0fd68d44"),
	FANMADE("The New Adventure of Roger Wilco (v1.00)", "e5f0a7cb8d49f66b89114951888ca688"),
	FANMADE("The Ruby Cast (v0.02)", "ed138e461bb1516e097007e017ab62df"),
	FANMADE("The Shadow Plan", "c02cd10267e721f4e836b1431f504a0a"),
	FANMADE("The Sorceror's Appraisal", "b121ba95d2beb6c16e2f762a13b8baa2"),
	FANMADE("Time Quest (Demo v0.1)", "12e1a6f03ea4b8c5531acd0400b4ed8d"),
	FANMADE("Time Quest (Demo v0.2)", "7b710608abc99e0861ac59b967bf3f6d"),
	FANMADE_SVP("Time Quest", "90314f473d8317be5cd1f0306f139aea", 300, 0x2440, Common::kPlatformCoCo3),
	FANMADE("Toby's World (Demo)", "3f8ebea0eb32303e65e2a6e8341c6741"),
	FANMADE("Tonight The Shrieking Corpses Bleed (Demo v0.11)", "bcc57a7c8d563fa0c333107ae1c0a6e6"),
	FANMADE("Tonight The Shrieking Corpses Bleed (v1.01)", "36b38f621b38e8d104aa0807302dc8c9"),
	FANMADE("Turks' Quest - Heir to the Planet", "3d19254b737c8b218e5bc4580542b79a"),
	FANMADE("Ultimate AGI Fangame (Demo)", "2d14d6fa2a2136d681e46e06821905bf"),
	FANMADE("URI Quest (v0.173 Feb 27)", "3986eefcf546dafc45f920ae91a697c3"),
	FANMADE("URI Quest (v0.173 Jan 29)", "494150940d34130605a4f2e67ee40b12"),
	{
		// V - The Graphical Adventure
		{
			"agi-fanmade",
			"V - The Graphical Adventure (Demo 2)",
			AD_ENTRY1s("vdir", "c71f5c1e008d352ae9040b77fcf79327", 3080),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_USEEXTRAASTITLE | ADGF_AUTOGENTARGET,
			GAMEOPTIONS_DEFAULT
		},
		GID_FANMADE,
		GType_V3,
		GF_FANMADE,
		0x3149
	},
	FANMADE_SVP("V - The Graphical Adventure", "1646eaade74f137a9041eb427a389969", 768, 0x2440, Common::kPlatformCoCo3),

	FANMADE("Voodoo Girl - Queen of the Darned (v1.2 2002 Jan 1)", "ae95f0c77d9a97b61420fd192348b937"),
	FANMADE("Voodoo Girl - Queen of the Darned (v1.2 2002 Mar 29)", "11d0417b7b886f963d0b36789dac4c8f"),
	FANMADE("Voodoo Girl - Queen of the Darned (v1.2.2 2002 Dec 1)", "cf539f553d92600b24059b4b70b920c3"),
	FANMADE("Wizaro (v0.1)", "abeec1eda6eaf8dbc52443ea97ff140c"),

	{ AD_TABLE_END_MARKER, 0, 0, 0, 0 }
};

/**
 * The fallback game descriptor used by the AGI engine's fallbackDetector.
 * Contents of this struct are to be overwritten by the fallbackDetector.
 */
static AGIGameDescription g_fallbackDesc = {
	{
		"",
		"",
		AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
		Common::UNK_LANG,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GAMEOPTIONS_DEFAULT
	},
	GID_FANMADE,
	GType_V2,
	GF_FANMADE,
	0x2917
};

} // End of namespace Agi
