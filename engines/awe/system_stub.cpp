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

#include "common/system.h"
#include "common/events.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "graphics/screen.h"
#include "awe/gfx.h"
#include "awe/metaengine.h"
#include "awe/system_stub.h"

namespace Awe {

struct SystemStubScummVM : SystemStub {
private:
	bool _isAnniversaryEdition;

public:
	static const int kJoystickIndex = 0;
	static const int kJoystickCommitValue = 16384;
	static const float kAspectRatio;

	::Graphics::Screen *_screen = nullptr;
	float _aspectRatio[4] = { 0.0 };
	int _screenshot = 0;

	SystemStubScummVM(bool isAnniversaryEdition) :
		_isAnniversaryEdition(isAnniversaryEdition) {}
	~SystemStubScummVM() override;

	void init(const DisplayMode &dm) override;
	void fini() override;

	void prepareScreen(int &w, int &h, float ar[4]) override;
	void updateScreen() override;
	void setScreenPixels(const Graphics::Surface &src) override;
	void setPalette(const Color pal[16]) override;

	void processEvents() override;
	void sleep(uint32 duration) override;
	virtual uint32 getTimeStamp() override;

	void setAspectRatio(int w, int h);
};

const float SystemStubScummVM::kAspectRatio = 16.f / 10.f;

SystemStubScummVM::~SystemStubScummVM() {
	delete _screen;
}

void SystemStubScummVM::init(const DisplayMode &dm) {
	// Initialize backend
	initGraphics(dm.width, dm.height, &Gfx::_format);
	_screen = new Graphics::Screen();

	_screenshot = 1;
	_dm = dm;
}

void SystemStubScummVM::fini() {
	delete _screen;
	_screen = nullptr;
}

void SystemStubScummVM::prepareScreen(int &w, int &h, float ar[4]) {
	w = _screen->w;
	h = _screen->h;

	ar[0] = _aspectRatio[0];
	ar[1] = _aspectRatio[1];
	ar[2] = _aspectRatio[2];
	ar[3] = _aspectRatio[3];
}

void SystemStubScummVM::updateScreen() {
	_screen->update();
}

void SystemStubScummVM::setScreenPixels(
	const Graphics::Surface &src) {
	assert(src.w == _screen->w && src.h == _screen->h);

	_screen->blitFrom(src);
}

void SystemStubScummVM::setPalette(const Color pal[16]) {
	for (int i = 0; i < 16; ++i)
		g_system->getPaletteManager()->setPalette(pal[i].rgb, i, 1);
}


void SystemStubScummVM::processEvents() {
	Common::Event ev;
	while (g_system->getEventManager()->pollEvent(ev)) {
		// WORKAROUND: Anniversary editions have a separate
		// jump button than the up arrow; for earlier releases,
		// remap the jump to up, which does the same thing
		if (!_isAnniversaryEdition &&
			ev.customType == KEYBIND_JUMP)
			ev.customType = KEYBIND_UP;

		switch (ev.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			_pi.quit = true;
			break;

		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			switch (ev.customType) {
			case KEYBIND_LEFT:
				_pi.dirMask |= PlayerInput::DIR_LEFT;
				break;
			case KEYBIND_RIGHT:
				_pi.dirMask |= PlayerInput::DIR_RIGHT;
				break;
			case KEYBIND_UP:
				_pi.dirMask |= PlayerInput::DIR_UP;
				break;
			case KEYBIND_DOWN:
				_pi.dirMask |= PlayerInput::DIR_DOWN;
				break;
			case KEYBIND_SELECT:
				_pi.action = true;
				break;
			case KEYBIND_JUMP:
				_pi.jump = true;
				break;
			case KEYBIND_CODE:
				_pi.code = true;
				break;
			default:
				break;
			}
			break;

		case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
			switch (ev.customType) {
			case KEYBIND_LEFT:
				_pi.dirMask &= ~PlayerInput::DIR_LEFT;
				break;
			case KEYBIND_RIGHT:
				_pi.dirMask &= ~PlayerInput::DIR_RIGHT;
				break;
			case KEYBIND_UP:
				_pi.dirMask &= ~PlayerInput::DIR_UP;
				break;
			case KEYBIND_DOWN:
				_pi.dirMask &= ~PlayerInput::DIR_DOWN;
				break;
			case KEYBIND_SELECT:
				_pi.action = false;
				break;
			case KEYBIND_JUMP:
				_pi.jump = false;
				break;
			case KEYBIND_CODE:
				_pi.code = false;
				break;
			default:
				break;
			}
			break;

		case Common::EVENT_KEYUP:
			switch (ev.kbd.keycode) {
			case Common::KEYCODE_p:
				_pi.pause = true;
				break;
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_AC_BACK:
				_pi.back = true;
				break;
			case Common::KEYCODE_AC_HOME:
				_pi.quit = true;
				break;
			default:
				break;
			}
			break;

		case Common::EVENT_KEYDOWN:
			if (ev.kbd.flags & Common::KBD_ALT) {
				if (ev.kbd.keycode == Common::KEYCODE_x) {
					_pi.quit = true;
				}
				break;
			} else if (ev.kbd.flags & Common::KBD_CTRL) {
				if (ev.kbd.keycode == Common::KEYCODE_f) {
					_pi.fastMode = true;
				}
				break;
			}

			_pi.lastChar = ev.kbd.keycode;
			break;

		default:
			break;
		}
	}
}

void SystemStubScummVM::sleep(uint32 duration) {
	g_system->delayMillis(duration);
}

uint32 SystemStubScummVM::getTimeStamp() {
	return g_system->getMillis();
}

void SystemStubScummVM::setAspectRatio(int w, int h) {
	const float currentAspectRatio = w / (float)h;
	if (int(currentAspectRatio * 100) == int(kAspectRatio * 100)) {
		_aspectRatio[0] = 0.f;
		_aspectRatio[1] = 0.f;
		_aspectRatio[2] = 1.f;
		_aspectRatio[3] = 1.f;
		return;
	}
	// pillar box
	if (currentAspectRatio > kAspectRatio) {
		const float inset = 1.f - kAspectRatio / currentAspectRatio;
		_aspectRatio[0] = inset / 2;
		_aspectRatio[1] = 0.f;
		_aspectRatio[2] = 1.f - inset;
		_aspectRatio[3] = 1.f;
		return;
	}
	// letter box
	if (currentAspectRatio < kAspectRatio) {
		const float inset = 1.f - currentAspectRatio / kAspectRatio;
		_aspectRatio[0] = 0.f;
		_aspectRatio[1] = inset / 2;
		_aspectRatio[2] = 1.f;
		_aspectRatio[3] = 1.f - inset;
		return;
	}
}

SystemStub *SystemStub_create(bool isAnniversaryEdition) {
	return new SystemStubScummVM(isAnniversaryEdition);
}

} // namespace Awe
