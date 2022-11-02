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

#ifndef STARK_UI_TOPMENU_H
#define STARK_UI_TOPMENU_H

#include "common/scummsys.h"
#include "common/rect.h"

#include "engines/stark/ui/window.h"

namespace Stark {

class VisualImageXMG;
class Button;

namespace Resources {
class Sound;
}

class TopMenu : public Window {
public:
	TopMenu(Gfx::Driver *gfx, Cursor *cursor);
	~TopMenu() override;

	// Window API
	void onGameLoop() override;
	void onRender() override;
	void onMouseMove(const Common::Point &pos) override;
	void onClick(const Common::Point &pos) override;

	/** The screen resolution changed, rebuild the text textures accordingly */
	void onScreenChanged();

	/** A new item has been added to the player's inventory. Play relevant animation */
	void notifyInventoryItemEnabled(uint16 itemIndex);

	/** A new entry has been added to the player's diary. Play relevant animation */
	void notifyDiaryEntryEnabled();

private:
	Button *getButtonAtPosition(const Common::Point &point) const;
	bool isAnimationPlaying() const;
	void updateAnimations();

	bool _widgetsVisible;

	Button *_inventoryButton;
	Button *_exitButton;
	Button *_optionsButton;
	int _newInventoryItemExplosionAnimTimeRemaining;
	int _newInventoryItemChestClosingAnimTimeRemaining;
	int _newDiaryEntryAnimTimeRemaining;
	Resources::Sound *_inventoryNewItemSound;
};

} // End of namespace Stark

#endif // STARK_UI_TOPMENU_H
