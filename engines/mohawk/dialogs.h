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
#include "common/ustr.h"

#include "engines/dialogs.h"

#include "gui/dialog.h"
#include "gui/widget.h"

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
	Common::U32String _message;
	GUI::StaticTextWidget *_text;

public:
	InfoDialog(MohawkEngine *vm, const Common::U32String &message);

	void setInfoText(const Common::U32String &message);

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
	PauseDialog(MohawkEngine* vm, const Common::U32String &message);
	void handleKeyDown(Common::KeyState state) override;
};

#ifdef ENABLE_MYST

class MystOptionsWidget : public GUI::OptionsContainerWidget {
public:
	MystOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);
	~MystOptionsWidget() override;

	// Widget API
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

	// OptionsContainerWidget API
	void load() override;
	bool save() override;

private:
	// OptionsContainerWidget API
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;

	bool isInGame() const;

	GUI::CheckboxWidget *_zipModeCheckbox;
	GUI::CheckboxWidget *_transitionsCheckbox;
	GUI::CheckboxWidget *_mystFlyByCheckbox;
	GUI::PopUpWidget *_languagePopUp;

	GUI::ButtonWidget *_dropPageButton;
	GUI::ButtonWidget *_showMapButton;
	GUI::ButtonWidget *_returnToMenuButton;
};

class MystMenuDialog : public MainMenuDialog {
public:
	MystMenuDialog(Engine *engine);
	~MystMenuDialog() override;

	// MainMenuDialog API
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
};

#endif

#ifdef ENABLE_RIVEN

class RivenOptionsWidget : public GUI::OptionsContainerWidget {
public:
	explicit RivenOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);
	~RivenOptionsWidget() override;

	// OptionsContainerWidget API
	void load() override;
	bool save() override;

private:
	// OptionsContainerWidget API
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;

	GUI::CheckboxWidget *_zipModeCheckbox;
	GUI::CheckboxWidget *_waterEffectCheckbox;
	GUI::PopUpWidget *_transitionModePopUp;
	GUI::PopUpWidget *_languagePopUp;
};

#endif

} // End of namespace Mohawk

#endif
