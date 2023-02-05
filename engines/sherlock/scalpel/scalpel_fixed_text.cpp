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

#include "sherlock/scalpel/scalpel_fixed_text.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

namespace Scalpel {

static const char *const fixedTextEN[] = {
	// Game hotkeys
	"LMTPOCIUGJFS",
	// SH1: Window buttons
	"EExit",
	"UUp",
	"DDown",
	// SH1: Inventory buttons
	"EExit",
	"LLook",
	"UUse",
	"GGive",
	// SH1: Journal text
	"Watson's Journal",
	"Page %d",
	// SH1: Journal buttons
	"EExit",
	"BBack 10",
	"UUp",
	"DDown",
	"AAhead 10",
	"SSearch",
	"FFirst Page",
	"LLast Page",
	"PPrint Text",
	// SH1: Journal search
	"EExit",
	"BBackward",
	"FForward",
	"Text Not Found !",
	// SH1: Settings
	"EExit",
	"MMusic on",
	"MMusic off",
	"PPortraits on",
	"PPortraits off",
	"JJoystick off",
	"NNew Font Style",
	"SSound Effects on",
	"SSound Effects off",
	"WWindows Slide",
	"WWindows Appear",
	"CCalibrate Joystick",
	"AAuto Help left",
	"AAuto Help right",
	"VVoices on",
	"VVoices off",
	"FFade by Pixel",
	"FFade Directly",
	"KKey Pad Slow",
	"KKey Pad Fast",
	// Load/Save
	"EExit",
	"LLoad",
	"SSave",
	"UUp",
	"DDown",
	"QQuit",
	// Quit Game
	"Are you sure you wish to Quit ?",
	"YYes",
	"NNo",
	// SH1: Press key text
	"PPress any Key for More.",
	"PPress any Key to Continue.",
	// SH1: Initial Inventory
	"A message requesting help",
	"A number of business cards",
	"Opera Tickets",
	"Cuff Link",
	"Wire Hook",
	"Note",
	"An open pocket watch",
	"A piece of paper with numbers on it",
	"A letter folded many times",
	"Tarot Cards",
	"An ornate key",
	"A pawn ticket",
	// SH1: User Interface
	"No, thank you.",
	"You can't do that.",
	"Done...",
	"Use ",
	" on %s",
	"Give ",
	" to %s",
	// SH1: People names
	"Sherlock Holmes",
	"Dr. Watson",
	"Inspector Lestrade",
	"Constable O'Brien",
	"Constable Lewis",
	"Sheila Parker",
	"Henry Carruthers",
	"Lesley",
	"An Usher",
	"Fredrick Epstein",
	"Mrs. Worthington",
	"The Coach",
	"A Player",
	"Tim",
	"James Sanders",
	"Belle",
	"Cleaning Girl",
	"Wiggins",
	"Paul",
	"The Bartender",
	"A Dirty Drunk",
	"A Shouting Drunk",
	"A Staggering Drunk",
	"The Bouncer",
	"The Coroner",
	"Reginald Snipes",
	"George Blackwood",
	"Lars",
	"The Chemist",
	"Inspector Gregson",
	"Jacob Farthington",
	"Mycroft",
	"Old Sherman",
	"Richard",
	"The Barman",
	"A Dandy Player",
	"A Rough-looking Player",
	"A Spectator",
	"Robert Hunt",
	"Violet",
	"Pettigrew",
	"Augie",
	"Anna Carroway",
	"A Guard",
	"Antonio Caruso",
	"Toby the Dog",
	"Simon Kingsley",
	"Alfred",
	"Lady Brumwell",
	"Madame Rosa",
	"Joseph Moorehead",
	"Mrs. Beale",
	"Felix",
	"Hollingston",
	"Constable Callaghan",
	"Sergeant Duncan",
	"Lord Brumwell",
	"Nigel Jaimeson",
	"Jonas",
	"Constable Dugan"
};

// sharp-s       : 0xE1 / octal 341
// small a-umlaut: 0x84 / octal 204
// small o-umlaut: 0x94 / octal 224
// small u-umlaut: 0x81 / octal 201
static const char *const fixedTextDE[] = {
	// Game hotkeys
	"SBRNOCTEGADU", // original: did not support hotkeys for actions
	// SH1: Window buttons
	"ZZur\201ck",
	"HHoch",
	"RRunter",
	// SH1: Inventory buttons
	"ZZur\201ck",
	"SSchau",
	"BBenutze",
	"GGib",
	// SH1: Journal text
	"Watsons Tagebuch",
	"Seite %d",
	// SH1: Journal buttons
	"ZZur\201ck", // original: "Zur\201ck"
	"o10 hoch",
	"HHoch",
	"RRunter",
	"u10 runter", // original: "10 runter"
	"SSuche",
	"EErste Seite",
	"LLetzte Seite",
	"DDrucke Text",
	// SH1: Journal search
	"ZZur\201ck",
	"RR\201ckw\204rts", // original: "Backward"
	"VVorw\204rts", // original: "Forward"
	"Text nicht gefunden!",
	// SH1: Settings
	"ZZur\201ck", // original interpreter: "Exit"
	"MMusik an",
	"MMusik aus",
	"PPortr\204ts an", // original interpreter: "Portraits"
	"PPortr\204ts aus",
	"JJoystick aus",
	"NNeue Schrift",
	"GGer\204uscheffekte on", // original interpreter: "Effekte"
	"GGer\204uscheffekte off",
	"FFenster gleitend",
	"FFenster direkt",
	"JJustiere Joystick",
	"HHilfe links",
	"HHilfe rechts",
	"SSprache an",
	"SSprache aus",
	"cSchnitt",
	"BBlende",
	"CCursor langsam",
	"CCursor schnell",
	// Load/Save
	"ZZur\201ck",
	"LLaden",
	"SSichern",
	"HHoch",
	"RRunter",
	"EEnde",
	// Quit Game
	"Das Spiel verlassen ?",
	"JJa",
	"NNein",
	// SH1: Press key text
	"MMehr auf Tastendruck...",
	"BBeliebige Taste dr\201cken.",
	// SH1: Initial Inventory
	"Ein Hilferuf von Lestrade",
	"Holmes' Visitenkarten",
	"Karten f\201rs Opernhaus",
	"Manschettenkn\224pfe",
	"Zum Haken verbogener Drahtkorb",
	"Mitteilung am Epstein",
	"Eine offene Taschenuhr",
	"Ein Zettel mit Zahlen drauf",
	"Ein mehrfach gefalteter Briefbogen",
	"Ein Tarot-Kartenspiel", // original interpreter: "Ein Tarock-Kartenspiel" [sic]
	"Ein verzierter Schl\201ssel",
	"Ein Pfandschein",
	// SH1: User Interface
	"Nein, vielen Dank.",
	"Nein, das geht wirklich nicht.", // original: "Nein, das geht wirklich nicht"
	"Fertig...",
	"Benutze ",
	" mit %s",
	"Gib ", // original: "Gebe "
	" an %s", // original: " zu %s"
	// SH1: People names
	"Sherlock Holmes",
	"Dr. Watson",
	"Inspektor Lestrade",
	"Konstabler O'Brien",
	"Konstabler Lewis",
	"Sheila Parker",
	"Henry Carruthers",
	"Lesley",
	"Platzanweiser",
	"Fredrick Epstein",
	"Mrs. Worthington",
	"Der Trainer",
	"Ein Spieler",
	"Tim",
	"James Sanders",
	"Belle",
	"Putzm\204dchen",
	"Wiggins",
	"Paul",
	"Gastwirt",
	"Schmutziger Betrunkener",
	"Lallender Betrunkener",
	"Torkelnder Betrunkener",
	"The Bouncer",
	"Der Leichenbeschauer",
	"Reginald Snipes",
	"George Blackwood",
	"Lars",
	"Apotheker",
	"Inspektor Gregson",
	"Jacob Farthington",
	"Mycroft",
	"Old Sherman",
	"Richard",
	"Barkeeper",
	"Jock Mahoney",
	"Nobby Charleton",
	"Zuschauer",
	"Robert Hunt",
	"Violet",
	"Pettigrew",
	"Augie",
	"Anna Carroway",
	"Wache",
	"Antonio Caruso",
	"Toby the Dog",
	"Simon Kingsley",
	"Alfred",
	"Lady Brumwell",
	"Madame Rosa",
	"Joseph Moorehead",
	"Mrs. Beale",
	"Felix",
	"Hollingston",
	"Konstabler Callaghan",
	"Sergeant Duncan",
	"Lord Brumwell",
	"Nigel Jaimeson",
	"Jonas",
	"Konstabler Dugan"
};

// up-side down exclamation mark - 0xAD / octal 255
// up-side down question mark - 0xA8 / octal 250
// n with a wave on top - 0xA4 / octal 244
// more characters see engines/sherlock/fixed_text.cpp
static const char *const fixedTextES[] = {
	// Game hotkeys
	"VMHTACIUDNFO",
	// SH1: Window buttons
	"aSalir", // original interpreter: "Exit"
	"SSubir",
	"BBajar",
	// SH1: Inventory buttons
	"SSalir", // original interpreter: "Exit"
	"MMirar",
	"UUsar",
	"DDar",
	// SH1: Journal text
	"Diario de Watson",
	"Pagina %d",
	// SH1: Journal buttons
	"aSalir", // original interpreter: "Exit"
	"RRetroceder",
	"SSubir",
	"JbaJar",
	"AAdelante",
	"BBuscar",
	"11a pagina",
	"UUlt pagina",
	"IImprimir",
	// SH1: Journal search
	"SSalir", // original interpreter: "Exit"
	"RRetroceder",
	"AAvanzar",
	"Texto no encontrado!",
	// SH1: Settings
	"aSalir", // original interpreter: "Exit"
	"MMusica si",
	"MMusica no",
	"RRetratos si",
	"RRetratos no",
	"JJoystick no",
	"NNuevo fuente",
	"Sefectos Sonido si",
	"Sefectos Sonido no",
	"Tven Tanas desliz.",
	"Tven Tanas aparecen",
	"CCalibrar Joystick",
	"yAyuda lzq", // TODO: check this
	"yAyuda Dcha",
	"VVoces si",
	"VVoces no",
	"FFundido a pixel",
	"FFundido directo",
	"eTeclado lento",
	"eTeclado rapido",
	// Load/Save
	"aSalir", // original interpreter: "Exit"
	"CCargar",
	"GGrabar",
	"SSubir",
	"BBajar",
	"AAcabar",
	// Quit Game
	"\250Seguro que quieres Acabar?",
	"SSi",
	"NNo",
	// SH1: Press key text
	"TTecla para ver mas",
	"TTecla para continuar",
	// SH1: Initial Inventory
	"Un mensaje solicitando ayuda",
	"Unas cuantas tarjetas de visita",
	"Entradas para la opera",
	"Unos gemelos",
	"Un gancho de alambre",
	"Una nota",
	"Un reloj de bolsillo abierto",
	"Un trozo de papel con unos numeros",
	"Un carta muy plegada",
	"Unas cartas de Tarot",
	"Una llave muy vistosa",
	"Una papeleta de empe\244o",
	// SH1: User Interface
	"No, gracias.",
	"No puedes hacerlo.", // original: "No puedes hacerlo"
	"Hecho...",
	"Usar ",
	" sobre %s",
	"Dar ",
	" a %s",
	// SH1: People names
	"Sherlock Holmes",
	"Dr. Watson",
	"El inspector Lestrade",
	"El agente O'Brien",
	"El agente Lewis",
	"Sheila Parker",
	"Henry Carruthers",
	"Lesley",
	"Un ujier",
	"Fredrick Epstein",
	"Mrs. Worthington",
	"El entrenador",
	"El jugador",
	"Tim",
	"James Sanders",
	"Belle",
	"La chica de la limpieza",
	"Wiggins",
	"Paul",
	"El barman",
	"Un sucio borracho",
	"Un borracho griton",
	"Un tambaleante borracho",
	"El gorila",
	"El forense",
	"Reginald Snipes",
	"George Blackwood",
	"Lars",
	"El quimico",
	"El inspector Gregson",
	"Jacob Farthington",
	"Mycroft",
	"Old Sherman",
	"Richard",
	"El barman",
	"Un jugador dandy",
	"Un duro jugador",
	"Un espectador",
	"Robert Hunt",
	"Violeta",
	"Pettigrew",
	"Augie",
	"Anna Carroway",
	"Un guarda",
	"Antonio Caruso",
	"El perro Toby",
	"Simon Kingsley",
	"Alfred",
	"Lady Brumwell",
	"Madame Rosa",
	"Joseph Moorehead",
	"Mrs. Beale",
	"Felix",
	"Hollingston",
	"El agente Callaghan",
	"El sargento Duncan",
	"Lord Brumwell",
	"Nigel Jaimeson",
	"Jonas",
	"El agente Dugan"
};

static const char *const fixedTextZH[] = {
	// Game hotkeys
	"LMTPOCIUGJFS",
	// SH1: Window buttons
	"E\xc2\xf7\xb6\x7d(E)", /* "E離開"; "EExit" */
	"U\xa4\x57(U)", /* "U上"; "UUp" */
	"D\xa4\x55(D)", /* "D下"; "DDown" */
	// SH1: Inventory buttons
	"E\xc2\xf7\xb6\x7d(E)", /* "E離開"; "EExit" */
	"L\xac\x64\xac\xdd(L)", /* "L查看"; "LLook" */
	"U\xa8\xcf\xa5\xce(U)", /* "U使用"; "UUse" */
	"G\xb5\xb9\xbb\x50(G)", /* "G給與"; "GGive" */
	// TODO: Inventorty next/prev buttons:
	//"\xa5\xaa\xad\xb6", /* "左頁"; */
	//"\xa5\xaa\xa4\x40", /* "左一"; */
	//"\xa5\x6b\xa4\x40", /* "右一"; */
	//"\xa5\x6b\xad\xb6", /* "右頁"; */
	// SH1: Journal text
	"\xb5\xd8\xa5\xcd\xaa\xba\xb5\xa7\xb0\x4f", /* "華生的筆記"; "Watson's Journal" */
	"\xb2\xc4\x25\x64\xad\xb6", /* "第%d頁"; "Page %d" */
	// SH1: Journal buttons
	"E\xc2\xf7\xb6\x7d(E)", /* "E離開"; "EExit" */
	"B\xab\x65\xa4\x51\xad\xb6(B)", /* "B前十頁"; "BBack 10" */
	"U\xa4\x57(U)", /* "U上"; "UUp" */
	"D\xa4\x55(D)", /* "D下"; "DDown" */
	"A\xab\xe1\xa4\x51\xad\xb6(A)", /* "A後十頁"; "AAhead 10" */
	"S\xb4\x4d\xa7\xe4(S)", /* "S尋找"; "SSearch" */
	"F\xad\xba\xad\xb6(F)", /* "F首頁"; "FFirst Page" */
	"L\xa9\xb3\xad\xb6(L)", /* "L底頁"; "LLast Page" */
	"P\xa6\x43\xa6\x4c(P)", /* "列印"; "PPrint Text" */
	// SH1: Journal search
	"\xc2\xf7\xb6\x7d", /* "E離開"; "Exit" */
	"\xab\x65\xb4\x4d", /* "前尋"; "Backward" */
	"\xab\xe1\xb4\x4d", /* "後尋"; "Forward" */
	"\xa8\x53\xa6\xb3\xa7\xe4\xa8\xec\x21", /* "沒有找到!"; "Text Not Found !" */
	// SH1: Settings
	"E\xc2\xf7\xb6\x7d(E)", /* "離開"; "EExit" */
	"M\xad\xb5\xbc\xd6\xb6\x7d(M)", /* "M音樂開"; "MMusic on" */
	"M\xad\xb5\xbc\xd6\xc3\xf6(M)", /* "M音樂關"; "MMusic off" */	
	"P\xa8\x76\xb9\xb3\xb6\x7d(P)", /* "P肖像開"; "PPortrait on" */
	"P\xa8\x76\xb9\xb3\xc3\xf6(P)", /* "P肖像關"; "PPortrait off" */
	"JJoystick off", // Not used in Chinese as this button is skipped
	"NNew Font Style", // Not used in Chinese as only one font is available
	"S\xad\xb5\xae\xc4\xb6\x7d(S)", /* "S音效開"; "SSound Effects on" */
	"S\xad\xb5\xae\xc4\xc3\xf6(S)", /* "S音效關"; "SSound Effects off" */	
	"W\xb5\xf8\xb5\xa1\xb7\xc6\xb1\xb2(W)", /* "W視窗滑捲"; "WWindow Slide Scroll" */
	"W\xb5\xf8\xb5\xa1\xa8\x71\xa5\x58(W)", /* "W視窗秀出"; "WWindow Show" */
	"C\xbd\xd5\xbe\xe3\xb7\x6e\xb1\xec(C)", /* "調整搖桿"; "CCalibrate Joystick" */
	"A\xbb\xb2\xa7\x55\xa5\xaa(A)", /* "A輔助左"; "AAuto Help left" */
	"A\xbb\xb2\xa7\x55\xa5\x6b(A)", /* "A輔助右"; "AAuto Help right" */
	"VVoices on", // Not used in Chinese as no voices are available
	"VVoices off", // Not used in Chinese as no voices are available
	"F\xb2\x48\xa5\x58\xc2\x49\xaa\xac(F)", /* "F淡出點狀"; "FFade by Pixel" */
	"F\xb2\x48\xa5\x58\xaa\xbd\xb1\xb5(F)", /* "F淡出直接"; "FFade Directly" */	
	"K\xc1\xe4\xaa\xa9\xba\x43(K)", /* "K鍵版慢"; "KKey Pad Slow" */
	"K\xc1\xe4\xaa\xa9\xa7\xd6(K)", /* "K鍵版快"; "KKey Pad Fast" */
	// Load/Save
	"EExit", // TODO
	"L\xb8\xfc\xa4\x4a(L)", /* "L載入"; "LLoad" */
	"S\xc0\x78\xa6\x73(S)", /* "S儲存"; "SSave" */
	"U\xa4\x57(U)", /* "U上"; "UUp" */
	"D\xa4\x55(D)", /* "D下"; "DDown" */
	"Q\xb5\xb2\xa7\xf4(Q)", /* "Q結束"; "QQuit" */
	// Quit Game
	"\xb1\x7a\xbd\x54\xa9\x77\xad\x6e\xb5\xb2\xa7\xf4\xb9\x43\xc0\xb8\xb6\xdc\x3f", /* "您確定要結束遊戲嗎?"; "Are you sure you wish to Quit ?" */
	"Y\xac\x4f(Y)", /* "Y是"; "YYes" */
	"N\xa4\xa3(N)", /* "N不"; "NNo" */
	// SH1: Press key text
	"P\xbd\xd0\xab\xf6\xa5\xf4\xb7\x4e\xc1\xe4\xc4\x7e\xc4\xf2\xa4\x55\xad\xb6\xa4\xba\xae\x65.(P)", /* "P請按任意鍵繼續下頁內容."; "PPress any Key for More." */
	"P\xbd\xd0\xab\xf6\xa5\xf4\xb7\x4e\xc1\xe4\xc4\x7e\xc4\xf2.(P)", /* "P請按任意鍵繼續."; "PPress any Key to Continue." */
	// SH1: Initial Inventory
	"\xab\x4b\xb1\xf8\xaf\xc8", /* "便條紙"; "A message requesting help" */
	"\xba\xd6\xba\xb8\xbc\xaf\xb4\xb5\xaa\xba\xa6\x57\xa4\xf9", /* "福爾摩斯的名片"; "A number of business cards" */
	"\xba\x71\xbc\x40\xb0\x7c\xc1\x70\xb2\xbc", /* "歌劇院聯票"; "Opera Tickets" */
	"\xb3\x53\xb3\xa7", /* "袖釦"; "Cuff Link" */
	"\xc5\x4b\xb5\xb7\xa4\xc4", /* "鐵絲勾"; "Wire Hook" */
	"\xa9\xf1\xa6\xe6\xb1\xf8", /* "放行條"; "Note" */
	"\xa5\xb4\xb6\x7d\xaa\xba\xc3\x68\xbf\xf6", /* "打開的懷錶"; "An open pocket watch" */
	"\xaf\xc8", /* "紙"; "A piece of paper with numbers on it" */
	"\xab\x48", /* "信"; "A letter folded many times" */
	"\xaf\xc8\xb5\x50", /* "紙牌"; "Tarot Cards" */
	"\xb5\xd8\xc4\x52\xaa\xba\xc6\x5f\xb0\xcd", /* "華麗的鑰匙"; "An ornate key" */
	"\xb7\xed\xb2\xbc", /* "當票"; "A pawn ticket" */
	// SH1: User Interface
	"\xa4\xa3\x2c\xc1\xc2\xc1\xc2\xb1\x7a\x2e", /* "不,謝謝您."; "No, thank you." */
	"You can't do that.", // TODO
	"\xa7\xb9\xb2\xa6\x2e\x2e\x2e", /* "完畢..."; "Done..." */
	"Use ", // TODO
	" on %s", // TODO
	"Give ", // TODO
	" to %s", // TODO
	// SH1: People names
	"\xba\xd6\xba\xb8\xbc\xaf\xb4\xb5", /* "福爾摩斯"; "Sherlock Holmes" */
	"\xb5\xd8\xa5\xcd\xc2\xe5\xa5\xcd", /* "華生醫生"; "Dr. Watson" */
	"\xb5\xdc\xb4\xb5\xb1\x5a\xbc\x77\xb1\xb4\xaa\xf8", /* "萊斯崔德探長"; "Inspector Lestrade" */
	"\xb6\xf8\xa5\xac\xb5\xdc\xa6\x77\xa8\xb5\xa6\xf5", /* "奧布萊安巡佐"; "Constable O'Brien" */
	"\xb9\x70\xba\xfb\xb4\xb5\xa8\xb5\xa6\xf5", /* "雷維斯巡佐"; "Constable Lewis" */
	"\xdf\xc4\xdb\x69\x2e\xa9\xac\xa7\x4a", /* "葸菈.帕克"; "Sheila Parker" */
	"\xa6\xeb\xa7\x51\x2e\xa5\x64\xcd\xba\xb7\xe6", /* "亨利.卡芮瑟"; "Henry Carruthers" */
	"\xb5\xdc\xb5\xb7\xb2\xfa", /* "萊絲莉"; "Lesley" */
	"\xa4\xde\xae\x79\xad\xfb", /* "引座員"; "An Usher" */
	"\xa5\xb1\xb7\xe7\xbc\x77\x2e\xa6\xe3\xa7\x42\xb4\xb5\xa5\xc5", /* "弗瑞德.艾伯斯汀"; "Fredrick Epstein" */
	"\xb4\xec\xa8\xaf\xb9\x79\xa4\xd3\xa4\xd3", /* "渥辛頓太太"; "Mrs. Worthington" */
	"\xb1\xd0\xbd\x6d", /* "教練"; "The Coach" */
	"\xa4\x40\xa6\x57\xb6\xa4\xad\xfb", /* "一名隊員"; "A Player" */
	"\xb4\xa3\xa9\x69", /* "提姆"; "Tim", */
	"\xa9\x69\xa4\x68\x2e\xae\xe1\xbc\x77\xb4\xb5", /* "姆士.桑德斯"; "James Sanders" */
	"\xa8\xa9\xb2\xfa", /* "貝莉"; "Belle" */
	"\xb2\x4d\xbc\xe4\xa4\x6b\xa4\x75", /* "清潔女工"; "Cleaning Girl" */
	"\xc3\x51\xaa\xf7\xb4\xb5", /* "魏金斯"; "Wiggins" */
	"\xab\x4f\xc3\xb9", /* "保羅"; "Paul" */
	"\xb0\x73\xab\x4f", /* "酒保"; "The Bartender" */
	"\xa4\x40\xad\xd3\xbb\xea\xc5\xbc\xaa\xba\xb0\x73\xb0\xad", /* "一個骯髒的酒鬼"; "A Dirty Drunk" */
	"\xa4\x40\xad\xd3\xa4\x6a\xc1\x6e\xbb\xa1\xb8\xdc\xaa\xba\xb0\x73\xb0\xad", /* "一個大聲說話的酒鬼"; "A Shouting Drunk" */
	"\xa4\x40\xad\xd3\xa8\xab\xb8\xf4\xb7\x6e\xb7\x45\xaa\xba\xb0\x73\xb0\xad", /* "一個走路搖幌的酒鬼"; "A Staggering Drunk" */
	"\xab\x4f\xc3\xf0", /* "保鏢"; "The Bouncer" */
	"\xc5\xe7\xab\xcd\xa9\x78", /* "驗屍官"; "The Coroner" */
	"\xc3\x4d\xa4\x68\xaa\x41\xa9\xb1\xaa\xba\xb9\xd9\xad\x70", /* "騎士服店的夥計"; "Reginald Snipes" lit. "The clerk of the knight clothing store" */
	"\xb3\xec\xaa\x76\x2e\xa5\xac\xb5\xdc\xa7\x4a\xa5\xee", /* "喬治.布萊克伍"; "George Blackwood" */
	"\xbf\xe0\xa6\xd5\xb4\xb5", /* "賴耳斯"; "Lars" */
	"\xc3\xc4\xa9\xd0\xa6\xd1\xaa\x4f", /* "藥房老板"; "The Chemist" lit "Pharmacy owner" */
	"\xb8\xaf\xb7\xe7\xb4\xcb\xb1\xb4\xaa\xf8", /* "葛瑞森探長"; "Inspector Gregson" */
	"\xb8\xeb\xa5\x69\xa7\x42\x2e\xaa\x6b\xa8\xaf\xb9\x79", /* "賈可伯.法辛頓"; "Jacob Farthington" */
	"\xb3\xc1\xa6\xd2\xa4\xd2", /* "麥考夫"; "Mycroft" */
	"\xa6\xd1\xb3\xb7\xb0\xd2", /* "老雪曼"; "Old Sherman" */
	"\xb2\x7a\xac\x64", /* "理查"; "Richard" */
	"\xbd\xd5\xb0\x73\xae\x76", /* "調酒師"; "The Barman" */
	"\xa4\x40\xad\xd3\xa4\x40\xac\x79\xaa\xba\xaa\xb1\xaa\xcc", /* "一個一流的玩者"; "A Dandy Player" */
	"\xa4\x40\xad\xd3\xa4\x54\xac\x79\xaa\xba\xaa\xb1\xaa\xcc", /* "一個三流的玩者"; "A Rough-looking Player" lit "A third-rate player" */
	"\xae\xc7\xc6\x5b\xaa\xcc", /* "旁觀者"; "A Spectator" */
	"\xc3\xb9\xa7\x42\x2e\xba\x7e\xaf\x53", /* "羅伯.漢特"; "Robert Hunt" */
	"Violet", // TODO, Maybe "\xcb\xa2\xb5\xdc\xaf\x53", /* "芃萊特" */
	"\xa8\xa9\xab\xd2\xae\xe6\xbe\x7c", /* "貝帝格魯"; "Pettigrew" */
	"\xb6\xf8\xa6\x4e", /* "奧吉"; "Augie" */
	"\xa6\x77\xae\x52\x2e\xa5\x64\xac\xa5\xc1\xa8", /* "安娜.卡洛薇"; "Anna Carroway" */
	"\xc4\xb5\xbd\xc3", /* "警衛"; "A Guard" */
	"\xa6\x77\xaa\x46\xa5\xa7\xb6\xf8\x2e\xa5\x64", /* "安東尼奧.卡"; "Antonio Caruso" */
	"\xa6\xab\xa4\xf1", /* "托比"; "Toby the Dog" lit "Toby" */
	"\xa6\xe8\xbb\x58\x2e\xaa\xf7\xb4\xb5\xb5\xdc", /* "西蒙.金斯萊"; "Simon Kingsley" */
	"\xa8\xc8\xa6\xf2\xa6\x43\xbc\x77", /* "亞佛列德"; "Alfred" */
	"\xa5\xac\xaa\xf9\xab\xc2\xba\xb8\xa4\xd2\xa4\x48", /* "布門威爾夫人"; "Lady Brumwell" */
	"\xc3\xb9\xb2\xef\xa4\xd2\xa4\x48", /* "羅莎夫人"; "Madame Rosa" */
	"\xac\xf9\xb7\xe6\x2e\xbc\xaf\xba\xb8\xae\xfc\xbc\x77", /* "約瑟.摩爾海德"; "Joseph Moorehead" */
	"\xb2\xa6\xba\xb8\xa4\xd3\xa4\xd3", /* "畢爾太太"; "Mrs. Beale" */
	"\xb5\xe1\xa7\x51\xa7\x4a\xb4\xb5", /* "菲利克斯"; "Felix" */
	"\xb2\xfc\xc6\x46\xb9\x79", /* "荷靈頓"; "Hollingston" */
	"\xa5\x64\xb5\xdc\xba\x7e\xa8\xb5\xa6\xf5", /* "卡萊漢巡佐"; "Constable Callaghan" */
	"\xbe\x48\xaa\xd6\xa8\xb5\xa6\xf5", /* "鄧肯巡佐"; "Sergeant Duncan" */
	"\xa5\xac\xaa\xf9\xab\xc2\xba\xb8\xc0\xef\xa4\x68", /* "布門威爾爵士"; "Lord Brumwell" */
	"\xa5\xa7\xae\xe6\x2e\xb3\xc7\xa9\x69\xb4\xcb", /* "尼格.傑姆森"; "Nigel Jaimeson" */
	"\xc1\xe9\xaf\xc7\xb4\xb5\x2e\xb7\xe7\xa7\x4a", /* "鍾納斯.瑞克"; "Jonas" */
	"\xbe\x48\xae\xda\xa8\xb5\xa6\xf5" /* "鄧根巡佐"; "Constable Dugan" */
};

// =========================================

// === Sherlock Holmes 1: Serrated Scalpel ===
static const char *const fixedTextEN_ActionOpen[] = {
	"This cannot be opened",
	"It is already open",
	"It is locked",
	"Wait for Watson",
	" ",
	"."
};

static const char *const fixedTextDE_ActionOpen[] = {
	"Das kann man nicht \224ffnen",
	"Ist doch schon offen!",
	"Leider verschlossen",
	"Warte auf Watson",
	" ",
	"."
};

static const char *const fixedTextES_ActionOpen[] = {
	"No puede ser abierto",
	"Ya esta abierto",
	"Esta cerrado",
	"Espera a Watson",
	" ",
	"."
};

static const char *const fixedTextZH_ActionOpen[] = {
	"\xb3\x6f\xb5\x4c\xaa\x6b\xa5\xb4\xb6\x7d\xaa\xba", /* "這無法打開的"; "This cannot be opened" */
	"\xa5\xa6\xa4\x77\xb8\x67\xa5\xb4\xb6\x7d\xa4\x46", /* "它已經打開了"; "It is already open" */
	"\xa5\xa6\xb3\x51\xc2\xea\xa6\xed\xa4\x46", /* "它被鎖住了"; "It is locked" */
	"\xb5\xa5\xab\xdd\xb5\xd8\xa5\xcd", /* "等待華生"; "Wait for Watson" */
	" ",
	"."
};

static const char *const fixedTextEN_ActionClose[] = {
	"This cannot be closed",
	"It is already closed",
	"The safe door is in the way"
};

static const char *const fixedTextDE_ActionClose[] = {
	"Das kann man nicht schlie\341en",
	"Ist doch schon zu!",
	"Die safet\201r ist Weg"
};

static const char *const fixedTextES_ActionClose[] = {
	"No puede ser cerrado",
	"Ya esta cerrado",
	"La puerta de seguridad esta entre medias"
};

static const char *const fixedTextZH_ActionClose[] = {
	"\xb3\x6f\xb5\x4c\xaa\x6b\xc3\xf6\xa6\xed\xaa\xba", /* "這無法關住的"; "This cannot be closed" */
	"\xa5\xa6\xa4\x77\xb8\x67\xc3\xf6\xb0\x5f\xa8\xd3\xa4\x46", /* "它已經關起來了"; "It is already closed" */
	"The safe door is in the way", // TODO
};

static const char *const fixedTextEN_ActionMove[] = {
	"This cannot be moved",
	"It is bolted to the floor",
	"It is too heavy",
	"The other crate is in the way"
};


static const char *const fixedTextDE_ActionMove[] = {
	"L\204\341t sich nicht bewegen",
	"Festged\201belt in der Erde...",
	"Oha, VIEL zu schwer",
	"Die andere Kiste ist im Weg" // original: "Der andere Kiste ist im Weg"
};

static const char *const fixedTextES_ActionMove[] = {
	"No puede moverse",
	"Esta sujeto a la pared",
	"Es demasiado pesado",
	"El otro cajon esta en mitad"
};

static const char *const fixedTextZH_ActionMove[] = {
	"\xb3\x6f\xb5\x4c\xaa\x6b\xb2\xbe\xb0\xca\xaa\xba", /* "這無法移動的"; "This cannot be moved" */
	"It is bolted to the floor",  // TODO
	"\xb3\x6f\xaa\x46\xa6\xe8\xa4\xd3\xad\xab\xa4\x46", /* "這東西太重了"; "It is too heavy" */
	"\xb3\x51\xa8\xe4\xa5\xa6\xaa\xba\xa4\xec\xbd\x63\xbe\xd7\xa6\xed\xb8\xf4\xa4\x46", /* "被其它的木箱擋住路了"; "The other crate is in the way" */
};

static const char *const fixedTextEN_ActionPick[] = {
	"Nothing of interest here",
	"It is bolted down",
	"It is too big to carry",
	"It is too heavy",
	"I think a girl would be more your type",
	"Those flowers belong to Penny",
	"She's far too young for you!",
	"I think a girl would be more your type!",
	"Government property for official use only"
};

static const char *const fixedTextDE_ActionPick[] = {
	"Nichts Interessantes da",
	"Zu gut befestigt",
	"Ist ja wohl ein bi\341chen zu gro\341, oder ?",
	"Oha, VIEL zu schwer",
	"Ich denke, Du stehst mehr auf M\204dchen ?",
	"Diese Blumen geh\224ren Penny",
	"Sie ist doch viel zu jung f\201r Dich!",
	"Ich denke, Du stehst mehr auf M\204dchen ?",
	"Staatseigentum - Nur f\201r den Dienstgebrauch !"
};

static const char *const fixedTextES_ActionPick[] = {
	"No hay nada interesante",
	"Esta anclado al suelo",
	"Es muy grande para llevarlo",
	"Pesa demasiado",
	"Creo que una chica sera mas tu tipo",
	"Esas flores pertenecen a Penny",
	"\255Es demasiado joven para ti!",
	"\255Creo que una chica sera mas tu tipo!",
	"Propiedad del gobierno para uso oficial"
};

static const char *const fixedTextZH_ActionPick[] = {
	"\xa8\x53\xa6\xb3\xa4\xb0\xbb\xf2\xa5\x69\xad\xc8\xb1\x6f\xae\xb3", /* "沒有什麼可值得拿"; "Nothing of interest here"  */
	"It is bolted down",  // TODO
	"It is too big to carry",  // TODO
	"\xa8\xba\xa4\xd3\xad\xab\xa4\x46", /* "那太重了"; "It is too heavy" */
	"I think a girl would be more your type",  // TODO
	"Those flowers belong to Penny",  // TODO
	"She's far too young for you!",  // TODO
	"I think a girl would be more your type!",  // TODO
	"\xac\x46\xa9\xb2\xa9\xd2\xa6\xb3\x2c\xb6\xc8\xaf\xe0\xa8\xd1\xa9\x78\xa4\xe8\xa8\xcf\xa5\xce" /* "政府所有,僅能供官方使用"; "Government property for official use only" */
};

static const char *const fixedTextEN_ActionUse[] = {
	"You can't do that",
	"It had no effect",
	"You can't reach it",
	"OK, the door looks bigger! Happy?",
	"Doors don't smoke"
};

static const char *const fixedTextDE_ActionUse[] = {
	"Nein, das geht wirklich nicht",
	"Tja keinerlei Wirkung",
	"Da kommst du nicht dran",
	"Na gut, die T\201r sieht jetzt gr\224\341er aus. Zufrieden?",
	"T\201ren sind Nichtraucher!"
};

static const char *const fixedTextES_ActionUse[] = {
	"No puedes hacerlo",
	"No tuvo ningun efecto",
	"No puedes alcanzarlo",
	"Bien, \255es enorme! \250Feliz?",
	"Las puertas no fuman"
};


static const char *const fixedTextZH_ActionUse[] = {
	"\xb1\x7a\xb5\x4c\xaa\x6b\xa8\xba\xbc\xcb\xa8\xcf\xa5\xce", /* "您無法那樣使用"; "You can't do that" */
	"\xa5\xa6\xac\x4f\xa8\x53\xa6\xb3\xae\xc4\xaa\x47\xaa\xba", /* "它是沒有效果的"; "It had no effect" */
	"\xb1\x7a\xb5\x4c\xaa\x6b\xa8\xec\xb9\x46\xa8\xba\xc3\xe4", /* "您無法到達那邊"; "You can't reach it" */
	"\xa6\x6e\xa4\x46\x21\xaa\xf9\xa4\x77\xb8\x67\xb6\x7d\xa4\x46\x2c\xb0\xaa\xbf\xb3\xb6\xdc\x3f", /* "好了!門已經開了,高興嗎?"; "OK, the door looks bigger! Happy?" */
	"\xaa\xf9\xb5\x4c\xaa\x6b\xa9\xe2\xb7\xcf" /* "門無法抽煙"; "Doors don't smoke" */
};

#define FIXEDTEXT_GETCOUNT(_name_) sizeof(_name_) / sizeof(byte *)
#define FIXEDTEXT_ENTRY(_name_)    _name_, FIXEDTEXT_GETCOUNT(_name_)

static const FixedTextActionEntry fixedTextEN_Actions[] = {
	{ FIXEDTEXT_ENTRY(fixedTextEN_ActionOpen) },
	{ FIXEDTEXT_ENTRY(fixedTextEN_ActionClose) },
	{ FIXEDTEXT_ENTRY(fixedTextEN_ActionMove) },
	{ FIXEDTEXT_ENTRY(fixedTextEN_ActionPick) },
	{ FIXEDTEXT_ENTRY(fixedTextEN_ActionUse) }
};

static const FixedTextActionEntry fixedTextDE_Actions[] = {
	{ FIXEDTEXT_ENTRY(fixedTextDE_ActionOpen) },
	{ FIXEDTEXT_ENTRY(fixedTextDE_ActionClose) },
	{ FIXEDTEXT_ENTRY(fixedTextDE_ActionMove) },
	{ FIXEDTEXT_ENTRY(fixedTextDE_ActionPick) },
	{ FIXEDTEXT_ENTRY(fixedTextDE_ActionUse) }
};

static const FixedTextActionEntry fixedTextES_Actions[] = {
	{ FIXEDTEXT_ENTRY(fixedTextES_ActionOpen) },
	{ FIXEDTEXT_ENTRY(fixedTextES_ActionClose) },
	{ FIXEDTEXT_ENTRY(fixedTextES_ActionMove) },
	{ FIXEDTEXT_ENTRY(fixedTextES_ActionPick) },
	{ FIXEDTEXT_ENTRY(fixedTextES_ActionUse) }
};

static const FixedTextActionEntry fixedTextZH_Actions[] = {
	{ FIXEDTEXT_ENTRY(fixedTextZH_ActionOpen) },
	{ FIXEDTEXT_ENTRY(fixedTextZH_ActionClose) },
	{ FIXEDTEXT_ENTRY(fixedTextZH_ActionMove) },
	{ FIXEDTEXT_ENTRY(fixedTextZH_ActionPick) },
	{ FIXEDTEXT_ENTRY(fixedTextZH_ActionUse) }
};

// =========================================

static const FixedTextLanguageEntry fixedTextLanguages[] = {
	{ Common::DE_DEU,   fixedTextDE, fixedTextDE_Actions },
	{ Common::ES_ESP,   fixedTextES, fixedTextES_Actions },
	{ Common::EN_ANY,   fixedTextEN, fixedTextEN_Actions },
	{ Common::ZH_TWN,   fixedTextZH, fixedTextZH_Actions },
	{ Common::UNK_LANG, fixedTextEN, fixedTextEN_Actions }
};

// =========================================

// =========================================

ScalpelFixedText::ScalpelFixedText(SherlockEngine *vm) : FixedText(vm) {
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

const char *ScalpelFixedText::getText(int fixedTextId) {
	return _curLanguageEntry->fixedTextArray[fixedTextId];
}

const Common::String ScalpelFixedText::getActionMessage(FixedTextActionId actionId, int messageIndex) {
	assert(actionId >= 0);
	assert(messageIndex >= 0);
	const FixedTextActionEntry *curActionEntry = &_curLanguageEntry->actionArray[actionId];

	assert(messageIndex < curActionEntry->fixedTextArrayCount);
	return Common::String(curActionEntry->fixedTextArray[messageIndex]);
}

} // End of namespace Scalpel

} // End of namespace Sherlock
