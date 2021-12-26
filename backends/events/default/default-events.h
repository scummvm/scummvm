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

#if !defined(BACKEND_EVENTS_DEFAULT_H) && !defined(DISABLE_DEFAULT_EVENTMANAGER)
#define BACKEND_EVENTS_DEFAULT_H

#include "common/events.h"
#include "common/queue.h"

namespace Common {
class Keymapper;
#ifdef ENABLE_VKEYBD
class VirtualKeyboard;
#endif
class VirtualMouse;
}


class DefaultEventManager : public Common::EventManager, Common::EventObserver {
#ifdef ENABLE_VKEYBD
	Common::VirtualKeyboard *_vk;
#endif

	Common::VirtualMouse *_virtualMouse;

	Common::Keymapper *_keymapper;

	Common::ArtificialEventSource _artificialEventSource;

	Common::Queue<Common::Event> _eventQueue;
	bool notifyEvent(const Common::Event &ev) override {
		_eventQueue.push(ev);
		return true;
	}

	Common::Point _mousePos;
	int _buttonState;
	int _modifierState;
	bool _shouldQuit;
	bool _shouldReturnToLauncher;
	bool _confirmExitDialogActive;

public:
	DefaultEventManager(Common::EventSource *boss);
	~DefaultEventManager();

	void init() override;
	bool pollEvent(Common::Event &event) override;
	void pushEvent(const Common::Event &event) override;
	void purgeMouseEvents() override;
	void purgeKeyboardEvents() override;

	Common::Point getMousePos() const override { return _mousePos; }
	int getButtonState() const override { return _buttonState; }
	int getModifierState() const override { return _modifierState; }
	int shouldQuit() const override { return _shouldQuit; }
	int shouldReturnToLauncher() const override { return _shouldReturnToLauncher; }
	void resetReturnToLauncher() override { _shouldReturnToLauncher = false; }
	void resetQuit() override { _shouldQuit = false; }

	Common::Keymapper *getKeymapper() override { return _keymapper; }
	Common::Keymap *getGlobalKeymap() override;
};

#endif
