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

#include "cryomni3d/datstream.h"

#include "cryomni3d/versailles/engine.h"

namespace CryOmni3D {
namespace Versailles {

const uint CryOmni3DEngine_Versailles::kSpritesMapTable[] = {
	/*   0 */ 242, 240, 243, 241, 256,  93,  97,  94, 160,  98, 178, 161, 179, 196, 197, 244,
	/*  16 */ 142, 245, 143, 254,  95,  99, 113,  96, 100, 180, 114, 181,  73, 144,  74, 250,
	/*  32 */ 202, 145, 170, 251, 203, 130, 206, 171,  49, 131, 207, 115, 116, 222,  75,  85,
	/*  48 */  76, 252, 204, 236,  86, 172, 253, 205, 237, 132,  81, 208, 173, 133,  82, 209,
	/*  64 */  24, 101,  25, 102,  87, 198,  88,  83, 258, 199,  84, 259, 257, 260,  26, 103,
	/*  80 */  28,  44,  27, 104,  29,  45, 200, 105, 201, 106, 162, 163,  32,  30,  46, 126,
	/*  96 */  33,  31,  47,   5, 127, 122, 219, 227, 123, 220, 107,  69, 108,  70, 164, 165,
	/* 112 */  89,   4,  90,  36,  34,  58, 128, 109,  37,  35, 255, 129, 110, 124, 125,  71,
	/* 128 */  40,  72,  41,  91,  92,  59, 228,  38,   7,  60, 111, 229,  39, 149, 121, 138,
	/* 144 */ 112,   6, 139, 148,  42,  43, 232, 230, 233, 231, 140, 141, 134, 150, 135, 234,
	/* 160 */ 151,  20, 226, 261, 235,  21, 262, 166, 246, 167, 136,  50, 247, 215, 152, 137,
	/* 176 */  51, 216, 153,  22, 117,  48,  23, 225, 118, 223, 182, 168, 248, 183, 169,  54,
	/* 192 */  52, 249, 217,  55,  53, 218,   8, 214, 119, 120, 186, 184, 154,  61, 187, 185,
	/* 208 */ 155,  62,  56,  57, 188, 156,  65,  63, 210, 189, 157,  66,  64, 211,  19,   3,
	/* 224 */  80, 221,   1, 263,  78,  67, 174, 212,  68, 175, 213, 190, 191, 238,   0, 239,
	/* 240 */ 224,  77, 146,   2, 147,  79, 158, 176, 159, 177, 194, 192, 195, 193, /*-1u, -1u*/
};
const uint CryOmni3DEngine_Versailles::kSpritesMapTableSize = ARRAYSIZE(kSpritesMapTable);

const LevelInitialState CryOmni3DEngine_Versailles::kLevelInitialStates[] = {
	{  1, M_PI,   0. }, // Level 1
	{  9, M_PI,   0. }, // Level 2
	{ 10, M_PI_2, 0. }, // Level 3
	{ 10, 0.,     0. }, // Level 4
	{ 14, M_PI,   0. }, // Level 5
	{  8, 0.,     0. }, // Level 6
	{  1, M_PI,   0. }, // Level 7
	{  4, M_PI,   0. }  // Level 8
};

const FakeTransitionActionPlace CryOmni3DEngine_Versailles::kFakeTransitions[] = {
	{31141, 15},
	{31142, 16},
	{31143, 17},
	{32201, 18},
	{32202, 19},
	{32203, 20},
	{32204, 21},
	{35330, 36},
	{34172, 18},
	{0, 0} // Must be the last one
};

static void readSubtitles(Common::HashMap<Common::String, Common::Array<SubtitleEntry> > &subtitles,
                          DATSeekableStream *data) {
	uint16 vidsCount = data->readUint16LE();
	for (uint16 i = 0; i < vidsCount; i++) {
		Common::String vidName = data->readString16();
		Common::Array<SubtitleEntry> &entries = subtitles[vidName];

		uint16 linesCount = data->readUint16LE();
		entries.resize(linesCount);
		for (uint16 j = 0; j < linesCount; j++) {
			SubtitleEntry &entry = entries[j];

			entry.frameStart = data->readUint32LE();
			entry.text = data->readString16();
		}
	}
}

void CryOmni3DEngine_Versailles::loadStaticData() {
	// This should match data in devtools/create_cryomni3d_dat
	DATSeekableStream *data = getStaticData(MKTAG('V', 'R', 'S', 'L'), 1);

	// In the dat file we have
	// file names
	data->readString16Array16(_localizedFilenames);
	assert(_localizedFilenames.size() == LocalizedFilenames::kMax);

	// epigraph settings, bomb password
	_epigraphContent = data->readString16();
	_epigraphPassword = data->readString16();

	if (getLanguage() == Common::JA_JPN) {
		_bombAlphabet = data->readString16().decode(Common::kWindows932);
		_bombPassword = data->readString16().decode(Common::kWindows932);
	} else {
		_bombAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ '";
		_bombPassword = data->readString16();
	}

	// messages, paintings titles
	data->readString16Array16(_messages);
	if ((getLanguage() == Common::JA_JPN) ||
	        (getLanguage() == Common::KO_KOR) ||
	        (getLanguage() == Common::ZH_TWN)) {
		assert(_messages.size() == 151);
	} else {
		assert(_messages.size() == 146);
	}
	data->readString16Array16(_paintingsTitles);
	assert(_paintingsTitles.size() == 48);

	_subtitles.clear();
	// Only CJK have subtitles, don't change dat format for other languages
	if ((getLanguage() == Common::JA_JPN) ||
	        (getLanguage() == Common::KO_KOR) ||
	        (getLanguage() == Common::ZH_TWN)) {
		readSubtitles(_subtitles, data);
	}

	delete data;
}

struct VideoSubSetting {
	const char *videoName;
	int16 textLeft;
	int16 textTop;
	int16 textRight;
	int16 textBottom;
	int16 drawLeft;
	int16 drawTop;
	int16 drawRight;
	int16 drawBottom;
};

static const VideoSubSetting kVideoSubSettings[] = {
	{"11D_LEB", 15, 11, 190, 479, 208, 129, 562, 479},
	{"11E_HUI", 330, 9, 620, 479, 111, 109, 321, 341},
	{"11E_MAN", 403, 12, 630, 479, 134, 89, 390, 405},
	{"11E_RAC", 10, 9, 241, 479, 271, 147, 628, 479},
	{"12E_HUI", 361, 16, 618, 479, 84, 107, 330, 479},
	{"13F_HUI", 373, 12, 633, 479, 96, 88, 341, 479},
	{"21B1_HUI", 355, 13, 625, 479, 96, 104, 337, 479},
	{"21F_BON", 324, 11, 628, 479, 84, 74, 307, 479},
	{"21F_BON2", 11, 13, 298, 479, 321, 99, 536, 424},
	{"21G_CON", 12, 13, 255, 479, 273, 156, 539, 479},
	{"21G_DAU", 358, 11, 631, 479, 82, 151, 346, 479},
	{"21G_HUI", 309, 17, 626, 479, 77, 85, 304, 479},
	{"21I_LEB", 343, 10, 628, 479, 38, 125, 330, 479},
	{"21Z_ALI", 380, 13, 627, 479, 184, 106, 369, 479},
	{"21Z_BOU", 365, 13, 629, 479, 95, 65, 341, 321},
	{"21Z_MON", 12, 11, 309, 479, 336, 101, 561, 406},
	{"21Z_PR", 10, 16, 352, 471, 375, 104, 567, 400},
	{"22G_DAU", 339, 13, 629, 479, 114, 152, 326, 479},
	{"23I_LEB", 341, 15, 627, 479, 67, 140, 325, 410},
	{"24Z_BON", 253, 23, 620, 479, 58, 166, 228, 439},
	{"31J_SUI", 9, 9, 183, 475, 195, 159, 428, 479},
	{"31L1_LUL", 367, 16, 628, 477, 136, 164, 359, 472},
	{"31M_SUI", 19, 16, 212, 479, 231, 193, 395, 479},
	{"31O_SUIA", 11, 12, 175, 479, 186, 118, 490, 479},
	{"31O_SUIP", 12, 9, 277, 466, 296, 183, 380, 349},
	{"31Q_SUI", 334, 15, 626, 479, 158, 169, 313, 308},
	{"31X_BO", 332, 11, 615, 479, 89, 78, 313, 296},
	{"31X_BON", 329, 12, 618, 456, 0, 171, 243, 479},
	{"31X_LOU", 12, 9, 267, 447, 280, 88, 639, 479},
	{"31X_SEI", 352, 12, 626, 479, 102, 98, 340, 479},
	{"32J_CRO", 418, 7, 618, 477, 103, 58, 402, 438},
	{"32M_MR", 13, 11, 175, 477, 184, 113, 476, 447},
	{"32Q_MON", 375, 17, 623, 479, 248, 161, 341, 259},
	{"32Q_RAC", 294, 11, 627, 479, 110, 152, 287, 479},
	{"32Q_RAC2", 374, 13, 625, 479, 0, 101, 366, 479},
	{"31O_SUIA", 11, 12, 175, 479, 186, 118, 490, 479},
	{"41C_HUI", 345, 17, 626, 479, 69, 147, 330, 479},
	{"41X2_CRO", 13, 13, 281, 479, 305, 113, 548, 427},
	{"42C_BON", 15, 13, 347, 479, 368, 173, 525, 410},
	{"43B1_MAI", 264, 15, 625, 479, 127, 154, 249, 296},
	{"43B1_SEI", 17, 14, 369, 479, 390, 142, 639, 479},
	{"43C_CON", 312, 11, 635, 479, 21, 137, 294, 476},
	{"43C_DUR", 11, 10, 295, 479, 311, 166, 639, 479},
	{"44C_BON", 17, 12, 331, 479, 358, 181, 531, 407},
	{"4_MAI", 325, 14, 630, 479, 35, 48, 308, 363},
	{"51L_LOU", 11, 11, 616, 161, 154, 165, 400, 479},
	{"51L_PRI", 26, 19, 601, 153, 130, 167, 311, 479},
	{"51M_LEB", 41, 29, 615, 188, 49, 200, 432, 479},
	{"51M_MAN", 23, 19, 618, 179, 211, 195, 449, 479},
	{"52A4_LAC", 12, 11, 258, 479, 273, 184, 465, 383},
	{"52L_BOU", 12, 12, 190, 479, 307, 56, 592, 332},
	{"52L_LOU", 8, 13, 604, 168, 135, 171, 413, 479},
	{"52L_PRI", 20, 17, 610, 167, 336, 182, 639, 479},
	{"53N_BON", 351, 13, 629, 479, 62, 119, 343, 418},
	{"54I_BON", 343, 14, 623, 479, 72, 117, 339, 440},
	{"61_BON", 10, 7, 311, 479, 336, 101, 581, 479},
	{"61_DUC", 10, 14, 344, 473, 376, 156, 639, 479},
	{"61_LEN", 13, 9, 269, 479, 285, 63, 590, 479},
	{"62_DUC", 18, 21, 317, 479, 388, 154, 614, 479},
};

void CryOmni3DEngine_Versailles::setupDialogVariables() {
#define SET_DIAL_VARIABLE(id, var) _dialogsMan.setupVariable(id, var)
	SET_DIAL_VARIABLE(0, "JOUEUR-PARLE-HUISSIER-PETIT-LEVER");
	SET_DIAL_VARIABLE(1, "HUBAS-PARLE-LEVER1");
	SET_DIAL_VARIABLE(2, "HUBAS-PARLE-LEVER2");
	SET_DIAL_VARIABLE(3, "LEBRUN-DIT-COLBERT");
	SET_DIAL_VARIABLE(4, "LEBRUN-PARLE-ESQUISSE");
	SET_DIAL_VARIABLE(5, "JOUEUR-PARLE-HUISSIER-GRAND-LEVER");
	SET_DIAL_VARIABLE(6, "BONTEMPS-PARLE-MAINTENON");
	SET_DIAL_VARIABLE(7, "BONTEMPS-PARLE-MAINTENON2");
	SET_DIAL_VARIABLE(8, "BONTEMPS-DEMANDE-INDICE");
	SET_DIAL_VARIABLE(9, "BONTEMPS-DIT-ENQUETE");
	SET_DIAL_VARIABLE(10, "JOUEUR-CONFIE-MESSAGE-HUISSIER");
	SET_DIAL_VARIABLE(11, "JOUEUR-PARLE-HUIMA1");
	SET_DIAL_VARIABLE(12, "MONSEIGNEUR-ATTEND-ESQUISSES");
	SET_DIAL_VARIABLE(13, "MONSEIGNEUR-PREVIENT-BONTEMPS");
	SET_DIAL_VARIABLE(14, "JOUEUR-MENT-MONSEIGNEUR");
	SET_DIAL_VARIABLE(15, "JOUEUR-ECOUTE-ALIAS");
	SET_DIAL_VARIABLE(16, "JOUEUR-PARLE-HUCON");
	SET_DIAL_VARIABLE(17, "BONTEMPS-ATTEND-OBJET-GALLERIE");
	SET_DIAL_VARIABLE(18, "SUISSE-APOLLON-PARLE-CLEF");
	SET_DIAL_VARIABLE(19, "SUISSE-CABINET-DEMANDE-AUTORISATION");
	SET_DIAL_VARIABLE(20, "SUISSE-VU-AUTORISATION");
	SET_DIAL_VARIABLE(21, "CROISSY-ACCEPTE-TEXTE");
	SET_DIAL_VARIABLE(22, "JOUEUR-POSSEDE-CLEF-PETITE-PORTE");
	SET_DIAL_VARIABLE(23, "SUISSE-REFUSE-CLEF");
	SET_DIAL_VARIABLE(24, "LULLY-ATTEND-MISSION-JOUEUR");
	SET_DIAL_VARIABLE(25, "LULLY-DONNE-MISSION1-JOUEUR");
	SET_DIAL_VARIABLE(26, "LULLY-DONNE-MISSION-JOUEUR");
	SET_DIAL_VARIABLE(27, "RACINE-REPOND-ETRANGERE");
	SET_DIAL_VARIABLE(28, "RACINE-REPOND-PEUPLES");
	SET_DIAL_VARIABLE(29, "LULLY-DONNE-MISSION2-JOUEUR");
	SET_DIAL_VARIABLE(30, "LULLY-DIT-CHAT-PENDU-JOUEUR");
	SET_DIAL_VARIABLE(31, "JOUEUR-DIT-PEUPLES-LULLY");
	SET_DIAL_VARIABLE(32, "LALANDE-PARLE-BONTEMPS-SCENE3");
	SET_DIAL_VARIABLE(33, "BONTEMPS-DONNE-AUTORISATION-CURIOSITES");
	SET_DIAL_VARIABLE(34, "BONTEMPS-ATTEND-PAMPHLET");
	SET_DIAL_VARIABLE(35, "BONTEMPS-VU-PAMPHLET-DECHIFFRE-LULLY");
	SET_DIAL_VARIABLE(36, "CROISSY-DIT-INEPTIES");
	SET_DIAL_VARIABLE(37, "CROISSY-ATTEND-PAMPHLET2");
	SET_DIAL_VARIABLE(38, "CROISSY-ATTEND-MEDAILLE");
	SET_DIAL_VARIABLE(39, "CROISSY-ATTEND-PAMPHLET2-2");
	SET_DIAL_VARIABLE(40, "JOUEUR-PARLE-CROISSY1");
	SET_DIAL_VARIABLE(41, "MONSIEUR-PARLE-LALANDE1");
	SET_DIAL_VARIABLE(42, "MONSIEUR-ATTEND-FUSAIN");
	SET_DIAL_VARIABLE(43, "MONSIEUR-DONNE-SOLUTION-MEDAILLES");
	SET_DIAL_VARIABLE(44, "HUISSIER-DIT-DINER");
	SET_DIAL_VARIABLE(45, "HUISSIER-DIT-PREVENIR-BONTEMPS");
	SET_DIAL_VARIABLE(46, "JOUEUR-POSSEDE-PAMPHLET-RELIGION");
	SET_DIAL_VARIABLE(47, "JOUEUR-PARLE-BONTEMPS-SCENE4");
	SET_DIAL_VARIABLE(48, "BONTEMPS-VU-PAPIER-CROISSY");
	SET_DIAL_VARIABLE(49, "BONTEMPS-ATTEND-OBJET-SCENE4");
	SET_DIAL_VARIABLE(50, "BONTEMPS-VU-PAMPHLET-GOUVERNEMENT");
	SET_DIAL_VARIABLE(51, "JOUEUR-PARLE-VAUBAN");
	SET_DIAL_VARIABLE(52, "JOUEUR-PARLE-CODE-LOUVOIS");
	SET_DIAL_VARIABLE(53, "LALANDE-ECOUTE-LOUVOIS");
	SET_DIAL_VARIABLE(54, "JOUEUR-PARLE-LACHAIZE");
	SET_DIAL_VARIABLE(55, "JOUEUR-PARLE-LACHAIZE2");
	SET_DIAL_VARIABLE(56, "LACHAIZE-ATTEND-TEXTE");
	SET_DIAL_VARIABLE(57, "LACHAIZE-VU-PAMPHLET-RELIGION");
	SET_DIAL_VARIABLE(58, "LACHAIZE-DIT-REFORME");
	SET_DIAL_VARIABLE(59, "LACHAIZE-PARLE-BOUILLON");
	SET_DIAL_VARIABLE(60, "BOUILLON-DIT-DRAGONNADES");
	SET_DIAL_VARIABLE(61, "JOUEUR-PARLE-BOUILLON");
	SET_DIAL_VARIABLE(62, "LACHAIZE-TROUVE-ECROUELLES");
	SET_DIAL_VARIABLE(63, "LACHAIZE-DIT-DRAGONNADES");
	SET_DIAL_VARIABLE(64, "LACHAIZE-DEMANDE-TEXTE");
	SET_DIAL_VARIABLE(65, "LACHAIZE-PARLE-ARCHITECTURE");
	SET_DIAL_VARIABLE(66, "JOUEUR-DIT-DRAGONNADES");
	SET_DIAL_VARIABLE(67, "BOUILLON-ATTEND-PAMPHLET");
	SET_DIAL_VARIABLE(68, "BONTEMPS-PARLE-LUSTRE");
	SET_DIAL_VARIABLE(69, "BONTEMPS-ATTEND-MEMORANDUM2");
	SET_DIAL_VARIABLE(70, "BONTEMPS-DIT-PROMENADE");
	SET_DIAL_VARIABLE(71, "BONTEMPS-ATTEND-MEMORANDUM");
	SET_DIAL_VARIABLE(72, "LENOTRE-DIT-CALME");
	SET_DIAL_VARIABLE(73, "MAINE-DIT-APOTHICAIRIE");
	SET_DIAL_VARIABLE(74, "JOUEUR-PARLE-BONTEMPS-SCENE6");
	SET_DIAL_VARIABLE(75, "{JOUEUR-ESSAYE-OUVRIR-PORTE-CHAMBRE}");
	SET_DIAL_VARIABLE(76, "{JOUEUR-TROUVE-TITRE-ET-PAMPHLET}");
	SET_DIAL_VARIABLE(77, "{JOUEUR-ESSAYE-OUVRIR-PORTE-SALON}");
	SET_DIAL_VARIABLE(78, "{JOUEUR-MONTRE-PAPIER-ECRIT-ENCRE-SYMPATHIQUE}");
	SET_DIAL_VARIABLE(79, "{JOUEUR-MONTRE-UN-PAMPHLET}");
	SET_DIAL_VARIABLE(80, "{JOUEUR-MONTRE-TOUT-AUTRE-OBJET}");
	SET_DIAL_VARIABLE(81, "{JOUEUR-MONTRE-PAMPHLET-ARTS}");
	SET_DIAL_VARIABLE(82, "{JOUEUR-A-MONTRE-ESQUISSES-NON-TRIEES-LEBRUN}");
	SET_DIAL_VARIABLE(83, "{JOUEUR-DONNE-ESQUISSES}");
	SET_DIAL_VARIABLE(84, "{JOUEUR-SE-DIRIGE-VERS-MONSEIGNEUR-AVEC-ESQUISSES}");
	SET_DIAL_VARIABLE(85, "{JOUEUR-PRESENTE-FAUX-CROQUIS3}");
	SET_DIAL_VARIABLE(86, "{JOUEUR-PRESENTE-FAUX-CROQUIS2}");
	SET_DIAL_VARIABLE(87, "{JOUEUR-PRESENTE-FAUX-CROQUIS}");
	SET_DIAL_VARIABLE(88, "{LE JOUEUR-PRESENTE-ESQUISSES-TRIEES}");
	SET_DIAL_VARIABLE(89, "{LE JOUEUR-PRESENTE-AUTRES-ESQUISSES-OU-ESQUISSE-NON-TRIEES}");
	SET_DIAL_VARIABLE(90, "{JOUEUR-PRESENTE-PAMPHLET-SUR-LEBRUN}");
	SET_DIAL_VARIABLE(91, "{JOUEUR-PRESENTE-TOUT-AUTRE-PAMPHLET-OU-LETTRE}");
	SET_DIAL_VARIABLE(92, "{JOUEUR-MONTRE-ESQUISSE-DETRUITE}");
	SET_DIAL_VARIABLE(93, "{JOUEUR-MONTRE-TITRE-FABLE-APPARU-SUR-ESQUISSE}");
	SET_DIAL_VARIABLE(94, "{JOUEUR-MONTRE-AUTORISATION-DE-BONTEMPS}");
	SET_DIAL_VARIABLE(95, "{LE JOUEUR-A-TENTE-OUVRIR-PETITE-PORTE}");
	SET_DIAL_VARIABLE(96, "{JOUEUR-POSSEDE-CLE}");
	SET_DIAL_VARIABLE(97, "{JOUEUR-PRESENTE-PAMPHLET-PARTITION}");
	SET_DIAL_VARIABLE(98, "{JOUEUR-MONTRE-PAMPHLET-DECHIFFRE-PAR-LULLY}");
	SET_DIAL_VARIABLE(99, "{JOUEUR-MONTRE-MEDAILLES-MONSIEUR}");
	SET_DIAL_VARIABLE(100, "{JOUEUR-MONTRE-PAMPHLET-ARCHITECTURE}");
	SET_DIAL_VARIABLE(101, "{JOUEUR-MONTRE-EPIGRAPHE-MEDAILLES}");
	SET_DIAL_VARIABLE(102, "{JOUEUR-MONTRE-TOUT-AUTRE-CHOSE}");
	SET_DIAL_VARIABLE(103, "{JOUEUR-POSSEDE-FUSAIN-MEDAILLES}");
	SET_DIAL_VARIABLE(104, "{JOUEUR-MONTRE-FUSAIN-MEDAILLES}");
	SET_DIAL_VARIABLE(105, "{JOUEUR-PRESENTE-OBJET-HUISSIER}");
	SET_DIAL_VARIABLE(106, "{JOUEUR-APPROCHE-MADAME-MAINTENON}");
	SET_DIAL_VARIABLE(107, "{JOUEUR-DONNE-REPAS}");
	SET_DIAL_VARIABLE(108, "{JOUEUR-TROUVE-PLANS-VAUBAN}");
	SET_DIAL_VARIABLE(109, "{JOUEUR-ALLER-BUREAU-LOUVOIS}");
	SET_DIAL_VARIABLE(110, "{JOUEUR-MONTRE-PAMPHLET-RELIGION}");
	SET_DIAL_VARIABLE(111, "{JOUEUR-MONTRE-PAMPHLET-GOUVERNEMENT}");
	SET_DIAL_VARIABLE(112, "{JOUEUR-MONTRE-PAPIER-CROISSY}");
	SET_DIAL_VARIABLE(113, "{JOUEUR-MONTRE-ECROUELLES}");
	SET_DIAL_VARIABLE(114, "{LACHAIZE-TIENT-TEXTE}");
	SET_DIAL_VARIABLE(115, "{JOUEUR-VU-PLANS-SALON-DIANE}");
	SET_DIAL_VARIABLE(116, "{JOUEUR-VU-MEMORANDUM-DANS-LUSTRE-DU-SALON-DE-LA-GUERRE}");
	SET_DIAL_VARIABLE(117, "{JOUEUR-VU-MEMORANDUM-DANS-LUSTRE-DU-SALON-APOLLON}");
	SET_DIAL_VARIABLE(118, "{JOUEUR-MONTRE-MEMORANDUM}");
	SET_DIAL_VARIABLE(119, "{JOUEUR-POSSEDE-CLEF-3-ET-4}");
	SET_DIAL_VARIABLE(120, "{JOUEUR-DONNE-SIROP-DE-ROSE}");
	SET_DIAL_VARIABLE(121, "{JOUEUR-DONNE-AUTRE-MEDICAMENT}");
	SET_DIAL_VARIABLE(122, "{DUC_MAIN_A_PARLE}");
	SET_DIAL_VARIABLE(123, "{LEVEL1_FINI}");
	SET_DIAL_VARIABLE(124, "{LEVEL2_FINI}");
	SET_DIAL_VARIABLE(125, "{LEVEL3_FINI}");
	SET_DIAL_VARIABLE(126, "{LEVEL4_FINI}");
	SET_DIAL_VARIABLE(127, "{LEVEL5_FINI}");
	SET_DIAL_VARIABLE(128, "{LEVEL6_FINI}");
	SET_DIAL_VARIABLE(129, "{LEVEL7_FINI}");
	SET_DIAL_VARIABLE(130, "{JOUEUR_POSSEDE_PAMPHLET_ARCHI}");
	SET_DIAL_VARIABLE(131, "{FAUSSE_ESQ_OK}");
	SET_DIAL_VARIABLE(132, "{CURRENT_GAME_TIME1}");
	SET_DIAL_VARIABLE(133, "{CURRENT_GAME_TIME2}");
	SET_DIAL_VARIABLE(134, "{CURRENT_GAME_TIME3}");
	SET_DIAL_VARIABLE(135, "{CURRENT_GAME_TIME4}");
	SET_DIAL_VARIABLE(136, "{CURRENT_GAME_TIME5}");
	SET_DIAL_VARIABLE(137, "{JOUEUR_POSSEDE_EPIGRAPHE}");
#undef SET_DIAL_VARIABLE
	for (uint i = 0; i < ARRAYSIZE(kVideoSubSettings); i++) {
		const VideoSubSetting &vss = kVideoSubSettings[i];
		_dialogsMan.registerSubtitlesSettings(
		    vss.videoName,
		    DialogsManager::SubtitlesSettings(
		        vss.textLeft, vss.textTop, vss.textRight, vss.textBottom,
		        vss.drawLeft, vss.drawTop, vss.drawRight, vss.drawBottom));
	}
}

void CryOmni3DEngine_Versailles::initPlacesStates() {
	_placeStates.resize(100);
	// Reset all the objects before configuring some
	for (Common::Array<PlaceState>::iterator it = _placeStates.begin(); it != _placeStates.end();
	        it++) {
		// Useless because it's only a bunch of simple variables
		it->~PlaceState();
		new ((void *)it) PlaceState();
	}
#define SET_PLACE_STATE(id, init, filter, docImage) _placeStates[id] = PlaceState(init, filter, docImage)
#define FILTER_EVENT(level, place) &CryOmni3DEngine_Versailles::filterEventLevel ## level ## Place ## place
#define INIT_PLACE(level, place) &CryOmni3DEngine_Versailles::initPlaceLevel ## level ## Place ## place
	if (_currentLevel == 1) {
		SET_PLACE_STATE(1, nullptr, FILTER_EVENT(1, 1), "VS22");
		SET_PLACE_STATE(2, nullptr, FILTER_EVENT(1, 2), "VS20");
		SET_PLACE_STATE(3, INIT_PLACE(1, 3), FILTER_EVENT(1, 3), "VS19");
		SET_PLACE_STATE(4, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(5, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(6, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(7, nullptr, nullptr, nullptr); // Filter is a leftover
		// WORKAROUND: Missing VS21
		SET_PLACE_STATE(8, nullptr, nullptr, "VS21");
		SET_PLACE_STATE(9, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(10, nullptr, nullptr, "VS31");
		SET_PLACE_STATE(11, nullptr, nullptr, "VS31");
		SET_PLACE_STATE(12, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(13, nullptr, nullptr, "VS31");
		SET_PLACE_STATE(14, nullptr, FILTER_EVENT(1, 14), nullptr);
	} else if (_currentLevel == 2) {
		SET_PLACE_STATE(1, nullptr, FILTER_EVENT(2, 1), "VS22");
		SET_PLACE_STATE(2, nullptr, FILTER_EVENT(2, 2), "VS20");
		SET_PLACE_STATE(3, nullptr, nullptr, "VS19");
		SET_PLACE_STATE(4, nullptr, nullptr, "VS18");
		SET_PLACE_STATE(5, nullptr, FILTER_EVENT(2, 5), nullptr);
		SET_PLACE_STATE(6, nullptr, nullptr, "VS19");
		SET_PLACE_STATE(7, nullptr, nullptr, nullptr);
		// WORKAROUND: Missing VS21
		SET_PLACE_STATE(8, nullptr, nullptr, "VS21");
		SET_PLACE_STATE(9, INIT_PLACE(2, 9), FILTER_EVENT(2, 9), "VS23");
		SET_PLACE_STATE(10, nullptr, nullptr, "VS31");
		SET_PLACE_STATE(11, nullptr, FILTER_EVENT(2, 11), "VS31");
		SET_PLACE_STATE(12, nullptr, FILTER_EVENT(2, 12), "VS24");
		SET_PLACE_STATE(13, nullptr, nullptr, "VS31");
		SET_PLACE_STATE(14, nullptr, FILTER_EVENT(2, 14), nullptr);
	} else if (_currentLevel == 3) {
		SET_PLACE_STATE(1, nullptr, nullptr, "VS35");
		SET_PLACE_STATE(2, nullptr, nullptr, "VS40");
		SET_PLACE_STATE(3, nullptr, FILTER_EVENT(3, 3), "VS40");
		SET_PLACE_STATE(4, nullptr, nullptr, "VS36");
		SET_PLACE_STATE(5, nullptr, nullptr, "VS36");
		SET_PLACE_STATE(6, nullptr, nullptr, "VS30");
		SET_PLACE_STATE(7, nullptr, nullptr, "VS30");
		SET_PLACE_STATE(8, nullptr, nullptr, "VS30");
		SET_PLACE_STATE(9, nullptr, nullptr, "VS39");
		SET_PLACE_STATE(10, nullptr, FILTER_EVENT(3, 10), "VS28");
		SET_PLACE_STATE(11, nullptr, nullptr, "VS28");
		SET_PLACE_STATE(12, nullptr, nullptr, "VS30");
		SET_PLACE_STATE(13, nullptr, FILTER_EVENT(3, 13), "VS27");
		SET_PLACE_STATE(14, nullptr, nullptr, "VS26");
		SET_PLACE_STATE(15, nullptr, FILTER_EVENT(3, 15), "VS25");
		SET_PLACE_STATE(16, nullptr, nullptr, "VS24");
		SET_PLACE_STATE(17, nullptr, FILTER_EVENT(3, 17), "VS25");
		SET_PLACE_STATE(18, nullptr, FILTER_EVENT(3, 18), nullptr);
		SET_PLACE_STATE(19, nullptr, FILTER_EVENT(3, 19), "VS26");
		SET_PLACE_STATE(20, nullptr, FILTER_EVENT(3_5, 20), nullptr);
		SET_PLACE_STATE(21, nullptr, nullptr, "VS28");
		SET_PLACE_STATE(22, nullptr, FILTER_EVENT(3, 22), "VS26");
		SET_PLACE_STATE(23, nullptr, FILTER_EVENT(3, 23), nullptr);
		SET_PLACE_STATE(24, nullptr, nullptr, "VS30");
	} else if (_currentLevel == 4) {
		SET_PLACE_STATE(1, nullptr, nullptr, "VS35");
		SET_PLACE_STATE(2, nullptr, nullptr, "VS40");
		SET_PLACE_STATE(3, nullptr, nullptr, "VS40");
		SET_PLACE_STATE(4, nullptr, nullptr, "VS36");
		SET_PLACE_STATE(5, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(6, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(7, nullptr, nullptr, "VS17");
		SET_PLACE_STATE(8, nullptr, nullptr, "VS17");
		SET_PLACE_STATE(9, INIT_PLACE(4, 9), nullptr, nullptr);
		SET_PLACE_STATE(10, nullptr, FILTER_EVENT(4, 10), "VS18");
		SET_PLACE_STATE(11, nullptr, nullptr, "VS20");
		SET_PLACE_STATE(12, nullptr, FILTER_EVENT(4, 12_13_14), "VS31");
		SET_PLACE_STATE(13, nullptr, FILTER_EVENT(4, 12_13_14), "VS31");
		SET_PLACE_STATE(14, nullptr, FILTER_EVENT(4, 12_13_14), "VS31");
		SET_PLACE_STATE(15, nullptr, FILTER_EVENT(4, 15), "VS36");
		SET_PLACE_STATE(16, nullptr, FILTER_EVENT(4, 16), nullptr);
		SET_PLACE_STATE(17, nullptr, FILTER_EVENT(4, 17), nullptr);
	} else if (_currentLevel == 5) {
		SET_PLACE_STATE(1, nullptr, nullptr, "VS35");
		SET_PLACE_STATE(2, nullptr, nullptr, "VS35");
		SET_PLACE_STATE(3, nullptr, nullptr, "VS36");
		SET_PLACE_STATE(4, nullptr, nullptr, "VS36");
		SET_PLACE_STATE(5, nullptr, nullptr, "VS36");
		SET_PLACE_STATE(6, INIT_PLACE(5, 6), nullptr, "VS30");
		SET_PLACE_STATE(7, nullptr, nullptr, "VS30");
		SET_PLACE_STATE(8, nullptr, nullptr, "VS30");
		SET_PLACE_STATE(9, nullptr, FILTER_EVENT(5, 9), "VS39");
		SET_PLACE_STATE(10, nullptr, nullptr, "VS28");
		SET_PLACE_STATE(11, nullptr, nullptr, "VS16");
		SET_PLACE_STATE(12, nullptr, nullptr, "VS30");
		SET_PLACE_STATE(13, nullptr, nullptr, "VS27");
		SET_PLACE_STATE(14, nullptr, FILTER_EVENT(5, 14), "VS26");
		SET_PLACE_STATE(15, nullptr, FILTER_EVENT(5, 15), "VS25");
		SET_PLACE_STATE(16, nullptr, FILTER_EVENT(5, 16), "VS24");
		SET_PLACE_STATE(17, nullptr, nullptr, "VS25");
		SET_PLACE_STATE(18, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(19, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(20, nullptr, FILTER_EVENT(3_5, 20), nullptr);
		SET_PLACE_STATE(21, nullptr, nullptr, "VS28");
		SET_PLACE_STATE(22, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(23, nullptr, FILTER_EVENT(5, 23), nullptr);
		SET_PLACE_STATE(24, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(25, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(26, nullptr, nullptr, "VS16");
		SET_PLACE_STATE(27, nullptr, FILTER_EVENT(5, 27), "VS16");
		SET_PLACE_STATE(28, nullptr, FILTER_EVENT(5, 28), nullptr);
		SET_PLACE_STATE(29, nullptr, FILTER_EVENT(5, 29), "VS24");
		SET_PLACE_STATE(30, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(31, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(32, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(33, nullptr, FILTER_EVENT(5, 33), nullptr);
		SET_PLACE_STATE(34, nullptr, FILTER_EVENT(5, 34), nullptr);
	} else if (_currentLevel == 6) {
		SET_PLACE_STATE(1, nullptr, FILTER_EVENT(6, 1), "VS34");
		SET_PLACE_STATE(2, nullptr, FILTER_EVENT(6, Orangery), "VS32");
		SET_PLACE_STATE(3, nullptr, FILTER_EVENT(6, 3), "VS32");
		SET_PLACE_STATE(4, nullptr, FILTER_EVENT(6, Orangery), "VS32");
		SET_PLACE_STATE(5, nullptr, FILTER_EVENT(6, Orangery), "VS32");
		SET_PLACE_STATE(6, nullptr, FILTER_EVENT(6, Orangery), "VS32");
		SET_PLACE_STATE(7, nullptr, FILTER_EVENT(6, Orangery), "VS32");
		SET_PLACE_STATE(8, nullptr, FILTER_EVENT(6, Orangery), "VS32");
		SET_PLACE_STATE(9, nullptr, FILTER_EVENT(6, Orangery), "VS32");
		SET_PLACE_STATE(10, nullptr, FILTER_EVENT(6, Orangery), "VS32");
		SET_PLACE_STATE(11, nullptr, FILTER_EVENT(6, Orangery), "VS32");
		SET_PLACE_STATE(12, nullptr, FILTER_EVENT(6, Orangery), "VS32");
		SET_PLACE_STATE(13, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(14, nullptr, nullptr, "VS33"); // Filter is a leftover
		SET_PLACE_STATE(15, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(16, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(17, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(18, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(19, nullptr, FILTER_EVENT(6, 19), "VS33");
		SET_PLACE_STATE(20, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(21, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(22, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(23, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(24, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(25, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(26, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(27, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(28, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(29, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(30, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(31, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(32, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(33, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(34, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(35, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(36, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(37, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(38, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(39, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(40, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(41, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(42, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(43, nullptr, nullptr, "VS33");
		SET_PLACE_STATE(44, nullptr, nullptr, "VS33");
	} else if (_currentLevel == 7) {
		SET_PLACE_STATE(1, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(2, nullptr, FILTER_EVENT(7, 2), nullptr);
		SET_PLACE_STATE(3, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(4, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(5, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(6, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(7, nullptr, nullptr, nullptr); // Filter is a leftover
		SET_PLACE_STATE(8, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(9, nullptr, FILTER_EVENT(7, 9), nullptr);
		SET_PLACE_STATE(10, nullptr, FILTER_EVENT(7, 10_11_13), "VS31");
		SET_PLACE_STATE(11, nullptr, FILTER_EVENT(7, 10_11_13), "VS31");
		SET_PLACE_STATE(12, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(13, nullptr, FILTER_EVENT(7, 10_11_13), "VS31");
		SET_PLACE_STATE(14, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(15, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(16, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(17, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(18, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(19, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(20, nullptr, FILTER_EVENT(7, 20), nullptr);
		SET_PLACE_STATE(21, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(22, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(23, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(24, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(25, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(26, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(27, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(28, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(29, nullptr, nullptr, nullptr);
	} else if (_currentLevel == 8) {
		SET_PLACE_STATE(1, nullptr, nullptr, "VS35");
		SET_PLACE_STATE(2, nullptr, nullptr, "VS40");
		SET_PLACE_STATE(3, nullptr, nullptr, "VS40");
		SET_PLACE_STATE(4, nullptr, nullptr, "VS36");
		SET_PLACE_STATE(5, nullptr, nullptr, nullptr);
		// WORKAROUND: Missing VS30
		SET_PLACE_STATE(6, nullptr, nullptr, "VS30");
		SET_PLACE_STATE(7, nullptr, nullptr, "VS30");
		SET_PLACE_STATE(8, nullptr, nullptr, "VS30");
		SET_PLACE_STATE(9, nullptr, nullptr, "VS39");
		SET_PLACE_STATE(10, nullptr, nullptr, "VS28");
		SET_PLACE_STATE(11, nullptr, nullptr, "VS16");
		SET_PLACE_STATE(12, nullptr, nullptr, "VS30");
		SET_PLACE_STATE(13, nullptr, nullptr, "VS27");
		SET_PLACE_STATE(14, nullptr, nullptr, "VS26");
		SET_PLACE_STATE(15, nullptr, nullptr, "VS25");
		SET_PLACE_STATE(16, nullptr, nullptr, "VS24");
		SET_PLACE_STATE(17, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(18, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(19, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(20, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(21, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(22, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(23, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(24, nullptr, nullptr, "VS30");
		SET_PLACE_STATE(25, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(26, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(27, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(28, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(29, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(30, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(31, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(32, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(33, nullptr, nullptr, "VS31");
		SET_PLACE_STATE(34, nullptr, nullptr, "VS31");
		SET_PLACE_STATE(35, nullptr, nullptr, "VS31");
		SET_PLACE_STATE(36, nullptr, nullptr, "VS23");
		SET_PLACE_STATE(37, nullptr, nullptr, "VS22");
		SET_PLACE_STATE(38, nullptr, nullptr, "VS20");
		// WORKAROUND: In original game VS19 is in 49 and should be in 39
		SET_PLACE_STATE(39, nullptr, nullptr, "VS19");
		SET_PLACE_STATE(40, nullptr, nullptr, "VS18");
		SET_PLACE_STATE(41, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(42, nullptr, nullptr, "VS17");
		SET_PLACE_STATE(43, nullptr, nullptr, "VS17");
		SET_PLACE_STATE(44, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(45, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(46, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(47, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(48, nullptr, nullptr, nullptr);
		SET_PLACE_STATE(49, nullptr, nullptr, nullptr);
	}
#undef INIT_PLACE
#undef FILTER_EVENT
#undef SET_PLACE_STATE
}

void CryOmni3DEngine_Versailles::setupLevelActionsMask() {
	_actionMasks.clear();
#define SET_MASK(placeId, placeState, oldActionId, newActionId) _actionMasks[PlaceStateActionKey(placeId, placeState, oldActionId)] = newActionId
	if (_currentLevel == 1) {
		SET_MASK(1, 0, 11015, 0);
		SET_MASK(1, 0, 21015, 0);
		SET_MASK(1, 1, 21011, 0);
		SET_MASK(1, 1, 21012, 0);
		SET_MASK(1, 1, 21013, 0);
		SET_MASK(1, 1, 21014, 0);
		//  2, 0 is empty
		SET_MASK(2, 1, 51201, 0);
		SET_MASK(2, 1, 21202, 0);
		SET_MASK(2, 1, 21203, 0);
		SET_MASK(2, 2, 51201, 0);
		SET_MASK(2, 2, 21202, 0);
		SET_MASK(2, 2, 21203, 0);
		SET_MASK(2, 2, 11201, 1);
		SET_MASK(2, 2, 21201, 1);
		//  3, 0 is empty
		SET_MASK(3, 1, 11301, 0);
		SET_MASK(3, 1, 21301, 0);
		// 14, 0 is empty
		SET_MASK(14, 1, 31141, 0);
	} else if (_currentLevel == 2) {
		//  1, 0 is empty
		SET_MASK(1, 1, 12101, 0);
		SET_MASK(1, 1, 22101, 0);
		SET_MASK(11, 0, 12111, 0);
		SET_MASK(11, 0, 22111, 0);
		// 11, 1 is empty
		//  9, 0 is empty
		SET_MASK(9, 1, 52903, 0);
		SET_MASK(9, 1, 22903, 0);
		SET_MASK(9, 1, 52902, 12902);
		SET_MASK(9, 2, 52903, 0);
		SET_MASK(9, 2, 22903, 0);
		SET_MASK(9, 2, 52902, 0);
		SET_MASK(9, 2, 22902, 0);
	} else if (_currentLevel == 3) {
		SET_MASK(13, 0, 13131, 0);
		SET_MASK(13, 0, 23131, 0);
		SET_MASK(13, 1, 13131, 0);
		SET_MASK(13, 1, 23131, 0);
		SET_MASK(13, 1, 33130, 0);
		// 13, 2 is empty
		SET_MASK(13, 3, 33130, 0);
		// 14, 0 is empty
		SET_MASK(14, 1, 23220, 0);
		SET_MASK(15, 0, 13151, 43154);
		SET_MASK(15, 0, 23151, 0);
		// 15, 1 is empty
		SET_MASK(17, 0, 13151, 0);
		SET_MASK(17, 0, 23151, 0);
		// 17, 1 is empty
		// 16, 0 is empty
		SET_MASK(16, 1, 43160, 0);
		// 19, 0 is empty
		SET_MASK(19, 1, 43190, 0);
		SET_MASK(22, 0, 33220, 0);
		SET_MASK(22, 1, 13220, 0);
		SET_MASK(22, 1, 23220, 0);
		SET_MASK(22, 2, 13220, 0);
		SET_MASK(22, 2, 23220, 0);
		SET_MASK(22, 2, 33220, 0);
	} else if (_currentLevel == 4) {
		// 16, 0 is empty
		SET_MASK(16, 1, 14161, 0);
		SET_MASK(16, 1, 24161, 0);
		SET_MASK(16, 2, 14161, 0);
		SET_MASK(16, 2, 24161, 0);
		// 17, 0 is empty
		SET_MASK(17, 1, 34171, 0);
		// 7, 0 is empty
		SET_MASK(7, 1, 24081, 0);
		SET_MASK(7, 1, 44071, 0);
		// 8, 0 is empty
		SET_MASK(8, 1, 24081, 0);
		SET_MASK(9, 0, 54091, 0);
		SET_MASK(9, 0, 14091, 0);
		SET_MASK(9, 0, 24092, 0);
		SET_MASK(9, 0, 24091, 0);
		SET_MASK(9, 1, 14091, 54091);
		SET_MASK(9, 2, 54091, 24091);
		SET_MASK(10, 0, 14105, 0);
		SET_MASK(10, 0, 24105, 0);
		SET_MASK(10, 0, 24106, 0);
		SET_MASK(10, 0, 24107, 0);
		SET_MASK(10, 0, 54106, 0);
		//SET_MASK(10, 0, 54106, 0);
		SET_MASK(10, 1, 24106, 0);
		SET_MASK(10, 1, 24107, 0);
		SET_MASK(10, 1, 54106, 0);
		//SET_MASK(10, 1, 54106, 0);
		SET_MASK(10, 2, 14104, 24104);
	} else if (_currentLevel == 5) {
		SET_MASK(27, 0, 15270, 0);
		SET_MASK(27, 0, 25270, 0);
		SET_MASK(27, 1, 15270, 0);
		SET_MASK(27, 1, 25270, 0);
		SET_MASK(27, 1, 35270, 0);
		SET_MASK(27, 2, 35270, 0);
		SET_MASK(9, 0, 15090, 0);
		SET_MASK(9, 0, 25090, 0);
		// 9, 1 is empty
		// 13, 0 is empty
		SET_MASK(13, 1, 25130, 0);
		SET_MASK(13, 1, 25131, 0);
		SET_MASK(13, 1, 55130, 0);
		SET_MASK(13, 1, 55131, 0);
		SET_MASK(16, 0, 28, 0);
		SET_MASK(16, 0, 35162, 0);
		SET_MASK(16, 1, 28, 0);
		SET_MASK(16, 1, 35162, 0);
		SET_MASK(16, 1, 25160, 0);
		SET_MASK(16, 1, 35161, 0);
		SET_MASK(16, 2, 28, 0);
		SET_MASK(16, 2, 35162, 0);
		SET_MASK(16, 2, 25160, 0);
		SET_MASK(16, 3, 35162, 28);
		SET_MASK(16, 3, 25160, 0);
		SET_MASK(16, 3, 35161, 0);
		SET_MASK(16, 3, 35160, 0);
		SET_MASK(16, 4, 35162, 28);
		SET_MASK(16, 4, 25160, 0);
		SET_MASK(16, 5, 25160, 0);
		SET_MASK(16, 5, 35160, 0);
		SET_MASK(16, 6, 35162, 28);
		SET_MASK(16, 6, 25160, 0);
		SET_MASK(16, 6, 35161, 0);
		SET_MASK(16, 6, 35160, 0);
		// 29, 0 is empty
		SET_MASK(29, 1, 35290, 0);
		SET_MASK(15, 0, 15090, 43154);
		SET_MASK(15, 0, 25090, 0);
		// 15, 1 is empty
		SET_MASK(17, 0, 15090, 0);
		SET_MASK(17, 0, 25090, 0);
		// 17, 1 is empty
		// 33, 0 is empty
		SET_MASK(33, 1, 35330, 0);
	} else if (_currentLevel == 6) {
		//  1, 0 is empty
		SET_MASK(1, 1, 36010, 0);
		//  3, 0 is empty
		SET_MASK(3, 1, 36030, 0);
		SET_MASK(14, 0, 26140, 0);
		SET_MASK(14, 0, 16140, 0);
		//  14, 1 is empty
		SET_MASK(19, 0, 36190, 0);
		SET_MASK(19, 1, 16190, 0);
		SET_MASK(19, 1, 26190, 0);
		SET_MASK(19, 2, 36190, 0);
		SET_MASK(19, 2, 16190, 0);
		SET_MASK(19, 2, 26190, 0);
	} else if (_currentLevel == 7) {
		//  9, 0 is empty
		SET_MASK(9, 1, 37090, 0);
	} else if (_currentLevel == 8) {
		// Nothing to mask
	} else {
		error("Invalid level");
	}
#undef SET_MASK
}

void CryOmni3DEngine_Versailles::initWhoSpeaksWhere() {
	_whoSpeaksWhere.clear();
#define SET_WHO(placeId, actionId, dialog) _whoSpeaksWhere[PlaceActionKey(placeId, actionId)] = dialog
	if (_currentLevel == 1) {
		SET_WHO(1, 11015, "13F_HUI");
		SET_WHO(1, 12101, "21F_BON");
		SET_WHO(1, 52903, "21G_DAU");
		SET_WHO(1, 52902, "21G_DAU");
		SET_WHO(2, 11201, "11E_HUI");
		SET_WHO(2, 51201, "11E_RAC");
		SET_WHO(3, 11301, "11D_LEB");
		SET_WHO(5, 12501, "21B1_HUI");
		if (currentGameTime() >= 2) {
			SET_WHO(2, 11201, "12E_HUI");
		}
	} else if (_currentLevel == 2) {
		SET_WHO(1, 12101, "21F_BON");
		SET_WHO(9, 52903, "21G_DAU");
		SET_WHO(9, 52902, "21G_DAU");
		SET_WHO(9, 12902, "22G_DAU");
		SET_WHO(9, 11201, "11E_HUI");
		SET_WHO(9, 12901, "21G_HUI");
		SET_WHO(5, 12501, "21B1_HUI");
		SET_WHO(10, 12130, "21Z_ALI");
		SET_WHO(10, 12130, "21Z_MON");
		SET_WHO(10, 12111, "24Z_BON");
		SET_WHO(11, 12130, "21Z_MON");
		SET_WHO(11, 12111, "24Z_BON");
		SET_WHO(13, 12130, "21Z_ALI");
		SET_WHO(13, 12130, "21Z_MON");
		SET_WHO(13, 12111, "24Z_BON");
		SET_WHO(12, 12121, "23I_LEB");
		SET_WHO(10, 52130, "21Z_ALI");
		SET_WHO(11, 52130, "21Z_ALI");
		SET_WHO(13, 52130, "21Z_ALI");
		SET_WHO(10, 52101, "21Z_MON");
		if (currentGameTime() >= 2) {
			SET_WHO(9, 52902, "22G_DAU");
		}
	} else if (_currentLevel == 3) {
		SET_WHO(13, 13130, "31M_SUI");
		SET_WHO(13, 13131, "32M_MR");
		SET_WHO(10, 13100, "31O_SUIA");
		SET_WHO(10, 13101, "31O_SUIP");
		SET_WHO(22, 13220, "31L1_LUL");
		SET_WHO(6, 13060, "31Q_SUI");
		SET_WHO(15, 13150, "31J_SUI");
		SET_WHO(17, 13150, "31J_SUI");
		SET_WHO(3, 13030, "31X_BON");
		SET_WHO(24, 53240, "32Q_MON");
		SET_WHO(24, 13241, "32Q_RAC2");
		SET_WHO(4, 53041, "31X_SEI");
		SET_WHO(4, 53040, "31X_LOU");
		SET_WHO(15, 13151, "32J_CRO");
		SET_WHO(17, 13151, "32J_CRO");
	} else if (_currentLevel == 4) {
		SET_WHO(10, 14104, "41C_HUI");
		SET_WHO(10, 14105, "42C_BON");
		SET_WHO(16, 14161, "41X2_CRO");
		SET_WHO(10, 54106, "43C_CON");
		SET_WHO(10, 54106, "43C_DUR");
		SET_WHO(9, 54091, "43B1_SEI");
		SET_WHO(9, 14091, "43B1_SEI");
		if (currentGameTime() >= 4) {
			SET_WHO(9, 54091, "4_MAI");
			SET_WHO(9, 14091, "4_MAI");
		}
	} else if (_currentLevel == 5) {
		SET_WHO(27, 15270, "52A4_LAC");
		SET_WHO(9, 15090, "53N_BON");
		SET_WHO(13, 55130, "51M_MAN");
		SET_WHO(13, 55131, "51M_MAN");
		SET_WHO(14, 55140, "52L_LOU");
		SET_WHO(14, 55140, "52L_PRI");
		SET_WHO(14, 15142, "52L_BOU");
		SET_WHO(13, 13130, "53M_SUI");
		if (currentGameTime() >= 4) {
			SET_WHO(9, 15090, "54I_BON");
		}
	} else if (_currentLevel == 6) {
		SET_WHO(9, 16090, "61_LEN");
		SET_WHO(19, 16190, "61_DUC");
		SET_WHO(14, 16140, "61_BON");
		if (_gameVariables[GameVariables::kMaineTalked] == 1) {
			SET_WHO(19, 16190, "62_DUC");
		}
	}
#undef SET_WHO
}

void CryOmni3DEngine_Versailles::initDocPeopleRecord() {
	_docPeopleRecord.clear();
#define SET_INFO(actionId, record) _docPeopleRecord[actionId] = record
	SET_INFO(22501, "VC25");
	SET_INFO(22401, "VC19");
	SET_INFO(22402, "VC24");
	SET_INFO(22403, "VC24");
	SET_INFO(22404, "VC24");
	SET_INFO(22405, "VC24");
	SET_INFO(22406, "VC24");
	SET_INFO(22407, "VC24");
	SET_INFO(22408, "VC24");
	SET_INFO(21201, "VC25");
	SET_INFO(21202, "VS12");
	SET_INFO(21203, "VA13");
	SET_INFO(21011, "VC13");
	SET_INFO(21012, "VC11");
	SET_INFO(21013, "VC10");
	SET_INFO(21014, "VC18");
	SET_INFO(22901, "VC25");
	SET_INFO(21015, "VC25");
	SET_INFO(22101, "VC18");
	SET_INFO(22903, "VC12");
	SET_INFO(22902, "VC10");
	SET_INFO(22131, "VC16");
	SET_INFO(22111, "VC18");
	SET_INFO(21301, "VA12");
	SET_INFO(22121, "VA12");
	SET_INFO(22103, "VC20");
	SET_INFO(22102, "VC15");
	SET_INFO(23100, "VC23");
	SET_INFO(23101, "VC23");
	SET_INFO(23130, "VC23");
	SET_INFO(23060, "VC23");
	SET_INFO(23150, "VC23");
	SET_INFO(23220, "VA11");
	SET_INFO(23131, "VC11");
	SET_INFO(23241, "VA13");
	SET_INFO(23151, "VR12");
	SET_INFO(23030, "VC18");
	SET_INFO(23040, "VR11");
	SET_INFO(23041, "VR13");
	SET_INFO(23240, "VC15");
	SET_INFO(24104, "VC25");
	SET_INFO(24105, "VC18");
	SET_INFO(24106, "VC12");
	SET_INFO(24107, "VC19");
	SET_INFO(24102, "VC21");
	SET_INFO(24103, "VC21");
	SET_INFO(24081, "VC21");
	SET_INFO(24101, "VC24");
	SET_INFO(24092, "VC14");
	SET_INFO(24091, "VR13");
	SET_INFO(24161, "VR12");
	SET_INFO(25270, "VC26");
	SET_INFO(25261, "VC26");
	//SET_INFO(25260, nullptr); // Don't put empty records
	SET_INFO(25130, "VA12");
	SET_INFO(25131, "VS12");
	SET_INFO(25060, "VC23");
	SET_INFO(25061, "VC22");
	SET_INFO(25160, "VC23");
	SET_INFO(25140, "VR11");
	SET_INFO(25141, "VC16");
	SET_INFO(25142, "VC20");
	SET_INFO(25143, "VC15");
	SET_INFO(25145, "VC17");
	SET_INFO(25090, "VC18");
	SET_INFO(26190, "VC13");
	SET_INFO(24161, "VR12");
	SET_INFO(26090, "VS13");
	SET_INFO(26140, "VC18");
	SET_INFO(27111, "VC21");
#undef SET_INFO
}

} // End of namespace Versailles
} // End of namespace CryOmni3D
