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
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

// German and French executable interface labels, stored as UTF-8.
const char *const kText[RunawayStrings::kTextCount][2] = {
	{"BETRACHTE ", "REGARDER "},
	{"NIMM ", "PRENDRE "},
	{"BENUTZE ", "UTILISER "},
	{"VERSTAUE ", "DEPOSER "},
	{" MIT ", " AVEC "},
	{"INVENTAR", "INVENTAIRE"},
	{"WEITERSPIELEN", "CONTINUER LA PARTIE"},
	{"\303\226FFNE ", "OUVRIR "},
	{"SCHLIESSE ", "FERMER "},
	{"VERSCHIEBE ", "DEPLACER "},
	{"SIEH AUS DEM ", "REGARDER A TRAVERS "},
	{"BENUTZE DAS ", "APPELER AVEC "},
	{"NIMM ETWAS AUS DEM ", "PRENDRE QUELQUE CHOSE DANS "},
	{"ZIEHE DEN STECKER DER K\303\234HLTRUHE HERAUS", "DEBRANCHER CONGELATEUR"},
	{"HEMD", "CHEMISE"},
	{"PUTZLAPPEN", "PEAU DE CHAMOIS HUMIDE"},
	{"SCHMUTZLAPPEN", "PEAU DE CHAMOIS SALE"}
};

const char *const kItemNames[][2] = {
	{"", ""},
	{"LEDERLAPPEN", "PEAU DE CHAMOIS"},
	{"BRECHSTANGE", "PIED DE BICHE"},
	{"SPR\303\234HREINIGER", "PRODUIT NETTOYANT"},
	{"PACKUNG BUTTER", "TABLETTE DE BEURRE"},
	{"SPRENGAUSR\303\234STUNG", "DYNAMITE ET DETONATEUR"},
	{"HELM AUS DEM ZWEITEN WELTKRIEG", "CASQUE DE LA SECONDE GUERRE MONDIALE"},
	{"SCHRAUBE", "VIS"},
	{"GARDEROBENKNAUF", "PORTE-MANTEAUX"},
	{"TABLETTE", "PILLULE"},
	{"LIPPENSTIFT", "ROUGE A LEVRES"},
	{"HAUFEN LIPPENSTIFTE", "STOCK DE ROUGE A LEVRES"},
	{"SONNEN\303\226L", "HUILE SOLAIRE"},
	{"WALKIE-TALKIE", "TALKIE-WALKIE"},
	{"MUNITIONSSTREIFEN", "BANDE DE MUNITIONS"},
	{"BASKETBALL", "BALLON DE BASKET"},
	{"SATTLERNADEL UND FADEN", "FIL A COUDRE ET AIGUILLE"},
	{"BEHELFS-BOHRER", "TOURNEVIS ARTISANAL"},
	{"SACK GESCH\303\204LTER ERDN\303\234SSE", "SAC DE CACAHOUETES ENTIERES"},
	{"GLAS ERDNUSSBUTTER", "POT DE BEURRE DE CACAHOUETES"},
	{"SONNENBRILLE", "LUNETTES DE SOLEIL"},
	{"FUSSRASTE", "REPOSE-PIED DE MOTO"},
	{"HANDSTAUBSAUGER", "ASPIRATEUR A MAIN"},
	{"PLAN", "PLAN GENIAL"},
	{"BLASEBALG", "SOUFFLET"},
	{"EIMER", "SEAU"},
	{"SCHL\303\234SSEL", "CLE DU REFRIGERATEUR"}
};

const char *const kShackHotspotNames[][2] = {
	{"", ""},
	{"T\303\234R", "PORTE"},
	{"SPALT", "ORIFICE"},
	{"FENSTER", "FENETRE"},
	{"FENSTER", "PETITE FENETRE"},
	{"HOLZST\303\234CK", "MORCEAU DE BOIS ROND"},
	{"FALLT\303\234R", "TRAPPE"},
	{"STOFFFETZEN", "TISSU DECHIRE"},
	{"K\303\234HLTRUHE", "CONGELATEUR"},
	{"FETZEN", "CHIFFON"},
	{"STECKER", "PRISE ELECTRIQUE"},
	{"ABFLUSS", "TUYAU DE VIDANGE DU CONGELATEUR"},
	{"TELEFON", "TELEPHONE"},
	{"REGAL", "ETAGERES"},
	{"BRECHSTANGE", "PIED-DE-BICHE"},
	{"BRILLENRESTE", "LUNETTES CASSEES"},
	{"BLASEBALG", "SOUFFLET"},
	{"DOSE", "BOUTEILLE"},
	{"K\303\234HLTRUHENDECKEL", "COUVERCLE DU CONGELATEUR"},
	{"ABFLUSS", "TUYAU DE VIDANGE DU CONGELATEUR"}
};

} // End of anonymous namespace

Common::U32String RunawayStrings::text(TextId id) const {
	assert(id < kTextCount);
	return Common::U32String(kText[id][_languageIndex]);
}

Common::U32String RunawayStrings::itemName(byte item) const {
	return Common::U32String(item < ARRAYSIZE(kItemNames) ? kItemNames[item][_languageIndex] : "");
}

Common::U32String RunawayStrings::shackHotspotName(byte hotspot) const {
	return Common::U32String(hotspot < ARRAYSIZE(kShackHotspotNames) ? kShackHotspotNames[hotspot][_languageIndex] : "");
}

} // End of namespace Hollywood
