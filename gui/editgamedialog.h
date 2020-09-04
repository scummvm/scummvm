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

#ifndef GUI_EDITGAMEDIALOG_H
#define GUI_EDITGAMEDIALOG_H

#include "engines/game.h"
#include "gui/dialog.h"
#include "gui/options.h"
#include "gui/widget.h"

namespace GUI {

class BrowserDialog;
class CommandSender;
class DomainEditTextWidget;
class ListWidget;
class ButtonWidget;
class PicButtonWidget;
class GraphicsWidget;
class StaticTextWidget;
class EditTextWidget;
class SaveLoadChooser;

/*
* A dialog that allows the user to edit a config game entry.
* TODO: add widgets for some/all of the following
* - Maybe scaler/graphics mode. But there are two problems:
*   1) Different backends can have different scalers with different names,
*      so we first have to add a way to query those... no Ender, I don't
*      think a bitmasked property() value is nice for this,  because we would
*      have to add to the bitmask values whenever a backends adds a new scaler).
*   2) At the time the launcher is running, the GFX backend is already setup.
*      So when a game is run via the launcher, the custom scaler setting for it won't be
*      used. So we'd also have to add an API to change the scaler during runtime
*      (the SDL backend can already do that based on user input, but there is no API
*      to achieve it)
*   If the APIs for 1&2 are in place, we can think about adding this to the Edit&Option dialogs
*/

class EditGameDialog : public OptionsDialog {
	typedef Common::String String;
	typedef Common::U32String U32String;
	typedef Common::Array<Common::String> StringArray;
public:
	EditGameDialog(const String &domain);

	void open() override;
	void apply() override;
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;

protected:
	void setupGraphicsTab() override;

	EditTextWidget *_descriptionWidget;
	DomainEditTextWidget *_domainWidget;

	StaticTextWidget *_gamePathWidget;
	StaticTextWidget *_extraPathWidget;
	StaticTextWidget *_savePathWidget;
	ButtonWidget *_extraPathClearButton;
	ButtonWidget *_savePathClearButton;

	StaticTextWidget *_langPopUpDesc;
	PopUpWidget *_langPopUp;
	StaticTextWidget *_platformPopUpDesc;
	PopUpWidget *_platformPopUp;

	CheckboxWidget *_globalGraphicsOverride;
	CheckboxWidget *_globalShaderOverride;
	CheckboxWidget *_globalAudioOverride;
	CheckboxWidget *_globalMIDIOverride;
	CheckboxWidget *_globalMT32Override;
	CheckboxWidget *_globalVolumeOverride;

	OptionsContainerWidget *_engineOptions;
};

} // End of namespace GUI

#endif
