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

#ifndef MM1_VIEWS_ENH_WHO_WILL_TRY_H
#define MM1_VIEWS_ENH_WHO_WILL_TRY_H

#include "mm/mm1/views_enh/party_view.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

typedef void (*WhoWillProc)(int charNum);

class WhoWillTry : public PartyView {
private:
	WhoWillProc _callback = nullptr;

	void selectChar(uint charNum);

protected:
	/**
	 * Return true if a character should be selected by default
	 */
	bool selectCharByDefault() const override {
		return false;
	}
public:
	WhoWillTry();
	virtual ~WhoWillTry() {}

	static void display(WhoWillProc callback);
	void open(WhoWillProc callback);

	bool msgGame(const GameMessage &msg) override;
	void draw() override;
	bool msgAction(const ActionMessage &msg) override;
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
