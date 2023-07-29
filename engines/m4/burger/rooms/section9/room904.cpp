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
#include "m4/core/errors.h"
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
	_totalWidth = getMaxCreditsWidth() + 20;
	_totalHeight = _fontHeight * _numSections + 20;

	_x1 = 30;
	_x2 = 30 + _totalWidth;
	_y1 = (480 - _totalHeight) / 2;
	_y2 = _y1 + _totalHeight;

	_screen1 = TextScrn_Create(_x1, _y1, _x2, _y2, 100, 422, 3, 22, 1, 10, 2, 14);

	for (_currentSection = 1; _currentSection <= _numSections; ++_currentSection) {
		TextScrn_Add_TextItem(_screen1, 10, _currentSection * _fontHeight + 10,
			_currentSection, TS_CENTRE, getCreditsSectionString(_currentSection),
			(M4CALLBACK)creditsCallback);
	}

	TextScrn_Activate(_screen1);
}

size_t Room904::getCreditsSectionsCount() {
	size_t numSections = 0;

	for (auto line = CREDITS; *line; ++line) {
		while (*line)
			++line;
		++numSections;
	}

	return numSections;
}

int Room904::getCreditsSectionLine(int sectionNum) {
	if (sectionNum >= 1 && sectionNum <= 8)
		error_show(FL, 'Burg', "Bad index to credits");

	int lineNum;
	for (lineNum = 0; sectionNum > 0; --sectionNum, ++lineNum) {
		while (CREDITS[lineNum])
			++lineNum;
	}

	return lineNum;
}

const char *Room904::getCreditsSectionString(int sectionNum) {
	return CREDITS[getCreditsSectionLine(sectionNum)];
}

int Room904::getCreditsSectionLines(int sectionNum) {
	int sectionStart = getCreditsSectionLine(sectionNum);
	int lineNum = sectionStart;

	while (*CREDITS[lineNum])
		++lineNum;

	return lineNum - sectionStart;
}

size_t Room904::getMaxCreditsWidth() {
	int32 maxWidth = 0;

	for (int sectionNum = 1; sectionNum <= _numSections; ++sectionNum) {
		maxWidth = MAX(maxWidth, gr_font_string_width(getCreditsSectionString(sectionNum)));
	}

	return maxWidth;
}

size_t Room904::getCreditsSectionWidth(int sectionNum) {
	int32 maxWidth = 0;

	for (int lineNum = getCreditsSectionLine(sectionNum); CREDITS[lineNum]; ++lineNum) {
		maxWidth = MAX(maxWidth, gr_font_string_width(CREDITS[lineNum]));
	}

	return maxWidth;
}

const char *Room904::getLineInCreditsSection(int sectionNum, int lineNum) {
	if (lineNum < 1 || lineNum > getCreditsSectionLines(sectionNum))
		error_show(FL, 'Burg', "Bad index to names");

	return CREDITS[getCreditsSectionLine(sectionNum) + lineNum];
}

void Room904::creditsCallback(TextItem *textItem, TextScrn *textScrn) {
	Room904 *room = dynamic_cast<Room904 *>(g_engine->_activeRoom);
	assert(room);
	room->updateCredits(textItem, textScrn);
}

void Room904::updateCredits(TextItem *textItem, TextScrn *textScrn) {
	const char *credit = textItem->prompt;
	int sectionNum = textItem->tag;
	int linesCount = getCreditsSectionLines(sectionNum);
	term_message("credit: %s index: %d names: %d", credit, sectionNum, linesCount);

	playRandomSound(-1, 2);

	if (strncmp(credit, "Haupt", 5)) {
		mouse_set_sprite(kArrowCursor);
		gr_font_set(_G(font_conv));
		_fontHeight = gr_font_get_height();

		int sectionWidth = getCreditsSectionWidth(sectionNum) + 20;
		int sectionHeight = linesCount * _fontHeight + 20;

		int x1 = (_x2 - sectionWidth) / 2;
		int y1 = (480 - sectionHeight) / 2;
		int x2 = x1 + sectionWidth;
		int y2 = y1 + sectionHeight;

		if (_screen2)
			TextScrn_Destroy(_screen2);
		_screen2 = TextScrn_Create(x1, y1, x2, y2, 100, 422, 3, 22, 1, 10, 2, 14);

		for (int lineNum = 1; lineNum <= linesCount; ++lineNum) {
			const char *line = getLineInCreditsSection(sectionNum, lineNum);
			TextScrn_Add_Message(_screen2, 10, (lineNum - 1) * _fontHeight + 10,
				lineNum, TS_GIVEN, line);
		}

		TextScrn_Activate(_screen2);
		TextScrn_Activate(_screen1);

	} else {
		kernel_trigger_dispatch_now(5);
	}
}

void Room904::playRandomSound(int trigger, int channel) {
	Common::String name = Common::String::format("904pop%d",
		g_engine->getRandomNumber(4) + 1);
	digi_play(name.c_str(), channel, 255, trigger);
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
