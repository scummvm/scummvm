/* Copyright (C) 2024 Giovanni Cascione <ing.cascione@gmail.com>
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
#include "common/translation.h"
#include "gui/browser.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/list.h"
#include "gui/widgets/popup.h"
#include "gui/widgets/richtext.h"

#define COMMON_HOOKS_FOLDER "scummvm_hooks"

class LibretroOptionsWidget final : public GUI::OptionsContainerWidget {
public:
	explicit LibretroOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);
	~LibretroOptionsWidget() override;
	void load() override;
	bool save() override;

private:
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
	bool generatePlaylist(Common::String playlistPath);
	bool cleanFolder(Common::String &path);

	GUI::StaticTextWidget *_playlistPath;
	GUI::StaticTextWidget *_playlistStatus;

	GUI::PopUpWidget *_playlistVersion;
	GUI::PopUpWidget *_hooksLocation;
	GUI::CheckboxWidget *_hooksClear;
};
