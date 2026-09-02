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

#ifndef MOHAWK_ZOOMBINI_PAGES_DIALOG_CREDITS_H
#define MOHAWK_ZOOMBINI_PAGES_DIALOG_CREDITS_H

#include "audio/mixer.h"

#include "mohawk/zoombini_pages/dialog_base.h"

namespace Mohawk {

/**
 * Scrolling credits dialog.
 *
 * Credit paragraphs are loaded from the localized text provider and rendered
 * into a clipped scroll surface. The page keeps its own elapsed-frame counter
 * so the scroll position is deterministic and can be advanced by the normal
 * page frame callback.
 */
class ZoombiniDialogCredits : public ZoombiniDialog {
public:
	/** Create the credits dialog. */
	ZoombiniDialogCredits(MohawkEngine_Zoombini *vm);
	/** Release credit text and scroll resources. */
	~ZoombiniDialogCredits() override;

	/** Pause the underlying page music while the credits are visible. */
	void open() override;
	/** Load the text and logo features used by the credits screen. */
	void loadFeatures() override;
	/** Advance the scroll and redraw the visible credit lines. */
	void onEveryFrame() override;

protected:
	/** Shape used for the TLC Bink logo. */
	static constexpr uint16 kTlcCreditsBinkLogoShape = 8;
	/** Shape used for the TLC Miles logo. */
	static constexpr uint16 kTlcCreditsMilesLogoShape = 7;
	/** Number of blank lines before the TLC logos. */
	static constexpr uint32 kTlcCreditsPreLogoBlankLineCount = 2;
	/** Number of logical lines occupied by the TLC logos. */
	static constexpr uint32 kTlcCreditsLogoLineCount = 4;
	/** Number of blank lines after the TLC logos. */
	static constexpr uint32 kTlcCreditsPostLogoBlankLineCount = 15;
	/** X coordinate of the TLC Bink logo. */
	static constexpr int16 kTlcCreditsBinkLogoX = 203;
	/** X coordinate of the TLC Miles logo. */
	static constexpr int16 kTlcCreditsMilesLogoX = 349;

	/** Advance the deterministic credits scroll counter. */
	void updateCreditScrollElapsedFrames();
	/** Draw one credit line at the requested screen position. */
	void drawCreditLine(const Common::U32String &lineText, bool isTitle, int16 topY);
	/** Return the clipped rectangle used by the credit text. */
	Common::Rect getCreditLineRect() const;
	/** Return the total number of logical lines in the credits. */
	uint32 getCreditScrollLineCount() const;
	/** Draw TLC end logos whose lines intersect the current scroll window. */
	void drawTlcEndLogos(int32 elapsedFrames, int32 baseLineIdx, int32 startLineIdx, int32 endLineIdx);

	/** Handle pointer input that advances or closes the credits. */
	ZmbEventHandleResult creditScreen_onMouseLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Handle keyboard input for the credits screen. */
	ZmbEventHandleResult creditScreen_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);
	/** Handle key release input for the credits screen. */
	ZmbEventHandleResult creditScreen_onKeyUp(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);

	/** Render the visible credits into the clipped scroll surface. */
	ZmbRenderResult creditScreen_render(ZmbFeature *feature);
	/** Get the Y position of a line in a given frame */
	int32 drawLines_getLinePosY(int32 elapsedFrames, int32 lineIdx);
	/** In a given frame, find the smallest lineIdx where posY >= 0 */
	int32 drawLines_getStartLineIdx(int32 elapsedFrames);
	/** In a given frame, find the largest lineIdx where posY <= 480 */
	int32 drawLines_getEndLineIdx(int32 elapsedFrames);

	/**
	 * Credits layout rectangles are owned by each dialog instance because
	 * Common::Rect requires runtime construction and must not create global
	 * C++ constructors in ScummVM.
	 */
	/** Screen rectangle containing the scrolling credit text. */
	const Common::Rect _textRect = Common::Rect(0x00BE, 0x01C3, 0x01C2, 0x01D2);
	/** Destination rectangle used when scrolling the credit surface. */
	const Common::Rect _scrollDestRect = Common::Rect(0x00BE, 0x0010, 0x01C2, 0x01D3);
	/** Source rectangle used when scrolling the credit surface. */
	const Common::Rect _scrollSrcRect = Common::Rect(0x00BE, 0x000F, 0x01C2, 0x01D2);
	/** Rectangle copied when restoring the credits surface. */
	const Common::Rect _blitRect = Common::Rect(0x00BE, 0x000F, 0x01C2, 0x01D2);

	/** Localized credit paragraphs and their title flags. */
	Common::Array<ZoombiniText::CreditParagraph> _creditParagraphs;
	/** Total logical line count including title and blank lines. */
	uint32 _totalCreditLines = 0;
	/** Frame at which the previous scroll update occurred. */
	uint32 _lastCreditScrollFrameCounter = 0;
	/** Elapsed scroll frames since the credits opened. */
	int32 _creditScrollElapsedFrames = 0;
	/** Number of scroll frames advanced per engine frame. */
	int16 _creditScrollFramesPerFrame = 1;
	/** Mixer handle for the background music started by this credits dialog. */
	Audio::SoundHandle _creditsMusicHandle;
	/** Whether @ref _creditsMusicHandle was started successfully. */
	bool _hasCreditsMusicHandle = false;
};

} // End of namespace Mohawk

#endif
