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

#include "sherlock/sherlock.h"
#include "sherlock/fixed_text.h"

namespace Sherlock {

static const Common::String fixedTextEN[] = {
	// Window buttons
	"Exit",
	"Up",
	"Down",
	// Inventory buttons
	"Exit",
	"Look",
	"Use",
	"Give",
	// Journal text
	"Watson's Journal",
	"Page %d",
	// Journal buttons
	"Exit",
	"Back 10",
	"Up",
	"Down",
	"Ahead 10",
	"Search",
	"First Page",
	"Last Page",
	"Print Text",
	// Journal search
	"Exit",
	"Backward",
	"Forward",
	"Text Not Found !"
};

// sharp-s       : 0xE1 / octal 341
// small a-umlaut: 0x84 / octal 204
// small o-umlaut: 0x94 / octal 224
// small u-umlaut: 0x81 / octal 201
static const Common::String fixedTextDE[] = {
	// Window buttons
	"Zur\201ck",
	"Hoch",
	"Runter",
	// Inventory buttons
	"Zur\201ck",
	"Schau",
	"Benutze",
	"Gib",
	// Journal text
	"Watsons Tagebuch",
	"Seite %d",
	// Journal buttons
	"Zur\201ck",
	"10 hoch",
	"Hoch",
	"Runter",
	"10 runter",
	"Suche",
	"Erste Seite",
	"Letzte Seite",
	"Drucke Text",
	// Journal search
	"Zur\201ck",
	"R\201ckw\204rts", // original: "Backward"
	"Vorw\204rts", // original: "Forward"
	"Text nicht gefunden!"
};

// up-side down exclamation mark - 0xAD / octal 255
// up-side down question mark - 0xA8 / octal 250
static const Common::String fixedTextES[] = {
	// Window buttons
	"Exit",
	"Subir",
	"Bajar",
	// Inventory buttons
	"Exit",
	"Mirar",
	"Usar",
	"Dar",
	// Journal text
	"Diario de Watson",
	"Pagina %d",
	// Journal buttons
	"Exit",
	"Retroceder",
	"Subir",
	"baJar",
	"Adelante",
	"Buscar",
	"1a pagina",
	"Ult pagina",
	"Imprimir",
	// Journal search
	"Exit",
	"Retroceder",
	"Avanzar",
	"Texto no encontrado!"
};

// =========================================

static const Common::String fixedTextEN_ActionOpen[] = {
	"This cannot be opened",
	"It is already open",
	"It is locked",
	"Wait for Watson",
	" ",
	"."
};

static const Common::String fixedTextDE_ActionOpen[] = {
	"Das kann man nicht \224ffnen",
	"Ist doch schon offen!",
	"Leider verschlossen",
	"Warte auf Watson",
	" ",
	"."
};

static const Common::String fixedTextES_ActionOpen[] = {
	"No puede ser abierto",
	"Ya esta abierto",
	"Esta cerrado",
	"Espera a Watson",
	" ",
	"."
};

static const Common::String fixedTextEN_ActionClose[] = {
	"This cannot be closed",
	"It is already closed",
	"The safe door is in the way"
};

static const Common::String fixedTextDE_ActionClose[] = {
	"Das kann man nicht schlie\341en",
	"Ist doch schon zu!",
	"Die safet\201r ist Weg"
};

static const Common::String fixedTextES_ActionClose[] = {
	"No puede ser cerrado",
	"Ya esta cerrado",
	"La puerta de seguridad esta entre medias"
};

static const Common::String fixedTextEN_ActionMove[] = {
	"This cannot be moved",
	"It is bolted to the floor",
	"It is too heavy",
	"The other crate is in the way"
};


static const Common::String fixedTextDE_ActionMove[] = {
	"L\204\341t sich nicht bewegen",
	"Festged\201belt in der Erde...",
	"Oha, VIEL zu schwer",
	"Der andere Kiste ist im Weg" // [sic]
};

static const Common::String fixedTextES_ActionMove[] = {
	"No puede moverse",
	"Esta sujeto a la pared",
	"Es demasiado pesado",
	"El otro cajon esta en mitad"
};

static const Common::String fixedTextEN_ActionPick[] = {
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

static const Common::String fixedTextDE_ActionPick[] = {
	"Nichts Interessantes da",
	"Zu gut befestigt",
	"Ist ja wohl ein bi\341chen zu gro\341, oder ?",
	"Oha, VIEL zu schwer",
	"Ich denke, Du stehst mehr auf M\204dchen ?",
	"Diese Blumen geh\224ren Penny",
	"Sie ist doch viel zu jung f\201r Dich!",
	"Ich denke, Du stehst mehr auf M\204dchen ?",
	"Staatseigentum - Nur für den Dienstgebrauch !"
};

static const Common::String fixedTextES_ActionPick[] = {
	"No hay nada interesante",
	"Esta anclado al suelo",
	"Es muy grande para llevarlo",
	"Pesa demasiado",
	"Creo que una chica sera mas tu tipo",
	"Esas flores pertenecen a Penny",
	"\255Es demasiado joven para ti!"
	"\255Creo que una chica sera mas tu tipo!",
	"Propiedad del gobierno para uso oficial"
};

static const Common::String fixedTextEN_ActionUse[] = {
	"You can't do that",
	"It had no effect",
	"You can't reach it",
	"OK, the door looks bigger! Happy?"
	"Doors don't smoke"
};

static const Common::String fixedTextDE_ActionUse[] = {
	"Nein, das geht wirklich nicht",
	"Tja keinerlei Wirkung",
	"Da kommst du nicht dran",
	"Na gut, die Tür sieht jetzt gr\224\341er aus. Zufrieden?"
	"Türen sind Nichtraucher!"
};

static const Common::String fixedTextES_ActionUse[] = {
	"No puedes hacerlo",
	"No tuvo ningun efecto",
	"No puedes alcanzarlo",
	"Bien, \255es enorme! \250Feliz?"
	"Las puertas no fuman"
};

static const FixedTextActionEntry fixedTextEN_Actions[] = {
	{ fixedTextEN_ActionOpen },
	{ fixedTextEN_ActionClose },
	{ fixedTextEN_ActionMove },
	{ fixedTextEN_ActionPick },
	{ fixedTextEN_ActionUse }
};

static const FixedTextActionEntry fixedTextDE_Actions[] = {
	{ fixedTextDE_ActionOpen },
	{ fixedTextDE_ActionClose },
	{ fixedTextDE_ActionMove },
	{ fixedTextDE_ActionPick },
	{ fixedTextDE_ActionUse }
};

static const FixedTextActionEntry fixedTextES_Actions[] = {
	{ fixedTextES_ActionOpen },
	{ fixedTextES_ActionClose },
	{ fixedTextES_ActionMove },
	{ fixedTextES_ActionPick },
	{ fixedTextES_ActionUse }
};

// =========================================

static const FixedTextLanguageEntry fixedTextLanguages[] = {
	{ Common::DE_DEU,   fixedTextDE, fixedTextDE_Actions },
	{ Common::ES_ESP,   fixedTextES, fixedTextES_Actions },
	{ Common::EN_ANY,   fixedTextEN, fixedTextEN_Actions },
	{ Common::UNK_LANG, fixedTextEN, fixedTextEN_Actions }
};

// =========================================

// =========================================

FixedText::FixedText(SherlockEngine *vm) : _vm(vm) {
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

const Common::String FixedText::getText(FixedTextId fixedTextId) {
	return _curLanguageEntry->fixedTextArray[fixedTextId];
}

const Common::String FixedText::getActionMessage(FixedTextActionId actionId, int messageIndex) {
	assert(actionId >= 0);
	return _curLanguageEntry->actionArray[actionId].fixedTextArray[messageIndex];
}

} // End of namespace Sherlock
