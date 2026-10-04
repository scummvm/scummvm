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

#ifndef ULTIMA3_VIEWS_TITLE_H
#define ULTIMA3_VIEWS_TITLE_H

#include "common/array.h"
#include "graphics/managed_surface.h"
#include "graphics/views/view.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

using namespace Graphics::Views;

/**
 * A rectangular region of EXOD.IBM revealed onto the screen, in pixels:
 * (srcY, dstY, x, width, height) -- see drawTitleBox1-6/drawSparkleBox
 */
struct TitleBox {
	int srcY, dstY, x, width, height;
};

/**
 * The title screen, ported directly from titleScreenAndChainToBootup and
 * its helpers (drawTitleBox1-6/drawSparkleBox/plotPixel2bpp/
 * drawAnimatedPixelPath/runBootFlagAnimation/drawAnimationFrameRow):
 * BLANK.IBM as a background, then a sequence of rectangular regions of
 * EXOD.IBM revealed on top of it (some instantly, some via a "sparkle"
 * randomized reveal), NAME.DAT's hand-drawn signature pixel-path, and
 * ANIMATE.DAT's waving-flag animation, each separated by a pause. Any
 * key/action sets a pending flag that fast-forwards through every
 * remaining reveal/pause (matching the original's keyboard-buffer-peek
 * behaviour: a single buffered keystroke stays "pending" and skips every
 * subsequent wait without needing to be pressed again) until a final
 * step consumes it and moves on to the logo screen.
 */
class Title : public View {
private:
	// Prefixed to avoid colliding with the file-scope TitleBox constants of
	// the same names (BOX2/BOX3/BOX5/BOX6) in title.cpp -- an unscoped
	// enum's values are injected into the class's own scope
	enum Phase {
		PH_BOX1_SPARKLE, PH_BOX1_WAIT,
		PH_BOX4_SPARKLE, PH_BOX4_WAIT,
		PH_BOX5, PH_BOX5_WAIT,
		PH_PIXEL_PATH, PH_PIXEL_PATH_WAIT,
		PH_BOX6, PH_BOX6_WAIT,
		PH_FLAG_ANIM, PH_FLAG_ANIM_WAIT,
		PH_BOX2,
		PH_SOUND_BURST,
		PH_BOX3, PH_BOX3_WAIT,
		PH_DRAIN
	};

	Phase _phase = PH_BOX1_SPARKLE;
	bool _keyPending = false;
	uint16 _prngState = 0x9DE3;
	int _waitCounter = 0;

	// Sparkle-box sub-state: 0-63 selects threshold (pass*4), 64 = the
	// unconditional final full-detail pass
	int _sparklePass = 0;

	// drawAnimatedPixelPath (NAME.DAT) sub-state: byte offset of the next
	// (length, row) pair
	uint _pixelPathPos = 0;

	// runBootFlagAnimation (ANIMATE.DAT) sub-state, mirroring si/bl/al
	int _flagSi = 2;
	int _flagBl = 0x2A;
	int _flagAl = 0;
	bool _flagAlActive = false;

	Graphics::ManagedSurface _canvas;
	Graphics::ManagedSurface _portrait;
	Common::Array<byte> _nameData;
	Common::Array<byte> _animateData;

	void loadPic(Graphics::ManagedSurface &surf, const Common::String &filename);
	void loadRaw(Common::Array<byte> &data, const Common::String &filename);

	// Reveals one word-column (8 px) of `box` per PRNG roll when sparkle is
	// true (drawSparkleBox's ah=0FFh mode); a plain full copy otherwise
	void drawBox(const TitleBox &box, bool sparkle, int threshold);

	// One 92x16px flag-flutter frame (drawAnimationFrameRow), plus its
	// 4px erase margins on each side
	void drawFlagFrame(int bxParam, int axParam);

	// One step of the NAME.DAT signature reveal; returns false once the
	// terminator is hit
	bool stepPixelPath();

	// One step of the flag animation's nested si/bl/al loop; returns false
	// once fully finished
	bool stepFlagAnimation();

	// Advances _waitCounter by one tick; returns true once the wait is over
	// (naturally, or because a key is already pending)
	bool tickWait();

	void playSoundBurst();
	void showLogoScreen();

public:
	Title();
	~Title() override {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	void draw() override;
	void timeout() override;

	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgMouseDown(const MouseDownMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
};

} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
