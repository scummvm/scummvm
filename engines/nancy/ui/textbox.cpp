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
#include "engines/nancy/ui/scrolltextbox.h"

namespace Nancy {
namespace UI {

Textbox::Textbox() :
		RenderObject(g_nancy->getGameType() >= kGameTypeNancy10 ? 10 : 6),
		_scrollbar(nullptr),
		_scrollbarPos(0),
		_highlightRObj(g_nancy->getGameType() >= kGameTypeNancy10 ? 11 : 7),
		_fontIDOverride(-1),
		_autoClearTime(0),
		_scrollTextBox(nullptr) {}

Textbox::~Textbox() {
	delete _scrollbar;
	delete _scrollTextBox;
}

void Textbox::init() {
	// Nancy 10+: the game text box is a UIPopupHeader overlay popup (UICO in
	// Nancy 10, SCTB in Nancy 11). Delegate to a ScrollTextBox; the code below
	// is the Nancy 1-9 bottom-right flat box.
	if (g_nancy->getGameType() >= kGameTypeNancy10) {
		_scrollTextBox = new ScrollTextBox();
		_scrollTextBox->init();
		return;
	}

	auto *tbox = GetEngineData(TBOX);
	assert(tbox);

	auto *bsum = GetEngineData(BSUM);
	assert(bsum);

	moveTo(bsum->textboxScreenPosition);
	_highlightRObj.moveTo(bsum->textboxScreenPosition);
	initSurfaces(tbox->innerBoundingBox.width(), tbox->innerBoundingBox.height(), g_nancy->_graphics->getScreenPixelFormat(),
		tbox->textBackground, tbox->highlightTextBackground);

	Common::Rect outerBoundingBox = _screenPosition;
	outerBoundingBox.moveTo(0, 0);
	_drawSurface.create(_fullSurface, outerBoundingBox);

	RenderObject::init();

	// zOrder bumped by 2 to avoid overlap with the inventory box curtains in The Vampire Diaries
	_scrollbar = new Scrollbar(	11,
								tbox->scrollbarSrcBounds,
								tbox->scrollbarDefaultPos,
								tbox->scrollbarMaxScroll - tbox->scrollbarDefaultPos.y);
	_scrollbar->init();
}

void Textbox::registerGraphics() {
	if (_scrollTextBox) {
		_scrollTextBox->registerGraphics();
		return;
	}

	RenderObject::registerGraphics();
	if (_scrollbar)
		_scrollbar->registerGraphics();
	_highlightRObj.registerGraphics();
	_highlightRObj.setVisible(false);
}

void Textbox::updateGraphics() {
	if (_scrollTextBox) {
		// The ScrollTextBox is registered separately and updates itself.
		return;
	}

	if (_autoClearTime && g_nancy->getTotalPlayTime() > _autoClearTime)
		clear();

	if (_needsTextRedraw)
		drawTextbox();

	if (_scrollbar && _scrollbarPos != _scrollbar->getPos()) {
		_scrollbarPos = _scrollbar->getPos();

		onScrollbarMove();
	}

	RenderObject::updateGraphics();
}

void Textbox::setFullMode(bool open, uint32 timeoutMs) {
	// No-op: the Nancy 10+ overlay shows itself when it holds text, and the
	// Nancy 1-9 box has no full mode. Kept for the FrameTextBox AR's API.
}

void Textbox::handleInput(NancyInput &input) {
	if (_scrollTextBox) {
		_scrollTextBox->handleInput(input);
		return;
	}

	if (!isVisible())
		return;

	if (_scrollbar)
		_scrollbar->handleInput(input);

	bool hasHighlight = false;
	for (uint i = 0; i < _hotspots.size(); ++i) {
		Common::Rect hotspot = _hotspots[i];
		hotspot.translate(0, -_drawSurface.getOffsetFromOwner().y);
		Common::Rect hotspotOnScreen = convertToScreen(hotspot).findIntersectingRect(_screenPosition);
		if (hotspotOnScreen.contains(input.mousePos)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);

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

	if (!hasHighlight && _highlightRObj.isVisible())
		_highlightRObj.setVisible(false);
}

void Textbox::drawTextbox() {
	auto *tbox = GetEngineData(TBOX);
	assert(tbox);

	Common::Rect textBounds = _fullSurface.getBounds();
	uint16 highlightFontID = tbox->highlightConversationFontID;

	// TODO: These bounds are not right: the right offset is a bit off,
	// and the left offset takes into account the scrollbar, which doesn't
	// exist in this widget.
	textBounds.top += tbox->upOffset;
	textBounds.bottom -= tbox->downOffset;
	textBounds.left += tbox->leftOffset;
	textBounds.right -= tbox->rightOffset;

	uint16 baseFontID = (_fontIDOverride != -1) ? _fontIDOverride : tbox->defaultFontID;

	const Font *font = g_nancy->_graphics->getFont(baseFontID);
	textBounds.top -= font->getFontHeight();

	HypertextParser::drawAllText(textBounds, 0, baseFontID, highlightFontID);

	// The Nancy 1-9 box is always on screen. (Nancy 10+ renders through the
	// ScrollTextBox and never reaches this path.)
	setVisible(true);
}

void Textbox::clear() {
	if (_scrollTextBox) {
		_scrollTextBox->clear();
		return;
	}

	if (!_textLines.empty()) {
		HypertextParser::clear();
		if (_scrollbar) {
			_scrollbar->resetPosition();
			onScrollbarMove();
		}
		_fontIDOverride = -1;
		_needsRedraw = true;
		_autoClearTime = 0;
	}
}

void Textbox::addTextLine(const Common::String &text, uint32 autoClearTime) {
	// WORKAROUND: Don't draw debug strings in the textbox. Refer to bug
	// #16745 for a case in Nancy9, scene 2579 (after making a sandwich).
	// TODO: Check why this text doesn't appear in the original. All the
	// dependencies of the associated AR are satisfied.
	Common::String debugString = Common::String::format("%d *** ", NancySceneState.getSceneInfo().sceneID);
	if (text.contains(debugString))
		return;

	if (_scrollTextBox) {
		_scrollTextBox->addTextLine(text, autoClearTime);
		return;
	}

	HypertextParser::addTextLine(text);

	if (autoClearTime != 0) {
		// Start a timer, after which the textbox will automatically be cleared.
		// Currently only used by inventory closed captions
		_autoClearTime = g_nancy->getTotalPlayTime() + autoClearTime;
	}

	if (_scrollbar) {
		_scrollbar->resetPosition();
		onScrollbarMove();
	}
}

void Textbox::setOverrideFont(const uint fontID) {
	if (_scrollTextBox) {
		_scrollTextBox->setOverrideFont(fontID);
		return;
	}

	auto *bsum = GetEngineData(BSUM);
	assert(bsum);

	if (fontID >= bsum->numFonts) {
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
	// As early as nancy3 this behavior stopped being relevant, as the original
	// engine always scrolls down to the bottom of the entire inner surface.
	// However, that makes the scrollbar almost unusable, so I'm not changing this.
	auto *tbox = GetEngineData(TBOX);
	assert(tbox);

	return _drawnTextHeight + tbox->upOffset + tbox->downOffset;
}

} // End of namespace UI
} // End of namespace Nancy
