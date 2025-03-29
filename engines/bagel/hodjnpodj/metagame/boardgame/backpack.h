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

#ifndef HODJNPODJ_METAGAME_Backpack_H
#define HODJNPODJ_METAGAME_Backpack_H

#include "bagel/hodjnpodj/metagame/views/dialog.h"
#include "bagel/hodjnpodj/gfx/button.h"
#include "bagel/hodjnpodj/metagame/bgen/invent.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

class Backpack : public Dialog {
private:
	OkButton _okButton;
	int nFirstSlot = 0;
	int nItemsPerRow = 0;
	int nItemsPerColumn = 0;
	CInventory *pInventory = nullptr;

public:
	Backpack();
	~Backpack() override {}

	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg);
	void draw() override;
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
