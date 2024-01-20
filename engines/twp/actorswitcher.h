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

#ifndef TWP_ACTORSWITCHER_H
#define TWP_ACTORSWITCHER_H

#include "twp/scenegraph.h"
#include "common/func.h"

namespace Twp {

#define asNone 0
#define asOn 0x1
#define asTemporaryUnselectable 0x2

typedef void SelectFunc(int id);

// This is where all the information about the actor icon stands
struct ActorSwitcherSlot {
	ActorSwitcherSlot(const Common::String& icon, Color back, Color frame, SelectFunc* selectFunc, int id = 0);

	void select();

	Common::String icon;
	Color back, frame;
	SelectFunc* selectFunc;
	int id;
};

// This allows to change the selected actors or to display the options (gear icon)
class ActorSwitcher : public Node {
public:
	ActorSwitcher();

	void update(const Common::Array<ActorSwitcherSlot>& slots, float elapsed);
	bool isMouseOver() const { return _mouseOver; }
	void setFlash(int flash) { _flash = flash; }

protected:
	void drawCore(Math::Matrix4 trsf) override final;
	void drawSprite(const SpriteSheetFrame &sf, Texture *texture, Color color, Math::Matrix4 trsf);
	void drawIcon(const Common::String &icon, Color backColor, Color frameColor, Math::Matrix4 trsf, int index);
	Math::Matrix4 transform(Math::Matrix4 trsf, int index);
	float getAlpha(size_t index) const;
	float height() const;
	int iconIndex(Math::Vector2d pos) const;
	Common::Rect rect() const;

public:
	int _mode = asNone;        // current mode

private:
	bool _mouseOver = false;                  // true when mouse is over the icons
	bool _down = false;                       // true when mouse button is down
	float _alpha = 0.f;                       // alpha value for the icon when flash is ON (flash != 0)
	int _flash = 0;                           // flash = 0: disable, flash = -1: enable, other values: time to flash
	float _flashElapsed = 0.f;                // flash time elapsed in seconds
	float _animElapsed = 0.f, _animPos = 0.f; // animation time elapsed in seconds and current position in the animation [0f-1f]
	Common::Array<ActorSwitcherSlot> _slots;  // list of slots containing icon, colors and select function
};

} // namespace Twp

#endif
