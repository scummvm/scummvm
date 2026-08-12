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

#include "common/rect.h"
#include "graphics/managed_surface.h"
#include "mads/core/general.h"

namespace Common {
struct Event;
}

namespace MADS {
namespace RexNebular {

class MacResourceProvider;
class MacNebularMenu;
class RexNebularEngine;

class MacNebular {
private:
	RexNebularEngine &_engine;
	MacResourceProvider *_resources = nullptr;
	MacNebularMenu *_menus = nullptr;
	Graphics::ManagedSurface _output;
	Graphics::ManagedSurface _popup;
	Common::Rect _popupRect;
	RGBcolor _palette[256];
	bool _useOriginalMenus;
	bool _fullFrameActive = false;
	bool _popupActive = false;
	bool _layoutLogged = false;

public:
	explicit MacNebular(RexNebularEngine &engine);
	~MacNebular();

	void initGraphics();
	bool initResources();
	void applyGameSettings();
	void selectDifficulty();
	int selectResumeSlot();
	bool usesOriginalMenus() const { return _useOriginalMenus; }
	void setFullFrameActive(bool active) { _fullFrameActive = active; }
	void setOuterMenuActive(bool active);
	Common::Point screenToGame(const Common::Point &point) const;
	Common::Point gameToScreen(const Common::Point &point) const;
	bool handleMacEvent(Common::Event &event);
	void setPalette(const RGBcolor *palette, int firstColor, int numColors);
	void getPalette(RGBcolor *palette, int firstColor, int numColors) const;
	void presentScreen(int shakeOffset);
	void showPopup();
	void hidePopup();
};

} // namespace RexNebular
} // namespace MADS

#endif
