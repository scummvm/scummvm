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

#include "mohawk/console.h"
#include "mohawk/mohawk.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_pages/dialog_credits.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

ZoombiniDialogCredits::ZoombiniDialogCredits(MohawkEngine_Zoombini *vm) : ZoombiniDialog(vm, ZoombiniPageType::kCreditScreen) {
}

ZoombiniDialogCredits::~ZoombiniDialogCredits() {
	if (_hasCreditsMusicHandle) {
		_vm->_mixer->stopHandle(_creditsMusicHandle);
		_creditsMusicHandle = Audio::SoundHandle();
		_hasCreditsMusicHandle = false;
	}

	// [Deliberate MIDI difference]
	// Do not stop the entire MIDI. Pause MIDI when opening, and resume MIDI when closing.
	_vm->_midi->pause(false);
}

void ZoombiniDialogCredits::open() {
	// [Deliberate MIDI difference]
	// Do not stop the entire MIDI. Pause MIDI when opening, and resume MIDI when closing.
	_vm->_midi->pause(true);
	if (_vm->isVersionFamilyTlcV2())
		_vm->_sound->pauseActiveMusicSoundsForDialog();
}

void ZoombiniDialogCredits::onEveryFrame() {
	if (_totalCreditLines == 0)
		return;

	// Credit text advances independently of the static SCRB runner.
	// Invalidate the fixed scroll viewport explicitly so the previous and next text rows are both cleared and redrawn during this frame.
	Common::Rect dirtyRect = _scrollSrcRect;
	dirtyRect.extend(_scrollDestRect);
	dirtyRect.extend(getCreditLineRect());
	addExternalDirtyRect(dirtyRect);
}

void ZoombiniDialogCredits::updateCreditScrollElapsedFrames() {
	const uint32 frameDelta = _currentFrameCounter - _lastCreditScrollFrameCounter;
	_lastCreditScrollFrameCounter = _currentFrameCounter;

	_creditScrollElapsedFrames += static_cast<int32>(frameDelta) * _creditScrollFramesPerFrame;
	_creditScrollElapsedFrames = MAX<int32>(_creditScrollElapsedFrames, 0);
}

void ZoombiniDialogCredits::drawCreditLine(const Common::U32String &lineText, bool isTitle, int16 topY) {
	ZoombiniGraphics::TextConf tc;
	tc._fontUsage = ZoombiniFontUsage::kFontText;
	tc._hAlign = Graphics::kTextAlignCenter;
	tc._vAlign = Graphics::kTextAlignCenter;
	tc._textPalette = isTitle ? ZoombiniGraphics::kColor26_DarkRed : ZoombiniGraphics::kColor23_DarkOrange;
	tc._wordWrap = false;

	Common::Rect drawRect = getCreditLineRect();
	drawRect.top = topY;
	drawRect.bottom = static_cast<int16>(drawRect.top + 16);
	_vm->_gfx->drawText(ZoombiniGraphics::kShapeScreen, lineText, drawRect, tc);
}

Common::Rect ZoombiniDialogCredits::getCreditLineRect() const {
	Common::Rect lineRect = _textRect;
	lineRect.left = 0;
	lineRect.right = ZoombiniGraphics::kScreenWidth;
	return lineRect;
}

uint32 ZoombiniDialogCredits::getCreditScrollLineCount() const {
	if (_vm->isVersionFamilyTlcV2())
		return _totalCreditLines + kTlcCreditsPreLogoBlankLineCount +
			   kTlcCreditsLogoLineCount + kTlcCreditsPostLogoBlankLineCount;

	return _totalCreditLines;
}

void ZoombiniDialogCredits::drawTlcEndLogos(int32 elapsedFrames, int32 baseLineIdx, int32 startLineIdx, int32 endLineIdx) {
	if (!_vm->isVersionFamilyTlcV2())
		return;

	const int32 logoStartLineIdx = static_cast<int32>(_totalCreditLines + kTlcCreditsPreLogoBlankLineCount + 1);
	const int32 logoEndLineIdx = static_cast<int32>(_totalCreditLines + kTlcCreditsPreLogoBlankLineCount + kTlcCreditsLogoLineCount);
	if (endLineIdx < logoStartLineIdx || logoEndLineIdx < startLineIdx)
		return;

	const int32 logoTopY = drawLines_getLinePosY(elapsedFrames, baseLineIdx + logoStartLineIdx);
	if (ZoombiniGraphics::kScreenHeight <= logoTopY || logoTopY <= -64)
		return;

	const int16 logoY = static_cast<int16>(logoTopY);
	const ZmbResource creditsBitmap(ZmbResource::kSystem, kSysResShapeBitmap0020_Credits);
	_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen, creditsBitmap, kTlcCreditsBinkLogoShape, Common::Point(kTlcCreditsBinkLogoX, logoY), false);
	_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen, creditsBitmap, kTlcCreditsMilesLogoShape, Common::Point(kTlcCreditsMilesLogoX, logoY), false);
}

void ZoombiniDialogCredits::loadFeatures() {
	_pageStartFrameTime = _vm->_system->getMillis();
	_pageStartFrameCounter = _vm->getAnimationFrameCounter(_pageStartFrameTime);
	_lastCreditScrollFrameCounter = _pageStartFrameCounter;
	_creditScrollElapsedFrames = 0;
	_creditScrollFramesPerFrame = 1;

	_vm->_text->getLocalizedCredits(_creditParagraphs);
	_totalCreditLines = 0;
	for (const ZoombiniText::CreditParagraph &paragraph : _creditParagraphs)
		_totalCreditLines += paragraph.getTotalLineCount();

	// [Deliberate animation difference]
	// In original, the textline are rendered only if their y pixels can be fully drawn, creating a "crolling in by line" effect.
	// In ScummVM, the textlines are rendered regardless of their y pixels, creating a smooth "scrolling in by pixels" effect.

	// [Deliberate timing adaptation]
	// Elapsed-frame scrolling replaces the fixed-tick gate
	// while the single credits runner keeps the frame shapes on top.

	ZmbFeature::EventHooks hooksBackground;
	hooksBackground.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniDialogCredits::creditScreen_render));
	hooksBackground.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniDialogCredits::creditScreen_onMouseLButtonDown));
	hooksBackground.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniDialogCredits::creditScreen_onKeyDown));
	hooksBackground.setKeyUpFunc(static_cast<ZmbFeature::OnKeyUpFunc>(&ZoombiniDialogCredits::creditScreen_onKeyUp));
	loadScrbFeature(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0020_Credits), kSysResScrb0020_DialogCredits, 0,
					ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST,
					hooksBackground);

	Audio::SoundHandle *creditsMusicHandle = _vm->_sound->playSound(ZmbResource(ZmbResource::kSystem, kSysResSound20104_TownBGM),
																	   Audio::Mixer::SoundType::kMusicSoundType, true);
	if (creditsMusicHandle) {
		_creditsMusicHandle = *creditsMusicHandle;
		_hasCreditsMusicHandle = true;
	}
}

ZmbEventHandleResult ZoombiniDialogCredits::creditScreen_onMouseLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)feature;
	(void)absPos;
	(void)relPos;

	close();
	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniDialogCredits::creditScreen_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	(void)feature;
	(void)kbdRepeat;

	// Enhanced debug controls: rewind with "[" and fast-forward with "]".
	if (_vm->useEnhancedKbdShortcuts()) {
		switch (kbd.keycode) {
		case Common::KEYCODE_LEFTBRACKET:
			_creditScrollFramesPerFrame = -5;
			return ZmbEventHandleResult::kConsumed;
		case Common::KEYCODE_RIGHTBRACKET:
			_creditScrollFramesPerFrame = 15;
			return ZmbEventHandleResult::kConsumed;
		default:
			break;
		}
	}

	close();
	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniDialogCredits::creditScreen_onKeyUp(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	(void)feature;
	(void)kbdRepeat;

	// Enhanced debug controls: rewind with "[" and fast-forward with "]".
	if (_vm->useEnhancedKbdShortcuts() &&
		(kbd.keycode == Common::KEYCODE_LEFTBRACKET || kbd.keycode == Common::KEYCODE_RIGHTBRACKET)) {
		_creditScrollFramesPerFrame = 1;
		return ZmbEventHandleResult::kConsumed;
	}

	return ZmbEventHandleResult::kPassthrough;
}

ZmbRenderResult ZoombiniDialogCredits::creditScreen_render(ZmbFeature *feature) {
	// The credits runner owns the black backdrop.
	// Restricting it to @ref ZoombiniDialogCredits::_blitRect would leak captured page pixels.
	// Those pixels would appear behind the frame art in the non-scroll area.
	_vm->_gfx->fillArea(ZoombiniGraphics::kShapeScreen, ZoombiniGraphics::kColor2D_Black);
	updateCreditScrollElapsedFrames();

	if (_totalCreditLines == 0)
		return blitShapes(feature);

	const int32 scrollLineCount = static_cast<int32>(getCreditScrollLineCount());

	// Draw one credit line every 16 frames.
	// - lineIdx = frameCounter / 16
	// In each frame, the text pixel rects are scrolled up by 1 pixel to create scrolling effect.
	// - (scroll) y: 16 ~ 467 to 15 ~ 466, (text) y: 451 ~ 466
	// That implementation lags behind if the frame rate is lower than 60.

	// So instead, we determines which/how many lines to draw based on elapsed frames.
	// The text area patterns at frames divided by 16 should behave like:
	// - y: 451 - 466 (line N), 451-16 ~ 466-16 (line N-1), 451-32 ~ 466-32 (line N-2), ...

	// Find the lines to be drawn
	const int32 elapsedFrames = _creditScrollElapsedFrames;
	int32 rawStartLineIdx = MAX(0, drawLines_getStartLineIdx(elapsedFrames));
	int32 rawEndLineIdx = drawLines_getEndLineIdx(elapsedFrames);

	int32 baseLineIdx = (rawStartLineIdx / scrollLineCount) * scrollLineCount;
	int32 startLineIdx = rawStartLineIdx % scrollLineCount;
	int32 endLineIdx = MIN<int32>(rawEndLineIdx, scrollLineCount);

	// Find the paragraphs to be drawn
	do {
		int32 lineIdx = 0;
		for (const ZoombiniText::CreditParagraph &paragraph : _creditParagraphs) {
			const int32 paragraphLineCount = static_cast<int32>(paragraph.getTotalLineCount());
			if (lineIdx + paragraphLineCount <= startLineIdx) {
				lineIdx += paragraphLineCount;
				continue;
			}

			// Text lines
			for (uint32 li = 0; li < paragraph._lines.size() && lineIdx <= endLineIdx; li++) {
				lineIdx += 1;

				if (lineIdx < startLineIdx)
					continue;

				assert(startLineIdx <= lineIdx || lineIdx <= endLineIdx);

				drawCreditLine(paragraph._lines[li], li == 0 && paragraph._firstLineIsTitle,
							   static_cast<int16>(drawLines_getLinePosY(elapsedFrames, baseLineIdx + lineIdx)));
			}

			// Blank lines
			lineIdx += static_cast<int32>(paragraph._blankLineCount);
		}

		// TLC v2.0 retail: Draw the logo tBMPs after the credits text.
		drawTlcEndLogos(elapsedFrames, baseLineIdx, startLineIdx, endLineIdx);

		// End reached, loop to the beginning of the credits
		if (endLineIdx == scrollLineCount) {
			baseLineIdx += scrollLineCount;
			startLineIdx = 0;
			endLineIdx = rawEndLineIdx % scrollLineCount;
			continue;
		}

		break;
	} while (true);

	return blitShapes(feature);
}

int32 ZoombiniDialogCredits::drawLines_getLinePosY(int32 elapsedFrames, int32 lineIdx) {
	return _textRect.top - elapsedFrames + 16 * lineIdx;
}

int32 ZoombiniDialogCredits::drawLines_getStartLineIdx(int32 elapsedFrames) {
	return (elapsedFrames - _textRect.top) / 16;
}

int32 ZoombiniDialogCredits::drawLines_getEndLineIdx(int32 elapsedFrames) {
	return (ZoombiniGraphics::kScreenHeight + elapsedFrames - _textRect.top) / 16;
}

} // End of namespace Mohawk
