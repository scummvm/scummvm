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

#ifndef SCOOBY_RAYLIB_HOST_H
#define SCOOBY_RAYLIB_HOST_H

#include "common/ptr.h"

#include "common/array.h"

#include "scooby/graphics/indexed_frame.h"
#include "scooby/input/controller_buttons.h"

// Owns the process-global raylib window, GPU texture, native input sampling,
// event pump, and indexed-frame expansion.

namespace Scooby {
typedef struct Color {
	unsigned char r;        // Color red value
	unsigned char g;        // Color green value
	unsigned char b;        // Color blue value
	unsigned char a;        // Color alpha value
} Color;

class RaylibHost {
public:
	// Opens a resizable 4:3 window and creates one point-filtered
	// logical-frame texture. Returns nullptr (after printing a diagnostic to
	// stderr) if raylib could not create the window or texture -- this
	// project has no exceptions to throw out of a failed constructor.
	static Common::ScopedPtr<RaylibHost> create();

	~RaylibHost();

	RaylibHost(const RaylibHost &) = delete;
	RaylibHost &operator=(const RaylibHost &) = delete;

	// Gets whether the native close control requested process shutdown.
	bool shouldClose() const { return false; } // WindowShouldClose(); }

	// Consumes one Escape press used to skip exactly one current intro
	// presentation. True only on the key-down edge reported by raylib.
	bool isIntroSkipPressed() const { return false; } // IsKeyPressed(KEY_ESCAPE); }

	// Samples the backend-neutral player controls from keyboard, mouse
	// buttons, and gamepad zero.
	Scooby::ControllerButtons getControllerButtons(Scooby::ControllerButtons backButton);

	// Expands one indexed frame and submits it through the native event pump.
	void present(const Scooby::IndexedFrame &frame);

private:
	RaylibHost();

	// Returns false (leaving the object safely destructible either way) if
	// the window or texture could not be created.
	bool init();

	static Scooby::ControllerButtons getKeyboardAndMouseButtons(Scooby::ControllerButtons backButton);
	static Scooby::ControllerButtons getGamepadButtons();
	static uint8 expandLevel(uint8 level);

	// Texture2D _texture;
	Common::Array<Scooby::Color> _rgbaPixels;
	bool _textureValid;
	bool _windowOpen;
	ControllerButtons _buttons = ControllerButtons::None;
};
} // namespace Scooby

#endif // SCOOBY_RAYLIB_HOST_H
