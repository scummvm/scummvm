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

#ifndef MOHAWK_DIALOGS_H
#define MOHAWK_DIALOGS_H

#include "mohawk/mohawk.h"

#include "common/events.h"
#include "common/str.h"
#include "gui/dialog.h"

namespace GUI {
class SaveLoadChooser;
class ButtonWidget;
class CheckboxWidget;
class CommandSender;
class PopUpWidget;
class StaticTextWidget;
}

namespace Mohawk {

class MohawkEngine;

class InfoDialog : public GUI::Dialog {
protected:
	MohawkEngine *_vm;
	Common::String _message;
	GUI::StaticTextWidget *_text;

public:
	InfoDialog(MohawkEngine *vm, const Common::String &message);

	void setInfoText(const Common::String &message);

	void handleMouseDown(int x, int y, int button, int clickCount) override {
		setResult(0);
		close();
	}

	void handleKeyDown(Common::KeyState state) override {
		setResult(state.ascii);
		close();
	}

	void reflowLayout() override;
};

class PauseDialog : public InfoDialog {
public:
	PauseDialog(MohawkEngine* vm, const Common::String &message);
	void handleKeyDown(Common::KeyState state) override;
};

#if defined(ENABLE_MYST) || defined(ENABLE_RIVEN)

class MohawkOptionsDialog : public GUI::Dialog {
public:
	explicit MohawkOptionsDialog();
	~MohawkOptionsDialog() override;

	void reflowLayout() override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
};

#endif

#ifdef ENABLE_MYST

class MohawkEngine_Myst;

class MystOptionsDialog : public MohawkOptionsDialog {
public:
	explicit MystOptionsDialog(MohawkEngine_Myst *vm);
	~MystOptionsDialog() override;

	enum ResultAction {
		kActionSaveSettings = 1,
		kActionDropPage,
		kActionShowMap,
		kActionGoToMenu,
		kActionShowCredits
	};

	void setCanDropPage(bool canDropPage);
	void setCanShowMap(bool canShowMap);
	void setCanReturnToMenu(bool canReturnToMenu);

	bool getZipMode() const;
	void setZipMode(bool enabled);
	bool getTransitions() const;
	void setTransitions(bool enabled);

	void open() override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

	int getLoadSlot() const { return _loadSlot; }
	int getSaveSlot() const { return _saveSlot; }
	Common::String getSaveDescription() const { return _saveDescription; }

private:
	void save();
	void load();

	MohawkEngine_Myst *_vm;

	GUI::ButtonWidget    *_loadButton;
	GUI::ButtonWidget    *_saveButton;
	GUI::ButtonWidget    *_quitButton;

	GUI::SaveLoadChooser *_loadDialog;
	GUI::SaveLoadChooser *_saveDialog;

	int _loadSlot;
	int _saveSlot;
	Common::String _saveDescription;

	bool _canDropPage;
	bool _canShowMap;
	bool _canReturnToMenu;

	GUI::CheckboxWidget *_zipModeCheckbox;
	GUI::CheckboxWidget *_transitionsCheckbox;

	GUI::ButtonWidget *_dropPageButton;
	GUI::ButtonWidget *_showMapButton;
	GUI::ButtonWidget *_returnToMenuButton;
};

#endif

#ifdef ENABLE_RIVEN

class MohawkEngine_Riven;

class RivenOptionsDialog : public MohawkOptionsDialog {
public:
	explicit RivenOptionsDialog(MohawkEngine_Riven *vm);
	~RivenOptionsDialog() override;

	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

	bool getZipMode() const;
	void setZipMode(bool enabled);
	bool getWaterEffect() const;
	void setWaterEffect(bool enabled);
	uint32 getTransitions() const;
	void setTransitions(uint32 mode);
private:
	MohawkEngine_Riven *_vm;

	GUI::CheckboxWidget *_zipModeCheckbox;
	GUI::CheckboxWidget *_waterEffectCheckbox;
	GUI::StaticTextWidget *_transitionModeCaption;
	GUI::PopUpWidget *_transitionModePopUp;
};

#endif

} // End of namespace Mohawk

#endif
