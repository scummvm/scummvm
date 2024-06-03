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

#ifndef ULTIMA8_GUMPS_COMPUTERGUMP_H
#define ULTIMA8_GUMPS_COMPUTERGUMP_H

#include "ultima/ultima8/gumps/modal_gump.h"
#include "ultima/ultima8/gfx/fonts/rendered_text.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/classtype.h"

namespace Ultima {
namespace Ultima8 {

/**
 * The gump for showing the computer with text in Crusader
 */
class ComputerGump : public ModalGump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	ComputerGump();
	ComputerGump(const Std::string &msg);
	~ComputerGump() override;

	// Close on mouse click on key press
	Gump *onMouseDown(int button, int32 mx, int32 my) override;
	bool OnKeyDown(int key, int mod) override;

	// Init the gump, call after construction
	void InitGump(Gump *newparent, bool take_focus = true) override;

	void run() override;

	void Paint(RenderSurface *, int32 lerp_factor, bool scaled) override;

	INTRINSIC(I_readComputer);

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

private:
	void nextScreen();

	bool nextChar();

	RenderedText *_renderedLines[14];

	Common::Array<Common::String> _textLines;

	//! The current line from the full text
	uint32 _curTextLine;

	//! The current line in the rendered lines array
	uint32 _curDisplayLine;

	//! The current char within the current line
	uint32 _charOff;

	//! The frame when the next character will be added
	uint32 _nextCharTick;

	//! Tick now (timed separately to the kernel as this is run when game is paused)
	uint32 _tick;

	//! Whether display is currently paused waiting for input (with "MORE" at the bottom)
	bool _paused;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
