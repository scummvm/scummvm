/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_VISUAL_EFFECTS_BUBBLES_H
#define STARK_VISUAL_EFFECTS_BUBBLES_H

#include "engines/stark/visual/effects/effect.h"

#include "common/array.h"
#include "common/rect.h"

namespace Stark {

namespace Gfx {
class Driver;
class SurfaceRenderer;
class Texture;
}

/**
 * A visual effect for drawing small bubbles moving up on top of the scene
 */
class VisualEffectBubbles : public VisualEffect {
public:
	static const VisualType TYPE = Visual::kEffectBubbles;

	explicit VisualEffectBubbles(Gfx::Driver *gfx, const Common::Point &size);
	~VisualEffectBubbles() override;

	/** Set the effect's parameters using the string based data format found in the game data files */
	void setParams(const Common::String &params);

	/** Draw the effect at the designated position */
	void render(const Common::Point &position);

private:

	enum Kind {
		kSmall  = 1,
		kLarge  = 2,
		kRandom = 3
	};

	struct Bubble {
		Common::Point position;
		Kind kind;
	};

	// Parameters
	uint _bubbleCount;
	Kind _kind;
	int _sourcePositionRatioX;
	int _maxVerticalSpeed;
	int _maxHorizontalSpeed;
	byte _mainColorR;
	byte _mainColorG;
	byte _mainColorB;

	// State
	Common::Point _sourcePosition;
	uint32 _mainColor;
	uint32 _darkColor;
	Common::Array<Bubble> _bubbles;

	void update();
	void drawBubble(const Bubble &bubble) const;
	void drawSmallBubble(const Bubble &bubble) const;
	void drawLargeBubble(const Bubble &bubble) const;
};

} // End of namespace Stark

#endif // STARK_VISUAL_EFFECTS_BUBBLES_H
