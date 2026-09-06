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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "graphics/fonts/dosfont.h"
#include "math/utils.h"

#include "freescape/games/3dck/3dck.h"

namespace Freescape {

uint32 KitEngine::indicatorColor(byte color) const {
	return _scriptSurface.format.ARGBToColor(255,
		_palette[3 * color], _palette[3 * color + 1], _palette[3 * color + 2]);
}

void KitEngine::printMessage(uint16 indicator, const Common::String &message) {
	if (!indicator || indicator > _indicatorData.size() / 17)
		return;
	const uint16 *data = &_indicatorData[17 * (indicator - 1)];
	if (data[0] != 1 || data[1] >= _screenW || data[2] >= _screenH)
		return;
	Common::Rect rect(data[1], data[2], MIN<int>(data[1] + data[3], _screenW),
		MIN<int>(data[2] + data[4], _screenH));
	if (rect.isEmpty())
		return;
	Graphics::Surface surface = _scriptSurface.getSubArea(rect);
	Graphics::DosFont font;
	int x = 0, y = 0;
	for (uint i = 0; i < message.size(); i++) {
		byte chr = message[i];
		if (chr == 0x1b && i + 1 < message.size()) {
			chr = message[++i];
			if (chr == 'N') {
				x = 0;
				y += 8;
				continue;
			}
			if (chr >= 'A' && chr <= 'Z')
				continue;
		}
		if (y >= surface.h)
			break;
		Common::Rect cell(x, y, MIN(x + 8, int(surface.w)), MIN(y + 8, int(surface.h)));
		surface.fillRect(cell, indicatorColor(data[11]));
		font.drawChar(&surface, chr, x, y, indicatorColor(data[10]));
		x += 8;
		if (x >= surface.w) {
			x = 0;
			y += 8;
		}
	}
}

void KitEngine::updateIndicators() {
	Graphics::DosFont font;
	for (uint i = 0; i < _indicatorData.size(); i += 17) {
		const uint16 *data = &_indicatorData[i];
		if (data[0] < 2 || data[0] > 4 || data[1] >= _screenW || data[2] >= _screenH)
			continue;
		Common::Rect rect(data[1], data[2], MIN<int>(data[1] + data[3], _screenW),
			MIN<int>(data[2] + data[4], _screenH));
		if (rect.isEmpty())
			continue;
		int32 first = int32((uint32(data[5]) << 16) | data[6]);
		int32 last = int32((uint32(data[7]) << 16) | data[8]);
		int32 value = CLIP<int32>(_kitVariables[data[9] & 0xff], MIN(first, last), MAX(first, last));
		uint32 foreground = indicatorColor(data[10]), background = indicatorColor(data[11]);
		_scriptSurface.fillRect(rect, background);
		if (data[0] == 2) {
			int digits = MIN<int>(rect.width() / 8, 8);
			Common::String number = Common::String::format("%0*d", digits, value);
			Graphics::Surface surface = _scriptSurface.getSubArea(rect);
			font.drawString(&surface, number, 0, 0, rect.width(), foreground);
		} else if (first != last) {
			int length = data[0] == 3 ? rect.width() : rect.height();
			int filled = (int64(value) - first) * length / (int64(last) - first);
			if (data[0] == 3)
				rect.right = rect.left + filled;
			else
				rect.top = rect.bottom - filled;
			if (!rect.isEmpty())
				_scriptSurface.fillRect(rect, foreground);
		}
	}
}

void KitEngine::drawUI() {
	updateIndicators();
	drawFullscreenSurface(_scriptSurface.surfacePtr());
	_gfx->setViewport(_fullscreenViewArea);
	_gfx->renderCrossair(_crossairPosition);
}

bool KitEngine::handleInput(const Common::Event &event) {
	if (event.type == Common::EVENT_KEYDOWN) {
		_kitVariables[15] = event.kbd.ascii;
		if (!_kitVariables[15] && event.kbd.keycode < 128)
			_kitVariables[15] = event.kbd.keycode;
	} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
		int control = -1;
		switch (event.customType) {
		case kActionEscape:
			return false;
		case kActionChangeMode:
			_kitVariables[15] = ' ';
			return false;
		case kActionSkip:
			_kitVariables[15] = ' ';
			return true;
		case kActionMoveUp: control = 0; break;
		case kActionMoveDown: control = 1; break;
		case kActionMoveLeft: control = 3; break;
		case kActionMoveRight: control = 2; break;
		case kActionRotateUp: control = 8; break;
		case kActionRotateDown: control = 9; break;
		case kActionRotateLeft: control = 6; break;
		case kActionRotateRight: control = 7; break;
		case kActionTurnBack: control = 12; break;
		case kActionShoot: control = 30; break;
		case kActionActivate: control = 31; break;
		case kActionInfoMenu:
			_kitVariables[15] = 'I';
			return true;
		default: break;
		}
		if (control >= 0) {
			byte key = _controlData[5 * control + 4] >> 8;
			if (key != 0xff)
				_kitVariables[15] = key;
		}
		if (event.customType == kActionShoot || event.customType == kActionActivate) {
			if (!_scriptFrameActive)
				interact(event.customType == kActionShoot);
			return true;
		}
		// Track held movement keys during DELAY; movement itself waits.
		bool movement = event.customType == kActionMoveUp || event.customType == kActionMoveDown ||
			event.customType == kActionMoveLeft || event.customType == kActionMoveRight;
		return _scriptFrameActive && !movement;
	} else if (_scriptFrameActive && event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END &&
			(event.customType == kActionRiseOrFlyUp || event.customType == kActionLowerOrFlyDown)) {
		_moveUp = _moveDown = false;
		return true;
	} else if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
		Common::Point mouse = getNormalizedPosition(event.mouse);
		if (_shootMode)
			_crossairPosition = mouse;
		int buttons = g_system->getEventManager()->getButtonState();
		_kitVariables[16] = ((buttons & Common::EventManager::LBUTTON) ? 1 : 0) |
			((buttons & Common::EventManager::RBUTTON) ? 2 : 0);
		_kitVariables[16] |= event.type == Common::EVENT_LBUTTONDOWN ? 1 : 2;
		_kitVariables[17] = mouse.x;
		_kitVariables[18] = mouse.y;
		if (!_scriptFrameActive)
			interact(event.type == Common::EVENT_LBUTTONDOWN);
		return true;
	}
	return false;
}

void KitEngine::interact(bool shot) {
	if (!_viewArea.contains(_crossairPosition) || (shot && !(_kitVariables[20] & 1)))
		return;
	float x = 2.0f * (_crossairPosition.x - _viewArea.left) / _viewArea.width() - 1;
	float y = 1 - 2.0f * (_crossairPosition.y - _viewArea.top) / _viewArea.height();
	float projection = tan(Math::deg2rad(_fieldOfView) / 2);
	Math::Vector3d direction = directionToVector(_pitch + Math::rad2deg(atan(y * projection / _viewAspectRatio)),
		_yaw - Math::rad2deg(atan(x * projection)), false);
	Object *object = _currentArea->checkCollisionRay(Math::Ray(_position, direction), 8192, true);
	if (shot) {
		_kitVariables[21]++;
		if (_kitVariables[20] & 2)
			_shootingFrames = 3;
	}
	if (!object || !object->isGeometric())
		return;
	if (!shot) {
		float distance = 0;
		for (int axis = 0; axis < 3; axis++) {
			if (object->getSize().getValue(axis) * _currentArea->getScale() > 1000)
				return;
			distance += ABS(object->getOrigin().getValue(axis) + object->getSize().getValue(axis) / 2 - _position.getValue(axis));
		}
		if (distance > _activationRange)
			return;
	}
	executeObjectConditions(static_cast<GeometricObject *>(object), shot, false, !shot);
}

} // namespace Freescape
