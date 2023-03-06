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

#ifndef SCUMM_DIALOG_SESSION_SELECTOR_H
#define SCUMM_DIALOG_SESSION_SELECTOR_H

#include "gui/dialog.h"
#include "gui/widgets/list.h"
#include "common/fs.h"
#include "common/hashmap.h"
#include "common/stack.h"
#include "common/str.h"

#include "scumm/he/intern_he.h"
#include "scumm/he/net/net_main.h"
#include "scumm/he/net/net_defines.h"

namespace Scumm {

class StaticTextWidget;

class SessionSelectorDialog : public Dialog {
public:
	SessionSelectorDialog(Scumm::ScummEngine_v90he *vm);

	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
	void handleTickle() override;

private:
	ScummEngine_v90he *_vm;

	uint32 _timestamp;

	GUI::Widget *_joinButton;
	GUI::StaticTextWidget *_queryProgressText;

	GUI::ListWidget *_list;
};


} // End of namespace Scumm

#endif
