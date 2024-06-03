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

#include "ultima/ultima8/gumps/message_box_gump.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/gfx/fonts/font_manager.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/gfx/render_surface.h"
#include "ultima/ultima8/gfx/texture.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MessageBoxGump)

MessageBoxGump::MessageBoxGump()
	: ModalGump(), _titleColour(TEX32_PACK_RGB(0, 0, 0)) {

}

MessageBoxGump::MessageBoxGump(const Std::string &title, const Std::string &message, uint32 titleColour,
							   Std::vector<Std::string> *buttons) :
		ModalGump(0, 0, 100, 100), _title(title), _message(message), _titleColour(titleColour) {
	if (buttons)
		buttons->swap(_buttons);

	if (_buttons.empty())
		_buttons.push_back(Std::string("Ok"));
}

MessageBoxGump::~MessageBoxGump(void) {
}

#define MBG_PADDING 16

void MessageBoxGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	// work out sizes of the text
	Font *font = FontManager::get_instance()->getTTFont(0);

	int32 width, height;
	unsigned int rem;

	// Title width
	font->getStringSize(_title, width, height);
	int title_w = width;

	// Width of _buttons
	int buttons_w = MBG_PADDING;
	for (size_t i = 0; i < _buttons.size(); i++) {
		width = height = 0;
		font->getStringSize(_buttons[i], width, height);
		buttons_w += width + MBG_PADDING;
	}

	// Message size
	font->getTextSize(_message, width, height, rem);

	_dims.setWidth(MBG_PADDING + width + MBG_PADDING);
	if (_dims.width() < MBG_PADDING + title_w + MBG_PADDING) _dims.setWidth(MBG_PADDING + title_w + MBG_PADDING);
	if (_dims.width() < buttons_w) _dims.setWidth(buttons_w);

	_dims.setHeight(23 + MBG_PADDING + height + MBG_PADDING + 28);

	// Title
	Gump *w = new TextWidget(MBG_PADDING, 2, _title, false, 0);
	w->InitGump(this, false);

	// Message
	w = new TextWidget(MBG_PADDING, 23 + MBG_PADDING, _message, false, 0, width, height);
	w->InitGump(this, false);

	// Buttons (right aligned)
	int off = _dims.width() - buttons_w;
	for (size_t i = 0; i < _buttons.size(); i++) {
		w = new ButtonWidget(off, _dims.height() - 23, _buttons[i], false, 1, TEX32_PACK_RGBA(0xD0, 0x00, 0xD0, 0x80));
		w->SetIndex(static_cast<int32>(i));
		w->InitGump(this, false);
		width = height = 0;
		font->getStringSize(_buttons[i], width, height);
		off += width + MBG_PADDING;
	}

	Mouse *mouse = Mouse::get_instance();
	mouse->pushMouseCursor(Mouse::MOUSE_HAND);
}

void MessageBoxGump::Close(bool no_del) {
	Mouse *mouse = Mouse::get_instance();
	mouse->popMouseCursor();

	ModalGump::Close(no_del);
}

void MessageBoxGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool /*scaled*/) {
	// Background is partially transparent
	surf->fillBlended(TEX32_PACK_RGBA(0, 0, 0, 0x80), _dims);

	uint32 color = TEX32_PACK_RGB(0xFF, 0xFF, 0xFF);
	if (!IsFocus())
		color = TEX32_PACK_RGB(0x7F, 0x7F, 0x7F);

	// outer border
	surf->fill32(color, 0, 0, _dims.width(), 1);
	surf->fill32(color, 0, 0, 1, _dims.height());
	surf->fill32(color, 0, _dims.height() - 1, _dims.width(), 1);
	surf->fill32(color, _dims.width() - 1, 0, 1, _dims.height());

	// line above _buttons
	surf->fill32(color, 0, _dims.height() - 28, _dims.width(), 1);

	// line below _title
	surf->fill32(color, 0, 23, _dims.width(), 1);

	// Highlight behind _message
	color = IsFocus() ? _titleColour : TEX32_PACK_RGB(0, 0, 0);
	surf->fill32(color, 1, 1, _dims.width() - 2, 22);
}

void MessageBoxGump::ChildNotify(Gump *child, uint32 msg) {
	ButtonWidget *buttonWidget = dynamic_cast<ButtonWidget *>(child);
	if (buttonWidget && (msg == ButtonWidget::BUTTON_CLICK || msg == ButtonWidget::BUTTON_DOUBLE)) {
		_processResult = child->GetIndex();
		Close();
	}
}

ProcId MessageBoxGump::Show(Std::string _title, Std::string _message, uint32 titleColour, Std::vector<Std::string> *_buttons) {
	Gump *gump = new MessageBoxGump(_title, _message, titleColour, _buttons);
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);
	gump->CreateNotifier();
	return gump->GetNotifyProcess()->getPid();
}

void MessageBoxGump::saveData(Common::WriteStream *ws) {
	warning("Trying to save ModalGump");
}

bool MessageBoxGump::loadData(Common::ReadStream *rs, uint32 version) {
	warning("Trying to load ModalGump");

	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
