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

#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

namespace Tattoo {

static const char *const fixedTextEN[] = {
	"Money",
	"Money",
	"Card",
	"Card",
	"Tobacco",
	"Tobacco",
	"Timetable",
	"Timetable",
	"Summons",
	"Summons",
	"Foolscap",
	"Foolscap",
	"Damp Paper",
	"Foolscap",
	"Bull's Eye",
	"Bull's Eye Lantern",

	"Open",
	"Look",
	"Talk",
	"Use",
	"Journal",
	"Inventory",
	"Options",
	"Solve",
	"with",
	"No effect...",
	"This person has nothing to say at the moment",
	"Picked up",

	"Page %d",
	"Close Journal",
	"Search Journal",
	"Save Journal",
	"Abort Search",
	"Search Backwards",
	"Search Forwards",
	"Text Not Found !",
	// Darts
	"Holmes",
	"Jock",
	"Bull",
	"Round: %d",
	"Turn Total: %d",
	"Dart # %d",
	"Hit a key",
	"To start",
	"Press a key",
	"GAME OVER!",
	"BUSTED!",
	"%s Wins",
	"Scored %d point", // original: treated 1 point and multiple points the same ("Scored 1 points")
	"Scored %d points",
	"Hit a %d",
	"Hit double %d",
	"Hit triple %d",
	"Hit a bullseye",
	"Hit double bullseye",
	"Hit triple bullseye",

	"Apply",
	"Water",
	"Heat",
	"Load Game",
	"Save Game",
	"Music",
	"Sound Effects",
	"Voices",
	"Text Windows",
	"Transparent Menus",
	"Change Font Style",
	"Off",
	"On",
	"Quit",
	"Are you sure you",
	"wish to Quit ?",
	"Yes",
	"No",
	"Enter Password",
	"Going East", // correct password, was not and should not to be translated
	"Watson's Journal",
	"Journal saved as journal.txt",
	// SH2: People names
	"Sherlock Holmes",
	"Dr. Watson",
	"Mrs. Hudson",
	"Stanley Forbes",
	"Mycroft Holmes",
	"Wiggins",
	"Police Constable Burns",
	"Augustus Trimble",
	"Police Constable Daley",
	"Matron",
	"Sister Grace",
	"Preston McCabe",
	"Bob Colleran",
	"Jonas Rigby",
	"Police Constable Roach",
	"James Dewar",
	"Sergeant Jeremy Duncan",
	"Inspector Gregson",
	"Inspector Lestrade",
	"Jesse Needhem",
	"Arthur Fleming",
	"Mr. Thomas Pratt",
	"Mathilda (Tillie) Mason",
	"Adrian Russell",
	"Eldridge Whitney",
	"Hepplethwaite",
	"Horace Silverbridge",
	"Old Sherman",
	"Maxwell Verner",
	"Millicent Redding",
	"Virgil Silverbridge",
	"George O'Keeffe",
	"Lord Denys Lawton",
	"Jenkins",
	"Jock Mahoney",
	"Bartender",
	"Lady Cordelia Lockridge",
	"Pettigrew",
	"Sir Avery Fanshawe",
	"Hodgkins",
	"Wilbur \"Birdy\" Heywood",
	"Jacob Farthington",
	"Philip Bledsoe",
	"Sidney Fowler",
	"Professor Theodore Totman",
	"Rose Hinchem",
	"Tallboy",
	"Ethlebert \"Stitch\" Rumsey",
	"Charles Freedman",
	"Nigel Hemmings",
	"Fairfax Carter",
	"Wilhelm II",
	"Wachthund",
	"Jonathan Wilson",
	"David Lloyd-Jones",
	"Edward Hargrove",
	"Misteray",
	"The Lascar",
	"Parrot",
	"Vincent Scarrett",
	"Alexandra",
	"Queen Victoria",
	"John Brown",
	"A Patient",
	"A Patient",
	"Patron",
	"Queen Victoria",
	"Patient in white",
	"Lush",
	"Drunk",
	"Prostitute",
	"Mudlark",
	"Grinder",
	"Bouncer",
	"Agnes Ratchet",
	"Aloysius Ratchet",
	"Real Estate Agent",
	"Candy Clerk",
	"Beadle",
	"Prussian",
	"Mrs. Rowbottom",
	"Miss Lloyd-Jones",
	"Tavern patron",
	"User",
	"Toby",
	"Stationer",
	"Law Clerk",
	"Ministry Clerk",
	"Bather",
	"Maid",
	"Lady Fanshawe",
	"Sidney Ratchet",
	"Boy",
	"Patron",
	"Constable Brit",
	"Wagon Driver"
};

// sharp-s       : 0xE1 / octal 341
// small a-umlaut: 0x84 / octal 204
// small o-umlaut: 0x94 / octal 224
// small u-umlaut: 0x81 / octal 201
// large O-umlaut: 0x99 / octal 231
static const char *const fixedTextDE[] = {
	"Geld",
	"Geld",
	"S. Holmes",
	"S. Holmes",
	"Tabak",
	"Tabak",
	"Plan",
	"Plan",
	"Aufforderg.",
	"Aufforderg.",
	"Blatt pap.",
	"Pergament",
	"Dunstig pap",
	"Dunstig pap",
	"Handlampe",
	"Handlampe",

	"\231ffne",
	"Schau",
	"Rede",
	"Benutze",
	"Tagebuch",
	"Tasche",
	"Optionen",
	"L\224sen",
	"mit",
	"Keine Wirkung...",
	"Diese Person wei\341 im Augenblick nichts zu berichten.",
	"Picked up", // <-- ??

	"Seite %d",
	"Schlie\341en",
	"Durchsuchen", // original: "Lessen"
	"In Datei sichern", // original: "Speichern"
	"Suche abbrechen",
	"R\201ckw\204rts suchen ",
	"Vorw\204rts suchen ",
	"Text nicht gefunden",
	// Darts
	"Holmes",
	"Jock",
	"Bull",
	"Runde: %d",
	"Gesamt: %d",
	"Pfeil # %d",
	"Taste dr\201cken",
	"zum Starten",
	"Taste dr\201cken",
	"SPIEL BEENDET!",
	"VERLOREN!",
	"%s gewinnt!", // "Holmes Gewinnt!", "%s Gewinnt!", original: "%s Gewinnt!"
	"Erzielte %d Punkt", // original: treated 1 point and multiple points the same ("Scored 1 points")
	"Erzielte %d Punkte",
	"%d getroffen", // original: "Treffer %s %d"
	"Doppel %d getroffen", // original: see above
	"Dreifach %d getroffen", // original: see above
	"Bullseye getroffen",
	"Doppel Bullseye getroffen",
	"Dreifach Bullseye getroffen",

	"Benutze",
	"Wasser",
	"Erhitze",
	"Spiel laden",
	"Spiel sichern",
	"Musik",
	"Soundeffekte",
	"Voices",
	"Textfenster",
	"Transparente Men\201s",
	"Schriftart andern",
	"Aus",
	"An",
	"Ende",
	"Spiel beenden? ",
	"Sind Sie sicher ?",
	"Ja",
	"Nein",
	"Pa\341wort eingeben",
	"Going East", // correct password, was not and should not to be translated
	"Watsons Tagebuch",
	"Journal gespeichert als journal.txt",
	// SH2: People names
	"Sherlock Holmes", // note: People names were not translated in the German interpreter
	"Dr. Watson",
	"Mrs. Hudson",
	"Stanley Forbes",
	"Mycroft Holmes",
	"Wiggins",
	"Police Constable Burns",
	"Augustus Trimble",
	"Police Constable Daley",
	"Matron",
	"Sister Grace",
	"Preston McCabe",
	"Bob Colleran",
	"Jonas Rigby",
	"Police Constable Roach",
	"James Dewar",
	"Sergeant Jeremy Duncan",
	"Inspector Gregson",
	"Inspector Lestrade",
	"Jesse Needhem",
	"Arthur Fleming",
	"Mr. Thomas Pratt",
	"Mathilda (Tillie) Mason",
	"Adrian Russell",
	"Eldridge Whitney",
	"Hepplethwaite",
	"Horace Silverbridge",
	"Old Sherman",
	"Maxwell Verner",
	"Millicent Redding",
	"Virgil Silverbridge",
	"George O'Keeffe",
	"Lord Denys Lawton",
	"Jenkins",
	"Jock Mahoney",
	"Bartender",
	"Lady Cordelia Lockridge",
	"Pettigrew",
	"Sir Avery Fanshawe",
	"Hodgkins",
	"Wilbur \"Birdy\" Heywood",
	"Jacob Farthington",
	"Philip Bledsoe",
	"Sidney Fowler",
	"Professor Theodore Totman",
	"Rose Hinchem",
	"Tallboy",
	"Ethlebert \"Stitch\" Rumsey",
	"Charles Freedman",
	"Nigel Hemmings",
	"Fairfax Carter",
	"Wilhelm II",
	"Wachthund",
	"Jonathan Wilson",
	"David Lloyd-Jones",
	"Edward Hargrove",
	"Misteray",
	"The Lascar",
	"Parrot",
	"Vincent Scarrett",
	"Alexandra",
	"Queen Victoria",
	"John Brown",
	"A Patient",
	"A Patient",
	"Patron",
	"Queen Victoria",
	"Patient in white",
	"Lush",
	"Drunk",
	"Prostitute",
	"Mudlark",
	"Grinder",
	"Bouncer",
	"Agnes Ratchet",
	"Aloysius Ratchet",
	"Real Estate Agent",
	"Candy Clerk",
	"Beadle",
	"Prussian",
	"Mrs. Rowbottom",
	"Miss Lloyd-Jones",
	"Tavern patron",
	"User",
	"Toby",
	"Stationer",
	"Law Clerk",
	"Ministry Clerk",
	"Bather",
	"Maid",
	"Lady Fanshawe",
	"Sidney Ratchet",
	"Boy",
	"Patron",
	"Constable Brit",
	"Wagon Driver"
};

// small a w/ accent grave: 0x85 / octal 205
// small e w/ accent acute: 0x82 / octal 202
// small e w/ accent grave: 0x8A / octal 212
// small e w/ circonflexe:  0x88 / octal 210
// small cedilla:           0x87 / octal 207
static const char *const fixedTextFR[] = {
	"Argent",
	"Argent",
	"S. Holmes",
	"S. Holmes",
	"Tabac",
	"Tabac",
	"Horaire",
	"Horaire",
	"Convocation",
	"Convocation",
	"Feuille",
	"Feuille",
	"F. humide",
	"Feuille",
	"Lanterne",
	"Lanterne",

	"Ouvrir",
	"Regarder",
	"Parler",
	"Utiliser",
	"Journal", // <--
	"Inventaire",
	"Options",
	"R\202soudre",
	"avec",
	"Sans effet...",
	"Cette personne n'a rien \205 ajouter pour le moment.",
	"Picked up", // <-- ??

	"Page %d",
	"Fermer",
	"Rechercher",
	"Sauvegarder",
	"Annuler ",
	"Chercher avant",
	"Chercher apr\212s",
	"Texte introuvable !",
	// Darts
	"Holmes",
	"Jock",
	"Bull",
	"Tour: %d",
	"Total: %d",
	"Fl\202chette # %d",
	"Appuyez sur C",
	"pour commencer",
	"Appuyez sur C",
	"FIN DE LA PARTIE!", // original: "Fin de la partie!"
	"FIASCO!",
	"%s a gagn\202!", // "Holmes Gagnant!", "%s Gagnant!"
	"Rapporte %d point", // original: treated 1 point and multiple points the same ("Scored 1 points")
	"Rapporte %d points", // original: Total des points: %d",
	"Touche un %d", // original: ???
	"Touche double %d",
	"Touche triple %d",
	"Touche le Bullseye",
	"Touche double Bullseye",
	"Touche triple Bullseye",

	"Mouillez",
	"Puis",
	"Chauffez",
	"Charger",
	"Sauvegarder",
	"Musique",
	"Sons",
	"Voix",
	"Fen\210tres de texte", // 0x88
	"Menu Transparent",
	"Changer la fonte",
	"Aus", // ???
	"An", // ???
	"Quitter",
	"Voulez-vous quitter?",
	"Sind Sie sicher ?", // ???
	"Oui",
	"Non",
	"Entrez le mot de passe",
	"Going East", // correct password, was not and should not to be translated
	"Journal de Watson",
	"Journal enregistree comme journal.txt",
	// SH2: People names
	"Sherlock Holmes",
	"Dr. Watson",
	"Mme. Hudson",
	"Stanley Forbes",
	"Mycroft Holmes",
	"Wiggins",
	"Sergent Burns",
	"Augustus Trimble",
	"Sergent Daley",
	"Infirmi\212re chef",
	"Mme. Grace",
	"Preston McCabe",
	"Bob Colleran",
	"Jonas Rigby",
	"Sergent Roach",
	"James Dewar",
	"Sergent Jeremy Duncan",
	"Inspecteur Gregson",
	"Inspecteur Lestrade",
	"Jesse Needhem",
	"Arthur Fleming",
	"M. Thomas Pratt",
	"Mathilda (Tillie) Mason",
	"Adrian Russell",
	"Eldridge Whitney",
	"Hepplethwaite",
	"Horace Silverbridge",
	"Sherman",
	"Maxwell Verner",
	"Millicent Redding",
	"Virgil Silverbridge",
	"George O'Keeffe",
	"Lord Denys Lawton",
	"Jenkins",
	"Jock Mahoney",
	"Serveur",
	"Lady Cordelia Lockridge",
	"Pettigrew",
	"Sir Avery Fanshawe",
	"Hodgkins",
	"Wilbur \"Birdy\" Heywood",
	"Jacob Farthington",
	"Philip Bledsoe",
	"Sidney Fowler",
	"Professeur Theodore Totman",
	"Rose Hinchem",
	"Tallboy",
	"Ethlebert \"Stitch\" Rumsey",
	"Charles Freedman",
	"Nigel Hemmings",
	"Fairfax Carter",
	"Wilhelm II",
	"Wachthund",
	"Jonathan Wilson",
	"David Lloyd-Jones",
	"Edward Hargrove",
	"Misteray",
	"Le Lascar",
	"Oiseau",
	"Vincent Scarrett",
	"Alexandra",
	"Queen Victoria",
	"John Brown",
	"Patient",
	"Patient",
	"Client",
	"Queen Victoria",
	"Patient en blanc",
	"Ivrogne",
	"Ivrogne",
	"Belle femme",
	"Mudlark",
	"Broyeur",
	"Videur",
	"Agnes Ratchet",
	"Aloysius Ratchet",
	"Immobilier",
	"Gar\207on",
	"Beadle",
	"Prussian",
	"Mme. Rowbottom",
	"Mme Lloyd-Jones",
	"Tavern Client",
	"User",
	"Toby",
	"Papeterie",
	"Law Clerc",
	"Ministry Employ\202",
	"Clint du thermes",
	"Bonne",
	"Lady Fanshawe",
	"Sidney Ratchet",
	"Gar\207on",
	"Client",
	"Sergent Brit",
	"Wagon Driver"
};

// small a w/ accent bottom to top : 0xA0 / octal 240
// small i w/ accent bottom to top : 0xA1 / octal 241
// small o w/ accent bottom to top : 0xA2 / octal 242
// small u w/ accent bottom to top : 0xA3 / octal 243
// small n w/ wavy line            : 0xA4 / octal 244
// inverted question mark          : 0xA8 / octal 250
static const char *const fixedTextES[] = {
	"Dinero",
	"Dinero",
	"Tarjeta",
	"Tarjeta",
	"Tabaco",
	"Tabaco",
	"Horarios",
	"Horarios",
	"Mensaje",
	"Mensaje",
	"Papel",
	"Papel",
	"Papel",
	"Papel",
	"Linterna",
	"Linterna",

	"Abrir",
	"Mirar",
	"Hablar",
	"Usar",
	"Diario",
	"Inventario",
	"Opciones",
	"Resolver",
	"con",
	"Sin Efecto...",
	"Esta persona no tiene nada que decir en este momento",
	"Picked up", // <-- ??

	"P\240gina %d",
	"Cerrar Diario",
	"Buscar en Diario",
	"Salvar en Archivo",
	"Detener B\243squeda",
	"Buscar Hacia Atr\240s",
	"Buscar Hacia Delante",
	"Texto No Encontrado !",

	"Holmes", //
	"Jock",
	"Bull",
	"Vuelta: %d",
	"Total del Turno: %d",
	"Dardo # %d",
	"Pulsa una tecla",
	"para empezar",
	"Pulsa una tecla",
	"FIN DE LA PARTIDA!",
	"ROTO!",
	"%s gana!", // "Holmes Gana!", "%s Gana!", original: "%s Gana!"
	"Puntuado %d punto", // original: treated 1 point and multiple points the same ("Scored 1 points")
	"Puntuado %d puntos",
	"Golpe un %d",
	"Gople doble %d",
	"Gople triple %d",
	"Golpe un ojo de buey",
	"Gople doble ojo de buey",
	"Gople triple ojo de buey",

	"aplicar",
	"Agua",
	"Calentar",
	"Cargar Partida",
	"Salvar Partida",
	"M\243sica",
	"Efectos de Sonido",
	"Voces",
	"Ventanas de Texto",
	"Men\243s Transparentes",
	"Cambiar Tipo de Letra",
	"Off", // ???
	"On", // ???
	"Salir",
	"quieres salir?",
	"\250Est\240s seguro de que",
	"Si",
	"Non",
	"Introducir Palabra Clave",
	"Vas al Este", // correct password, was translated in Spanish version (???)
	"Diario de Watson",
	"Diario guarda como journal.txt",
	// SH2: People names
	"Sherlock Holmes",
	"Dr. Watson",
	"Sta. Hudson",
	"Stanley Forbes",
	"Mycroft Holmes",
	"Wiggins",
	"Agente de Polic\241a Burns",
	"Augustus Trimble",
	"Agente de Polic\241a Daley",
	"Enfermera",
	"Hermana Grace",
	"Preston McCabe",
	"Bob Colleran",
	"Jonas Rigby",
	"Agente de Polic\241a Roach",
	"James Dewar",
	"Sargento Jeremy Duncan",
	"Inspector Gregson",
	"Inspector Lestrade",
	"Jesse Needhem",
	"Arthur Fleming",
	"Sr. Thomas Pratt",
	"Mathilda (Tillie) Mason",
	"Adrian Russell",
	"Eldridge Whitney",
	"Hepplethwaite",
	"Horace Silverbridge",
	"Old Sherman",
	"Maxwell Verner",
	"Millicent Redding",
	"Virgil Silverbridge",
	"George O'Keeffe",
	"Lord Denys Lawton",
	"Jenkins",
	"Jock Mahoney",
	"Camarero",
	"Lady Cordelia Lockridge",
	"Pettigrew",
	"Sir Avery Fanshawe",
	"Hodgkins",
	"Wilbur \"Birdy\" Heywood",
	"Jacob Farthington",
	"Philip Bledsoe",
	"Sidney Fowler",
	"Profesor Theodore Totman",
	"Rose Hinchem",
	"Tallboy",
	"Ethlebert \"Stitch\" Rumsey",
	"Charles Freedman",
	"Nigel Hemmings",
	"Fairfax Carter",
	"Guillermo II",
	"Wachthund",
	"Jonathan Wilson",
	"David Lloyd-Jones",
	"Edward Hargrove",
	"Misteray",
	"The Lascar",
	"Loro",
	"Vincent Scarrett",
	"Alexandra",
	"Reina Victoria",
	"John Brown",
	"Un Paciente",
	"Un Paciente",
	"Cliente",
	"Reina Victoria",
	"Paciente de blanco",
	"Exuberante",
	"Borracho",
	"Prostituta",
	"Mudlark",
	"Grinder",
	"Mat\242n", // sic? means block in Spanish and not bouncer?!
	"Agnes Ratchet",
	"Aloysius Ratchet",
	"Agente de Bienes Ra\241ces",
	"Candy Clerk",
	"Beadle",
	"Prusiano",
	"Sta. Rowbottom",
	"Sra. Lloyd-Jones",
	"Parroquiano",
	"Usuario",
	"Toby",
	"Papelero",
	"Secretario de Justicia",
	"Secreatrio del Ministerio",
	"Ba\244ista",
	"Criada",
	"Lady Fanshawe",
	"Sidney Ratchet",
	"Chico",
	"Patron", // ??? not found??
	"Agente Brit",
	"Cochero"
};

static const char *const fixedTextZH[] = {
	"Money", /* Used as id, not shown, not translated.  */
	"\xc7\xae", /* 钱; Money */
	"Card", /* Used as id, not shown, not translated.  */
	"\xc3\xfb\xc6\xac", /* 名片; Card */
	"Tobacco", /* Used as id, not shown, not translated.  */
	"\xd1\xcc\xb6\xb7", /* 烟斗; Tobacco */
	"Timetable", /* Used as id, not shown, not translated.  */
	"\xca\xb1\xbf\xcc\xb1\xed", /* 时刻表; Timetable */
	"Summons", /* Used as id, not shown, not translated.  */
	"\xb1\xe3\xcc\xf5", /* 便条; Summons */
	"Foolscap", /* Used as id, not shown, not translated.  */
	"\xbb\xee\xd2\xb3\xd6\xbd", /* 活页纸; Foolscap */
	"Damp Paper", /* Used as id, not shown, not translated.  */
	"\xca\xaa\xbb\xee\xd2\xb3\xd6\xbd", /* 湿活页纸; Foolscap */
	"Bull's Eye", /* Used as id, not shown, not translated.  */
	"\xc5\xa3\xd1\xdb\xbf\xf3\xb5\xc6", /* 牛眼矿灯; Bull's Eye Lantern */

	"\xb4\xf2\xbf\xaa", /* 打开; Open */
	"\xb9\xdb\xb2\xec", /* 观察; Look */
	"\xbd\xbb\xcc\xb8", /* 交谈; Talk */
	"\xca\xb9\xd3\xc3", /* 使用; Use */
	"\xca\xd6\xbc\xc7", /* 手记; Journal */
	"\xce\xef\xc6\xb7", /* 物品; Inventory */
	"\xd1\xa1\xcf\xee", /* 选项; Options */
	"\xbd\xe2\xbe\xf6", /* 解决; Solve */
	"\xd3\xda", /* 于; with */
	"\xc3\xbb\xd3\xd0\xd0\xa7\xb9\xfb", /* 没有效果; No effect... */
	"\xd5\xe2\xb8\xf6\xc8\xcb\xcf\xd6\xd4\xda\xc3\xbb\xd3\xd0\xbb\xb0\xcb\xb5\xa1\xa3", /* 这个人现在没有话说。; This person has nothing to say at the moment */
	"\xbc\xf1\xc6\xf0", /* 捡起; Picked up*/

	"\xd2\xb3 %d", /* 页 %d; Page %d */
	"\xba\xcf\xc9\xcf\xca\xd6\xbc\xc7", /* 合上手记; Close Journal */
	"\xb2\xe9\xbf\xb4\xca\xd6\xbc\xc7", /* 查看手记; Search Journal */
	"\xb4\xe6\xb4\xa2\xca\xd6\xbc\xc7", /* 存储手记; Save Journal */
	"\xb7\xc5\xc6\xfa\xcb\xd1\xcb\xf7", /* 放弃搜索; Abort search */
	"\xcf\xf2\xba\xf3\xcb\xd1\xcb\xf7", /* 向后搜索; Search Backwards  */
	"\xcf\xf2\xc7\xb0\xcb\xd1\xcb\xf7", /* 向前搜索; Search Forwards  */
	"\xd3\xd0\xb9\xd8\xce\xc4\xd7\xd6\xce\xb4\xb1\xbb\xb7\xa2\xcf\xd6 !", /* 有关文字未被发现 !; Text not found */
	// Darts
	"\xb8\xa3\xb6\xfb\xc4\xa6\xcb\xb9", /* 福尔摩斯; Holmes */
	"\xd5\xd1\xbf\xcb", /* 昭克; Jock */
	"Bull", // Untranslated in the original
	"\xc2\xd6\xca\xfd\x3a %d", /* 轮数: %d; Round: %d  */
	"\xb1\xbe\xc2\xd6\xd7\xdc\xb5\xc3\xb7\xd6\x3a %d", /* 本轮总得分: %d; Turn Total: %d */
	"\xb7\xc9\xef\xda \x23 %d", /* 飞镖 # %d; Dart # %d */
	// Following three looks like they are reordered to always give 按任一键开始 (Press any key to start)
	"\xb0\xb4\xc8\xce\xd2\xbb\xbc\xfc", /* 按任一键; Hit a key */
	"\xb0\xb4\xc8\xce\xd2\xbb\xbc\xfc", /* 按任一键; To start */
	"\xbf\xaa\xca\xbc", /* 开始; Press a key */
	"\xd3\xce\xcf\xb7\xbd\xe1\xca\xf8!", /* 游戏结束!; GAME OVER! */
	"\xca\xa7\xb0\xdc!", /* 失败!; BUSTED! */
	"%s \xca\xa4\xc0\xfb\x21", /* %s 胜利!; %s Wins */
	"\xb5\xc3 %d \xb5\xe3", /* 得 %d 点; Scored %d point.  */
	"\xb5\xc3 %d \xb5\xe3", /* 得 %d 点; Scored %d point.  */
	"\xb4\xf2\xd6\xd0 %d", /* 打中 %d; Hit a %d */
	"\xb4\xf2\xd6\xd0 \xcb\xab\xb1\xb6 %d", /* 打中 双倍 %d; Hit double %d */
	"\xb4\xf2\xd6\xd0 \xc8\xfd\xb1\xb6 %d", /* 打中 三倍 %d; Hit triple %d */
	"\xb4\xf2\xd6\xd0 \xc5\xa3\xd1\xdb", /* 打中 牛眼; Hit a bullseye */
	"\xb4\xf2\xd6\xd0 \xcb\xab\xb1\xb6 \xc5\xa3\xd1\xdb", /* 打中 双倍 牛眼; Hit double bullseye */
	"\xb4\xf2\xd6\xd0 \xc8\xfd\xb1\xb6 \xc5\xa3\xd1\xdb", /* 打中 三倍 牛眼; Hit triple bullseye */

	"Apply", // Was not translated in the original
	"Water", // Was not translated in the original
	"Heat", // Was not translated in the original
	"@$txh\x8b]\x98Vh$@", /* 载入进度; Load Game. */
	"@$TBT\x86]\x98Vh$@", /* 储存进度; Save game. */
	"\xd2\xf4\xc0\xd6", /* 音乐; Music */
	"\xd2\xf4\xd0\xa7", /* 音效; Sound Effects */
	"\xd3\xef\xd2\xf4", /* 语音; Voices */
	"\xb6\xd4\xbb\xb0\xb4\xb0\xbf\xda", /* 对话窗口; Text Windows */
	"\xcd\xb8\xc3\xf7\xb2\xcb\xb5\xa5", /* 透明菜单; Transparent menus  */
	"\xb8\xc4\xb1\xe4\xd7\xd6\xcc\xe5\xb7\xe7\xb8\xf1", /* 改变字体风格; Change Font Style  */
	"\xb9\xd8", /* 关 ; Off  */
	"\xbf\xaa", /* 开 ; On */
	"\xcd\xcb\xb3\xf6", /* 退出; Quit  */
	"\xc4\xe3\xc8\xb7\xb6\xa8\xd2\xaa", /* 你确定要; Are you sure you */
	"\xcd\xcb\xb3\xf6\xc2\xf0\x3f", /* 退出吗?; wish to Quit ? */
	"\xc8\xb7\xb6\xa8", /* 确定; Yes */
	"\xb7\xc5\xc6\xfa", /* 放弃; No */
	"\xca\xe4\xc8\xeb\xc3\xdc\xc2\xeb", /* 输入密码; Enter password */
	"Going East", // correct password, was not and should not to be translated
	"\xbb\xaa\xc9\xfa\xb5\xc4\xca\xd6\xbc\xc7", /* 华生的手记; Watson's Journal */
	"\xca\xd6\xbc\xc7\xd2\xd1\xb1\xbb\xb3\xc9\xb9\xa6\xb5\xc4\xb4\xa2\xb4\xe6\xd6\xc1 journal.txt", // 手记已被成功的储存至 journal.txt; Journal saved as journal.txt. */
	// SH2: People names
	"\xd0\xaa\xc2\xe5\xbf\xcb\xa1\xa4\xb8\xa3\xb6\xfb\xc4\xa6\xcb\xb9", /* 歇洛克・福尔摩斯; Sherlock Holmes */
	"\xbb\xaa\xc9\xfa\xd2\xbd\xc9\xfa", /* 华生医生; Dr. Watson */
	"\xb9\xfe\xb5\xc2\xd1\xb7\xcc\xab\xcc\xab", /* 哈德逊太太; Mrs. Hudson */
	"\xcb\xb9\xcc\xb9\xc0\xfb\xa1\xa4\xb8\xa3\xb2\xbc\xcb\xb9", /* 斯坦利・福布斯; Stanley Forbes */
	"\xc2\xf5\xbf\xcb\xc2\xde\xb7\xf2\xcc\xd8\xa1\xa4\xb8\xa3\xb6\xfb\xc4\xa6\xcb\xb9", /* 迈克罗夫特・福尔摩斯; Mycroft Holmes */
	"\xce\xac\xbd\xf0\xcb\xb9", /* 维金斯; Wiggins */
	"\xb2\xae\xb6\xf7\xbe\xaf\xb9\xd9", /* 伯恩警官; Police Constable Burns */
	"\xb0\xc2\xbc\xaa\xa1\xa4\xb4\xba\xb2\xae", /* 奥吉・春伯; Augustus Trimble */
	"\xb5\xc2\xc0\xfb\xbe\xaf\xb9\xd9", /* 德利警官; Police Constable Daley */
	"\xbb\xa4\xca\xbf\xb3\xa4", /* 护士长; Matron (lit. HEad nurse) */
	"\xb8\xf1\xc0\xd7\xcb\xb9\xd0\xde\xc5\xae", /* 格雷斯修女; Sister Grace */
	"\xc6\xd5\xc0\xd7\xcb\xb9\xb6\xd9\xa1\xa4\xc2\xf3\xbf\xcb\xb1\xc8", /* 普雷斯顿・麦克比; Preston McCabe */
	"\xb1\xab\xb2\xaa\xa1\xa4\xbf\xc2\xc0\xd5\xc8\xf0", /* 鲍勃・柯勒瑞; Bob Colleran */
	"\xc7\xed\xc4\xc7\xcb\xb9\xa1\xa4\xc8\xf0\xb8\xf1\xb1\xc8", /* 琼那斯・瑞格比; Jonas Rigby */
	"\xc2\xde\xb3\xb9\xbe\xaf\xb9\xd9", /* 罗彻警官; Police Constable Roach */
	"\xd5\xb2\xc4\xb7\xcb\xb9\xa1\xa4\xb5\xcf\xce\xd6", /* 詹姆斯・迪沃; James Dewar */
	"\xd5\xab\xc3\xdc\xa1\xa4\xb5\xcb\xbf\xcf\xbe\xaf\xb9\xd9", /* 斋密・邓肯警官; Sergeant Jeremy Duncan */
	"\xb8\xf1\xc0\xd7\xc9\xad\xbe\xaf\xb3\xa4", /* 格雷森警长; Inspector Gregson */
	"\xc0\xd5\xb5\xc2\xbe\xaf\xb3\xa4", /* 勒德警长; Inspector Lestrade */
	"\xbd\xdc\xce\xf7\xa1\xa4\xc4\xe1\xba\xd5", /* 杰西・尼赫; Jesse Needhem */
	"\xd1\xc7\xc9\xaa\xa1\xa4\xb8\xa5\xc0\xb3\xc3\xf7", /* 亚瑟・弗莱明; Arthur Fleming */
	"\xcd\xd0\xc2\xed\xcb\xb9\xa1\xa4\xc6\xd5\xc0\xb3\xcc\xd8\xcf\xc8\xc9\xfa", /* 托马斯・普莱特先生; Mr. Thomas Pratt */
	"\xc2\xea\xc9\xaa\xb4\xef\xa1\xa4\xc3\xb7\xc9\xad", /* 玛瑟达・梅森; Mathilda (Tillie) Mason */
	"\xb0\xac\xb5\xc7\xa1\xa4\xc2\xde\xc8\xfb\xb6\xfb", /* 艾登・罗塞尔; Adrian Russell */
	"\xb0\xa3\xc6\xe6\xa1\xa4\xbb\xdd\xcc\xd8\xc4\xe1", /* 埃奇・惠特尼; Eldridge Whitney */
	"\xba\xa3\xc6\xd5\xce\xac", /* 海普维; Hepplethwaite */
	"\xba\xc9\xc0\xb3\xca\xbf\xa1\xa4\xcb\xb9\xce\xac\xb2\xbc\xc8\xf0\xc6\xe6", /* 荷莱士・斯维布瑞奇; Horace Silverbridge */
	"\xc0\xcf\xc9\xe1\xc2\xfc", /* 老舍曼; Old Sherman */
	"\xc2\xf3\xb6\xfb\xa1\xa4\xce\xac\xc4\xc7", /* 麦尔・维那; Maxwell Verner */
	"\xd6\xec\xee\xc8\xa1\xa4\xc8\xf0\xb6\xa1", /* 朱钊・瑞丁; Millicent Redding */
	"\xce\xac\xbc\xaa\xb6\xfb\xa1\xa4\xcb\xb9\xce\xac\xb2\xbc\xc8\xf0\xc6\xe6", /* 维吉尔・斯维布瑞奇; Virgil Silverbridge */
	"\xc7\xc7\xd6\xce\xa1\xa4\xb0\xc2\xbb\xf9\xb7\xf2", /* 乔治・奥基夫; George O'Keefe */
	"\xb5\xa4\xc4\xe1\xcb\xb9\xa1\xa4\xc2\xe5\xb6\xd9\xd1\xab\xbe\xf4", /* 丹尼斯・洛顿勋爵; Lord Denys Lawton */
	"\xbd\xdc\xbd\xf0\xcb\xb9", /* 杰金斯; Jenkins */
	"\xd5\xd1\xbf\xcb\xa1\xa4\xc2\xf3\xb9\xfe\xc4\xe1", /* 昭克・麦哈尼; Jock Mahoney */
	"\xbe\xc6\xb0\xc9\xc0\xcf\xb0\xe5", /* 酒吧老板; Bartender (lit. bar owner) */
	"\xbf\xc2\xf7\xec\xc0\xf2\xd1\xc7\xa1\xa4\xc2\xe5\xbf\xcb\xc8\xf0\xc6\xe6\xd0\xa1\xbd\xe3", /* 柯黛莉亚・洛克瑞奇小姐; Lady Cordelia Lockridge */
	"\xc5\xc1\xcc\xe1\xb8\xf1\xc2\xb7", /* 帕提格路; Pettigrew */
	"\xb0\xac\xb7\xf0\xc8\xf0\xa1\xa4\xb7\xb6\xd0\xa4\xbe\xf4\xca\xbf", /* 艾佛瑞・范肖爵士; Sir Avery Fanshawe */
	"\xbb\xf4\xbd\xf0\xcb\xb9", /* 霍金斯; Hodgkins */
	"\xcd\xfe\xb6\xfb\xb2\xa9\xa1\xa4\xba\xda\xce\xe9\xa1\xb0\xc4\xf1\xcd\xb7\xb6\xf9\xa1\xb1", /* 威尔博・黑伍“鸟头儿”; Wilbur "Birdy" Heywood */
	"\xd1\xc5\xbf\xc9\xb2\xbc\xa1\xa4\xb7\xa8\xd0\xc2\xb6\xd9", /* 雅可布・法新顿; Jacob Farthington */
	"\xb7\xc6\xc0\xfb\xa1\xa4\xb2\xbc\xc0\xb3\xcb\xd5", /* 菲利・布莱苏; Philip Bledsoe */
	"\xcb\xb9\xc4\xe1\xa1\xa4\xb8\xa3\xc0\xd5", /* 斯尼・福勒; Sidney Fowler */
	"\xcb\xb9\xb6\xe0\xa1\xa4\xcd\xd0\xc2\xfc\xbd\xcc\xca\xda", /* 斯多・托曼教授; Professor Theodore Totman */
	"\xc2\xde\xcb\xb9\xa1\xa4\xd0\xc1\xb3\xbc", /* 罗斯・辛臣; Rose Hinchem */
	"\xcd\xd0\xb2\xa8\xd2\xc1", /* 托波伊; Tallboy */
	"\xd2\xc0\xc0\xd5\xb2\xa9\xa1\xa4\xc8\xf0\xc9\xad\xa1\xb0\xb2\xc3\xb7\xec\xa1\xb1", /* 依勒博・瑞森“裁缝”; Ethlebert "Stitch" Rumsey */
	"\xb2\xe9\xb6\xfb\xcb\xb9\xa1\xa4\xb8\xa5\xc0\xef\xb5\xc2\xc2\xfc", /* 查尔斯・弗里德曼; Charles Freedman */
	"\xc4\xe1\xbc\xaa\xb6\xfb\xa1\xa4\xba\xa3\xc3\xf7\xcb\xb9", /* 尼吉尔・海明斯; Nigel Hemmings */
	"\xb7\xa8\xb6\xfb\xa1\xa4\xbf\xa8\xb6\xfb\xcc\xd8", /* 法尔・卡尔特; Fairfax Carter */
	"\xcd\xfe\xba\xd5\xb6\xfe\xca\xc0", /* 威赫二世; Wilhelm II */
	"\xce\xd6\xc9\xad", /* 沃森; Wachthund */
	"\xc7\xed\xc4\xc7\xa1\xa4\xcd\xfe\xb6\xfb\xd1\xb7", /* 琼那・威尔逊; Jonathan Wilson */
	"\xb4\xf3\xce\xc0\xa1\xa4\xc2\xde\xd2\xc1\xa1\xa4\xc7\xed\xcb\xb9", /* 大卫・罗伊・琼斯; David Lloyd-Jones */
	"\xb0\xae\xb5\xc2\xbb\xaa\xa1\xa4\xb9\xfe\xb8\xf1\xc8\xf0\xb8\xa5", /* 爱德华・哈格瑞弗; Edward Hargrove */
	"\xc3\xdc\xcb\xb9\xcc\xd8\xc8\xf0", /* 密斯特瑞; Misteray */
	"\xc0\xad\xcb\xb9\xbf\xa8", /* 拉斯卡; The Lascar */
	"\xf0\xd0\xf0\xc4", /* 鹦鹉; Parrot */
	"\xce\xc4\xc9\xad\xcc\xd8\xa1\xa4\xcb\xb9\xbf\xa8\xc0\xd7\xcc\xd8", /* 文森特・斯卡雷特; Vincent Scarrett */
	"\xd1\xc7\xc0\xfa\xc9\xa3\xb5\xc2\xc0\xad", /* 亚历桑德拉; Alexandra */
	"\xce\xac\xb6\xe0\xc0\xfb\xd1\xc7\xc5\xae\xcd\xf5", /* 维多利亚女王; Queen Victoria */
	"\xd4\xbc\xba\xb2\xa1\xa4\xb2\xbc\xc0\xca", /* 约翰・布朗; John Brown */
	"\xb2\xa1\xc8\xcb", /* 病人; A patient */
	"\xb2\xa1\xc8\xcb", /* 病人; A patient */
	"\xb9\xcb\xbf\xcd", /* 顾客; Patron */
	"\xce\xac\xb6\xe0\xc0\xfb\xd1\xc7\xc5\xae\xcd\xf5", /* 维多利亚女王; Queen Victoria */
	"\xc8\xab\xc9\xed\xb9\xfc\xb0\xd7\xb5\xc4\xb2\xa1\xc8\xcb", /* 全身裹白的病人; Patient in white */
	"\xd7\xed\xba\xba", /* 醉汉; Lush */
	"\xd7\xed\xba\xba", /* 醉汉; Drunk */
	"\xbc\xcb\xc5\xae", /* 妓女; Prostitute */
	"\xc2\xea\xb5\xc2\xc0\xad\xbf\xc2", /* 玛德拉柯; Mudlark */
	"\xd0\xde\xb2\xb9\xbd\xb3", /* 修补匠; Grinder */
	"\xbe\xde\xc8\xcb", /* 巨人; Bouncer */
	"\xb0\xa3\xc4\xe1\xa1\xa4\xc2\xde\xc7\xd0\xcc\xd8", /* 埃尼・罗切特; Agnes Ratchet */
	"\xb0\xa2\xc2\xe5\xce\xf7\xb6\xfb\xcb\xb9\xa1\xa4\xc2\xde\xc7\xd0\xcc\xd8", /* 阿洛西尔斯・罗切特; Aloysius Ratchet */
	"\xbb\xca\xbc\xd2\xb7\xbf\xb5\xd8\xb2\xfa\xb4\xfa\xc0\xed\xc8\xcb", /* 皇家房地产代理人; Real Estate Agent */
	"\xd6\xb0\xd4\xb1", /* 职员; Candy Clerk */
	"\xd0\xa1\xc0\xf4", /* 小吏; Beadle */
	"\xc6\xd5\xc2\xb3\xca\xbf\xc8\xcb", /* 普鲁士人; Prussian */
	"\xc2\xde\xb2\xae\xb6\xd9\xb7\xf2\xc8\xcb", /* 罗伯顿夫人; Mrs. Rowbottom */
	"\xc2\xde\xd2\xc1\xa1\xa4\xc7\xed\xcb\xb9\xd0\xa1\xbd\xe3", /* 罗伊・琼斯小姐; Lloyd-Jones */
	"\xbe\xc6\xb5\xea\xb9\xcb\xbf\xcd", /* 酒店顾客; Tavern patron */
	"\xca\xb9\xd3\xc3\xd5\xdf", /* 使用者; User */
	"\xcd\xd0\xb1\xc8", /* 托比; Toby */
	"\xce\xc4\xbe\xdf\xc9\xcc", /* 文具商; Stationer */
	"\xd6\xb0\xd4\xb1", /* 职员; Law Clerk */
	"\xd6\xb0\xd4\xb1", /* 职员; Ministry Clerk */
	"\xe3\xe5\xd4\xa1\xd5\xdf", /* 沐浴者; Bather */
	"\xca\xcc\xc5\xae", /* 侍女; Maid */
	"\xb7\xb6\xd0\xa4\xcc\xab\xcc\xab", /* 范肖太太; Lady Fanshawe */
	"\xcb\xb9\xc4\xe1\xa1\xa4\xc2\xde\xc7\xd0\xcc\xd8", /* 斯尼・罗切特; Sidney Ratchet */
	"\xc4\xd0\xba\xa2", /* 男孩; Boy */
	"\xb9\xcb\xbf\xcd", /* 顾客; Patron */
	"\xb2\xbc\xc8\xf0\xcc\xd8\xbe\xaf\xb9\xd9", /* 布瑞特警官; Constable Brit */
	"\xc2\xed\xb3\xb5\xb3\xb5\xb7\xf2", /* 马车车夫; Wagon Driver */
};

/* This is extracted from the translation. Note that the quality is poor.  */
static const char *const fixedTextRU[] = {
	"Money",
	"Money",
	"Card",
	"Card",
	"Tobacco",
	"Tobacco",
	"Timetable",
	"Timetable",
	"Summons",
	"Summons",
	"Foolscap",
	"Foolscap",
	"Damp Paper",
	"Foolscap",
	"Bull's Eye",
	"Bull's Eye Lantern",

	"Open",
	"\x7f\x98\x61\x96" /* Глаз */,
	"Talk",
	"Use",
	"\x81\x9d\x70\x9a\x61\x98" /* Журнал */,
	"\x85\x70\x65\x94\x99\x65\x9c\xa3" /* Предметы */,
	"\x4f\x9b\x9f\x97\x97" /* Опции */,
	"Solve",
	"with",
	"No effect...",
	"This person has nothing to say at the moment",
	"Picked up",

	"Page %d",
	"\x82\x61\x6b\x70\xa3\x9c\xa4" /* Закрыть */,
	"\x85\x6f\x97\x63\x6b" /* Поиск */,
	"\x43\x6f\x78\x70\x61\x9a\x97\x9c\xa4" /* Сохранить */,
	"\x85\x70\x65\x70\x92\x61\x9c\xa4" /* Прервать */,
	"\x4f\x91\x70\x61\x9c\x9a\xa3\x97\x20\x9b\x6f\x97\x63\x6b" /* Обратныи поиск */,
	"\x85\x6f\x97\x63\x6b\x20\x92\x9b\x65\x70\x65\x94" /* Поиск вперед */,
	"Text Not Found !",
	// Darts
	"\x20\x58\x6f\x98\x99\x63" /*  Холмс */,
	"Jock",
	"Bull",
	"Round: %d",
	"Turn Total: %d",
	"Dart # %d",
	"Hit a key",
	"To start",
	"Press a key",
	"GAME OVER!",
	"BUSTED!",
	"%s Wins",
	"Scored %d point", // original: treated 1 point and multiple points the same ("Scored 1 points")
	"Scored %d points",
	"Hit a %d",
	"Hit double %d",
	"Hit triple %d",
	"Hit a bullseye",
	"Hit double bullseye",
	"Hit triple bullseye",

	"Apply",
	"Water",
	"Heat",
	"\x82\x61\x93\x70\x9d\x96\x97\x9c\xa4" /* Загрузить */,
	"\x43\x6f\x78\x70\x61\x9a\x97\x9c\xa4" /* Сохранить */,
	"\x4d\x9d\x96\xa3\x6b" /* Музык */,
	"\x8e\x9e\x9e\x65\x6b\x9c\xa3" /* Эффекты */,
	"\x50\x65\xa0\xa4" /* Речь */,
	"\x54\x65\x6b\x63\x9c" /* Текст */,
	"\x85\x70\x6f\x96\x70\x61\xa0\x9a\x6f\x65\x20\x99\x65\x9a\xa6" /* Прозрачное меню */,
	"\x43\x9c\x97\x98\xa4\x20\xa1\x70\x97\x9e\x9c\x61" /* Стиль шрифта */,
	"\x48\x65\x9c" /* Нет */,
	"\x80\x61" /* Да */,
	"\x42\xa3\x78\x94" /* Выхд */,
	"\x42\xa3\x20\x9d\x92\x65\x70\x65\x9a\xa3\x2c\x20\xa0\x9c\x6f" /* Вы уверены, что */,
	"\x78\x6f\x9c\x97\x9c\x65\x20\x92\xa3\x97\x9c\x97\x65" /* хотите выитие */,
	"\x80\x61" /* Да */,
	"\x48\x65" /* Не */,
	"Enter Password",
	"Going East", // correct password, was not and should not to be translated
	"Watson's Journal",
	"Journal saved as journal.txt",
	// SH2: People names
	"Sherlock Holmes",
	"Dr. Watson",
	"Mrs. Hudson",
	"Stanley Forbes",
	"Mycroft Holmes",
	"Wiggins",
	"Police Constable Burns",
	"Augustus Trimble",
	"Police Constable Daley",
	"Matron",
	"Sister Grace",
	"Preston McCabe",
	"Bob Colleran",
	"Jonas Rigby",
	"Police Constable Roach",
	"James Dewar",
	"Sergeant Jeremy Duncan",
	"Inspector Gregson",
	"Inspector Lestrade",
	"Jesse Needhem",
	"Arthur Fleming",
	"Mr. Thomas Pratt",
	"Mathilda (Tillie) Mason",
	"Adrian Russell",
	"Eldridge Whitney",
	"Hepplethwaite",
	"Horace Silverbridge",
	"Old Sherman",
	"Maxwell Verner",
	"Millicent Redding",
	"Virgil Silverbridge",
	"George O'Keeffe",
	"Lord Denys Lawton",
	"Jenkins",
	"Jock Mahoney",
	"Bartender",
	"Lady Cordelia Lockridge",
	"Pettigrew",
	"Sir Avery Fanshawe",
	"Hodgkins",
	"Wilbur \"Birdy\" Heywood",
	"Jacob Farthington",
	"Philip Bledsoe",
	"Sidney Fowler",
	"Professor Theodore Totman",
	"Rose Hinchem",
	"Tallboy",
	"Ethlebert \"Stitch\" Rumsey",
	"Charles Freedman",
	"Nigel Hemmings",
	"Fairfax Carter",
	"Wilhelm II",
	"Wachthund",
	"Jonathan Wilson",
	"David Lloyd-Jones",
	"Edward Hargrove",
	"Misteray",
	"The Lascar",
	"Parrot",
	"Vincent Scarrett",
	"Alexandra",
	"Queen Victoria",
	"John Brown",
	"A Patient",
	"A Patient",
	"Patron",
	"Queen Victoria",
	"Patient in white",
	"Lush",
	"Drunk",
	"Prostitute",
	"Mudlark",
	"Grinder",
	"Bouncer",
	"Agnes Ratchet",
	"Aloysius Ratchet",
	"Real Estate Agent",
	"Candy Clerk",
	"Beadle",
	"Prussian",
	"Mrs. Rowbottom",
	"Miss Lloyd-Jones",
	"Tavern patron",
	"User",
	"Toby",
	"Stationer",
	"Law Clerk",
	"Ministry Clerk",
	"Bather",
	"Maid",
	"Lady Fanshawe",
	"Sidney Ratchet",
	"Boy",
	"Patron",
	"Constable Brit",
	"Wagon Driver"
};

static const FixedTextLanguageEntry fixedTextLanguages[] = {
	{ Common::DE_DEU,   fixedTextDE },
	{ Common::ES_ESP,   fixedTextES },
	{ Common::EN_ANY,   fixedTextEN },
	{ Common::FR_FRA,   fixedTextFR },
	{ Common::ZH_CHN,   fixedTextZH },
	{ Common::RU_RUS,   fixedTextRU },
	{ Common::UNK_LANG, fixedTextEN }
};

TattooFixedText::TattooFixedText(SherlockEngine *vm) : FixedText(vm) {
	// Figure out which fixed texts to use
	Common::Language curLanguage = _vm->getLanguage();

	const FixedTextLanguageEntry *curLanguageEntry = fixedTextLanguages;

	while (curLanguageEntry->language != Common::UNK_LANG) {
		if (curLanguageEntry->language == curLanguage)
			break; // found current language
		curLanguageEntry++;
	}
	_curLanguageEntry = curLanguageEntry;
}

const char *TattooFixedText::getText(int fixedTextId) {
	return _curLanguageEntry->fixedTextArray[fixedTextId];
}

const Common::String TattooFixedText::getActionMessage(FixedTextActionId actionId, int messageIndex) {
	return Common::String();
}


} // End of namespace Tattoo

} // End of namespace Sherlock
