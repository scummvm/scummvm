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
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/gumps/desktop_gump.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/usecode/uc_process.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(KeypadGump)

static const int TXT_CONTAINER_IDX = 0x100;
// Actually the max val where we will allow another digit to be entered
static const int MAX_CODE_VAL = 9999999;
static const int CHEAT_CODE_VAL = 74697689;

KeypadGump::KeypadGump(int targetValue, uint16 ucnotifypid): ModalGump(0, 0, 5, 5),
		_value(0), _targetValue(targetValue), _ucNotifyPid(ucnotifypid) {
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
	// Default result is 0xff
	SetResult(0xff);
}

void KeypadGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);
	// TODO: paint text version of _value.
}

bool KeypadGump::OnKeyDown(int key, int mod) {
	switch (key) {
	case Common::KEYCODE_ESCAPE: {
		_value = 0xff;
		Close();
		break;
	}
	case Common::KEYCODE_0:
	case Common::KEYCODE_1:
	case Common::KEYCODE_2:
	case Common::KEYCODE_3:
	case Common::KEYCODE_4:
	case Common::KEYCODE_5:
	case Common::KEYCODE_6:
	case Common::KEYCODE_7:
	case Common::KEYCODE_8:
	case Common::KEYCODE_9: {
		onDigit(key - (int)Common::KEYCODE_0);
		updateDigitDisplay();
		AudioProcess *audio = AudioProcess::get_instance();
		if (audio)
			audio->playSFX(0x3b, 0x10, _objId, 1);
		break;
	}
	break;
	default:
		break;
	}

	return true;
}

void KeypadGump::onDigit(int digit) {
	assert(digit >= 0 && digit <= 9);
	if (_value < MAX_CODE_VAL) {
		_value *= 10;
		_value += digit;
	}
}

void KeypadGump::ChildNotify(Gump *child, uint32 message) {
	//ObjId cid = child->getObjId();
	bool update = true;
	if (message == ButtonWidget::BUTTON_CLICK) {
		uint16 sfxno = 0x3b;
		int buttonNo = child->GetIndex();
		if (buttonNo < 9) {
			onDigit(buttonNo + 1);
		} else if (buttonNo == 10) {
			onDigit(0);
		} else if (buttonNo == 9) {
			_value /= 10;
			sfxno = 0x3a;
		} else if (buttonNo == 11) {
			update = false;
			if (_value == _targetValue || _value == CHEAT_CODE_VAL) {
				sfxno = 0x32;
				SetResult(_value);
			} else {
				// wrong.
				sfxno = 0x31;
				SetResult(0);
			}
			Close();
			// Note: careful, this is now deleted.
		}
		AudioProcess *audio = AudioProcess::get_instance();
		if (audio && sfxno)
			audio->playSFX(sfxno, 0x10, _objId, 1);
	}
	if (update) {
		updateDigitDisplay();
	}
}

void KeypadGump::updateDigitDisplay() {
	Gump *txt = Gump::FindGump(&FindByIndex<TXT_CONTAINER_IDX>);
	if (txt)
		txt->Close();
	txt = new Gump(25, 12, 200, 12);
	txt->InitGump(this);
	txt->SetIndex(TXT_CONTAINER_IDX);

	Std::vector<Gump *> digits;
	Shape *digitshape = GameData::get_instance()->getGumps()->getShape(12);
	int val = _value;
	while (val) {
		int digitval = val % 10;
		if (digitval == 0)
			digitval = 10;
		Gump *digit = new Gump(0, 0, 6, 12);
		digit->SetShape(digitshape, digitval - 1);
		digit->InitGump(txt);
		digits.push_back(digit);
		val /= 10;
	}

	int xoff = 0;
	while (digits.size()) {
		Gump *digit = digits.back();
		digits.pop_back();
		digit->setRelativePosition(TOP_LEFT, xoff);
		xoff += 6;
	}
}

void KeypadGump::Close(bool no_del) {
	_processResult = _value;

	if (_ucNotifyPid) {
		UCProcess *ucp = dynamic_cast<UCProcess *>(Kernel::get_instance()->getProcess(_ucNotifyPid));
		assert(ucp);
		ucp->setReturnValue(_value);
		ucp->wakeUp(_value);
	}

	ModalGump::Close(no_del);
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

	UCProcess *current = dynamic_cast<UCProcess *>(Kernel::get_instance()->getRunningProcess());
	assert(current);

	ModalGump *gump = new KeypadGump(target, current->getPid());
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);

	current->suspend();

	return 0;
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
