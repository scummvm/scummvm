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

#include "sherlock/tattoo/widget_hangman.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

void WidgetHangman::show() {
	Events &events = *_vm->_events;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	char answers[3][10];
	Common::Point lines[3];
	const char *solutions[3];
	int numWide, spacing;
	ImageFile *paper;
	Common::Point cursorPos;
	byte cursorColor = 254;
	bool solved = false;
	bool done = false;
	bool flag = false;
	size_t i = 0;

	switch (_vm->getLanguage()) {
	case Common::FR_FRA:
		lines[0] = Common::Point(34, 210);
		lines[1] = Common::Point(72, 242);
		lines[2] = Common::Point(34, 276);
		numWide = 8;
		spacing = 19;
		paper = new ImageFile("paperf.vgs");
		break;

	case Common::DE_DEU:
		lines[0] = Common::Point(44, 73);
		lines[1] = Common::Point(56, 169);
		lines[2] = Common::Point(47, 256);
		numWide = 7;
		spacing = 19;
		paper = new ImageFile("paperg.vgs");
		break;

	default:
		// English
		lines[0] = Common::Point(65, 84);
		lines[1] = Common::Point(65, 159);
		lines[2] = Common::Point(75, 234);
		numWide = 5;
		spacing = 20;
		paper = new ImageFile("paper.vgs");
		break;
	}
	
	ImageFrame &paperFrame = (*paper)[0];
	Common::Rect paperBounds(paperFrame._width, paperFrame._height);
	paperBounds.moveTo((screen.w() - paperFrame._width) / 2, (screen.h() - paperFrame._height) / 2);

	for (int line = 0; line<3; ++line) {
		lines[line].x += paperBounds.left;
		lines[line].y += paperBounds.top;

		for (i = 0; i <= (size_t)numWide; ++i)
			answers[line][i] = 0;
	}

	screen._backBuffer1.blitFrom(paperFrame, Common::Point(paperBounds.left + screen._currentScroll.x, 0));

	// If they have already solved the puzzle, put the answer on the graphic
	if (_vm->readFlags(299)) {
		for (int line = 0; line < 3; ++line) {
			cursorPos.y = lines[line].y - screen.fontHeight() - 2;

			for (i = 0; i < strlen(solutions[line]); ++i) {
				cursorPos.x = lines[line].x + 8 - screen.widestChar() / 2 + i * spacing;
				screen.gPrint(Common::Point(cursorPos.x + screen.widestChar() / 2 - 
					screen.charWidth(solutions[line][i]) / 2, cursorPos.y), 0, "%c", solutions[line][i]);
			}
		}
	}

	screen.slamRect(paperBounds);
	cursorPos = Common::Point(lines[0].x + 8 - screen.widestChar() / 2, lines[0].y - screen.fontHeight() - 2);
	int line = 0;

	// If they have not solved the puzzle, let them solve it here
	if (!_vm->readFlags(299)) {
		do {
			while (!events.kbHit()) {
				// See if a key or a mouse button is pressed
				events.pollEventsAndWait();
				events.setButtonState();

				flag = !flag;
				if (flag) {
					screen._backBuffer1.fillRect(Common::Rect(cursorPos.x + screen._currentScroll.x, cursorPos.y, 
						cursorPos.x + screen.widestChar() + screen._currentScroll.x - 1, cursorPos.y + screen.fontHeight() - 1), cursorColor);
					if (answers[line][i])
						screen.gPrint(Common::Point(cursorPos.x + screen.widestChar() / 2 - screen.charWidth(answers[line][i]) / 2, 
							cursorPos.y), 0, "%c", answers[line][i]);
					screen.slamArea(cursorPos.x, cursorPos.y, screen.widestChar(), screen.fontHeight());
				} else {
					screen.setDisplayBounds(Common::Rect(cursorPos.x + screen._currentScroll.x, cursorPos.y, 
						cursorPos.x + screen.widestChar() + screen._currentScroll.x, cursorPos.y + screen.fontHeight()));
					screen._backBuffer->blitFrom(paperFrame, Common::Point(paperBounds.left + screen._currentScroll.x, paperBounds.top));
					screen.resetDisplayBounds();

					if (answers[line][i])
						screen.gPrint(Common::Point(cursorPos.x + screen.widestChar() / 2 - screen.charWidth(answers[line][i]) / 2, 
							cursorPos.y), 0, "%c", answers[line][i]);
					screen.slamArea(cursorPos.x, cursorPos.y, screen.widestChar(), screen.fontHeight());
				}

				if (!events.kbHit())
					events.wait(2);
			}

			if (events.kbHit()) {
				Common::KeyState keyState = events.getKey();

				if (((toupper(keyState.ascii) >= 'A') && (toupper(keyState.ascii) <= 'Z')) ||
					((keyState.ascii >= 128) && ((keyState.ascii <= 168) || (keyState.ascii == 225)))) {
					answers[line][i] = keyState.ascii;
					keyState.keycode = Common::KEYCODE_RIGHT;
				}

				screen.setDisplayBounds(Common::Rect(cursorPos.x + screen._currentScroll.x, cursorPos.y, 
					cursorPos.x + screen.widestChar() + screen._currentScroll.x, cursorPos.y + screen.fontHeight()));
				screen._backBuffer->blitFrom(paperFrame, Common::Point(paperBounds.left + screen._currentScroll.x, paperBounds.top));
				screen.resetDisplayBounds();

				if (answers[line][i])
					screen.gPrint(Common::Point(cursorPos.x + screen.widestChar() / 2 - screen.charWidth(answers[line][i]) / 2,
						cursorPos.y), 0, "%c", answers[line][i]);
				screen.slamArea(cursorPos.x, cursorPos.y, screen.widestChar(), screen.fontHeight());

				switch (keyState.keycode) {
				case Common::KEYCODE_ESCAPE:
					done = true;
					break;

				case Common::KEYCODE_UP:
					if (line) {
						line--;
						if (i >= strlen(solutions[line]))
							i = strlen(solutions[line]) - 1;
					}
					break;

				case Common::KEYCODE_DOWN:
					if (line < 2) {
						++line;
						if (i >= strlen(solutions[line]))
							i = strlen(solutions[line]) - 1;
					}
					break;

				case Common::KEYCODE_BACKSPACE:
				case Common::KEYCODE_LEFT:
					if (i)
						--i;
					else if (line) {
						--line;
						
						i = strlen(solutions[line]) - 1;
					}

					if (keyState.keycode == Common::KEYCODE_BACKSPACE)
						answers[line][i] = ' ';
					break;

				case Common::KEYCODE_RIGHT:
					if (i < strlen(solutions[line]) - 1)
						i++;
					else if (line < 2) {
						++line;
						i = 0;
					}
					break;

				case Common::KEYCODE_DELETE:
					answers[line][i] = ' ';
					break;

				default:
					break;
				}
			}

			cursorPos.x = lines[line].x + 8 - screen.widestChar() / 2 + i * spacing;
			cursorPos.y = lines[line].y - screen.fontHeight() - 2;

			// See if all of their anwers are correct
			if (!scumm_stricmp(answers[0], solutions[0]) && !scumm_stricmp(answers[1], solutions[1]) && 
					!scumm_stricmp(answers[2], solutions[2])) {
				done = true;
				solved = true;
			}
		} while (!done && !_vm->shouldQuit());
	} else {
		// They have already solved the puzzle, so just display the solution and wait for a mouse or key click
		do {
			events.pollEventsAndWait();
			events.setButtonState();

			if ((events.kbHit()) || (events._released) || (events._rightReleased)) {
				done = true;
				events.clearEvents();
			}
		} while (!done && !_vm->shouldQuit());
	}

	delete paper;
	screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(paperBounds.left + screen._currentScroll.x, paperBounds.top),
		Common::Rect(paperBounds.left + screen._currentScroll.x, paperBounds.top,
		paperBounds.right + screen._currentScroll.x, paperBounds.bottom));
	scene.doBgAnim();

	screen.slamArea(paperBounds.left + screen._currentScroll.x, paperBounds.top,
		paperBounds.width(), paperBounds.height());

	// Don't call the talk files if the puzzle has already been solved
	if (_vm->readFlags(299))
		return;

	// If they solved the puzzle correctly, set the solved flag and run the appropriate talk scripts
	if (solved) {
		talk.talkTo("SLVE12S.TLK");
		talk.talkTo("WATS12X.TLK");
		_vm->setFlags(299);
	} else {
		talk.talkTo("HOLM12X.TLK");
	}
}

} // End of namespace Tattoo

} // End of namespace Sherlock
