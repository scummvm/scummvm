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

#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

namespace Tattoo {

static const char *const FIXED_TEXT_ENGLISH[] = {
	"Money",
	"Card",
	"Tobacco",
	"Timetable",
	"Summons",
	"Foolscap",
	"Damp Paper",
	"Bull's Eye",

	"Money",
	"Card",
	"Tobacco",
	"Timetable",
	"Summons",
	"Foolscap",
	"Foolscap",
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

	"Holmes",
	"Jock",
	"Bull",
	"Round",
	"Turn Total",
	"Dart",
	"to start",
	"Hit a key",
	"Press a key",
	"bullseye",
	"GAME OVER",
	"BUSTED",
	"Wins",
	"Scored",
	"points",
	"Hit",
	"Double",
	"Triple",

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
	"Going East"
};

static const char *const FIXED_TEXT_GERMAN[] = {
	"Geld",
	"S. Holmes",
	"Tabak",
	"Plan",
	"Aufforderg.",
	"Blatt pap.",
	"Dunstig pap",
	"Handlampe",

	"Geld",
	"S. Holmes",
	"Tabak",
	"Plan",
	"Aufforderg.",
	"Pergament",
	"Dunstig pap",
	"Handlampe",

	"ffne",
	"Schau",
	"Rede",
	"Benutze",
	"Journal",
	"Inventory",
	"Options",
	"Losen",
	"mit",
	"Keine Wirkung...",
	"Diese Person weic im Augenblick nichts zu berichten.",

	"Seite %d",
	"Schliecen",
	"Lessen",
	"In Datei sichern",
	"Suche abbrechen",
	"Rbckwarts suchen ",
	"Vorwarts suchen ",
	"Text nicht gefunden",

	"Holmes",
	"Jock",
	"Bull",
	"Runde",
	"Gesamt",
	"Pfeil",
	"zum Starten",
	"Taste dracken",
	"Taste dracken",
	"Bullseye",
	"SPIEL BEENDET",
	"VERLOREN",
	"Gewinnt",
	"Erzielte",
	"Punkte",
	"Treffer",
	"Doppel",
	"Dreifach",

	"Benutze",
	"Wasser",
	"Erhitze",
	"Spiel laden",
	"Spiel sichern",
	"Musik",
	"Soundeffekte",
	"Voices",
	"Textfenster",
	"Transparente Menbs",
	"Schriftart andern",
	"Aus",
	"An",
	"Ende",
	"Spiel beenden? ",
	"Sind Sie sicher ?",
	"Ja",
	"Nein",
	"Pacwort eingeben",
	"Going East"
};

TattooFixedText::TattooFixedText(SherlockEngine *vm) : FixedText(vm) {
	if (vm->getLanguage() == Common::DE_DEU)
		_fixedText = FIXED_TEXT_GERMAN;
	else
		_fixedText = FIXED_TEXT_ENGLISH;
}

const char *TattooFixedText::getText(int fixedTextId) {
	return _fixedText[fixedTextId];
}

const Common::String TattooFixedText::getActionMessage(FixedTextActionId actionId, int messageIndex) {
	return Common::String();
}


} // End of namespace Tattoo

} // End of namespace Sherlock
