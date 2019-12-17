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

#ifndef ULTIMA8_GUMPS_MESSAGEBOXGUMP_H
#define ULTIMA8_GUMPS_MESSAGEBOXGUMP_H

#include "ultima/ultima8/gumps/modal_gump.h"
#include "ultima/ultima8/usecode/intrinsics.h"

#include "ultima/ultima8/std/containers.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class MessageBoxGump : public ModalGump {
	std::string title;
	std::string message;
	std::vector<std::string> buttons;
	int title_colour;
public:
	ENABLE_RUNTIME_CLASSTYPE()

	MessageBoxGump();
	MessageBoxGump(const std::string &title, const std::string &message, uint32 title_colour, std::vector<std::string> *buttons);
	virtual ~MessageBoxGump();

	// Init the gump, call after construction
	virtual void InitGump(Gump *newparent, bool take_focus = true);
	virtual void Close(bool no_del = false);

	bool loadData(IDataSource *ids, uint32 version);

	virtual void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled);

	//! Create a Message Box
	//! \param title Title of the message box
	//! \param message Message to be displayed in box
	//! \param title_colour The colour to be displayed behind the title bar
	//! \param buttons Array of button names to be displayed. Default is "Ok"
	//! \return Pid of process that will have the result when finished
	static ProcId Show(std::string title, std::string message, uint32 title_colour = 0xFF30308F, std::vector<std::string> *buttons = 0);
	static ProcId Show(std::string title, std::string message, std::vector<std::string> *buttons) {
		return Show(title, message, 0xFF30308F, buttons);
	}

	virtual void ChildNotify(Gump *child, uint32 msg);

protected:
	virtual void saveData(ODataSource *ods);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
