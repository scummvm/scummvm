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

#ifndef GLK_DIALOGS_H
#define GLK_DIALOGS_H

#include "glk/glk.h"

#include "common/str.h"
#include "gui/dialog.h"
#include "gui/widget.h"
#include "gui/widgets/edittext.h"

namespace GUI {
class PopUpWidget;
class StaticTextWidget;
class EditTextWidget;
class CheckboxWidget;
} // namespace GUI

namespace Glk {

class GlkEngine;

class GlkOptionsWidget : public GUI::OptionsContainerWidget {

	enum {
		kTColorChangedCmd = 'TCLR',
		kGColorChangedCmd = 'GCLR',
		kWColorChangedCmd = 'WCLR',
		kBColorChangedCmd = 'BCLR',
		kCColorChangedCmd = 'CCLR',
		kLColorChangedCmd = 'LCLR',
		kMColorChangedCmd = 'MCLR',
		kTHexChangedCmd = 'THEX',
		kGHexChangedCmd = 'GHEX',
		kWHexChangedCmd = 'WHEX',
		kBHexChangedCmd = 'BHEX',
		kCHexChangedCmd = 'CHEX',
		kLHexChangedCmd = 'LHEX',
		kMHexChangedCmd = 'MHEX'
	};

public:
	GlkOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);
	~GlkOptionsWidget() override;

	// OptionsContainerWidget API
	void load() override;
	bool save() override;

	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
	void reflowLayout() override;

private:
	// OptionsContainerWidget API
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
	GUI::PopUpWidget *_tfontPopUps[style_NUMSTYLES];
	GUI::PopUpWidget *_gfontPopUps[style_NUMSTYLES];
	GUI::PopUpWidget *_tcolorPopUps[style_NUMSTYLES];
	GUI::PopUpWidget *_gcolorPopUps[style_NUMSTYLES];
	GUI::PopUpWidget *_wcolorPopUps[style_NUMSTYLES];
	GUI::PopUpWidget *_bcolorPopUps[style_NUMSTYLES];
	GUI::PopUpWidget *_ccolorPopUps[style_NUMSTYLES];
	GUI::PopUpWidget *_lcolorPopUps[style_NUMSTYLES];
	GUI::PopUpWidget *_mcolorPopUps[style_NUMSTYLES];
	GUI::EditTextWidget *_manualTColorHexInput;
	GUI::EditTextWidget *_manualGColorHexInput;
	GUI::EditTextWidget *_manualWColorHexInput;
	GUI::EditTextWidget *_manualBColorHexInput;
	GUI::EditTextWidget *_wborderx;
	GUI::EditTextWidget *_wbordery;
	GUI::EditTextWidget *_manualCColorHexInput;
	GUI::EditTextWidget *_manualLColorHexInput;
	GUI::EditTextWidget *_manualMColorHexInput;
	GUI::EditTextWidget *_linkStyle;
	GUI::EditTextWidget *_caretShape;
	GUI::EditTextWidget *_morePrompt;
	GUI::CheckboxWidget *_styleHint;
	GUI::CheckboxWidget *_safeClicks;
	GUI::EditTextWidget *_cols;
	GUI::EditTextWidget *_rows;
	GUI::CheckboxWidget *_lockcols;
	GUI::CheckboxWidget *_lockrows;
	GUI::CheckboxWidget *_justify;
	GUI::CheckboxWidget *_caps;
	GUI::PopUpWidget *_quotes;
	GUI::PopUpWidget *_dashes;
	GUI::PopUpWidget *_spaces;
	GUI::CheckboxWidget *_graphics;
	GUI::EditTextWidget *_wmarginx;
	GUI::EditTextWidget *_wmarginy;
	GUI::EditTextWidget *_wpaddingx;
	GUI::EditTextWidget *_wpaddingy;
	GUI::EditTextWidget *_tmarginx;
	GUI::EditTextWidget *_tmarginy;
	GUI::EditTextWidget *_leading;
	GUI::EditTextWidget *_baseline;
	GUI::EditTextWidget *_monosize;
	GUI::EditTextWidget *_propsize;
	GUI::PopUpWidget *_morealign;
	GUI::PopUpWidget *_morefont;
	GUI::StaticTextWidget *_fontHeadinglbl;
	GUI::StaticTextWidget *_colorHeadinglbl;
	GUI::StaticTextWidget *_borderHeadinglbl;
	GUI::StaticTextWidget *_wmarginHeadinglbl;
	GUI::StaticTextWidget *_tmarginHeadinglbl;
	GUI::StaticTextWidget *_wpaddingHeadinglbl;
	GUI::StaticTextWidget *_moreHeadinglbl;
	GUI::StaticTextWidget *_typographyHeadinglbl;
	GUI::StaticTextWidget *_userExperiencelbl;

	void layoutHeadings();
};
} // namespace Glk

#endif
