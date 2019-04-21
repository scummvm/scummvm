/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BLADERUNNER_KIA_SECTION_BASE_H
#define BLADERUNNER_KIA_SECTION_BASE_H

#include "common/scummsys.h" // for "override" keyword

namespace Common {
struct KeyState;
}

namespace Graphics {
struct Surface;
}

namespace BladeRunner {

class BladeRunnerEngine;

class KIASectionBase {
protected:
	BladeRunnerEngine *_vm;

public:
	bool _scheduledSwitch;

	KIASectionBase(BladeRunnerEngine *vm);
	virtual ~KIASectionBase();

	virtual void open() {}
	virtual void close() {}

	virtual void draw(Graphics::Surface &surface) {}

	virtual void handleKeyUp(const Common::KeyState &kbd) {}
	virtual void handleKeyDown(const Common::KeyState &kbd) {}
	virtual void handleMouseMove(int mouseX, int mouseY) {}
	virtual void handleMouseDown(bool mainButton) {}
	virtual void handleMouseUp(bool mainButton) {}
	virtual void handleMouseScroll(int direction) {} // Added by ScummVM team

protected:
	virtual void onButtonPressed(int buttonId) {}

};

} // End of namespace BladeRunner

#endif
