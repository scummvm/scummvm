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

#include "gui/dialog.h"
#include "common/array.h"

namespace GUI {
	class StaticTextWidget;
	class ScrollContainerWidget;
	class ButtonWidget;
}

class UnknownGameDialog : public GUI::Dialog {
public:
	UnknownGameDialog(const Common::String &reportData, const Common::String &reportTranslated, const Common::String &bugtrackerAffectedEngine);
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	virtual Common::String generateBugtrackerURL();
	virtual void reflowLayout();

protected:
	void rebuild();

private:
	Common::String _reportData;
	Common::String _reportTranslated;
	Common::String _bugtrackerGameData;
	Common::String _bugtrackerAffectedEngine;

	GUI::ScrollContainerWidget *_textContainer;
	Common::Array<GUI::StaticTextWidget*> _textWidgets;
	GUI::ButtonWidget* _openBugTrackerUrlButton;
	GUI::ButtonWidget* _copyToClipboardButton;
	GUI::ButtonWidget* _closeButton;
};
