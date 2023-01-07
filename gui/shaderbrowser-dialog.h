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

#ifndef SHADERBROWSER_DIALOG_H
#define SHADERBROWSER_DIALOG_H

#include "gui/dialog.h"
#include "gui/widgets/edittext.h"

namespace GUI {

class ListWidget;
class EditTextWidget;
class CommandSender;

enum {
	kFBModeLoad = 0,
	kFBModeSave
};

class ShaderBrowserDialog : public Dialog {
public:
	ShaderBrowserDialog(const Common::String &initialSelection);

	void open() override;
	void reflowLayout() override;

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;

	Common::String getResult() { return Dialog::getResult() ? _fileName->getEditString().encode() : Common::String(); }

protected:
	EditTextWidget   *_fileName;
	ListWidget	     *_fileList;
	Common::SearchSet _shaderSet;
	Common::String	  _search;

	EditTextWidget   *_searchWidget;
#ifndef DISABLE_FANCY_THEMES
	GraphicsWidget	 *_searchPic;
#endif
	StaticTextWidget *_searchDesc;
	ButtonWidget     *_searchClearButton;

	void updateListing();
	void normalieFileName();
};

} // End of namespace GUI

#endif
