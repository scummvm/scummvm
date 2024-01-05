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

#include "twp/actorswitcher.h"
#include "twp/twp.h"
#include "twp/util.h"

#define DISABLE_ALPHA 0.f
#define ENABLE_ALPHA 1.f
#define INACTIVE_ALPHA 0.5f
#define ACTOR_SEP 60.f
#define MARGIN 30.f
#define ANIM_DURATION 0.120f

namespace Twp {

ActorSwitcherSlot::ActorSwitcherSlot(const Common::String &icon, Color back, Color frame, SelectFunc *selectFunc, int id) {
	this->icon = icon;
	this->back = back;
	this->frame = frame;
	this->selectFunc = selectFunc;
	this->id = id;
}

void ActorSwitcherSlot::select() {
	if(selectFunc) {
		selectFunc(id);
	}
}

ActorSwitcher::ActorSwitcher() : Node("ActorSwitcher") {
	_alpha = ENABLE_ALPHA;
}

Math::Matrix4 ActorSwitcher::transform(Math::Matrix4 trsf, int index) {
	float animPos = _mouseOver ? _animPos : 1.f;
	Math::Vector3d pos(SCREEN_WIDTH - MARGIN, SCREEN_HEIGHT - MARGIN - animPos * ACTOR_SEP * index, 0.f);
	Math::Vector2d s(2.f, 2.f);
	trsf.translate(pos);
	Twp::scale(trsf, s);
	return trsf;
}

float ActorSwitcher::getAlpha(int index) const {
	if (index == (_slots.size() - 1))
		return ENABLE_ALPHA;
	if (_mode & asTemporaryUnselectable)
		return INACTIVE_ALPHA;
	if (_mode & asOn)
		return _mouseOver ? ENABLE_ALPHA : _alpha;
	return _mouseOver ? INACTIVE_ALPHA : DISABLE_ALPHA;
}

void ActorSwitcher::drawIcon(const Common::String &icon, Color backColor, Color frameColor, Math::Matrix4 trsf, int index) {
	SpriteSheet *gameSheet = g_engine->_resManager.spriteSheet("GameSheet");
	Texture *texture = g_engine->_resManager.texture(gameSheet->meta.image);
	const SpriteSheetFrame &iconBackFrame = gameSheet->frameTable["icon_background"];
	const SpriteSheetFrame &iconActorFrame = gameSheet->frameTable[icon];
	const SpriteSheetFrame &iconFrame = gameSheet->frameTable["icon_frame"];
	Math::Matrix4 t = transform(trsf, index);
	float alpha = getAlpha(index);

	drawSprite(iconBackFrame, texture, Color::withAlpha(backColor, alpha), t);
	drawSprite(iconActorFrame, texture, Color::withAlpha(Color(), alpha), t);
	drawSprite(iconFrame, texture, Color::withAlpha(frameColor, alpha), t);
}

void ActorSwitcher::drawCore(Math::Matrix4 trsf) {
	if (_mouseOver) {
		for (int i = 0; i < _slots.size(); i++) {
			ActorSwitcherSlot &slot = _slots[i];
			drawIcon(slot.icon, slot.back, slot.frame, trsf, i);
		}
	} else if (_slots.size() > 0) {
		ActorSwitcherSlot &slot = _slots[0];
		drawIcon(slot.icon, slot.back, slot.frame, trsf, 0);
	}
}

void ActorSwitcher::drawSprite(const SpriteSheetFrame &sf, Texture *texture, Color color, Math::Matrix4 trsf) {
	Math::Vector3d pos(sf.spriteSourceSize.left - sf.sourceSize.getX() / 2.f, -sf.spriteSourceSize.height() - sf.spriteSourceSize.top + sf.sourceSize.getY() / 2.f, 0.f);
	trsf.translate(pos);
	g_engine->getGfx().drawSprite(sf.frame, *texture, color, trsf);
}

float ActorSwitcher::height() const {
	float n = _mouseOver ? _slots.size() : 1;
	return n * ACTOR_SEP;
}

int ActorSwitcher::iconIndex(Math::Vector2d pos) const {
	float y = SCREEN_HEIGHT - pos.getY();
	return _slots.size() - 1 - (int)((height() - y) / ACTOR_SEP);
}

Common::Rect ActorSwitcher::rect() const {
	float h = height();
	return Common::Rect(Common::Point(SCREEN_WIDTH - ACTOR_SEP, SCREEN_HEIGHT - h), ACTOR_SEP, h);
}

void ActorSwitcher::update(const Common::Array<ActorSwitcherSlot> &slots, float elapsed) {
	if (_visible) {
		_slots = slots;

		// update flash icon
		if ((_flash != 0) && ((_flash == -1) || (_flashElapsed < _flash))) {
			_flashElapsed = _flashElapsed + elapsed;
			_alpha = 0.6f + 0.4f * sin(M_PI * 2.f * _flashElapsed);
		} else {
			_flash = 0;
			_flashElapsed = 0.f;
			_alpha = INACTIVE_ALPHA;
		}

		// check if mouse is over actor icons or gear icon
		Math::Vector2d scrPos = g_engine->winToScreen(g_engine->_cursor.pos);
		bool oldMouseOver = _mouseOver;
		_mouseOver = !_down && rect().contains(scrPos.getX(), scrPos.getY());

		// update anim
		_animElapsed = _animElapsed + elapsed;

		// stop anim or flash if necessary
		if (oldMouseOver != _mouseOver) {
			_animElapsed = 0.f;
			if (_mouseOver)
				_flash = 0;
		}

		// update anim pos
		_animPos = MIN(1.f, _animElapsed / ANIM_DURATION);

		// check if we select an actor or gear icon
		if (_mouseOver && (g_engine->_cursor.leftDown) && !_down) {
			_down = true;
			// check if we allow to select an actor
			int iconIdx = iconIndex(scrPos);
			if ((_mode & asOn) || (iconIdx == (_slots.size() - 1))) {
				if (_slots[iconIdx].selectFunc != nullptr)
					_slots[iconIdx].select();
			}
		}
		if (!g_engine->_cursor.leftDown)
			_down = false;
	}
}

} // namespace Twp
