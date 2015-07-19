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

#include "sherlock/scalpel/scalpel_fixed_text.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

namespace Scalpel {

static const char *const fixedTextEN[] = {
	// SH1: Window buttons
	"Exit",
	"Up",
	"Down",
	// SH1: Inventory buttons
	"Exit",
	"Look",
	"Use",
	"Give",
	// SH1: Journal text
	"Watson's Journal",
	"Page %d",
	// SH1: Journal buttons
	"Exit",
	"Back 10",
	"Up",
	"Down",
	"Ahead 10",
	"Search",
	"First Page",
	"Last Page",
	"Print Text",
	// SH1: Journal search
	"Exit",
	"Backward",
	"Forward",
	"Text Not Found !",
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
	// SH2: Verbs
	"Open",
	"Look",
	"Talk",
	"Journal"
};

// sharp-s       : 0xE1 / octal 341
// small a-umlaut: 0x84 / octal 204
// small o-umlaut: 0x94 / octal 224
// small u-umlaut: 0x81 / octal 201
static const char *const fixedTextDE[] = {
	// SH1: Window buttons
	"Zur\201ck",
	"Hoch",
	"Runter",
	// SH1: Inventory buttons
	"Zur\201ck",
	"Schau",
	"Benutze",
	"Gib",
	// SH1: Journal text
	"Watsons Tagebuch",
	"Seite %d",
	// SH1: Journal buttons
	"Zur\201ck",
	"10 hoch",
	"Hoch",
	"Runter",
	"10 runter",
	"Suche",
	"Erste Seite",
	"Letzte Seite",
	"Drucke Text",
	// SH1: Journal search
	"Zur\201ck",
	"R\201ckw\204rts", // original: "Backward"
	"Vorw\204rts", // original: "Forward"
	"Text nicht gefunden!",
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
	"Ein Tarock-Kartenspiel", // [sic]
	"Ein verzierter Schl\201ssel",
	"Ein Pfandschein",
	// SH2: Verbs
	"\231ffne",
	"Schau",
	"Rede",
	"Tagebuch"
};

// up-side down exclamation mark - 0xAD / octal 255
// up-side down question mark - 0xA8 / octal 250
// n with a wave on top - 0xA4 / octal 244
static const char *const fixedTextES[] = {
	// SH1: Window buttons
	"Exit",
	"Subir",
	"Bajar",
	// SH1: Inventory buttons
	"Exit",
	"Mirar",
	"Usar",
	"Dar",
	// SH1: Journal text
	"Diario de Watson",
	"Pagina %d",
	// SH1: Journal buttons
	"Exit",
	"Retroceder",
	"Subir",
	"baJar",
	"Adelante",
	"Buscar",
	"1a pagina",
	"Ult pagina",
	"Imprimir",
	// SH1: Journal search
	"Exit",
	"Retroceder",
	"Avanzar",
	"Texto no encontrado!",
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
	"Der andere Kiste ist im Weg" // [sic]
};

static const char *const fixedTextES_ActionMove[] = {
	"No puede moverse",
	"Esta sujeto a la pared",
	"Es demasiado pesado",
	"El otro cajon esta en mitad"
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

// =========================================

// TODO:
// It seems there was a French version of Sherlock Holmes 2
static const FixedTextLanguageEntry fixedTextLanguages[] = {
	{ Common::DE_DEU,   fixedTextDE, fixedTextDE_Actions },
	{ Common::ES_ESP,   fixedTextES, fixedTextES_Actions },
	{ Common::EN_ANY,   fixedTextEN, fixedTextEN_Actions },
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
