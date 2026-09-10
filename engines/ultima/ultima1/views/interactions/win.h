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

#ifndef ULTIMA1_VIEWS_INTERACTIONS_WIN_H
#define ULTIMA1_VIEWS_INTERACTIONS_WIN_H

#include "ultima/ultima1/views/interactions/interaction.h"
#include "ultima/shared/gfx/gfx_surface.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

using namespace Ultima::Shared::Messages;

/**
 * Shown when Mondain is defeated for good (the gem has already been
 * destroyed, so it can't be undone). Plays out the original's
 * playMondainDefeatCutscene: a small inset dialog announcing "THOU ART
 * VICTORIOUS!" (with a triple chime, matching showMondainDeathDialog),
 * then any key wipes to the ending text, which reveals itself top-down
 * over about 10 seconds - matching the original's own scanline-by-scanline
 * drawing of its "nif.bin" ending picture, which turned out (once decoded)
 * to itself just be a 320x168 1-bit-per-pixel bitmap of this same epilogue
 * text, not an illustration. A further keypress or action - during or
 * after the reveal - takes the player on to the main menu.
 *
 * The separate "Mondain is dead." message - shown in the Commands area
 * the moment his hit points reach zero, which doesn't necessarily stick if
 * the gem hasn't been destroyed yet - belongs to updateMondainState's end
 * of turn handling, not this view
 */
class Win : public Interaction {
private:
	enum Phase {
		PHASE_DIALOG,
		PHASE_GLYPH
	};

	Phase _phase = PHASE_DIALOG;

	// The ending text, rendered once into its own surface on focus, then
	// revealed gradually from the top down as _revealedRows grows
	Shared::Gfx::GfxSurface _textSurface;
	int _revealedRows = 0;
	int _elapsedTicks = 0;

	/**
	 * Draws the small "Mondain is dead!" dialog
	 */
	void drawDialog();

	/**
	 * Renders the full ending text into _textSurface, once, up front
	 */
	void renderEndingText();

	/**
	 * Handles a keypress/action: the first wipes from the dialog to the
	 * ending text reveal; a later one moves on to the main menu
	 */
	void advancePhase();

public:
	Win() : Interaction("Win") {
	}
	~Win() override {
	}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	void draw() override;
	bool tick() override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
