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

#include "common/config-manager.h"

#include "hollywood/monsters/game_strings.h"

namespace Hollywood {

const HollywoodGameStrings kSpanishGameStrings = {
	{ " a", "Ir a", "Hablar con", "Coger", "Mirar", "Usar", "Abrir", "Cerrar", "Dar" },
	{ "", "", "Hablar", "Coger", "Mirar", "Usar", "Abrir", "Cerrar", "Dar" },
	" a",
	" con",
	"Inventario",

	"Salvar",
	"Cargar",
	"Salir",
	"Jugar",
	"Musica ON",
	"Musica OFF",
	"SFX ON",
	"SFX OFF",
	"Test ON",
	"Test OFF",
	"Volumen musica",
	"Volumen SFX",
	"Volumen voz",
	"Velocidad texto",
	"Texto",
	"Voz",
	"Ambos",
	"\xa8" "Est\xa0s seguro?",
	"S",
	"N",

	" pasadizo",
	" ata\xa3" "d ancho",
	" discos",
	" Cambiar bebida",
	" Abrir grifo",
	" Dejar de hacer cocteles",
	{ "T\xa3nel deteriorado", "Campamento", "Minero", "Vagoneta volcada",
		"Grieta", "Caseta", "Escaleras", "Cascada", "Ascensor" }
};

const HollywoodGameStrings kItalianGameStrings = {
	{ " a", "Andare a", "Parlare con", "Prendere", "Guardare", "Usare", "Aprire", "Chiudere", "Dare" },
	{ "", "", "Parlare", "Prendere", "Guardare", "Usare", "Aprire", "Chiudere", "Dare" },
	" a",
	" con",
	"Inventario",

	"Salvare",
	"Caricare",
	"Uscire",
	"Giocare",
	"Musica ON",
	"Musica OFF",
	"SFX ON",
	"SFX OFF",
	"Test ON",
	"Test OFF",
	"Volume musica",
	"Volume SFX",
	"Volume voce",
	"Velocit\x85 testo",
	"Testo",
	"Voce",
	"Entrambe",
	"Sei sicuro?",
	"S",
	"N",

	" passaggio",
	" cassa da morto larga", // WORKAROUND: The Italian executable leaves this variant in Spanish.
	" dischi",
	" Cambiare bevanda",
	" Aprire rubinetto",
	" Smettere di fare cocktail",
	{ "Galleria dissestata", "Accampamento", "Minatore", "Vagoncino rovesciato",
		"Crepa", "Capanno", "Scale", "Cascata", "Ascensore" }
};

const HollywoodGameStrings kEnglishGameStrings = {
	{ " to", "Walk to", "Talk with", "Take", "Look at", "Use", "Open", "Close", "Give" },
	{ "", "", "Talk", "Take", "Look", "Use", "Open", "Close", "Give" },
	" to",
	" with",
	"Inventory",

	"Save",
	"Load",
	"Quit",
	"Play",
	"Music ON",
	"Music OFF",
	"SFX ON",
	"SFX OFF",
	"Test ON",
	"Test OFF",
	"Music volume",
	"SFX volume",
	"Speech volume",
	"Text speed",
	"Text",
	"Speech",
	"Both",
	"Are you sure?",
	"Y",
	"N",

	" passage",
	" wide coffin",
	" records",
	" Change drink",
	" Open tap",
	" Stop making cocktails",
	{ "Damaged tunnel", "Camp", "Miner", "Overturned cart",
		"Crack", "Hut", "Stairs", "Waterfall", "Elevator" }
};

const HollywoodGameStrings &getGameStrings(Common::Language language) {
	if (ConfMan.hasKey("language")) {
		const Common::Language selectedLanguage = Common::parseLanguage(ConfMan.get("language"));
		if (selectedLanguage == Common::EN_ANY || selectedLanguage == Common::EN_GRB ||
				selectedLanguage == Common::EN_USA)
			language = selectedLanguage;
	}

	switch (language) {
	case Common::EN_ANY:
	case Common::EN_GRB:
	case Common::EN_USA:
		return kEnglishGameStrings;
	case Common::IT_ITA:
		return kItalianGameStrings;
	default:
		return kSpanishGameStrings;
	}
}

} // End of namespace Hollywood
