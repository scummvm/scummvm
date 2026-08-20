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

#ifndef ULTIMA1_VIEWS_TITLE_H
#define ULTIMA1_VIEWS_TITLE_H

#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "ultima/shared/gfx/view.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

using namespace Shared::Messages;

/**
 * Contains the loop of the Presents screen, blurb, and the castle scenes.
 * Any key press interrupts this.
 */
class Title : public Shared::Gfx::View {
private:
	enum State {
		PRESENTS, INTRO_A, INTRO_B, CASTLE, CREDITS
	};
	State _state = PRESENTS;
	int _castleStep = 0;

	// Logo pixel-reveal animation state (see revealLogoPixels)
	int _logoPixelsDrawn = 0;
	uint32 _logoLfsr = 1;
	bool _logoTextShown = false;

	// Credits screen's top/bottom logo slide-in state (see drawCreditsLogo)
	int _creditsLine = 0;
	bool _creditsTextShown = false;

	// Assets loaded on msgFocus/freed on msgUnfocus: the Origin logo, the
	// static castle picture, and its 3 animated flag frames
	Graphics::ManagedSurface _logo, _castle;
	Graphics::ManagedSurface _flags[3];
	Graphics::Palette _castlePalette;

	void showCredits();
	void showMainMenu();
	void drawCastleFlag(Shared::Gfx::GfxSurface &s, int xp);
	void resetLogoReveal();
	void revealLogoPixels(Shared::Gfx::GfxSurface &s);
	void drawCreditsLogo(Shared::Gfx::GfxSurface &s);

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
} // namespace Ultima1
} // namespace Ultima

#endif
