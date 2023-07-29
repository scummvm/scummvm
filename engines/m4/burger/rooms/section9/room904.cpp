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
 * along with this program.  If not, see <http://www.gnu.org/licenses/ },.
 *
 */

#include "m4/burger/rooms/section9/room904.h"
#include "m4/burger/vars.h"
#include "m4/m4.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *const CREDITS[] = {
	"Entwurf",
	"Spielentwurf:  Robert Aitken",
	" ",
	"Spieltext:  Robert Aitken",
	" ",
	"Zus\xe4""tzliche Dialoge:  Mark Solomon",
	" ",
	"Originalidee:",
	"                   Robert Aitken",
	"                   Matthew Powell",
	"Graphik",
	"K\xfc""nstlerische Leitung:  Andrew Pratt",
	" ",
	"Entwurf der Charaktere:",
	"                   Glen Lovett",
	"                   Sean Newton",
	" ",
	"Hintergrund-Layout:  Glen Lovett",
	" ",
	"Landschaftsgestaltung:",
	"                 Bob Parr",
	"                 Andrew Pratt",
	" ",
	"Graphische Technik:  Garth Buzzard",
	" ",
	"Zus\xe4""tzliche Grafik:",
	"                 Darren Brereton",
	"                 Shelley Burke",
	"                 Garth Buzzard",
	"                 Karl Johanson",
	"                 Calvin Jones",
	"                 Dwight Lockhart",
	"                 Bob Parr",
	"                 Andrew Pratt",
	" ",
	"Benutzeroberfl\xe4""che, Grafikdetails:  Andrew Pratt",
	nullptr,
	"Animationen",
	"Animationsleitung:  Sean Newton",
	" ",
	"Animationen:",
	"            Jim Bridle",
	"            Hilary Denny",
	"            Nelson Dewey",
	"            Richelle Grist",
	"            Glen Lovett",
	"            Kay Lovett",
	"            Tim Maltby",
	"            Stephen McCallum",
	"            Shelley McIntosh",
	"            Jean Paynter",
	"            Gary Scott",
	"            Nigel Tully",
	"            Adrie Van Viersen",
	" ",
	"Animationsdesign:  Robert Aitken",
	" ",
	"3D-Modellierung, Animationen:",
	"            Nelson Dewey",
	"            David Henry",
	" ",
	"Animationstechnik:  Robert McMurtry",
	nullptr,
	"Kolorierung",
	"Kolorierung:",
	"            Allan Cameron",
	"            Scott Douglas",
	"            Cindy Johnson",
	" ",
	"Zus\xe4""tzliche Kolorierung:",
	"            Brent Arnst",
	"            Rob Barrett",
	"            Garth Buzzard",
	"            Chris Capell",
	"            Jacob Dewey",
	"            Bruce Elder",
	"            Andrew James",
	"            Darren Keetley",
	"            Cal Jones",
	"            Jeff Kuipers",
	"            Dwight Lockhart",
	"            Kimberlie Manuel",
	"            Connie McCann",
	"            Stephen Rowe",
	"            Peter Taylor",
	nullptr,
	"Audio",
	"Originalmusik:  Darren McGrath",
	" ",
	"Soundgestaltung:  Darren McGrath",
	" ",
	"Audio-Produktion:",
	"          Darren McGrath",
	"          Ton Synchron",
	" ",
	"Casting:",
	"          Robert Aitken",
	"          Darren McGrath",
	"          Kris Zimmerman",
	" ",
	"Dialogschnitt:",
	"          Darren McGrath",
	"          Allan Cameron",
	"          Ton Synchron",
	nullptr,
	"Programmierung",
	"Leitung Spielprogrammierung:  Matthew Powell",
	" ",
	"Programmierung:",
	"       Robert McMurtry",
	"       Paul Lahaise",
	"       Chris Petkau",
	"       Jason Bevins",
	"       David Thomas",
	" ",
	"Leitung Systemprogrammierung:  Nick Porcino",
	" ",
	"Systemprogrammierung:",
	"       Andras Kovacs",
	"       Michael Ellis",
	"       Xi Huang",
	"       Tinman",
	"       Paul Lahaise",
	"       Matthew Powell",
	"       Cody Jones",
	" ",
	"International:  Mojo Systems",
	" ",
	"Konversation:  Robert Aitken",
	nullptr,
	"Produktion"
	"Produktionsleitung:"
	"            Fran\xe7""ois Robillard,",
	"            Eidos Deutschland",
	" ",
	"Produktionsassistenz:  Danielle Cooper",
	" ",
	"Deutsche Produktion:",
	"            Alp Altun",
	"            Eva Hoogh",
	"            Anskje Kirschner",
	"            Christian Zoch",
	" ",
	"International:",
	"            Kimberlie Manuel, Mojo Systems",
	"            Katrin van der Leeden",
	nullptr,
	"Qualit\xe4""tssicherung",
	"Michael Beninger",
	"Kimberlie Manuel",
	"Chris Capell",
	"Bruce Elder",
	"Darren Keetley",
	" ",
	"International: Mojo Systems",
	nullptr,
	"Handbuch",
	"Handbuch: Hope Hickli",
	" ",
	"Grafik und Layout des Handbuchs:",
	"                 Calvin Jones",
	"                 Bob Parr",
	nullptr,
	"Die Charaktere",
	"(in alphabetischer Reihenfolge)",
	"Angus  -  Peter Bieringer",
	"Astral  -  Susanne Sternberg",
	"Bauarbeiter - Alp Altun",
	"Bork  -  Dee Bradley Baker",
	"Burl  -  Nico K\xf6""nig",
	"Computer  -  Ulrike Herm",
	"Elgus  -  Peter Bieringer",
	"Erz\xe4""hler  -  Ingo Abel",
	"Eugene  -  Franz-Josef Steffens",
	"Fernsehstimmen  -  Ulrike Herm, Robert Missler",
	"   Olaf Ritter, Thomas Stein, Angela Stresemann",
	"Flumix  -  Wolfgang J\xfc""rgen",
	"Harry  -  Hans Sievers",
	"Gus  -  Peter Bieringer",
	"Odie  -  Dagmar 'The Kid'",
	"Perkins  -  Michael Quiatkowski",
	"Roxy  -  Susanne Sternberg",
	"Sheriff  -  Hans Sievers",
	"Stolie  -  Hans Sievers",
	"Tante Polly  -  Verena Wiet",
	"Unseren Dank an Lynda Gran f\xfc""r unsere Gehaltsschecks,",
	"Vera  -  Angela Stresemann",
	"Vipe  -  Alexander Draeger",
	"VP  -  Franz-Josef Steffens",
	"Wilbur  -  Robert Missler",
	"Zlarg  -  Wolfgang Draeger",
	nullptr,
	"Vielen Dank",
	"Unseren Dank an Lynda Gran f\xfc""r unsere Gehaltsschecks,",
	"an Laura McCallum f\xfc""r ihr Organisationsgenie",
	"und an Carol Schieck f\xfc""r ihr immer geduldiges L\xe4""cheln.",
	" ",
	"Vielen Dank an Vancouver Island Brewing. Jungs,",
	"ihr habt uns Wochenende um Wochenende gerettet!",
	" ",
	"Besonderen Dank an bedeutsame andere ",
	"samt Nachwuchs, die so oft auf ihre Lieben ",
	"verzichten mu\xdf""ten, damit dieses Spiel ",
	"entstehen konnte.",
	" ",
	"Ganz besonderen Dank an Anskje Kirschner!",
	nullptr,
	" "
	" ",
	nullptr,
	"Hauptmen\xfc",
	" ",
	nullptr,
	nullptr
};

void Room904::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room904::init() {
	digi_preload("902music");
	digi_preload("904pop1");
	digi_preload("904pop2");
	digi_preload("904pop3");
	digi_preload("904pop4");
	digi_preload("904pop5");

	_G(kernel).suppress_fadeup = true;
	pal_fade_set_start(_G(master_palette), 0);
	pal_fade_init(_G(master_palette), 0, 255, 100, 60, 1);
	player_set_commands_allowed(false);
}

void Room904::shutdown() {
	if (_screen1)
		TextScrn_Destroy(_screen1);
	if (_screen2)
		TextScrn_Destroy(_screen2);

	_G(player).command_ready = true;
}

void Room904::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		player_set_commands_allowed(true);
		creditsSetup();
		break;

	case 2:
		// TODO

	default:
		break;
	}
}

void Room904::creditsSetup() {
	_numSections = getCreditsSectionsCount();

	gr_font_set(_G(font_inter));
	_fontHeight = gr_font_get_height();

	// TODO
}

size_t Room904::getCreditsSectionsCount() const {
	size_t numSections = 0;

	for (auto line = CREDITS; *line; ++line) {
		while (*line)
			++line;
		++numSections;
	}

	return numSections;
}

size_t Room904::getMaxCreditsWidth() const {
	for (int sectionNum = 1; sectionNum <= _numSections; ++sectionNum) {
		// TODO
	}

	return 0;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
