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

#include "ultima8/misc/pent_include.h"
#include "ultima8/gumps/message_box_gump.h"

#include "ultima8/gumps/widgets/text_widget.h"
#include "ultima8/gumps/widgets/button_widget.h"
#include "ultima8/games/game_data.h"
#include "ultima8/graphics/shape.h"
#include "ultima8/graphics/gump_shape_archive.h"
#include "ultima8/graphics/shape_frame.h"
#include "ultima8/usecode/uc_machine.h"
#include "ultima8/gumps/gump_notify_process.h"
#include "ultima8/world/item.h"
#include "ultima8/world/get_object.h"
#include "ultima8/kernel/core_app.h"
#include "ultima8/games/game_info.h"
#include "ultima8/misc/util.h"
#include "ultima8/graphics/fonts/font_manager.h"
#include "ultima8/graphics/fonts/font.h"
#include "ultima8/ultima8.h"
#include "ultima8/graphics/render_surface.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MessageBoxGump, ModalGump)

MessageBoxGump::MessageBoxGump()
	: ModalGump() {

}

MessageBoxGump::MessageBoxGump(const std::string &title_, const std::string &message_, uint32 title_colour_,
                               std::vector<std::string> *buttons_) :
		ModalGump(0, 0, 100, 100), title(title_), message(message_), title_colour(title_colour_) {
	if (buttons_)
		buttons_->swap(buttons);
	
	if (buttons.empty())
		buttons.push_back(std::string("Ok"));
}

MessageBoxGump::~MessageBoxGump(void) {
}

#define MBG_PADDING 16

void MessageBoxGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	// work out sizes of the text
	Pentagram::Font *font = FontManager::get_instance()->getTTFont(0);

	int width;
	int height;
	unsigned int rem;

	// Title width
	font->getStringSize(title, width, height);
	int title_w = width;

	// Width of buttons
	int buttons_w = MBG_PADDING;
	for (size_t i = 0; i < buttons.size(); i++) {
		width = height = 0;
		font->getStringSize(buttons[i], width, height);
		buttons_w += width + MBG_PADDING;
	}

	// Message size
	font->getTextSize(message, width, height, rem);

	dims.w = MBG_PADDING + width + MBG_PADDING;
	if (dims.w < MBG_PADDING + title_w + MBG_PADDING) dims.w = MBG_PADDING + title_w + MBG_PADDING;
	if (dims.w < buttons_w) dims.w = buttons_w;

	dims.h = 23 + MBG_PADDING + height + MBG_PADDING + 28;

	// Title
	Gump *w = new TextWidget(MBG_PADDING, 2, title, false, 0);
	w->InitGump(this, false);

	// Message
	w = new TextWidget(MBG_PADDING, 23 + MBG_PADDING, message, false, 0, width, height);
	w->InitGump(this, false);

	// Buttons (right aligned)
	int off = dims.w - buttons_w;
	for (size_t i = 0; i < buttons.size(); i++) {
		w = new ButtonWidget(off, dims.h - 23, buttons[i], false, 1, 0x80D000D0);
		w->SetIndex(static_cast<int32>(i));
		w->InitGump(this, false);
		width = height = 0;
		font->getStringSize(buttons[i], width, height);
		off += width + MBG_PADDING;
	}

	Ultima8Engine *guiapp = Ultima8Engine::get_instance();
	guiapp->pushMouseCursor();
	guiapp->setMouseCursor(Ultima8Engine::MOUSE_POINTER);
}

void MessageBoxGump::Close(bool no_del) {
	Ultima8Engine *guiapp = Ultima8Engine::get_instance();
	guiapp->popMouseCursor();

	ModalGump::Close(no_del);
}

void MessageBoxGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool /*scaled*/) {
	// Background is partially transparent
	surf->FillBlended(0x80000000, 0, 0, dims.w, dims.h);

	uint32 line_colour = 0xFFFFFFFF;
	if (!IsFocus()) line_colour = 0xFF7F7F7F;

	// outer border
	surf->Fill32(line_colour, 0, 0, dims.w, 1);
	surf->Fill32(line_colour, 0, 0, 1, dims.h);
	surf->Fill32(line_colour, 0, dims.h - 1, dims.w, 1);
	surf->Fill32(line_colour, dims.w - 1, 0, 1, dims.h);

	// line above buttons
	surf->Fill32(line_colour, 0, dims.h - 28, dims.w, 1);

	// line below title
	surf->Fill32(line_colour, 0, 23, dims.w, 1);

	// Highlight behind message..
	if (IsFocus()) surf->Fill32(title_colour, 1, 1, dims.w - 2, 22);
	else surf->Fill32(0xFF000000, 1, 1, dims.w - 2, 22);
}

void MessageBoxGump::ChildNotify(Gump *child, uint32 msg) {
	if (child->IsOfType<ButtonWidget>() && msg == ButtonWidget::BUTTON_CLICK) {
		process_result = child->GetIndex();
		Close();
	}
}

ProcId MessageBoxGump::Show(std::string title, std::string message, uint32 title_colour, std::vector<std::string> *buttons) {
	Gump *gump = new MessageBoxGump(title, message, title_colour, buttons);
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);
	gump->CreateNotifier();
	return gump->GetNotifyProcess()->getPid();
}

void MessageBoxGump::saveData(ODataSource *ods) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");
}

bool MessageBoxGump::loadData(IDataSource *ids, uint32 version) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");

	return false;
}

} // End of namespace Ultima8
