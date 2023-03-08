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

namespace {

#define FLAGS(x, y, z, a, b, c, d, e, f, id) { Common::UNK_LANG, Common::UNK_LANG, Common::UNK_LANG, Common::UNK_LANG, Common::kPlatformUnknown, x, y, z, a, b, c, d, e, f, id }
#define FLAGSXLANG(extraLang, x, y, z, a, b, c, d, e, f, id) { Common::UNK_LANG, extraLang, Common::UNK_LANG, Common::UNK_LANG, Common::kPlatformUnknown, x, y, z, a, b, c, d, e, f, id }
#define FLAGS_FAN(fanLang, repLang, x, y, z, a, b, c, d, e, f, id) { Common::UNK_LANG, Common::UNK_LANG, fanLang, repLang, Common::kPlatformUnknown, x, y, z, a, b, c, d, e, f, id }

#define KYRA1_FLOPPY_FLAGS FLAGS(false, false, false, false, false, false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_FLOPPY_CMP_FLAGS FLAGS(false, false, false, false, false, false, false, false, true, Kyra::GI_KYRA1)
#define KYRA1_OLDFLOPPY_FLAGS FLAGS(false, false, false, true, false, false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_OLDFLOPPY_FAN_FLAGS(x, y) FLAGS_FAN(x, y, false, false, false, true, false, false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_AMIGA_FLAGS FLAGS(false, false, false, false, false, false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_TOWNS_FLAGS FLAGS(false, true, false, false, false, false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_TOWNS_SJIS_FLAGS FLAGS(false, true, false, false, true, false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_CD_FLAGS FLAGS(false, true, true, false, false, false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_CD_FAN_FLAGS(x, y) FLAGS_FAN(x, y, false, true, true, false, false, false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_DEMO_FLAGS FLAGS(true, false, false, false, false, false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_DEMO_CD_FLAGS FLAGS(true, true, true, false, false, false, false, false, false, Kyra::GI_KYRA1)

#define KYRA2_FLOPPY_FLAGS FLAGS(false, false, false, false, false, false, false, false, false, Kyra::GI_KYRA2)
#define KYRA2_FLOPPY_CMP_FLAGS FLAGS(false, false, false, false, false, false, false, false, true, Kyra::GI_KYRA2)
#define KYRA2_FLOPPY_FAN_FLAGS(x, y) FLAGS_FAN(x, y, false, false, false, false, false, false, false, false, false, Kyra::GI_KYRA2)
#define KYRA2_CD_FLAGS FLAGS(false, false, true, false, false, false, false, false, false, Kyra::GI_KYRA2)
#define KYRA2_CD_FAN_FLAGS(x, y) FLAGS_FAN(x, y, false, false, true, false, false, false, false, false, false, Kyra::GI_KYRA2)
#define KYRA2_CD_DEMO_FLAGS FLAGS(true, false, true, false, false, false, false, false, false, Kyra::GI_KYRA2)
#define KYRA2_DEMO_FLAGS FLAGS(true, false, false, false, false, false, false, false, false, Kyra::GI_KYRA2)
#define KYRA2_TOWNS_FLAGS FLAGS(false, false, false, false, false, false, false, false, false, Kyra::GI_KYRA2)
#define KYRA2_TOWNS_SJIS_FLAGS FLAGS(false, false, false, false, true, false, false, false, false, Kyra::GI_KYRA2)

#define KYRA3_CD_FLAGS FLAGS(false, false, true, false, false, false, false, true, true, Kyra::GI_KYRA3)
#define KYRA3_CD_4LANG_FLAGS(extraLang) FLAGSXLANG(extraLang, false, false, true, false, false, false, false, true, true, Kyra::GI_KYRA3)
#define KYRA3_CD_FAN_FLAGS(x, y) FLAGS_FAN(x, y, false, false, true, false, false, false, false, true, false, Kyra::GI_KYRA3)

#define LOL_CD_FLAGS FLAGS(false, false, true, false, false, false, false, false, false, Kyra::GI_LOL)
#define LOL_CD_FAN_FLAGS(x, y) FLAGS_FAN(x, y, false, false, true, false, false, false, false, false, false, Kyra::GI_LOL)
#define LOL_FLOPPY_FLAGS FLAGS(false, false, false, false, false, false, false, false, false, Kyra::GI_LOL)
#define LOL_FLOPPY_FAN_FLAGS(x, y) FLAGS_FAN(x, y, false, false, false, false, false, false, false, false, false, Kyra::GI_LOL)
#define LOL_FLOPPY_CMP_FLAGS FLAGS(false, false, false, false, false, false, false, false, true, Kyra::GI_LOL)
#define LOL_PC9801_FLAGS FLAGS(false, false, false, false, true, true, false, false, false, Kyra::GI_LOL)
#define LOL_FMTOWNS_FLAGS FLAGS(false, false, false, false, true, false, false, false, false, Kyra::GI_LOL)
#define LOL_DEMO_FLAGS FLAGS(true, true, false, false, false, false, false, false, false, Kyra::GI_LOL)
#define LOL_CD_DEMO_FLAGS FLAGS(true, false, true, false, false, false, false, false, false, Kyra::GI_LOL)
#define LOL_KYRA2_DEMO_FLAGS FLAGS(true, false, false, false, false, false, false, false, false, Kyra::GI_KYRA2)

#define EOB_FLAGS FLAGS(false, false, false, false, false, false, false, false, false, Kyra::GI_EOB1)
#define EOB_SPANISH_FLAGS FLAGS(false, false, false, true, false, false, false, false, false, Kyra::GI_EOB1)
#define EOB_PC98_FLAGS FLAGS(false, false, false, false, true, true, false, false, false, Kyra::GI_EOB1)
#define EOB2_FLAGS FLAGS(false, false, false, false, false, false, false, false, false, Kyra::GI_EOB2)
#define EOB2_FMTOWNS_FLAGS FLAGS(false, false, false, false, true, false, true, false, false, Kyra::GI_EOB2)

static const char msg_missingLangResources[]			= _s("Missing language specific game code and/or resources.");
static const char msg_fanTrans_missingLangResources[]	= _s("Missing language specific game code and/or resources for this fan translation.");
static const char msg_fanTrans_unsupportiveTranslator[]	= _s("The fan translator does not wish his translation to be incorporated into ScummVM.");
static const char msg_nonEngineDemo[]					= _s("Demo plays simple animations without using Westwood's Engine.");

const KYRAGameDescription adGameDescs[] = {
	/* disable these targets until they get supported
	// Each DISK*.EXE file contains an embedded ZIP archive
	{
		{
			"kyra1",
			0,
			AD_ENTRY1("DISK1.EXE", "c8641d0414d6c966d0a3dad79db07bf4"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO4(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK)
		},
		KYRA1_FLOPPY_CMP_FLAGS
	},

	{
		{
			"kyra1",
			0,
			AD_ENTRY1("DISK1.EXE", "5d5cee4c3d0b68d586788b74243d254a"),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO4(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK)
		},
		KYRA1_FLOPPY_CMP_FLAGS
	},
	*/

	{ // floppy v1.0
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1s("GEMCUT.EMC", "3c244298395520bb62b5edfe41688879", 6792),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},

	{ // floppy v1.3
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1s("GEMCUT.EMC", "796e44863dd22fa635b042df1bf16673", 6816),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},

	{ // floppy v1.7
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1s("GEMCUT.EMC", "abf8eb360e79a6c2a837751fbd4d3d24", 7148),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},

	{
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1s("GEMCUT.EMC", "6018e1dfeaca7fe83f8d0b00eb0dd049", 7216),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},

	{ // from Arne.F
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1("GEMCUT.EMC", "f0b276781f47c130f423ec9679fe9ed9"),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},

	{ // russian fan translation
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1("GEMCUT.EMC", "689b62b7519215c1b2571d466c95624c"),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_OLDFLOPPY_FAN_FLAGS(Common::RU_RUS, Common::EN_ANY)
	},

	{ // floppy v2.2 from VooD
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1s("GEMCUT.EMC", "8909b41596913b3f5deaf3c9f1017b01", 7030),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},

	{ // floppy v1.8 from clemmy
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1("GEMCUT.EMC", "747861d2a9c643c59fdab570df5b9093"),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},

	{ // from gourry
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1("GEMCUT.EMC", "ef08c8c237ee1473fd52578303fc36df"),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},

	{ // Czech fan translation of v1.0, bugreport #7773
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1s("GEMCUT.EMC", "20f876423f4caa20f5de6b4fc5dfafeb", 6686),
			Common::CS_CZE,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_OLDFLOPPY_FAN_FLAGS(Common::CS_CZE, Common::EN_ANY)
	},

	{ // from trembyle
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1("GEMCUT.EMC", "3f319d6908830a46ff42229a39a2c7ec"),
			Common::ZH_TWN,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},

	{ // from trembyle
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1s("GEMCUT.EMC", "57907d931675dbd16386c1d81d18fee4", 6904),
			Common::KO_KOR,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},

	{
		{
			"kyra1",
			0,
			AD_ENTRY1s("GEMCUT.EMC", "2a3f44e179f1e9f7643e90083c747571", 6814),
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDIAMIGA, GUIO_RENDERAMIGA)
		},
		KYRA1_AMIGA_FLAGS
	},

	{
		{
			"kyra1",
			0,
			AD_ENTRY1s("GEMCUT.EMC", "74f99e9ed99abf8d0429826d78485a2a", 7214),
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDIAMIGA, GUIO_RENDERAMIGA)
		},
		KYRA1_AMIGA_FLAGS
	},

	{
		{
			"kyra1",
			0,
			AD_ENTRY1s("GEMCUT.EMC", "ed6ed782ead16d9dba0719a347e01eea", 7146),
			Common::FR_FRA,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDIAMIGA, GUIO_RENDERAMIGA)
		},
		KYRA1_AMIGA_FLAGS
	},

	{ // Supplied by CaptainHIT in bug report #11596
		{
			"kyra1",
			0,
			AD_ENTRY1s("GEMCUT.EMC", "d0d1f35c5e2b6dee64b048bb77d1fc00", 6998),
			Common::IT_ITA,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDIAMIGA, GUIO_RENDERAMIGA)
		},
		KYRA1_AMIGA_FLAGS
	},

	{ // Floppy/CD v1.0
		{
			"kyra1",
			0,
			AD_ENTRY2s("GEMCUT.EMC", "796e44863dd22fa635b042df1bf16673", 6816,
					   "BEAD.CPS",   "3038466f65b7751451844707187aa401", 534),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDIGM, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},

	{
		{
			"kyra1",
			"StuffIt multi-floppy",
			AD_ENTRY1s("xn--Legend of Kyrandia Installer-o11r", "d:23ae9506979d844c29cccb40774e471e", 1363968),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDIGM, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_CMP_FLAGS
	},

	{
		{
			"kyra1",
			"StuffIt",
			AD_ENTRY1s("xn--Install Legend of Kyrandia-jf8p", "d:e279c397f46b0913abb5595e2cdce2ed", 10276153),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDIGM, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_CMP_FLAGS
	},

	{ // FM-TOWNS version
		{
			"kyra1",
			0,
			AD_ENTRY2s("EMC.PAK",	  "a046bb0b422061aab8e4c4689400343a", 167021,
					   "TWMUSIC.PAK", "e53bca3a3e3fb49107d59463ec387a59", 140352),
			Common::EN_ANY,
			Common::kPlatformFMTowns,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDITOWNS, GUIO_RENDERFMTOWNS)
		},
		KYRA1_TOWNS_FLAGS
	},

	{
		{
			"kyra1",
			0,
			AD_ENTRY2s("JMC.PAK",	  "9c5707a2a478e8167e44283246612d2c", 168001,
					   "TWMUSIC.PAK", "e53bca3a3e3fb49107d59463ec387a59", 140352),
			Common::JA_JPN,
			Common::kPlatformFMTowns,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDITOWNS, GUIO_RENDERFMTOWNS)
		},
		KYRA1_TOWNS_SJIS_FLAGS
	},

	// PC-9801 floppy + CD / PC-9821 floppy version all use the same data files, thus we mark it as non-CD game.
	{
		{
			"kyra1",
			"",
			AD_ENTRY2s("JMC.PAK",	  "9c5707a2a478e8167e44283246612d2c", -1,
					   "MUSIC98.PAK", "02fc212f799331b769b274e33d87b37f", -1),
			Common::JA_JPN,
			Common::kPlatformPC98,
			ADGF_NO_FLAGS,
			GUIO4(GUIO_NOSPEECH, GUIO_MIDIPC98, GUIO_RENDERPC9821, GUIO_RENDERPC9801)
		},
		KYRA1_TOWNS_SJIS_FLAGS
	},

	{
		{
			"kyra1",
			"CD",
			AD_ENTRY1("GEMCUT.PAK", "fac399fe62f98671e56a005c5e94e39f"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_CD_FLAGS
	},

	{ // Hebrew fan translation
		{
			"kyra1",
			"CD",
			AD_ENTRY1("GEMCUT.PAK", "20c141be61ed01bfda09197e0452bcf7"),
			Common::HE_ISR,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_CD_FAN_FLAGS(Common::HE_ISR, Common::EN_ANY)
	},

	{ // CD v3.7
		{
			"kyra1",
			"CD",
			AD_ENTRY1s("GEMCUT.PAK", "230f54e6afc007ab4117159181a1c722", 33771),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_CD_FLAGS
	},

	{ // CD v3.7
		{
			"kyra1",
			"CD",
			AD_ENTRY1("GEMCUT.PAK", "b037c41768b652a040360ffa3556fd2a"),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_CD_FLAGS
	},

	{ // italian fan translation see bug #7567 "KYRA: add Italian CD Version to kyra.dat"
		{
			"kyra1",
			"CD",
			AD_ENTRY1("GEMCUT.PAK", "d8327fc4b7a72b23c900fa13aef4093a"),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{ // Spanish fan translation
		{
			"kyra1",
			"CD",
			AD_ENTRY1("GEMCUT.PAK", "d9fb36376939f3d31ec2c3746ff6da1b"),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_CD_FAN_FLAGS(Common::ES_ESP, Common::EN_ANY)
	},

	{ // Kyra 1 Mac CD as mentioned in bug #7695 "KYRA1: Add support for Macintosh CD" by nnooiissee
		{
			"kyra1",
			"CD",
			AD_ENTRY2s("GEMCUT.PAK", "d3d4b281cd357230aabcec46843d04bd", -1,
					   "BEAD.CPS",   "3038466f65b7751451844707187aa401", -1),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_CD,
			GUIO0()
		},
		KYRA1_CD_FLAGS
	},

	{ // Polish fan translation of Macintosh CD version
		{
			"kyra1",
			"CD",
			AD_ENTRY2s("GEMCUT.PAK", "8ddf09fd6bfafdb27b4cf31c5a6fc91d", -1,
					   "BEAD.CPS",	 "3038466f65b7751451844707187aa401", -1),
			Common::PL_POL,
			Common::kPlatformMacintosh,
			ADGF_CD,
			GUIO0()
		},
		KYRA1_CD_FAN_FLAGS(Common::PL_POL, Common::EN_ANY)
	},

	{
		{
			"kyra1",
			"CD",
			AD_ENTRY2s("GEMCUT.PAK", "4a0cb720e824295bcbccbd1407652110", -1,
					   "BEAD.CPS",	 "3038466f65b7751451844707187aa401", -1),
			Common::DE_DEU,
			Common::kPlatformMacintosh,
			ADGF_CD,
			GUIO0()
		},
		KYRA1_CD_FLAGS
	},

	{
		{
			"kyra1",
			"CD",
			AD_ENTRY2s("GEMCUT.PAK", "b71ee090aa12e80ed2ba068826d92bed", -1,
					   "BEAD.CPS",   "3038466f65b7751451844707187aa401", -1),
			Common::FR_FRA,
			Common::kPlatformMacintosh,
			ADGF_CD,
			GUIO0()
		},
		KYRA1_CD_FLAGS
	},

	{
		{
			"kyra1",
			"Demo",
			AD_ENTRY1("DEMO1.WSA", "fb722947d94897512b13b50cc84fd648"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_DEMO_FLAGS
	},

	{ // Modern fan-made Russian translation by Siberian GRemlin
		{
			"kyra1",
			msg_fanTrans_unsupportiveTranslator,
			AD_ENTRY1("MAIN_ENG.CPS", "535765395e3594bfd9b727834028e288"),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_CD | ADGF_UNSUPPORTED,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_CD_FAN_FLAGS(Common::RU_RUS, Common::EN_ANY)
	},

	{	// Special Kyrandia 1 CD demo
		{
			"kyra1",
			"Demo/CD",
			AD_ENTRY3s(	"INTRO.VRM",	"e3045fb69b8c29db84b8fda3ccbdac54", 3308968,
						"TEXT_GER.CPS", "c0aa3396593b7265c41c0acb2251a28f", 9301,
						"ALFX.DRV",		"478116a7e6698b5cd51982ab48777fe6", 6531),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO | ADGF_CD,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_DEMO_CD_FLAGS
	},

	{ // Floppy version
		{
			"kyra2",
			0,
			AD_ENTRY1("WESTWOOD.001", "3f52dda68c4f7696c8309038be9f4151"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_CMP_FLAGS
	},

	{ // Floppy version
		{
			"kyra2",
			0,
			AD_ENTRY1("WESTWOOD.001", "d787b9559afddfe058b84c0b3a787224"),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_CMP_FLAGS
	},

	{ // Floppy version extracted
		{
			"kyra2",
			"Extracted",
			AD_ENTRY1("FATE.PAK", "1ba18be685ad8e5a0ab5d46a0ce4d345"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_FLAGS
	},

	{ // Floppy version extracted
		{
			"kyra2",
			"Extracted",
			AD_ENTRY1("FATE.PAK", "262fb69dd8e52e596c7aefc6456f7c1b"),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_FLAGS
	},

	{ // Floppy version extracted
		{
			"kyra2",
			"Extracted",
			AD_ENTRY1("FATE.PAK", "f7de11506b4c8fdf64bc763206c3e4e7"),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_FLAGS
	},

	{ // Floppy version extracted
		{
			"kyra2",
			"Extracted",
			AD_ENTRY1("FATE.PAK", "e0a70c31b022cb4bb3061890020fc27c"),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_FLAGS
	},

	{ // Floppy version extracted
		{
			"kyra2",
			"Extracted",
			AD_ENTRY1("FATE.PAK", "7c6c82f9beac616d2f4accf65c448162"),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_FLAGS
	},

	{ // Floppy version extracted
		{
			"kyra2",
			"Extracted",
			AD_ENTRY1("FATE.PAK", "5bfc4dd48778ca1f3a9e001e345a9774"),
			Common::ZH_TWN,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_FLAGS
	},

	{ // Floppy version extracted. Bugreport #7666
		{
			"kyra2",
			msg_fanTrans_missingLangResources, // Reason for being unsupported
			AD_ENTRY1s("FATE.PAK", "ac81bcd4aa6e0921a87eb099827a8b06", 107309),
			Common::PL_POL,
			Common::kPlatformDOS,
			ADGF_UNSUPPORTED,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_FLAGS
	},

	{ // Floppy version extracted
		{
			"kyra2",
			"Extracted",
			AD_ENTRY1("CH01-S00.DLG", "54b7a5a94f6e1ec91f0fb1311eec09ab"),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_FAN_FLAGS(Common::RU_RUS, Common::EN_ANY)
	},

	{ // Floppy version extracted
		{
			"kyra2",
			"Extracted",
			AD_ENTRY1("CH01-S00.DLG", "7c36c0e63ab8c81cbb3ea58681331366"),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_FAN_FLAGS(Common::RU_RUS, Common::EN_ANY)
	},

	{ // CD version
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "28cbad1c5bf06b2d3825ae57d760d032"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FLAGS
	},

	{
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "28cbad1c5bf06b2d3825ae57d760d032"),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FLAGS
	},

	{
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "28cbad1c5bf06b2d3825ae57d760d032"),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FLAGS
	},

	// Italian fan translation, see bug #7642 "KYRA: add support for Italian version of Kyrandia 2&3"
	{ // CD version
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "30487f3b8d7790c7857f4769ff2dd125"),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "30487f3b8d7790c7857f4769ff2dd125"),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "30487f3b8d7790c7857f4769ff2dd125"),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "39772ff82e42c4c520050518deb82e64"),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "39772ff82e42c4c520050518deb82e64"),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "39772ff82e42c4c520050518deb82e64"),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	// Russian fan translation
	{ // CD version
		{
			"kyra2",
			msg_fanTrans_unsupportiveTranslator,
			AD_ENTRY1("FERRY.CPS", "763e2103858347d4ffffc329910d323f"),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_CD | ADGF_UNSUPPORTED,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FAN_FLAGS(Common::RU_RUS, Common::EN_ANY)
	},

	// Spanish fan translation
	{ // CD version
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "943f6d7fc9a8f0ed15d0a0b5c1f221b0"),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FAN_FLAGS(Common::ES_ESP, Common::EN_ANY)
	},

	{ // Hebrew Fan Translation
		{
			"kyra2",
			"CD",
			AD_ENTRY2s("FATE.PAK", "28cbad1c5bf06b2d3825ae57d760d032", -1,
					   "8FAT.FNT", "12424362a537e1335b10323c4013bb1d", -1),
			Common::HE_ISR,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FAN_FLAGS(Common::HE_ISR, Common::EN_ANY)
	},

	{ // Interactive Demo
		{
			"kyra2",
			"CD/Demo",
			AD_ENTRY1("THANKS.CPS", "b1a78d990b120bb2234b7094f74e30a5"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD | ADGF_DEMO,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_DEMO_FLAGS
	},

	{ // Interactive Demo
		{
			"kyra2",
			"CD/Demo",
			AD_ENTRY1("THANKS.CPS", "b1a78d990b120bb2234b7094f74e30a5"),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD | ADGF_DEMO,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_DEMO_FLAGS
	},

	{ // Interactive Demo
		{
			"kyra2",
			"CD/Demo",
			AD_ENTRY1("THANKS.CPS", "b1a78d990b120bb2234b7094f74e30a5"),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD | ADGF_DEMO,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_DEMO_FLAGS
	},

	{ // Non-Interactive Demos
		{
			"kyra2",
			"Demo",
			AD_ENTRY1("VOC.PAK", "ecb3561b63749158172bf21528cf5f45"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_DEMO_FLAGS
	},

	{ // FM-TOWNS
		{
			"kyra2",
			0,
			AD_ENTRY1("WSCORE.PAK", "c44de1302b67f27d4707409987b7a685"),
			Common::EN_ANY,
			Common::kPlatformFMTowns,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDITOWNS, GUIO_RENDERFMTOWNS)
		},
		KYRA2_TOWNS_FLAGS
	},

	{
		{
			"kyra2",
			0,
			AD_ENTRY1("WSCORE.PAK", "c44de1302b67f27d4707409987b7a685"),
			Common::JA_JPN,
			Common::kPlatformFMTowns,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDITOWNS, GUIO_RENDERFMTOWNS)
		},
		KYRA2_TOWNS_SJIS_FLAGS
	},

	{ // PC-9821
		{
			"kyra2",
			"CD",
			AD_ENTRY1("WSCORE.PAK", "c44de1302b67f27d4707409987b7a685"),
			Common::EN_ANY,
			Common::kPlatformPC98,
			ADGF_CD,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDIPC98, GUIO_RENDERPC9821)
		},
		KYRA2_TOWNS_FLAGS
	},

	{
		{
			"kyra2",
			"CD",
			AD_ENTRY1("WSCORE.PAK", "c44de1302b67f27d4707409987b7a685"),
			Common::JA_JPN,
			Common::kPlatformPC98,
			ADGF_CD,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDIPC98, GUIO_RENDERPC9821)
		},
		KYRA2_TOWNS_SJIS_FLAGS
	},

	// Kyra3

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "3833ff312757b8e6147f464cca0a6587", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FLAGS
	},

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "3833ff312757b8e6147f464cca0a6587", -1),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FLAGS
	},

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "3833ff312757b8e6147f464cca0a6587", -1),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FLAGS
	},

	{ // Hebrew fan translation
		{
			"kyra3",
			0,
			AD_ENTRY2s("ONETIME.PAK", "3833ff312757b8e6147f464cca0a6587", -1,
					   "8FAT.FNT", "12424362a537e1335b10323c4013bb1d", -1),
			Common::HE_ISR,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FAN_FLAGS(Common::HE_ISR, Common::EN_ANY)
	},

	// Mac version
	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "3833ff312757b8e6147f464cca0a6587", -1),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FLAGS
	},

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "3833ff312757b8e6147f464cca0a6587", -1),
			Common::DE_DEU,
			Common::kPlatformMacintosh,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FLAGS
	},

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "3833ff312757b8e6147f464cca0a6587", -1),
			Common::FR_FRA,
			Common::kPlatformMacintosh,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FLAGS
	},

	// Spanish fan translation, see bug #7637 "KYRA3: Add support for Spanish fan translation"
	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "9aaca21d2a205ca02ec53132f2911794", -1),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FAN_FLAGS(Common::ES_ESP, Common::EN_ANY)
	},

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "9aaca21d2a205ca02ec53132f2911794", -1),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FAN_FLAGS(Common::ES_ESP, Common::EN_ANY)
	},

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "9aaca21d2a205ca02ec53132f2911794", -1),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FAN_FLAGS(Common::ES_ESP, Common::EN_ANY)
	},

	// Italian fan translation, see bug #7642 "KYRA: add support for Italian version of Kyrandia 2&3"
	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "ee2d4d056a5de5333a3c6bda055b3cb4", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FAN_FLAGS(Common::IT_ITA, Common::FR_FRA)
	},

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "ee2d4d056a5de5333a3c6bda055b3cb4", -1),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FAN_FLAGS(Common::IT_ITA, Common::FR_FRA)
	},

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "ee2d4d056a5de5333a3c6bda055b3cb4", -1),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FAN_FLAGS(Common::IT_ITA, Common::FR_FRA)
	},

	{
		{
			"kyra3",
			0,
			{
				{ "ONETIME.PAK", 0, "3833ff312757b8e6147f464cca0a6587", -1 },
				{ "ALBUM.TRG", 0, "5e40de0c74cc2321928a15019252d468", -1 },
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FAN_FLAGS(Common::RU_RUS, Common::DE_DEU)
	},

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "93b4ca2847ab7cfe52188cbde2ffe561", -1),
			Common::ZH_CHN,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_4LANG_FLAGS(Common::ZH_CHN)
	},

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "93b4ca2847ab7cfe52188cbde2ffe561", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_4LANG_FLAGS(Common::ZH_CHN)
	},

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "93b4ca2847ab7cfe52188cbde2ffe561", -1),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_4LANG_FLAGS(Common::ZH_CHN)
	},

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "93b4ca2847ab7cfe52188cbde2ffe561", -1),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_4LANG_FLAGS(Common::ZH_CHN)
	},

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "4fe8ff05002f3a4d645f11cd326fcef2", -1),
			Common::ZH_TWN,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_4LANG_FLAGS(Common::ZH_TWN)
	},

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "4fe8ff05002f3a4d645f11cd326fcef2", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_4LANG_FLAGS(Common::ZH_TWN)
	},

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "4fe8ff05002f3a4d645f11cd326fcef2", -1),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
			KYRA3_CD_4LANG_FLAGS(Common::ZH_TWN)
	},

	{
		{
			"kyra3",
			0,
			AD_ENTRY1s("ONETIME.PAK", "4fe8ff05002f3a4d645f11cd326fcef2", -1),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
			KYRA3_CD_4LANG_FLAGS(Common::ZH_TWN)
	},

	// Lands of Lore CD
	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"05a4f588fb81dc9c0ef1f2ec20d89e24", -1,
					   "L01.PAK",		"759a0ac26808d77ea968bd392355ba1d", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FLAGS
	},

	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"05a4f588fb81dc9c0ef1f2ec20d89e24", -1,
					   "L01.PAK",		"759a0ac26808d77ea968bd392355ba1d", -1),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FLAGS
	},

	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"05a4f588fb81dc9c0ef1f2ec20d89e24", -1,
					   "L01.PAK",		"759a0ac26808d77ea968bd392355ba1d", -1),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FLAGS
	},

	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"9e4bab499b7ea9337b91ac29fcba6d13", -1,
					   "L01.PAK",		"759a0ac26808d77ea968bd392355ba1d", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FLAGS
	},

	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"9e4bab499b7ea9337b91ac29fcba6d13", -1,
					   "L01.PAK",		"759a0ac26808d77ea968bd392355ba1d", -1),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FLAGS
	},

	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"9e4bab499b7ea9337b91ac29fcba6d13", -1,
					   "L01.PAK",		"759a0ac26808d77ea968bd392355ba1d", -1),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FLAGS
	},

	// Incorrectly added multilanguage CD version
	// Based on Bugreport #6284
	{
		{
			"lol",
			_s("You added the game incorrectly. Please add the root folder of the game."),
			AD_ENTRY1s("GENERAL.PAK", "2dd5305434b11b40cbda565c0f729913", 1225285),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_WARNING | ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		LOL_CD_FLAGS
	},

	// Incorrectly added GOG CD version
	// Based on Bugreport #11769
	{
		{
			"lol",
			// I18N: The file in the game directory needs to be extracted
			_s("You added the game incorrectly. The GAME.DAT file is an ISO image and you need to extract the game data files from it."), // Reason for being unsupported
			AD_ENTRY2s("VOC.PAK", "eb398f09ba3321d872b6174a68a987d9", 2276340,
					   "GAME.DAT", "f6ec0ee628b6b2ea4a1b551e3071d84b", 306751488),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSUPPORTED | ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		LOL_CD_FLAGS
	},

	// Russian fan translation. Version 1. 03.01.2011
	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"19354b0f464295c38c801d30588df062", -1,
					   "L01.PAK",		"174d37f21e0336c5d91020f8c58717ef", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FAN_FLAGS(Common::RU_RUS, Common::DE_DEU)
	},

	// Russian fan translation. Version 2. 06.07.2014
	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"19354b0f464295c38c801d30588df062", -1,
					   "L01.PAK",		"d9b9644b709de55f990d5ef73e0f6d44", -1),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FAN_FLAGS(Common::RU_RUS, Common::DE_DEU)
	},

	// Russian fan translation. Version 24.02.2019
	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"19354b0f464295c38c801d30588df062", -1,
					   "L01.PAK",		"8aa1860a3e9f84c6f11118a0181bcfaf", -1),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FAN_FLAGS(Common::RU_RUS, Common::DE_DEU)
	},

	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"19354b0f464295c38c801d30588df062", -1,
					   "L01.PAK",		"174d37f21e0336c5d91020f8c58717ef", -1),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FAN_FLAGS(Common::RU_RUS, Common::DE_DEU)
	},

	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"19354b0f464295c38c801d30588df062", -1,
					   "L01.PAK",		"174d37f21e0336c5d91020f8c58717ef", -1),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FAN_FLAGS(Common::RU_RUS, Common::DE_DEU)
	},

	// Italian fan translation
	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"05a4f588fb81dc9c0ef1f2ec20d89e24", -1,
					   "L01.PAK",		"898485c0eb7bb4403fdd63bf5191f37e", -1),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"05a4f588fb81dc9c0ef1f2ec20d89e24", -1,
					   "L01.PAK",		"898485c0eb7bb4403fdd63bf5191f37e", -1),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"05a4f588fb81dc9c0ef1f2ec20d89e24", -1,
					   "L01.PAK",		"898485c0eb7bb4403fdd63bf5191f37e", -1),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"9e4bab499b7ea9337b91ac29fcba6d13", -1,
					   "L01.PAK",		"898485c0eb7bb4403fdd63bf5191f37e", -1),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"9e4bab499b7ea9337b91ac29fcba6d13", -1,
					   "L01.PAK",		"898485c0eb7bb4403fdd63bf5191f37e", -1),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("GENERAL.PAK",	"9e4bab499b7ea9337b91ac29fcba6d13", -1,
					   "L01.PAK",		"898485c0eb7bb4403fdd63bf5191f37e", -1),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"lol",
			"CD",
			AD_ENTRY2s("LEVEL01.FRE", "3f5a8856e86b8bc6207becd73426017c", -1,
					   "LEVEL02.FRE", "03cf8239fd4b4ead22e6b771af84b036", -1),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_FAN_FLAGS(Common::ES_ESP, Common::FR_FRA)
	},

	{
		{
			"lol",
			0,
			AD_ENTRY1s("WESTWOOD.1", "c656aa9a2b4032d341e3dc8e3525b917", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_FLOPPY_CMP_FLAGS
	},

	{
		{
			"lol",
			0,
			AD_ENTRY1s("WESTWOOD.1", "320b2828be595c491903f467094f05eb", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_FLOPPY_CMP_FLAGS
	},

	{
		{
			"lol",
			0,
			AD_ENTRY1s("WESTWOOD.1", "3c61cb7de5b2ec452f5851f5075207ee", -1),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_FLOPPY_CMP_FLAGS
	},

	{ // French floppy version 1.20, bug #6111 "KYRA: LOL Floppy FR version unknown"
		{
			"lol",
			0,
			AD_ENTRY1s("WESTWOOD.1", "43857e24d1fc6731f3b13d9ed6db8c3a", -1),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_FLOPPY_CMP_FLAGS
	},

	{ // Spanish floppy version
		{
			"lol",
			0,
			AD_ENTRY1s("WESTWOOD.1", "6c99a3e6b904cc9e8c026061305309de", -1),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_FLOPPY_CMP_FLAGS
	},

	{
		{
			"lol",
			"Extracted",
			AD_ENTRY2s("GENERAL.PAK",  "2aaa30e120c08af87196820e9dd4bf73", -1,
					   "CHAPTER7.PAK", "eb92bf7ebb4e890add1233a6b0c810ff", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_FLOPPY_FLAGS
	},

	{
		{
			"lol",
			"Extracted",
			AD_ENTRY2s("GENERAL.PAK",  "0f1fabc1f67b772a30d8e05ece720ac5", -1,
					   "CHAPTER7.PAK", "482308aba1c40ee32449b91b0c63b990", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_FLOPPY_FLAGS
	},

	{
		{
			"lol",
			"Extracted",
			AD_ENTRY2s("GENERAL.PAK",  "d119e3b57f8e5edcbb90980ca6f4215a", -1,
					   "CHAPTER7.PAK", "71a3d3cb1554294646a389e5c345cf28", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_FLOPPY_FLAGS
	},

	{
		{
			"lol",
			"Extracted",
			AD_ENTRY2s("GENERAL.PAK",  "996e66e81054d36249907a1d8158da3d", -1,
					   "CHAPTER7.PAK", "cabee57f00d6d84b65a732b6868a4959", -1),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_FLOPPY_FLAGS
	},

	{ // French floppy version 1.20, bug #6264 "Lands of Lore - french version 1.20 MD5"
		{
			"lol",
			"Extracted",
			AD_ENTRY2s("GENERAL.PAK",  "a9e22c450c4f1de6a600261183430394", -1,
					   "CHAPTER7.PAK", "fb5294f7445318876741c8db39ba0b1a", -1),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_FLOPPY_FLAGS
	},

	{ // French floppy version 1.23, bug #6111 "KYRA: LOL Floppy FR version unknown"
		{
			"lol",
			"Extracted",
			AD_ENTRY2s("GENERAL.PAK",  "f4fd14f244bd7c7fa08d026fafe44cc5", -1,
					   "CHAPTER7.PAK", "733e33c8444c93843dac3b683c283eaa", -1),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_FLOPPY_FLAGS
	},

	{ // Spanish floppy version 1.23, bug #9699 "Lands of Lore - Spanish version 1.23 MD5"
		{
			"lol",
			"Extracted",
			AD_ENTRY2s("GENERAL.PAK",  "ca1208aa60c5f3e3a7e06a0420a11526", -1,
					   "CHAPTER7.PAK", "1f11dc4ba0473eec9d6d93bbf2ee9c8a", -1),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_FLOPPY_FLAGS
	},

	{ // Chinese floppy version bug #7893
		{
			"lol",
			"Extracted",
			AD_ENTRY2s("GENERAL.PAK",  "f2c305bfb10c08371e947ab4d0a2f5f5", -1,
					   "CHAPTER7.PAK", "f3cbab6d945ceda35149a5c9a324a8fe", -1),
			Common::ZH_CHN,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_FLOPPY_FLAGS
	},

	{ // German version - Bug 9557
		{
			"lol",
			"Extracted",
			AD_ENTRY2s("GENERAL.PAK",  "8e144826df680397876e0fd7ce30c701", -1,
					   "CHAPTER7.PAK", "1240fb870aba719f5887ca7270227699", -1),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_FLOPPY_FLAGS
	},

	{ // Japanese PC9821 version - Bug 9828
		{
			"lol",
			"Extracted",
			AD_ENTRY2s("GENERAL.PAK",  "8902b233405461025e25093b9c4c8d2c", -1,
					   "CHAPTER7.PAK", "32a46c338a3d6249c273cc3b3881bb8a", -1),
			Common::JA_JPN,
			Common::kPlatformPC98,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIPC98, GUIO_RENDERPC9801, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_PC9801_FLAGS
	},

	{ // Russian fan translation
		{
			"lol",
			"Extracted",
			AD_ENTRY2s("GENERAL.PAK",  "d8f4c1153aed2418f41f886c3fb27543", -1,
					   "CHAPTER7.PAK", "f0b8a2fdff951738834fadc12248ac1f", -1),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_FLOPPY_FAN_FLAGS(Common::RU_RUS, Common::EN_ANY)
	},

	{
		{
			"lol",
			0,
			AD_ENTRY2s("GENERAL.PAK",	"3fe6539b9b09084c0984eaf7170464e9", -1,
					   "MUS.PAK",		"008dc69d8cbcdb6bae30e270fab26e76", -1),
			Common::JA_JPN,
			Common::kPlatformPC98,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIPC98, GUIO_RENDERPC9801, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_PC9801_FLAGS
	},

	{
		{
			"lol",
			0,
			AD_ENTRY2s("GENERAL.PAK",	"2e4d4ce54bac9162e11fcba6907b576e", -1,
					   "TMUS.PAK",		"5543dae575164e51856f5a49cfd6b368", -1),
			Common::JA_JPN,
			Common::kPlatformFMTowns,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDITOWNS, GUIO_RENDERFMTOWNS, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_FMTOWNS_FLAGS
	},

	{
		{
			"lol",
			"Demo",
			AD_ENTRY2s("INTRO.PAK",		"4bc22a3b57f19a49212c5de58ab014d6", -1,
					   "INTROVOC.PAK",	"7e578e4f1da31c1f294e14a8e8f3cc44", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		LOL_DEMO_FLAGS
	},

	{
		{
			"lol",
			"Demo",
			AD_ENTRY1s("GENERAL.PAK", "e94863d86c4597a2d581d05481c152ba", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		LOL_DEMO_FLAGS
	},

	{ // Interactive CD demo, Bugreport #6777
		{
			"lol",
			"CD/Demo",
			AD_ENTRY2s("GENERAL.PAK",	"17e442d3b6109d4ae8354fa55d6c8121", -1,
					   "L01.PAK",		"759a0ac26808d77ea968bd392355ba1d", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD | ADGF_DEMO,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_DEMO_FLAGS
	},

	{ // Interactive CD demo, Bugreport #6777
		{
			"lol",
			"CD/Demo",
			AD_ENTRY2s("GENERAL.PAK",	"17e442d3b6109d4ae8354fa55d6c8121", -1,
					   "L01.PAK",		"759a0ac26808d77ea968bd392355ba1d", -1),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD | ADGF_DEMO,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_DEMO_FLAGS
	},

	{ // Interactive CD demo, Bugreport #6777
		{
			"lol",
			"CD/Demo",
			AD_ENTRY2s("GENERAL.PAK",	"17e442d3b6109d4ae8354fa55d6c8121", -1,
					   "L01.PAK",		"759a0ac26808d77ea968bd392355ba1d", -1),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD | ADGF_DEMO,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS, GAMEOPTION_LOL_SAVENAMES)
		},
		LOL_CD_DEMO_FLAGS
	},

	{
		{
			"eob",
			0,
			AD_ENTRY1s("EOBDATA3.PAK", "61aff543131bd61a8b7d7dc901a8278b", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIPCSPK, GUIO_MIDIPCJR, GUIO_RENDERVGA, GUIO_RENDEREGA, GUIO_RENDERCGA, GAMEOPTION_EOB_HPGRAPHS, GAMEOPTION_EOB_MOUSESWAP)
		},
		EOB_FLAGS
	},

	{
		{
			"eob",
			0,
			AD_ENTRY1s("TEXT.DAT", "fb59b50f97fd1806756911d986b9b2b5", -1),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIPCSPK, GUIO_MIDIPCJR, GUIO_RENDERVGA, GUIO_RENDEREGA, GUIO_RENDERCGA, GAMEOPTION_EOB_HPGRAPHS, GAMEOPTION_EOB_MOUSESWAP)
		},
		EOB_FLAGS
	},

	{	// Italian fan translation
		{
			"eob",
			0,
			AD_ENTRY1s("EOBDATA3.PAK", "3ed915ab5b94d60dbfe1b55379889c51", -1),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIPCSPK, GUIO_MIDIPCJR, GUIO_RENDERVGA, GUIO_RENDEREGA, GUIO_RENDERCGA, GAMEOPTION_EOB_HPGRAPHS, GAMEOPTION_EOB_MOUSESWAP)
		},
		EOB_FLAGS
	},

	{	// Spanish version
		{
			"eob",
			0,
			AD_ENTRY1s("EOBDATA3.PAK", "a7800a8ea8251f678530eb952de7b815", -1),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO9(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIPCSPK, GUIO_MIDIPCJR, GUIO_RENDERVGA, GUIO_RENDEREGA, GUIO_RENDERCGA, GAMEOPTION_EOB_HPGRAPHS, GAMEOPTION_EOB_MOUSESWAP)
		},
		EOB_SPANISH_FLAGS
	},

	{
		{
			"eob",
			0,
			AD_ENTRY1s("TEXT.CPS", "e9c2a9d81c8ce2dc35d92ece5950c45d", -1),
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIAMIGA, GUIO_RENDERAMIGA, GAMEOPTION_EOB_HPGRAPHS, GAMEOPTION_EOB_MOUSESWAP)
		},
		EOB_FLAGS
	},

	{
		{
			"eob",
			0,
			AD_ENTRY1s("TEXT.CPS", "66253c0f435c5947e6f7166bd94f21e3", -1),
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIAMIGA, GUIO_RENDERAMIGA, GAMEOPTION_EOB_HPGRAPHS, GAMEOPTION_EOB_MOUSESWAP)
		},
		EOB_FLAGS
	},

	{
		{
			"eob",
			0,
			AD_ENTRY1s("BRICK3.BIN", "7fb13157102accf6ddb806d5ad36c833", -1),
			Common::JA_JPN,
			Common::kPlatformPC98,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIPC98, GUIO_RENDERPC9801, GAMEOPTION_EOB_HPGRAPHS, GAMEOPTION_EOB_MOUSESWAP)
		},
		EOB_PC98_FLAGS
	},

	{
		{
			"eob",
			0,
			AD_ENTRY2s("L12",		"190804270d12e806b5c7264fd9690200", -1,
					   "PLAYFLD",	"e9dbc6944e6c00801f3932808f98e443", -1),
			Common::EN_ANY,
			Common::kPlatformSegaCD,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDISEGACD, GAMEOPTION_EOB_MOUSESWAP)
		},
		EOB_FLAGS
	},

	{
		{
			"eob",
			0,
			AD_ENTRY2s("L12",		"2095d1612a37264281e37996bd8c3bc2", -1,
					   "PLAYFLD",	"e9dbc6944e6c00801f3932808f98e443", -1),
			Common::JA_JPN,
			Common::kPlatformSegaCD,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDISEGACD, GAMEOPTION_EOB_MOUSESWAP)
		},
		EOB_FLAGS
	},

	{
		{
			"eob2",
			0,
			AD_ENTRY1s("LEVEL15.INF", "10f19eab75c73d0476dc58bcf70fff7a", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO7(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GUIO_RENDEREGA, GAMEOPTION_EOB_HPGRAPHS, GAMEOPTION_EOB_MOUSESWAP)
		},
		EOB2_FLAGS
	},

	{
		{
			"eob2",
			0,
			AD_ENTRY1s("LEVEL15.INF", "ce54243ad1ca4447f521340428da2c91", -1),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO7(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GUIO_RENDEREGA, GAMEOPTION_EOB_HPGRAPHS, GAMEOPTION_EOB_MOUSESWAP)
		},
		EOB2_FLAGS
	},

	{ // Spanish version
		{
			"eob2",
			0,
			AD_ENTRY1s("LEVEL15.INF", "099c683dc4e66171b19b64ea3e90aa12", -1),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO7(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GUIO_RENDEREGA, GAMEOPTION_EOB_HPGRAPHS, GAMEOPTION_EOB_MOUSESWAP)
		},
		EOB2_FLAGS
	},

	{ // Autodesk (Flic) demo
		{
			"eob2",
			msg_nonEngineDemo, // Reason for being unsupported
			AD_ENTRY1s("DARKMOON", "46b94e1308764864746db07df64ddcc0", -1),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO | ADGF_UNSUPPORTED,
			0
		},
		EOB2_FLAGS
	},

	{
		{
			"eob2",
			msg_missingLangResources, // Reason for being unsupported
			AD_ENTRY1s("LEVEL15.INF", "f972f628d21bae404a7d52bb287c0012", -1),
			Common::ZH_ANY,
			Common::kPlatformDOS,
			ADGF_UNSUPPORTED,
			GUIO7(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GUIO_RENDEREGA, GAMEOPTION_EOB_HPGRAPHS, GAMEOPTION_EOB_MOUSESWAP)
		},
		EOB2_FLAGS
	},

	{
		{
			"eob2",
			0,
			AD_ENTRY1s("AZURE.SDT", "2915098f2d1bdcfa518f857a26bb3324", -1),
			Common::JA_JPN,
			Common::kPlatformFMTowns,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDITOWNS, GUIO_RENDERFMTOWNS, GAMEOPTION_EOB_HPGRAPHS, GAMEOPTION_EOB_MOUSESWAP)
		},
		EOB2_FMTOWNS_FLAGS
	},

	{
		{
			"eob2",
			0,
			AD_ENTRY1s("TEXT.CPS", "e7350914bd7ca68039a4bc6aa42c43d4", -1),
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIAMIGA, GUIO_RENDERAMIGA, GAMEOPTION_EOB_HPGRAPHS, GAMEOPTION_EOB_MOUSESWAP)
		},
		EOB2_FLAGS
	},

	{
		{
			"eob2",
			0,
			AD_ENTRY1s("TEXT.CPS", "562adb7c1780481205b4d86ce1a54f20", -1),
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIAMIGA, GUIO_RENDERAMIGA, GAMEOPTION_EOB_HPGRAPHS, GAMEOPTION_EOB_MOUSESWAP)
		},
		EOB2_FLAGS
	},

	{ AD_TABLE_END_MARKER, FLAGS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0) }
};

const PlainGameDescriptor gameList[] = {
	{ "kyra1", "The Legend of Kyrandia" },
	{ "kyra2", "The Legend of Kyrandia: The Hand of Fate" },
	{ "kyra3", "The Legend of Kyrandia: Malcolm's Revenge" },
	{ "lol", "Lands of Lore: The Throne of Chaos" },
	{ "eob", "Eye of the Beholder" },
	{ "eob2", "Eye of the Beholder II: The Legend of Darkmoon" },
	{ 0, 0 }
};

} // End of anonymous namespace
