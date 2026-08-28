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

#include "hollywood/game_strings.h"

namespace Hollywood {

const HollywoodGameStrings kSpanishGameStrings = {
	{ " a", "Ir a", "Hablar con", "Coger", "Mirar", "Usar", "Abrir", "Cerrar", "Dar" },
	{ "", "", "Hablar", "Coger", "Mirar", "Usar", "Abrir", "Cerrar", "Dar" },
	" a ",
	" con ",
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
	" Cambiar bebida",
	" Abrir grifo",
	" Dejar de hacer cocteles"
};

const HollywoodGameStrings kItalianGameStrings = {
	{ " a", "Andare a", "Parlare con", "Prendere", "Guardare", "Usare", "Aprire", "Chiudere", "Dare" },
	{ "", "", "Parlare", "Prendere", "Guardare", "Usare", "Aprire", "Chiudere", "Dare" },
	" a ",
	" con ",
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
	" cassa da morto larga", // The executable's wide variant mistakenly remains Spanish.
	" Cambiare bevanda",
	" Aprire rubinetto",
	" Smettere di fare cocktail"
};

const HollywoodGameStrings &getGameStrings(Common::Language language) {
	if (language == Common::IT_ITA)
		return kItalianGameStrings;

	return kSpanishGameStrings;
}

} // End of namespace Hollywood
