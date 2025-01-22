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

#include "mediastation/mediastation.h"

namespace MediaStation {

enum TransitionType {
	kTransitionFadeToBlack = 300,
	kTransitionFadeToPalette = 301,
	kTransitionSetToPalette = 302,
	kTransitionSetToBlack = 303,
	kTransitionFadeToColor = 304,
	kTransitionSetToColor = 305,
	kTransitionSetToPercentOfPalette = 306,
	kTransitionFadeToPaletteObject = 307,
	kTransitionSetToPaletteObject = 308,
	kTransitionSetToPercentOfPaletteObject = 309
};

void MediaStationEngine::effectTransition(Common::Array<Operand> &args) {
	TransitionType transitionType = static_cast<TransitionType>(args[0].getInteger());
	switch (transitionType) {
	case kTransitionFadeToBlack:
	case kTransitionSetToBlack: {
		// TODO: Implement transition.
		warning("MediaStationEngine::effectTransition(): Fading/setting to black not implemented");
		break;
	}

	case kTransitionFadeToPalette:
	case kTransitionSetToPalette: {
		// TODO: Implement transition by getting palette out of current context.
		warning("MediaStationEngine::effectTransition(): Fading/setting to palette not implemented, changing palette immediately");
		break;
	}

	case kTransitionFadeToColor:
	case kTransitionSetToColor: {
		// TODO: Implement transitions.
		warning("MediaStationEngine::effectTransition(): Fading/setting to color not implemented");
		break;
	}

	case kTransitionFadeToPaletteObject: {
		// TODO: Implement transition.
		warning("MediaStationEngine::effectTransition(): Fading to palette object not implemented, changing palette immediately");
		Asset *asset = args[1].getAsset();
		g_engine->setPalette(asset);
		break;
	}

	case kTransitionSetToPaletteObject: {
		Asset *asset = args[1].getAsset();
		g_engine->setPalette(asset);
		break;
	}

	case kTransitionSetToPercentOfPaletteObject: {
		double percentComplete = args[1].getDouble();

		// TODO: Implement percent of palette transition.
		warning("MediaStationEngine::effectTransition(): Setting to %f%% of palette not implemented, changing palette immediately", percentComplete);
		Asset *asset = args[2].getAsset();
		g_engine->setPalette(asset);
		break;
	}

	default: {
		error("MediaStationEngine::effectTransition(): Got unknown transition type %d", static_cast<uint>(transitionType));
	}
	}
}

} // End of namespace MediaStation