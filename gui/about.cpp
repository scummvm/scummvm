/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "base/engine.h"
#include "base/version.h"
#include "gui/about.h"
#include "gui/newgui.h"
#include "gui/widget.h"

namespace GUI {

enum {
	kScrollStartDelay = 2500,
	kScrollMillisPerPixel = 80,
	
	kXOff = 3,
	kYOff = 2
};

// TODO: Probably should display something regarding GPL
// We could just list our full credits section here now...
//
// The following commands can be put at the start of a line (all subject to change):
//   \C, \L, \R  -- set center/left/right alignment
//   \c0 - \c4   -- set a custom color:
//                  0 normal text (green)
//                  1 highlighted text (light green)
//                  2 light border (light gray)
//                  3 dark border (dark gray)
//                  4 background (black)
const char *about_lines[] = {
"\\C""Copyright (C) 2002-2004 The ScummVM project",
"\\C""http://www.scummvm.org",
"\\C""",
"\\C""LucasArts SCUMM Games (C) LucasArts",
"\\C""Humongous SCUMM Games (C) Humongous",
"\\C""Simon the Sorcerer (C) Adventure Soft",
"\\C""Beneath a Steel Sky (C) Revolution",
"\\C""Broken Sword Games (C) Revolution",
"\\C""Flight of the Amazon Queen (C) John Passfield",
"\\C""and Steve Stamatiadis",
"\\C""",
"\\C\\c1""The ScummVM team:",
"\\L\\c0""  James Brown",
"\\L\\c2""    Lead developer",
"\\L\\c0""  Max Horn",
"\\L\\c2""    Lead developer",
"\\L\\c0""  Torbjorn Andersson",
"\\L\\c2""    Engine: SCUMM, Broken Sword II",
"\\L\\c0""  David Eriksson",
"\\L\\c2""    Engine: Flight of the Amazon Queen",
"\\L\\c0""  Robert Goeffringmann",
"\\L\\c2""    Engine: Beneath a Steel Sky, Broken Sword I",
"\\L\\c0""  Jonathan Gray",
"\\L\\c2""    Engine: SCUMM, Broken Sword II",
"\\L\\c0""  Travis Howell",
"\\L\\c2""    Engine: SCUMM, Simon the Sorcerer",
"\\L\\c0""  Oliver Kiehl",
"\\L\\c2""    Engine: Beneath a Steel Sky, Simon",
"\\L\\c0""  Pawel Kolodziejski",
"\\L\\c2""    Engine: SCUMM (Codecs, iMUSE, Smush, etc.)",
"\\L\\c0""  Gregory Montoir",
"\\L\\c2""    Engine: Flight of the Amazon Queen",
"\\L\\c0""  Joost Peters",
"\\L\\c2""    Engine: Beneath a Steel Sky, Flight of the",
"\\L\\c2""    Amazon Queen",
"\\L\\c0""  Eugene Sandulenko",
"\\L\\c2""    Engine: SCUMM (FT INSANE, bugfixes)",
"\\L\\c0""  Chris Apers",
"\\L\\c2""    Port: PalmOS",
"\\L\\c0""  Nicolas Bacca",
"\\L\\c2""    Port: PocketPC/WinCE port",
"\\L\\c0""  Marcus Comstedt",
"\\L\\c2""    Port: Dreamcast",
"\\L\\c0""  Ruediger Hanke",
"\\L\\c2""    Port: MorphOS",
"\\L\\c0""  Jamieson Christian",
"\\L\\c2""    iMUSE, MIDI, all things musical",
"\\L\\c0""  Jerome Fisher",
"\\L\\c2""    MT-32 emulator",
"\\L\\c0""  Jochen Hoenicke",
"\\L\\c2""    Speaker & PCjr sound support, Adlib work",
"\\L\\c0""",
"\\C\\c1""Retired Team Members:",
"\\L\\c0""  Ralph Brorsen",
"\\L\\c2""    Help with GUI implementation",
"\\L\\c0""  Vincent Hamm",
"\\L\\c2""    Co-Founder",
"\\L\\c0""  Felix Jakschitsch",
"\\L\\c2""    Zak256 reverse engineering",
"\\L\\c0""  Mutwin Kraus",
"\\L\\c2""    Original MacOS porter",
"\\L\\c0""  Peter Moraliyski",
"\\L\\c2""    Port: GP32",
"\\L\\c0""  Jeremy Newman",
"\\L\\c2""    Former webmaster",
"\\L\\c0""  Ludvig Strigeus",
"\\L\\c2""    Original ScummVM and SimonVM author",
"\\L\\c0""  Lionel Ulmer",
"\\L\\c2""    Port: X11",
"\\L\\c0""",
"\\C\\c1""Contributors:",
"\\L\\c0""  Tore Anderson",
"\\L\\c2""    Packaging for Debian GNU/Linux",
"\\L\\c0""  Stuart Caie",
"\\L\\c2""    Decoders for Simon 1 Amiga data files",
"\\L\\c0""  Janne Huttunen",
"\\L\\c2""    V3 actor mask support, Dig/FT Smush audio",
"\\L\\c0""  Kovacs Endre Janos",
"\\L\\c2""    Several fixes for Simon1",
"\\L\\c0""  Jeroen Janssen",
"\\L\\c2""    Numerous readability and bugfix patches",
"\\L\\c0""  Claudio Matsuoka",
"\\L\\c2""    Daily Linux/BeOS builds ",
"\\L\\c0""  Mikesch Nepomuk",
"\\L\\c2""    MI1 VGA floppy patches",
"\\L\\c0""  Nicolas Noble",
"\\L\\c2""    Config file and ALSA support",
"\\L\\c0""  Willem Jan Palenstijn",
"\\L\\c2""    Packaging for Fedora/RedHat",
"\\L\\c0""  Quietust",
"\\L\\c2""    Sound support for Amiga SCUMM V2 and",
"\\L\\c2""    V3 games",
"\\L\\c0""  Andreas Roever",
"\\L\\c2""    Broken Sword 1/2 MPEG2 cutscene support",
"\\L\\c0""  Edward Rudd",
"\\L\\c2""    Fixes for playing MP3 versions of MI1/Loom",
"\\L\\c2""    audio",
"\\L\\c0""  Daniel Schepler",
"\\L\\c2""    Final MI1 CD music support, initial Ogg",
"\\L\\c2""    Vorbis support",
"\\L\\c0""  Andre Souza",
"\\L\\c2""    SDL-based OpenGL renderer",
"\\L\\c0""  Tim 'realmz'",
"\\L\\c2""    Initial MI1 CD music support",
"\\L\\c0""",
"\\L\\c0""  And to all the contributors, users, and beta",
"\\L\\c0""  testers we've missed. Thanks!",
"\\L\\c0""",
"\\C\\c1""Special thanks to:",
"\\L\\c0""  Sander Buskens",
"\\L\\c2""    For his work on the initial reversing of",
"\\L\\c2""    Monkey2",
"\\L\\c0""  Canadacow",
"\\L\\c2""    For the original MT-32 emulator",
"\\L\\c0""  Kevin Carnes",
"\\L\\c2""    For Scumm16, the basis of ScummVM's older",
"\\L\\c2""    gfx codecs",
"\\L\\c0""  Jezar",
"\\L\\c2""    For his freeverb filter implementation",
"\\L\\c0""  Jim Leiterman",
"\\L\\c2""    Various info on his FM-TOWNS/Marty SCUMM",
"\\L\\c2""    ports",
"\\L\\c0""  Jimmi Thogersen",
"\\L\\c2""    For ScummRev, and much obscure",
"\\L\\c2""    code/documentation",
"\\L\\c0""  Tristan",
"\\L\\c2""    For additional work on the original MT-32",
"\\L\\c2""    emulator",
"\\L\\c0""",
"\\L\\c0""Tony Warriner and everyone at Revolution",
"\\L\\c0""Software Ltd. for sharing with us the source",
"\\L\\c0""of some of their brilliant games, allowing us",
"\\L\\c0""to release Beneath a Steel Sky as freeware...",
"\\L\\c0""and generally being supportive above and",
"\\L\\c0""beyond the call of duty.",
"\\L\\c0""",
"\\L\\c0""John Passfield and Steve Stamatiadis for",
"\\L\\c0""sharing the source of their classic title,",
"\\L\\c0""Flight of the Amazon Queen and also being",
"\\L\\c0""incredibly supportive.",
"\\L\\c0""",
"\\L\\c0""Aric Wilmunder, Ron Gilbert, David Fox, Vince",
"\\L\\c0""Lee, and all those at LucasFilm/LucasArts who",
"\\L\\c0""made SCUMM the insane mess to reimplement",
"\\L\\c0""that it is today. Feel free to drop us a line",
"\\L\\c0""and tell us what you think, guys!",
};

AboutDialog::AboutDialog()
	: Dialog(10, 20, 300, 174),
	_scrollPos(0), _scrollTime(0) {
	
	int i;
	
	_lineHeight = g_gui.getFont().getFontHeight() + 3;

	for (i = 0; i < 1; i++)
		_lines.push_back("");

	Common::String version("\\C\\c0""ScummVM ");
	version += gScummVMVersion;
	_lines.push_back(version);

	Common::String date("\\C\\c2""(built on ");
	date += gScummVMBuildDate;
	date += ')';
	_lines.push_back(date);

	Common::String features("\\C\\c2""Supports: ");
	features += gScummVMFeatures;
	_lines.push_back(features);

	_lines.push_back("");
	
	for (i = 0; i < ARRAYSIZE(about_lines); i++)
		_lines.push_back(about_lines[i]);
}

void AboutDialog::open() {
	_scrollTime = g_system->getMillis() + kScrollStartDelay;
	_scrollPos = 0;

	Dialog::open();
}

void AboutDialog::drawDialog() {
	// Blend over the background
	g_gui.blendRect(_x, _y, _w, _h, g_gui._bgcolor);

	// Draw text
	// TODO: Add a "fade" effect for the top/bottom text lines
	const int firstLine = _scrollPos / _lineHeight;
	const int lastLine = MIN((uint)(_scrollPos + _h) / _lineHeight + 1, _lines.size());
	int y = _y + kYOff - (_scrollPos % _lineHeight);

	for (int line = firstLine; line < lastLine; line++) {
		const char *str = _lines[line].c_str();
		Graphics::TextAlignment align = Graphics::kTextAlignCenter;
		OverlayColor color = g_gui._textcolor;
		while (str[0] == '\\') {
			switch (str[1]) {
			case 'C':
				align = Graphics::kTextAlignCenter;
				break;
			case 'L':
				align = Graphics::kTextAlignLeft;
				break;
			case 'R':
				align = Graphics::kTextAlignRight;
				break;
			case 'c':
				switch (str[2]) {
				case '0':
					color = g_gui._textcolor;
					break;
				case '1':
					color = g_gui._textcolorhi;
					break;
				case '2':
					color = g_gui._color;
					break;
				case '3':
					color = g_gui._shadowcolor;
					break;
				case '4':
					color = g_gui._bgcolor;
					break;
				default:
					warning("Unknown color type '%c'", str[2]);
				}
				str++;
				break;
			default:
				warning("Unknown scroller opcode '%c'\n", str[1]);
				break;
			}
			str += 2;
		}
		// Trim leading whitespaces if center mode is on
		if (align == Graphics::kTextAlignCenter)
			while (*str && *str == ' ')
				str++;
	
		g_gui.drawString(str, _x + kXOff, y, _w - 2 * kXOff, color, align);
		y += _lineHeight;
	}

	// Draw a border
	g_gui.box(_x, _y, _w, _h, g_gui._color, g_gui._shadowcolor);

	// Finally blit it all to the screen
	g_gui.addDirtyRect(_x, _y, _w, _h);
}


void AboutDialog::handleTickle() {
	const uint32 t = g_system->getMillis();
	int scrollOffset = ((int)t - (int)_scrollTime) / kScrollMillisPerPixel;
	if (scrollOffset > 0) {
		_scrollPos += scrollOffset;
		_scrollTime = t;

		if (_scrollPos < 0) {
			_scrollPos = 0;
		} else if ((uint32)_scrollPos > _lines.size() * _lineHeight) {
			_scrollPos = 0;
			_scrollTime += kScrollStartDelay;
		} else {
			g_gui.addDirtyRect(_x, _y, _w, _h);
		}
		draw();	// Issue a full redraw
	}
}

void AboutDialog::handleMouseUp(int x, int y, int button, int clickCount) {
	// Close upon any mouse click
	close();
}

void AboutDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
}

void AboutDialog::handleKeyUp(uint16 ascii, int keycode, int modifiers) {
	close();
}


} // End of namespace GUI
