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

#ifndef MM1_VIEWS_ENH_SELECT_NUMBER_H
#define MM1_VIEWS_ENH_SELECT_NUMBER_H

#include "mm/mm1/views_enh/scroll_view.h"
#include "mm/shared/xeen/sprites.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

typedef void (*SelectNumberProc)(int choiceNum);

class SelectNumberSubview : public ScrollView {
private:
	SelectNumberProc _callback = nullptr;
	int _maxNumber = 0;

public:
	SelectNumberSubview();
	virtual ~SelectNumberSubview() {}

	void open(int maxNum, SelectNumberProc callback);
	void close() override;

	void draw() override;
	bool msgMouseDown(const MouseDownMessage &msg) override;
};

class SelectNumber : public ScrollView {
private:
	SelectNumberSubview _subView;

protected:
	/**
	 * Start displaying the numeric choices subview
	 */
	void openNumbers(int maxNum);

	/**
	 * Stop displaying the numeric choices subview
	 */
	void closeNumbers() {
		_subView.close();
	}

public:
	SelectNumber(const Common::String &name) : ScrollView(name) {}
	virtual ~SelectNumber() {}

	bool msgMouseDown(const MouseDownMessage &msg) override;
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
