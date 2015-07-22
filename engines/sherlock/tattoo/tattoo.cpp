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
	switch (_scene->_goToScene) {
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
			if (_creditLines[idx]._position.x >= 65536) {
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
	// TODO
}

} // End of namespace Tattoo

} // End of namespace Sherlock
