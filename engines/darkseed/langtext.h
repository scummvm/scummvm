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

#ifndef DARKSEED_LANGTEXT_H
#define DARKSEED_LANGTEXT_H

namespace Darkseed {

struct I18nText {
	const char *en;
	const char *es;
	const char *fr;
	const char *de;
};

struct TextWithPosition {
	int x;
	int y;
	const char *text;
};

struct I18NTextWithPosition {
	const TextWithPosition en;
	const TextWithPosition es;
	const TextWithPosition fr;
	const TextWithPosition de;
};

const char *getI18NText(const I18nText &text);
const TextWithPosition &getI18NTextWithPosition(const I18NTextWithPosition &i18nTextWithPosition);

constexpr I18nText kI18N_CarKeysIgnitionText = {
	"You see the car keys in the ignition.",
	"VES LAS LLAVES DEL COCHE EN EL CONTACTO.",
	"VOUS VOYEZ LES CLEFS DE LA VOTTURE DAUS LE STARTER.",
	"DU SIEHSTDIE AUTOSCHLUSSEL IM ANLASSER."
};

constexpr I18nText kI18N_YouSeeIronBarsText = {
	"You see the iron bars of your cell.",
	"VES LAS BARRAS DE HIERRO DE TU CELDA.",
	"VOUS VOYEZ LES BARREAUX DE FER DE VOTRE CELLULE.",
	"DU SIEHST DIE EISENSTANGEN IN DER ZELLE."
};

constexpr I18nText kI18N_YouSeeDelbertText = {
	"You see Delbert, not much to look at.",
	"VES A DELBERT, NO HAY MUCHO QUE VER.",
	"VOUS VOYEZ DELBERT, PAS GRAND CHOSE A VOIR.",
	"DU SIEHST DELBERT, NICHT VIEL ZU SEHEN"
};

constexpr I18nText kI18N_YouSeeTheClerkText = {
	"You see the clerk.",
	"VES AL EMPLEADO.",
	"VOUS VOYEZ L'HOMME.",
	"DU SIEHST DEN MANN."
};

constexpr I18nText kI18N_YouSeeTheOpenGloveBoxText = {
	"You see the open glove box.",
	"VES LA CAJA DE LOS GUANTES ABIERTA.",
	"VOUS VOYEZ LA BOITE DE GANT OUVERTE.",
	"DU SIEHST DAS OFFENE HANDSCHUHFACH."
};

constexpr I18nText kI18N_youSeeTheText = {
  "You see the",
  "VES",
  "VOUS VOYEZ",
  "DU SIEHST"
};

constexpr I18nText kI18N_TheCopsIgnoreYourDemandsText = {
  "The cops ignore your demands for attention.",
  "LOS POLICIAS IGNORAN TUS LLAMADAS DE ATENCION.",
  "LES FLICS IGNORENT VOTRE DEMANDE D'ATTENTION.",
  "DIE POLIZISTEN BEACHTEN NICHT DEINE FORDERUNGEN."
};

constexpr I18nText kI18N_ThePhoneIsRingingText = {
  "The phone is ringing.",
  "EL TELEFONO ESTA SONANDO.",
  "LE TELEPHONE SONNE.",
  "DAS TELEFON KLINGELT."
};

constexpr I18nText kI18N_TheDoorbellIsRingingText = {
  "The doorbell is ringing.",
  "EL TIMBRE DE LA PUERTA ESTA SONANDO.",
  "LA SONETTE DE LA PORTE SONNE.",
  "DIE TUERKLINGEL LAEUTET."
};

constexpr I18nText kI18N_ChooseAnItemBeforeText = {
  "Choose an item before giving clerk more money.",
  "ELIGE UN OBJETO ANTES DE DARLE AL EMPLEADO MAS DINERO.",
  "CHOISISSEZ QUELQUE CHOSE AVANT DE REMETTRE L'ARGENT A VENDEUR.",
  "SUCHE ETWAS AUS BEVOR DU DEN MANN BEZAHLST."
};

constexpr I18nText kI18N_YouTouchDelbertText = {
  "You touch Delbert...",
  "TOCAS A DELBERT...",
  "VOUS TOUCHEZ DELBERT.",
  "GREIFE DELBERT AN..."
};

constexpr I18nText kI18N_YouTouchTheColdIronBarsText = {
  "You touch the cold iron bars.",
  "TOCAS LAS FRIAS BARRAS DE HIERRO.",
  "VOUS TOUCHEZ LES BARREAUX DE FER.",
  "GREIFE DIE KALTEN EISEN STANGEN AN."
};

constexpr I18nText kI18N_TheSergeantSaysNiceGunText = {
  "The sergeant says 'Nice gun eh? It's a Browning'",
  "EL SARGENTO DICE: 'BUENA PISTOLA, EH? ES UNA BROWNING.'",
  "LE SERGENT DIT: BEAU REVOLVER HEIN, C'EST UN BROWNING.",
  "DER SEARGENT SAGT 'SCHOENE PISTOLE, EH? ES IST EIN BROWNING.'"
};

constexpr I18nText kI18N_YouTurnOnTheMusicText = {
  "You turn on the music.",
  "PONES MUSICA.",
  "VOUS METTEZ LA MUSIQUE.",
  "SCHALTE DIE MUSIK AN."
};

constexpr I18nText kI18N_YouTurnOffTheMusicText = {
  "You turn off the music.",
  "QUITAS LA MUSICA.",
  "VOUS ARRETEZ LA MUSIQUE.",
  "SCHALTE DIE MUSIK AB."
};

constexpr I18nText kI18N_YouTouchTheOrnateSignalText = {
  "You touch the surface of the ornate sigil.",
  "TOCAS LA SUPERFICIE DE LA FIGURA ADORNADA.",
  "VOUS TOUCHEZ LA SURFACE DE LA PIERRE MAGIQUE.",
  "GREIFE DIE VERZAUBERTEN STEINE AN."
};

constexpr I18nText kI18N_ThisSentryCannotBeStoppedWithText = {
	"This sentry cannot be stopped with a",
	"NO PUEDES PARAR A ESTE CENTINELA CON",
	"VOUS N'ARRETEREZ PAS LA SENTINELLE AVEC",
	"DIESE WACHE KANN NICHT AUFGEHALTEN WERDEN MIT"
};

constexpr I18nText kI18N_HasNoEffectOnTheAlienTubesText = {
	"The %s has no effect on the alien tubes.",
	"%s NO TIENE EFECTO SOBRE LOS TUBOS ALIENIGENAS.",
	"%s RESTE SANS EFFET SUR LES TUBES EXTRA-TERRESTRES.",
	"%s BEWIRKT NICHTS AN AUSSERIRDISCHEN TUNNELS."
};

constexpr I18nText kI18N_YouIncinerateTheText = {
	"You incinerate the %s inside the power nexus!.",
	"INCINERAS %s DENTRO DE LA FUENTA DE ENERGIA.",
	"VOUS INCINEREZ %s A L'INTERIEUR DU LIEN DE PUISSANCE.",
	"DU VERNICHTEST %s MIT DER STROMVERSORGUNG!"
};

constexpr I18nText kI18N_HasNoEffectOnTheAlienMonstrosityText = {
	"The %s has no effect on this alien monstrosity.",
	"%s NO TIENE EFECTO SOBRE ESTA MONSTRUOSIDAD ALIENIGENA.",
	"%s RESTE SANS EFFET SUR CETTE MONSTRUOSITE EXTRA-TERRESTRE.",
	"%s BEWIRKT NICHTS AM AUSSERIRDISCHEN MONSTRUM."
};

constexpr I18nText kI18N_cutTheWiresText = {
	"The %s is not strong enough to cut the cables.\nPerhaps if you had some wire cutters?",
	"%s NO ES LO BASTANTE RESISTENTE COMO PARA CORTAR LOS CABLES.\nY SI TUVIERAS UN CORTACABLES?",
	"%s NE POURRA PAS COUPER LES CABLES.\nET SI VOUS AVIEZ DES PINCES COUPANTES?",
	"%s IST NICHT STARK GENUG, UM DIE KABEL ABZUSCHNEIDEN.\nHAST DU VIELLEICHT EINEN KABELSCHNEIDER?"
};

constexpr I18nText kI18N_NoEffectOnTheProtectedAncientText = {
	"The %s has no effect on the protected ancient.",
	"%s NO TIENE EFECTO SOBRE EL ANTEPASADO PROTEGIDO.",
	"%s N'A AUCUN EFFET SUR L'ANCIEN PROTEGE.",
	"%s BEWIRKT NICHTS BEI DEM GESCHUETZTEN ALTEN."
};

constexpr I18nText kI18N_YouHideTheObjectUnderThePillowText = {
	"You hide the %s under the pillow.",
	"OCULTAS %s BAJO LA ALMOHADA.",
	"VOUS CACHEZ %s SOUS L'OREILLER.",
	"DU VERSTECKST %s UNTER DEM KISSEN."
};

constexpr I18nText kI18N_DontPutTheObjectInTheTrunkText = {
	"Don't put the %s in the trunk, you'll forget it.",
	"NO PONGAS %s EN EL BAUL, TE OLVIDARAS.",
	"NE METTEZ PAS %s DANS LE COFFRE, VOUS L'OUBLIEREZ.",
	"WENN DU %s IN DEN TRUHE LEGST, VERGISST DU ES." // TODO check this is correct.
};

constexpr I18nText kI18N_TheCarWontStartWithTheText = {
	"The car won't start with the %s.",
	"EL COCHE NO SE PONDRA EN MARCHA CON %s.",
	"LA VOITURE NE DEMARRERA PAS AVEC %s.",
	"DAS AUTO SPRINGT NICHT AN MIT %s."
};

constexpr I18nText kI18N_IfYouPutTheObjectInTheTrunkText = {
	"If you put the %s in the trunk, you'll forget it.",
	"SI PONES %s EN EL BAUL, TE OLVIDARAS.",
	"SI VOUS METTEZ %s DANS LE COFFRE, VOUS L'OUBLIEREZ.",
	"WENN DU %s IN DEN TRUHE LEGST, VERGISST DU ES."
};

constexpr I18nText kI18N_TheObjectIsYoursYouHaventLostItText = {
	"The %s is yours, you haven't lost it.",
	"%s ES TUYA, NO LA HASA PERDIDO.",
	"%s EST A VOUS, VOUS NE L'AVEZ PAS PERDUE.",
	"%s GEHOERT DIR, DU HAST ES NICHT VERLOREN."
};

constexpr I18nText kI18N_notAGoodPlaceToHideTheText = {
	"Not a good place to hide the %s.",
	"NO ES UN BUEN SITIO PARA OCULTAR %S.",
	"VOUS NE POURREZ PAS CACHER %S.",
	"KEIN GUTES VERSTECK FUER %S."
};

constexpr I18nText kI18N_youTryToPutTheObjectInsideButTheDoorWontOpenText = {
	"You try to put the %s inside, but the door won't open",
	"INTENTAS PONER %s DENTRO, PERO LA PUERTA NO SE ABRIRA.",
	"VOUS ESSAYEZ DE METTRE %s A L'INTERIEUR, MAIS LA PORTE NE S'OUVRIRA PAS.",
	"DU VERSUCHST, %s HINEINZUSTELLEN, ABER DIE TUER OEFFNET SICH NICHT."
};

constexpr I18nText kI18N_theKitchenIsNoPlaceToKeepTheText = {
	"The kitchen is no place to keep the %s.",
	"LA COCINA NO ES LUGAR PARA GUARDAR %s.",
	"LA CUISINE NE CONVIENT PAS A %s.",
	"DIE KUECHE IST KEIN PLATZ FUER %s."
};

constexpr I18nText kI18N_youllForgetTheObjectHereText = {
	"You'll forget the %s here.",
	"OLIVIDARAS %s AQUI.",
	"VOUS OUBLIEREZ %s ICI.",
	"DU WIRST NOCH %s HIER VERGESSEN."
};

constexpr I18nText kI18N_youdRatherHaveTheObjectWithYouText = {
	"You'd rather have the %s with you.",
	"SERIA MEJOR TENER %s CONTIGO.",
	"IL VAUDRAIT MIEUX AVOIR %s SUR VOUS.",
	"DU HAETTEST LIEBER %s BEI DIR."
};

constexpr I18nText kI18N_theObjectHasNoEffectText = {
	"The %s has no effect.",
	"%s NO TIENE EFECTO.",
	"%s N'A AUCUN EFFET.",
	"%s BEWIRKT NICHTS."
};

constexpr I18nText kI18N_thisIsNotAGoodPlaceForTheText = {
	"This is not a good place for the %s.",
	"NO ES UN BUEN SITIO PARA %s.",
	"CE N'EST PAS UN BON ENDROIT POUR %s.",
	"DAS IST KEIN GUTER PLATZ FUER %s."
};

constexpr I18nText kI18N_youSeeAReflectionOfTheText = {
	"You see a reflection of the %s.",
	"VES UN REFLEJO DE %s.",
	"VOUS VOYEZ UN REFLET REPRESENTANT %s.",
	"DU SIEHST EIN SPIEGELBILD VON %s."
};

constexpr I18nText kI18N_youDontWantToLeaveTheObjectUnderTheBedText = {
	"You don't want to leave the %s under the bed.",
	"NO QUIERES DEJAR %s DEBAJO DE LA CAMA.",
	"POURQUOI LAISSERIEZ-VOUS %s SOUS LE LIT?",
	"DU WILLST %s NICHT UNTER DEM BETT LASSEN."
};

constexpr I18nText kI18N_genResponse0_usingTheObjectOnTheObjectItMustBeYourHeadachesText = {
	"Using the %s on the %s doesn't make any sense, it must be your headaches!",
	"UTILIZAR %s EN %s NO TIENE SENTIDO, TE PRODUCIRA DOLOR DE CABEZA",
	"POURQUOI UTILISER %s SUR %s PRENEZ DONC DE L'ASPIRINE!",
	"GEBRAUCHEN VON %s AUF %s MACHT KEINEN SINN, DAS MUSS AN DEINEN KOPFSCHMERZEN LIEGEN!"
};

constexpr I18nText kI18N_genResponse1_theObjectWillDoNothingToTheText = {
	"The %s will do nothing to the %s.",
	"%s NO TIENE NADA QUE VER CON %s.",
	"%s NE FERONT RIEN A %s.",
	"%s HAT NICHTS ZU TUN MIT %s"
};

constexpr I18nText kI18N_genResponse2_theObjectDoesntHaveAnyEffectOnTheText = {
	"The %s doesn't have any effect on the %s.",
	"%s NO LE CAUSARA EFECTO A %s.",
	"%s N'AURA AUCUN EFFET SUR %s.",
	"%s BEWIRKT NICHTS MIT %s."
};

constexpr I18nText kI18N_genResponse3_theObjectHasNothingToDoWithTheText = {
	"The %s has nothing to do with %s.",
	"%s NO TIENE NADA QUE VER CON %s.",
	"%s N'A AUCUN RAPPORT AVEC %s.",
	"%s HAT NICHTS ZU TUN MIT %s."
};

constexpr I18nText kI18N_genResponse4_areYouFeelingAlrightText = {
	"Are you feeling alright?",
	"TE ENCUENTRAS BIEN?",
	"VOUS ETES SUR QUE CA VA BIEN?",
	"FUEHLST DU DICH GUT?"
};

constexpr I18nText kI18N_blankText = {
	"",
	"",
	"",
	""
};
} // namespace Darkseed

#endif //DARKSEED_LANGTEXT_H
