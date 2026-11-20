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
#include "hollywood/runaway/scene_data.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

// German, French, Spanish and English interface labels, plus localized
// messages for ScummVM saves and settings. All strings are UTF-8.
const char *const kText[RunawayStrings::kTextCount][4] = {
	{"BETRACHTE ", "REGARDER ", "MIRAR ", "LOOK AT "},
	{"NIMM ", "PRENDRE ", "COGER ", "TAKE "},
	{"BENUTZE ", "UTILISER ", "USAR ", "USE "},
	{"VERSTAUE ", "DEPOSER ", "DEJAR ", "PUT AWAY "},
	{" MIT ", " AVEC ", " CON ", " WITH "},
	{"INVENTAR", "INVENTAIRE", "INVENTARIO", "INVENTORY"},
	{"WEITERSPIELEN", "CONTINUER LA PARTIE", "CONTINUAR JUEGO", "RESUME GAME"},
	{"\303\226FFNE ", "OUVRIR ", "ABRIR ", "OPEN "},
	{"SCHLIESSE ", "FERMER ", "CERRAR ", "CLOSE "},
	{"VERSCHIEBE ", "DEPLACER ", "MOVER ", "MOVE "},
	{"SIEH AUS DEM ", "REGARDER A TRAVERS ", "MIRAR A TRAV\303\211S DE LA ", "LOOK THROUGH "},
	{"BENUTZE DAS ", "APPELER AVEC ", "LLAMAR POR ", "USE "},
	{"NIMM ETWAS AUS DEM ", "PRENDRE QUELQUE CHOSE DANS ", "COGER ALGO DE LA ", "TAKE FROM "},
	{"ZIEHE DEN STECKER DER K\303\234HLTRUHE HERAUS", "DEBRANCHER CONGELATEUR", "DESENCHUFAR CONGELADOR", "UNPLUG COOLER"},
	{"HEMD", "CHEMISE", "CAMISA", "SHIRT"},
	{"PUTZLAPPEN", "PEAU DE CHAMOIS HUMIDE", "GAMUZA HUMEDECIDA", "WET CHAMOIS"},
	{"SCHMUTZLAPPEN", "PEAU DE CHAMOIS SALE", "GAMUZA MUGRIENTA", "FILTHY CHAMOIS"},
	{"OPTIONEN", "OPTIONS", "OPCIONES", "OPTIONS"},
	{"LADEN", "CHARGER", "CARGAR", "LOAD"},
	{"SICHERN", "SAUVEGARDER", "SALVAR", "SAVE"},
	{"L\303\226SCHEN", "EFFACER", "BORRAR", "DELETE"},
	{"WEITERSPIELEN", "JOUER", "JUGAR", "PLAY"},
	{"VERLASSEN", "QUITTER", "SALIR", "QUIT"},
	{"MUSIK", "Musique", "M\303\272sica", "Music"},
	{"ANTI-ALIASING", "Antialiasing des sprites", "Antidentado Sprite", "Antialiasing"},
	{"SCROLLING", "Mode de d\303\251filement", "M\303\251todo Scroll", "Scrolling"},
	{"MAUSZEIGERKLICKEN", "Bruitage des ic\303\264nes", "Sonido puntero rat\303\263n", "Pointer sound"},
	{"UNTERTITEL", "Sous-titres", "Subt\303\255tulos", "Subtitles"},
	{"EIN", "OUI", "ON", "ON"},
	{"AUS", "NON", "OFF", "OFF"},
	{"SCHNELL", "RAPIDE", "R\303\201PIDO", "FAST"},
	{"LANGSAM", "FLUIDE", "LENTO", "SLOW"},
	{"LAUTST\303\204RKE", "Volume", "Volumen", "Volume"},
	{"HELLIGKEIT", "Luminosit\303\251", "Brillo", "Brightness"},
	{"MITWIRKENDE", "G\303\251n\303\251rique", "Cr\303\251ditos", "Credits"},
	{"EINSTELLUNGEN SICHERN", "Sauvegarder la configuration", "Salvar Configuraci\303\263n", "Save settings"},
	{"VORGABEEINSTELLUNGEN", "Configuration par d\303\251faut", "Configuraci\303\263n Inicial", "Default settings"},
	{"SIND SIE SICHER?", "CONFIRMER ?", "\302\277EST\303\201S SEGURO?", "ARE YOU SURE?"},
	{"JA", "OUI", "S\303\215", "YES"},
	{"NEIN", "NON", "NO", "NO"},
	{"V", "Q", "L", "Q"},
	{"STEIGE DURCH DIE FALLT\303\234R HINAB", "SORTIR PAR LA TRAPPE", "SALIR POR LA TRAMPILLA", "LEAVE THROUGH THE HATCH"},
	{"NAME DES SPIELSTANDES EINGEBEN:", "NOM DE LA SAUVEGARDE :", "INTRODUCE EL NOMBRE DE LA PARTIDA:", "ENTER A NAME FOR THIS SAVE:"},
	{"KEINE SPIELST\303\204NDE VORHANDEN", "AUCUNE SAUVEGARDE", "NO HAY PARTIDAS SALVADAS", "NO SAVED GAMES"},
	{"NEUER SPIELSTAND", "NOUVELLE SAUVEGARDE", "NUEVA PARTIDA SALVADA", "NEW SAVE"},
	{"SPIELSTAND GESICHERT", "PARTIE SAUVEGARDEE", "PARTIDA SALVADA", "GAME SAVED"},
	{"AKTUELLE EINSTELLUNGEN GESICHERT", "CONFIGURATION SAUVEGARDEE", "CONFIGURACI\303\223N SALVADA", "SETTINGS SAVED"},
	{"VORGEGEBENE EINSTELLUNGEN WIEDERHERGESTELLT", "CONFIGURATION PAR DEFAUT RESTAUREE", "CONFIGURACI\303\223N INICIAL RESTAURADA", "DEFAULT SETTINGS RESTORED"},
	{"SPIELSTAND KONNTE NICHT GESICHERT WERDEN", "IMPOSSIBLE DE SAUVEGARDER LA PARTIE", "NO SE HA PODIDO SALVAR LA PARTIDA", "COULD NOT SAVE THE GAME"},
	{"SPIELSTAND KONNTE NICHT GELADEN WERDEN", "IMPOSSIBLE DE CHARGER LA PARTIE", "NO SE HA PODIDO CARGAR LA PARTIDA", "COULD NOT LOAD THE GAME"},
	{"SPIELSTAND KONNTE NICHT GEL\303\226SCHT WERDEN", "IMPOSSIBLE D'EFFACER LA SAUVEGARDE", "NO SE HA PODIDO BORRAR LA PARTIDA", "COULD NOT DELETE THE SAVE"},
	{"KEIN FREIER SPEICHERPLATZ", "AUCUN EMPLACEMENT LIBRE", "NO QUEDAN ESPACIOS LIBRES", "NO FREE SAVE SLOTS"},
	{"H", "E", "B", "D"},
	{"MITWIRKENDE KONNTEN NICHT GELADEN WERDEN", "IMPOSSIBLE DE CHARGER LE GENERIQUE", "NO SE HAN PODIDO CARGAR LOS CR\303\211DITOS", "COULD NOT LOAD THE CREDITS"},
	{"RUNAWAY VON ANFANG AN SPIELEN", "JOUER A RUNAWAY DEPUIS LE DEBUT", "JUGAR A RUNAWAY DESDE EL PRINCIPIO", "PLAY RUNAWAY FROM THE VERY BEGINNING"},
	{"KAPITEL %d SPIELEN", "JOUER AU CHAPITRE %d", "JUGAR AL CAP\303\215TULO %d", "PLAY CHAPTER %d"},
	{nullptr, nullptr, "ROTULADOR", "Dry marker pen"},
	{nullptr, nullptr, "JERINGUILLA CON ALCOHOL", "Syringe with alcohol"},
	{nullptr, nullptr, "FICHA M\303\211DICA FALSA", "Fake medical chart"},
	{nullptr, nullptr, "CABEZA CON PELUCA", "Head with wig"},
	{nullptr, nullptr, "VASO CON AGUA", "Glass with water"},
	{nullptr, nullptr, "CAJA DE JERINGUILLAS", "Box of syringes"},
	{nullptr, nullptr, "FICHA M\303\211DICA", "Medical chart"},
	{nullptr, nullptr, "FICHA FALSA", "Fake medical chart"},
	{nullptr, nullptr, nullptr, "Condom"},
	{nullptr, nullptr, "PILA CONGELADA", "Frozen battery"},
	{nullptr, nullptr, "DESENCASTRADOR MAYA", "Mayan prying tool"},
	{nullptr, nullptr, "CUENCO CON GRANOS MOLIDOS", "Bowl with ground coffee"},
	{nullptr, nullptr, "CUENCO", "Empty bowl"},
	{nullptr, nullptr, "CAZO CON PILA", "Ladle with battery"},
	{nullptr, nullptr, "LIBROS DE ANTROPOLOG\303\215A", "Anthropology books"},
	{nullptr, nullptr, "DOCUMENTOS", "Documents"},
	{nullptr, nullptr, "L\303\201SER \"MANIPULADO\"", "Altered laser"},
	{nullptr, nullptr, "MALET\303\215N DE BARNICES", "Briefcase full of varnish"},
	{nullptr, nullptr, "WILLY", "Willy"},
	{nullptr, nullptr, "CUENCOS Y JARRAS MAYAS", "Mayan bowls and jars"},
	{nullptr, nullptr, "CUBO", "Bucket"},
	{nullptr, nullptr, "TETERA", "Teapot"},
	{nullptr, nullptr, "TAZ\303\223N", "Bowl"},
	{nullptr, nullptr, "C\303\201MARA T\303\211RMICA", "Thermal chamber"},
	{nullptr, nullptr, "LLAVE", "Key"},
	{nullptr, nullptr, "BOMBONA DE NITR\303\223GENO", "Liquid nitrogen tank"},
	{nullptr, nullptr, "MICROSCOPIO ELECTR\303\223NICO", "Electron microscope"},
	{nullptr, nullptr, "BALA IMPROVISADA", "Makeshift bullet"},
	{nullptr, nullptr, "MONT\303\223N DE BALAS IMPROVISADAS", "Pile of makeshift bullets"}
};

// Hospital names from the Spanish DVD executable at 0x5c2424..0x5c2598.
const char *const kHospitalItemNames[][4] = {
	{"", "", "", ""},
	{nullptr, nullptr, "CARTERA", "Wallet"},
	{nullptr, nullptr, "LLAVES DEL COCHE", "Car keys"},
	{nullptr, nullptr, "RESGUARDO", "Order form"},
	{nullptr, nullptr, "TROZO DE \303\201MBAR", "Piece of amber"},
	{nullptr, nullptr, "BOLSA DE GINA", "Gina's bag"},
	{nullptr, nullptr, "FICHA M\303\211DICA DE GINA", "Gina's medical chart"},
	{nullptr, nullptr, "S\303\201BANA", "Sheet"},
	{nullptr, nullptr, "TABLETA DE PASTILLAS", "Package of pills"},
	{nullptr, nullptr, "CAJA DE CERILLAS", "Matchbox"},
	{nullptr, nullptr, "PELUCA", "Wig"},
	{nullptr, nullptr, "ROTULADOR", "Marker pen"},
	{nullptr, nullptr, "BOTE DE ALCOHOL", "Bottle of alcohol"},
	{nullptr, nullptr, "VASO", "Glass"},
	{nullptr, nullptr, "ALMOHADONES", "Pillows"},
	{nullptr, nullptr, "JERINGUILLA", "Syringe"},
	{nullptr, nullptr, "FICHA M\303\211DICA EN BLANCO", "Blank medical chart"},
	{nullptr, nullptr, "CABEZA DEL MU\303\221ECO", "Head"},
	{nullptr, nullptr, "SPRAY", "Flammable spray can"},
	{nullptr, nullptr, "VADEM\303\211CUM", "Vademecum"}
};

const char *const kMuseumItemNames[][4] = {
	{"", "", "", ""},
	{nullptr, nullptr, "CARTERA", "Wallet"},
	{nullptr, nullptr, "LLAVES DEL COCHE", "Car keys"},
	{nullptr, nullptr, "RESGUARDO", "Receipt"},
	{nullptr, nullptr, "TROZO DE \303\201MBAR", "Piece of amber"},
	{nullptr, nullptr, "GRABADORA", "Tape recorder"},
	{nullptr, nullptr, "PILA", "Battery"},
	{nullptr, nullptr, "CRUCIFIJO", "Crucifix"},
	{nullptr, nullptr, "EXTRA\303\221O OBJETO MAYA", "Strange Mayan object"},
	{nullptr, nullptr, "BARNIZ INCOLORO", "Colorless varnish"},
	{nullptr, nullptr, "POLVOS DE TALCO", "Talcum powder"},
	{nullptr, nullptr, "CARETA MAYA", "Mayan mask"},
	{nullptr, nullptr, "RUB\303\215", "Ruby"},
	{nullptr, nullptr, "TARJETA DE WILLY", "Willy's business card"},
	{nullptr, nullptr, "CUENCO CON GRANOS", "Bowl with coffee beans"},
	{nullptr, nullptr, "VASO DE CAF\303\211", "Cup of coffee"},
	{nullptr, nullptr, "PAR DE LIBROS", "Pair of books"},
	{nullptr, nullptr, "EXTRA\303\221A LLAVE", "Strange key"},
	{nullptr, nullptr, "CAZO", "Ladle"},
	{nullptr, nullptr, "ENVASE DE CAF\303\211", "Package of coffee"}
};

const char *const kCabinItemNames[][4] = {
	{"", "", "", ""},
	{"LEDERLAPPEN", "PEAU DE CHAMOIS", "GAMUZA", "CHAMOIS"},
	{"BRECHSTANGE", "PIED DE BICHE", "PALANCA", "CROWBAR"},
	{"SPR\303\234HREINIGER", "PRODUIT NETTOYANT", "BOTE DE LIMPIEZA", "BOTTLE OF CLEANSER"},
	{"PACKUNG BUTTER", "TABLETTE DE BEURRE", "PAQUETE DE MANTEQUILLA", "TUB OF BUTTER"},
	{"SPRENGAUSR\303\234STUNG", "DYNAMITE ET DETONATEUR", "EQUIPO DE DINAMITERO", "DYNAMITE AND DETONATION EQUIPMENT"},
	{"HELM AUS DEM ZWEITEN WELTKRIEG", "CASQUE DE LA SECONDE GUERRE MONDIALE", "CASCO DE LA SEGUNDA GUERRA MUNDIAL", "WORLD WAR II HELMET"},
	{"SCHRAUBE", "VIS", "TORNILLO", "SCREW"},
	{"GARDEROBENKNAUF", "PORTE-MANTEAUX", "POMO DEL PERCHERO", "COAT RACK PEG"},
	{"TABLETTE", "PILLULE", "PASTILLA", "PILL"},
	{"LIPPENSTIFT", "ROUGE A LEVRES", "PINTALABIOS", "LIPSTICK"},
	{"HAUFEN LIPPENSTIFTE", "STOCK DE ROUGE A LEVRES", "MONT\303\223N DE PINTALABIOS", "PILE OF LIPSTICK"},
	{"SONNEN\303\226L", "HUILE SOLAIRE", "ACEITE BRONCEADOR", "TANNING OIL"},
	{"WALKIE-TALKIE", "TALKIE-WALKIE", "WALKIE TALKIE", "WALKIE-TALKIE"},
	{"MUNITIONSSTREIFEN", "BANDE DE MUNITIONS", "RISTRA DE BALAS", "BULLET BELT"},
	{"BASKETBALL", "BALLON DE BASKET", "BAL\303\223N DE BALONCESTO", "BASKETBALL"},
	{"SATTLERNADEL UND FADEN", "FIL A COUDRE ET AIGUILLE", "AGUJA Y ROLLO DE HILO DE ZAPATERO", "NEEDLE AND THREAD"},
	{"BEHELFS-BOHRER", "TOURNEVIS ARTISANAL", "SACACORCHOS IMPROVISADO", "MAKESHIFT CORKSCREW"},
	{"SACK GESCH\303\204LTER ERDN\303\234SSE", "SAC DE CACAHOUETES ENTIERES", "SAQUITO DE CACAHUETES PELADOS", "PACKAGE OF SHELLED PEANUTS"},
	{"GLAS ERDNUSSBUTTER", "POT DE BEURRE DE CACAHOUETES", "TARRO DE MANTEQUILLA DE CACAHUETE", "JAR OF PEANUT BUTTER"},
	{"SONNENBRILLE", "LUNETTES DE SOLEIL", "GAFAS DE SOL", "SUNGLASSES"},
	{"FUSSRASTE", "REPOSE-PIED DE MOTO", "ESTRIBO DE LA MOTO", "MOTORCYCLE FOOT REST"},
	{"HANDSTAUBSAUGER", "ASPIRATEUR A MAIN", "ASPIRADORA DE MANO", "HANDHELD VACUUM CLEANER"},
	{"PLAN", "PLAN GENIAL", "PLAN", "PLAN"},
	{"BLASEBALG", "SOUFFLET", "FUELLE", "BELLOWS"},
	{"EIMER", "SEAU", "CUBO", "BUCKET"},
	{"SCHL\303\234SSEL", "CLE DU REFRIGERATEUR", "LLAVE", "REFRIGERATOR KEY"}
};

const char *const kShackHotspotNames[][4] = {
	{"", "", "", ""},
	{"T\303\234R", "PORTE", "PUERTA", "DOOR"},
	{"SPALT", "ORIFICE", "RENDIJA", "CRACK"},
	{"FENSTER", "FENETRE", "VENTANA", "WINDOW"},
	{"FENSTER", "PETITE FENETRE", "VENTANA PEQUE\303\221A", "SMALL WINDOW"},
	{"HOLZST\303\234CK", "MORCEAU DE BOIS ROND", "TROZO REDONDO DE MADERA", "ROUND PIECE OF WOOD"},
	{"FALLT\303\234R", "TRAPPE", "TRAMPILLA", "TRAP DOOR"},
	{"STOFFFETZEN", "TISSU DECHIRE", "TELA HECHA JIRONES", "FABRIC TORN TO SHREDS"},
	{"K\303\234HLTRUHE", "CONGELATEUR", "CONGELADOR", "FREEZER"},
	{"FETZEN", "CHIFFON", "TRAPO", "RAG"},
	{"STECKER", "PRISE ELECTRIQUE", "ENCHUFE", "ELECTRIC PLUG"},
	{"ABFLUSS", "TUYAU DE VIDANGE DU CONGELATEUR", "DESAG\303\234E DEL CONGELADOR", "FREEZER DRAIN"},
	{"TELEFON", "TELEPHONE", "TEL\303\211FONO", "TELEPHONE"},
	{"REGAL", "ETAGERES", "ESTANTER\303\215A", "SHELVES"},
	{"BRECHSTANGE", "PIED-DE-BICHE", "PALANCA", "CROWBAR"},
	{"BRILLENRESTE", "LUNETTES CASSEES", "GAFAS ROTAS", "BROKEN GLASSES"},
	{"BLASEBALG", "SOUFFLET", "FUELLE", "BELLOWS"},
	{"DOSE", "BOUTEILLE", "BOTE", "CAN"},
	{"K\303\234HLTRUHENDECKEL", "COUVERCLE DU CONGELATEUR", "TAPA DEL CONGELADOR", "FREEZER LID"},
	{"ABFLUSS", "TUYAU DE VIDANGE DU CONGELATEUR", "DESAG\303\234E DEL CONGELADOR", "FREEZER DRAIN"}
};

// Indexed by the original room's hotspot IDs. Untranslated rooms retain
// the English names in their scene tables.
const char *const kBedroomHotspotNamesES[] = {
	"", "PUERTA DEL BA\303\221O", "GINA", "VASO", "TABLETA DE PASTILLAS", "S\303\201BANA",
	"TAQUILLA", "BOLSA DE GINA", "VENTANA", "FICHA M\303\211DICA DE GINA", "PLACA", "PACIENTE FALSO",
	"ESPITA ANTI-INCENDIOS", "TIMBRE", "TIMBRE", "RADIADOR", "P\303\223STER", "P\303\223STER",
	"PLANO DE SALIDAS DE EMERGENCIA", "CAMA", "", "", "", "PUERTA DEL PASILLO"
};

const char *const kStoreroomHotspotNamesES[] = {
	"", "VENTANA", "RADIADOR", "CAJA", "BOTE DE SPRAY", "LIBRO", "PAPEL QUE ASOMA", "MU\303\221ECO DE ANATOM\303\215A",
	"ARCHIVADOR", "ARCHIVADOR", "FRASCOS", "CAJA", "CAJAS DE MEDICAMENTOS", "SOPORTE PARA SUERO", "BOTELLA DE SUERO", "ALMOHADONES"
};

const char *const kBathroomHotspotNamesES[] = {
	"", "PAPELERA", "GRIFO", "PUERTA", "BOTE", "CARTEL", "TUBER\303\215A", "ALCACHOFA", "ARMARIO", "TAZA DEL WATER"
};

const char *const kMuseumOfficeHotspotNamesES[] = {
	"", "PUERTA", "PAR DE LIBROS", "HENDIDURA", "DINERO", "PAPELES", "APARATO",
	"ESTANTER\303\215A", "CAJA FUERTE SECRETA", "ESTANTER\303\215A", "VENTANA"
};

const char *const kMuseumLaboratoryHotspotNamesES[] = {
	"", "PUERTA", "ESTANTER\303\215A", "SUSAN", "TORNO EL\303\211CTRICO", "MICROSCOPIO",
	"M\303\201QUINA DE AGUA", "PAPELERA", "BOMBONA", "M\303\201QUINA", "L\303\201SER",
	"ESTANTER\303\215A", "MALET\303\215N", "ESQUELETO", "EXTRA\303\221A BROCHA"
};

const char *const kMuseumHallHotspotNamesES[] = {
	"", "ESCALERA", "M\303\201QUINA DE CAF\303\211", "PUERTA DEL DESPACHO DE CLIVE", "EXTINTOR", "PAPELERA",
	"PUERTA DEL LABORATORIO DE RESTAURACI\303\223N", "PUERTA", "PANEL", "ESCALERA", "TEL\303\211FONO",
	"VITRINA", "ESCALERA", "ESCALERA", "EMPLEADO DE LIMPIEZA", "EXPOSICI\303\223N MAYA", "PLACA", "PLACA"
};

const char *const kMuseumExhibitionHotspotNamesES[] = {
	"", "VEST\303\215BULO", "ESTATUA", "ESTATUA", "OBJETOS MAYAS", "CONSTRUCCI\303\223N MAYA",
	"CONSTRUCCI\303\223N MAYA", "OBJETO MAYA", "OBJETO MAYA", "OBJETO MAYA"
};

const char *const kMuseumSecurityHotspotNamesES[] = {
	"", "PUERTA", "BOTIQU\303\215N", "ARMARIO ACRISTALADO", "BOT\303\223N", "CARETA MAYA",
	"ESC\303\201NER", "BOMBONA", "OBJETO DORADO", "APARATO", "CAZO"
};

const char *const kDesertMapHotspotNamesES[] = {
	"", "LUGAR DESCONOCIDO", "CABA\303\221A", "LUGAR DESCONOCIDO", "LUGAR DESCONOCIDO", "LUGAR DESCONOCIDO",
	"AUTOB\303\232S", "POZO DE PETR\303\223LEO", "HANGAR", "CEMENTERIO DE AVIONES", "VAG\303\223N ABANDONADO"
};

const char *const kCabinExteriorHotspotNamesES[] = {
	"", "CAMINO", "", "MECEDORA", "NEUM\303\201TICOS", "PUERTA", "TRAMPILLA", "CUBO"
};

const char *const kAirplaneHotspotNamesES[] = {
	"", "CAMINO", "CASCO", "CINCHA COLGANDO", "AMETRALLADORA", "BALAZO", "PUESTO DEL PILOTO",
	"PUESTO DEL ARTILLERO", "RISTRA DE BALAS"
};

const char *const kBusExteriorHotspotNamesES[] = {
	"", "CAMINO", "LULA", "MARIOLA", "INTERIOR DEL AUTOCAR", "BA\303\232L", "BOTE", "SILLA",
	"RADIO CASSETTE", "GAFAS DE SOL", "ACEITE BRONCEADOR", "SILLA DE CAMPO"
};

const char *const kOilWellHotspotNamesES[] = {
	"", "CAMINO", "MARIOLA", "DETONADOR", "POZO PETROL\303\215FERO", "ARBOL", "ROCA", "CHARCO DE PETR\303\223LEO"
};

const char *const kRailwayYardHotspotNamesES[] = {
	"", "CAMINO", "CAJA DE MADERA", "CASETA", "PUERTA", "EXPLOSIVOS", "DEP\303\223SITO DE AGUA",
	"VAG\303\223N", "INTERIOR DEL VAG\303\223N", "HORMIGUERO", "CARTEL", "ESCALERA", "SERR\303\215N"
};

const char *const kHangarHotspotNamesES[] = {
	"", "CAMINO", "PUERTAS DEL GRANERO", "TARRO", "VENTANA", "MOTO", "CHULO", "RESTOS DE MECEDORA",
	"TROZO DE METAL", "BARRIL", "BARRIL", "HELIC\303\223PTERO", "LATA", "CUBO",
	// These two hotspots originally used complete action captions instead of names.
	"EXTERIOR", "CHULO", "LATA DE CERVEZA", "MANTEQUILLA DE CACAHUETE", "ESTRIBO"
};

const char *const kTrainCarHotspotNamesES[] = {
	"", "EXTERIOR DEL VAG\303\223N", "BARRILES", "CADENA", "CACHIVACHES", "BARRIL", "CUBO",
	"RELOJ", "CUBO", "BARRIL DE P\303\223LVORA"
};

const char *const kBusRearHotspotNamesES[] = {
	"", "PARTE DELANTERA DEL AUTOCAR", "NEVERA", "CARLA", "COPA", "REJILLA", "BOTAS", "TORO",
	"CUADRO", "AGUJA E HILO", "PARTE DELANTERA DEL AUTOCAR"
};

const char *const kBusFrontHotspotNamesES[] = {
	"", "EXTERIOR DEL AUTOCAR", "PARTE TRASERA DEL AUTOCAR", "ESTANTE DERECHO", "EJE", "BOLSILLO",
	"GAFAS DE SOL", "OBJETO BAJO LA CAMA", "ESTANTE CENTRAL", "ESTANTE IZQUIERDO", "PARTE TRASERA DEL AUTOCAR",
	"PUERTA", "MONT\303\223N DE PINTALABIOS", "MANIVELA IMPROVISADA", "MANGO", "COLLARES", "ANILLOS Y PENDIENTES",
	"EXTERIOR DEL AUTOCAR", "ESTANTE DERECHO"
};

const struct {
	uint16 scene;
	const char *const *names[4];
	uint count;
} kHotspotNames[] = {
	{kHospitalBedroom, {nullptr, nullptr, kBedroomHotspotNamesES, nullptr}, ARRAYSIZE(kBedroomHotspotNamesES)},
	{kHospitalStoreroom, {nullptr, nullptr, kStoreroomHotspotNamesES, nullptr}, ARRAYSIZE(kStoreroomHotspotNamesES)},
	{kHospitalBathroom, {nullptr, nullptr, kBathroomHotspotNamesES, nullptr}, ARRAYSIZE(kBathroomHotspotNamesES)},
	{kMuseumOffice, {nullptr, nullptr, kMuseumOfficeHotspotNamesES, nullptr}, ARRAYSIZE(kMuseumOfficeHotspotNamesES)},
	{kMuseumLaboratory, {nullptr, nullptr, kMuseumLaboratoryHotspotNamesES, nullptr}, ARRAYSIZE(kMuseumLaboratoryHotspotNamesES)},
	{kMuseumHall, {nullptr, nullptr, kMuseumHallHotspotNamesES, nullptr}, ARRAYSIZE(kMuseumHallHotspotNamesES)},
	{kMuseumExhibition, {nullptr, nullptr, kMuseumExhibitionHotspotNamesES, nullptr}, ARRAYSIZE(kMuseumExhibitionHotspotNamesES)},
	{kMuseumSecurity, {nullptr, nullptr, kMuseumSecurityHotspotNamesES, nullptr}, ARRAYSIZE(kMuseumSecurityHotspotNamesES)},
	{5010, {nullptr, nullptr, kDesertMapHotspotNamesES, nullptr}, ARRAYSIZE(kDesertMapHotspotNamesES)},
	{5020, {nullptr, nullptr, kCabinExteriorHotspotNamesES, nullptr}, ARRAYSIZE(kCabinExteriorHotspotNamesES)},
	{5050, {nullptr, nullptr, kAirplaneHotspotNamesES, nullptr}, ARRAYSIZE(kAirplaneHotspotNamesES)},
	{5060, {nullptr, nullptr, kBusExteriorHotspotNamesES, nullptr}, ARRAYSIZE(kBusExteriorHotspotNamesES)},
	{5070, {nullptr, nullptr, kOilWellHotspotNamesES, nullptr}, ARRAYSIZE(kOilWellHotspotNamesES)},
	{5080, {nullptr, nullptr, kRailwayYardHotspotNamesES, nullptr}, ARRAYSIZE(kRailwayYardHotspotNamesES)},
	{5100, {nullptr, nullptr, kHangarHotspotNamesES, nullptr}, ARRAYSIZE(kHangarHotspotNamesES)},
	{5110, {nullptr, nullptr, kTrainCarHotspotNamesES, nullptr}, ARRAYSIZE(kTrainCarHotspotNamesES)},
	{5160, {nullptr, nullptr, kBusRearHotspotNamesES, nullptr}, ARRAYSIZE(kBusRearHotspotNamesES)},
	{5170, {nullptr, nullptr, kBusFrontHotspotNamesES, nullptr}, ARRAYSIZE(kBusFrontHotspotNamesES)}
};

} // End of anonymous namespace

Common::U32String RunawayStrings::localize(const char *const *strings) const {
	return Common::U32String(strings[_languageIndex] ? strings[_languageIndex] : strings[3]);
}

Common::U32String RunawayStrings::text(TextId id) const {
	assert(id < kTextCount);
	return localize(kText[id]);
}

Common::U32String RunawayStrings::itemName(byte chapter, byte item) const {
	if (chapter == 1 && item < ARRAYSIZE(kHospitalItemNames)) {
		return localize(kHospitalItemNames[item]);
	}
	if (chapter == 2 && item < ARRAYSIZE(kMuseumItemNames)) {
		return localize(kMuseumItemNames[item]);
	}
	if (chapter == 3 && item < ARRAYSIZE(kCabinItemNames)) {
		return localize(kCabinItemNames[item]);
	}
	return Common::U32String();
}

Common::U32String RunawayStrings::hotspotName(uint16 scene, byte hotspot, const char *fallback) const {
	for (uint i = 0; i < ARRAYSIZE(kHotspotNames); ++i) {
		if (kHotspotNames[i].scene == scene && hotspot < kHotspotNames[i].count) {
			const char *const *names = kHotspotNames[i].names[_languageIndex];
			if (names) {
				return Common::U32String(names[hotspot]);
			}
			break;
		}
	}
	return Common::U32String(fallback);
}

Common::U32String RunawayStrings::shackHotspotName(byte hotspot) const {
	return Common::U32String(hotspot < ARRAYSIZE(kShackHotspotNames) ? kShackHotspotNames[hotspot][_languageIndex] : "");
}

} // End of namespace Hollywood
