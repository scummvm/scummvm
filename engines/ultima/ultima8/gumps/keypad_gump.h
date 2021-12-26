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

#ifndef ULTIMA8_GUMPS_KEYPADGUMP_H
#define ULTIMA8_GUMPS_KEYPADGUMP_H

#include "ultima/ultima8/gumps/modal_gump.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/classtype.h"

namespace Ultima {
namespace Ultima8 {

/**
 * The keypad with numbers gump
 */
class KeypadGump : public ModalGump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	KeypadGump(int targetValue, uint16 ucnotifypid);
	~KeypadGump() override;

	void Close(bool no_del=false) override;

	void InitGump(Gump *newparent, bool take_focus = true) override;

	void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled) override;

	bool OnKeyDown(int key, int mod) override;
	bool OnTextInput(int unicode) override;
	void ChildNotify(Gump *child, uint32 message) override;

	INTRINSIC(I_showKeypad);

	bool loadData(Common::ReadStream *rs);
	void saveData(Common::WriteStream *ws) override;

protected:
	void updateDigitDisplay();

	void onDigit(int digit);

	ObjId _buttons[12];

	int _value;
	int _targetValue;
	uint16 _ucNotifyPid;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
