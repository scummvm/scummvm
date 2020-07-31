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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/gumps/keypad_gump.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/gumps/desktop_gump.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(KeypadGump)

KeypadGump::KeypadGump(int targetValue): ModalGump(0, 0, 5, 5),
		_value(0), _targetValue(targetValue) {
	Mouse *mouse = Mouse::get_instance();
	mouse->pushMouseCursor();
	mouse->setMouseCursor(Mouse::MOUSE_HAND);
	for (int i = 0; i < 12; i++) {
		_buttons[i] = 0;
	}
}

KeypadGump::~KeypadGump() {
	Mouse::get_instance()->popMouseCursor();
}

void KeypadGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	_shape = GameData::get_instance()->getGumps()->getShape(10);
	UpdateDimsFromShape();

	static const int buttonShapeNum = 11;
	static const uint16 xoffs[] = {0xc, 0x27, 0x42};
	static const uint16 yoffs[] = {0x19, 0x32, 0x4a, 0x62};

	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 3; x++) {
			int bnum = y * 3 + x;
			FrameID button_up(GameData::GUMPS, buttonShapeNum, bnum);
			FrameID button_down(GameData::GUMPS, buttonShapeNum, bnum + 12);
			Gump *widget = new ButtonWidget(xoffs[x], yoffs[y], button_up, button_down);
			widget->InitGump(this);
			widget->SetIndex(bnum);
			_buttons[bnum] = widget->getObjId();
		}
	}
}

void KeypadGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);
	// TODO: paint text version of _value.
}

bool KeypadGump::OnKeyDown(int key, int mod) {
	switch (key) {
	case Common::KEYCODE_ESCAPE: {
		Close();
	}
	break;
	default:
		break;
	}

	return true;
}

void KeypadGump::ChildNotify(Gump *child, uint32 message) {
	//ObjId cid = child->getObjId();
	if (message == ButtonWidget::BUTTON_CLICK) {
		uint16 sfxno = 0;
		int buttonNo = child->GetIndex();
		if (buttonNo < 9) {
			_value *= 10;
			_value += buttonNo + 1;
			sfxno = 0x3b;
		} else if (buttonNo == 10) {
			_value *= 10;
			sfxno = 0x3b;
		} else if (buttonNo == 9) {
			_value /= 10;
			sfxno = 0x3a;
		} else if (buttonNo == 11) {
			SetResult(_value);
			// TODO: Do something as a result of this other than just play a sound.
			if (_value == _targetValue) {
				sfxno = 0x32;
				Close();
				// Note: careful, this is now deleted.
			} else {
				// wrong.
				sfxno = 0x31;
				_value = 0;
			}
		}
		AudioProcess *audio = AudioProcess::get_instance();
		if (audio && sfxno)
			audio->playSFX(sfxno, 0x10, _objId, 1);
	}
}

bool KeypadGump::OnTextInput(int unicode) {
	if (!(unicode & 0xFF80)) {
		//char c = unicode & 0x7F;
		// TODO: Accept numeric keyboard inputs
	}
	return true;
}

uint32 KeypadGump::I_showKeypad(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(target)
	ModalGump *gump = new KeypadGump(target);
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);
	gump->CreateNotifier();

	return gump->GetNotifyProcess()->getPid();
}

bool KeypadGump::loadData(Common::ReadStream *rs) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");
	return true;
}

void KeypadGump::saveData(Common::WriteStream *ws) {
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}

} // End of namespace Ultima8
} // End of namespace Ultima
