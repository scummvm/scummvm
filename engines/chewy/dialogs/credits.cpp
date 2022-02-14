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

#include "chewy/dialogs/credits.h"
#include "chewy/events.h"
#include "chewy/file.h"
#include "chewy/global.h"

namespace Chewy {
namespace Dialogs {

static const int16 CREDITS_POS[65][2] = {
	{40,  160}, {80,  170}, {40,  190}, {80,  200}, {80,  210},
	{40,  230}, {80,  240}, {80,  250}, {80,  260}, {40,  280},
	{80,  290}, {80,  300}, {80,  310}, {80,  320}, {40,  340},
	{80,  350}, {80,  360}, {40,  380}, {80,  390}, {40,  410},
	{80,  420}, {40,  440}, {80,  450}, {40,  470}, {80,  480},
	{80,  490}, {80,  500}, {80,  510}, {80,  520}, {40,  540},
	{80,  550}, {80,  560}, {80,  570}, {80,  580}, {40,  600},
	{80,  610}, {80,  620}, {80,  630}, {80,  640}, {80,  650},
	{80,  660}, {80,  670}, {80,  680}, {80,  690}, {80,  700},
	{80,  710}, {80,  720}, {80,  730}, {80,  740}, {80,  750},
	{80,  760}, {40,  780}, {40,  790}, {40,  800}, {40,  810},
	{40,  820}, {40,  840}, {80,  850}, {40,  870}, {80,  880},
	{80,  890}, {80,  910}, {80,  920}, {80,  930}, {80,  940}
};

static const bool CREDITS_TYPE[65] = {
	true, false, true, false, false, true, false, false, false, true,
	false, false, false, false, true, false, false, true, false, true,
	false, true, false, true, false, false, false, false, false, true,
	false, false, false, false, true, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false,
	false, true, true, true, true, true, true, false, true, false,
	false, true, true, true, true
};

static const char *CREDITS_TEXT[65] = {
	"Idea & Story:",
	"Carsten Wieland",
	"Programming:",
	"Helmut Theuerkauf",
	"Alexander Diessner",
	"Graphics & Animation:",
	"Carsten Wieland",
	"Nihat Keesen",
	"Stefan Frank",
	"Text & Dialogues:",
	"Helmut Theuerkauf",
	"Alexander Diessner",
	"Carsten Wieland",
	"Wolfgang Walk",
	"Music:",
	"Carsten Wieland",
	"Stelter Studios",
	"Sound FX:",
	"Helmut Theuerkauf",
	"Producer & Lecturer:",
	"Wolfgang Walk",
	"Minister of financial affairs:",
	"Carsten (Dagobert) Korte",
	"Testers:",
	"Lutz Rafflenbeul",
	"Thomas Friedmann",
	"Bernhard Ewers",
	"Christian von der Hotline",
	"Carsten Korte",
	"The voices:",
	"Chewy......Renier Baaken",
	"Howard.....Wolfgang Walk",
	"Nichelle...Indhira Mohammed",
	"Clint......Alexander Schottky",
	"Also cast:",
	"Renier Baaken",
	"Guido B\x94sherz",
	"Gerhard Fehn",
	"Alice Krause",
	"Reinhard Lie\xE1",
	"Willi Meyer",
	"Nicole Meister",
	"Lutz Rafflenbeul",
	"Alexander Schottky",
	"Bernd Schulze",
	"Susanne Simenec",
	"Helmut Theuerkauf",
	"Andreas Vogelpoth",
	"Mark Wagener",
	"Wolfgang Walk",
	"Thomas Piet Wiesenm\x81ller",
	"Speech recorded by",
	"tmp Studio, Moers by Willi Meyer",
	"Cut by Hartmut Stelter",
	"Studios Hamburg and",
	"Carsten Wieland",
	"Soundsystem:",
	"AIL (c) Miles Design",
	"Adventure Engine:",
	"I.C.M. developed by",
	"New Generation Software",
	"Song Boo Boo Ba Baby composed",
	"by Haiko Ruttmann,",
	"Lyrics Wolfgang Walk,",
	"featuring Indhira Mohammed."
};

void Credits::execute() {
	int lineScrolled = 0;
	int fontCol;

	room->open_handle(GBOOK, "rb", 0);
	room->load_tgp(5, &room_blk, 1, 0, GBOOK);
	_G(spieler).scrollx = 0;
	_G(spieler).scrolly = 0;
	_G(out)->setze_zeiger(_G(screen0));
	room->set_ak_pal(&room_blk);
	fx->blende1(_G(workptr), _G(screen0), _G(pal), 150, 0, 0);

	for (int i = 0; i < 6; ++i) {
		int color = 63 - (6 * i);

		_G(out)->raster_col(6 - i, color, 0, 0);
		_G(out)->raster_col(7 + i, color, 0, 0);

		color = 63 - (4 * i);
		_G(out)->raster_col(37 - i, color, color, color);
		_G(out)->raster_col(38 + i, color, color, color);
	}

	_G(spieler).DelaySpeed = 2;

	for (;;) {
		if (_G(in)->get_switch_code() == Common::KEYCODE_ESCAPE || SHOULD_QUIT)
			break;

		// Display the starfield background
		_G(out)->setze_zeiger(_G(workptr));
		_G(out)->map_spr2screen(_G(ablage)[room_blk.AkAblage],
			_G(spieler).scrollx, _G(spieler).scrolly);

		// Animate moving the background
		if (++_G(spieler).scrollx >= 320)
			_G(spieler).scrollx = 0;

		if (_G(in)->get_switch_code() == Common::KEYCODE_ESCAPE)
			break;

		++lineScrolled;
		g_events->delay(50);
		bool stillScrolling = false;

		for (int i = 0; i < 65; ++i) {
			int destY = CREDITS_POS[i][1] - lineScrolled;
			if (destY >= 160 || destY <= 40)
				continue;

			if (CREDITS_TYPE[i]) {
				fontCol = 32;
				_G(out)->set_fontadr(_G(font6x8));
				_G(out)->set_vorschub(_G(fvorx6x8), _G(fvory6x8));
			} else {
				fontCol = 1;
				_G(out)->set_fontadr(_G(font8x8));
				_G(out)->set_vorschub(_G(fvorx8x8), _G(fvory8x8));
			}
			stillScrolling = true;

			int fgCol = fontCol + (160 - destY) / 10;
			_G(out)->printxy(CREDITS_POS[i][0], destY, fgCol, 300, _G(scr_width), CREDITS_TEXT[i]);
		}

		if (!stillScrolling)
			break;

		_G(out)->setze_zeiger(nullptr);
		_G(out)->back2screen(_G(workpage));
	}

	_G(out)->set_fontadr(_G(font8x8));
	_G(out)->set_vorschub(_G(fvorx8x8), _G(fvorx8x8));
	room->open_handle("back/episode1.tgp", "rb", 0);

	room->set_ak_pal(&room_blk);
	hide_cur();
	uhr->reset_timer(0, 5);
}

} // namespace Dialogs
} // namespace Chewy
