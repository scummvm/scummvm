/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/file.h"
#include "common/md5.h"
#include "common/translation.h"

#include "gui/error.h"

#include "engines/grim/md5check.h"
#include "engines/grim/grim.h"

namespace Grim {

// Grim retail

const char *gfupd101[] = {
	"bc57c4db2ab814f3bb115ec5702b62f4"
};
const char *year4mus[] = {
	"939e041887c1b14f99ad951ac8e3cb25" //english unpatched
};
const char *year3mus[] = {
	"df556a8fc151f6f48bb68a8cef855e7d" //english unpatched
};
const char *year2mus[] = {
	"775376fccf0415258be6b33e7e6e4985" //english unpatched
};
const char *year1mus[] = {
	"c555c95b0f7557617da364091589d6ea" //english unpatched
};
const char *year0mus[] = {
	"fea10f11abe3ac32d53e4798f80d5ba0" //english unpatched
};
const char *vox0004[] = {
	"ae53a36f77eb0d42a28653b28c345439", //english unpatched
	"57915d4dd3aa5aa44817477b7bd41ce6", //italian unpatched
	"bae246c5335c1d139b97a48277ee8841", //german unpatched
	"a49dce6e1bb83ad40b47e03903aaff1b", //french unpatched
	"3577a8b6cf013bc5b9c9133e29d1f4f0", //spanish unpatched
	"2be5b0cc13a83f59945d31080b0949bb"  //brasilian-portuguese
};
const char *vox0003[] = {
	"7c88ad7c949a516793b584299f33ed23", //english unpatched
	"f900cb063fbd0ef2a0516e9a00291d13", //italian unpatched
	"5bda9a36181c1fea368491b842c11efe", //german unpatched
	"c4aa2c45b24f351e8fce40a141084fb0", //french unpatched
	"1d4e94e3d3fd803a02c9993412359082", //spanish unpatched
	"e107073a05f588df0c057b594c28b50e"  //brasilian-portuguese
};
const char *vox0002[] = {
	"70419878e32377d9d2906fbcb2cb59b5", //english unpatched
	"616e4147c39268b650ce112bf79ea3a1", //italian unpatched
	"5ea9fde04f3315a51fe1fe9c5bdb9d0d", //german unpatched
	"24bbf1f6d3d63ae07ad2dd06bd7dad8c", //french unpatched
	"c4484dadd4a33066290e1847523d7d64", //spanish unpatched
	"0748f44ee1e67b6d7555a644a1fa43ce"  //brasilian-portuguese
};
const char *vox0001[] = {
	"40beb2e0a6572dc89fddb21f282bc237", //english pre-patched
	"6a32f8079d2d98d1c10ec8364765fd4b", //english unpatched
	"a0455d5d883ed46f0bda5030e4b5654c", //italian unpatched
	"e13eb421b9490cbaeef66f0cf6156764", //german unpatched
	"e940d5a4a4f2229f0a6674bf45464457", //french unpatched
	"77cdd6f02bc3f7085bff17c454066de3", //spanish unpatched
	"31f261f87cc4854055c1ca1afddf677e"  //brasilian-portuguese
};
const char *vox0000[] = {
	"c7900f32d48d7e265629bd355a21c795", //english pre-patched
	"67627f2d9314f9c0ae55dd2099fc052f", //english unpatched
	"c9e1a755f280b3ebff58578c4e5f1e52", //italian unpatched
	"e884221099ba62dacf04a35be8428419", //german unpatched
	"dbe35af39eec29db21a4c6e1c228cc9d", //french unpatched
	"076bf048de229ad8cefcb3d7c0bfd134", //spanish unpatched
	"d48832ea234015e74590654ada02e163"  //brasilian-portuguese
};
const char *movie04[] = {
	"605d6f2cc4d21fbe702d8e637b5bcfa1", //english unpatched
	"7c8d7e0f4beaca3910658290e8b14cf8", //italian unpatched
	"f1b8c1de4f82099ab080f9cbe2b02da6", //german unpatched
	"7e2a4578010cdf5407bb24dcfb6705e4", //french unpatched
	"2daf2ec272a7be49cbfde053482f182c", //spanish unpatched
	"d9bfe59690a5de0500d18e29cdaa293f"  //brasilian-portuguese
};
const char *movie03[] = {
	"113a768e7206016df72f9d2f5c1851b7", //english unpatched
	"0fdb7dda9ebc049eac386d29bface366", //italian unpatched
	"d2aae2e27616f78016a71818e528acda", //german unpatched
	"c2ba7adbf09b7a4f69c180f1c74297ee", //french unpatched
	"fb87d5a9e3a81d3ffd521dee4432c4da", //spanish unpatched
	"5280486c3aee4fb1f9dcbc289f5f58b9"  //brasilian-portuguese
};
const char *movie02[] = {
	"24a8704a331a03097d195f3597fe21e1", //english unpatched
	"129dad4e40987e91c5549c819d0dad1d", //italian unpatched
	"a4d5c108a167afb0674e26790edf65e6", //german unpatched
	"90fce3e6e1f91455d92b072f773f0a67", //french unpatched
	"f5879c544f4c1f1f2e2d2e92c026c245", //spanish unpatched
	"000185c50bbf09bcd7421667d46a9e5c"  //brasilian-portuguese
};
const char *movie01[] = {
	"548a19a0a4d4ccb8660966d646210820", //english unpatched
	"9fd30d0289f9b4d5c09e821a0a478db5", //italian unpatched
	"500bc51ce211d528f6c7170f479c9525", //german unpatched
	"16674e1496e4fb3f0f5f173f12e30617", //french unpatched
	"ede9fb7d035a5d4af496f23f811707a2", //spanish unpatched
	"a88876b3347106d49b733c178f0d606a"  //brasilian-portuguese
};
const char *movie00[] = {
	"0c6b8e4fa74024c4afdf7758f8d8b1a0" //english unpatched
};
const char *data004[] = {
	"2cdb79d3606965a9a0a3378507488dd7" //english unpatched
};
const char *data003[] = {
	"76dcfc2c21f3412415674c65611fb76d", //english unpatched
	"7f76c20f1d77db287473ffef10b8b309" //german unpatched
};
const char *data002[] = {
	"d236403c0d860961963db0134380001f" //english unpatched
};
const char *data001[] = {
	"84cf3072586fe2840ae16774adb6f5cb" //english unpatched
};
const char *data000[] = {
	"2069b8bf113119910df8219e787e7e94", //english pre-patched
	"08e2505a6a7fd90d3920131b1297c60f" //english unpatched
};
const char *credits[] = {
	"6dcecad8f01657184f8576aab8fb3f00", //english unpatched
	"1827307248d55c07642342c9213a4723", //italian unpatched
	"2e6319c2ec5772ced5dc9f8b41eb5de7", //german unpatched
	"cd71ca4e600198277f22e944988f7516", //french unpatched
	"836a2081d5e57ed6ef5eaade7f770b0e", //spanish unpatched
	"368baa2f319a72278035095e4f4a4466"  //brasilian-portuguese
};
const char *local[] = {
	"6142624ce13ea3c9079aa80918010c4a", //italian unpatched
	"27c961c16f9eaf0260781883150e5611", //german unpatched
	"c9dd41f66883959de27f80bbe457f30d", //french unpatched
	"d22648d6787c2f8f0a789ee3ed0c08f7", //spanish unpatched
	"04d76cbbd5e603f35318bebb64f69378"  //brasilian-portuguese
};


// Grim demo

const char *gfdemo01[] = {
	"25d831d57a93eb3ab8edbb07b7b63943"
};
const char *gdemo001[] = {
	"aa44d6c01a1b03d4607901933635d385"
};
const char *gfdemo01ger[] = {
	"fd728f040557118b7ca436f8205029e5"
};
const char *gfdemo01fra[] = {
	"fd728f040557118b7ca436f8205029e5" // Same as german
};
const char *gfdemo01ita[] = {
	"fd728f040557118b7ca436f8205029e5" // Same as german
};
const char *gfdemo01spa[] = {
	"fd728f040557118b7ca436f8205029e5" // Same as german
};
const char *grimdemo[] = {
	"3ba28e7e36a49b5fd01ba98e3c772fe8"
};
const char *sound001[] = {
	"c91a7d405d15fb198a9cb31f89637026"
};
const char *voice001[] = {
	"f24a45079394fee296a0f7fad07c7fad"
};
const char *voice001ger[] = {
	"d1d68735ae10148ecccb6b5000a4db96"
};
const char *voice001fra[] = {
	"43f56fca727e117b724051c740202c26"
};
const char *voice001ita[] = {
	"412bd831ec42d226fa173e0f55cb47cd"
};
const char *voice001spa[] = {
	"00cb825deca33b96719bc157a5427c40"
};

// EMI retail PC version

const char *emi_artAll[] = {
	"8c7db9dab564854f2c4bab0571104780", // english patched
	"7c1da307c5c3eb1ba65b7c1a2e6b5bce", // german patched
	"f2ec4854639cd25792dd3e88fb08a1e6", // spanish patched
	"e0fbba846efca842553bb1a726a25dcf", // french patched
	"b9838ab13a672a42b1fbc8893b94ca26", // italian patched
	"52111c719bcccacd4b4b8548007edd9c" 	// Russian version
};
const char *emi_artJam[] = {
	"e5ff286dbf7b23d1ad41dd8defe48291", // english patched
	"f0bcfa22f77d12e25920148d405a9615", // german patched
	"86ceeb03620d316f2dba86f68e40bebb", // spanish patched
	"6b9c6c88a0cd95f92f60db823fca4baa", // french patched
	"70a6fe11cc2de45f7b0650c8ea749156", // italian patched
	"10cb2cf22497d0007263116db68692a6", // Mac version
};
const char *emi_artLuc[] = {
	"1fcaef5755e6f6c3c30c89536022bd72", // english patched
	"0f6bcf8997ddd3cbdf161132103f769d", // german patched
	"cc956c2b6e99329bb970c9c18ed63b80", // spanish patched
	"1d388d0c12f99ba9af540f1b8b98e34b", // french patched
	"24216ef38709dc0be3771755bd274b79", // italian patched
};
const char *emi_artMel[] = {
	"66144e92a0742fdc9cc6c4f892c53e84", // english patched
	"ed1d2548374339775110bf5826db9f6d", // german patched
	"9b49161e7bb4db1dd3cb66d33940c22f", // spanish patched
	"33f3dc6eaf6ec50a9dbad3858dc00876", // french patched
	"640559d6854c3aa5b63bfad1a9afd60e", // italian patched
};
const char *emi_artMon[] = {
	"3510567bda8a4e8c5aee4c3b3bb1f98e", // english patched
	"5ff8b38e44fe1d3aeeb277f35f932d7b", // german patched
	"1d69547c39a9a85bfb1bf50c503f479d", // spanish patched
	"408a55615fef4282b9996aa0df8373e3", // french patched
	"a27c1a33652473d7caf26c069520a765", // italian patched
};
const char *emi_lip[] = {
	"57eceeae6b82028243ede6c97c334c57", // english patched
	"0ad24338f33f8ed9b4ab21e43f99aff0", // german patched
	"c0e9f1197338440b005431d2b365d287", // spanish patched
	"53973abc64a8ecaf963ba7e2dc6f7b42", // french patched
	"9b64f65a9473d11e34055137c4d978d1", // italian patched
};
const char *emi_local[] = {
	"c3b694d941c05264e8c37dc2be75ca1c", // english patched
	"6bd6582e9ab602c1f86c8593563cc9e5"	// Russian version
};
const char *emi_patch[] = {
	"067100a100b3ca9283b796480aa39b52", // english patched
};
const char *emi_update[] = {
	"7c7dbd2349d49e382a2dea40bed448e0", // english
	"dc1376baf7c8be28854a6fa620ad2278", // german
	"12c5db7d07d198d1506ab8b141dd594d", // spanish
	"cc5ff3bb8f78a0eb4b8e0feb9cdd2e87", // french
	"943ad57c2873b5eb9928e6b04a5b48d4", // italian
	"6d2516abcdce4266ad3ae873040258ab", // portuguese
};
const char *emi_sfx[] = {
	"a5d2ded26255518b82030466c2368168", // english patched
};
const char *emi_voiceAll[] = {
	"18d3996c7de4d460b4cd4ee5897a90ae", // english patched
	"e65a13f2906899ab6eca2cce3c4cb514", // german patched
	"073ecbe5f23d17536dce591174bac593", // spanish patched
	"86126ac852312452ee79558a5e76d7fd", // french patched
	"9b6b4748e872712c267d65778a3ed2da", // italian patched
};
const char *emi_voiceJam[] = {
	"2318a4680a89b93f1f71b09e66c65c86", // english patched
	"3df7d4964fe33f22be67920b21d65710", // german patched
	"173b1b949a84746d51d51ccbf7e9856e", // spanish patched
	"89947e7e6064356ebfde10b644f3d612", // french patched
	"a50b0d5d7f3aecd8629bd5e8c0647bcc", // italian patched
};
const char *emi_voiceLuc[] = {
	"d5e4842e421bdb891e6cbf88741e9119", // english patched
	"60d96bb92042729037b37c0056f93267", // german patched
	"2e0279b8753129f6ce167cc0823a4865", // spanish patched
	"ab9fe90e177cd03af1e34059117cc995", // french patched
	"c6ed446d4448de214dbdcbb6cbcca41b", // italian patched
};
const char *emi_voiceMel[] = {
	"b8b19fc4a9438f51a99f7cf3cad9c419", // english patched
	"0511a19405b763e2dd9ab5b0637757ee", // german patched
	"15154f16af25c55d7b797174032abe8a", // spanish patched
	"3e678cef88037ebd6c52988744401c03", // french patched
	"d1af7a4f0a2bed25c43d3fd197b6df3b", // italian patched
};
const char *emi_voiceMon[] = {
	"9327a6bea951b28bac31bdb52007dc41", // english patched
	"e3046478916f0000a657f9ebf498e120", // german patched
	"eda144022632cafc45c3174abc774468", // spanish patched
	"ef130c43a84c9d88a3e95abc51839292", // french patched
	"bc4d8ab5231f620c46117205578a2642", // italian patched
};

// EMI retail PS2 version

const char *emiPS2_artAll[] = {
	"7bed6dd54ea6509b708e93017c14df8d", // english
};
const char *emiPS2_artJam[] = {
	"199c713bbfc22ae941502ffd60ea794d", // english
};
const char *emiPS2_artLuc[] = {
	"7add9758294c2118621e900940e66838", // english
};
const char *emiPS2_artMel[] = {
	"514f1a684022831c178a18d13a16b6ad", // english PAL?
	"c4f51395440049942fae39bf6155e82c", // english NTSC
};
const char *emiPS2_artMon[] = {
	"db945bd9d5fa990447bbabf6a1e00e5b", // english
};
const char *emiPS2_lip[] = {
	"57eceeae6b82028243ede6c97c334c57", // english
};
const char *emiPS2_local[] = {
	"da26ad3a739f2fa341988901ce8dfa49", // english
};
/*
const char *emiPS2_music1[] = {
    "a44753d9aaed016e6c7b398b55c97d3d", // english
};
const char *emiPS2_music2[] = {
    "204cb3d451e4bf71ae5432ab33ca3a49", // english
};
*/
const char *emiPS2_sfx[] = {
	"877e90e1329182caaf27d2ce4d3c9c42", // english
};
const char *emiPS2_voiceAll[] = {
	"194a1e545a348df0fe488012bc703338", // english
};
const char *emiPS2_voiceJam[] = {
	"686b45178a4b2fdb5bee2097739feea9", // english
};
const char *emiPS2_voiceLuc[] = {
	"c5e8e0ce9986385b6c1f31b68768be47", // english
};
const char *emiPS2_voiceMel[] = {
	"54663fc82520f1f02d7f3d9b29894540", // english
};
const char *emiPS2_voiceMon[] = {
	"b4a938b9c116e9370e992c8edfb00c35", // english
};

// EMI demo

const char *emid_i9n[] = {
	"d913504ec3a2fe52ee02a20a154a2b17", // english patched
	"1cd0547b7cb26082274624b146dab3f1", // french
	"0e011c72372acc4baeac259b46ae30f4", // german
	"25523ee7687096382cbd3f3b6d30b84c", // spanish
};
const char *emid_lip[] = {
	"0a145aa7acc1a68a738c7a6f27d23283", // english patched
};
const char *emid_MagDemo[] = {
	"0a7f0e72d3fab641f0d7b0efbaa15cdc", // english patched
};
const char *emid_tile[] = {
	"23fdff61743c5621963ca04e8ca43000", // english patched
};
const char *emid_voice[] = {
	"7f9867d48b5e0af5cb3fbd8d79741f5d", // english patched
};

// EMI Macintosh
const char *emi_installer[] = {
	"93a639e3221405862dc46e9706216c00", // German (EFMI Installer)
	"a42f8aa079a6d23c285fceba191e67a4", // English (Monkey Island 4 Installer)
};

bool MD5Check::_initted = false;
Common::Array<MD5Check::MD5Sum> *MD5Check::_files = nullptr;
int MD5Check::_iterator = -1;

void MD5Check::init() {
	if (_initted) {
		return;
	}
	_initted = true;
	_files = new Common::Array<MD5Sum>();

	#define MD5SUM(filename, sums) _files->push_back(MD5Sum(filename, sums, sizeof(sums) / sizeof(const char *)));

	if (g_grim->getGameType() == GType_GRIM) {
		if (g_grim->getGameFlags() & ADGF_DEMO) {
			bool isDemoWithVideo = false;
			if (g_grim->getGameLanguage() == Common::DE_DEU) {
				MD5SUM("gfdemo01.lab", gfdemo01ger)
				MD5SUM("voice001.lab", voice001ger)
			} else if (g_grim->getGameLanguage() == Common::FR_FRA) {
				MD5SUM("gfdemo01.lab", gfdemo01fra)
				MD5SUM("voice001.lab", voice001fra)
			} else if (g_grim->getGameLanguage() == Common::IT_ITA) {
				MD5SUM("gfdemo01.lab", gfdemo01ita)
				MD5SUM("voice001.lab", voice001ita)
			} else if (g_grim->getGameLanguage() == Common::ES_ESP) {
				MD5SUM("gfdemo01.lab", gfdemo01spa)
				MD5SUM("voice001.lab", voice001spa)
			} else {
				// Check which version we have
				Common::File test;
				if (!test.open("gfdemo01.lab")) {
					isDemoWithVideo = true;
					MD5SUM("gdemo001.lab", gdemo001);
				} else {
					MD5SUM("gfdemo01.lab", gfdemo01)
					MD5SUM("voice001.lab", voice001)
				}
			}
			if (!isDemoWithVideo) {
				MD5SUM("sound001.lab", sound001)
			}
			MD5SUM("grimdemo.mus", grimdemo)
		} else {
			MD5SUM("gfupd101.exe", gfupd101)
			MD5SUM("year4mus.lab", year4mus)
			MD5SUM("year3mus.lab", year3mus)
			MD5SUM("year2mus.lab", year2mus)
			MD5SUM("year1mus.lab", year1mus)
			MD5SUM("year0mus.lab", year0mus)
			MD5SUM("vox0004.lab", vox0004)
			MD5SUM("vox0003.lab", vox0003)
			MD5SUM("vox0002.lab", vox0002)
			MD5SUM("vox0001.lab", vox0001)
			MD5SUM("vox0000.lab", vox0000)
			MD5SUM("movie04.lab", movie04)
			MD5SUM("movie03.lab", movie03)
			MD5SUM("movie02.lab", movie02)
			MD5SUM("movie01.lab", movie01)
			MD5SUM("movie00.lab", movie00)
			MD5SUM("data004.lab", data004)
			MD5SUM("data003.lab", data003)
			MD5SUM("data002.lab", data002)
			MD5SUM("data001.lab", data001)
			MD5SUM("data000.lab", data000)
			MD5SUM("credits.lab", credits)
			if (g_grim->getGameLanguage() != Common::EN_ANY) {
				MD5SUM("local.lab", local)
			}
		}
	} else {
		if (g_grim->getGameFlags() & ADGF_DEMO) {
			MD5SUM("i9n.lab", emid_i9n)
			MD5SUM("lip.lab", emid_lip)
			MD5SUM("MagDemo.lab", emid_MagDemo)
			MD5SUM("tile.lab", emid_tile)
			MD5SUM("voice.lab", emid_voice)
		} else if (g_grim->getGamePlatform() == Common::kPlatformPS2) {
			MD5SUM("artAll.m4b", emiPS2_artAll)
			MD5SUM("artJam.m4b", emiPS2_artJam)
			MD5SUM("artLuc.m4b", emiPS2_artLuc)
			MD5SUM("artMel.m4b", emiPS2_artMel)
			MD5SUM("artMon.m4b", emiPS2_artMon)
			MD5SUM("lip.m4b", emiPS2_lip)
			MD5SUM("local.m4b", emiPS2_local)
			MD5SUM("sfx.m4b", emiPS2_sfx)
			MD5SUM("voiceAll.m4b", emiPS2_voiceAll)
			MD5SUM("voiceJam.m4b", emiPS2_voiceJam)
			MD5SUM("voiceLuc.m4b", emiPS2_voiceLuc)
			MD5SUM("voiceMel.m4b", emiPS2_voiceMel)
			MD5SUM("voiceMon.m4b", emiPS2_voiceMon)
		} else {
			if (g_grim->getGamePlatform() == Common::kPlatformMacintosh) {
				if (g_grim->getGameLanguage() == Common::DE_DEU) {
					// Known to be the correct filename for german
					MD5SUM("EFMI Installer", emi_installer)
				} else {
					// Known to be the correct filename for english
					MD5SUM("Monkey Island 4 Installer", emi_installer)
				}
			}
			MD5SUM("artAll.m4b", emi_artAll)
			MD5SUM("artJam.m4b", emi_artJam)
			MD5SUM("artLuc.m4b", emi_artLuc)
			MD5SUM("artMel.m4b", emi_artMel)
			MD5SUM("artMon.m4b", emi_artMon)
			MD5SUM("lip.m4b", emi_lip)
			MD5SUM("local.m4b", emi_local)
			MD5SUM("sfx.m4b", emi_sfx)
			MD5SUM("voiceAll.m4b", emi_voiceAll)
			MD5SUM("voiceJam.m4b", emi_voiceJam)
			MD5SUM("voiceLuc.m4b", emi_voiceLuc)
			MD5SUM("voiceMel.m4b", emi_voiceMel)
			MD5SUM("voiceMon.m4b", emi_voiceMon)

			const char *updateFilename = g_grim->getUpdateFilename();
			if (updateFilename)
				MD5SUM(updateFilename, emi_update)
		}
	}

	#undef MD5SUM
}

void MD5Check::clear() {
	delete _files;
	_files = nullptr;
	_initted = false;
}

bool MD5Check::checkMD5(const MD5Sum &sums, const char *md5) {
	for (int i = 0; i < sums.numSums; ++i) {
		if (strcmp(sums.sums[i], md5) == 0) {
			return true;
		}
	}
	return false;
}

bool MD5Check::checkFiles() {
	startCheckFiles();
	bool ok = true;
	while (_iterator != -1) {
		ok = advanceCheck() && ok;
	}

	return ok;
}

void MD5Check::startCheckFiles() {
	init();
	_iterator = 0;
}

bool MD5Check::advanceCheck(int *pos, int *total) {
	if (_iterator < 0) {
		return false;
	}

	const MD5Sum &sum = (*_files)[_iterator++];
	if (pos) {
		*pos = _iterator;
	}
	if (total) {
		*total = _files->size();
	}
	if ((uint32)_iterator == _files->size()) {
		_iterator = -1;
	}

	Common::File file;
	if (file.open(sum.filename)) {
		Common::String md5 = Common::computeStreamMD5AsString(file);
		if (!checkMD5(sum, md5.c_str())) {
			warning("'%s' may be corrupted. MD5: '%s'", sum.filename, md5.c_str());
			GUI::displayErrorDialog(Common::U32String::format(_("The game data file %s may be corrupted.\nIf you are sure it is "
									"not please provide the ScummVM team the following code, along with the file name, the language and a "
									"description of your game version (i.e. dvd-box or jewelcase):\n%s"), sum.filename, md5.c_str()));
			return false;
		}
	} else {
		warning("Could not open %s for checking", sum.filename);
		GUI::displayErrorDialog(Common::U32String::format(_("Could not open the file %s for checking.\nIt may be missing or "
								"you may not have the rights to open it.\nGo to https://wiki.scummvm.org/index.php/Datafiles to see a list "
								"of the needed files."), sum.filename));
		return false;
	}

	return true;
}

}
