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

#include "common/array.h"
#include "common/hash-str.h"
#include "common/str.h"
#include "gui/dialog.h"
#include "gui/widget.h"
#include "gui/widgets/edittext.h"

namespace GUI {
class PopUpWidget;
class StaticTextWidget;
class EditTextWidget;
class CheckboxWidget;
class ButtonWidget;
class TabWidget;
} // namespace GUI

namespace Glk {

class GlkEngine;
class GlkOptionsState;
class GlkColorSwatchWidget;
class GlkOptionsLabelWidget;

class GlkOptionsWidget : public GUI::OptionsContainerWidget {

	enum {
		kOptionEditedCmd = 'GOED',
		kInterpreterDefaultsCmd = 'GRID',
		kSaveGlobalCmd = 'GGLB',
		kRestoreDefaultsCmd = 'GRST',
		kStyleSelectionCmd = 'GSSL',
		kPreviewCmd = 'GLKP',
		kResetStyleFontCmd = 'GRSF',
		kResetStyleForegroundCmd = 'GRSG',
		kResetStyleBackgroundCmd = 'GRSB'
	};

public:
	GlkOptionsWidget(GuiObject *boss, const Common::String &name,
		const Common::String &domain, InterpreterType interpreterType);
	~GlkOptionsWidget() override;

	// OptionsContainerWidget API
	void load() override;
	bool save() override;
	bool validate() override;
	bool handleOptionsKeyDown(Common::KeyState state) override;
	void setHostContext(HostContext context) override;
	void setDomain(const Common::String &domain) override;
	void reflowLayout() override;

	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	friend class GlkOptionsPageLayout;
	friend class GlkOptionsTabWidget;

	GlkOptionsState *_settings;
	const Conf *_conf;
	GUI::TabWidget *_tabs;
	GUI::ButtonWidget *_preview;
	GUI::ButtonWidget *_restoreDefaults;
	GUI::ButtonWidget *_interpreterDefaults;
	GUI::ButtonWidget *_saveGlobal;
	HostContext _hostContext;
	bool _loading;
	bool _previewFontsAvailable;
	Common::HashMap<Common::String, bool, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _editedControls;
	Common::Array<GlkOptionsLabelWidget *> _optionLabels;

	struct ControlBinding {
		Common::String key;
		GUI::Widget *widget;
		const char *label;
		ControlBinding(const char *name, GUI::Widget *control, const char *labelName = nullptr)
			: key(name), widget(control), label(labelName) {}
	};
	enum ColorTarget {
		kNormalProseForeground,
		kNormalGridForeground,
		kWindowBackground,
		kDividerColor,
		kCaretColor,
		kLinkColor,
		kMoreColor,
		kColorTargetCount
	};
	struct ColorBinding {
		ColorTarget target;
		const char *prefix;
		const char *key;
		const char *legacyOverride;
		GUI::PopUpWidget *popup;
		GUI::EditTextWidget *hex;
		GlkColorSwatchWidget *swatch;
		bool isNormalForeground() const {
			return target == kNormalProseForeground || target == kNormalGridForeground;
		}
		bool allowsDefault() const { return !isNormalForeground(); }
	};
	// Widgets remain owned by their GUI containers.
	ColorBinding _colors[kColorTargetCount];

	Common::Array<ControlBinding> _controls;
	Common::StringMap _loadedControls;

	GUI::PopUpWidget *_tfontPopUp;
	GUI::EditTextWidget *_wborderx;
	GUI::EditTextWidget *_wbordery;
	GUI::PopUpWidget *_linkStyle;
	GUI::PopUpWidget *_caretShape;
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
	GUI::PopUpWidget *_styleWindow;
	GUI::PopUpWidget *_styleName;
	GUI::PopUpWidget *_styleFont;
	GUI::EditTextWidget *_styleForeground;
	GUI::EditTextWidget *_styleBackground;
	GlkColorSwatchWidget *_styleForegroundSwatch;
	GlkColorSwatchWidget *_styleBackgroundSwatch;
	GUI::ButtonWidget *_resetStyleFont;
	GUI::ButtonWidget *_resetStyleForeground;
	GUI::ButtonWidget *_resetStyleBackground;
	bool _loadedStyleGrid;
	int _loadedStyle;

	GUI::EditTextWidget *createEdit(const char *labelName, const Common::U32String &label,
		const char *name, const Common::U32String &tooltip);
	GUI::CheckboxWidget *createCheckbox(const char *labelName, const Common::U32String &label,
		const char *name, const Common::U32String &tooltip);
	GUI::PopUpWidget *createPopup(const char *labelName, const Common::U32String &label,
		const char *name, const Common::U32String &tooltip);
	GlkOptionsLabelWidget *createOptionLabel(GUI::GuiObject *boss, const Common::String &name,
		const Common::U32String &text, const Common::U32String &tooltip,
		GUI::ThemeEngine::FontStyle font = GUI::ThemeEngine::kFontStyleBold);
	void createColorBinding(ColorTarget target, const char *prefix,
		const char *key, const char *legacyOverride, const Common::U32String &label,
		const Common::U32String &tooltip);
	GlkOptionsLabelWidget *findOptionLabel(const Common::String &name) const;
	void loadControls();
	void loadColor(const ColorBinding &color);
	void loadStyleEditor(GUI::Widget *only = nullptr);
	void rememberControl(GUI::Widget *widget);
	void rememberControls();
	void refreshControl(GUI::Widget *widget);
	void refreshControlValue(GUI::Widget *widget);
	void updateLayoutDependencies();
	void updateActionState();
	void checkPreviewFonts();
	void updatePreviewAvailability(bool reflow = true);
	bool controlChanged(GUI::Widget *widget) const;
	bool hasPendingControls(GUI::Widget *only) const;
	bool hasPendingStyle(GUI::Widget *only = nullptr) const;
	bool synchronizeDraft(bool reportError, GUI::Widget *only = nullptr, bool checkFonts = true);
	bool synchronizePending(GlkOptionsState &pending, bool reportError, GUI::Widget *only);
	bool synchronizeStyleEditor(GlkOptionsState &pending, bool reportError, GUI::Widget *only = nullptr);
	void acceptPending(GlkOptionsState &pending, bool checkFonts = true);
	bool invalidControl(GUI::Widget *widget, bool reportError);
	void showGlobalPreferenceError(const Common::String &key);
	void focusControl(GUI::Widget *widget);
	bool selectPage(int page, bool userRequested);
	void reflowAfterPageChange(bool userRequested);
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
};
} // namespace Glk

#endif
