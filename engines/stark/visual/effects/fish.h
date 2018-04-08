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

#ifndef STARK_VISUAL_EFFECTS_FISH_H
#define STARK_VISUAL_EFFECTS_FISH_H

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
 * A visual effect for drawing a group of fish swimming around on top of the scene
 */
class VisualEffectFish : public VisualEffect {
public:
	static const VisualType TYPE = Visual::kEffectFish;

	explicit VisualEffectFish(Gfx::Driver *gfx, const Common::Point &size);
	~VisualEffectFish() override;

	/** Set the effect's parameters using the string based data format found in the game data files */
	void setParams(const Common::String &params);

	/** Draw the effect at the designated position */
	void render(const Common::Point &position);

private:

	struct Frame {
		float weight1;
		float weight2;
		float weight3;
		float weight4;
	};

	struct Fish {
		Common::Point currentPosition;
		Common::Point previousPosition;
		Common::Point point1;
		Common::Point point2;
		Common::Point point3;
		Common::Point point4;
	};

	// Parameters
	uint _fishCount;
	int _fishPathWidthRatio;
	int _fishPathHeightRatio;
	int _fishPathWidth;
	int _fishPathHeight;
	int _masterPathWidth;
	int _masterPathHeight;
	byte _mainColorR;
	byte _mainColorG;
	byte _mainColorB;
	uint32 _mainColor;
	uint32 _otherColor;
	uint _numFrames;
	uint _maxRadius;

	// State
	uint32 _currentFrame;
	Common::Array<Frame> _frames;
	Common::Array<Fish> _fishList;
	Fish _masterPath;

	void update();
	void drawFish(const Fish &fish);
};

} // End of namespace Stark

#endif // STARK_VISUAL_EFFECTS_FISH_H
