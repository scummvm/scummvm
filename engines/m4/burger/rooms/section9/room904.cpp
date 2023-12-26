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

static const char *const CREDITS_ENG[] = {
	"Design",
	"Game Designer:  Robert Aitken",
	" ",
	"Writer:  Robert Aitken",
	" ",
	"Additional Dialog:  Mark Solomon",
	" ",
	"Original Concept:",
	"                   Robert Aitken",
	"                   Matthew Powell",
	nullptr,
	"Art",
	"Art Director:  Andrew Pratt",
	" ",
	"Character Design:",
	"                   Glen Lovett",
	"                   Sean Newton",
	" ",
	"Background Layout:  Glen Lovett",
	" ",
	"Background Art:",
	"                 Bob Parr",
	"                 Andrew Pratt",
	" ",
	"Art Technician:  Garth Buzzard",
	" ",
	"Additional Art:",
	"                 Darren Brereton",
	"                 Shelley Burke",
	"                 Garth Buzzard",
	"                 Karl Johanson",
	"                 Calvin Jones",
	"                 Dwight Lockhart",
	"                 Bob Parr",
	"                 Andrew Pratt",
	" ",
	"GUI and Detail Art:  Andrew Pratt",
	nullptr,
	"Animation",
	"Lead Animator:  Sean Newton",
	" ",
	"Animators:",
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
	"Animation Design:  Robert Aitken",
	" ",
	"3D Modeling/Animation:",
	"            Nelson Dewey",
	"            David Henry",
	" ",
	"Animation Technician:  Robert McMurtry",
	nullptr,
	"Coloring",
	"Colorists:",
	"            Allan Cameron",
	"            Scott Douglas",
	"            Cindy Johnson",
	" ",
	"Additional Coloring:",
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
	"Original Score:  Darren McGrath",
	" ",
	"Sound Design:  Darren McGrath",
	" ",
	"Audio Producer:  Darren McGrath",
	" ",
	"Casting:",
	"          Robert Aitken",
	"          Darren McGrath",
	"          Kris Zimmerman",
	" ",
	"Dialog Cutting:",
	"          Darren McGrath",
	"          Allan Cameron",
	nullptr,
	"Programming",
	"Game Lead:  Matthew Powell",
	" ",
	"Game:",
	"       Robert McMurtry",
	"       Paul Lahaise",
	"       Chris Petkau",
	"       Jason Bevins",
	"       David Thomas",
	" ",
	"Systems Lead:  Nick Porcino",
	" ",
	"Systems:",
	"       Andras Kovacs",
	"       Michael Ellis",
	"       Xi Huang",
	"       Tinman",
	"       Paul Lahaise",
	"       Matthew Powell",
	"       Cody Jones",
	" ",
	"Windows '95:  Chris Blackwell",
	" ",
	"Macintosh:",
	"       Nick Porcino",
	"       Michael Ellis",
	" ",
	"Conversations:  Robert Aitken",
	nullptr,
	"Production",
	"Producer:  Fran\xe7""ois Robillard",
	" ",
	"Assistant Producer:  Danielle Cooper",
	nullptr,
	"Quality Assurance",
	"Michael Beninger",
	"Kimberlie Manuel",
	"Chris Capell",
	"Bruce Elder",
	"Darren Keetley",
	nullptr,
	"Documentation",
	"User's Guide Writer: Hope Hickli",
	" ",
	"Art and Layout:",
	"                 Calvin Jones",
	"                 Bob Parr",
	nullptr,
	"Audio Cast",
	"Wilbur  -  Rob Paulsen",
	"Zlarg  -  Jim Cummings",
	"Flumix  -  Dee Bradley Baker",
	"Astral  -  E.G. Daily",
	"Harry  -  Michael Bell",
	"Aunt Polly  -  Candi Milo",
	"Burl  -  Phil Hayes",
	"Elmo Perkins  -  Nick Jameson",
	"Sheriff  -  Phil Hayes",
	"Gus/Angus/Elgus  -  Mike Donovan",
	"Odie  -  Rob Paulsen",
	"Vipe  -  Mike Donovan",
	"Roxy  -  Candi Milo",
	"Vera  -  Pat Lentz",
	"Eugene  -  Phil Hayes",
	"Stolie  -  Mike Donovan",
	"Borks  -  Dee Bradley Baker",
	"VP of Protein Procurement  -  Jim Cummings",
	"Truffles  -  Dee Bradley Baker",
	"Junkyard Dog  -  Rob Paulsen",
	"Ship's Computer  -  Candi Milo",
	"Mutant Gerbils  -  Dee Bradley Baker",
	nullptr,
	"Special Thanks",
	"Thanks to Lynda Gran for our paychecks,",
	"Laura McCallum for keeping us organized",
	"and Carol Schieck for always smiling.",
	" ",
	"Thanks to Vancouver Island Brewing for",
	"keeping us happy every weekend.",
	" ",
	"Very Special Thanks to Significant Others",
	"and Offspring who often made do without",
	"their loved ones so that this product could",
	"be made.",
	nullptr,
	" ",
	" ",
	nullptr,
	"Back To Main Menu",
	" ",
	nullptr,
	nullptr
};

static const char *const CREDITS_DEU[] = {
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
	nullptr,
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
	"Produktion",
	"Produktionsleitung:",
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
	"Fernsehstimmen  -  Ulrike Herm, Robert Missler,",
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

#define CREDITS (g_engine->getLanguage() == Common::DE_DEU ? CREDITS_DEU : CREDITS_ENG)

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
		if (_currentSection < _numSections) {
			playRandomSound(2, 1);
			TextScrn_Add_TextItem(_screen1, 10, (_currentSection - 1) * _fontHeight + 10,
				_currentSection, TS_CENTRE, getCreditsSectionString(_currentSection),
				(M4CALLBACK)creditsCallback);
			TextScrn_Activate(_screen1);
		}
		break;

	case 3:
		playRandomSound(-1, 2);
		break;

	case 4:
		digi_play_loop("902music", 3, 155, -1);
		break;

	case 5:
		player_set_commands_allowed(false);
		pal_fade_init(_G(master_palette), 0, 255, 0, 30, 6);
		break;

	case 6:
		_G(game).setRoom(_G(executing) == WHOLE_GAME ? 903 : 901);
		break;

	default:
		break;
	}
}

void Room904::parser() {
	if (player_said("go back"))
		kernel_trigger_dispatch_now(5);
}

void Room904::creditsSetup() {
	_numSections = getCreditsSectionsCount();

	gr_font_set(_G(font_inter));
	_fontHeight = gr_font_get_height();
	_totalWidth = getMaxCreditsHeaderWidth() + 20;
	_totalHeight = _fontHeight * _numSections + 20;

	_x1 = 30;
	_x2 = 30 + _totalWidth;
	_y1 = (480 - _totalHeight) / 2;
	_y2 = _y1 + _totalHeight;

	_screen1 = TextScrn_Create(_x1, _y1, _x2, _y2, 100, 422, 3, 22, 1, 10, 2, 14);

	for (_currentSection = 1; _currentSection <= _numSections; ++_currentSection) {
		TextScrn_Add_TextItem(_screen1, 10, (_currentSection - 1) * _fontHeight + 10,
			_currentSection, TS_CENTRE, getCreditsSectionString(_currentSection),
			(M4CALLBACK)creditsCallback);
	}

	TextScrn_Activate(_screen1);
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

int Room904::getCreditsSectionLine(int sectionNum) const {
	if (sectionNum < 1 || sectionNum > _numSections)
		error_show(FL, 'Burg', "Bad index to credits");

	int lineNum;
	for (lineNum = 0; sectionNum > 1; --sectionNum, ++lineNum) {
		while (CREDITS[lineNum])
			++lineNum;
	}

	return lineNum;
}

const char *Room904::getCreditsSectionString(int sectionNum) const {
	return CREDITS[getCreditsSectionLine(sectionNum)];
}

int Room904::getCreditsSectionLines(int sectionNum) const {
	int sectionStart = getCreditsSectionLine(sectionNum);
	int lineNum = sectionStart;

	while (CREDITS[lineNum])
		++lineNum;

	return lineNum - sectionStart;
}

size_t Room904::getMaxCreditsHeaderWidth() const {
	int32 maxWidth = 0;

	for (int sectionNum = 1; sectionNum <= _numSections; ++sectionNum) {
		const char *tmp = getCreditsSectionString(sectionNum);
		int w = gr_font_string_width(tmp);
		debug("%d", w);

		maxWidth = MAX(maxWidth, gr_font_string_width(getCreditsSectionString(sectionNum)));
	}

	return maxWidth;
}

size_t Room904::getCreditsSectionWidth(int sectionNum) const {
	int32 maxWidth = 0;

	for (int lineNum = getCreditsSectionLine(sectionNum); CREDITS[lineNum]; ++lineNum) {
		maxWidth = MAX(maxWidth, gr_font_string_width(CREDITS[lineNum]));
	}

	return maxWidth;
}

const char *Room904::getLineInCreditsSection(int sectionNum, int lineNum) const {
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

	if (strncmp(credit, "Haupt", 5) && strncmp(credit, "Back ", 5)) {
		mouse_set_sprite(kArrowCursor);
		gr_font_set(_G(font_conv));
		_fontHeight = gr_font_get_height();

		int sectionWidth = getCreditsSectionWidth(sectionNum) + 20;
		int sectionHeight = linesCount * _fontHeight + 20;

		int x1 = (640 - _x2 - sectionWidth) / 2 + _x2;
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
