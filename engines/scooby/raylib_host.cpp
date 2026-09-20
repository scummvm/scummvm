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

#include "raylib_host.h"

#include "common/error.h"

// #include <cstdio>

#include "common/events.h"
#include "common/system.h"
#include "graphics/screen.h"
#include "rendering/frame_viewport.h"

namespace Scooby {
RaylibHost::RaylibHost() : _rgbaPixels(static_cast<std::size_t>(IndexedFrame::Width) *
							  static_cast<std::size_t>(IndexedFrame::Height)) {}
	//   _textureValid(false), _windowOpen(false) {

Common::ScopedPtr<RaylibHost> RaylibHost::create() {
	Common::ScopedPtr<RaylibHost> host(new RaylibHost());
	if (!host->init()) {
		// host's destructor tears down whatever partially succeeded
		// (texture and/or window), mirroring the C# constructor's
		// try/catch/finally cleanup path.
		return nullptr;
	}
	return host;
}

bool RaylibHost::init() {
	// SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	// InitWindow(896, 672, "Scooby-Doo Mystery");
	// if (!IsWindowReady()) {
	// 	error("raylib did not create the Scooby-Doo Mystery window.\n");
	// 	return false;
	// }
	// _windowOpen = true;
	//
	// SetWindowMinSize(640, 480);
	// SetExitKey(KEY_NULL);
	//
	// Image image = GenImageColor(IndexedFrame::Width, IndexedFrame::Height, BLACK);
	// _texture = LoadTextureFromImage(image);
	// UnloadImage(image);
	//
	// if (!IsTextureValid(_texture)) {
	// 	error("raylib did not create the indexed frame texture.\n");
	// 	return false;
	// }
	// _textureValid = true;
	//
	// SetTextureFilter(_texture, TEXTURE_FILTER_POINT);
	// SetTextureWrap(_texture, TEXTURE_WRAP_CLAMP);
	return true;
}

RaylibHost::~RaylibHost() {
	// if (_textureValid) {
	// 	UnloadTexture(_texture);
	// 	_textureValid = false;
	// }
	// if (_windowOpen) {
	// 	CloseWindow();
	// 	_windowOpen = false;
	// }
}

ControllerButtons RaylibHost::getControllerButtons(ControllerButtons backButton) {
	Common::Event e;
	while (g_system->getEventManager()->pollEvent(e)) {
		switch (e.type) {
		case Common::EVENT_KEYDOWN : {
			if (e.kbd.keycode == Common::KEYCODE_UP) {
				_buttons |= ControllerButtons::Up;
			}
			if (e.kbd.keycode == Common::KEYCODE_DOWN) {
				_buttons |= ControllerButtons::Down;
			}
			if (e.kbd.keycode == Common::KEYCODE_LEFT) {
				_buttons |= ControllerButtons::Left;
			}
			if (e.kbd.keycode == Common::KEYCODE_RIGHT) {
				_buttons |= ControllerButtons::Right;
			}
			if (e.kbd.keycode == Common::KEYCODE_z) {
				_buttons |= ControllerButtons::A;
			}
			if (e.kbd.keycode == Common::KEYCODE_x) {
				_buttons |= ControllerButtons::B;
			}
			if (e.kbd.keycode == Common::KEYCODE_c) {
				_buttons |= ControllerButtons::C;
			}
			if (e.kbd.keycode == Common::KEYCODE_RETURN) {
				_buttons |= ControllerButtons::Start;
			}
			if (e.kbd.keycode == Common::KEYCODE_ESCAPE) {
				_buttons |= backButton;
			}
			break;
		}
		case Common::EVENT_KEYUP : {
			if (e.kbd.keycode == Common::KEYCODE_UP) {
				_buttons &= buttonMask(ControllerButtons::Up);
			}
			if (e.kbd.keycode == Common::KEYCODE_DOWN) {
				_buttons &= buttonMask(ControllerButtons::Down);
			}
			if (e.kbd.keycode == Common::KEYCODE_LEFT) {
				_buttons &= buttonMask(ControllerButtons::Left);
			}
			if (e.kbd.keycode == Common::KEYCODE_RIGHT) {
				_buttons &= buttonMask(ControllerButtons::Right);
			}
			if (e.kbd.keycode == Common::KEYCODE_z) {
				_buttons &= buttonMask(ControllerButtons::A);
			}
			if (e.kbd.keycode == Common::KEYCODE_x) {
				_buttons &= buttonMask(ControllerButtons::B);
			}
			if (e.kbd.keycode == Common::KEYCODE_c) {
				_buttons &= buttonMask(ControllerButtons::C);
			}
			if (e.kbd.keycode == Common::KEYCODE_RETURN) {
				_buttons &= buttonMask(ControllerButtons::Start);
			}
			if (e.kbd.keycode == Common::KEYCODE_ESCAPE) {
				_buttons |= backButton;
			}
		}
		}
	}
	return getGamepadButtons() | getKeyboardAndMouseButtons(_buttons);
}

ControllerButtons RaylibHost::getKeyboardAndMouseButtons(ControllerButtons backButton) {
	return backButton;
	// ControllerButtons buttons = ControllerButtons::None;
	// if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
	// 	buttons |= ControllerButtons::Up;
	// }
	// if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
	// 	buttons |= ControllerButtons::Down;
	// }
	// if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
	// 	buttons |= ControllerButtons::Left;
	// }
	// if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
	// 	buttons |= ControllerButtons::Right;
	// }
	// if (IsKeyDown(KEY_Y) || IsKeyDown(KEY_Z) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
	// 	buttons |= ControllerButtons::A;
	// }
	// if (IsKeyDown(KEY_X) || IsKeyDown(KEY_SPACE) || IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
	// 	buttons |= ControllerButtons::B;
	// }
	// if (IsKeyDown(KEY_C) || IsKeyDown(KEY_TAB) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
	// 	buttons |= ControllerButtons::C;
	// }
	// if (IsKeyDown(KEY_ESCAPE) || IsKeyDown(KEY_BACKSPACE)) {
	// 	buttons |= backButton;
	// }
	// if (IsKeyDown(KEY_ENTER)) {
	// 	buttons |= ControllerButtons::Start;
	// }

	// return buttons;
}

ControllerButtons RaylibHost::getGamepadButtons() {
	ControllerButtons buttons = ControllerButtons::None;
	// if (!IsGamepadAvailable(0)) {
	// 	return buttons;
	// }
	//
	// if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) {
	// 	buttons |= ControllerButtons::Up;
	// }
	// if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {
	// 	buttons |= ControllerButtons::Down;
	// }
	// if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) {
	// 	buttons |= ControllerButtons::Left;
	// }
	// if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) {
	// 	buttons |= ControllerButtons::Right;
	// }
	// if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) {
	// 	buttons |= ControllerButtons::A;
	// }
	// if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
	// 	buttons |= ControllerButtons::B;
	// }
	// if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
	// 	buttons |= ControllerButtons::C;
	// }
	// if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) {
	// 	buttons |= ControllerButtons::Start;
	// }

	return buttons;
}

void RaylibHost::present(const IndexedFrame &frame) {
	byte pal[64 *3];
	for (int i = 0; i < 64; i++) {
		pal[i*3] = expandLevel(frame._palette[i].redLevel);
		pal[i*3+1] = expandLevel(frame._palette[i].greenLevel);
		pal[i*3+2] = expandLevel(frame._palette[i].blueLevel);
	}
	frame._screen.setPalette(pal, 0, 64);

	frame._screen.copyRectToSurface(frame._pixels.data(), frame.Width, 0, 0, frame.Width, frame.Height);
	frame._screen.makeAllDirty();
	frame._screen.update();
	// for (std::size_t pixelIndex = 0; pixelIndex < _rgbaPixels.size(); ++pixelIndex) {
	// 	const PaletteColor &color = frame._palette[frame._pixels[pixelIndex]];
	// 	Color expanded;
	// 	expanded.r = expandLevel(color.redLevel);
	// 	expanded.g = expandLevel(color.greenLevel);
	// 	expanded.b = expandLevel(color.blueLevel);
	// 	expanded.a = 255;
	// 	_rgbaPixels[pixelIndex] = expanded;
	// }
	//
	// UpdateTexture(_texture, _rgbaPixels.data());
	// BeginDrawing();
	// ClearBackground(BLACK);
	// FrameViewport::fromCurrentWindow().draw(_texture);
	// EndDrawing();
}

uint8 RaylibHost::expandLevel(uint8 level) {
	return static_cast<uint8>((level << 5) | (level << 2) | (level >> 1));
}
} // namespace Scooby