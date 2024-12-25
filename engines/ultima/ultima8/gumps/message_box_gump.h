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

#ifndef ULTIMA8_GUMPS_MESSAGEBOXGUMP_H
#define ULTIMA8_GUMPS_MESSAGEBOXGUMP_H

#include "ultima/ultima8/gumps/modal_gump.h"
#include "ultima/ultima8/usecode/intrinsics.h"

#include "ultima/shared/std/string.h"
#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/misc/classtype.h"
#include "ultima/ultima8/gfx/texture.h"

namespace Ultima {
namespace Ultima8 {

/**
 * A modal message box (for errors, etc)
 * In Crusader, this should be used for things like the "Targeting reticle [in]active." message
 */
class MessageBoxGump : public ModalGump {
	Std::string _title;
	Std::string _message;
	Std::vector<Std::string> _buttons;
	int _titleColour;
public:
	ENABLE_RUNTIME_CLASSTYPE()

	MessageBoxGump();
	MessageBoxGump(const Std::string &title, const Std::string &message, uint32 title_colour, Std::vector<Std::string> *buttons);
	~MessageBoxGump() override;

	// Init the gump, call after construction
	void InitGump(Gump *newparent, bool take_focus = true) override;
	void Close(bool no_del = false) override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled) override;

	//! Create a Message Box
	//! \param title Title of the message box
	//! \param message Message to be displayed in box
	//! \param titleColour The colour to be displayed behind the title bar
	//! \param buttons Array of button names to be displayed. Default is "Ok"
	//! \return Pid of process that will have the result when finished
	static ProcId Show(Std::string title, Std::string message, uint32 titleColour, Std::vector<Std::string> *buttons = 0);
	static ProcId Show(Std::string title, Std::string message, Std::vector<Std::string> *buttons) {
		return Show(title, message, TEX32_PACK_RGB(0x30, 0x30, 0x8F), buttons);
	}

	void ChildNotify(Gump *child, uint32 msg) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
