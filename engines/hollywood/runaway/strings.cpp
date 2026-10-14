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

// German, French and Spanish executable interface labels, stored as UTF-8.
const char *const kText[RunawayStrings::kTextCount][3] = {
	{"BETRACHTE ", "REGARDER ", "MIRAR "},
	{"NIMM ", "PRENDRE ", "COGER "},
	{"BENUTZE ", "UTILISER ", "USAR "},
	{"VERSTAUE ", "DEPOSER ", "DEJAR "},
	{" MIT ", " AVEC ", " CON "},
	{"INVENTAR", "INVENTAIRE", "INVENTARIO"},
	{"WEITERSPIELEN", "CONTINUER LA PARTIE", "CONTINUAR JUEGO"},
	{"\303\226FFNE ", "OUVRIR ", "ABRIR "},
	{"SCHLIESSE ", "FERMER ", "CERRAR "},
	{"VERSCHIEBE ", "DEPLACER ", "MOVER "},
	{"SIEH AUS DEM ", "REGARDER A TRAVERS ", "MIRAR A TRAV\303\211S DE LA "},
	{"BENUTZE DAS ", "APPELER AVEC ", "LLAMAR POR "},
	{"NIMM ETWAS AUS DEM ", "PRENDRE QUELQUE CHOSE DANS ", "COGER ALGO DE LA "},
	{"ZIEHE DEN STECKER DER K\303\234HLTRUHE HERAUS", "DEBRANCHER CONGELATEUR", "DESENCHUFAR CONGELADOR"},
	{"HEMD", "CHEMISE", "CAMISA"},
	{"PUTZLAPPEN", "PEAU DE CHAMOIS HUMIDE", "GAMUZA HUMEDECIDA"},
	{"SCHMUTZLAPPEN", "PEAU DE CHAMOIS SALE", "GAMUZA MUGRIENTA"},
	{"OPTIONEN", "OPTIONS", "OPCIONES"},
	{"LADEN", "CHARGER", "CARGAR"},
	{"SICHERN", "SAUVEGARDER", "SALVAR"},
	{"L\303\226SCHEN", "EFFACER", "BORRAR"},
	{"WEITERSPIELEN", "JOUER", "JUGAR"},
	{"VERLASSEN", "QUITTER", "SALIR"},
	{"MUSIK", "Musique", "M\303\272sica"},
	{"ANTI-ALIASING", "Antialiasing des sprites", "Antidentado Sprite"},
	{"SCROLLING", "Mode de d\303\251filement", "M\303\251todo Scroll"},
	{"MAUSZEIGERKLICKEN", "Bruitage des ic\303\264nes", "Sonido puntero rat\303\263n"},
	{"UNTERTITEL", "Sous-titres", "Subt\303\255tulos"},
	{"EIN", "OUI", "ON"},
	{"AUS", "NON", "OFF"},
	{"SCHNELL", "RAPIDE", "R\303\201PIDO"},
	{"LANGSAM", "FLUIDE", "LENTO"},
	{"LAUTST\303\204RKE", "Volume", "Volumen"},
	{"HELLIGKEIT", "Luminosit\303\251", "Brillo"},
	{"MITWIRKENDE", "G\303\251n\303\251rique", "Cr\303\251ditos"},
	{"EINSTELLUNGEN SICHERN", "Sauvegarder la configuration", "Salvar Configuraci\303\263n"},
	{"VORGABEEINSTELLUNGEN", "Configuration par d\303\251faut", "Configuraci\303\263n Inicial"},
	{"SIND SIE SICHER?", "CONFIRMER ?", "\302\277EST\303\201S SEGURO?"},
	{"JA", "OUI", "S\303\215"},
	{"NEIN", "NON", "NO"},
	{"V", "Q", "L"},
	{"STEIGE DURCH DIE FALLT\303\234R HINAB", "SORTIR PAR LA TRAPPE", "SALIR POR LA TRAMPILLA"}
};

const char *const kItemNames[][3] = {
	{"", "", ""},
	{"LEDERLAPPEN", "PEAU DE CHAMOIS", "GAMUZA"},
	{"BRECHSTANGE", "PIED DE BICHE", "PALANCA"},
	{"SPR\303\234HREINIGER", "PRODUIT NETTOYANT", "BOTE DE LIMPIEZA"},
	{"PACKUNG BUTTER", "TABLETTE DE BEURRE", "PAQUETE DE MANTEQUILLA"},
	{"SPRENGAUSR\303\234STUNG", "DYNAMITE ET DETONATEUR", "EQUIPO DE DINAMITERO"},
	{"HELM AUS DEM ZWEITEN WELTKRIEG", "CASQUE DE LA SECONDE GUERRE MONDIALE", "CASCO DE LA SEGUNDA GUERRA MUNDIAL"},
	{"SCHRAUBE", "VIS", "TORNILLO"},
	{"GARDEROBENKNAUF", "PORTE-MANTEAUX", "POMO DEL PERCHERO"},
	{"TABLETTE", "PILLULE", "PASTILLA"},
	{"LIPPENSTIFT", "ROUGE A LEVRES", "PINTALABIOS"},
	{"HAUFEN LIPPENSTIFTE", "STOCK DE ROUGE A LEVRES", "MONT\303\223N DE PINTALABIOS"},
	{"SONNEN\303\226L", "HUILE SOLAIRE", "ACEITE BRONCEADOR"},
	{"WALKIE-TALKIE", "TALKIE-WALKIE", "WALKIE TALKIE"},
	{"MUNITIONSSTREIFEN", "BANDE DE MUNITIONS", "RISTRA DE BALAS"},
	{"BASKETBALL", "BALLON DE BASKET", "BAL\303\223N DE BALONCESTO"},
	{"SATTLERNADEL UND FADEN", "FIL A COUDRE ET AIGUILLE", "AGUJA Y ROLLO DE HILO DE ZAPATERO"},
	{"BEHELFS-BOHRER", "TOURNEVIS ARTISANAL", "SACACORCHOS IMPROVISADO"},
	{"SACK GESCH\303\204LTER ERDN\303\234SSE", "SAC DE CACAHOUETES ENTIERES", "SAQUITO DE CACAHUETES PELADOS"},
	{"GLAS ERDNUSSBUTTER", "POT DE BEURRE DE CACAHOUETES", "TARRO DE MANTEQUILLA DE CACAHUETE"},
	{"SONNENBRILLE", "LUNETTES DE SOLEIL", "GAFAS DE SOL"},
	{"FUSSRASTE", "REPOSE-PIED DE MOTO", "ESTRIBO DE LA MOTO"},
	{"HANDSTAUBSAUGER", "ASPIRATEUR A MAIN", "ASPIRADORA DE MANO"},
	{"PLAN", "PLAN GENIAL", "PLAN"},
	{"BLASEBALG", "SOUFFLET", "FUELLE"},
	{"EIMER", "SEAU", "CUBO"},
	{"SCHL\303\234SSEL", "CLE DU REFRIGERATEUR", "LLAVE"}
};

const char *const kShackHotspotNames[][3] = {
	{"", "", ""},
	{"T\303\234R", "PORTE", "PUERTA"},
	{"SPALT", "ORIFICE", "RENDIJA"},
	{"FENSTER", "FENETRE", "VENTANA"},
	{"FENSTER", "PETITE FENETRE", "VENTANA PEQUE\303\221A"},
	{"HOLZST\303\234CK", "MORCEAU DE BOIS ROND", "TROZO REDONDO DE MADERA"},
	{"FALLT\303\234R", "TRAPPE", "TRAMPILLA"},
	{"STOFFFETZEN", "TISSU DECHIRE", "TELA HECHA JIRONES"},
	{"K\303\234HLTRUHE", "CONGELATEUR", "CONGELADOR"},
	{"FETZEN", "CHIFFON", "TRAPO"},
	{"STECKER", "PRISE ELECTRIQUE", "ENCHUFE"},
	{"ABFLUSS", "TUYAU DE VIDANGE DU CONGELATEUR", "DESAG\303\234E DEL CONGELADOR"},
	{"TELEFON", "TELEPHONE", "TEL\303\211FONO"},
	{"REGAL", "ETAGERES", "ESTANTER\303\215A"},
	{"BRECHSTANGE", "PIED-DE-BICHE", "PALANCA"},
	{"BRILLENRESTE", "LUNETTES CASSEES", "GAFAS ROTAS"},
	{"BLASEBALG", "SOUFFLET", "FUELLE"},
	{"DOSE", "BOUTEILLE", "BOTE"},
	{"K\303\234HLTRUHENDECKEL", "COUVERCLE DU CONGELATEUR", "TAPA DEL CONGELADOR"},
	{"ABFLUSS", "TUYAU DE VIDANGE DU CONGELATEUR", "DESAG\303\234E DEL CONGELADOR"}
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
