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

#include "glk/glk.h"
#include "glk/dialogs.h"

#include "gui/ThemeEval.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/saveload.h"
#include "gui/ThemeEngine.h"
#include "gui/widget.h"
#include "gui/widgets/list.h"
#include "gui/widgets/popup.h"
#include "gui/widgets/scrollcontainer.h"
#include "gui/widgets/tab.h"

#include "glk/conf.h"
#include "glk/fonts.h"
#include "glk/options.h"
#include "glk/preview.h"
#include "glk/screen.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/translation.h"

#define NEW_LABEL(name, text, tooltip) \
	do { \
		createOptionLabel(name, _(text), _(tooltip)); \
	} while (0)

#define NEW_EDIT(widget, name, tooltip) \
	widget = new GlkOptionsEditTextWidget(_currentBoss, name, Common::U32String(), _(tooltip), kOptionEditedCmd)

#define NEW_CHECKBOX(widget, name, tooltip) \
	widget = new GlkOptionsCheckboxWidget(_currentBoss, name, Common::U32String(), _(tooltip), kOptionEditedCmd)

#define NEW_POPUP(widget, name, tooltip) \
	widget = new GlkOptionsPopUpWidget(_currentBoss, name, _(tooltip), kOptionEditedCmd)

#define NEW_EDIT_CMD(widget, name, tooltip, cmd) \
	widget = new GlkOptionsEditTextWidget(_currentBoss, name, Common::U32String(), _(tooltip), cmd)

#define NEW_LABEL_EDIT(lbl_name, text, edit_widget, edit_name, tooltip) \
	do { \
		NEW_LABEL(lbl_name, text, tooltip); \
		NEW_EDIT(edit_widget, edit_name, tooltip); \
	} while (0)

#define NEW_LABEL_CHECKBOX(lbl_name, text, chk_widget, chk_name, tooltip) \
	do { \
		NEW_LABEL(lbl_name, text, tooltip); \
		NEW_CHECKBOX(chk_widget, chk_name, tooltip); \
	} while (0)

#define NEW_LABEL_POPUP(lbl_name, text, pop_widget, pop_name, tooltip) \
	do { \
		NEW_LABEL(lbl_name, text, tooltip); \
		NEW_POPUP(pop_widget, pop_name, tooltip); \
	} while (0)

namespace Glk {

static const uint32 kGlkEditFinishedCmd = 'GEFN';

static const char *fontFaceLabels[] = {
		_s("Monospace Regular"),
		_s("Monospace Bold"),
		_s("Monospace Italic"),
		_s("Monospace Bold Italic"),
		_s("Proportional Regular"),
		_s("Proportional Bold"),
		_s("Proportional Italic"),
		_s("Proportional Bold Italic")
	};

enum GlkOptionsPage {
	kGlkOptionsReading,
	kGlkOptionsCompatibility,
	kGlkOptionsColors,
	kGlkOptionsStyles,
	kGlkOptionsLayout,
	kGlkOptionsInteraction,
	kGlkOptionsPaging
};

class GlkOptionsLabelWidget : public GUI::StaticTextWidget {
public:
	GlkOptionsLabelWidget(GUI::GuiObject *boss, const Common::String &name,
			const Common::U32String &text, const Common::U32String &tooltip,
			GUI::ThemeEngine::FontStyle font)
		: GUI::StaticTextWidget(boss, name, text, tooltip,
			font, Common::UNK_LANG, false) {
	}

	int wrappedHeight(int width) const {
		Common::Array<Common::U32String> lines;
		g_gui.getFont(_font).wordWrapText(_label, MAX(1, width), lines);
		return MAX(1, (int)lines.size()) * g_gui.getFontHeight(_font);
	}

protected:
	void reflowLayout() override {
		if (!isVisible()) {
			_x = _y = _w = _h = 0;
			return;
		}
		setAlign(Graphics::kTextAlignInvalid);
		GUI::StaticTextWidget::reflowLayout();
	}
	void drawWidget() override {
		Common::Array<Common::U32String> lines;
		g_gui.getFont(_font).wordWrapText(_label, MAX(1, (int)_w), lines);
		const int height = g_gui.getFontHeight(_font);
		int y = _y + MAX(0, (_h - (int)lines.size() * height) / 2);
		for (uint i = 0; i < lines.size(); ++i, y += height)
			g_gui.theme()->drawText(Common::Rect(_x, y, _x + _w, y + height),
				lines[i], _state, _align, GUI::ThemeEngine::kTextInversionNone,
				0, false, _font, _fontColor);
	}

};

class GlkOptionsCheckboxWidget : public GUI::CheckboxWidget {
public:
	GlkOptionsCheckboxWidget(GUI::GuiObject *boss, const Common::String &name,
			const Common::U32String &label, const Common::U32String &tooltip, uint32 cmd)
		: GUI::CheckboxWidget(boss, name, label, tooltip, cmd) {}
	bool wantsFocus() override { return isEnabled(); }
};

class GlkOptionsButtonWidget : public GUI::ButtonWidget {
public:
	GlkOptionsButtonWidget(GUI::GuiObject *boss, const Common::String &name,
			const Common::U32String &label, const Common::U32String &tooltip, uint32 cmd)
		: GUI::ButtonWidget(boss, name, label, tooltip, cmd) {}
	bool wantsFocus() override { return isEnabled(); }
	void reflowLayout() override {
		if (isVisible())
			GUI::ButtonWidget::reflowLayout();
		else
			_x = _y = _w = _h = 0;
	}
};

class GlkOptionsEditTextWidget : public GUI::EditTextWidget {
private:
	bool _longText;

public:
	GlkOptionsEditTextWidget(GUI::GuiObject *boss,
			const Common::String &name, const Common::U32String &text,
			const Common::U32String &tooltip, uint32 cmd = 0)
		: GUI::EditTextWidget(boss, name, text, tooltip, cmd, kGlkEditFinishedCmd),
		  _longText(false) {
		setFinishOnFocusLoss(true);
	}

	void setLongText() {
		_longText = true;
	}

	void getMinSize(int &minWidth, int &minHeight) override {
		const Graphics::Font &font = g_gui.getFont(GUI::ThemeEngine::kFontStyleNormal);
		int digitWidth = 0;
		for (const char *digit = "0123456789abcdefABCDEF"; *digit; ++digit)
			digitWidth = MAX(digitWidth, font.getCharWidth(*digit));
		// Six RGB digits also leave room for numeric values and fractions.
		// Longer numeric spellings remain editable through horizontal scrolling.
		const int contentWidth = _longText ? MAX(digitWidth * 6,
			font.getStringWidth(getEditString())) : digitWidth * 6;
		setMinContentWidth(contentWidth);
		GUI::EditTextWidget::getMinSize(minWidth, minHeight);
	}

};

class GlkOptionsPopUpWidget : public GUI::PopUpWidget {
public:
	GlkOptionsPopUpWidget(GUI::GuiObject *boss, const Common::String &name,
			const Common::U32String &tooltip, uint32 cmd = 0)
		: GUI::PopUpWidget(boss, name, tooltip, cmd) {
		setNaturalSize(true);
	}

	bool wantsFocus() override { return isEnabled(); }

};

static int glkSwatchSize() {
	return MAX(8, g_gui.getFontHeight() + 2);
}

class GlkColorSwatchWidget : public GUI::Widget {
private:
	uint _rgb;
	bool _valid;

public:
	GlkColorSwatchWidget(GUI::GuiObject *boss, const Common::String &name,
			const Common::U32String &tooltip)
		: GUI::Widget(boss, name, tooltip), _rgb(0), _valid(false) {
		_type = GUI::kGraphicsWidget;
		setFlags(GUI::WIDGET_ENABLED | GUI::WIDGET_CLEARBG);
	}

	void setColor(const Common::String &text) {
		Common::String normalized;
		_valid = GlkOptionsState::parseColor(text, normalized);
		if (_valid)
			_rgb = strtol(normalized.c_str(), nullptr, 16);
		markAsDirty();
	}

	void getMinSize(int &minimumWidth, int &minimumHeight) override {
		minimumWidth = minimumHeight = glkSwatchSize();
	}

protected:
	void drawWidget() override {
		if (_w <= 2 || _h <= 2)
			return;
		Graphics::ManagedSurface sample;
		sample.create(_w, _h, g_gui.theme()->getPixelFormat());
		const int brightness = ((_rgb >> 16) & 0xff) * 299 +
			((_rgb >> 8) & 0xff) * 587 + (_rgb & 0xff) * 114;
		const byte frame = brightness > 127000 ? 0 : 255;
		sample.fillRect(Common::Rect(0, 0, sample.w, sample.h),
			sample.format.RGBToColor(frame, frame, frame));
		if (_valid)
			sample.fillRect(Common::Rect(1, 1, sample.w - 1, sample.h - 1),
				sample.format.RGBToColor((_rgb >> 16) & 0xff,
					(_rgb >> 8) & 0xff, _rgb & 0xff));
		g_gui.theme()->drawManagedSurface(Common::Point(_x, _y),
			sample, Graphics::ALPHA_OPAQUE);
	}
};

class GlkOptionsTabWidget : public GUI::TabWidget {
	GlkOptionsWidget &_owner;
public:
	GlkOptionsTabWidget(GlkOptionsWidget &owner, GUI::GuiObject *boss, const Common::String &name)
		: GUI::TabWidget(boss, name), _owner(owner) {
	}

	GUI::Widget *getActivePageWidgets() const {
		return _firstWidget;
	}

	Common::Rect getBodyRect() const {
		return Common::Rect(getAbsX() + _bodyLP,
			getAbsY() + _tabHeight + _bodyTP,
			getAbsX() + _w - _bodyRP,
			getAbsY() + getHeight() - _bodyBP);
	}

protected:
	bool canChangeTab(int tabID) override {
		return _owner.synchronizeDraft(true);
	}
	void activeTabChanged(int previousTab) override {
		_owner.reflowAfterPageChange(true);
	}
};

struct GlkColor {
	const char *name;
	unsigned int rgb;
};

static const GlkColor GLK_COLORS[] = {
	{ _s("white"), 0xFFFFFF },
	{ _s("green"), 0x00FF00 },
	{ _s("red"),   0xFF0000 },
	{ _s("blue"),  0x0000FF },
	{ _s("black"), 0x000000 },
	{ _s("grey"),  0x808080 }
};

static int getGlkOptionPageInset(int textHeight, float scale) {
	if (scale <= 0.0f)
		scale = 1.0f;

	const int minimum = MAX((int)(scale * 2.0f + 0.5f), textHeight / 4);
	int inset = MAX(1, (int)(minimum / scale));
	while ((int)(inset * scale) < minimum)
		++inset;
	return inset;
}

struct GlkOptionThemeMetrics {
	int textHeight;
	int editHeight;
	int popupHeight;
	int checkboxHeight;
	int buttonHeight;
	int spacing;
};

struct GlkOptionRowControl {
	const char *name;
	const char *type;
	GUI::Widget *widget;
	int width;
	int height;
};

class GlkOptionsPageLayout {
private:
	GUI::ThemeEval &_layouts;
	const GlkOptionsWidget &_owner;
	Common::String _pageName;
	int _availableWidth;
	int _labelColumnWidth;
	int _checkboxLabelWidth;
	int _effectiveInset;
	GlkOptionThemeMetrics _metrics;
	int _contentHeight;
	int _blocks;
	int _styleEditorWidth;
	int _styleColorEditWidth;
	int _styleResetWidth;

	Common::String getFullName(const char *name) const {
		return _pageName + "." + name;
	}

	int getLabelWidth(const char *name) const {
		GUI::StaticTextWidget *label =
			_owner.findOptionLabel(getFullName(name));
		// Font::wordWrapText wraps at equality, so leave one pixel of slack.
		return label ? g_gui.getFont(GUI::ThemeEngine::kFontStyleBold).getStringWidth(label->getLabel()) + 1 : 0;
	}

	int getControlHeight(const char *type) const {
		if (!strcmp(type, "PopUp"))
			return _metrics.popupHeight;
		if (!strcmp(type, "Checkbox"))
			return _metrics.checkboxHeight;
		if (!strcmp(type, "Button"))
			return _metrics.buttonHeight;
		return _metrics.editHeight;
	}

	int getControlWidth(GUI::Widget *control, const char *type,
			int height) const {
		int width = height;
		int minimumHeight = -1;
		control->getMinSize(width, minimumHeight);
		if (width < 0)
			width = height;
		if (!strcmp(type, "Checkbox"))
			width += height + g_gui.xmlEval()->getVar(
				"Globals.Checkbox.Spacing", 0);
		else if (!strcmp(type, "Button"))
			width = MAX(width + _metrics.spacing * 2,
				_layouts.getVar("Globals.Button.Width", width));
		return width;
	}

	void addBlock(int height) {
		if (_blocks)
			_contentHeight += _metrics.spacing;
		_contentHeight += height;
		++_blocks;
	}

	void setControl(GlkOptionRowControl &control, const char *name,
			const char *type, GUI::Widget *widget, int width = -1,
			int height = -1) const {
		control.name = name;
		control.type = type;
		control.widget = widget;
		control.width = width;
		control.height = height;
	}

	void addMeasuredRow(const char *label, GlkOptionRowControl *controls,
			uint controlCount) {
		int controlsWidth = 0;
		int controlsHeight = 0;
		for (uint index = 0; index < controlCount; ++index) {
			GlkOptionRowControl &control = controls[index];
			if (control.height < 0)
				control.height = getControlHeight(control.type);
			if (control.width < 0)
				control.width = getControlWidth(control.widget, control.type,
					control.height);
			if (index)
				controlsWidth += _metrics.spacing;
			controlsWidth += control.width;
			controlsHeight = MAX(controlsHeight, control.height);
		}

		const bool checkbox = controlCount == 1 && !strcmp(controls[0].type, "Checkbox");
		int labelWidth = MAX(getLabelWidth(label), _labelColumnWidth);
		if (checkbox)
			labelWidth = MAX(labelWidth, _checkboxLabelWidth);
		// A checkbox keeps its label beside it, wrapping within the remaining
		// row width. Other rows stack only when their measured contents cannot
		// fit, rather than when a label exceeds the preferred label column.
		if (checkbox)
			labelWidth = MIN(labelWidth, MAX(1, _availableWidth - _metrics.spacing - controlsWidth));
		const bool stacked = !checkbox &&
			labelWidth + _metrics.spacing + controlsWidth > _availableWidth;
		if (stacked)
			labelWidth = _availableWidth;
		GlkOptionsLabelWidget *labelWidget = static_cast<GlkOptionsLabelWidget *>(_owner.findOptionLabel(getFullName(label)));
		const int labelHeight = labelWidget ? labelWidget->wrappedHeight(labelWidth) : _metrics.textHeight;
		if (controlCount == 1 && (!strcmp(controls[0].type, "PopUp") ||
				controls[0].widget == _owner._morePrompt))
			controls[0].width = stacked ? _availableWidth : _availableWidth - labelWidth - _metrics.spacing;
		const bool stackControls = controlsWidth > _availableWidth;
		int controlsBlockHeight = controlsHeight;
		if (stackControls) {
			controlsBlockHeight = 0;
			for (uint index = 0; index < controlCount; ++index) {
				if (index)
					controlsBlockHeight += _metrics.spacing;
				controlsBlockHeight += controls[index].height;
			}
		}
		_layouts.addLayout(stacked ? GUI::ThemeLayout::kLayoutVertical :
				GUI::ThemeLayout::kLayoutHorizontal, _metrics.spacing,
			GUI::ThemeLayout::kItemAlignCenter)
			.addPadding(0, 0, 0, 0)
			.addWidget(label, "", labelWidth, labelHeight,
				stacked ? Graphics::kTextAlignStart :
				Graphics::kTextAlignEnd, true, true);
		if (stacked) {
			_layouts.addLayout(stackControls ?
					GUI::ThemeLayout::kLayoutVertical :
					GUI::ThemeLayout::kLayoutHorizontal,
					_metrics.spacing, GUI::ThemeLayout::kItemAlignCenter)
				.addPadding(0, 0, 0, 0);
		}
		for (uint index = 0; index < controlCount; ++index) {
			if (!controls[index].widget) {
				_layouts.addSpace(controls[index].width);
				continue;
			}
			_layouts.addWidget(controls[index].name, "",
				MIN(controls[index].width, _availableWidth),
				controls[index].height, Graphics::kTextAlignStart, true, true);
		}
		if (stacked)
			_layouts.closeLayout();
		_layouts.closeLayout();
		addBlock(stacked ? labelHeight + _metrics.spacing +
			controlsBlockHeight : MAX(labelHeight,
			controlsBlockHeight));
	}

public:
	GlkOptionsPageLayout(GUI::ThemeEval &layouts,
			const GlkOptionsWidget &owner, const Common::String &pageName,
			int availableWidth, int labelColumnWidth, int layoutInset,
			int effectiveInset,
			const GlkOptionThemeMetrics &metrics)
		: _layouts(layouts), _owner(owner), _pageName(pageName),
		  _availableWidth(availableWidth),
		  _labelColumnWidth(labelColumnWidth), _checkboxLabelWidth(0), _effectiveInset(effectiveInset),
		  _metrics(metrics),
		  _contentHeight(0), _blocks(0), _styleEditorWidth(-1),
		  _styleColorEditWidth(-1), _styleResetWidth(-1) {
		_layouts.addDialog(pageName, "GlkOptionsDialog.TabWidget")
			.addPadding(0, 0, 0, 0)
			.addLayout(GUI::ThemeLayout::kLayoutVertical, metrics.spacing)
				.addPadding(layoutInset, layoutInset, layoutInset, layoutInset);
	}

	void setCheckboxLabels(const char *const *labels, uint count) {
		for (uint i = 0; i < count; ++i)
			_checkboxLabelWidth = MAX(_checkboxLabelWidth, getLabelWidth(labels[i]));
	}

	void addExplanation(const char *name) {
		const GlkOptionsLabelWidget *label = static_cast<GlkOptionsLabelWidget *>(_owner.findOptionLabel(getFullName(name)));
		const int height = label->wrappedHeight(_availableWidth);
		_layouts.addWidget(name, "", _availableWidth, height, Graphics::kTextAlignStart, true, true);
		addBlock(height);
	}

	void addOptionRow(const char *label, const char *control,
			GUI::Widget *controlWidget, const char *controlType) {
		GlkOptionRowControl controls[1];
		setControl(controls[0], control, controlType, controlWidget);
		addMeasuredRow(label, controls, ARRAYSIZE(controls));
	}

	void addColorRow(const char *prefix, GUI::PopUpWidget *popupWidget,
			GUI::EditTextWidget *hexWidget, GUI::Widget *swatchWidget) {
		const Common::String label = Common::String(prefix) + "Color";
		const Common::String popup = Common::String(prefix) + "Color0";
		const Common::String swatch = Common::String(prefix) + "Swatch";
		const Common::String hex = Common::String(prefix) + "Hex";
		const Common::String hash = Common::String(prefix) + "Prefix";
		GlkOptionRowControl controls[4];
		setControl(controls[0], popup.c_str(), "PopUp", popupWidget);
		setControl(controls[1], swatch.c_str(), "", swatchWidget,
			glkSwatchSize(), glkSwatchSize());
		setControl(controls[2], hash.c_str(), "", _owner.findOptionLabel(getFullName(hash.c_str())),
			g_gui.getStringWidth("#") + 1, _metrics.editHeight);
		setControl(controls[3], hex.c_str(), "EditTextWidget", hexWidget);
		addMeasuredRow(label.c_str(), controls, ARRAYSIZE(controls));
	}

	void addStyleRow(const char *label, const char *control,
			GUI::Widget *controlWidget, const char *controlType,
			const char *reset, GUI::ButtonWidget *resetWidget) {
		GlkOptionRowControl controls[2];
		setControl(controls[0], control, controlType, controlWidget,
			_styleEditorWidth);
		setControl(controls[1], reset, "Button", resetWidget,
			_styleResetWidth);
		addMeasuredRow(label, controls, ARRAYSIZE(controls));
	}

	void setStyleColumns(GUI::Widget *font, GUI::Widget *foreground,
			GUI::Widget *background) {
		const int foregroundWidth = getControlWidth(foreground,
			"EditTextWidget", _metrics.editHeight);
		const int backgroundWidth = getControlWidth(background,
			"EditTextWidget", _metrics.editHeight);
		_styleColorEditWidth = MAX(foregroundWidth, backgroundWidth);
		_styleEditorWidth = MAX(getControlWidth(font, "PopUp",
			_metrics.popupHeight), glkSwatchSize() + _metrics.spacing * 2 +
			g_gui.getStringWidth("#") + 1 + _styleColorEditWidth);
		const Common::U32String labels[] = { _("Use global default"), _("Global default"),
			_("Use interpreter default"), _("Interpreter default") };
		_styleResetWidth = 0;
		for (uint i = 0; i < ARRAYSIZE(labels); ++i)
			_styleResetWidth = MAX(_styleResetWidth, g_gui.getStringWidth(labels[i]) + _metrics.spacing * 4);
	}

	void addStyleColorRow(const char *label, const char *swatch,
			GUI::Widget *swatchWidget, const char *control,
			GUI::Widget *controlWidget, const char *reset,
			GUI::ButtonWidget *resetWidget) {
		const Common::String prefix = Common::String(control) + "Prefix";
		const int prefixWidth = g_gui.getStringWidth("#") + 1;
		GlkOptionRowControl controls[5];
		setControl(controls[0], swatch, "", swatchWidget,
			glkSwatchSize(), glkSwatchSize());
		setControl(controls[1], prefix.c_str(), "", _owner.findOptionLabel(getFullName(prefix.c_str())),
			prefixWidth, _metrics.editHeight);
		setControl(controls[2], control, "EditTextWidget", controlWidget, _styleColorEditWidth);
		uint count = 3;
		const int gap = _styleEditorWidth - glkSwatchSize() - prefixWidth -
			_styleColorEditWidth - _metrics.spacing * 3;
		if (gap > 0 && _styleEditorWidth + _metrics.spacing + _styleResetWidth <= _availableWidth)
			setControl(controls[count++], "", "", nullptr, gap, 0);
		setControl(controls[count++], reset, "Button", resetWidget, _styleResetWidth);
		addMeasuredRow(label, controls, count);
	}

	void close() {
		_layouts.closeLayout().closeDialog();
	}

	int getContentHeight() const {
		return _contentHeight + _effectiveInset * 2;
	}
};

void GlkOptionsWidget::createColorBinding(ColorTarget target, const char *prefix,
		const char *key, const char *legacyOverride, const Common::U32String &label,
		const Common::U32String &tooltip, uint32 popupCommand, uint32 hexCommand) {
	ColorBinding &color = _colors[target];
	color.target = target;
	color.prefix = prefix;
	color.key = key;
	color.legacyOverride = legacyOverride;
	color.popupCommand = popupCommand;
	color.hexCommand = hexCommand;
	const Common::String name = Common::String("GlkOptionsColors.") + prefix;
	color.hex = new GlkOptionsEditTextWidget(_currentBoss, name + "Hex", Common::U32String(), tooltip, hexCommand);
	createOptionLabel(name + "Prefix", Common::U32String("#"), Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
	color.popup = new GlkOptionsPopUpWidget(_currentBoss, name + "Color0", tooltip, popupCommand);
	color.swatch = new GlkColorSwatchWidget(_currentBoss, name + "Swatch", tooltip);
	createOptionLabel(name + "Color", label, tooltip);
	if (color.allowsDefault())
		color.popup->appendEntry(_("<default>"), 7);
	for (uint i = 0; i < ARRAYSIZE(GLK_COLORS); ++i)
		color.popup->appendEntry(_(GLK_COLORS[i].name), i);
	color.popup->appendEntry(_("<custom>"), 6);
}

GlkOptionsWidget::GlkOptionsWidget(GuiObject *boss, const Common::String &name,
		const Common::String &domain, InterpreterType interpreterType)
	: GUI::OptionsContainerWidget(boss, name, "GlkOptionsDialog", domain),
	  _settings(new GlkOptionsState(interpreterType, domain)),
	  _conf(&_settings->resolved()),
	  _currentBoss(nullptr), _tabs(nullptr), _preview(nullptr),
	  _restoreDefaults(nullptr), _interpreterDefaults(nullptr), _saveGlobal(nullptr),
	  _hostContext(kLauncherOptions), _loading(true),
	  _styleWindow(nullptr), _styleName(nullptr),
	  _styleFont(nullptr), _styleForeground(nullptr), _styleBackground(nullptr),
	  _styleForegroundSwatch(nullptr), _styleBackgroundSwatch(nullptr),
	  _resetStyleFont(nullptr), _resetStyleForeground(nullptr),
	  _resetStyleBackground(nullptr), _loadedStyleGrid(false),
	  _loadedStyle(style_Normal) {
	setContentSized(true);

	_tabs = new GlkOptionsTabWidget(*this, widgetsBoss(),
		"GlkOptionsDialog.TabWidget");
	const int readingTab = _tabs->addTab(_("Reading"), "GlkOptionsReading");
	const int compatibilityTab = _tabs->addTab(_("Compatibility"),
		"GlkOptionsCompatibility");
	const int colorsTab = _tabs->addTab(_("Colors"), "GlkOptionsColors");
	const int stylesTab = _tabs->addTab(_("Styles"), "GlkOptionsStyles");
	const int layoutTab = _tabs->addTab(_("Layout"), "GlkOptionsLayout");
	const int interactionTab = _tabs->addTab(_("Interaction"),
		"GlkOptionsInteraction");
	const int pagingTab = _tabs->addTab(_("Paging & Text"),
		"GlkOptionsPaging");
	_tabs->setActiveTab(readingTab);
	_currentBoss = _tabs;

	NEW_LABEL_POPUP("GlkOptionsReading.TFont", _("Normal prose font:"),
		_tfontPopUp, "GlkOptionsReading.TFont0",
		_("Font of the Normal prose style; other styles are unchanged"));
	for (int font = MONOR; font <= PROPZ; ++font)
		_tfontPopUp->appendEntry(_(fontFaceLabels[font]), font);
	NEW_LABEL("GlkOptionsReading.GridExplanation",
		"Fixed-grid windows require fixed-width fonts. Adjust individual text styles on the Styles page.",
		"Prose windows can also contain fixed-width text.");

	_tabs->setActiveTab(colorsTab);

	createColorBinding(kNormalProseForeground, "T", nullptr, nullptr,
		_("Normal prose text:"), _("Foreground of the Normal prose style"), kTColorChangedCmd, kTHexChangedCmd);

	createColorBinding(kNormalGridForeground, "G", nullptr, nullptr,
		_("Normal fixed-grid text:"), _("Foreground of the Normal fixed-grid style"), kGColorChangedCmd, kGHexChangedCmd);

	createColorBinding(kWindowBackground, "W", "windowcolor", "windowcolor_override",
		_("Window background:"), _("Background color of text windows"), kWColorChangedCmd, kWHexChangedCmd);

	createColorBinding(kDividerColor, "B", "bordercolor", "bordercolor_override",
		_("Divider:"), _("Color of dividers between panes"), kBColorChangedCmd, kBHexChangedCmd);

	_tabs->setActiveTab(layoutTab);
	NEW_LABEL_EDIT("GlkOptionsLayout.wborderx", _("Horizontal divider:"), _wborderx, "GlkOptionsLayout.wborderhorizontal", _("Horizontal divider thickness"));
	NEW_LABEL_EDIT("GlkOptionsLayout.wbordery", _("Vertical divider:"), _wbordery, "GlkOptionsLayout.wbordervertical", _("Vertical divider thickness"));

	_tabs->setActiveTab(colorsTab);

	createColorBinding(kCaretColor, "C", "caretcolor", nullptr,
		_("Insertion cursor:"), _("Color of the insertion cursor"), kCColorChangedCmd, kCHexChangedCmd);

	createColorBinding(kLinkColor, "L", "linkcolor", nullptr,
		_("Clickable text:"), _("Color of clickable text"), kLColorChangedCmd, kLHexChangedCmd);

	// I18N: "More..." prompts when text exceeds the window size.
	createColorBinding(kMoreColor, "M", "morecolor", nullptr,
		_("More:"), _("Color for the \"More...\" markers in the text"), kMColorChangedCmd, kMHexChangedCmd);

	_tabs->setActiveTab(stylesTab);
	NEW_LABEL("GlkOptionsStyles.StyleWindowLabel", _("Window family:"),
		_("Choose prose or fixed-width windows"));
	_styleWindow = new GlkOptionsPopUpWidget(_currentBoss,
		"GlkOptionsStyles.StyleWindow",
		_("Choose prose or fixed-width windows"), kStyleSelectionCmd);
	_styleWindow->appendEntry(_("Prose text"), 0);
	_styleWindow->appendEntry(_("Fixed-width text and status"), 1);
	NEW_LABEL("GlkOptionsStyles.StyleNameLabel", _("Style:"),
		_("GLK text style"));
	_styleName = new GlkOptionsPopUpWidget(_currentBoss,
		"GlkOptionsStyles.StyleName", _("GLK text style"),
		kStyleSelectionCmd);
	_styleName->appendEntry(_("Normal"), style_Normal);
	_styleName->appendEntry(_("Emphasized"), style_Emphasized);
	_styleName->appendEntry(_("Preformatted"), style_Preformatted);
	_styleName->appendEntry(_("Heading"), style_Header);
	_styleName->appendEntry(_("Subheading"), style_Subheader);
	_styleName->appendEntry(_("Alert"), style_Alert);
	_styleName->appendEntry(_("Note"), style_Note);
	_styleName->appendEntry(_("Block quote"), style_BlockQuote);
	_styleName->appendEntry(_("Input"), style_Input);
	_styleName->appendEntry(_("User style 1"), style_User1);
	_styleName->appendEntry(_("User style 2"), style_User2);
	NEW_LABEL("GlkOptionsStyles.StyleFontLabel", _("Font:"),
		_("Font for this style"));
	_styleFont = new GlkOptionsPopUpWidget(_currentBoss,
		"GlkOptionsStyles.StyleFont", _("Font for this style"), kStyleEditedCmd);
	for (int font = MONOR; font <= PROPZ; ++font)
		_styleFont->appendEntry(_(fontFaceLabels[font]), font);
	_resetStyleFont = new GlkOptionsButtonWidget(_currentBoss,
		"GlkOptionsStyles.ResetStyleFont", _("Use interpreter default"),
		_("Remove only this font override"), kResetStyleFontCmd);
	NEW_LABEL("GlkOptionsStyles.StyleForegroundLabel", _("Text color:"),
		_("Complete #RRGGBB color for this style"));
	_styleForeground = new GlkOptionsEditTextWidget(_currentBoss,
		"GlkOptionsStyles.StyleForeground", Common::U32String(),
		_("Complete #RRGGBB color for this style"),
		kStyleForegroundHexCmd);
	createOptionLabel("GlkOptionsStyles.StyleForegroundPrefix", Common::U32String("#"), Common::U32String(),
		GUI::ThemeEngine::kFontStyleNormal);
	_styleForegroundSwatch = new GlkColorSwatchWidget(_currentBoss,
		"GlkOptionsStyles.StyleForegroundSwatch",
		_("Selected text color for this style"));
	_resetStyleForeground = new GlkOptionsButtonWidget(_currentBoss,
		"GlkOptionsStyles.ResetStyleForeground", _("Use interpreter default"),
		_("Remove only this text-color override"),
		kResetStyleForegroundCmd);
	NEW_LABEL("GlkOptionsStyles.StyleBackgroundLabel",
		_("Background color:"),
		_("Complete #RRGGBB color for this style"));
	_styleBackground = new GlkOptionsEditTextWidget(_currentBoss,
		"GlkOptionsStyles.StyleBackground", Common::U32String(),
		_("Complete #RRGGBB color for this style"),
		kStyleBackgroundHexCmd);
	createOptionLabel("GlkOptionsStyles.StyleBackgroundPrefix", Common::U32String("#"), Common::U32String(),
		GUI::ThemeEngine::kFontStyleNormal);
	_styleBackgroundSwatch = new GlkColorSwatchWidget(_currentBoss,
		"GlkOptionsStyles.StyleBackgroundSwatch",
		_("Selected background color for this style"));
	_resetStyleBackground = new GlkOptionsButtonWidget(_currentBoss,
		"GlkOptionsStyles.ResetStyleBackground", _("Use interpreter default"),
		_("Remove only this background-color override"),
		kResetStyleBackgroundCmd);
	_tabs->setActiveTab(interactionTab);
	NEW_LABEL_POPUP("GlkOptionsInteraction.linkstyle", _("Clickable-text underline:"), _linkStyle, "GlkOptionsInteraction.linkStyle", _("Underline used for clickable text"));
	_linkStyle->appendEntry(_("No underline"), 0);
	_linkStyle->appendEntry(_("Thin underline"), 1);
	_linkStyle->appendEntry(_("Thick underline"), 2);

	NEW_LABEL_POPUP("GlkOptionsInteraction.caretshape", _("Insertion cursor:"), _caretShape, "GlkOptionsInteraction.caretShape", _("Shape of the insertion cursor"));
	_caretShape->appendEntry(_("Small dot"), 0);
	_caretShape->appendEntry(_("Large dot"), 1);
	_caretShape->appendEntry(_("Thin line"), 2);
	_caretShape->appendEntry(_("Thick line"), 3);
	_caretShape->appendEntry(_("Block"), 4);

	_tabs->setActiveTab(pagingTab);
	NEW_LABEL_EDIT("GlkOptionsPaging.moreprompt", _("Paging prompt:"),
		_morePrompt, "GlkOptionsPaging.morePrompt",
		_("Marker shown when more text is waiting; leave blank for the game default"));
	static_cast<GlkOptionsEditTextWidget *>(_morePrompt)->setLongText();

	_tabs->setActiveTab(interactionTab);
	NEW_LABEL_CHECKBOX("GlkOptionsInteraction.stylehintlabel", _("Game formatting (restart required):"), _styleHint, "GlkOptionsInteraction.stylehint", _("Allow the game to request formatting changes, including changes to your selected appearance."));
	NEW_LABEL_CHECKBOX("GlkOptionsInteraction.safeclickslabel", _("Safe clicks:"), _safeClicks, "GlkOptionsInteraction.safeclicks", _("Safely apply clicks while input is pending"));

	_tabs->setActiveTab(layoutTab);

	// I18N: This is setting for number of text columns
	NEW_LABEL_EDIT("GlkOptionsLayout.colslbl", _("Content width (columns):"), _cols, "GlkOptionsLayout.cols", _("Maximum text width measured using fixed-width cells"));
	// I18N: This is setting for number of text rows
	NEW_LABEL_EDIT("GlkOptionsLayout.rowslbl", _("Content height (rows):"), _rows, "GlkOptionsLayout.rows", _("Maximum text height measured in rows"));

	NEW_LABEL_CHECKBOX("GlkOptionsLayout.lockcolslbl", _("Limit content width:"), _lockcols, "GlkOptionsLayout.lockcols", _("Enable the content-width limit"));
	NEW_LABEL_CHECKBOX("GlkOptionsLayout.lockrowslbl", _("Limit content height:"), _lockrows, "GlkOptionsLayout.lockrows", _("Enable the content-height limit"));
	_lockcols->setCmd(kLayoutDependencyCmd);
	_lockrows->setCmd(kLayoutDependencyCmd);

	_tabs->setActiveTab(readingTab);
	NEW_LABEL_CHECKBOX("GlkOptionsReading.justifylbl",
		// I18N: This is a setting for enabling text justification
		_("Justify:"),
		_justify, "GlkOptionsReading.justify", _("Enable text justification"));

	_tabs->setActiveTab(pagingTab);

	// I18N: These is setting for type of text quote symbols
	NEW_LABEL_POPUP("GlkOptionsPaging.quoteslbl", _("Typographic quotes:"),
		 _quotes, "GlkOptionsPaging.quotes", _("Choose typographic quotes"));
	_quotes->appendEntry(_c("Off", "quotes"), 0);
	// I18N: This is a setting for using normal typographic quotes, which are like in most books, with the opening quote higher than the closing one
	_quotes->appendEntry(_c("Normal", "quotes"), 1);
	// I18N: This is a setting for using "rabid" quotes, which are like normal typographic quotes but with the opening quote lower than the closing one, like in some comic books
	_quotes->appendEntry(_c("Rabid", "quotes"), 2);

	// I18N: This is a setting for forcing all input to be in uppercase
	_tabs->setActiveTab(interactionTab);
	NEW_LABEL_CHECKBOX("GlkOptionsInteraction.capslbl", _("Uppercase input:"), _caps, "GlkOptionsInteraction.caps", _("Force uppercase input"));

	// I18N: This is a setting for type of text dash symbols
	_tabs->setActiveTab(pagingTab);
	NEW_LABEL_POPUP("GlkOptionsPaging.dasheslbl", _("Dashes:"), _dashes, "GlkOptionsPaging.dashes",
		// I18N: This is a setting for type of text dash symbols
		_("Types of dashes"));
	_dashes->appendEntry(_("Off"), 0);
	// I18N: This is a setting for using normal typographic dashes, which are like in most books, with the em dash being the longest and the en dash being half of it
	_dashes->appendEntry(_("Em dashes"), 1);
	// I18N: This is a setting for using "en+em" dashes, which are like normal typographic dashes but with the en dash being the same length as the em dash, like in some comic books
	_dashes->appendEntry(_("En+Em dashes"), 2);

	// I18N: This is a setting for type of spaces in the text
	NEW_LABEL_POPUP("GlkOptionsPaging.spaceslbl", _("Sentence spacing:"), _spaces, "GlkOptionsPaging.spaces",
		// I18N: This is a setting for type of spaces in the text
		_("Types of spaces"));
	_spaces->appendEntry(_("Off"), 0);
	// I18N: This is a setting for compressing double spaces into single ones in text
	_spaces->appendEntry(_("Compress double spaces"), 1);
	// I18N: This is a setting for expanding single spaces into double ones in text
	_spaces->appendEntry(_("Expand single spaces"), 2);

	_tabs->setActiveTab(interactionTab);
	NEW_LABEL_CHECKBOX("GlkOptionsInteraction.graphicslbl", _("Game pictures (restart required):"), _graphics, "GlkOptionsInteraction.graphics", _("Show pictures supplied by the game"));

	_tabs->setActiveTab(layoutTab);

	NEW_LABEL_EDIT("GlkOptionsLayout.wmarginxlbl", _("Outer margin, horizontal:"), _wmarginx, "GlkOptionsLayout.wmarginx", _("Space between the display edge and GLK windows"));
	NEW_LABEL_EDIT("GlkOptionsLayout.wmarginylbl", _("Outer margin, vertical:"), _wmarginy, "GlkOptionsLayout.wmarginy", _("Space between the display edge and GLK windows"));
	NEW_LABEL_EDIT("GlkOptionsLayout.wpaddingxlbl", _("Pane spacing, horizontal:"), _wpaddingx, "GlkOptionsLayout.wpaddingx", _("Space between adjacent GLK panes"));
	NEW_LABEL_EDIT("GlkOptionsLayout.wpaddingylbl", _("Pane spacing, vertical:"), _wpaddingy, "GlkOptionsLayout.wpaddingy", _("Space between adjacent GLK panes"));
	NEW_LABEL_EDIT("GlkOptionsLayout.tmarginxlbl", _("Text inset, horizontal:"), _tmarginx, "GlkOptionsLayout.tmarginx", _("Space between pane borders and text"));
	NEW_LABEL_EDIT("GlkOptionsLayout.tmarginylbl", _("Text inset, vertical:"), _tmarginy, "GlkOptionsLayout.tmarginy", _("Space between pane borders and text"));

	_tabs->setActiveTab(compatibilityTab);

	// I18N: This is a setting for leading, which is the vertical distance between text rows
	NEW_LABEL_EDIT("GlkOptionsCompatibility.leadinglbl",
		_("Minimum line height:"), _leading,
		"GlkOptionsCompatibility.leading",
		_("Compatibility override for the minimum distance between text rows"));
	// I18N: This is a setting for baseline, which is the invisible horizontal line on which text sits
	NEW_LABEL_EDIT("GlkOptionsCompatibility.baselinelbl",
		_("Text baseline:"), _baseline,
		"GlkOptionsCompatibility.baseline",
		_("Compatibility override for the baseline position"));

	// I18N: Font size scaling for the monospace text font
	_tabs->setActiveTab(readingTab);
	NEW_LABEL("GlkOptionsReading.monosizelbl", "Fixed-width font size:", "Fixed-width font size in points (10-25)");
	NEW_EDIT_CMD(_monosize, "GlkOptionsReading.monosize", "Fixed-width font size in points (10-25)", kPreviewSettingsCmd);
	// I18N: Font size scaling for the proportional text font
	NEW_LABEL("GlkOptionsReading.propsizelbl", "Proportional font size:", "Proportional font size in points (10-25)");
	NEW_EDIT_CMD(_propsize, "GlkOptionsReading.propsize", "Proportional font size in points (10-25)", kPreviewSettingsCmd);

	_tabs->setActiveTab(pagingTab);
	// I18N: This is a setting for alignment of the "More..." prompt in text windows
	NEW_LABEL_POPUP("GlkOptionsPaging.morealignlbl", _("Paging prompt alignment:"),
		_morealign, "GlkOptionsPaging.morealign", _("Alignment of the paging marker"));
	_morealign->appendEntry(_("Left"), 0);
	_morealign->appendEntry(_("Center"), 1);
	_morealign->appendEntry(_("Right"), 2);

	NEW_LABEL_POPUP("GlkOptionsPaging.morefontlbl", _("Paging prompt font:"),
		_morefont, "GlkOptionsPaging.morefont", _("Font for the paging marker"));
	for (int f = MONOR; f <= PROPZ; ++f)
		_morefont->appendEntry(_(fontFaceLabels[f]), f);

	_tabs->setActiveTab(readingTab);
	_currentBoss = widgetsBoss();
	_preview = new GlkOptionsButtonWidget(_currentBoss,
		"GlkOptionsDialog.Preview", _("Preview"),
		_("Preview the current draft without changing the running game"),
		kPreviewCmd);
	_restoreDefaults = new GlkOptionsButtonWidget(_currentBoss,
		"GlkOptionsDialog.RestoreDefaults", _("Use Global Defaults"),
		_("Remove this target's overrides for settings in this editor. Global preferences remain applicable."),
		kRestoreDefaultsCmd);
	_interpreterDefaults = new GlkOptionsButtonWidget(_currentBoss,
		"GlkOptionsDialog.InterpreterDefaults", _("Use Interpreter Defaults"),
		_("Remove this target's overrides and ignore global preferences for settings in this editor."),
		kInterpreterDefaultsCmd);
	_saveGlobal = new GlkOptionsButtonWidget(_currentBoss,
		"GlkOptionsDialog.SaveGlobal", _("Save Customizations as Global Defaults"),
		_("Stage this target's explicit customizations as global defaults. Later edits stay target-specific. OK saves; Cancel discards."),
		kSaveGlobalCmd);
	createOptionLabel("GlkOptionsDialog.RestartNotice",
		_("Appearance changes may apply the next time this game starts."), Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
	createOptionLabel("GlkOptionsDialog.PreferenceNotice",
		_("Per-game settings override global settings. If neither specifies a value, interpreter defaults apply.\nChoose OK to save your changes, or Cancel to discard them."), Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
	createOptionLabel("GlkOptionsDialog.ActionStatus", Common::U32String(), Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
	createOptionLabel("GlkOptionsDialog.PreviewStatus", Common::U32String(), Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
	findOptionLabel("GlkOptionsDialog.PreviewStatus")->setVisible(false);

}

GlkOptionsWidget::~GlkOptionsWidget() {
	delete _settings;
}

GUI::StaticTextWidget *GlkOptionsWidget::createOptionLabel(
		const Common::String &name, const Common::U32String &text,
		const Common::U32String &tooltip, GUI::ThemeEngine::FontStyle font) {
	GUI::StaticTextWidget *label = new GlkOptionsLabelWidget(_currentBoss,
		name, text, tooltip, font);
	_optionLabels.push_back(label);
	return label;
}

GUI::StaticTextWidget *GlkOptionsWidget::findOptionLabel(
		const Common::String &name) const {
	for (uint index = 0; index < _optionLabels.size(); ++index) {
		if (_optionLabels[index]->getName() == name)
			return _optionLabels[index];
	}
	return nullptr;
}

void GlkOptionsWidget::setHostContext(HostContext context) {
	_hostContext = context;
	_preview->setVisible(context == kLauncherOptions);
	_saveGlobal->setVisible(context == kLauncherOptions);
	GUI::StaticTextWidget *status = findOptionLabel("GlkOptionsDialog.PreviewStatus");
	status->setVisible(context == kLauncherOptions && !status->getLabel().empty());
}

void GlkOptionsWidget::updateActionState() {
	GUI::StaticTextWidget *status = findOptionLabel("GlkOptionsDialog.ActionStatus");
	status->setLabel(_settings->hasGlobalChanges() ?
		_("Global defaults are staged.") : Common::U32String());
}

void GlkOptionsWidget::setDomain(const Common::String &domain) {
	if (_domain == domain)
		return;

	GUI::OptionsContainerWidget::setDomain(domain);
	_settings->setDomain(domain);
}

void GlkOptionsWidget::focusControl(GUI::Widget *widget) {
	const char *pages[] = { "GlkOptionsReading.", "GlkOptionsCompatibility.", "GlkOptionsColors.",
		"GlkOptionsStyles.", "GlkOptionsLayout.", "GlkOptionsInteraction.", "GlkOptionsPaging." };
	for (uint i = 0; i < ARRAYSIZE(pages); ++i) {
		if (widget->getName().hasPrefix(pages[i]))
			selectPage(i, false);
	}
	if (_parentDialog)
		_parentDialog->setFocusWidget(widget);
	GUI::ScrollContainerWidget *scroll = dynamic_cast<GUI::ScrollContainerWidget *>(_boss);
	if (scroll) {
		if (widget == _tabs) {
			// Only the strip, not the entire page body, needs to be visible.
			if (widget->getAbsY() < scroll->getAbsY() ||
					widget->getChildY() > scroll->getAbsY() + scroll->getHeight())
				scroll->setScrollPosition(scroll->getScrollPosition() + widget->getAbsY() - scroll->getAbsY());
		} else {
			scroll->ensureVisible(widget);
		}
	}
}

bool GlkOptionsWidget::selectPage(int page, bool userRequested) {
	if (userRequested)
		return _tabs->requestActiveTab(page);
	if (page == _tabs->getActiveTab())
		return true;
	_tabs->setActiveTab(page);
	reflowAfterPageChange(false);
	return true;
}

void GlkOptionsWidget::reflowAfterPageChange(bool userRequested) {
	GUI::ScrollContainerWidget *scroll = dynamic_cast<GUI::ScrollContainerWidget *>(_boss);
	const int position = scroll ? scroll->getScrollPosition() : 0;
	if (_parentDialog) {
		_parentDialog->reflowLayout();
	} else if (_w > 0 && _h > 0) {
		reflowLayout();
	}
	if (scroll)
		scroll->setScrollPosition(position);
	if (userRequested)
		focusControl(_tabs);
	// The old page and footer can extend beyond their new bounds.
	g_gui.redrawFull();
}

bool GlkOptionsWidget::handleOptionsKeyDown(Common::KeyState state) {
	if (!_parentDialog)
		return false;
	if (state.keycode == Common::KEYCODE_ESCAPE) {
		GUI::OptionsContainerWidget::handleCommand(nullptr, GUI::kCloseWithResultCmd, (uint32)-1);
		return true;
	}
	GUI::Widget *focused = _parentDialog->getFocusWidget();
	if (focused && focused != this && !containsWidget(focused))
		return false;
	const bool backwards = (state.flags & Common::KBD_SHIFT) != 0;
	if ((state.flags & Common::KBD_CTRL) && (state.keycode == Common::KEYCODE_TAB ||
			state.keycode == Common::KEYCODE_PAGEUP || state.keycode == Common::KEYCODE_PAGEDOWN)) {
		const int direction = backwards || state.keycode == Common::KEYCODE_PAGEUP ? -1 : 1;
		selectPage((_tabs->getActiveTab() + direction + _tabs->getTabCount()) % _tabs->getTabCount(), true);
		return true;
	}
	if (focused == _tabs && (state.keycode == Common::KEYCODE_LEFT || state.keycode == Common::KEYCODE_RIGHT)) {
		int direction = state.keycode == Common::KEYCODE_LEFT ? -1 : 1;
		if (g_gui.useRTL())
			direction = -direction;
		selectPage((_tabs->getActiveTab() + direction + _tabs->getTabCount()) % _tabs->getTabCount(), true);
		return true;
	}
	if (state.keycode == Common::KEYCODE_TAB) {
		// Finishing an edit can enable or disable a property reset button.
		// Refresh before choosing the next focus target.
		if (focused && focused->getType() == GUI::kEditTextWidget && controlChanged(focused))
			synchronizeDraft(false, focused);
		Common::Array<GUI::Widget *> controls;
		GUI::Widget *widget = static_cast<GlkOptionsTabWidget *>(_tabs)->getActivePageWidgets();
		for (; widget; widget = widget->next()) {
			if (widget->isVisible() && widget->isEnabled() &&
					(widget->getType() == GUI::kEditTextWidget || widget->getType() == GUI::kPopUpWidget ||
					widget->getType() == GUI::kCheckboxWidget || widget->getType() == GUI::kButtonWidget)) {
				uint index = 0;
				while (index < controls.size() && (controls[index]->getRelY() < widget->getRelY() ||
						(controls[index]->getRelY() == widget->getRelY() &&
						(g_gui.useRTL() ? controls[index]->getRelX() > widget->getRelX() : controls[index]->getRelX() < widget->getRelX()))))
					++index;
				controls.insert_at(index, widget);
			}
		}
		controls.insert_at(0, _tabs);
		if (_preview->isVisible() && _preview->isEnabled())
			controls.push_back(_preview);
		controls.push_back(_restoreDefaults);
		controls.push_back(_interpreterDefaults);
		if (_saveGlobal->isVisible())
			controls.push_back(_saveGlobal);
		int current = -1;
		for (uint i = 0; i < controls.size(); ++i) {
			if (controls[i] == focused)
				current = i;
		}
		const int next = current < 0 ? (backwards ? controls.size() - 1 : 0) : current + (backwards ? -1 : 1);
		if (next < 0 || next >= (int)controls.size()) {
			_parentDialog->releaseFocus();
			return false;
		}
		focusControl(controls[next]);
		return true;
	}
	if (focused && focused != _tabs && focused->getType() != GUI::kEditTextWidget) {
		if (state.keycode == Common::KEYCODE_RETURN || state.keycode == Common::KEYCODE_SPACE) {
			focused->handleMouseDown(1, 1, 1, 1);
			focused->handleMouseUp(1, 1, 1, 1);
			return true;
		}
		if (focused->getType() == GUI::kPopUpWidget && (state.keycode == Common::KEYCODE_UP || state.keycode == Common::KEYCODE_DOWN)) {
			focused->handleMouseWheel(1, 1, state.keycode == Common::KEYCODE_UP ? -1 : 1);
			return true;
		}
	}
	return false;
}

static void setIntegerWidget(GUI::EditTextWidget *widget, int value) {
	widget->setEditString(Common::U32String(Common::String::format("%d", value)));
}

static bool showInvalidOption();
static Common::String controlValue(GUI::Widget *widget);

static void setSimpleColorPopUp(GUI::PopUpWidget *popup,
		GUI::EditTextWidget *hexInput, GlkColorSwatchWidget *swatch,
		const Common::String &color, bool overrideEnabled,
		int defaultValue = 7) {
	swatch->setColor(color);
	hexInput->setEditString(Common::U32String(color));
	if (!overrideEnabled) {
		popup->setSelectedTag(defaultValue);
		return;
	}


	unsigned int rgb = 0;
	sscanf(color.c_str(), "%x", &rgb);
	bool found = false;

	// See if it matches one of our predefined 6 colors
	for (int i = 0; i <= 5; ++i) {
		if (GLK_COLORS[i].rgb == rgb) {
			popup->setSelectedTag(i);
			found = true;
			break;
		}
	}

	// If it doesn't match standard colors, set to Custom (6)
	if (!found) {
		popup->setSelectedTag(6);
	}
}

void GlkOptionsWidget::loadColor(const ColorBinding &color) {
	Common::String value;
	bool explicitValue = true;
	if (color.isNormalForeground()) {
		value = _settings->getStyleForeground(color.target == kNormalGridForeground, style_Normal);
	} else {
		uint fallback = 0;
		switch (color.target) {
		case kWindowBackground: fallback = _conf->_windowColor; break;
		case kDividerColor: fallback = _conf->_borderColor; break;
		case kCaretColor: fallback = _conf->_propInfo._caretColor; break;
		case kLinkColor: fallback = _conf->_propInfo._linkColor; break;
		case kMoreColor: fallback = _conf->_propInfo._moreColor; break;
		default: break;
		}
		value = _settings->getColor(color.key, fallback);
		explicitValue = _settings->source(color.key) != kGlkInterpreterDefault;
	}
	setSimpleColorPopUp(color.popup, color.hex, color.swatch, value, explicitValue);
}

void GlkOptionsWidget::load() {
	_loading = true;
	_conf = &_settings->resolved();
	_tfontPopUp->setSelectedTag(_settings->getStyleFont(false, style_Normal));
	for (uint i = 0; i < ARRAYSIZE(_colors); ++i)
		loadColor(_colors[i]);

	setIntegerWidget(_wmarginx, _settings->getInt("wmarginx", _conf->_wMarginX));
	setIntegerWidget(_wmarginy, _settings->getInt("wmarginy", _conf->_wMarginY));
	setIntegerWidget(_wpaddingx, _settings->getInt("wpaddingx", _conf->_wPaddingX));
	setIntegerWidget(_wpaddingy, _settings->getInt("wpaddingy", _conf->_wPaddingY));
	setIntegerWidget(_wborderx, _settings->getInt("wborderx", _conf->_wBorderX));
	setIntegerWidget(_wbordery, _settings->getInt("wbordery", _conf->_wBorderY));
	setIntegerWidget(_tmarginx, _settings->getInt("tmarginx", _conf->_tMarginX));
	setIntegerWidget(_tmarginy, _settings->getInt("tmarginy", _conf->_tMarginY));
	setIntegerWidget(_leading, _settings->getInt("leading", _conf->_monoInfo._leading));
	setIntegerWidget(_baseline, _settings->getInt("baseline", _conf->_propInfo._baseLine));
	setIntegerWidget(_cols, _settings->getInt("cols", _conf->_cols));
	setIntegerWidget(_rows, _settings->getInt("rows", _conf->_rows));

	_safeClicks->setState(_settings->getBool("safeclicks", _conf->_safeClicks));
	_styleHint->setState(_settings->getBool("stylehint", _conf->_styleHint != 0));
	_lockcols->setState(_settings->getBool("lockcols", _conf->_lockCols != 0));
	_lockrows->setState(_settings->getBool("lockrows", _conf->_lockRows != 0));
	_justify->setState(_settings->getBool("justify", _conf->_propInfo._justify != 0));
	_caps->setState(_settings->getBool("caps", _conf->_propInfo._caps != 0));
	_graphics->setState(_settings->getBool("graphics", _conf->_graphics));

	_linkStyle->setSelectedTag(_settings->getInt("linkstyle", _conf->_propInfo._linkStyle));
	_caretShape->setSelectedTag(_settings->getInt("caretshape", _conf->_propInfo._caretShape));
	const Common::U32String morePrompt = Common::U32String(_settings->getString("moreprompt",
		_conf->_propInfo._morePrompt));
	_morePrompt->setEditString(morePrompt);
	_quotes->setSelectedTag(_settings->getInt("quotes", _conf->_propInfo._quotes));
	_dashes->setSelectedTag(_settings->getInt("dashes", _conf->_propInfo._dashes));
	_spaces->setSelectedTag(_settings->getInt("spaces", _conf->_propInfo._spaces));
	const Common::String monoSize = _settings->hasDraftPreference("monosize") ?
		_settings->getDraftPreference("monosize") :
		Common::String::format("%g", _conf->_monoInfo._size);
	const Common::String propSize = _settings->hasDraftPreference("propsize") ?
		_settings->getDraftPreference("propsize") :
		Common::String::format("%g", _conf->_propInfo._size);
	_monosize->setEditString(Common::U32String(monoSize));
	_propsize->setEditString(Common::U32String(propSize));
	_morealign->setSelectedTag(_settings->getInt("morealign", _conf->_propInfo._moreAlign));
	_morefont->setSelectedTag(_settings->getFont("morefont", _conf->_propInfo._moreFont));
	if (_styleWindow->getSelected() < 0)
		_styleWindow->setSelectedTag(0);
	if (_styleName->getSelected() < 0)
		_styleName->setSelectedTag(style_Normal);
	loadStyleEditor();
	updateLayoutDependencies();
	rememberControls();
	_loading = false;
	updateActionState();
	updatePreviewAvailability();
}

void GlkOptionsWidget::updatePreviewAvailability() {
	if (_hostContext != kLauncherOptions) {
		_preview->setEnabled(false);
		return;
	}
	GlkOptionsState candidate(*_settings);
	GUI::EditTextWidget *sizes[] = { _monosize, _propsize };
	const char *keys[] = { "monosize", "propsize" };
	for (uint i = 0; i < ARRAYSIZE(sizes); ++i) {
		if (controlChanged(sizes[i]))
			candidate.commitString(keys[i], sizes[i]->getEditString().encode());
	}
	const Common::U32String reason = glkPreviewUnavailableReason(candidate);
	_preview->setEnabled(reason.empty());
	GUI::StaticTextWidget *status = findOptionLabel("GlkOptionsDialog.PreviewStatus");
	status->setVisible(!reason.empty());
	if (status->getLabel() != reason) {
		status->setLabel(reason);
		if (_parentDialog)
			_parentDialog->reflowLayout();
	}
	_preview->setTooltip(reason.empty() ? _("Preview the current draft without changing the running game") : reason);
}

void GlkOptionsWidget::updateLayoutDependencies() {
	_cols->setEnabled(_lockcols->getState());
	_rows->setEnabled(_lockrows->getState());
}

void GlkOptionsWidget::loadStyleEditor(GUI::Widget *only) {
	if (!only) {
		_loadedStyleGrid = _styleWindow->getSelectedTag() == 1;
		_loadedStyle = _styleName->getSelectedTag();
		if (_loadedStyle < 0 || _loadedStyle >= style_NUMSTYLES)
			_loadedStyle = style_Normal;
	}
	if (!only || only == _styleFont) {
		const FACES selectedFont = _settings->getStyleFont(_loadedStyleGrid, _loadedStyle);
		_styleFont->clearEntries();
		const int lastFont = _loadedStyleGrid ? MONOZ : PROPZ;
		for (int font = MONOR; font <= lastFont; ++font)
			_styleFont->appendEntry(_(fontFaceLabels[font]), font);
		if (_loadedStyleGrid && selectedFont >= PROPR)
			_styleFont->appendEntry(_(fontFaceLabels[selectedFont]), selectedFont);
		_styleFont->setSelectedTag(selectedFont);
	}
	if (!only || only == _styleForeground) {
		const Common::String foreground = _settings->getStyleForeground(_loadedStyleGrid, _loadedStyle);
		_styleForeground->setEditString(Common::U32String(foreground));
		_styleForegroundSwatch->setColor(foreground);
	}
	if (!only || only == _styleBackground) {
		const Common::String background = _settings->getStyleBackground(_loadedStyleGrid, _loadedStyle);
		_styleBackground->setEditString(Common::U32String(background));
		_styleBackgroundSwatch->setColor(background);
	}
	GUI::ButtonWidget *buttons[] = { _resetStyleFont, _resetStyleForeground, _resetStyleBackground };
	GUI::Widget *controls[] = { _styleFont, _styleForeground, _styleBackground };
	const StyleProperty properties[] = { kStyleFont, kStyleForeground, kStyleBackground };
	for (uint i = 0; i < ARRAYSIZE(buttons); ++i) {
		if (only && only != controls[i])
			continue;
		const bool overridden = _settings->isStylePropertyOverridden(_loadedStyleGrid, _loadedStyle, properties[i]);
		const bool global = _settings->styleDefaultSource(_loadedStyleGrid, _loadedStyle, properties[i]) == kGlkApplicationPreference;
		buttons[i]->setEnabled(overridden);
		buttons[i]->setLabel(global ? (overridden ? _("Use global default") : _("Global default")) :
			(overridden ? _("Use interpreter default") : _("Interpreter default")));
		rememberControl(controls[i]);
	}
}

bool GlkOptionsWidget::synchronizeStyleEditor(GlkOptionsState &pending, bool reportError, GUI::Widget *only) {
	GUI::Widget *controls[] = { _styleFont, _styleForeground, _styleBackground };
	for (uint i = 0; i < ARRAYSIZE(controls); ++i) {
		if ((only && only != controls[i]) || !controlChanged(controls[i]))
			continue;
		if (i == 0) {
			const int font = _styleFont->getSelectedTag();
			if (font < MONOR || font > PROPZ || (_loadedStyleGrid && font >= PROPR))
				return invalidControl(_styleFont, reportError);
			pending.commitStyleFont(_loadedStyleGrid, _loadedStyle, (FACES)font);
		} else {
			const Common::String color = controlValue(controls[i]);
			Common::String normalized;
			if (!GlkOptionsState::parseColor(color, normalized))
				return invalidControl(controls[i], reportError);
			pending.commitStyleColor(_loadedStyleGrid, _loadedStyle, i == 1, color);
		}
	}
	return true;
}

void GlkOptionsWidget::defineLayout(GUI::ThemeEval &layouts,
		const Common::String &layoutName,
		const Common::String &overlayedLayout) const {
	GlkOptionThemeMetrics themeMetrics;
	themeMetrics.textHeight = MAX(
		g_gui.xmlEval()->getVar("Globals.Line.Height", 16),
		g_gui.getFontHeight(GUI::ThemeEngine::kFontStyleBold));
	themeMetrics.editHeight = MAX(themeMetrics.textHeight,
		g_gui.getFontHeight(GUI::ThemeEngine::kFontStyleNormal) + 3);
	themeMetrics.popupHeight = layouts.getVar("Globals.PopUp.Height",
		themeMetrics.textHeight);
	themeMetrics.checkboxHeight = layouts.getVar("Globals.Checkbox.Height",
		themeMetrics.textHeight);
	themeMetrics.buttonHeight = layouts.getVar("Globals.Button.Height",
		themeMetrics.textHeight);
	themeMetrics.spacing = MAX(1, themeMetrics.textHeight / 4);
	const int lineHeight = themeMetrics.textHeight;
	int availableWidth = getWidth();
	int16 overlayX = 0;
	int16 overlayY = 0;
	int16 overlayWidth = 0;
	int16 overlayHeight = 0;
	if (layouts.getWidgetData(overlayedLayout, overlayX, overlayY,
			overlayWidth, overlayHeight) && overlayWidth > 0)
		availableWidth = overlayWidth;

	const int horizontalPadding = layouts.getVar(
		"Globals.TabWidget.Body.Padding.Left", 0) + layouts.getVar(
		"Globals.TabWidget.Body.Padding.Right", 0);
	availableWidth = MAX(1,
		availableWidth - horizontalPadding);
	const float scale = g_gui.getScaleFactor();
	const int pageInset = getGlkOptionPageInset(themeMetrics.textHeight, scale);
	const int effectivePageInset = (int)(pageInset * scale);
	const int contentWidth = MAX(1,
		availableWidth - effectivePageInset * 2);
	const int labelWidth = contentWidth / 3;
	const int rowSpacing = themeMetrics.spacing;

	GlkOptionsPageLayout reading(layouts, *this, "GlkOptionsReading",
		contentWidth, labelWidth, pageInset, effectivePageInset,
		themeMetrics);
	reading.addOptionRow("TFont", "TFont0", _tfontPopUp, "PopUp");
	reading.addExplanation("GridExplanation");
	reading.addOptionRow("propsizelbl", "propsize", _propsize,
		"EditTextWidget");
	reading.addOptionRow("monosizelbl", "monosize", _monosize,
		"EditTextWidget");
	reading.addOptionRow("justifylbl", "justify", _justify, "Checkbox");
	reading.close();

	GlkOptionsPageLayout compatibility(layouts, *this,
		"GlkOptionsCompatibility", contentWidth, labelWidth,
		pageInset, effectivePageInset, themeMetrics);
	compatibility.addOptionRow("leadinglbl", "leading", _leading,
		"EditTextWidget");
	compatibility.addOptionRow("baselinelbl", "baseline", _baseline,
		"EditTextWidget");
	compatibility.close();

	GlkOptionsPageLayout colors(layouts, *this, "GlkOptionsColors",
		contentWidth, labelWidth, pageInset, effectivePageInset,
		themeMetrics);
	for (uint i = 0; i < ARRAYSIZE(_colors); ++i) {
		const ColorBinding &color = _colors[i];
		colors.addColorRow(color.prefix, color.popup, color.hex, color.swatch);
	}
	colors.close();

	GlkOptionsPageLayout styles(layouts, *this, "GlkOptionsStyles",
		contentWidth, labelWidth, pageInset, effectivePageInset,
		themeMetrics);
	styles.addOptionRow("StyleWindowLabel", "StyleWindow", _styleWindow,
		"PopUp");
	styles.addOptionRow("StyleNameLabel", "StyleName", _styleName,
		"PopUp");
	styles.setStyleColumns(_styleFont, _styleForeground, _styleBackground);
	styles.addStyleRow("StyleFontLabel", "StyleFont", _styleFont,
		"PopUp", "ResetStyleFont", _resetStyleFont);
	styles.addStyleColorRow("StyleForegroundLabel", "StyleForegroundSwatch",
		_styleForegroundSwatch, "StyleForeground", _styleForeground,
		"ResetStyleForeground", _resetStyleForeground);
	styles.addStyleColorRow("StyleBackgroundLabel", "StyleBackgroundSwatch",
		_styleBackgroundSwatch, "StyleBackground", _styleBackground,
		"ResetStyleBackground", _resetStyleBackground);
	styles.close();

	GlkOptionsPageLayout layoutPage(layouts, *this, "GlkOptionsLayout",
		contentWidth, labelWidth, pageInset, effectivePageInset,
		themeMetrics);
	const char *layoutCheckboxes[] = { "lockcolslbl", "lockrowslbl" };
	layoutPage.setCheckboxLabels(layoutCheckboxes, ARRAYSIZE(layoutCheckboxes));
	layoutPage.addOptionRow("wmarginxlbl", "wmarginx", _wmarginx, "EditTextWidget");
	layoutPage.addOptionRow("wmarginylbl", "wmarginy", _wmarginy, "EditTextWidget");
	layoutPage.addOptionRow("tmarginxlbl", "tmarginx", _tmarginx, "EditTextWidget");
	layoutPage.addOptionRow("tmarginylbl", "tmarginy", _tmarginy, "EditTextWidget");
	layoutPage.addOptionRow("wpaddingxlbl", "wpaddingx", _wpaddingx, "EditTextWidget");
	layoutPage.addOptionRow("wpaddingylbl", "wpaddingy", _wpaddingy, "EditTextWidget");
	layoutPage.addOptionRow("wborderx", "wborderhorizontal", _wborderx, "EditTextWidget");
	layoutPage.addOptionRow("wbordery", "wbordervertical", _wbordery, "EditTextWidget");
	layoutPage.addOptionRow("lockcolslbl", "lockcols", _lockcols, "Checkbox");
	layoutPage.addOptionRow("colslbl", "cols", _cols, "EditTextWidget");
	layoutPage.addOptionRow("lockrowslbl", "lockrows", _lockrows, "Checkbox");
	layoutPage.addOptionRow("rowslbl", "rows", _rows, "EditTextWidget");
	layoutPage.close();

	GlkOptionsPageLayout interaction(layouts, *this,
		"GlkOptionsInteraction", contentWidth, labelWidth,
		pageInset, effectivePageInset, themeMetrics);
	const char *interactionCheckboxes[] = { "capslbl", "stylehintlabel", "safeclickslabel", "graphicslbl" };
	interaction.setCheckboxLabels(interactionCheckboxes, ARRAYSIZE(interactionCheckboxes));
	interaction.addOptionRow("caretshape", "caretShape", _caretShape,
		"PopUp");
	interaction.addOptionRow("linkstyle", "linkStyle", _linkStyle,
		"PopUp");
	interaction.addOptionRow("capslbl", "caps", _caps, "Checkbox");
	interaction.addOptionRow("stylehintlabel", "stylehint", _styleHint,
		"Checkbox");
	interaction.addOptionRow("safeclickslabel", "safeclicks", _safeClicks,
		"Checkbox");
	interaction.addOptionRow("graphicslbl", "graphics", _graphics,
		"Checkbox");
	interaction.close();

	GlkOptionsPageLayout paging(layouts, *this, "GlkOptionsPaging",
		contentWidth, labelWidth, pageInset, effectivePageInset,
		themeMetrics);
	paging.addOptionRow("moreprompt", "morePrompt", _morePrompt,
		"EditTextWidget");
	paging.addOptionRow("morealignlbl", "morealign", _morealign, "PopUp");
	paging.addOptionRow("morefontlbl", "morefont", _morefont, "PopUp");
	paging.addOptionRow("quoteslbl", "quotes", _quotes, "PopUp");
	paging.addOptionRow("dasheslbl", "dashes", _dashes, "PopUp");
	paging.addOptionRow("spaceslbl", "spaces", _spaces, "PopUp");
	paging.close();

	const int pageHeights[] = { reading.getContentHeight(), compatibility.getContentHeight(),
		colors.getContentHeight(), styles.getContentHeight(), layoutPage.getContentHeight(),
		interaction.getContentHeight(), paging.getContentHeight() };
	const int pageHeight = pageHeights[_tabs->getActiveTab()];
	const int tabHeaderHeight = layouts.getVar(
		"Globals.TabWidget.Tab.Height", lineHeight);
	const int tabHeight = tabHeaderHeight + pageHeight;
	const int footerGap = MAX(rowSpacing * 2, themeMetrics.buttonHeight / 2);
	GUI::ButtonWidget *buttons[] = { _preview, _restoreDefaults, _interpreterDefaults, _saveGlobal };
	const char *names[] = { "Preview", "RestoreDefaults", "InterpreterDefaults", "SaveGlobal" };
	const Common::U32String labels[] = { _("Preview"), _("Use Global Defaults"), _("Use Interpreter Defaults"), _("Save Customizations as Global Defaults") };
	const Common::U32String shortLabels[] = { _("Preview"), _("Global Defaults"), _("Interpreter Defaults"), _("Save Globally") };
	int widths[4];
	int totalWidth = 0;
	for (uint i = 0; i < ARRAYSIZE(buttons); ++i) {
		buttons[i]->setLabel(labels[i]);
		int ignoredHeight;
		buttons[i]->getMinSize(widths[i], ignoredHeight);
		if (widths[i] + rowSpacing * 2 > availableWidth) {
			buttons[i]->setLabel(shortLabels[i]);
			buttons[i]->getMinSize(widths[i], ignoredHeight);
		}
		widths[i] = MIN(availableWidth, widths[i] + rowSpacing * 2);
		if (buttons[i]->isVisible())
			totalWidth += widths[i] + rowSpacing;
	}
	const bool singleRow = totalWidth - rowSpacing <= availableWidth;
	const int columnWidths[] = { MAX(widths[0], widths[2]), MAX(widths[1], widths[3]) };
	const bool twoColumns = !singleRow && _hostContext == kLauncherOptions &&
		columnWidths[0] + rowSpacing + columnWidths[1] <= availableWidth;
	const GlkOptionsLabelWidget *notice = static_cast<GlkOptionsLabelWidget *>(findOptionLabel("GlkOptionsDialog.RestartNotice"));
	const GlkOptionsLabelWidget *preferences = static_cast<GlkOptionsLabelWidget *>(findOptionLabel("GlkOptionsDialog.PreferenceNotice"));
	const GlkOptionsLabelWidget *status = static_cast<GlkOptionsLabelWidget *>(findOptionLabel("GlkOptionsDialog.ActionStatus"));
	const GlkOptionsLabelWidget *previewStatus = static_cast<GlkOptionsLabelWidget *>(findOptionLabel("GlkOptionsDialog.PreviewStatus"));
	layouts.addDialog(layoutName, overlayedLayout)
		.addPadding(0, 0, 0, 0)
		.addLayout(GUI::ThemeLayout::kLayoutVertical, 0)
			.addPadding(0, 0, 0, 0)
			.addWidget("TabWidget", "TabWidget", -1, tabHeight, Graphics::kTextAlignStart, true, true)
			.addSpace(footerGap)
			.addWidget("RestartNotice", "", -1, notice->wrappedHeight(availableWidth), Graphics::kTextAlignStart, true, true)
			.addSpace(rowSpacing)
			.addLayout(singleRow ? GUI::ThemeLayout::kLayoutHorizontal : GUI::ThemeLayout::kLayoutVertical, rowSpacing)
				.addPadding(0, 0, 0, 0);
	for (uint i = 0; i < ARRAYSIZE(buttons); ++i) {
		if (!buttons[i]->isVisible())
			continue;
		if (twoColumns && i % 2 == 0)
			layouts.addLayout(GUI::ThemeLayout::kLayoutHorizontal, rowSpacing).addPadding(0, 0, 0, 0);
		layouts.addWidget(names[i], "", twoColumns ? columnWidths[i % 2] : widths[i], themeMetrics.buttonHeight,
			Graphics::kTextAlignCenter, true, true);
		if (twoColumns && i % 2 == 1)
			layouts.closeLayout();
	}
	layouts.closeLayout()
			.addSpace(rowSpacing)
			.addWidget("PreferenceNotice", "", -1, preferences->wrappedHeight(availableWidth), Graphics::kTextAlignStart, true, true)
			.addSpace(rowSpacing)
			.addWidget("ActionStatus", "", -1, status->wrappedHeight(availableWidth), Graphics::kTextAlignStart, true, true);
	if (_hostContext == kLauncherOptions && !previewStatus->getLabel().empty())
		layouts.addSpace(rowSpacing).addWidget("PreviewStatus", "", -1, previewStatus->wrappedHeight(availableWidth), Graphics::kTextAlignStart, true, true);
	layouts.closeLayout()
		.closeDialog();
}

static bool getColorDraft(GUI::PopUpWidget *popup,
		GUI::EditTextWidget *hexInput, bool allowDefault, bool &useDefault,
		Common::String &color) {
	const int index = popup->getSelectedTag();
	useDefault = allowDefault && index == 7;
	if (useDefault)
		return true;

	if (index >= 0 && index <= 5) {
		color = Common::String::format("%06x", GLK_COLORS[index].rgb);
		return true;
	}

	if (index != 6 || !hexInput)
		return false;
	return GlkOptionsState::parseColor(hexInput->getEditString().encode(), color);
}

static bool showInvalidOption() {
	GUI::MessageDialog dialog(_("One or more GLK appearance values are invalid. "
		"No GLK settings were changed."));
	dialog.runModal();
	return false;
}

bool GlkOptionsWidget::invalidControl(GUI::Widget *widget, bool reportError) {
	widget->setInvalid(true);
	if (!reportError)
		return false;
	focusControl(widget);
	return showInvalidOption();
}

static Common::String controlValue(GUI::Widget *widget) {
	GUI::EditTextWidget *edit = dynamic_cast<GUI::EditTextWidget *>(widget);
	GUI::PopUpWidget *popup = dynamic_cast<GUI::PopUpWidget *>(widget);
	GUI::CheckboxWidget *checkbox = dynamic_cast<GUI::CheckboxWidget *>(widget);
	if (edit)
		return edit->getEditString().encode();
	if (popup)
		return Common::String::format("%d", popup->getSelectedTag());
	if (checkbox)
		return checkbox->getState() ? "true" : "false";
	return Common::String();
}

void GlkOptionsWidget::rememberControl(GUI::Widget *widget) {
	widget->setInvalid(false);
	_loadedControls.setVal(widget->getName(), controlValue(widget));
	_editedControls.erase(widget->getName());
}

void GlkOptionsWidget::refreshControl(GUI::Widget *widget) {
	_conf = &_settings->resolved();
	refreshControlValue(widget);
	updateLayoutDependencies();
	updatePreviewAvailability();
}

void GlkOptionsWidget::refreshControlValue(GUI::Widget *widget) {
	if (widget == _styleFont || widget == _styleForeground || widget == _styleBackground) {
		loadStyleEditor(widget);
		if (_loadedStyle != style_Normal || widget == _styleBackground)
			return;
		if (widget == _styleFont) {
			if (_loadedStyleGrid)
				return;
			widget = _tfontPopUp;
		} else {
			widget = _colors[_loadedStyleGrid ? kNormalGridForeground : kNormalProseForeground].popup;
		}
	}
	if (widget == _tfontPopUp) {
		_tfontPopUp->setSelectedTag(_settings->getStyleFont(false, style_Normal));
		if (!_loadedStyleGrid && _loadedStyle == style_Normal)
			loadStyleEditor(_styleFont);
	}
	for (uint i = 0; i < ARRAYSIZE(_colors); ++i) {
		const ColorBinding &color = _colors[i];
		if (widget != color.popup && widget != color.hex)
			continue;
		loadColor(color);
		rememberControl(color.popup);
		rememberControl(color.hex);
		if (color.isNormalForeground() && _loadedStyle == style_Normal &&
				_loadedStyleGrid == (color.target == kNormalGridForeground))
			loadStyleEditor(_styleForeground);
	}
	rememberControl(widget);
}

void GlkOptionsWidget::rememberControls() {
	_editedControls.clear();
	if (_controls.empty()) {
		_controls.push_back(ControlBinding("wmarginx", _wmarginx));
		_controls.push_back(ControlBinding("wmarginy", _wmarginy));
		_controls.push_back(ControlBinding("wpaddingx", _wpaddingx));
		_controls.push_back(ControlBinding("wpaddingy", _wpaddingy));
		_controls.push_back(ControlBinding("wborderx", _wborderx));
		_controls.push_back(ControlBinding("wbordery", _wbordery));
		_controls.push_back(ControlBinding("tmarginx", _tmarginx));
		_controls.push_back(ControlBinding("tmarginy", _tmarginy));
		_controls.push_back(ControlBinding("leading", _leading));
		_controls.push_back(ControlBinding("baseline", _baseline));
		_controls.push_back(ControlBinding("cols", _cols));
		_controls.push_back(ControlBinding("rows", _rows));
		_controls.push_back(ControlBinding("safeclicks", _safeClicks));
		_controls.push_back(ControlBinding("stylehint", _styleHint));
		_controls.push_back(ControlBinding("lockcols", _lockcols));
		_controls.push_back(ControlBinding("lockrows", _lockrows));
		_controls.push_back(ControlBinding("justify", _justify));
		_controls.push_back(ControlBinding("caps", _caps));
		_controls.push_back(ControlBinding("graphics", _graphics));
		_controls.push_back(ControlBinding("linkstyle", _linkStyle));
		_controls.push_back(ControlBinding("caretshape", _caretShape));
		_controls.push_back(ControlBinding("moreprompt", _morePrompt));
		_controls.push_back(ControlBinding("quotes", _quotes));
		_controls.push_back(ControlBinding("dashes", _dashes));
		_controls.push_back(ControlBinding("spaces", _spaces));
		_controls.push_back(ControlBinding("monosize", _monosize));
		_controls.push_back(ControlBinding("propsize", _propsize));
		_controls.push_back(ControlBinding("morealign", _morealign));
		_controls.push_back(ControlBinding("morefont", _morefont));
	}
	for (uint i = 0; i < _controls.size(); ++i)
		rememberControl(_controls[i].widget);
	rememberControl(_tfontPopUp);
	for (uint i = 0; i < ARRAYSIZE(_colors); ++i) {
		rememberControl(_colors[i].popup);
		rememberControl(_colors[i].hex);
	}
}

bool GlkOptionsWidget::controlChanged(GUI::Widget *widget) const {
	return _editedControls.contains(widget->getName()) ||
		_loadedControls.getValOrDefault(widget->getName()) != controlValue(widget);
}

bool GlkOptionsWidget::synchronizeDraft(bool reportError, GUI::Widget *only) {
	const bool wasLoading = _loading;
	_loading = true;
	GlkOptionsState pending(*_settings);
	const bool valid = synchronizePending(pending, reportError, only);
	if (valid) {
		*_settings = pending;
		if (only)
			refreshControl(only);
		else
			load();
	}
	_loading = wasLoading;
	return valid;
}

bool GlkOptionsWidget::synchronizePending(GlkOptionsState &pending, bool reportError, GUI::Widget *only) {
	for (uint i = 0; i < _controls.size(); ++i) {
		const ControlBinding &control = _controls[i];
		if ((only && only != control.widget) || !controlChanged(control.widget))
			continue;
		const GlkOptionContract *contract = findGlkOptionContract(control.key);
		const Common::String text = controlValue(control.widget);
		int integer = 0;
		double number = 0;
		bool valid = true;
		switch (contract->type) {
		case kGlkOptionInteger:
		case kGlkOptionFont:
			valid = GlkOptionsState::parseInteger(text, contract->minimum, contract->maximum, integer);
			if (valid)
				pending.commitString(control.key, contract->type == kGlkOptionFont ?
					Screen::getFontName((FACES)integer) : Common::String::format("%d", integer));
			break;
		case kGlkOptionFloat:
			valid = GlkOptionsState::parseFloat(text, contract->minimum, contract->maximum, number);
			if (valid)
				pending.commitString(control.key, text);
			break;
		default:
			pending.commitString(control.key, text);
			break;
		}
		if (!valid)
			return invalidControl(control.widget, reportError);
	}
	if ((!only || only == _tfontPopUp) && controlChanged(_tfontPopUp)) {
		const int font = _tfontPopUp->getSelectedTag();
		if (font < MONOR || font > PROPZ)
			return invalidControl(_tfontPopUp, reportError);
		pending.commitStyleFont(false, style_Normal, (FACES)font);
	}
	for (uint i = 0; i < ARRAYSIZE(_colors); ++i) {
		const ColorBinding &binding = _colors[i];
		if (only && only != binding.hex && only != binding.popup)
			continue;
		const bool editedHex = controlChanged(binding.hex);
		if (!editedHex && !controlChanged(binding.popup))
			continue;
		Common::String color;
		bool useDefault = false;
		const bool valid = editedHex ? GlkOptionsState::parseColor(binding.hex->getEditString().encode(), color) :
			getColorDraft(binding.popup, binding.hex, binding.allowsDefault(), useDefault, color);
		if (!valid)
			return invalidControl(binding.hex, reportError);
		if (binding.isNormalForeground()) {
			pending.commitStyleColor(binding.target == kNormalGridForeground, style_Normal, true, color);
		} else if (useDefault) {
			pending.removePreference(binding.key);
			if (binding.legacyOverride)
				pending.removePreference(binding.legacyOverride);
		} else {
			pending.commitString(binding.key, color);
		}
	}
	return synchronizeStyleEditor(pending, reportError, only);
}

bool GlkOptionsWidget::validate() {
	return synchronizeDraft(true);
}

bool GlkOptionsWidget::save() {
	// Accepted preferences take effect at the next interpreter startup.
	return synchronizeDraft(true) && _settings->apply();
}

static void setManualColorHex(GUI::PopUpWidget *popup,
		GUI::EditTextWidget *hexInput, GlkColorSwatchWidget *swatch,
		const Common::String &defaultColor = Common::String()) {
	int idx = popup->getSelectedTag();

	if (idx == 7) {
		hexInput->setEditString(Common::U32String(defaultColor));
		swatch->setColor(defaultColor);
		return;
	}
	if (idx < 0 || idx > 5)
		return;

	byte r = (GLK_COLORS[idx].rgb >> 16) & 0xFF;
	byte g = (GLK_COLORS[idx].rgb >> 8)  & 0xFF;
	byte b = (GLK_COLORS[idx].rgb		& 0xFF);
	hexInput->setEditString(Common::U32String(Common::String::format("%02X%02X%02X", r, g, b)));
	swatch->setColor(hexInput->getEditString().encode());
}

static void setColorPopupFromHex(GUI::PopUpWidget *popup,
		GUI::EditTextWidget *hexInput, GlkColorSwatchWidget *swatch) {
	Common::String hex;
	if (!GlkOptionsState::parseColor(hexInput->getEditString().encode(), hex)) {
		swatch->setColor(Common::String());
		popup->setSelectedTag(6);
		return;
	}
	swatch->setColor(hex);

	unsigned int rgb = 0;
	sscanf(hex.c_str(), "%x", &rgb);

	bool found = false;
	for (int i = 0; i <= 5; ++i) {
		if (GLK_COLORS[i].rgb == rgb) {
			popup->setSelectedTag(i);
			found = true;
			break;
		}
	}

	if (!found)
		popup->setSelectedTag(6);

	popup->markAsDirty();
}

void GlkOptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (_loading)
		return;
	GUI::Widget *edited = dynamic_cast<GUI::Widget *>(sender);
	if (cmd == kGlkEditFinishedCmd) {
		// Focus alone is not an edit. Invalid text stays in place without
		// stealing focus back; acceptance will report it before any writes.
		if (edited && controlChanged(edited))
			synchronizeDraft(false, edited);
		return;
	}
	if (edited && (edited->getType() == GUI::kEditTextWidget ||
			edited->getType() == GUI::kCheckboxWidget || edited->getType() == GUI::kPopUpWidget) &&
			edited != _styleWindow && edited != _styleName)
		_editedControls.setVal(edited->getName(), "true");
	if (cmd == kOptionEditedCmd || cmd == kStyleEditedCmd) {
		if (edited && edited->getType() != GUI::kEditTextWidget)
			synchronizeDraft(false, edited);
		return;
	}
	for (uint i = 0; i < ARRAYSIZE(_colors); ++i) {
		const ColorBinding &color = _colors[i];
		if (cmd == color.popupCommand) {
			setManualColorHex(color.popup, color.hex, color.swatch,
				color.allowsDefault() ? _settings->inheritedValue(color.key) : Common::String());
			_loadedControls.setVal(color.hex->getName(), controlValue(color.hex));
			_editedControls.erase(color.hex->getName());
			synchronizeDraft(false, color.popup);
			return;
		}
		if (cmd == color.hexCommand) {
			setColorPopupFromHex(color.popup, color.hex, color.swatch);
			return;
		}
	}
	if (cmd == kStyleForegroundHexCmd) {
		_styleForegroundSwatch->setColor(
			_styleForeground->getEditString().encode());
		return;
	} else if (cmd == kStyleBackgroundHexCmd) {
		_styleBackgroundSwatch->setColor(
			_styleBackground->getEditString().encode());
		return;
	} else if (cmd == kLayoutDependencyCmd) {
		synchronizeDraft(false, edited);
		updateLayoutDependencies();
		return;
	} else if (cmd == kPreviewSettingsCmd) {
		updatePreviewAvailability();
		return;
	} else if (cmd == kPreviewCmd) {
		if (_hostContext != kLauncherOptions || !_preview->isEnabled())
			return;
		if (!synchronizeDraft(true))
			return;
		showGlkPreview(*_settings);
		return;
	} else if (cmd == kStyleSelectionCmd) {
		GlkOptionsState pending(*_settings);
		const bool wasLoading = _loading;
		_loading = true;
		const bool valid = synchronizeStyleEditor(pending, true);
		_loading = wasLoading;
		if (!valid) {
			_styleWindow->setSelectedTag(_loadedStyleGrid ? 1 : 0);
			_styleName->setSelectedTag(_loadedStyle);
			return;
		}
		*_settings = pending;
		_loading = true;
		loadStyleEditor();
		_loading = wasLoading;
		return;
	} else if (cmd == kResetStyleFontCmd ||
			cmd == kResetStyleForegroundCmd ||
			cmd == kResetStyleBackgroundCmd) {
		StyleProperty property = kStyleFont;
		if (cmd == kResetStyleForegroundCmd)
			property = kStyleForeground;
		else if (cmd == kResetStyleBackgroundCmd)
			property = kStyleBackground;
		_settings->resetStyleProperty(_loadedStyleGrid, _loadedStyle,
			property);
		_loading = true;
		refreshControl(property == kStyleFont ? static_cast<GUI::Widget *>(_styleFont) :
			(property == kStyleForeground ? _styleForeground : _styleBackground));
		_loading = false;
		return;
	} else if (cmd == kSaveGlobalCmd) {
		if (_hostContext != kLauncherOptions || !synchronizeDraft(true))
			return;
		Common::String invalid;
		if (!_settings->stageGlobalPreferences(invalid)) {
			GUI::MessageDialog message(Common::U32String::format(
				_("Cannot save global defaults: the stored value for '%s' is invalid. Edit or reset it first."),
				invalid.c_str()));
			message.runModal();
			return;
		}
		load();
		if (_parentDialog)
			_parentDialog->reflowLayout();
		return;
	} else if (cmd == kRestoreDefaultsCmd || cmd == kInterpreterDefaultsCmd) {
		_settings->resetAllPreferences(cmd == kInterpreterDefaultsCmd);
		load();
		if (_parentDialog)
			_parentDialog->reflowLayout();
		return;
	}

	OptionsContainerWidget::handleCommand(sender, cmd, data);
}

} // End of namespace Glk
