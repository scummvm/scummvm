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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/font.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/iff.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/state/designselect.h"
#include "engines/nancy/state/scene.h"

namespace Common {
DECLARE_SINGLETON(Nancy::State::DesignSelect);
}

namespace Nancy {
namespace State {

// The swatch sits to the right of the design's name, hanging slightly below it
static const int16 kSwatchOffsetX = 36;
static const int16 kSwatchOffsetY = 6;

void DesignSelect::process() {
	switch (_state) {
	case kInit:
		init();
		// fall through
	case kRun:
		run();
		break;
	case kStop:
		stop();
		break;
	}
}

void DesignSelect::onStateEnter(const NancyState::NancyState prevState) {
	registerGraphics();
}

bool DesignSelect::onStateExit(const NancyState::NancyState nextState) {
	return true;
}

void DesignSelect::registerGraphics() {
	_background.registerGraphics();
	_list.registerGraphics();

	if (_acceptButton) {
		_acceptButton->registerGraphics();
	}

	if (_cancelButton) {
		_cancelButton->registerGraphics();
	}
}

void DesignSelect::buildDesignList() {
	_designs.clear();

	const PCUI *pcui = GetEngineData(PCUI);
	if (!pcui || pcui->characters.empty()) {
		return;
	}

	// The original enumerates the game directory's .DAT files and keeps the ones
	// whose name contains the FIRST character's base name, which is why only
	// Nancy ever has a choice of designs
	const Common::String &baseName = pcui->characters[0].imageName;

	Common::ArchiveMemberList members;
	SearchMan.listMatchingMembers(members, Common::Path(baseName + "*.dat"));

	for (const auto &member : members) {
		Common::String treeName = member->getName();
		const uint32 dot = treeName.findLastOf('.');
		if (dot == Common::String::npos) {
			continue;
		}

		treeName.erase(dot);

		// Each design's label and swatch come from the PUIH chunk at the head
		// of its own boot IFF
		if (!g_nancy->_resource->readCifTree(treeName, "dat", 1)) {
			continue;
		}

		IFF *iff = g_nancy->_resource->loadIFF(Common::Path(treeName + "_boot"));
		if (!iff) {
			continue;
		}

		Common::SeekableReadStream *chunkStream = iff->getChunkStream("PUIH");
		if (chunkStream) {
			PUIH header(chunkStream);
			_designs.push_back(Design());
			Design &design = _designs.back();
			design.name = treeName;
			design.themeName = header.themeName;
			design.swatchImageName = header.swatchImageName;

			delete chunkStream;
		}

		delete iff;

		if (_designs.size() >= _designData->designRowDests.size()) {
			// The screen has a fixed number of rows to lay these out in
			break;
		}
	}

	// The design already in use starts out selected
	const Common::String current = g_nancy->getPlayerCharacterDesign(0);
	for (uint i = 0; i < _designs.size(); ++i) {
		Design &design = _designs[i];
		design.labelDest = _designData->designRowDests[i];

		Graphics::ManagedSurface swatch;
		if (g_nancy->_resource->loadImage(design.swatchImageName, swatch, design.name)) {
			design.swatchDest = Common::Rect(swatch.w, swatch.h);
			design.swatchDest.moveTo(design.labelDest.right + kSwatchOffsetX,
									design.labelDest.bottom + kSwatchOffsetY - swatch.h);

			// Swatches never change, so they become part of the backdrop the
			// labels are drawn over
			_listBase.blitFrom(swatch, design.swatchDest.origin());
		}

		if (design.name.equalsIgnoreCase(current)) {
			_selected = i;
		}
	}

	if (_selected < 0 && !_designs.empty()) {
		_selected = 0;
	}
}

void DesignSelect::drawDesigns() {
	const Font *font = g_nancy->_graphics->getFont(_designData->fontID);
	const Font *highlightFont = g_nancy->_graphics->getFont(_designData->highlightFontID);
	if (!font || !_list._drawSurface.getPixels()) {
		return;
	}

	// Start from the swatches alone, so the previous selection's highlight goes away
	_list._drawSurface.blitFrom(_listBase);

	for (uint i = 0; i < _designs.size(); ++i) {
		const Design &design = _designs[i];
		const Font *rowFont = ((int)i == _selected && highlightFont) ? highlightFont : font;

		// The original anchors its text on the bottom row of the glyphs, while
		// drawString takes the top of the line
		rowFont->drawString(&_list._drawSurface, design.themeName,
			design.labelDest.left, design.labelDest.bottom - rowFont->getFontHeight(),
			_list._drawSurface.w - design.labelDest.left, 0);
	}

	_list.setVisible(true);
	_list.setNeedsRedraw(true);
}

int DesignSelect::designAt(const Common::Point &mousePos) const {
	for (uint i = 0; i < _designs.size(); ++i) {
		if (_designs[i].labelDest.contains(mousePos) || _designs[i].swatchDest.contains(mousePos)) {
			return i;
		}
	}

	return -1;
}

void DesignSelect::init() {
	_designData = GetEngineData(LDSN);
	assert(_designData);

	_background.init(_designData->backgroundImageName);
	_background.registerGraphics();

	// The overlay is a small sprite sheet holding the buttons' pressed and
	// highlighted artwork, not a layer of its own
	g_nancy->_resource->loadImage(_designData->overlayImageName, _buttonSprites);

	// The design list is drawn over the backdrop, so it covers the whole screen
	const uint16 width = MAX<uint16>(_background._drawSurface.w, 1);
	const uint16 height = MAX<uint16>(_background._drawSurface.h, 1);
	const uint32 transColor = g_nancy->_graphics->getTransColor();

	_listBase.create(width, height, g_nancy->_graphics->getInputPixelFormat());
	_listBase.clear(transColor);
	_list._drawSurface.create(width, height, _listBase.format);
	_list._drawSurface.setTransparentColor(transColor);
	_list.moveTo(Common::Rect(width, height));
	_list.setTransparent(true);

	if (_buttonSprites.getPixels() &&
			_designData->buttonDests.size() >= LDSN::kNumButtons &&
			_designData->buttonDownSrcs.size() >= LDSN::kNumButtons &&
			_designData->buttonHighlightSrcs.size() >= LDSN::kNumButtons) {
		_acceptButton.reset(new UI::Button(5, _buttonSprites, _designData->buttonDownSrcs[0],
			_designData->buttonDests[0], _designData->buttonHighlightSrcs[0]));
		_acceptButton->init();
		_acceptButton->setVisible(false);

		_cancelButton.reset(new UI::Button(5, _buttonSprites, _designData->buttonDownSrcs[1],
			_designData->buttonDests[1], _designData->buttonHighlightSrcs[1]));
		_cancelButton->init();
		_cancelButton->setVisible(false);
	}

	buildDesignList();
	drawDesigns();

	g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
	g_nancy->setMouseEnabled(true);

	registerGraphics();

	_state = kRun;
}

void DesignSelect::run() {
	NancyInput input = g_nancy->_input->getInput();

	if (_acceptButton) {
		_acceptButton->handleInput(input);

		if (_acceptButton->_isClicked) {
			g_nancy->_sound->playSound("BUOK");
			_accepted = true;
			_state = kStop;
			return;
		}
	}

	if (_cancelButton) {
		_cancelButton->handleInput(input);

		if (_cancelButton->_isClicked) {
			g_nancy->_sound->playSound("BUOK");
			_state = kStop;
			return;
		}
	}

	const int hovered = designAt(input.mousePos);
	if (hovered >= 0 && (input.input & NancyInput::kLeftMouseButtonUp) && hovered != _selected) {
		_selected = hovered;
		drawDesigns();
	}

	g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
}

void DesignSelect::stop() {
	if (g_nancy->_sound->isSoundPlaying("BUOK")) {
		return;
	}

	if (_accepted && _selected >= 0) {
		// Only the choice is recorded here; the scene rebuilds its UI from it
		// once the player is back in the game
		NancySceneState.setPlayerCharacterDesign(0, _designs[_selected].name);
	}

	// Hand the setup menu back the state it was opened from, so closing it in
	// turn returns to the game rather than back here
	g_nancy->setState(NancyState::kSetup, g_nancy->getPreviousState());
}

} // End of namespace State
} // End of namespace Nancy
