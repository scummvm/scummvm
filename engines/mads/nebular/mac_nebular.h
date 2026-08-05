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

#ifndef MADS_NEBULAR_MAC_NEBULAR_H
#define MADS_NEBULAR_MAC_NEBULAR_H

#include "common/array.h"
#include "common/rect.h"
#include "graphics/managed_surface.h"

namespace MADS {
namespace RexNebular {

class MacResourceProvider;
class RexNebularEngine;

class MacNebular {
private:
	RexNebularEngine &_engine;
	MacResourceProvider *_resources = nullptr;
	Common::Array<byte> _output;
	Graphics::ManagedSurface _popup;
	Common::Rect _popupRect;
	bool _popupActive = false;
	bool _layoutLogged = false;

public:
	explicit MacNebular(RexNebularEngine &engine);
	~MacNebular();

	void initGraphics();
	bool initResources();
	void applyGameSettings();
	Common::Point screenToGame(const Common::Point &point) const;
	Common::Point gameToScreen(const Common::Point &point) const;
	void presentScreen(int shakeOffset);
	void showPopup();
	void hidePopup();
};

} // namespace RexNebular
} // namespace MADS

#endif
