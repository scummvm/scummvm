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

#include "engines/util.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo_resources.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/widget_base.h"
#include "sherlock/people.h"

namespace Sherlock {

namespace Tattoo {

TattooEngine::TattooEngine(OSystem *syst, const SherlockGameDescription *gameDesc) :
		SherlockEngine(syst, gameDesc), _darts(this) {
	_creditsActive = false;
	_runningProlog = false;
	_fastMode = false;
	_allowFastMode = true;
	_transparentMenus = true;
	_creditSpeed = 4;
}

TattooEngine::~TattooEngine() {
}

void TattooEngine::showOpening() {
	// No implementation - opening is done using in-game scenes
}

void TattooEngine::initialize() {
	initGraphics(640, 480, true);

	// Initialize the base engine
	SherlockEngine::initialize();

	// Initialise the global flags
	_flags.resize(3200);
	_flags[1] = _flags[4] = _flags[76] = true;
	_runningProlog = true;

	// Add some more files to the cache
	_res->addToCache("walk.lib");
	
	// Set up list of people
	for (int idx = 0; idx < TATTOO_MAX_PEOPLE; ++idx) {
		_people->_characters.push_back(PersonData(
			getLanguage() == Common::FR_FRA ? FRENCH_NAMES[idx] : ENGLISH_NAMES[idx],
			PORTRAITS[idx], nullptr, nullptr));
	}

	// Load the inventory
	loadInventory();

	// Starting scene
	_scene->_goToScene = STARTING_INTRO_SCENE;

	// Load an initial palette
	loadInitialPalette();
}

void TattooEngine::startScene() {
	TattooUserInterface &ui = *(TattooUserInterface *)_ui;

	switch (_scene->_goToScene) {
	case 7:
	case 8:
	case 18:
	case 53:
	case 68:
		// Load overlay mask(s) for the scene
		ui._mask = new ImageFile(Common::String::format("res%02d.msk", _scene->_goToScene));
		if (_scene->_goToScene == 8)
			ui._mask1 = new ImageFile("res08a.msk");
		else if (_scene->_goToScene == 18 || _scene->_goToScene == 68)
			ui._mask1 = new ImageFile("res08a.msk");
		break;

	case OVERHEAD_MAP:
	case OVERHEAD_MAP2:
		// Show the map
		_scene->_currentScene = OVERHEAD_MAP;
		_scene->_goToScene = _map->show();

		_people->_savedPos = Common::Point(-1, -1);
		_people->_savedPos._facing = -1;
		break;

	case 101:
		// Darts Board minigame
		_darts.playDarts(GAME_CRICKET);
		break;
	
	case 102:
		// Darts Board minigame
		_darts.playDarts(GAME_301);
		break;

	case 103:
		// Darts Board minigame
		_darts.playDarts(GAME_501);
		break;

	default:
		break;
	}

	_events->setCursor(ARROW);
}

void TattooEngine::loadInitialPalette() {
	byte palette[768];
	Common::SeekableReadStream *stream = _res->load("room.pal");
	stream->read(palette, PALETTE_SIZE);
	_screen->translatePalette(palette);
	_screen->setPalette(palette);

	delete stream;
}

void TattooEngine::loadInventory() {
	Inventory &inv = *_inventory;

	Common::String inv1 = _fixedText->getText(kFixedText_Inv1);
	Common::String inv2 = _fixedText->getText(kFixedText_Inv2);
	Common::String inv3 = _fixedText->getText(kFixedText_Inv3);
	Common::String inv4 = _fixedText->getText(kFixedText_Inv4);
	Common::String inv5 = _fixedText->getText(kFixedText_Inv5);
	Common::String inv6 = _fixedText->getText(kFixedText_Inv6);
	Common::String inv7 = _fixedText->getText(kFixedText_Inv7);
	Common::String inv8 = _fixedText->getText(kFixedText_Inv8);
	Common::String invDesc1 = _fixedText->getText(kFixedText_InvDesc1);
	Common::String invDesc2 = _fixedText->getText(kFixedText_InvDesc2);
	Common::String invDesc3 = _fixedText->getText(kFixedText_InvDesc3);
	Common::String invDesc4 = _fixedText->getText(kFixedText_InvDesc4);
	Common::String invDesc5 = _fixedText->getText(kFixedText_InvDesc5);
	Common::String invDesc6 = _fixedText->getText(kFixedText_InvDesc6);
	Common::String invDesc7 = _fixedText->getText(kFixedText_InvDesc7);
	Common::String invDesc8 = _fixedText->getText(kFixedText_InvDesc8);
	Common::String solve = _fixedText->getText(kFixedText_Solve);

	// Initial inventory
	inv._holdings = 5;
	inv.push_back(InventoryItem(0, inv1, invDesc1, "_ITEM01A"));
	inv.push_back(InventoryItem(0, inv2, invDesc2, "_ITEM02A"));
	inv.push_back(InventoryItem(0, inv3, invDesc3, "_ITEM03A"));
	inv.push_back(InventoryItem(0, inv4, invDesc4, "_ITEM04A"));
	inv.push_back(InventoryItem(0, inv5, invDesc5, "_ITEM05A"));

	// Hidden items
	inv.push_back(InventoryItem(0, inv6, invDesc6, "_PAP212D", solve));
	inv.push_back(InventoryItem(0, inv7, invDesc7, "_PAP212I"));
	inv.push_back(InventoryItem(0, inv8, invDesc8, "_LANT02I"));
}

void TattooEngine::initCredits() {
	Common::SeekableReadStream *stream = _res->load("credits.txt");
	int spacing = _screen->fontHeight() * 2;
	int yp = _screen->h();

	_creditsActive = true;
	_creditLines.clear();

	while (stream->pos() < stream->size()) {
		Common::String line = stream->readLine();

		if (line.hasPrefix("Scroll Speed")) {
			const char *p = line.c_str() + 12;
			while ((*p < '0') || (*p > '9'))
				p++;
			
			_creditSpeed = atoi(p);
		} else if (line.hasPrefix("Y Spacing")) {
			const char *p = line.c_str() + 12;
			while ((*p < '0') || (*p > '9'))
				p++;

			spacing = atoi(p) + _screen->fontHeight() + 1;
		} else {
			int width = _screen->stringWidth(line) + 2;

			_creditLines.push_back(CreditLine(line, Common::Point((_screen->w() - width) / 2 + 1, yp), width));
			yp += spacing;
		}
	}

	// Post-processing for finding split lines
	for (int l = 0; l < (int)_creditLines.size(); ++l) {
		CreditLine &cl = _creditLines[l];
		const char *p = strchr(cl._line.c_str(), '-');

		if (p != nullptr && p[1] == '>') {
			cl._line2 = Common::String(p + 3);
			cl._line = Common::String(cl._line.c_str(), p);
			
			int width = cl._width;
			int width1 = _screen->stringWidth(cl._line);
			int width2 = _screen->stringWidth(cl._line2);

			int c = 1;
			for (int l1 = l + 1; l1 < (int)_creditLines.size(); ++l1) {
				if ((p = strchr(_creditLines[l1]._line.c_str(), '-')) != nullptr) {
					if (p[1] == '>') {
						Common::String line1 = Common::String(_creditLines[l1]._line.c_str(), p);
						Common::String line2 = Common::String(p + 3);

						width1 = MAX(width1, _screen->stringWidth(line1));

						if (_screen->stringWidth(line2) > width2)
							width2 = _screen->stringWidth(line2);
						++c;
					} else {
						break;
					}
				} else {
					break;
				}
			}

			width = width1 + width2 + _screen->widestChar();
			width1 += _screen->widestChar();

			for (int l1 = l; l1 < l + c; ++l1) {
				_creditLines[l1]._width = width;
				_creditLines[l1]._xOffset = width1;
			}

			l += c - 1;
		}
	}

	delete stream;
}

void TattooEngine::drawCredits() {
	Common::Rect screenRect(0, 0, _screen->w(), _screen->h());
	Surface &bb1 = _screen->_backBuffer1;

	for (uint idx = 0; idx < _creditLines.size() && _creditLines[idx]._position.y < _screen->h(); ++idx) {
		if (screenRect.contains(_creditLines[idx]._position)) {
			if (!_creditLines[idx]._line2.empty()) {
				int x1 = _creditLines[idx]._position.x;
				int x2 = x1 + _creditLines[idx]._xOffset;
				const Common::String &line1 = _creditLines[idx]._line;
				const Common::String &line2 = _creditLines[idx]._line2;

				bb1.writeString(line1, Common::Point(x1 - 1, _creditLines[idx]._position.y - 1), 0);
				bb1.writeString(line1, Common::Point(x1, _creditLines[idx]._position.y - 1), 0);
				bb1.writeString(line1, Common::Point(x1 + 1, _creditLines[idx]._position.y - 1), 0);

				bb1.writeString(line1, Common::Point(x1 - 1, _creditLines[idx]._position.y), 0);
				bb1.writeString(line1, Common::Point(x1 + 1, _creditLines[idx]._position.y), 0);

				bb1.writeString(line1, Common::Point(x1 - 1, _creditLines[idx]._position.y + 1), 0);
				bb1.writeString(line1, Common::Point(x1, _creditLines[idx]._position.y + 1), 0);
				bb1.writeString(line1, Common::Point(x1 + 1, _creditLines[idx]._position.y + 1), 0);

				bb1.writeString(line1, Common::Point(x1, _creditLines[idx]._position.y), INFO_TOP);

				bb1.writeString(line2, Common::Point(x2 - 1, _creditLines[idx]._position.y - 1), 0);
				bb1.writeString(line2, Common::Point(x2, _creditLines[idx]._position.y - 1), 0);
				bb1.writeString(line2, Common::Point(x2 + 1, _creditLines[idx]._position.y - 1), 0);

				bb1.writeString(line2, Common::Point(x2 - 1, _creditLines[idx]._position.y), 0);
				bb1.writeString(line2, Common::Point(x2 + 1, _creditLines[idx]._position.y), 0);

				bb1.writeString(line2, Common::Point(x2 - 1, _creditLines[idx]._position.y + 1), 0);
				bb1.writeString(line2, Common::Point(x2, _creditLines[idx]._position.y + 1), 0);
				bb1.writeString(line2, Common::Point(x2 + 1, _creditLines[idx]._position.y + 1), 0);

				bb1.writeString(line2, Common::Point(x2, _creditLines[idx]._position.y), INFO_TOP);
			} else {
				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x - 1, _creditLines[idx]._position.y - 1), 0);
				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x, _creditLines[idx]._position.y - 1), 0);
				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x + 1, _creditLines[idx]._position.y - 1), 0);

				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x - 1, _creditLines[idx]._position.y), 0);
				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x + 1, _creditLines[idx]._position.y), 0);

				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x - 1, _creditLines[idx]._position.y + 1), 0);
				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x, _creditLines[idx]._position.y + 1), 0);
				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x + 1, _creditLines[idx]._position.y + 1), 0);

				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x, _creditLines[idx]._position.y), INFO_TOP);
			}
		}
	}
}

void TattooEngine::blitCredits() {
	Common::Rect screenRect(0, -_creditSpeed, _screen->w(), _screen->h() + _creditSpeed);

	for (uint idx = 0; idx < _creditLines.size(); ++idx) {
		if (screenRect.contains(_creditLines[idx]._position)) {
			Common::Rect r(_creditLines[idx]._width, _screen->fontHeight() + 2);
			r.moveTo(_creditLines[idx]._position.x, _creditLines[idx]._position.y - 1);

			_screen->slamRect(r);
		}

		_creditLines[idx]._position.y -= _creditSpeed;
	}
}

void TattooEngine::eraseCredits() {
	Common::Rect screenRect(0, -_creditSpeed, _screen->w(), _screen->h() + _creditSpeed);

	for (uint idx = 0; idx < _creditLines.size(); ++idx) {
		if (screenRect.contains(_creditLines[idx]._position)) {
			Common::Rect r(_creditLines[idx]._width, _screen->fontHeight() + 3);
			r.moveTo(_creditLines[idx]._position.x, _creditLines[idx]._position.y - 1 + _creditSpeed);

			_screen->restoreBackground(r);
		}
	}

	if (_creditLines[_creditLines.size() - 1]._position.y < -_creditSpeed) {
		_creditLines.clear();
		_creditsActive = false;
		setFlags(!3000);
	}
}

void TattooEngine::doHangManPuzzle() {
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

	switch (getLanguage()) {
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
	paperBounds.moveTo((_screen->w() - paperFrame._width) / 2, (_screen->h() - paperFrame._height) / 2);

	for (int line = 0; line<3; ++line) {
		lines[line].x += paperBounds.left;
		lines[line].y += paperBounds.top;

		for (i = 0; i <= (size_t)numWide; ++i)
			answers[line][i] = 0;
	}

	_screen->_backBuffer1.blitFrom(paperFrame, Common::Point(paperBounds.left + _screen->_currentScroll.x, 0));

	// If they have already solved the puzzle, put the answer on the graphic
	if (readFlags(299)) {
		for (int line = 0; line < 3; ++line) {
			cursorPos.y = lines[line].y - _screen->fontHeight() - 2;

			for (i = 0; i < strlen(solutions[line]); ++i) {
				cursorPos.x = lines[line].x + 8 - _screen->widestChar() / 2 + i * spacing;
				_screen->gPrint(Common::Point(cursorPos.x + _screen->widestChar() / 2 - 
					_screen->charWidth(solutions[line][i]) / 2, cursorPos.y), 0, "%c", solutions[line][i]);
			}
		}
	}

	_screen->slamRect(paperBounds);
	cursorPos = Common::Point(lines[0].x + 8 - _screen->widestChar() / 2, lines[0].y - _screen->fontHeight() - 2);
	int line = 0;

	// If they have not solved the puzzle, let them solve it here
	if (!readFlags(299)) {
		do {
			while (!_events->kbHit()) {
				// See if a key or a mouse button is pressed
				_events->pollEventsAndWait();
				_events->setButtonState();

				flag = !flag;
				if (flag) {
					_screen->_backBuffer1.fillRect(Common::Rect(cursorPos.x + _screen->_currentScroll.x, cursorPos.y, 
						cursorPos.x + _screen->widestChar() + _screen->_currentScroll.x - 1, cursorPos.y + _screen->fontHeight() - 1), cursorColor);
					if (answers[line][i])
						_screen->gPrint(Common::Point(cursorPos.x + _screen->widestChar() / 2 - _screen->charWidth(answers[line][i]) / 2, 
							cursorPos.y), 0, "%c", answers[line][i]);
					_screen->slamArea(cursorPos.x, cursorPos.y, _screen->widestChar(), _screen->fontHeight());
				} else {
					_screen->setDisplayBounds(Common::Rect(cursorPos.x + _screen->_currentScroll.x, cursorPos.y, 
						cursorPos.x + _screen->widestChar() + _screen->_currentScroll.x, cursorPos.y + _screen->fontHeight()));
					_screen->_backBuffer->blitFrom(paperFrame, Common::Point(paperBounds.left + _screen->_currentScroll.x, paperBounds.top));
					_screen->resetDisplayBounds();

					if (answers[line][i])
						_screen->gPrint(Common::Point(cursorPos.x + _screen->widestChar() / 2 - _screen->charWidth(answers[line][i]) / 2, 
							cursorPos.y), 0, "%c", answers[line][i]);
					_screen->slamArea(cursorPos.x, cursorPos.y, _screen->widestChar(), _screen->fontHeight());
				}

				if (!_events->kbHit())
					_events->wait(2);
			}

			if (_events->kbHit()) {
				Common::KeyState keyState = _events->getKey();

				if (((toupper(keyState.ascii) >= 'A') && (toupper(keyState.ascii) <= 'Z')) ||
					((keyState.ascii >= 128) && ((keyState.ascii <= 168) || (keyState.ascii == 225)))) {
					answers[line][i] = keyState.ascii;
					keyState.keycode = Common::KEYCODE_RIGHT;
				}

				_screen->setDisplayBounds(Common::Rect(cursorPos.x + _screen->_currentScroll.x, cursorPos.y, 
					cursorPos.x + _screen->widestChar() + _screen->_currentScroll.x, cursorPos.y + _screen->fontHeight()));
				_screen->_backBuffer->blitFrom(paperFrame, Common::Point(paperBounds.left + _screen->_currentScroll.x, paperBounds.top));
				_screen->resetDisplayBounds();

				if (answers[line][i])
					_screen->gPrint(Common::Point(cursorPos.x + _screen->widestChar() / 2 - _screen->charWidth(answers[line][i]) / 2,
						cursorPos.y), 0, "%c", answers[line][i]);
				_screen->slamArea(cursorPos.x, cursorPos.y, _screen->widestChar(), _screen->fontHeight());

				switch (keyState.keycode) {
				case Common::KEYCODE_ESCAPE:
					done = true;
					break;

				case Common::KEYCODE_UP:
				case Common::KEYCODE_KP8:
					if (line) {
						line--;
						if (i >= strlen(solutions[line]))
							i = strlen(solutions[line]) - 1;
					}
					break;

				case Common::KEYCODE_DOWN:
				case Common::KEYCODE_KP2:
					if (line < 2) {
						++line;
						if (i >= strlen(solutions[line]))
							i = strlen(solutions[line]) - 1;
					}
					break;

				case Common::KEYCODE_BACKSPACE:
				case Common::KEYCODE_LEFT:
				case Common::KEYCODE_KP4:
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
				case Common::KEYCODE_KP6:
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

			cursorPos.x = lines[line].x + 8 - _screen->widestChar() / 2 + i * spacing;
			cursorPos.y = lines[line].y - _screen->fontHeight() - 2;

			// See if all of their anwers are correct
			if (!scumm_stricmp(answers[0], solutions[0]) && !scumm_stricmp(answers[1], solutions[1]) && 
					!scumm_stricmp(answers[2], solutions[2])) {
				done = true;
				solved = true;
			}
		} while (!done && !shouldQuit());
	} else {
		// They have already solved the puzzle, so just display the solution and wait for a mouse or key click
		do {
			_events->pollEventsAndWait();
			_events->setButtonState();

			if ((_events->kbHit()) || (_events->_released) || (_events->_rightReleased)) {
				done = true;
				_events->clearEvents();
			}
		} while (!done && !shouldQuit());
	}

	delete paper;
	_screen->_backBuffer1.blitFrom(_screen->_backBuffer2, Common::Point(paperBounds.left + _screen->_currentScroll.x, paperBounds.top),
		Common::Rect(paperBounds.left + _screen->_currentScroll.x, paperBounds.top,
		paperBounds.right + _screen->_currentScroll.x, paperBounds.bottom));
	_scene->doBgAnim();

	_screen->slamArea(paperBounds.left + _screen->_currentScroll.x, paperBounds.top,
		paperBounds.width(), paperBounds.height());

	// Don't call the talk files if the puzzle has already been solved
	if (readFlags(299))
		return;

	// If they solved the puzzle correctly, set the solved flag and run the appropriate talk scripts
	if (solved) {
		_talk->talkTo("SLVE12S.TLK");
		_talk->talkTo("WATS12X.TLK");
		setFlags(299);
	} else {
		_talk->talkTo("HOLM12X.TLK");
	}
}

} // End of namespace Tattoo

} // End of namespace Sherlock
