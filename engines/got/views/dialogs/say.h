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

#ifndef GOT_VIEWS_DIALOGS_SAY_H
#define GOT_VIEWS_DIALOGS_SAY_H

#include "got/gfx/gfx_pics.h"
#include "got/views/dialogs/dialog.h"

namespace Got {
namespace Views {
namespace Dialogs {

class Say : public Dialog {
	enum WaitResponse {
		WAIT_NONE,
		WAIT_MORE,
		WAIT_DONE
	};

private:
	Gfx::Pics _speakerIcon;
	int _item = 0;
	int _type = 0;
	int _picIndex = 0;
	const char *_content = nullptr;
	int _contentLength = 0;
	int _woopCtr = 0;
	int _contentCtr = 0;
	WaitResponse _waitForResponse = WAIT_NONE;

	/**
     * Advance to showing the entirety of the current page
     */
	void showEntirePage();

public:
	Say();
	virtual ~Say() {}

	static void show(int item, const Gfx::Pics &speakerIcon, int type);

	void draw() override;
	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool tick() override;
};

} // namespace Dialogs
} // namespace Views
} // namespace Got

#endif
