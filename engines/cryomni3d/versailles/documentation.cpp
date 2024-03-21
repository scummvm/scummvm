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

#include "common/file.h"
#include "common/system.h"
#include "graphics/managed_surface.h"
#include "image/bmp.h"

#include "cryomni3d/font_manager.h"
#include "cryomni3d/mouse_boxes.h"
#include "cryomni3d/sprites.h"

#include "cryomni3d/versailles/documentation.h"
#include "cryomni3d/versailles/engine.h"

namespace CryOmni3D {
namespace Versailles {

const Versailles_Documentation::TimelineEntry Versailles_Documentation::kTimelineEntries[] = {
	{ "1638", 340,  15 },
	{ "1643", 470,  30 },
	{ "1648", 380,  45 },
	{ "1649", 500,  60 },
	{ "1650", 420,  75 },
	{ "1651", 520,  90 },
	{ "1652", 450, 105 },
	{ "1654", 540, 120 },
	{ "1658", 470, 135 },
	{ "1659", 550, 150 },
	{ "1660", 480, 165 },
	{ "1661", 560, 180 },
	{ "1662", 490, 195 },
	{ "1663", 560, 210 },
	{ "1664", 490, 225 },
	{ "1665", 560, 240 },
	{ "1666", 490, 255 },
	{ "1667", 560, 270 },
	{ "1668", 490, 285 },
	{ "1670", 550, 300 },
	{ "1671", 480, 315 },
	{ "1672", 540, 330 },
	{ "1673", 470, 345 },
	{ "1674", 530, 360 },
	{ "1675", 460, 375 },
	{ "1678", 510, 390 },
	{ "1680", 430, 405 },
	{ "1681", 490, 420 },
	{ "1682", 400, 435 },
	{ "1683", 450, 450 },
	{ "1684", 156, 444 },
	{ "1685",  81, 439 },
	{ "1686", 140, 422 },
	{ "1687",  73, 413 },
	{ "1689", 128, 401 },
	{ "1697",  62, 389 },
	{ "1700", 121, 378 },
	{ "1701",  62, 366 },
	{ "1702", 121, 355 },
	{ "1709",  62, 344 },
	{ "1710", 121, 333 },
	{ "1712",  67, 322 },
	{ "1715", 128, 311 },
};

void Versailles_Documentation::init(const Sprites *sprites, FontManager *fontManager,
									const Common::StringArray *messages, CryOmni3DEngine_Versailles *engine,
									const Common::Path &allDocsFilePath, const Common::Path &linksDocsFilePath) {
	_sprites = sprites;
	_fontManager = fontManager;
	_messages = messages;
	_engine = engine;
	_allDocsFilePath = allDocsFilePath;
	_linksDocsFilePath = linksDocsFilePath;

	// Japanese version of Versailles handles records attributes with multilines
	_multilineAttributes = (_engine->getLanguage() == Common::JA_JPN);

	// Build list of records
	Common::File allDocsFile;

	if (!allDocsFile.open(_allDocsFilePath)) {
		error("Can't open %s", _allDocsFilePath.toString(Common::Path::kNativeSeparator).c_str());
	}

	uint allDocsSize = allDocsFile.size();
	char *allDocs = new char[allDocsSize + 1];
	char *end = allDocs + allDocsSize;
	allDocsFile.read(allDocs, allDocsSize);
	allDocs[allDocsSize] = '\0';
	allDocsFile.close();

	const char *patterns[] = { "FICH=", nullptr };
	RecordInfo record = { uint(-1), uint(-1), uint(-1) };

	char *currentPos = allDocs;
	char *lastRecordName = nullptr;
	bool first = true;

	while (true) {
		currentPos = getDocPartAddress(currentPos, end, patterns);
		if (!currentPos) {
			break;
		}
		currentPos -= 5;
		if (first) {
			record.position = currentPos - allDocs;
			record.id = 0;

			lastRecordName = currentPos + 5;

			first = false;
		} else {
			record.size = (currentPos - allDocs) - record.position;
			//debug("Found record %s", lastRecordName);
			_records[lastRecordName] = record;
			_recordsOrdered.push_back(lastRecordName);

			record.id++;
			record.position = currentPos - allDocs;

			lastRecordName = currentPos + 5;
		}
		// Next line
		currentPos += strlen(currentPos) + 1;
	}
	record.size = allDocsSize - record.position;
	_records[lastRecordName] = record;
	_recordsOrdered.push_back(lastRecordName);

	delete[] allDocs;
}

void Versailles_Documentation::handleDocArea() {
	_engine->showMouse(false);

	// Load all links lazily and free them at the end to not waste memory
	// Maybe it's not really useful
	getLinks("ALL00", _allLinks);

	bool end = false;
	while (!end) {
		Common::String selectedRecord = docAreaHandleSummary();
		if (selectedRecord == "") {
			end = true;
		} else if (selectedRecord == "VT00") {
			selectedRecord = docAreaHandleTimeline();
			if (selectedRecord != "") {
				if (docAreaHandleRecords(selectedRecord) == 2) {
					end = true;
				}
			}
		} else {
			if (docAreaHandleRecords(selectedRecord) == 2) {
				end = true;
			}
		}
	}

	_allLinks.clear();

	_engine->showMouse(true);
}

void Versailles_Documentation::handleDocInGame(const Common::String &record) {
	_visitTrace.clear();
	_currentRecord = record;

	Graphics::ManagedSurface docSurface;
	Common::String nextRecord;
	MouseBoxes boxes(3);

	_engine->showMouse(false);
	bool end = false;
	while (!end) {
		inGamePrepareRecord(docSurface, boxes);
		uint action = inGameHandleRecord(docSurface, boxes, nextRecord);
		switch (action) {
		case 0:
			// Back
			if (!_visitTrace.empty()) {
				_currentRecord = _visitTrace.back();
				_visitTrace.pop_back();
				break;
			}
		// No previous record, like a quit
		// fall through
		case 1:
			// Quit
			end = true;
			break;
		case 2:
			// Follow hyperlink keeping trace
			_visitTrace.push_back(_currentRecord);
			_currentRecord = nextRecord;
			break;
		default:
			error("Invalid case %d when displaying doc record", action);
		}
	}
	_engine->showMouse(true);
}

Common::String Versailles_Documentation::docAreaHandleSummary() {
	struct Category {
		const char *record;
		const char *bmp;
		Common::Point imgPos;
		Common::Rect linesPos;
		const Common::String *title;
		Graphics::Surface highlightedImg;

		Category(const char *record_, const char *bmp_, const Common::Point &imgPos_,
		         const Common::Rect &linesPos_, const Common::String *title_) :
			record(record_), bmp(bmp_), imgPos(imgPos_), linesPos(linesPos_), title(title_) { }
	} categories[8] = {
		Category(
		    "VA00",
		    "VA.bmp",
		    Common::Point(142, 402),
		    Common::Rect(174, 372, 284, 372),
		    &(*_messages)[68]),
		Category(
		    "VR00",
		    "VR.bmp",
		    Common::Point(82, 187),
		    Common::Rect(89, 187, 217, 212),
		    &(*_messages)[69]),
		Category(
		    "VC00",
		    "VC.bmp",
		    Common::Point(176, 105),
		    Common::Rect(177, 107, 292, 130),
		    &(*_messages)[70]),
		Category(
		    "VT00",
		    "VH.bmp", //sic
		    Common::Point(283, 467),
		    Common::Rect(311, 451, 466, 451),
		    &(*_messages)[73]),
		Category(
		    "VV00",
		    "VV.bmp",
		    Common::Point(68, 305),
		    Common::Rect(94, 292, 300, 292),
		    &(*_messages)[71]),
		Category(
		    "VS00",
		    "VS.bmp",
		    Common::Point(321, 70),
		    Common::Rect(322, 71, 540, 94),
		    &(*_messages)[72]),
		Category(
		    nullptr,
		    nullptr,
		    Common::Point(256, 212),
		    Common::Rect(),
		    nullptr),
		Category(
		    nullptr,
		    nullptr,
		    Common::Point(600, 450),
		    Common::Rect(),
		    nullptr)
	};
	Image::BitmapDecoder bmpDecoder;
	Common::File file;

	Image::ImageDecoder *imageDecoder = _engine->loadHLZ(_engine->getFilePath(kFileTypeDocBg, "SOM1.HLZ"));
	if (!imageDecoder) {
		return "";
	}
	const Graphics::Surface *bgFrame = imageDecoder->getSurface();

	for (uint i = 0; i < ARRAYSIZE(categories); i++) {
		if (!categories[i].bmp) {
			// No BMP to load
			continue;
		}
		if (!file.open(_engine->getFilePath(kFileTypeSpriteBmp, categories[i].bmp))) {
			error("Failed to open BMP file: %s", categories[i].bmp);
		}
		if (!bmpDecoder.loadStream(file)) {
			error("Failed to load BMP file: %s", categories[i].bmp);
		}
		categories[i].highlightedImg.copyFrom(*bmpDecoder.getSurface());
		bmpDecoder.destroy();
		file.close();
	}

	Graphics::ManagedSurface docSurface;
	docSurface.create(bgFrame->w, bgFrame->h, bgFrame->format);
	docSurface.blitFrom(*bgFrame);

	_fontManager->setCurrentFont(0);
	_fontManager->setTransparentBackground(true);
	_fontManager->setLineHeight(14);
	_fontManager->setSpaceWidth(0);
	_fontManager->setCharSpacing(1);
	_fontManager->setSurface(&docSurface);

	MouseBoxes boxes(8);
	boxes.setupBox(0, 104, 335, 177, 408);
	boxes.setupBox(1, 46, 122, 119, 195);
	boxes.setupBox(2, 140, 40, 213, 113);
	boxes.setupBox(3, 247, 402, 320, 475);
	boxes.setupBox(4, 32, 240, 105, 313);
	boxes.setupBox(5, 285, 5, 358, 78);
	// No box for 6
	boxes.setupBox(7, 0, 480 - _sprites->getCursor(225).getHeight(), 640, 480);

	_engine->setupPalette(imageDecoder->getPalette(), 0,
	                      imageDecoder->getPaletteColorCount());

	_engine->setCursor(181);
	_engine->showMouse(true);

	bool redraw = true;
	uint hoveredBox = uint(-1);
	uint selectedBox = uint(-1);

	while (selectedBox == uint(-1)) {
		if (redraw) {
			// Draw without worrying of already modified areas, that's handled when recomputing hoveredBox
			for (uint i = 0; i < ARRAYSIZE(categories); i++) {
				uint foreColor = 243;
				if (i == hoveredBox) {
					foreColor = 241;
					if (categories[hoveredBox].highlightedImg.getPixels() != nullptr) {
						docSurface.transBlitFrom(categories[i].highlightedImg, categories[i].imgPos - Common::Point(36,
						                         65));
					}
				}
				_fontManager->setForeColor(foreColor);
				if (categories[i].title) {
					uint x = categories[i].linesPos.right - _fontManager->getStrWidth(*categories[i].title);
					uint y = categories[i].linesPos.bottom - _fontManager->getFontMaxHeight() - 5;
					_fontManager->displayStr(x, y, *categories[i].title);

					// Draw line to text
					docSurface.vLine(categories[i].linesPos.left, categories[i].linesPos.top,
					                 categories[i].linesPos.bottom, foreColor);
					docSurface.hLine(categories[i].linesPos.left, categories[i].linesPos.bottom,
					                 categories[i].linesPos.right - 1, foreColor); // minus 1 because hLine draws inclusive
				}
			}
			docSurface.transBlitFrom(_sprites->getSurface(225), boxes.getBoxOrigin(7),
			                         _sprites->getKeyColor(225));

			g_system->copyRectToScreen(docSurface.getPixels(), docSurface.pitch, 0, 0, docSurface.w,
			                           docSurface.h);

			redraw = false;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);

		if (_engine->pollEvents()) {
			if (!_engine->getCurrentMouseButton()) {
				// Don't change highlighted icon when clicking
				Common::Point mouse = _engine->getMousePos();
				bool foundBox = false;
				for (uint i = 0; i < ARRAYSIZE(categories); i++) {
					if (boxes.hitTest(i, mouse)) {
						foundBox = true;
						if (i != hoveredBox) {
							hoveredBox = i;
							redraw = true;
						}
					}
				}
				if (!foundBox && hoveredBox != uint(-1)) {
					if (categories[hoveredBox].highlightedImg.getPixels() != nullptr) {
						// Restore original icon
						const Common::Point &imgPos = categories[hoveredBox].imgPos;
						docSurface.blitFrom(*bgFrame, Common::Rect(
						                        imgPos.x - 36, imgPos.y - 65, imgPos.x + 37, imgPos.y + 8),
						                    Common::Point(imgPos.x - 36, imgPos.y - 65));
					}
					hoveredBox = uint(-1);
					redraw = true;
				}
			}
			if (_engine->getDragStatus() == kDragStatus_Finished) {
				if (hoveredBox != uint(-1)) {
					selectedBox = hoveredBox;
				}
			}
			if (_engine->checkKeysPressed(1, Common::KEYCODE_ESCAPE)) {
				selectedBox = 7;
			}
		}
		if (_engine->shouldAbort()) {
			selectedBox = 7;
		}
	}

	_engine->showMouse(false);

	for (uint i = 0; i < ARRAYSIZE(categories); i++) {
		categories[i].highlightedImg.free();
	}

	delete imageDecoder;

	if (selectedBox == 7) {
		return "";
	} else {
		return categories[selectedBox].record;
	}
}

Common::String Versailles_Documentation::docAreaHandleTimeline() {
	Image::ImageDecoder *imageDecoder = _engine->loadHLZ(_engine->getFilePath(kFileTypeDocBg, "chrono1.HLZ"));
	if (!imageDecoder) {
		return "";
	}
	const Graphics::Surface *bgFrame = imageDecoder->getSurface();

	Graphics::ManagedSurface docSurface;
	docSurface.create(bgFrame->w, bgFrame->h, bgFrame->format);
	docSurface.blitFrom(*bgFrame);

	_fontManager->setCurrentFont(1);
	_fontManager->setTransparentBackground(true);
	_fontManager->setLineHeight(14);
	_fontManager->setSpaceWidth(0);
	_fontManager->setCharSpacing(1);
	_fontManager->setSurface(&docSurface);

	_engine->setupPalette(imageDecoder->getPalette(), 0,
	                      imageDecoder->getPaletteColorCount());

	_fontManager->displayStr(78, 10, (*_messages)[73]);
	docSurface.hLine(0, 39, 171, 241); // minus 1 because hLine draws inclusive

	_fontManager->setCurrentFont(0);

	MouseBoxes boxes(ARRAYSIZE(kTimelineEntries) + 1);
	for (uint box_id = 0; box_id < ARRAYSIZE(kTimelineEntries); box_id++) {
		boxes.setupBox(box_id, kTimelineEntries[box_id].x, kTimelineEntries[box_id].y,
		               kTimelineEntries[box_id].x + 30, kTimelineEntries[box_id].y + 20);
	}
	const uint leaveBoxId = ARRAYSIZE(kTimelineEntries);
	boxes.setupBox(leaveBoxId, 639 - _sprites->getCursor(105).getWidth(),
	               479 - _sprites->getCursor(105).getHeight(), 640, 480);

	_engine->setCursor(181);
	_engine->showMouse(true);

	bool redraw = true;
	uint hoveredBox = uint(-1);
	uint selectedBox = uint(-1);

	while (selectedBox == uint(-1)) {
		if (redraw) {
			// Draw without worrying of already modified areas, that's handled when recomputing hoveredBox
			for (uint i = 0; i < ARRAYSIZE(kTimelineEntries); i++) {
				_fontManager->setForeColor(i == hoveredBox ? 241 : 243);
				_fontManager->displayStr(kTimelineEntries[i].x, kTimelineEntries[i].y, kTimelineEntries[i].year);
			}
			docSurface.transBlitFrom(_sprites->getSurface(105), boxes.getBoxOrigin(leaveBoxId),
			                         _sprites->getKeyColor(105));

			g_system->copyRectToScreen(docSurface.getPixels(), docSurface.pitch, 0, 0,
			                           docSurface.w, docSurface.h);
			redraw = false;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);

		if (_engine->pollEvents()) {
			Common::Point mouse = _engine->getMousePos();
			if (!_engine->getCurrentMouseButton()) {
				// Don't change highlighted date when clicking
				bool foundBox = false;
				for (uint i = 0; i < ARRAYSIZE(kTimelineEntries); i++) {
					if (boxes.hitTest(i, mouse)) {
						foundBox = true;
						if (i != hoveredBox) {
							hoveredBox = i;
							redraw = true;
						}
					}
				}
				if (!foundBox && hoveredBox != uint(-1)) {
					hoveredBox = uint(-1);
					redraw = true;
				}
			}
			if (_engine->getDragStatus() == kDragStatus_Finished) {
				if (hoveredBox != uint(-1)) {
					selectedBox = hoveredBox;
				}
				if (boxes.hitTest(leaveBoxId, mouse)) {
					selectedBox = leaveBoxId;
				}
			}
			if (_engine->checkKeysPressed(1, Common::KEYCODE_ESCAPE)) {
				selectedBox = leaveBoxId;
			}
		}
		if (_engine->shouldAbort()) {
			selectedBox = leaveBoxId;
		}
	}

	_engine->showMouse(false);

	delete imageDecoder;

	if (selectedBox == leaveBoxId) {
		return "";
	} else {
		Common::String ret = "VT";
		ret += kTimelineEntries[selectedBox].year;
		return ret;
	}
}

uint Versailles_Documentation::docAreaHandleRecords(const Common::String &record) {
	uint action = uint(-1);

	_currentRecord = record;
	_visitTrace.clear();

	Graphics::ManagedSurface docSurface;
	Common::String nextRecord;
	MouseBoxes boxes(10 + ARRAYSIZE(kTimelineEntries));

	while (true) {
		if (action == uint(-1)) {
			_currentRecord.toUppercase();

			//debug("Displaying %s", _currentRecord.c_str());
			docAreaPrepareNavigation();
			docAreaPrepareRecord(docSurface, boxes);
			action = docAreaHandleRecord(docSurface, boxes, nextRecord);
		}

		switch (action) {
		case 0:
			action = uint(-1);
			// Back
			if (!_visitTrace.empty()) {
				_currentRecord = _visitTrace.back();
				_visitTrace.pop_back();
				break;
			}
		// No previous record, like a back to root
		// fall through
		case 1:
			// Back to root
			return 1;
		case 2:
			action = uint(-1);
			// Follow hyperlink keeping trace
			_visitTrace.push_back(_currentRecord);
			_currentRecord = nextRecord;
			break;
		case 3:
			action = uint(-1);
			// Follow hyperlink losing trace
			_visitTrace.clear();
			_currentRecord = nextRecord;
			break;
		case 6:
			// Quit
			return 2;
		case 7:
			action = uint(-1);
			// General map
			_visitTrace.clear();
			nextRecord = docAreaHandleGeneralMap();
			if (nextRecord == "") {
				// Go back to current record
				break;
			} else if (nextRecord != "VS00") {
				_currentRecord = nextRecord;
				break;
			}
		// castle has been selected, display its map
		// fall through
		case 8:
			action = uint(-1);
			// Castle map
			_visitTrace.clear();
			nextRecord = docAreaHandleCastleMap();
			if (nextRecord == "") {
				// Go back to current record
			} else if (nextRecord != "planG") {
				_currentRecord = nextRecord;
			} else {
				// We can't go up to previous case, so let's do a round
				action = 7;
			}
			break;
		case 9:
			action = uint(-1);
			// Start of category
			_currentRecord = _categoryStartRecord;
			break;
		default:
			error("Invalid case %d when displaying doc record", action);
		}
	}
	error("shouldn't be there");
}

void Versailles_Documentation::docAreaPrepareNavigation() {
	_currentInTimeline = false;
	_currentMapLayout = false;
	_currentHasMap = false;
	_currentLinks.clear();

	if (_currentRecord.hasPrefix("VA")) {
		_categoryStartRecord = "VA00";
		_categoryEndRecord = "VA15";
		_categoryTitle = (*_messages)[68];
	} else if (_currentRecord.hasPrefix("VC")) {
		_categoryStartRecord = "VC00";
		_categoryEndRecord = "VC26";
		_categoryTitle = (*_messages)[70];
	} else if (_currentRecord.hasPrefix("VR")) {
		_categoryStartRecord = "VR00";
		_categoryEndRecord = "VR14";
		_categoryTitle = (*_messages)[69];
	} else if (_currentRecord.hasPrefix("VS")) {
		_categoryStartRecord = "VS00";
		_categoryEndRecord = "VS37";
		_categoryTitle = (*_messages)[72];
		uint id = atoi(_currentRecord.c_str() + 2);
		if (id >= 16 && id <= 40) {
			_currentMapLayout = true;
		}
		if ((id >= 16 && id <= 31) ||
		        (id >= 35 && id <= 39)) {
			_currentHasMap = true;
		}
	} else if (_currentRecord.hasPrefix("VT")) {
		_categoryStartRecord = "VT00";
		_categoryEndRecord = "VT1715";
		_categoryTitle = (*_messages)[73];
		_currentInTimeline = true;
	} else if (_currentRecord.hasPrefix("VV")) {
		_categoryStartRecord = "VV00";
		_categoryEndRecord = "VV15";
		_categoryTitle = (*_messages)[71];
	}
	getLinks(_currentRecord, _currentLinks);
}

void Versailles_Documentation::docAreaPrepareRecord(Graphics::ManagedSurface &surface,
		MouseBoxes &boxes) {
	boxes.reset();

	setupRecordBoxes(true, boxes);

	Common::String title, subtitle, caption;
	Common::StringArray hyperlinks;
	Common::String text = getRecordData(_currentRecord, title, subtitle, caption, hyperlinks);

	drawRecordData(surface, text, title, subtitle, caption);

	if (_currentInTimeline) {
		surface.hLine(0, 39, 171, 241); // minus 1 because hLine draws inclusive
		_fontManager->setCurrentFont(0);
		_fontManager->setTransparentBackground(true);
		_fontManager->setLineHeight(14);
		_fontManager->setSpaceWidth(0);
		_fontManager->setCharSpacing(1);
		_fontManager->setSurface(&surface);
		_fontManager->setForeColor(243);
		for (uint box_id = 10; box_id < ARRAYSIZE(kTimelineEntries) + 10; box_id++) {
			boxes.display(box_id, *_fontManager);
		}
	}

	drawRecordBoxes(surface, true, boxes);
}

uint Versailles_Documentation::docAreaHandleRecord(Graphics::ManagedSurface &surface,
		MouseBoxes &boxes, Common::String &nextRecord) {
	// Hovering is only handled for timeline entries
	_engine->setCursor(181);
	_engine->showMouse(true);

	bool first = true;
	bool redraw = true;
	uint hoveredBox = uint(-1);
	uint action = uint(-1);

	while (action == uint(-1)) {
		if (redraw) {
			g_system->copyRectToScreen(surface.getPixels(), surface.pitch, 0, 0, surface.w, surface.h);
			redraw = false;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);

		if (_engine->pollEvents() || first) {
			first = false;
			if (_engine->shouldAbort()) {
				// Fake the quit
				action = 6;
			}
			Common::Point mouse = _engine->getMousePos();
			if (_currentInTimeline) {
				bool foundBox = false;
				for (uint i = 10; i < 10 + ARRAYSIZE(kTimelineEntries); i++) {
					if (boxes.hitTest(i, mouse)) {
						foundBox = true;
						if (i != hoveredBox) {
							_fontManager->setCurrentFont(0);
							_fontManager->setTransparentBackground(true);
							_fontManager->setSurface(&surface);
							if (hoveredBox != uint(-1)) {
								// Restore the previous entry hovered
								_fontManager->setForeColor(243);
								boxes.display(hoveredBox, *_fontManager);
							}
							hoveredBox = i;
							_fontManager->setForeColor(241);
							boxes.display(hoveredBox, *_fontManager);
							redraw = true;
						}
					}
				}
				if (!foundBox && hoveredBox != uint(-1)) {
					// Restore the previous entry hovered
					_fontManager->setForeColor(243);
					boxes.display(hoveredBox, *_fontManager);
					hoveredBox = uint(-1);
					redraw = true;
				}
			} else if (_currentHasMap) { // Mutually exclusive with timeline
				// No clash is possible for hoveredBox between timeline and map
				if (boxes.hitTest(8, mouse)) {
					if (hoveredBox != 8) {
						_engine->setCursor(145);
						hoveredBox = 8;
					}
				} else {
					if (hoveredBox == 8) {
						_engine->setCursor(181);
						hoveredBox = uint(-1);
					}
				}
			}
			if (_engine->getDragStatus() == kDragStatus_Pressed) {
				if (boxes.hitTest(2, mouse) && _currentLinks.size()) {
					Common::StringArray items;
					for (Common::Array<LinkInfo>::const_iterator it = _currentLinks.begin(); it != _currentLinks.end();
					        it++) {
						items.push_back(it->title);
					}
					Common::Rect iconRect = boxes.getBoxRect(2);
					uint selectedItem = handlePopupMenu(surface, Common::Point(iconRect.right, iconRect.top),
					                                    true, 20, items);
					if (selectedItem != uint(-1)) {
						nextRecord = _currentLinks[selectedItem].record;
						action = 2;
					}
				} else if (boxes.hitTest(3, mouse)) {
					Common::StringArray items;
					for (Common::Array<LinkInfo>::const_iterator it = _allLinks.begin(); it != _allLinks.end(); it++) {
						items.push_back(it->title);
					}
					Common::Rect iconRect = boxes.getBoxRect(3);
					uint selectedItem = handlePopupMenu(surface, Common::Point(iconRect.right, iconRect.top),
					                                    true, 20, items);
					if (selectedItem != uint(-1)) {
						nextRecord = _allLinks[selectedItem].record;
						action = 3;
					}
				}
			} else if (_engine->getDragStatus() == kDragStatus_Finished) {
				if (boxes.hitTest(0, mouse)) {
					// Back in history
					action = 0;
				} else if (boxes.hitTest(1, mouse)) {
					// Handle summary menu
					Common::StringArray items;
					items.push_back((*_messages)[61]);
					items.push_back((*_messages)[62]);
					uint selectedItem = handlePopupMenu(surface, boxes.getBoxOrigin(1), false, 20, items);
					if (selectedItem == 0) {
						action = 1;
					} else if (selectedItem == 1) {
						action = 7;
					}
				} else if (boxes.hitTest(4, mouse)) {
					// Next
					action = 4;
				} else if (boxes.hitTest(5, mouse)) {
					// Previous
					action = 5;
				} else if (boxes.hitTest(6, mouse)) {
					// Handle quit menu
					Common::StringArray items;
					items.push_back((*_messages)[60]);
					uint selectedItem = handlePopupMenu(surface, boxes.getBoxOrigin(6), false, 20, items);
					if (selectedItem == 0) {
						action = 6;
					}
				} else if (_currentHasMap && boxes.hitTest(8, mouse)) {
					// Map
					action = 8;
				} else if (boxes.hitTest(9, mouse)) {
					// Category name
					action = 9;
				} else if (_currentInTimeline && hoveredBox != uint(-1)) {
					// Clicked on a timeline entry
					nextRecord = "VT";
					nextRecord += kTimelineEntries[hoveredBox - 10].year;
					// Fake a global jump
					action = 3;
				}
			}
			if (action == 4 || action == 5) {
				if (action == 4 && _currentRecord == _categoryEndRecord) {
					action = uint(-1);
					continue;
				}
				if (action == 5 && _currentRecord == _categoryStartRecord) {
					action = uint(-1);
					continue;
				}
				Common::HashMap<Common::String, RecordInfo>::iterator hmIt = _records.find(_currentRecord);
				if (hmIt == _records.end()) {
					// Shouldn't happen
					action = uint(-1);
					continue;
				}
				uint recordId = hmIt->_value.id;
				if (action == 4) {
					recordId++;
				} else if (action == 5) {
					recordId--;
				}
				assert(recordId < _recordsOrdered.size());
				nextRecord = _recordsOrdered[recordId];
				// Fake a global jump
				action = 3;
			}
		}
	}

	_engine->showMouse(false);
	_engine->setCursor(181);
	return action;
}

Common::String Versailles_Documentation::docAreaHandleGeneralMap() {
	struct Area {
		Common::Rect areaPos;
		const char *record;
		const char *bmp;
		uint messageId;
		const Common::String *message;
		Common::Point messagePos;
		Graphics::Surface highlightedImg;

		Area(const Common::Point &areaPos_, const char *bmp_, uint messageId_,
		     const char *record_ = nullptr) :
			areaPos(areaPos_.x, areaPos_.y, areaPos_.x, areaPos_.y), record(record_), bmp(bmp_),
			messageId(messageId_), message(nullptr) { }
		Area(const Common::Rect &areaPos_, uint messageId_, const char *record_ = nullptr) :
			areaPos(areaPos_), record(record_), bmp(nullptr), messageId(messageId_), message(nullptr) { }
	} areas[] = {
		Area(Common::Point(174, 181), "APL.bmp", 74),
		Area(Common::Point(422, 129), "CHAT.bmp", 75, "VS00"),
		Area(Common::Point(193, 204), "COLN.bmp", 76, "VS02"),
		Area(Common::Point(327, 269), "LABY.bmp", 77, "VS33"),
		Area(Common::Point(327, 170), "LATN.bmp", 78),
		Area(Common::Point(396, 271), "ORG.bmp", 79, "VS32"),
		Area(Common::Point(385, 203), "PART.bmp", 80, "VS06"),
		Area(Common::Point(212, 193), "TAP.bmp", 81),
		Area(Common::Rect(0, 194, 154, 211), 86, "VS09"),
		Area(Common::Rect(396, 229, 450, 268), 87),
		Area(Common::Rect(394, 133, 450, 177), 88),
		Area(Common::Rect(489, 376, 592, 479), 89, "VS07"),
		Area(Common::Rect(327, 233, 386, 266), 90),
		Area(Common::Rect(395, 18, 451, 60), 91),
		Area(Common::Rect(383, 381, 477, 479), 92)
	};

	_fontManager->setCurrentFont(0);
	_fontManager->setTransparentBackground(true);
	_fontManager->setLineHeight(14);
	_fontManager->setSpaceWidth(0);
	_fontManager->setCharSpacing(1);

	MouseBoxes boxes(ARRAYSIZE(areas) + 1);

	Image::BitmapDecoder bmpDecoder;
	Common::File file;

	Image::ImageDecoder *imageDecoder = _engine->loadHLZ(_engine->getFilePath(kFileTypeDocBg, "PLANGR.HLZ"));
	if (!imageDecoder) {
		return "";
	}
	const Graphics::Surface *bgFrame = imageDecoder->getSurface();

	for (uint i = 0; i < ARRAYSIZE(areas); i++) {
		if (areas[i].bmp) {
			if (!file.open(_engine->getFilePath(kFileTypeSpriteBmp, areas[i].bmp))) {
				error("Failed to open BMP file: %s", areas[i].bmp);
			}
			if (!bmpDecoder.loadStream(file)) {
				error("Failed to load BMP file: %s", areas[i].bmp);
			}
			areas[i].highlightedImg.copyFrom(*bmpDecoder.getSurface());
			bmpDecoder.destroy();
			file.close();
			areas[i].areaPos.setWidth(areas[i].highlightedImg.w);
			areas[i].areaPos.setHeight(areas[i].highlightedImg.h);
		}
		areas[i].message = &(*_messages)[areas[i].messageId];
		uint lineWidth = _fontManager->getStrWidth(*areas[i].message);
		areas[i].messagePos.x = (areas[i].areaPos.left + areas[i].areaPos.right) / 2 - lineWidth / 2;
		areas[i].messagePos.y = areas[i].areaPos.top - 40;
		if (areas[i].messagePos.x < 8) {
			areas[i].messagePos.x = 8;
		} else if (areas[i].messagePos.x + lineWidth > 627) {
			areas[i].messagePos.x = 627 - lineWidth;
		}
		if (areas[i].messagePos.y < 5) {
			areas[i].messagePos.y = 5;
		}
		const Common::Rect &areaPos = areas[i].areaPos;
		boxes.setupBox(i, areaPos.left, areaPos.top, areaPos.right, areaPos.bottom);
	}
	boxes.setupBox(ARRAYSIZE(areas), 639 - _sprites->getCursor(105).getWidth(),
	               479 - _sprites->getCursor(105).getHeight(), 640, 480);

	Graphics::ManagedSurface mapSurface;
	mapSurface.create(bgFrame->w, bgFrame->h, bgFrame->format);
	mapSurface.blitFrom(*bgFrame);

	_fontManager->setSurface(&mapSurface);

	_engine->setupPalette(imageDecoder->getPalette(), 0,
	                      imageDecoder->getPaletteColorCount());

	_engine->setCursor(181);
	_engine->showMouse(true);

	bool redraw = true;
	uint hoveredBox = uint(-1);
	uint selectedBox = uint(-1);

	while (selectedBox == uint(-1)) {
		if (redraw) {
			// Draw without worrying of already modified areas, that's handled when recomputing hoveredBox
			if (hoveredBox != uint(-1)) {
				if (areas[hoveredBox].highlightedImg.getPixels() != nullptr) {
					mapSurface.transBlitFrom(areas[hoveredBox].highlightedImg,
					                         Common::Point(areas[hoveredBox].areaPos.left, areas[hoveredBox].areaPos.top));
				} else {
					uint middleX = (areas[hoveredBox].areaPos.left + areas[hoveredBox].areaPos.right) / 2;
					uint middleY = (areas[hoveredBox].areaPos.top + areas[hoveredBox].areaPos.bottom) / 2;
					uint spriteX = middleX - _sprites->getCursor(163).getWidth() / 2;
					uint spriteY = middleY - _sprites->getCursor(163).getHeight() / 2;
					mapSurface.transBlitFrom(_sprites->getSurface(163), Common::Point(spriteX, spriteY),
					                         _sprites->getKeyColor(163));
				}
				_fontManager->setForeColor(areas[hoveredBox].record == nullptr ? 243 : 240);
				Graphics::Surface subSurface = mapSurface.getSubArea(Common::Rect(areas[hoveredBox].messagePos.x -
				                               3,
				                               areas[hoveredBox].messagePos.y - 3,
				                               areas[hoveredBox].messagePos.x + _fontManager->getStrWidth(*areas[hoveredBox].message) + 3,
				                               areas[hoveredBox].messagePos.y + _fontManager->getFontMaxHeight() + 8));
				_engine->makeTranslucent(subSurface, subSurface);
				_fontManager->displayStr(areas[hoveredBox].messagePos.x, areas[hoveredBox].messagePos.y,
				                         *areas[hoveredBox].message);
			}
			mapSurface.transBlitFrom(_sprites->getSurface(105), boxes.getBoxOrigin(ARRAYSIZE(areas)),
			                         _sprites->getKeyColor(105));
			/*
			// For debugging only
			for(uint i = 0; i < ARRAYSIZE(areas); i++) {
			    mapSurface.frameRect(areas[i].areaPos, 0);
			}
			*/

			g_system->copyRectToScreen(mapSurface.getPixels(), mapSurface.pitch, 0, 0, mapSurface.w,
			                           mapSurface.h);

			redraw = false;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);

		if (_engine->pollEvents()) {
			Common::Point mouse = _engine->getMousePos();
			if (!_engine->getCurrentMouseButton()) {
				// Don't change highlighted icon when clicking
				bool foundBox = false;
				uint oldHoveredBox = hoveredBox;
				for (uint i = 0; i < ARRAYSIZE(areas); i++) {
					if (boxes.hitTest(i, mouse)) {
						if (i != hoveredBox) {
							hoveredBox = i;
							redraw = true;
						}
						foundBox = true;
						break;
					}
				}
				if (!foundBox && hoveredBox != uint(-1)) {
					hoveredBox = uint(-1);
					redraw = true;
				}
				if (hoveredBox != oldHoveredBox && oldHoveredBox != uint(-1)) {
					// Restore original area
					mapSurface.blitFrom(*bgFrame, areas[oldHoveredBox].areaPos,
					                    Common::Point(areas[oldHoveredBox].areaPos.left, areas[oldHoveredBox].areaPos.top));
					Common::Rect textRect(areas[oldHoveredBox].messagePos.x - 3,
					                      areas[oldHoveredBox].messagePos.y - 3,
					                      areas[oldHoveredBox].messagePos.x + _fontManager->getStrWidth(*areas[oldHoveredBox].message) + 3,
					                      areas[oldHoveredBox].messagePos.y + _fontManager->getFontMaxHeight() + 8);
					mapSurface.blitFrom(*bgFrame, textRect,
					                    Common::Point(textRect.left, textRect.top));
				}
			}
			if (_engine->getDragStatus() == kDragStatus_Finished) {
				if (hoveredBox != uint(-1) && areas[hoveredBox].record) {
					selectedBox = hoveredBox;
				} else if (boxes.hitTest(ARRAYSIZE(areas), mouse)) {
					selectedBox = ARRAYSIZE(areas);
				}
			}
			if (_engine->checkKeysPressed(1, Common::KEYCODE_ESCAPE)) {
				selectedBox = ARRAYSIZE(areas);
			}
			if (_engine->shouldAbort()) {
				selectedBox = ARRAYSIZE(areas);
			}
		}
	}

	_engine->showMouse(false);

	for (uint i = 0; i < ARRAYSIZE(areas); i++) {
		areas[i].highlightedImg.free();
	}

	delete imageDecoder;

	if (selectedBox == ARRAYSIZE(areas)) {
		return "";
	} else {
		return areas[selectedBox].record;
	}
}

Common::String Versailles_Documentation::docAreaHandleCastleMap() {
	struct Area {
		Common::Rect areaPos;
		bool fillArea;
		const char *record;
		uint messageId;
		Common::String message;
		Common::Point messagePos;
		Common::Rect areaPos1;
		Common::Rect areaPos2;

		Area(const Common::Rect &areaPos_, const char *record_, bool fillArea_ = true,
		     uint messageId_ = uint(-1)) :
			areaPos(areaPos_), record(record_), fillArea(fillArea_), messageId(messageId_) { }
		Area(const Common::Rect &areaPos_, const Common::Rect &areaPos1_,
		     const Common::Rect &areaPos2_, const char *record_, bool fillArea_ = true,
		     uint messageId_ = uint(-1)) :
			areaPos(areaPos_), areaPos1(areaPos1_), areaPos2(areaPos2_),
			record(record_), fillArea(fillArea_), messageId(messageId_) { }
	} areas[] = {
		/* 0 */
		Area(Common::Rect(212, 134, 239, 164), "VS16"),
		Area(Common::Rect(74, 160, 89, 173), "VS24"),
		Area(Common::Rect(93, 160, 109, 173), "VS25"),
		Area(Common::Rect(130, 160, 154, 173), "VS26"),
		Area(Common::Rect(158, 160, 171, 173), "VS27"),
		Area(Common::Rect(199, 160, 209, 171), "VS28"),
		Area(Common::Rect(74, 177, 89, 291), "VS31"),
		Area(Common::Rect(158, 178, 195, 193), "VS30"),
		Area(Common::Rect(199, 175, 209, 188), "VS29"),
		Area(Common::Rect(112, 220, 160, 249), "VS35"),
		/* 10 */
		Area(Common::Rect(93, 227, 106, 240), "VS23"),
		Area(Common::Rect(93, 244, 106, 257), "VS22"),
		Area(Common::Rect(93, 261, 106, 274), "VS20"),
		Area(Common::Rect(110, 255, 126, 269), "VS19"),
		Area(Common::Rect(133, 255, 155, 271), "VS18"),
		Area(Common::Rect(93, 285, 99, 295), "VS21"),
		Area(Common::Rect(152, 279, 173, 288), "VS17"),
		Area(Common::Rect(336, 113, 359, 136), Common::Rect(359, 116, 448, 134), Common::Rect(449, 113, 473, 136), "VS36"),
		Area(Common::Rect(336, 328, 359, 351), Common::Rect(359, 331, 448, 348), Common::Rect(449, 328, 473, 351), "VS36"),
		Area(Common::Rect(563, 0, 624, 139), "planG", false, 82),
		/* 20 */
		Area(Common::Rect(563, 300, 624, 462), "planG", false, 83),
		Area(Common::Rect(0, 0, 205, 152), "planG", false, 84),
		Area(Common::Rect(0, 318, 205, 465), "planG", false, 84),
		Area(Common::Rect(160, 210, 329, 267), "VS40", false),
		Area(Common::Rect(330, 158, 561, 315), "planG", false, 85),
	};

	_fontManager->setCurrentFont(0);
	_fontManager->setTransparentBackground(true);
	_fontManager->setLineHeight(14);
	_fontManager->setSpaceWidth(0);
	_fontManager->setCharSpacing(1);

	MouseBoxes boxes(ARRAYSIZE(areas) + 1);

	for (uint i = 0; i < ARRAYSIZE(areas); i++) {
		if (areas[i].messageId != uint(-1)) {
			areas[i].message = (*_messages)[areas[i].messageId];
		} else {
			areas[i].message = getRecordTitle(areas[i].record);
		}
		uint lineWidth = _fontManager->getStrWidth(areas[i].message);
		uint right;
		if (areas[i].areaPos2.right) {
			right = areas[i].areaPos2.right;
		} else {
			right = areas[i].areaPos.right;
		}
		areas[i].messagePos.x = (areas[i].areaPos.left + right) / 2 - lineWidth / 2;
		if (areas[i].fillArea) {
			areas[i].messagePos.y = areas[i].areaPos.top - 30;
		} else {
			areas[i].messagePos.y = (areas[i].areaPos.top + areas[i].areaPos.bottom) / 2 - 50;
		}
		if (areas[i].messagePos.x < 5) {
			areas[i].messagePos.x = 5;
		} else if (areas[i].messagePos.x + lineWidth > 630) {
			areas[i].messagePos.x = 630 - lineWidth;
		}
		if (areas[i].messagePos.y < 2) {
			areas[i].messagePos.y = 2;
		}
		Common::Rect areaPos = areas[i].areaPos;
		if (areas[i].areaPos2.right) {
			areaPos.right = areas[i].areaPos2.right;
			areaPos.bottom = areas[i].areaPos2.bottom;
		}
		boxes.setupBox(i, areaPos.left, areaPos.top, areaPos.right, areaPos.bottom);
	}
	boxes.setupBox(ARRAYSIZE(areas), 639 - _sprites->getCursor(105).getWidth(),
	               479 - _sprites->getCursor(105).getHeight(), 640, 480);

	Image::ImageDecoder *imageDecoder = _engine->loadHLZ(_engine->getFilePath(kFileTypeDocBg, "PLAN.HLZ"));
	if (!imageDecoder) {
		return "";
	}
	const Graphics::Surface *bgFrame = imageDecoder->getSurface();

	Graphics::ManagedSurface mapSurface;
	mapSurface.create(bgFrame->w, bgFrame->h, bgFrame->format);
	mapSurface.blitFrom(*bgFrame);

	_fontManager->setSurface(&mapSurface);

	_engine->setupPalette(imageDecoder->getPalette(), 0,
	                      imageDecoder->getPaletteColorCount());

	_engine->setCursor(181);
	_engine->showMouse(true);

	bool redraw = true;
	uint hoveredBox = uint(-1);
	uint selectedBox = uint(-1);

	while (selectedBox == uint(-1)) {
		if (redraw) {
			// Draw without worrying of already modified areas, that's handled when recomputing hoveredBox
			if (hoveredBox != uint(-1)) {
				if (areas[hoveredBox].fillArea) {
					Common::Rect rect(areas[hoveredBox].areaPos);
					rect.bottom += 1; // fillRect needs to fill including the limit
					rect.right += 1;
					mapSurface.fillRect(rect, 243);
					if (areas[hoveredBox].areaPos1.right) {
						rect = Common::Rect(areas[hoveredBox].areaPos1);
						rect.bottom += 1; // fillRect needs to fill including the limit
						rect.right += 1;
						mapSurface.fillRect(rect, 243);
					}
					if (areas[hoveredBox].areaPos2.right) {
						rect = Common::Rect(areas[hoveredBox].areaPos2);
						rect.bottom += 1; // fillRect needs to fill including the limit
						rect.right += 1;
						mapSurface.fillRect(rect, 243);
					}
				} else {
					uint middleX = (areas[hoveredBox].areaPos.left + areas[hoveredBox].areaPos.right) / 2;
					uint middleY = (areas[hoveredBox].areaPos.top + areas[hoveredBox].areaPos.bottom) / 2;
					uint spriteX = middleX - _sprites->getCursor(163).getWidth() / 2;
					uint spriteY = middleY - _sprites->getCursor(163).getHeight() / 2;
					mapSurface.transBlitFrom(_sprites->getSurface(163), Common::Point(spriteX, spriteY),
					                         _sprites->getKeyColor(163));
				}
				Common::Rect textRect(areas[hoveredBox].messagePos.x - 4,
				                      areas[hoveredBox].messagePos.y,
				                      areas[hoveredBox].messagePos.x + _fontManager->getStrWidth(areas[hoveredBox].message) + 5,
				                      areas[hoveredBox].messagePos.y + _fontManager->getFontMaxHeight() + 5);
				mapSurface.fillRect(textRect, 247);
				_fontManager->setForeColor(strcmp(areas[hoveredBox].record, "planG") == 0 ? 243 : 241);
				_fontManager->displayStr(areas[hoveredBox].messagePos.x, areas[hoveredBox].messagePos.y,
				                         areas[hoveredBox].message);
			}
			mapSurface.transBlitFrom(_sprites->getSurface(105), boxes.getBoxOrigin(ARRAYSIZE(areas)),
			                         _sprites->getKeyColor(105));
			/*
			// For debugging only
			for(uint i = 0; i < ARRAYSIZE(areas); i++) {
			    mapSurface.frameRect(areas[i].areaPos, 0);
			    if (areas[i].areaPos1.right) {
			        mapSurface.frameRect(areas[i].areaPos1, 0);
			    }
			    if (areas[i].areaPos2.right) {
			        mapSurface.frameRect(areas[i].areaPos2, 0);
			    }
			}
			*/

			g_system->copyRectToScreen(mapSurface.getPixels(), mapSurface.pitch, 0, 0,
			                           mapSurface.w, mapSurface.h);

			redraw = false;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);

		if (_engine->pollEvents()) {
			Common::Point mouse = _engine->getMousePos();
			if (!_engine->getCurrentMouseButton()) {
				// Don't change highlighted icon when clicking
				bool foundBox = false;
				uint oldHoveredBox = hoveredBox;
				for (uint i = 0; i < ARRAYSIZE(areas); i++) {
					if (boxes.hitTest(i, mouse)) {
						if (i != hoveredBox) {
							hoveredBox = i;
							redraw = true;
						}
						foundBox = true;
						break;
					}
				}
				if (!foundBox && hoveredBox != uint(-1)) {
					hoveredBox = uint(-1);
					redraw = true;
				}
				if (hoveredBox != oldHoveredBox && oldHoveredBox != uint(-1)) {
					// Restore original area
					Common::Rect areaPos = areas[oldHoveredBox].areaPos;
					if (areas[oldHoveredBox].areaPos2.right) {
						areaPos.right = areas[oldHoveredBox].areaPos2.right;
						areaPos.bottom = areas[oldHoveredBox].areaPos2.bottom;
					}
					areaPos.right += 1;
					areaPos.bottom += 1;
					mapSurface.blitFrom(*bgFrame, areaPos,
					                    Common::Point(areaPos.left, areaPos.top));
					Common::Rect textRect(areas[oldHoveredBox].messagePos.x - 4,
					                      areas[oldHoveredBox].messagePos.y,
					                      areas[oldHoveredBox].messagePos.x + _fontManager->getStrWidth(areas[oldHoveredBox].message) + 5,
					                      areas[oldHoveredBox].messagePos.y + _fontManager->getFontMaxHeight() + 5);
					mapSurface.blitFrom(*bgFrame, textRect,
					                    Common::Point(textRect.left, textRect.top));
				}
			}
			if (_engine->getDragStatus() == kDragStatus_Finished) {
				if (hoveredBox != uint(-1) && areas[hoveredBox].record) {
					selectedBox = hoveredBox;
				} else if (boxes.hitTest(ARRAYSIZE(areas), mouse)) {
					selectedBox = ARRAYSIZE(areas);
				}
			}
			if (_engine->checkKeysPressed(1, Common::KEYCODE_ESCAPE)) {
				selectedBox = ARRAYSIZE(areas);
			}
			if (_engine->shouldAbort()) {
				selectedBox = ARRAYSIZE(areas);
			}
		}
	}

	_engine->showMouse(false);

	delete imageDecoder;

	if (selectedBox == ARRAYSIZE(areas)) {
		return "";
	} else {
		return areas[selectedBox].record;
	}
}

void Versailles_Documentation::inGamePrepareRecord(Graphics::ManagedSurface &surface,
		MouseBoxes &boxes) {
	_categoryStartRecord = "";
	_categoryEndRecord = "";
	_categoryTitle = "";
	_currentLinks.clear();
	_currentInTimeline = false;
	_currentMapLayout = false;
	_currentHasMap = false;

	if (_currentRecord.hasPrefix("VS")) {
		uint id = atoi(_currentRecord.c_str() + 2);
		if (id >= 16 && id <= 40) {
			_currentMapLayout = true;
		}
	} else if (_currentRecord.hasPrefix("VT")) {
		error("There shouldn't be the timeline in game");
	}

	boxes.reset();

	setupRecordBoxes(false, boxes);

	Common::String title, subtitle, caption;
	Common::StringArray hyperlinks;
	Common::String text = getRecordData(_currentRecord, title, subtitle, caption, hyperlinks);
	convertHyperlinks(hyperlinks, _currentLinks);

	drawRecordData(surface, text, title, subtitle, caption);
	drawRecordBoxes(surface, false, boxes);
}

uint Versailles_Documentation::inGameHandleRecord(Graphics::ManagedSurface &surface,
		MouseBoxes &boxes, Common::String &nextRecord) {
	_engine->setCursor(181);
	_engine->showMouse(true);

	uint action = uint(-1);

	g_system->copyRectToScreen(surface.getPixels(), surface.pitch, 0, 0, surface.w, surface.h);

	while (action == uint(-1)) {
		g_system->updateScreen();
		g_system->delayMillis(10);

		if (_engine->pollEvents()) {
			if (_engine->shouldAbort()) {
				// Fake the quit
				action = 1;
			}
			Common::Point mouse = _engine->getMousePos();
			if (_engine->getDragStatus() == kDragStatus_Pressed) {
				if (boxes.hitTest(2, mouse) && _currentLinks.size()) {
					Common::StringArray items;
					for (Common::Array<LinkInfo>::const_iterator it = _currentLinks.begin(); it != _currentLinks.end();
					        it++) {
						items.push_back(it->title);
					}
					Common::Rect iconRect = boxes.getBoxRect(2);
					uint selectedItem = handlePopupMenu(surface, Common::Point(iconRect.right, iconRect.top),
					                                    true, 20, items);
					if (selectedItem != uint(-1)) {
						nextRecord = _currentLinks[selectedItem].record;
						action = 2;
					}
				}
			} else if (_engine->getDragStatus() == kDragStatus_Finished) {
				if (boxes.hitTest(0, mouse)) {
					// Back in history
					action = 0;
				} else if (boxes.hitTest(1, mouse)) {
					// Quit
					action = 1;
				}
			}
		}
	}

	_engine->showMouse(false);
	_engine->setCursor(181);
	return action;
}

void Versailles_Documentation::drawRecordData(Graphics::ManagedSurface &surface,
		const Common::String &text, const Common::String &title,
		const Common::String &subtitle, const Common::String &caption) {
	unsigned char foreColor = 247;
	Common::String background;
	Common::Rect blockTitle;
	Common::Rect blockHLine;
	Common::Rect blockSubTitle;
	Common::Rect blockCaption;
	Common::Rect blockContent1;
	Common::Rect blockContent2;

	if (_currentMapLayout) {
		blockTitle = Common::Rect(30, 8, 361, 38);
		blockHLine = Common::Rect(60, 35, 286, 35);
		blockSubTitle = Common::Rect(60, 40, 361, 70);
		blockCaption = Common::Rect(378, 293, 630, 344);
		blockContent1 = Common::Rect(60, 60, 272, 295);
		blockContent2 = Common::Rect(60, 295, 383, 437);
	} else if (_currentInTimeline) {
		blockTitle = Common::Rect(78, 10, 170, 33);
		//blockHLine = Common::Rect();
		blockSubTitle = Common::Rect(60, 40, 361, 70);
		blockCaption = Common::Rect(378, 293, 630, 344);
		blockContent1 = Common::Rect(47, 70, 420, 306);
		blockContent2 = Common::Rect(174, 306, 414, 411);
	} else if (_currentRecord == "VC02" ||
	           _currentRecord == "VC03" ||
	           _currentRecord == "VV01") {
		blockTitle = Common::Rect(30, 8, 361, 38);
		blockHLine = Common::Rect(60, 35, 378, 35);
		blockSubTitle = Common::Rect(60, 40, 361, 70);
		blockCaption = Common::Rect(378, 293, 630, 360);
		blockContent1 = Common::Rect(60, 80, 351, 355);
		blockContent2 = Common::Rect(60, 355, 605, 437);
	} else if (_currentRecord == "VV13" ||
	           _currentRecord == "VV08") {
		blockTitle = Common::Rect(30, 8, 361, 38);
		blockHLine = Common::Rect(60, 35, 286, 35);
		blockSubTitle = Common::Rect(60, 40, 361, 70);
		blockCaption = Common::Rect(378, 422, 630, 480);
		blockContent1 = Common::Rect(60, 60, 378, 285);
		blockContent2 = Common::Rect(60, 285, 378, 437);
	} else {
		blockTitle = Common::Rect(30, 8, 361, 38);
		blockHLine = Common::Rect(60, 35, 378, 35);
		blockSubTitle = Common::Rect(60, 40, 361, 70);
		blockCaption = Common::Rect(378, 293, 630, 360);
		blockContent1 = Common::Rect(60, 80, 351, 345);
		blockContent2 = Common::Rect(60, 345, 605, 437);
	}
	// Fix of overlapping areas for Chinese and Japanese (as in original binary)
	if ((_engine->getLanguage() == Common::JA_JPN ||
	        _engine->getLanguage() == Common::ZH_TWN) &&
	        !_currentMapLayout) {
		blockContent1.bottom += 30;
		blockContent2.top += 30;
	}

	if (_currentInTimeline) {
		background = "CHRONO1";
		foreColor = 241;
	} else {
		background = _currentRecord;
	}
	Common::Path backgroundPath = _engine->getFilePath(kFileTypeDocBg, background);
	if (!Common::File::exists(backgroundPath)) {
		background = _currentMapLayout ? "pas_fonP.hlz" : "pas_fond.hlz";
		backgroundPath = _engine->getFilePath(kFileTypeDocBg, background);
	}

	Image::ImageDecoder *imageDecoder = _engine->loadHLZ(backgroundPath);
	const Graphics::Surface *bgFrame = imageDecoder->getSurface();

	_engine->setupPalette(imageDecoder->getPalette(), 0,
	                      imageDecoder->getPaletteColorCount());

	surface.create(bgFrame->w, bgFrame->h, bgFrame->format);
	surface.blitFrom(*bgFrame);

	/*Common::String title, subtitle, caption;
	Common::StringArray hyperlinks;

	Common::String text = getRecordData(_currentRecord, title, subtitle, caption, hyperlinks);*/

	uint lineHeight = 21;

	if (_engine->getLanguage() == Common::JA_JPN ||
	        _engine->getLanguage() == Common::KO_KOR ||
	        _engine->getLanguage() == Common::ZH_TWN) {
		_fontManager->setCurrentFont(8);
	} else {
		_fontManager->setCurrentFont(4);
	}
	_fontManager->setTransparentBackground(true);
	_fontManager->setSpaceWidth(1);
	_fontManager->setCharSpacing(1);
	_fontManager->setForeColor(foreColor);
	_fontManager->setSurface(&surface);

	/*
	surface.frameRect(blockContent1, foreColor);
	surface.frameRect(blockContent2, foreColor);
	surface.frameRect(blockTitle, foreColor);
	surface.frameRect(blockSubTitle, foreColor);
	surface.frameRect(blockCaption, foreColor);
	*/

	Graphics::ManagedSurface backupSurface;
	backupSurface.copyFrom(surface);

	// This loop tries to adapt the interline space to make all the text fit in the blocks
	while (true) {
		_fontManager->setLineHeight(lineHeight);
		_fontManager->setupBlock(blockContent1);
		if (!_fontManager->displayBlockText(text)) {
			// All text was drawn
			break;
		}

		// Setup second zone
		blockContent2.top = _fontManager->blockTextLastPos().y + lineHeight;
		_fontManager->setupBlock(blockContent2);

		if (!_fontManager->displayBlockTextContinue()) {
			// All text was drawn
			break;
		}

		// Not all text could be drawn: shrink everything, restore image and do it again
		lineHeight--;
		surface.copyFrom(backupSurface);
	}

	_fontManager->setForeColor(foreColor);
	_fontManager->setCurrentFont(0);
	_fontManager->setTransparentBackground(true);
	_fontManager->setLineHeight(20);
	_fontManager->setCharSpacing(0);
	_fontManager->setSpaceWidth(2);

	//debug("Title: %s", title.c_str());
	_fontManager->setupBlock(blockTitle);
	_fontManager->displayBlockText(title);

	_fontManager->setCurrentFont(6);
	_fontManager->setLineHeight(14);
	_fontManager->setSpaceWidth(1);

	//debug("Subtitle: %s", subtitle.c_str());
	_fontManager->setupBlock(blockSubTitle);
	_fontManager->displayBlockText(subtitle);

	if (!_currentInTimeline) {
		surface.hLine(blockHLine.left, blockHLine.top, blockHLine.right - 1,
		              foreColor); // minus 1 because hLine draws inclusive
	}

	_fontManager->setSpaceWidth(0);

	_fontManager->setupBlock(blockCaption);
	_fontManager->displayBlockText(caption);

	delete imageDecoder;
}

void Versailles_Documentation::setupRecordBoxes(bool inDocArea, MouseBoxes &boxes) {
	// Layout of bar in doc area is Quit | Back |   | Previous | Category | Next |   | Trace | Hyperlinks | All records
	// Layout of bar in game is ==> Trace | Hyperlinks | Quit
	uint allRecordsX = 640 - _sprites->getCursor(19).getWidth();
	uint hyperlinksX = allRecordsX - _sprites->getCursor(242).getWidth() - 10;
	uint traceX = hyperlinksX - _sprites->getCursor(105).getWidth() - 10;

	if (_visitTrace.size()) {
		boxes.setupBox(0, traceX, 480 - _sprites->getCursor(105).getHeight() - 3,
		               traceX + _sprites->getCursor(105).getWidth(), 480);
	}
	if (inDocArea) {
		uint backX = _sprites->getCursor(225).getWidth() + 10; //Right to quit button

		_fontManager->setCurrentFont(0);
		_fontManager->setTransparentBackground(true);
		_fontManager->setSpaceWidth(0);
		_fontManager->setCharSpacing(1);
		uint categoryHalfWidth = _fontManager->getStrWidth(_categoryTitle) / 2;
		unsigned nextX = 320 + categoryHalfWidth + 20;
		unsigned prevX = 320 - categoryHalfWidth - 20 - _sprites->getCursor(76).getWidth();

		boxes.setupBox(3, allRecordsX, 480 - _sprites->getCursor(19).getHeight(),
		               allRecordsX + _sprites->getCursor(19).getWidth(), 480);
		boxes.setupBox(1, backX, 480 - _sprites->getCursor(227).getHeight(),
		               backX + _sprites->getCursor(227).getWidth(), 480);
		boxes.setupBox(9, 320 - categoryHalfWidth - 5, 480 - _sprites->getCursor(227).getHeight(),
		               320 + categoryHalfWidth + 5, 480);
		boxes.setupBox(4, nextX, 476 - _sprites->getCursor(72).getHeight(),
		               nextX + _sprites->getCursor(72).getWidth(), 476);
		boxes.setupBox(5, prevX, 476 - _sprites->getCursor(76).getHeight(),
		               prevX + _sprites->getCursor(76).getWidth(), 476);
		// Quit button
		boxes.setupBox(6, 0, 480 - _sprites->getCursor(225).getHeight(),
		               _sprites->getCursor(225).getWidth(), 480);
		// Map
		boxes.setupBox(8, 403, 305, 622, 428);
		if (_currentInTimeline) {
			for (uint box_id = 0; box_id < ARRAYSIZE(kTimelineEntries); box_id++) {
				boxes.setupBox(10 + box_id, kTimelineEntries[box_id].x, kTimelineEntries[box_id].y,
				               kTimelineEntries[box_id].x + 30, kTimelineEntries[box_id].y + 15, kTimelineEntries[box_id].year);
			}
		}
	} else {
		uint quitInGameX = 640 - _sprites->getCursor(105).getWidth();
		boxes.setupBox(1, quitInGameX, 480 - _sprites->getCursor(105).getHeight(),
		               quitInGameX + _sprites->getCursor(105).getWidth(), 480);
	}
	boxes.setupBox(2, hyperlinksX, 480 - _sprites->getCursor(242).getHeight(),
	               hyperlinksX + _sprites->getCursor(242).getWidth(), 480);
}

void Versailles_Documentation::drawRecordBoxes(Graphics::ManagedSurface &surface, bool inDocArea,
		MouseBoxes &boxes) {
	if (_visitTrace.size()) {
		surface.transBlitFrom(_sprites->getSurface(105), boxes.getBoxOrigin(0), _sprites->getKeyColor(105));
	}
	if (inDocArea) {
		surface.transBlitFrom(_sprites->getSurface(19), boxes.getBoxOrigin(3), _sprites->getKeyColor(19));
		surface.transBlitFrom(_sprites->getSurface(227), boxes.getBoxOrigin(1), _sprites->getKeyColor(227));

		surface.fillRect(boxes.getBoxRect(9), 243);

		_fontManager->setCurrentFont(0);
		_fontManager->setTransparentBackground(true);
		_fontManager->setSpaceWidth(0);
		_fontManager->setCharSpacing(1);
		_fontManager->setForeColor(240);
		Common::Point catPos = boxes.getBoxOrigin(9);
		catPos += Common::Point(5, 3);
		_fontManager->displayStr(catPos.x, catPos.y, _categoryTitle);

		if (_currentRecord == _categoryEndRecord) {
			surface.transBlitFrom(_sprites->getSurface(75), boxes.getBoxOrigin(4), _sprites->getKeyColor(75));
		} else {
			surface.transBlitFrom(_sprites->getSurface(72), boxes.getBoxOrigin(4), _sprites->getKeyColor(72));
		}
		if (_currentRecord == _categoryStartRecord) {
			surface.transBlitFrom(_sprites->getSurface(77), boxes.getBoxOrigin(5), _sprites->getKeyColor(77));
		} else {
			surface.transBlitFrom(_sprites->getSurface(76), boxes.getBoxOrigin(5), _sprites->getKeyColor(76));
		}
		surface.transBlitFrom(_sprites->getSurface(225), boxes.getBoxOrigin(6), _sprites->getKeyColor(225));
	} else {
		surface.transBlitFrom(_sprites->getSurface(105), boxes.getBoxOrigin(1), _sprites->getKeyColor(105));
	}
	if (_currentLinks.size()) {
		surface.transBlitFrom(_sprites->getSurface(242), boxes.getBoxOrigin(2), _sprites->getKeyColor(242));
	} else {
		surface.transBlitFrom(_sprites->getSurface(244), boxes.getBoxOrigin(2), _sprites->getKeyColor(244));
	}
}

uint Versailles_Documentation::handlePopupMenu(const Graphics::ManagedSurface
		&originalSurface,
		const Common::Point &anchor, bool rightAligned, uint itemHeight,
		const Common::StringArray &items) {

	uint maxTextWidth = 0;

	_fontManager->setCurrentFont(4);
	_fontManager->setTransparentBackground(true);
	_fontManager->setCharSpacing(1);

	for (Common::StringArray::const_iterator it = items.begin(); it != items.end(); it++) {
		uint width = _fontManager->getStrWidth(*it);
		if (width > maxTextWidth) {
			maxTextWidth = width;
		}
	}

	uint width = maxTextWidth + 2 * kPopupMenuMargin;
	uint height = itemHeight * items.size() + 2 * kPopupMenuMargin;

	uint hiddenItems = 0;
	int top = anchor.y - height;
	while (top < 0) {
		hiddenItems++;
		top += itemHeight;
	}
	unsigned shownItems = items.size() - hiddenItems;

	Common::Rect popupRect;
	if (rightAligned) {
		popupRect = Common::Rect(anchor.x - width, top, anchor.x, anchor.y);
	} else {
		popupRect = Common::Rect(anchor.x, top, anchor.x + width, anchor.y);
	}

	Graphics::ManagedSurface surface;
	surface.copyFrom(originalSurface);

	MouseBoxes boxes(shownItems);
	for (uint i = 0; i < shownItems; i++) {
		boxes.setupBox(i, popupRect.left + kPopupMenuMargin,
		               popupRect.top + kPopupMenuMargin + i * itemHeight,
		               popupRect.right - kPopupMenuMargin,
		               popupRect.top + kPopupMenuMargin + (i + 1) * itemHeight);
	}

	_fontManager->setSurface(&surface);

	bool fullRedraw = true;
	bool redraw = true;
	uint hoveredBox = uint(-1);
	uint action = uint(-1);
	uint lastShownItem = items.size() - 1;
	uint firstShownItem = lastShownItem - shownItems + 1;

	uint slowScrollNextEvent = g_system->getMillis() + 250;

	Common::Point mouse;

	while (action == uint(-1)) {
		if (redraw) {
			if (fullRedraw) {
				surface.fillRect(popupRect, 247);
				fullRedraw = false;
			}
			for (uint i = 0; i < shownItems; i++) {
				if (i == 0 && firstShownItem != 0) {
					// There are items before the first one: display an arrow
					surface.transBlitFrom(_sprites->getSurface(162),
					                      Common::Point(popupRect.left + kPopupMenuMargin,
					                                    popupRect.top + kPopupMenuMargin + i * itemHeight + 3),
					                      _sprites->getKeyColor(162));
				} else if (i == shownItems - 1 && lastShownItem != items.size() - 1) {
					// There are items after the last one: display an arrow
					surface.transBlitFrom(_sprites->getSurface(185),
					                      Common::Point(popupRect.left + kPopupMenuMargin,
					                                    popupRect.top + kPopupMenuMargin + i * itemHeight + 3),
					                      _sprites->getKeyColor(185));
				} else {
					// Display the item text
					_fontManager->setForeColor(i == hoveredBox ? 241 : 243);
					_fontManager->displayStr(popupRect.left + kPopupMenuMargin,
					                         popupRect.top + kPopupMenuMargin + i * itemHeight + 3, items[firstShownItem + i]);
				}
			}
			g_system->copyRectToScreen(surface.getPixels(), surface.pitch, 0, 0, surface.w, surface.h);
			redraw = false;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);

		if (_engine->pollEvents()) {
			if (_engine->shouldAbort()) {
				// Fake the quit
				break;
			}
			mouse = _engine->getMousePos();

			uint newHovered = uint(-1);
			for (uint i = 0; i < shownItems; i++) {
				if (boxes.hitTest(i, mouse)) {
					newHovered = i;
					break;
				}
			}

			if (newHovered != hoveredBox) {
				hoveredBox = newHovered;
				redraw = true;
			}
		}

		DragStatus dragStatus = _engine->getDragStatus();

		if (hoveredBox == uint(-1)) {
			if (dragStatus == kDragStatus_Pressed) {
				break;
			} else {
				continue;
			}
		}

		// From there we only act if there is something hovered
		if (hoveredBox == 0 && firstShownItem > 0) {
			// Scroll up fast
			firstShownItem--;
			lastShownItem--;
			slowScrollNextEvent = g_system->getMillis() + 250;
			fullRedraw = true;
			redraw = true;
		} else if (hoveredBox == 1 && firstShownItem > 0) {
			// Scroll up slow
			if (g_system->getMillis() > slowScrollNextEvent) {
				firstShownItem--;
				lastShownItem--;
				slowScrollNextEvent = g_system->getMillis() + 250;
				fullRedraw = true;
				redraw = true;
			}
		} else if (hoveredBox == shownItems - 2 && lastShownItem < items.size() - 1) {
			// Scroll down slow
			if (g_system->getMillis() > slowScrollNextEvent) {
				firstShownItem++;
				lastShownItem++;
				slowScrollNextEvent = g_system->getMillis() + 250;
				fullRedraw = true;
				redraw = true;
			}
		} else if (hoveredBox == shownItems - 1 && lastShownItem < items.size() - 1) {
			// Scroll down fast
			firstShownItem++;
			lastShownItem++;
			slowScrollNextEvent = g_system->getMillis() + 250;
			fullRedraw = true;
			redraw = true;
		} else if (dragStatus == kDragStatus_Finished) {
			action = hoveredBox + firstShownItem;
			continue;
		}
	}

	// Restore original surface
	g_system->copyRectToScreen(originalSurface.getPixels(), originalSurface.pitch, 0, 0,
	                           originalSurface.w, originalSurface.h);
	g_system->updateScreen();

	_engine->waitMouseRelease();

	return action;
}

/* Below is documentation files parsing */

char *Versailles_Documentation::getDocPartAddress(char *start, char *end, const char *patterns[]) {
	if (!start) {
		return nullptr;
	}
	char *foundPos = nullptr;
	const char *pattern;
	uint patternLen = uint(-1);
	for (const char **patternP = patterns; *patternP && !foundPos; patternP++) {
		pattern = *patternP;
		patternLen = strlen(pattern);
		/*debug("Matching %.10s", pattern);*/
		for (char *p = start; p < end - patternLen - 1; p++) {
			/*if (p == start || *p == '\r' || *p == '\0') {
			    debug("Line %.10s", p == start ? start : p+1);
			}*/
			if (p == start && !strncmp(p, pattern, patternLen)) {
				foundPos = p;
				break;
			} else if ((*p == '\r' || *p == '\0') && !strncmp(p + 1, pattern, patternLen)) {
				foundPos = p + 1;
				break;
			}
		}
	}
	if (!foundPos) {
		return nullptr;
	}
	/*debug("Matched %.10s", foundPos);*/
	foundPos += patternLen;
	if (_multilineAttributes) {
		char *eoa = foundPos;

		// Find next '='
		for (; eoa < end && *eoa != '\0' && *eoa != '='; eoa++) {}

		if (eoa == end || *eoa == '\0') {
			// This is the end of block or data has already been split
			return foundPos;
		}

		// Go back to start of line
		for (; eoa != foundPos && *eoa != '\r'; eoa--) {}
		*eoa = '\0';
	} else {
		char *eol = foundPos;
		for (; *eol != '\r' && *eol != '\0'; eol++) {}
		*eol = '\0';
	}
	return foundPos;
}

static bool hasEqualInLine(const char *text, const char *end) {
	for (; text < end && *text && *text != '\r' && *text != '='; text++) { }
	return text < end && *text == '=';
}

static const char *nextLine(const char *text, const char *end) {
	for (; text < end && *text && *text != '\r'; text++) { }
	return text < end ? text + 1 : end;
}

const char *Versailles_Documentation::getDocTextAddress(char *start, char *end) {
	if (!start) {
		return nullptr;
	}
	const char *foundPos = nullptr;
	const char *p = start;
	while (p < end) {
		if (hasEqualInLine(p, end)) {
			p = nextLine(p, end);
			if (p < end && !hasEqualInLine(p, end)) {
				// Only return the text that is after the last =
				foundPos = p;
			}
		} else {
			p = nextLine(p, end);
		}
	}
	return foundPos;
}

const char *Versailles_Documentation::getRecordCaption(char *start, char *end) {
	const char *patterns[] = { "LEGENDE=", "LEGENDE =", nullptr };
	const char *ret = getDocPartAddress(start, end, patterns);
	return ret;
}

const char *Versailles_Documentation::getRecordTitle(char *start, char *end) {
	const char *patterns[] = { "TITRE=", "TITRE =", nullptr };
	const char *ret = getDocPartAddress(start, end, patterns);
	return ret;
}

const char *Versailles_Documentation::getRecordSubtitle(char *start, char *end) {
	const char *patterns[] = { "SOUS-TITRE=", "SOUS_TITRE=", "SOUS-TITRE =", "SOUS_TITRE =", "SOUS TITRE=", nullptr };
	char *ret = getDocPartAddress(start, end, patterns);
	if (!ret) {
		return nullptr;
	}

	uint ln = strlen(ret);
	char *p = ret + ln + 1; // Got to end of line and check next line
	for (; p < end && *p && *p != '\r' && *p != '=' ; p++) { }
	if (*p == '=') {
		// Next line has a =, so it's not multiline
		return ret;
	}

	if (*p == '\r') {
		*p = '\0';
	}
	ret[ln] = '\r';

	return ret;
}

void Versailles_Documentation::getRecordHyperlinks(char *start, char *end,
		Common::StringArray &hyperlinks) {
	const char *const hyperlinksPatterns[] = { "SAVOIR-PLUS 1=", "SAVOIR-PLUS 2=", "SAVOIR-PLUS 3=" };

	hyperlinks.clear();
	for (uint hyperlinkId = 0; hyperlinkId < ARRAYSIZE(hyperlinksPatterns); hyperlinkId++) {
		const char *patterns[] = { hyperlinksPatterns[hyperlinkId], nullptr };
		const char *ret = getDocPartAddress(start, end, patterns);
		if (ret) {
			hyperlinks.push_back(ret);
		}
	}
}

Common::String Versailles_Documentation::getRecordTitle(const Common::String &record) {
	Common::HashMap<Common::String, RecordInfo>::iterator it = _records.find(record);
	if (it == _records.end()) {
		return "";
	}

	const RecordInfo &recordInfo = it->_value;
	Common::File allDocsFile;

	if (!allDocsFile.open(_allDocsFilePath)) {
		error("Can't open %s", _allDocsFilePath.toString(Common::Path::kNativeSeparator).c_str());
	}
	allDocsFile.seek(recordInfo.position);

	char *recordData = new char[recordInfo.size + 1];
	allDocsFile.read(recordData, recordInfo.size);
	recordData[recordInfo.size] = '\0';
	char *recordDataEnd = recordData + recordInfo.size + 1;

	Common::String title = getRecordTitle(recordData, recordDataEnd);

	delete[] recordData;

	return title;
}

Common::String Versailles_Documentation::getRecordData(const Common::String &record,
		Common::String &title, Common::String &subtitle, Common::String &caption,
		Common::StringArray &hyperlinks) {
	Common::HashMap<Common::String, RecordInfo>::iterator it = _records.find(record);
	if (it == _records.end()) {
		warning("Can't find %s record data", record.c_str());
		return "";
	}

	const RecordInfo &recordInfo = it->_value;
	Common::File allDocsFile;

	if (!allDocsFile.open(_allDocsFilePath)) {
		error("Can't open %s", _allDocsFilePath.toString(Common::Path::kNativeSeparator).c_str());
	}
	allDocsFile.seek(recordInfo.position);

	char *recordData = new char[recordInfo.size + 1];
	allDocsFile.read(recordData, recordInfo.size);
	recordData[recordInfo.size] = '\0';
	char *recordDataEnd = recordData + recordInfo.size + 1;

	const char *titleP = getRecordTitle(recordData, recordDataEnd);
	/*debug("Title: %s", titleP);*/
	title = titleP ? titleP : "";
	const char *subtitleP = getRecordSubtitle(recordData, recordDataEnd);
	/*debug("SubTitle: %s", subtitleP);*/
	subtitle = subtitleP ? subtitleP : "";
	const char *captionP = getRecordCaption(recordData, recordDataEnd);
	/*debug("Caption: %s", captionP);*/
	caption = captionP ? captionP : "";
	getRecordHyperlinks(recordData, recordDataEnd, hyperlinks);

	const char *textP = nullptr;
	if (_multilineAttributes) {
		const char *patterns[] = { "TEXTE=", "TEXT=", nullptr };
		textP = getDocPartAddress(recordData, recordDataEnd, patterns);
	} else {
		textP = getDocTextAddress(recordData, recordDataEnd);
	}

	assert(textP != nullptr);
	Common::String text(textP);

	delete[] recordData;

	return text;
}

void Versailles_Documentation::convertHyperlinks(const Common::StringArray &hyperlinks,
		Common::Array<LinkInfo> &links) {
	for (Common::StringArray::const_iterator it = hyperlinks.begin(); it != hyperlinks.end(); it++) {
		LinkInfo link;
		link.record = *it;
		link.record.toUppercase();
		link.title = getRecordTitle(link.record);
		links.push_back(link);
	}
}

void Versailles_Documentation::loadLinksFile() {
	if (_linksData) {
		return;
	}

	Common::File linksFile;
	if (!linksFile.open(_linksDocsFilePath)) {
		error("Can't open links file: %s", _linksDocsFilePath.toString(Common::Path::kNativeSeparator).c_str());
	}

	_linksSize = linksFile.size();
	_linksData = new char[_linksSize + 1];

	linksFile.read(_linksData, _linksSize);
	_linksData[_linksSize] = '\0';
}

void Versailles_Documentation::getLinks(const Common::String &record,
										Common::Array<LinkInfo> &links) {
	loadLinksFile();

	links.clear();

	Common::String pattern = "\r";
	pattern += record;

	const char *recordStart = strstr(_linksData, pattern.c_str());
	if (!recordStart) {
		return;
	}

	const char *p = recordStart + pattern.size(); // Go beyond the record name
	for (; *p != '\r' && *p != '\0'; p++) { } // Goto next line
	if (!*p) {
		return;
	}
	p++;

	bool finished = false;
	while (!finished) {
		if (!scumm_strnicmp(p, "REM=", 4)) {
			// Comment: goto next line
			for (; *p != '\r' && *p != '\0'; p++) { }
		} else if (!scumm_strnicmp(p, "LIEN=", 5)) {
			// Link: read it
			const char *linkStart = p + 5;
			const char *linkEnd = linkStart;
			for (; *linkEnd != '\r' && *linkEnd != ' ' && *linkEnd != '\0'; linkEnd++) { }
			LinkInfo link;
			link.record = Common::String(linkStart, linkEnd);
			link.record.toUppercase();
			link.title = getRecordTitle(link.record);
			links.push_back(link);
			// Advance to end of link and finish the line
			p = linkEnd;
			for (; *p != '\r' && *p != '\0'; p++) { }
			//debug("Link %s/%s", link.record.c_str(), link.title.c_str());
		} else {
			// Something else: we expect a blank line to continue, else we are on a new record
			for (; *p != '\r' && *p != '\0'; p++) {
				if (*p != ' ' && *p != '\t' && *p != '\n') {
					finished = true;
					break;
				}
			}
		}
		if (*p == '\0') {
			break;
		}
		p++;
	}
}

} // End of namespace Versailles
} // End of namespace CryOmni3D
