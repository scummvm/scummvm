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

#include "common/tokenizer.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/textbox.h"
#include "engines/nancy/ui/scrollbar.h"

namespace Nancy {
namespace UI {

Textbox::Textbox() :
		RenderObject(6),
		_scrollbar(nullptr),
		_scrollbarPos(0),
		_highlightRObj(7),
		_fontIDOverride(-1),
		_autoClearTime(0) {}

Textbox::~Textbox() {
	delete _scrollbar;
}

void Textbox::init() {
	const BSUM *bootSummary = (const BSUM *)g_nancy->getEngineData("BSUM");
	assert(bootSummary);

	const TBOX *textboxData = (const TBOX *)g_nancy->getEngineData("TBOX");
	assert(textboxData);

	moveTo(bootSummary->textboxScreenPosition);
	_highlightRObj.moveTo(bootSummary->textboxScreenPosition);
	initSurfaces(textboxData->innerBoundingBox.width(), textboxData->innerBoundingBox.height(), g_nancy->_graphicsManager->getScreenPixelFormat());

	Common::Rect outerBoundingBox = _screenPosition;
	outerBoundingBox.moveTo(0, 0);
	_drawSurface.create(_fullSurface, outerBoundingBox);

	RenderObject::init();

	// zOrder bumped by 2 to avoid overlap with the inventory box curtains in The Vampire Diaries
	_scrollbar = new Scrollbar(	11,
								textboxData->scrollbarSrcBounds,
								textboxData->scrollbarDefaultPos,
								textboxData->scrollbarMaxScroll - textboxData->scrollbarDefaultPos.y);
	_scrollbar->init();
}

void Textbox::registerGraphics() {
	RenderObject::registerGraphics();
	_scrollbar->registerGraphics();
	_highlightRObj.registerGraphics();
	_highlightRObj.setVisible(false);
}

void Textbox::updateGraphics() {
	if (_autoClearTime && g_nancy->getTotalPlayTime() > _autoClearTime) {
		clear();
	}

	if (_needsTextRedraw) {
		drawTextbox();
	}

	if (_scrollbarPos != _scrollbar->getPos()) {
		_scrollbarPos = _scrollbar->getPos();

		onScrollbarMove();
	}

	RenderObject::updateGraphics();
}

void Textbox::handleInput(NancyInput &input) {
	_scrollbar->handleInput(input);

	bool hasHighlight = false;
	for (uint i = 0; i < _hotspots.size(); ++i) {
		Common::Rect hotspot = _hotspots[i];
		hotspot.translate(0, -_drawSurface.getOffsetFromOwner().y);
		Common::Rect hotspotOnScreen = convertToScreen(hotspot).findIntersectingRect(_screenPosition);
		if (hotspotOnScreen.contains(input.mousePos)) {
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspotArrow);

			// Highlight the selected response
			if (g_nancy->getGameType() >= kGameTypeNancy2) {
				_highlightRObj.setVisible(true);
				Common::Rect hotspotInside = convertToLocal(hotspotOnScreen);
				hotspotInside.translate(0, _drawSurface.getOffsetFromOwner().y);
				_highlightRObj._drawSurface.create(_textHighlightSurface, hotspotInside);
				_highlightRObj.moveTo(hotspotOnScreen);
				hasHighlight = true;
			}

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				input.input &= ~NancyInput::kLeftMouseButtonUp;
				NancySceneState.clearLogicConditions();
				NancySceneState.setLogicCondition(i, g_nancy->_true);
			}

			break;
		}
	}

	if (!hasHighlight && _highlightRObj.isVisible()) {
		_highlightRObj.setVisible(false);
	}
}

void Textbox::drawTextbox() {
	const TBOX *textboxData = (const TBOX *)g_nancy->getEngineData("TBOX");
	assert(textboxData);

	HypertextParser::drawAllText(	Common::Point(textboxData->borderWidth, textboxData->firstLineOffset),				// x, y offset from surface edge
									_fullSurface.w - textboxData->maxWidthDifference - textboxData->borderWidth - 1,	// maximum width of text
									textboxData->lineHeight,															// expected height of text line
									textboxData->lineHeight + textboxData->lineHeight / 4,								// leading (vertical distance between two lines' bottoms)
									_fontIDOverride != -1 ? _fontIDOverride : textboxData->conversationFontID,			// font for basic text
									textboxData->highlightConversationFontID);											// font for highlight text

	setVisible(true);
}

void Textbox::clear() {
	if (_textLines.size()) {
		HypertextParser::clear();
		_scrollbar->resetPosition();
		onScrollbarMove();
		_fontIDOverride = -1;
		_needsRedraw = true;
		_autoClearTime = 0;
	}
}

void Textbox::addTextLine(const Common::String &text, uint32 autoClearTime) {
	HypertextParser::addTextLine(text);

	if (autoClearTime != 0) {
		// Start a timer, after which the textbox will automatically be cleared.
		// Currently only used by inventory closed captions
		_autoClearTime = g_nancy->getTotalPlayTime() + autoClearTime;
	}
}

void Textbox::overrideFontID(const uint fontID) {
	const BSUM *bootSummary = (const BSUM *)g_nancy->getEngineData("BSUM");
	assert(bootSummary);

	if (fontID >= bootSummary->numFonts) {
		error("Requested invalid override font ID %u in Textbox", fontID);
	}

	_fontIDOverride = fontID;
}

void Textbox::onScrollbarMove() {
	_scrollbarPos = CLIP<float>(_scrollbarPos, 0, 1);

	uint16 inner = getInnerHeight();
	uint16 outer = _screenPosition.height();

	if (inner > outer) {
		Common::Rect bounds = getBounds();
		bounds.moveTo(0, (inner - outer) * _scrollbarPos);
		_drawSurface.create(_fullSurface, bounds);
		_highlightRObj._drawSurface.create(_textHighlightSurface, bounds);
	} else {
		_drawSurface.create(_fullSurface, getBounds());
		_highlightRObj._drawSurface.create(_textHighlightSurface, getBounds());
	}

	_needsRedraw = true;
}

uint16 Textbox::getInnerHeight() const {
	const TBOX *textboxData = (const TBOX *)g_nancy->getEngineData("TBOX");
	assert(textboxData);

	// These calculations are _almost_ correct, but off by a pixel sometimes
	uint lineDist = textboxData->lineHeight + textboxData->lineHeight / 4;
	if (g_nancy->getGameType() == kGameTypeVampire) {
		return _numDrawnLines * lineDist + textboxData->firstLineOffset + 1;
	} else {
		return _numDrawnLines * lineDist + textboxData->firstLineOffset + lineDist / 2 - 1;
	}
}

} // End of namespace UI
} // End of namespace Nancy
