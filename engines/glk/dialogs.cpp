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
#include "gui/ThemeEngine.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"
#include "gui/widgets/scrollcontainer.h"
#include "gui/widgets/tab.h"

#include "glk/conf.h"
#include "glk/fonts.h"
#include "glk/options.h"
#include "glk/preview.h"
#include "glk/screen.h"
#include "glk/events.h"
#include "glk/window_text_buffer.h"
#include "glk/window_text_grid.h"

#include "common/config-manager.h"
#include "common/translation.h"

namespace Glk {

static const uint32 kGlkEditFinishedCmd = 'GEFN';

enum {
	kCustomColor = 6,
	kDefaultColor = 7
};

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

class GlkOptionsLabelWidget : public GUI::StaticTextWidget {
public:
	GlkOptionsLabelWidget(GUI::GuiObject *boss, const Common::String &name,
			const Common::U32String &text, const Common::U32String &tooltip,
			GUI::ThemeEngine::FontStyle font)
		: GUI::StaticTextWidget(boss, name, text, tooltip, font, Common::UNK_LANG, false) {
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
		sample.fillRect(Common::Rect(0, 0, sample.w, sample.h), sample.format.RGBToColor(frame, frame, frame));
		if (_valid)
			sample.fillRect(Common::Rect(1, 1, sample.w - 1, sample.h - 1),
				sample.format.RGBToColor((_rgb >> 16) & 0xff, (_rgb >> 8) & 0xff, _rgb & 0xff));
		g_gui.theme()->drawManagedSurface(Common::Point(_x, _y), sample, Graphics::ALPHA_OPAQUE);
	}
};

class GlkOptionsTabWidget : public GUI::TabWidget {
	GlkOptionsWidget &_owner;
public:
	GlkOptionsTabWidget(GlkOptionsWidget &owner, GUI::GuiObject *boss, const Common::String &name)
		: GUI::TabWidget(boss, name), _owner(owner) {
	}

	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override {
		handleEmbeddedCommand(sender, cmd, data);
	}

	GUI::Widget *getActivePageWidgets() const {
		return _firstWidget;
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

static Common::String controlValue(GUI::Widget *widget) {
	GUI::EditTextWidget *edit = dynamic_cast<GUI::EditTextWidget *>(widget);
	GUI::PopUpWidget *popup = dynamic_cast<GUI::PopUpWidget *>(widget);
	GUI::CheckboxWidget *checkbox = dynamic_cast<GUI::CheckboxWidget *>(widget);
	if (edit)
		return edit->getEditString().encode();
	if (popup)
		return Common::String::format("%d", (int)popup->getSelectedTag());
	if (checkbox)
		return checkbox->getState() ? "true" : "false";
	return Common::String();
}

static void setIntegerWidget(GUI::EditTextWidget *widget, int value) {
	widget->setEditString(Common::U32String(Common::String::format("%d", value)));
}

static int findColorPreset(uint rgb) {
	for (uint i = 0; i < ARRAYSIZE(GLK_COLORS); ++i)
		if (GLK_COLORS[i].rgb == rgb)
			return i;
	return kCustomColor;
}

static void setColorControls(GUI::PopUpWidget *popup,
		GUI::EditTextWidget *hexInput, GlkColorSwatchWidget *swatch,
		const Common::String &color, bool overrideEnabled) {
	swatch->setColor(color);
	hexInput->setEditString(Common::U32String(color));
	if (!overrideEnabled) {
		popup->setSelectedTag(kDefaultColor);
		return;
	}
	unsigned int rgb = 0;
	sscanf(color.c_str(), "%x", &rgb);
	popup->setSelectedTag(findColorPreset(rgb));
}

static void setColorFromPreset(GUI::PopUpWidget *popup,
		GUI::EditTextWidget *hexInput, GlkColorSwatchWidget *swatch,
		const Common::String &defaultColor) {
	const uint32 index = popup->getSelectedTag();

	if (index == kDefaultColor) {
		hexInput->setEditString(Common::U32String(defaultColor));
		swatch->setColor(defaultColor);
		return;
	}
	if (index >= ARRAYSIZE(GLK_COLORS))
		return;

	hexInput->setEditString(Common::U32String(Common::String::format("%06X", GLK_COLORS[index].rgb)));
	swatch->setColor(hexInput->getEditString().encode());
}

static void setColorPresetFromHex(GUI::PopUpWidget *popup,
		GUI::EditTextWidget *hexInput, GlkColorSwatchWidget *swatch) {
	Common::String hex;
	if (!GlkOptionsState::parseColor(hexInput->getEditString().encode(), hex)) {
		swatch->setColor(Common::String());
		popup->setSelectedTag(kCustomColor);
		return;
	}
	swatch->setColor(hex);

	unsigned int rgb = 0;
	sscanf(hex.c_str(), "%x", &rgb);

	popup->setSelectedTag(findColorPreset(rgb));

	popup->markAsDirty();
}

static bool getColorDraft(GUI::PopUpWidget *popup,
		GUI::EditTextWidget *hexInput, bool allowDefault, bool &useDefault,
		Common::String &color) {
	const int index = popup->getSelectedTag();
	useDefault = allowDefault && index == kDefaultColor;
	if (useDefault)
		return true;

	if (index >= 0 && index < (int)ARRAYSIZE(GLK_COLORS)) {
		color = Common::String::format("%06x", GLK_COLORS[index].rgb);
		return true;
	}

	if (index != kCustomColor || !hexInput)
		return false;
	return GlkOptionsState::parseColor(hexInput->getEditString().encode(), color);
}

GlkOptionsWidget::GlkOptionsWidget(GuiObject *boss, const Common::String &name,
		const Common::String &domain, InterpreterType interpreterType)
	: GUI::OptionsContainerWidget(boss, name, "GlkOptionsDialog", domain),
	  _settings(new GlkOptionsState(interpreterType, domain)),
	  _conf(nullptr),
	  _tabs(nullptr), _preview(nullptr),
	  _restoreDefaults(nullptr), _interpreterDefaults(nullptr), _saveGlobal(nullptr),
	  _hostContext(kLauncherOptions), _loading(true), _previewFontsAvailable(false),
	  _styleWindow(nullptr), _styleName(nullptr),
	  _styleFont(nullptr), _styleForeground(nullptr), _styleBackground(nullptr),
	  _styleForegroundSwatch(nullptr), _styleBackgroundSwatch(nullptr),
	  _resetStyleFont(nullptr), _resetStyleForeground(nullptr),
	  _resetStyleBackground(nullptr), _loadedStyleGrid(false),
	  _loadedStyle(style_Normal) {
	setContentSized(true);

	_tabs = new GlkOptionsTabWidget(*this, widgetsBoss(), "GlkOptionsDialog.TabWidget");
	const int readingTab = _tabs->addTab(_("Reading"), "GlkOptionsReading");
	_tfontPopUp = createPopup("GlkOptionsReading.TFont", _("Normal prose font:"),
		"GlkOptionsReading.TFont0", _("Font of the Normal prose style; other styles are unchanged"));
	for (int font = MONOR; font <= PROPZ; ++font)
		_tfontPopUp->appendEntry(_(fontFaceLabels[font]), font);
	createOptionLabel(_tabs, "GlkOptionsReading.GridExplanation",
		_("Fixed-grid windows require fixed-width fonts. Adjust individual text styles on the Styles page."),
		_("Prose windows can also contain fixed-width text."));
	// I18N: Add space between words so wrapped lines reach both margins.
	_justify = createCheckbox("GlkOptionsReading.justifylbl", _("Justify text:"),
		"GlkOptionsReading.justify", _("Enable text justification"));
#ifdef USE_TTS
	_speak = createCheckbox("GlkOptionsReading.speaklbl", _("Enable Text to Speech"),
		"GlkOptionsReading.speak", _("Use TTS to read the text"));
	_speakInput = createCheckbox("GlkOptionsReading.speakinputlbl", _("Also read input text"),
		"GlkOptionsReading.speak_input", _("Use TTS to read the input text"));
#endif
	// I18N: Point size of the font whose characters all have the same width.
	_monosize = createEdit("GlkOptionsReading.monosizelbl", _("Fixed-width font size:"),
		"GlkOptionsReading.monosize", _("Fixed-width font size in points (10-25)"));
	// I18N: Point size of the font whose character widths can differ.
	_propsize = createEdit("GlkOptionsReading.propsizelbl", _("Proportional font size:"),
		"GlkOptionsReading.propsize", _("Proportional font size in points (10-25)"));

	_tabs->addTab(_("Compatibility"), "GlkOptionsCompatibility");
	// I18N: Minimum distance between successive text baselines, in pixels.
	_leading = createEdit("GlkOptionsCompatibility.leadinglbl", _("Minimum line height:"),
		"GlkOptionsCompatibility.leading", _("Compatibility override for the minimum distance between text baselines, in pixels."));
	// I18N: Minimum distance from the top of a proportional-text line to its
	// baseline (the line on which letters sit), in pixels.
	_baseline = createEdit("GlkOptionsCompatibility.baselinelbl", _("Text baseline:"),
		"GlkOptionsCompatibility.baseline", _("Compatibility override for the minimum distance from the top of a prose line to its baseline, in pixels."));

	_tabs->addTab(_("Colors"), "GlkOptionsColors");
	createColorBinding(kNormalProseForeground, "T", nullptr, nullptr,
		_("Normal prose text:"), _("Foreground of the Normal prose style"));

	createColorBinding(kNormalGridForeground, "G", nullptr, nullptr,
		_("Normal fixed-grid text:"), _("Foreground of the Normal fixed-grid style"));

	createColorBinding(kWindowBackground, "W", "windowcolor", "windowcolor_override",
		_("Window background:"), _("Background color of text windows"));

	createColorBinding(kDividerColor, "B", "bordercolor", "bordercolor_override",
		_("Divider:"), _("Color of dividers between panes"));
	createColorBinding(kCaretColor, "C", "caretcolor", nullptr,
		_("Insertion cursor:"), _("Color of the insertion cursor"));

	createColorBinding(kLinkColor, "L", "linkcolor", nullptr,
		_("Clickable text:"), _("Color of clickable text"));

	createColorBinding(kMoreColor, "M", "morecolor", nullptr,
		// I18N: Color of the paging prompt shown when more text is waiting to be read.
		_("Paging prompt:"), _("Color of the paging prompt"));

	_tabs->addTab(_("Styles"), "GlkOptionsStyles");
	createOptionLabel(_tabs, "GlkOptionsStyles.StyleWindowLabel", _("Window type:"), _("Choose prose or fixed-grid windows, such as status windows."));
	_styleWindow = new GlkOptionsPopUpWidget(_tabs,
		"GlkOptionsStyles.StyleWindow",
		_("Choose prose or fixed-grid windows, such as status windows."), kStyleSelectionCmd);
	_styleWindow->appendEntry(_("Prose text"), 0);
	_styleWindow->appendEntry(_("Fixed-grid text"), 1);
	createOptionLabel(_tabs, "GlkOptionsStyles.StyleNameLabel", _("Style:"), _("GLK text style"));
	_styleName = new GlkOptionsPopUpWidget(_tabs, "GlkOptionsStyles.StyleName", _("GLK text style"), kStyleSelectionCmd);
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
	_styleFont = createPopup("GlkOptionsStyles.StyleFontLabel", _("Font:"),
		"GlkOptionsStyles.StyleFont", _("Font for this style"));
	for (int font = MONOR; font <= PROPZ; ++font)
		_styleFont->appendEntry(_(fontFaceLabels[font]), font);
	_resetStyleFont = new GlkOptionsButtonWidget(_tabs,
		"GlkOptionsStyles.ResetStyleFont", _("Use interpreter default"),
		_("Remove only this font override"), kResetStyleFontCmd);
	_styleForeground = createEdit("GlkOptionsStyles.StyleForegroundLabel", _("Text color:"),
		"GlkOptionsStyles.StyleForeground", _("Color for this style as six hexadecimal digits (RRGGBB)."));
	createOptionLabel(_tabs, "GlkOptionsStyles.StyleForegroundPrefix", Common::U32String("#"), Common::U32String(),
		GUI::ThemeEngine::kFontStyleNormal);
	_styleForegroundSwatch = new GlkColorSwatchWidget(_tabs,
		"GlkOptionsStyles.StyleForegroundSwatch",
		_("Selected text color for this style"));
	_resetStyleForeground = new GlkOptionsButtonWidget(_tabs,
		"GlkOptionsStyles.ResetStyleForeground", _("Use interpreter default"),
		_("Remove only this text-color override"), kResetStyleForegroundCmd);
	_styleBackground = createEdit("GlkOptionsStyles.StyleBackgroundLabel", _("Background color:"),
		"GlkOptionsStyles.StyleBackground", _("Color for this style as six hexadecimal digits (RRGGBB)."));
	createOptionLabel(_tabs, "GlkOptionsStyles.StyleBackgroundPrefix", Common::U32String("#"), Common::U32String(),
		GUI::ThemeEngine::kFontStyleNormal);
	_styleBackgroundSwatch = new GlkColorSwatchWidget(_tabs,
		"GlkOptionsStyles.StyleBackgroundSwatch",
		_("Selected background color for this style"));
	_resetStyleBackground = new GlkOptionsButtonWidget(_tabs,
		"GlkOptionsStyles.ResetStyleBackground", _("Use interpreter default"),
		_("Remove only this background-color override"), kResetStyleBackgroundCmd);

	_tabs->addTab(_("Layout"), "GlkOptionsLayout");
	_wborderx = createEdit("GlkOptionsLayout.wborderx", _("Vertical divider thickness:"),
		"GlkOptionsLayout.wborderhorizontal", _("Width of vertical dividers between panes, in pixels."));
	_wbordery = createEdit("GlkOptionsLayout.wbordery", _("Horizontal divider thickness:"),
		"GlkOptionsLayout.wbordervertical", _("Height of horizontal dividers between panes, in pixels."));
	// I18N: Maximum content width, measured in fixed-width character cells.
	_cols = createEdit("GlkOptionsLayout.colslbl", _("Content width (columns):"),
		"GlkOptionsLayout.cols", _("Maximum text width measured using fixed-width cells"));
	// I18N: Maximum content height, measured in fixed-width text rows.
	_rows = createEdit("GlkOptionsLayout.rowslbl", _("Content height (rows):"),
		"GlkOptionsLayout.rows", _("Maximum text height measured in rows"));

	_lockcols = createCheckbox("GlkOptionsLayout.lockcolslbl", _("Limit content width:"),
		"GlkOptionsLayout.lockcols", _("Enable the content-width limit"));
	_lockrows = createCheckbox("GlkOptionsLayout.lockrowslbl", _("Limit content height:"),
		"GlkOptionsLayout.lockrows", _("Enable the content-height limit"));
	_wmarginx = createEdit("GlkOptionsLayout.wmarginxlbl", _("Outer margin, horizontal:"),
		"GlkOptionsLayout.wmarginx", _("Space between the display edge and GLK windows"));
	_wmarginy = createEdit("GlkOptionsLayout.wmarginylbl", _("Outer margin, vertical:"),
		"GlkOptionsLayout.wmarginy", _("Space between the display edge and GLK windows"));
	_wpaddingx = createEdit("GlkOptionsLayout.wpaddingxlbl", _("Pane spacing, horizontal:"),
		"GlkOptionsLayout.wpaddingx", _("Space between adjacent GLK panes"));
	_wpaddingy = createEdit("GlkOptionsLayout.wpaddingylbl", _("Pane spacing, vertical:"),
		"GlkOptionsLayout.wpaddingy", _("Space between adjacent GLK panes"));
	_tmarginx = createEdit("GlkOptionsLayout.tmarginxlbl", _("Prose text margin, horizontal:"),
		"GlkOptionsLayout.tmarginx", _("Horizontal space around text inside prose windows, in pixels."));
	_tmarginy = createEdit("GlkOptionsLayout.tmarginylbl", _("Prose text margin, vertical:"),
		"GlkOptionsLayout.tmarginy", _("Vertical space around text inside prose windows, in pixels."));

	_tabs->addTab(_("Interaction"), "GlkOptionsInteraction");
	_linkStyle = createPopup("GlkOptionsInteraction.linkstyle", _("Clickable-text underline:"),
		"GlkOptionsInteraction.linkStyle", _("Underline used for clickable text"));
	_linkStyle->appendEntry(_("No underline"), 0);
	_linkStyle->appendEntry(_("Thin underline"), 1);
	_linkStyle->appendEntry(_("Thick underline"), 2);

	_caretShape = createPopup("GlkOptionsInteraction.caretshape", _("Cursor shape:"),
		"GlkOptionsInteraction.caretShape", _("Shape of the insertion cursor"));
	_caretShape->appendEntry(_("Small dot"), 0);
	_caretShape->appendEntry(_("Large dot"), 1);
	_caretShape->appendEntry(_("Thin line"), 2);
	_caretShape->appendEntry(_("Thick line"), 3);
	_caretShape->appendEntry(_("Block"), 4);
	_styleHint = createCheckbox("GlkOptionsInteraction.stylehintlabel", _("Game formatting (restart required):"),
		"GlkOptionsInteraction.stylehint", _("Allow the game to request formatting changes, including changes to your selected appearance."));
	_safeClicks = createCheckbox("GlkOptionsInteraction.safeclickslabel", _("Mouse input compatibility:"),
		"GlkOptionsInteraction.safeclicks", _("Allow game output or window clearing after a mouse or link click to cancel unfinished line input."));
	// I18N: Convert typed letters a-z to capitals A-Z.
	_caps = createCheckbox("GlkOptionsInteraction.capslbl", _("Uppercase input:"),
		"GlkOptionsInteraction.caps", _("Convert typed a-z to A-Z in prose line input. Other characters are unchanged."));
	_graphics = createCheckbox("GlkOptionsInteraction.graphicslbl", _("Game pictures (restart required):"),
		"GlkOptionsInteraction.graphics", _("Show pictures supplied by the game"));

	_tabs->addTab(_("Paging & Text"), "GlkOptionsPaging");
	_morePrompt = createEdit("GlkOptionsPaging.moreprompt", _("Paging prompt:"),
		"GlkOptionsPaging.morePrompt", _("Text shown when more output is waiting. Leave blank to show no prompt text; paging remains active."));
	static_cast<GlkOptionsEditTextWidget *>(_morePrompt)->setLongText();
	// I18N: Replace straight quotation marks in game output with curly ones.
	_quotes = createPopup("GlkOptionsPaging.quoteslbl", _("Typographic quotes:"),
		"GlkOptionsPaging.quotes", _("Replace straight quotes in prose with curly quotes. Aggressive mode also guesses opening single quotes."));
	_quotes->appendEntry(_c("Off", "quotes"), 0);
	// I18N: Use curly double quotes and apostrophes. Backticks become opening
	// single quotes.
	_quotes->appendEntry(_c("Normal", "quotes"), 1);
	// I18N: "Aggressive" works like Normal, but also guesses opening single
	// quotes. This can misread apostrophes in 'tis or '80s.
	_quotes->appendEntry(_c("Aggressive", "quotes"), 2);
	// I18N: Convert runs of hyphens to typographic dashes, except in preformatted text.
	_dashes = createPopup("GlkOptionsPaging.dasheslbl", _("Dashes:"),
		"GlkOptionsPaging.dashes", _("Replace two or three hyphens with typographic dashes, except in preformatted text."));
	_dashes->appendEntry(_("Off"), 0);
	// I18N: Replace two or three hyphens (-- or ---) with an em dash (U+2014).
	_dashes->appendEntry(_("Em dashes"), 1);
	// I18N: Replace -- with an en dash (U+2013), and --- with an em dash (U+2014).
	_dashes->appendEntry(_("En and em dashes"), 2);

	// I18N: Adjust spaces after periods, not spacing between all words.
	_spaces = createPopup("GlkOptionsPaging.spaceslbl", _("Sentence spacing:"),
		"GlkOptionsPaging.spaces", _("Adjust spaces after periods in prose: compress two spaces to one, or expand one to two."));
	_spaces->appendEntry(_("Off"), 0);
	// I18N: Replace two spaces after a period with one space.
	_spaces->appendEntry(_("Compress double spaces"), 1);
	// I18N: Replace a single space after a period with two spaces.
	_spaces->appendEntry(_("Expand single spaces"), 2);
	// I18N: Horizontal position of the paging prompt, such as "More...",
	// within the text window.
	_morealign = createPopup("GlkOptionsPaging.morealignlbl", _("Paging prompt alignment:"),
		"GlkOptionsPaging.morealign", _("Horizontal position of the paging prompt, such as \"More...\", within the text window."));
	_morealign->appendEntry(_("Left"), 0);
	_morealign->appendEntry(_("Center"), 1);
	_morealign->appendEntry(_("Right"), 2);

	_morefont = createPopup("GlkOptionsPaging.morefontlbl", _("Paging prompt font:"),
		"GlkOptionsPaging.morefont", _("Font for the paging marker"));
	for (int f = MONOR; f <= PROPZ; ++f)
		_morefont->appendEntry(_(fontFaceLabels[f]), f);

	_tabs->setActiveTab(readingTab);
	_preview = new GlkOptionsButtonWidget(widgetsBoss(),
		"GlkOptionsDialog.Preview", _("Preview"),
		_("Preview the current draft without changing the running game"), kPreviewCmd);
	_restoreDefaults = new GlkOptionsButtonWidget(widgetsBoss(),
		"GlkOptionsDialog.RestoreDefaults", _("Use Global Defaults"),
		_("Remove this game's appearance overrides and use global preferences where available. Speech settings are unchanged."),
		kRestoreDefaultsCmd);
	_interpreterDefaults = new GlkOptionsButtonWidget(widgetsBoss(),
		"GlkOptionsDialog.InterpreterDefaults", _("Use Interpreter Defaults"),
		_("Remove this game's appearance overrides and ignore global appearance preferences. Speech settings are unchanged."),
		kInterpreterDefaultsCmd);
	_saveGlobal = new GlkOptionsButtonWidget(widgetsBoss(),
		"GlkOptionsDialog.SaveGlobal", _("Save Customizations as Global Defaults"),
		_("Use this game's appearance and speech customizations as global defaults when you choose OK. When using interpreter defaults, also clear global appearance settings that this game has not customized. Cancel discards them. Later edits affect only this game."),
		kSaveGlobalCmd);
	createOptionLabel(widgetsBoss(), "GlkOptionsDialog.RestartNotice",
		_("Appearance changes may apply the next time this game starts."), Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
	createOptionLabel(widgetsBoss(), "GlkOptionsDialog.PreferenceNotice",
		_("Per-game settings override global settings. If neither specifies a value, interpreter defaults apply.\nChoose OK to save your changes, or Cancel to discard them."), Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
	createOptionLabel(widgetsBoss(), "GlkOptionsDialog.ActionStatus", Common::U32String(), Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
	createOptionLabel(widgetsBoss(), "GlkOptionsDialog.PreviewStatus", Common::U32String(), Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
	findOptionLabel("GlkOptionsDialog.PreviewStatus")->setVisible(false);

	_controls.push_back(ControlBinding("wmarginx", _wmarginx, "GlkOptionsLayout.wmarginxlbl"));
	_controls.push_back(ControlBinding("wmarginy", _wmarginy, "GlkOptionsLayout.wmarginylbl"));
	_controls.push_back(ControlBinding("wpaddingx", _wpaddingx, "GlkOptionsLayout.wpaddingxlbl"));
	_controls.push_back(ControlBinding("wpaddingy", _wpaddingy, "GlkOptionsLayout.wpaddingylbl"));
	_controls.push_back(ControlBinding("wborderx", _wborderx, "GlkOptionsLayout.wborderx"));
	_controls.push_back(ControlBinding("wbordery", _wbordery, "GlkOptionsLayout.wbordery"));
	_controls.push_back(ControlBinding("tmarginx", _tmarginx, "GlkOptionsLayout.tmarginxlbl"));
	_controls.push_back(ControlBinding("tmarginy", _tmarginy, "GlkOptionsLayout.tmarginylbl"));
	_controls.push_back(ControlBinding("leading", _leading, "GlkOptionsCompatibility.leadinglbl"));
	_controls.push_back(ControlBinding("baseline", _baseline, "GlkOptionsCompatibility.baselinelbl"));
	_controls.push_back(ControlBinding("cols", _cols, "GlkOptionsLayout.colslbl"));
	_controls.push_back(ControlBinding("rows", _rows, "GlkOptionsLayout.rowslbl"));
	_controls.push_back(ControlBinding("safeclicks", _safeClicks, "GlkOptionsInteraction.safeclickslabel"));
	_controls.push_back(ControlBinding("stylehint", _styleHint, "GlkOptionsInteraction.stylehintlabel"));
	_controls.push_back(ControlBinding("lockcols", _lockcols, "GlkOptionsLayout.lockcolslbl"));
	_controls.push_back(ControlBinding("lockrows", _lockrows, "GlkOptionsLayout.lockrowslbl"));
	_controls.push_back(ControlBinding("justify", _justify, "GlkOptionsReading.justifylbl"));
#ifdef USE_TTS
	_controls.push_back(ControlBinding("speak", _speak, "GlkOptionsReading.speaklbl"));
	_controls.push_back(ControlBinding("speak_input", _speakInput, "GlkOptionsReading.speakinputlbl"));
#endif
	_controls.push_back(ControlBinding("caps", _caps, "GlkOptionsInteraction.capslbl"));
	_controls.push_back(ControlBinding("graphics", _graphics, "GlkOptionsInteraction.graphicslbl"));
	_controls.push_back(ControlBinding("linkstyle", _linkStyle, "GlkOptionsInteraction.linkstyle"));
	_controls.push_back(ControlBinding("caretshape", _caretShape, "GlkOptionsInteraction.caretshape"));
	_controls.push_back(ControlBinding("moreprompt", _morePrompt, "GlkOptionsPaging.moreprompt"));
	_controls.push_back(ControlBinding("quotes", _quotes, "GlkOptionsPaging.quoteslbl"));
	_controls.push_back(ControlBinding("dashes", _dashes, "GlkOptionsPaging.dasheslbl"));
	_controls.push_back(ControlBinding("spaces", _spaces, "GlkOptionsPaging.spaceslbl"));
	_controls.push_back(ControlBinding("monosize", _monosize, "GlkOptionsReading.monosizelbl"));
	_controls.push_back(ControlBinding("propsize", _propsize, "GlkOptionsReading.propsizelbl"));
	_controls.push_back(ControlBinding("morealign", _morealign, "GlkOptionsPaging.morealignlbl"));
	_controls.push_back(ControlBinding("morefont", _morefont, "GlkOptionsPaging.morefontlbl"));
}

GlkOptionsWidget::~GlkOptionsWidget() {
	delete _settings;
}

GUI::EditTextWidget *GlkOptionsWidget::createEdit(const char *labelName,
		const Common::U32String &label, const char *name, const Common::U32String &tooltip) {
	createOptionLabel(_tabs, labelName, label, tooltip);
	return new GlkOptionsEditTextWidget(_tabs, name, Common::U32String(), tooltip, kOptionEditedCmd);
}

GUI::CheckboxWidget *GlkOptionsWidget::createCheckbox(const char *labelName,
		const Common::U32String &label, const char *name, const Common::U32String &tooltip) {
	createOptionLabel(_tabs, labelName, label, tooltip);
	return new GlkOptionsCheckboxWidget(_tabs, name, Common::U32String(), tooltip, kOptionEditedCmd);
}

GUI::PopUpWidget *GlkOptionsWidget::createPopup(const char *labelName,
		const Common::U32String &label, const char *name, const Common::U32String &tooltip) {
	createOptionLabel(_tabs, labelName, label, tooltip);
	return new GlkOptionsPopUpWidget(_tabs, name, tooltip, kOptionEditedCmd);
}

GlkOptionsLabelWidget *GlkOptionsWidget::createOptionLabel(
		GUI::GuiObject *boss, const Common::String &name, const Common::U32String &text,
		const Common::U32String &tooltip, GUI::ThemeEngine::FontStyle font) {
	GlkOptionsLabelWidget *label = new GlkOptionsLabelWidget(boss, name, text, tooltip, font);
	_optionLabels.push_back(label);
	return label;
}

void GlkOptionsWidget::createColorBinding(ColorTarget target, const char *prefix,
		const char *key, const char *legacyOverride, const Common::U32String &label,
		const Common::U32String &tooltip) {
	ColorBinding &color = _colors[target];
	color.target = target;
	color.prefix = prefix;
	color.key = key;
	color.legacyOverride = legacyOverride;
	const Common::String name = Common::String("GlkOptionsColors.") + prefix;
	color.hex = new GlkOptionsEditTextWidget(_tabs, name + "Hex", Common::U32String(), tooltip, kOptionEditedCmd);
	createOptionLabel(_tabs, name + "Prefix", Common::U32String("#"), Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
	color.popup = new GlkOptionsPopUpWidget(_tabs, name + "Color0", tooltip, kOptionEditedCmd);
	color.swatch = new GlkColorSwatchWidget(_tabs, name + "Swatch", tooltip);
	createOptionLabel(_tabs, name + "Color", label, tooltip);
	if (color.allowsDefault())
		color.popup->appendEntry(_("<default>"), kDefaultColor);
	for (uint i = 0; i < ARRAYSIZE(GLK_COLORS); ++i)
		color.popup->appendEntry(_(GLK_COLORS[i].name), i);
	color.popup->appendEntry(_("<custom>"), kCustomColor);
}

GlkOptionsLabelWidget *GlkOptionsWidget::findOptionLabel(
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

void GlkOptionsWidget::setDomain(const Common::String &domain) {
	if (_domain == domain)
		return;

	GUI::OptionsContainerWidget::setDomain(domain);
	_settings->setDomain(domain);
}

void GlkOptionsWidget::load() {
	_conf = &_settings->resolved();
	checkPreviewFonts();
	loadControls();
}

void GlkOptionsWidget::loadControls() {
	_loading = true;
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
#ifdef USE_TTS
	_speak->setState(_settings->getBool("speak", false));
	_speakInput->setState(_settings->getBool("speak_input", false));
#endif
	_caps->setState(_settings->getBool("caps", _conf->_propInfo._caps != 0));
	_graphics->setState(_settings->getBool("graphics", _conf->_graphics));

	_linkStyle->setSelectedTag(_settings->getInt("linkstyle", _conf->_propInfo._linkStyle));
	_caretShape->setSelectedTag(_settings->getInt("caretshape", _conf->_propInfo._caretShape));
	const Common::U32String morePrompt = Common::U32String(_settings->getString("moreprompt", _conf->_propInfo._morePrompt));
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
	setColorControls(color.popup, color.hex, color.swatch, value, explicitValue);
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
	GUI::Widget *controls[] = { _styleFont, _styleForeground, _styleBackground };
	GUI::ButtonWidget *buttons[] = { _resetStyleFont, _resetStyleForeground, _resetStyleBackground };
	const StyleProperty properties[] = { kStyleFont, kStyleForeground, kStyleBackground };
	for (uint i = 0; i < ARRAYSIZE(properties); ++i) {
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

void GlkOptionsWidget::rememberControl(GUI::Widget *widget) {
	widget->setInvalidMark(false);
	_loadedControls.setVal(widget->getName(), controlValue(widget));
	_editedControls.erase(widget->getName());
}

void GlkOptionsWidget::rememberControls() {
	_editedControls.clear();
	for (uint i = 0; i < _controls.size(); ++i)
		rememberControl(_controls[i].widget);
	rememberControl(_tfontPopUp);
	for (uint i = 0; i < ARRAYSIZE(_colors); ++i) {
		rememberControl(_colors[i].popup);
		rememberControl(_colors[i].hex);
	}
}

void GlkOptionsWidget::refreshControl(GUI::Widget *widget) {
	const bool wasLoading = _loading;
	_loading = true;
	refreshControlValue(widget);
	updateLayoutDependencies();
	updatePreviewAvailability();
	_loading = wasLoading;
}

void GlkOptionsWidget::refreshControlValue(GUI::Widget *widget) {
	GUI::Widget *styleControl = nullptr;
	if (widget == _styleFont || widget == _styleForeground || widget == _styleBackground) {
		styleControl = widget;
		if (_loadedStyle == style_Normal && widget == _styleFont && !_loadedStyleGrid)
			widget = _tfontPopUp;
		else if (_loadedStyle == style_Normal && widget == _styleForeground)
			widget = _colors[_loadedStyleGrid ? kNormalGridForeground : kNormalProseForeground].popup;
	}
	if (widget == _tfontPopUp) {
		_tfontPopUp->setSelectedTag(_settings->getStyleFont(false, style_Normal));
		if (!_loadedStyleGrid && _loadedStyle == style_Normal)
			styleControl = _styleFont;
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
			styleControl = _styleForeground;
	}
	if (styleControl)
		loadStyleEditor(styleControl);
	if (widget != styleControl)
		rememberControl(widget);
}

void GlkOptionsWidget::updateLayoutDependencies() {
	_cols->setEnabled(_lockcols->getState());
	_rows->setEnabled(_lockrows->getState());
}

void GlkOptionsWidget::updateActionState() {
	GUI::StaticTextWidget *status = findOptionLabel("GlkOptionsDialog.ActionStatus");
	status->setLabel(_settings->hasGlobalChanges() ?
		_("Choose OK to save the new global defaults, or Cancel to discard them.") : Common::U32String());
}

void GlkOptionsWidget::checkPreviewFonts() {
	if (_hostContext == kLauncherOptions)
		_previewFontsAvailable = glkPreviewFontsAvailable(*_settings, *_conf);
}

void GlkOptionsWidget::updatePreviewAvailability(bool reflow) {
	if (_hostContext != kLauncherOptions) {
		_preview->setEnabled(false);
		return;
	}
	GlkPreviewLayout layout;
	Common::U32String reason;
	if (!layout.measure())
		reason = _("Preview is unavailable with these display settings.");
	else if (!_previewFontsAvailable)
		reason = _("Preview is unavailable with these fonts or font sizes.");
	_preview->setEnabled(reason.empty());
	GUI::StaticTextWidget *status = findOptionLabel("GlkOptionsDialog.PreviewStatus");
	status->setVisible(!reason.empty());
	if (status->getLabel() != reason) {
		status->setLabel(reason);
		if (reflow && _parentDialog)
			_parentDialog->reflowLayout();
	}
	_preview->setTooltip(reason.empty() ? _("Preview the current draft without changing the running game") : reason);
}

bool GlkOptionsWidget::controlChanged(GUI::Widget *widget) const {
	return _editedControls.contains(widget->getName()) ||
		_loadedControls.getValOrDefault(widget->getName()) != controlValue(widget);
}

bool GlkOptionsWidget::hasPendingControls(GUI::Widget *only) const {
	for (uint i = 0; i < _controls.size(); ++i)
		if ((!only || only == _controls[i].widget) && controlChanged(_controls[i].widget))
			return true;
	if ((!only || only == _tfontPopUp) && controlChanged(_tfontPopUp))
		return true;
	for (uint i = 0; i < ARRAYSIZE(_colors); ++i) {
		const ColorBinding &color = _colors[i];
		if ((!only || only == color.hex || only == color.popup) &&
				(controlChanged(color.hex) || controlChanged(color.popup)))
			return true;
	}
	return hasPendingStyle(only);
}

bool GlkOptionsWidget::hasPendingStyle(GUI::Widget *only) const {
	GUI::Widget *controls[] = { _styleFont, _styleForeground, _styleBackground };
	for (uint i = 0; i < ARRAYSIZE(controls); ++i)
		if ((!only || only == controls[i]) && controlChanged(controls[i]))
			return true;
	return false;
}

bool GlkOptionsWidget::synchronizeDraft(bool reportError, GUI::Widget *only, bool checkFonts) {
	if (!hasPendingControls(only))
		return true;
	const bool wasLoading = _loading;
	_loading = true;
	GlkOptionsState pending(*_settings);
	const bool valid = synchronizePending(pending, reportError, only);
	if (valid) {
		acceptPending(pending, checkFonts);
		if (only)
			refreshControl(only);
		else
			loadControls();
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
		// Speech checkboxes use the draft without joining appearance resets.
		switch (contract ? contract->type : kGlkOptionBoolean) {
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

void GlkOptionsWidget::acceptPending(GlkOptionsState &pending, bool checkFonts) {
	const Conf &resolved = pending.resolved();
	const bool fontsChanged = checkFonts && _hostContext == kLauncherOptions &&
		glkPreviewFontInputsChanged(*_settings, *_conf, pending, resolved);
	// Assignment copies the resolved result into the object already referenced
	// by _conf; no reference to the temporary candidate escapes this function.
	*_settings = pending;
	if (fontsChanged)
		checkPreviewFonts();
}

bool GlkOptionsWidget::invalidControl(GUI::Widget *widget, bool reportError) {
	widget->setInvalidMark(true);
	if (!reportError)
		return false;
	focusControl(widget);
	Common::U32String explanation;
	if (widget->getType() == GUI::kPopUpWidget) {
		explanation = _("Choose one of the listed options for the highlighted setting.");
	} else {
		const GlkOptionContract *contract = nullptr;
		for (uint i = 0; i < _controls.size(); ++i) {
			if (_controls[i].widget == widget) {
				contract = findGlkOptionContract(_controls[i].key);
				break;
			}
		}
		if (contract && contract->type == kGlkOptionFloat) {
			explanation = Common::U32String::format(
				_("Enter a font size from %d to %d points in the highlighted field. Decimal values are allowed; use a dot for the decimal point."),
				contract->minimum, contract->maximum);
		} else if (contract && contract->type == kGlkOptionInteger) {
			explanation = Common::U32String::format(
				_("Enter a whole number from %d to %d in the highlighted field."),
				contract->minimum, contract->maximum);
		} else {
			explanation = _("Enter six hexadecimal digits (0-9 and A-F) in the highlighted color field, for example FF8800.");
		}
	}
	GUI::MessageDialog dialog(explanation);
	dialog.runModal();
	return false;
}

void GlkOptionsWidget::showGlobalPreferenceError(const Common::String &key) {
	const char *const pageTitles[] = {
		_s("Reading"), _s("Compatibility"), _s("Colors"), _s("Styles"),
		_s("Layout"), _s("Interaction"), _s("Paging & Text")
	};
	const char *const styleNames[] = {
		_s("Normal"), _s("Emphasized"), _s("Preformatted"), _s("Heading"),
		_s("Subheading"), _s("Alert"), _s("Note"), _s("Block quote"),
		_s("Input"), _s("User style 1"), _s("User style 2")
	};
	GUI::Widget *control = nullptr;
	GUI::StaticTextWidget *label = nullptr;
	Common::U32String description;
	for (uint i = 0; i < _controls.size(); ++i) {
		if (_controls[i].key == key) {
			control = _controls[i].widget;
			if (_controls[i].label)
				label = findOptionLabel(_controls[i].label);
			break;
		}
	}
	for (uint i = 0; !control && i < ARRAYSIZE(_colors); ++i) {
		if (_colors[i].key && key == _colors[i].key) {
			control = _colors[i].hex;
			label = findOptionLabel(Common::String("GlkOptionsColors.") + _colors[i].prefix + "Color");
		}
	}
	for (int grid = 0; !control && grid < 2; ++grid) {
		for (int style = 0; !control && style < style_NUMSTYLES; ++style) {
			const Common::String font = Common::String::format("%cfont_%d", grid ? 'g' : 't', style);
			const Common::String color = Common::String::format("%ccolor_%d", grid ? 'g' : 't', style);
			const bool foreground = key == color + "_fg_override";
			const bool background = key == color + "_bg_override";
			if (key != font && key != color && !foreground && !background)
				continue;
			const bool wasLoading = _loading;
			_loading = true;
			_styleWindow->setSelectedTag(grid);
			_styleName->setSelectedTag(style);
			loadStyleEditor();
			_loading = wasLoading;
			control = key == font ? static_cast<GUI::Widget *>(_styleFont) :
				(background ? _styleBackground : _styleForeground);
			const Common::U32String property = key == font ? _("Font") :
				(foreground ? _("Text color") : (background ? _("Background color") : _("Text and background colors")));
			description = Common::U32String::format(_("%S: %S, %S, %S"),
				_("Styles").c_str(),
				(grid ? _("Fixed-grid text") : _("Prose text")).c_str(),
				_(styleNames[style]).c_str(), property.c_str());
		}
	}
	if (control) {
		focusControl(control);
		if (label) {
			Common::U32String name = label->getLabel();
			if (!name.empty() && name.lastChar() == ':')
				name.deleteLastChar();
			description = Common::U32String::format(_("%S: %S"),
				_(pageTitles[_tabs->getActiveTab()]).c_str(), name.c_str());
		}
	}
	assert(!description.empty());
	GUI::MessageDialog dialog(Common::U32String::format(
		_("The saved value for the following setting cannot be used as a global default:\n%S\nChange or reset this setting before trying again."), description.c_str()));
	dialog.runModal();
}

bool GlkOptionsWidget::validate() {
	return synchronizeDraft(true);
}

bool GlkOptionsWidget::save() {
	bool appearanceChanged = false;
	if (!synchronizeDraft(true) || !_settings->apply(&appearanceChanged))
		return false;

	GlkEngine *engine = _hostContext == kInGameOptions ? dynamic_cast<GlkEngine *>(g_engine) : nullptr;
	if (appearanceChanged && engine && engine->getTargetName() == _domain) {
		engine->_conf->reload();
		engine->_conf->_imageW = engine->_screen->w;
		engine->_conf->_imageH = engine->_screen->h;
		engine->_screen->initialize();
		for (Windows::iterator it = engine->_windows->begin(); it != engine->_windows->end(); ++it) {
			Window *window = *it;
			if (window->_type == wintype_TextBuffer)
				static_cast<TextBufferWindow *>(window)->refreshStyles();
			else if (window->_type == wintype_TextGrid)
				static_cast<TextGridWindow *>(window)->refreshStyles();
		}
		engine->_windows->rearrange();
		Windows::_forceRedraw = true;
		engine->_events->redraw();
	}
	return true;
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
		_editedControls.setVal(edited->getName(), true);
	switch (cmd) {
	case kOptionEditedCmd:
		for (uint i = 0; i < ARRAYSIZE(_colors); ++i) {
			const ColorBinding &color = _colors[i];
			if (edited == color.popup) {
				const Common::String inherited = color.allowsDefault() && color.popup->getSelectedTag() == kDefaultColor ?
					_settings->inheritedValue(color.key) : Common::String();
				setColorFromPreset(color.popup, color.hex, color.swatch, inherited);
				_loadedControls.setVal(color.hex->getName(), controlValue(color.hex));
				_editedControls.erase(color.hex->getName());
				synchronizeDraft(false, color.popup);
				return;
			}
			if (edited == color.hex) {
				setColorPresetFromHex(color.popup, color.hex, color.swatch);
				return;
			}
		}
		if (edited == _styleForeground)
			_styleForegroundSwatch->setColor(_styleForeground->getEditString().encode());
		else if (edited == _styleBackground)
			_styleBackgroundSwatch->setColor(_styleBackground->getEditString().encode());
		if (edited && edited->getType() != GUI::kEditTextWidget)
			synchronizeDraft(false, edited);
		return;
	case kPreviewCmd: {
		if (_hostContext != kLauncherOptions || !_preview->isEnabled())
			return;
		// Opening prepares the actual sample fonts, so do not probe them first.
		if (!synchronizeDraft(true, nullptr, false))
			return;
		GlkPreviewLayout layout;
		if (layout.measure())
			_previewFontsAvailable = showGlkPreview(*_settings, *_conf);
		updatePreviewAvailability();
		return;
	}
	case kStyleSelectionCmd: {
		const bool wasLoading = _loading;
		_loading = true;
		const bool styleEdited = hasPendingStyle();
		if (styleEdited) {
			GlkOptionsState pending(*_settings);
			if (!synchronizeStyleEditor(pending, true)) {
				_styleWindow->setSelectedTag(_loadedStyleGrid ? 1 : 0);
				_styleName->setSelectedTag(_loadedStyle);
				_loading = wasLoading;
				return;
			}
			acceptPending(pending);
		}
		loadStyleEditor();
		_loading = wasLoading;
		if (styleEdited)
			updatePreviewAvailability();
		return;
	}
	case kResetStyleFontCmd:
	case kResetStyleForegroundCmd:
	case kResetStyleBackgroundCmd: {
		StyleProperty property = kStyleFont;
		if (cmd == kResetStyleForegroundCmd)
			property = kStyleForeground;
		else if (cmd == kResetStyleBackgroundCmd)
			property = kStyleBackground;
		GlkOptionsState pending(*_settings);
		pending.resetStyleProperty(_loadedStyleGrid, _loadedStyle, property);
		acceptPending(pending);
		refreshControl(property == kStyleFont ? static_cast<GUI::Widget *>(_styleFont) :
			(property == kStyleForeground ? _styleForeground : _styleBackground));
		return;
	}
	case kSaveGlobalCmd: {
		if (_hostContext != kLauncherOptions || !synchronizeDraft(true))
			return;
		Common::String invalid;
		GlkOptionsState pending(*_settings);
		if (!pending.stageGlobalPreferences(invalid)) {
			showGlobalPreferenceError(invalid);
			return;
		}
		acceptPending(pending);
		loadControls();
		if (_parentDialog)
			_parentDialog->reflowLayout();
		return;
	}
	case kRestoreDefaultsCmd:
	case kInterpreterDefaultsCmd: {
		GlkOptionsState pending(*_settings);
		pending.resetAllPreferences(cmd == kInterpreterDefaultsCmd);
		acceptPending(pending);
		loadControls();
		if (_parentDialog)
			_parentDialog->reflowLayout();
		g_gui.scheduleTopDialogRedraw();
		return;
	}
	default:
		OptionsContainerWidget::handleCommand(sender, cmd, data);
	}
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
		GUI::StaticTextWidget *label = _owner.findOptionLabel(getFullName(name));
		// Font::wordWrapText wraps at equality, so leave one pixel of slack.
		return label ? g_gui.getFont(GUI::ThemeEngine::kFontStyleBold).getStringWidth(label->getLabel()) + 1 : 0;
	}

	int getControlHeight(GUI::Widget *control) const {
		if (control->getType() == GUI::kPopUpWidget)
			return _metrics.popupHeight;
		if (control->getType() == GUI::kCheckboxWidget)
			return _metrics.checkboxHeight;
		if (control->getType() == GUI::kButtonWidget)
			return _metrics.buttonHeight;
		return _metrics.editHeight;
	}

	int getControlWidth(GUI::Widget *control, int height) const {
		int width = height;
		int minimumHeight = -1;
		control->getMinSize(width, minimumHeight);
		if (width < 0)
			width = height;
		if (control->getType() == GUI::kCheckboxWidget)
			width += height + g_gui.xmlEval()->getVar("Globals.Checkbox.Spacing", 0);
		else if (control->getType() == GUI::kButtonWidget)
			width = MAX(width + _metrics.spacing * 2, _layouts.getVar("Globals.Button.Width", width));
		return width;
	}

	void addBlock(int height) {
		if (_blocks)
			_contentHeight += _metrics.spacing;
		_contentHeight += height;
		++_blocks;
	}

	void setControl(GlkOptionRowControl &control, const char *name,
			GUI::Widget *widget, int width = -1,
			int height = -1) const {
		control.name = name;
		control.widget = widget;
		control.width = width;
		control.height = height;
	}

	void addMeasuredRow(const char *label, GlkOptionRowControl *controls,
			uint controlCount, bool grow = false) {
		int controlsWidth = 0;
		int controlsHeight = 0;
		for (uint index = 0; index < controlCount; ++index) {
			GlkOptionRowControl &control = controls[index];
			if (control.height < 0)
				control.height = getControlHeight(control.widget);
			if (control.width < 0)
				control.width = getControlWidth(control.widget, control.height);
			if (index)
				controlsWidth += _metrics.spacing;
			controlsWidth += control.width;
			controlsHeight = MAX(controlsHeight, control.height);
		}

		const bool checkbox = controlCount == 1 && controls[0].widget->getType() == GUI::kCheckboxWidget;
		int labelWidth = MAX(getLabelWidth(label), _labelColumnWidth);
		// A checkbox keeps its label beside it, wrapping within the remaining
		// row width. Other rows stack only when their measured contents cannot
		// fit, rather than when a label exceeds the preferred label column.
		if (checkbox) {
			labelWidth = MAX(labelWidth, _checkboxLabelWidth);
			labelWidth = MIN(labelWidth, MAX(1, _availableWidth - _metrics.spacing - controlsWidth));
		}
		const bool stacked = !checkbox &&
			labelWidth + _metrics.spacing + controlsWidth > _availableWidth;
		if (stacked)
			labelWidth = _availableWidth;
		GlkOptionsLabelWidget *labelWidget = _owner.findOptionLabel(getFullName(label));
		const int labelHeight = labelWidget ? labelWidget->wrappedHeight(labelWidth) : _metrics.textHeight;
		if (controlCount == 1 && grow)
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
		const GlkOptionsLabelWidget *label = _owner.findOptionLabel(getFullName(name));
		const int height = label->wrappedHeight(_availableWidth);
		_layouts.addWidget(name, "", _availableWidth, height, Graphics::kTextAlignStart, true, true);
		addBlock(height);
	}

	void addOptionRow(const char *label, const char *control,
			GUI::Widget *controlWidget, bool grow = false) {
		GlkOptionRowControl controls[1];
		setControl(controls[0], control, controlWidget);
		addMeasuredRow(label, controls, ARRAYSIZE(controls), grow);
	}

	void addColorRow(const char *prefix, GUI::PopUpWidget *popupWidget,
			GUI::EditTextWidget *hexWidget, GUI::Widget *swatchWidget) {
		const Common::String label = Common::String(prefix) + "Color";
		const Common::String popup = Common::String(prefix) + "Color0";
		const Common::String swatch = Common::String(prefix) + "Swatch";
		const Common::String hex = Common::String(prefix) + "Hex";
		const Common::String hash = Common::String(prefix) + "Prefix";
		GlkOptionRowControl controls[4];
		setControl(controls[0], popup.c_str(), popupWidget);
		setControl(controls[1], swatch.c_str(), swatchWidget, glkSwatchSize(), glkSwatchSize());
		setControl(controls[2], hash.c_str(), _owner.findOptionLabel(getFullName(hash.c_str())),
			g_gui.getStringWidth("#") + 1, _metrics.editHeight);
		setControl(controls[3], hex.c_str(), hexWidget);
		addMeasuredRow(label.c_str(), controls, ARRAYSIZE(controls));
	}

	void addStyleRow(const char *label, const char *control,
			GUI::Widget *controlWidget,
			const char *reset, GUI::ButtonWidget *resetWidget) {
		GlkOptionRowControl controls[2];
		setControl(controls[0], control, controlWidget, _styleEditorWidth);
		setControl(controls[1], reset, resetWidget, _styleResetWidth);
		addMeasuredRow(label, controls, ARRAYSIZE(controls));
	}

	void setStyleColumns(GUI::Widget *font, GUI::Widget *foreground,
			GUI::Widget *background) {
		const int foregroundWidth = getControlWidth(foreground, _metrics.editHeight);
		const int backgroundWidth = getControlWidth(background, _metrics.editHeight);
		_styleColorEditWidth = MAX(foregroundWidth, backgroundWidth);
		_styleEditorWidth = MAX(getControlWidth(font, _metrics.popupHeight), glkSwatchSize() + _metrics.spacing * 2 +
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
		setControl(controls[0], swatch, swatchWidget, glkSwatchSize(), glkSwatchSize());
		setControl(controls[1], prefix.c_str(), _owner.findOptionLabel(getFullName(prefix.c_str())),
			prefixWidth, _metrics.editHeight);
		setControl(controls[2], control, controlWidget, _styleColorEditWidth);
		uint count = 3;
		const int gap = _styleEditorWidth - glkSwatchSize() - prefixWidth -
			_styleColorEditWidth - _metrics.spacing * 3;
		if (gap > 0 && _styleEditorWidth + _metrics.spacing + _styleResetWidth <= _availableWidth)
			setControl(controls[count++], "", nullptr, gap, 0);
		setControl(controls[count++], reset, resetWidget, _styleResetWidth);
		addMeasuredRow(label, controls, count);
	}

	void close() {
		_layouts.closeLayout().closeDialog();
	}

	int getContentHeight() const {
		return _contentHeight + _effectiveInset * 2;
	}
};

void GlkOptionsWidget::reflowLayout() {
	// A refresh can replace the theme without changing the overlay size.
	// Update the status before emitting its layout, without reentering the parent.
	updatePreviewAvailability(false);
	GUI::OptionsContainerWidget::reflowLayout();
}

void GlkOptionsWidget::defineLayout(GUI::ThemeEval &layouts,
		const Common::String &layoutName,
		const Common::String &overlayedLayout) const {
	GlkOptionThemeMetrics themeMetrics;
	themeMetrics.textHeight = MAX(
		g_gui.xmlEval()->getVar("Globals.Line.Height", 16),
		g_gui.getFontHeight(GUI::ThemeEngine::kFontStyleBold));
	themeMetrics.editHeight = MAX(themeMetrics.textHeight, g_gui.getFontHeight(GUI::ThemeEngine::kFontStyleNormal) + 3);
	themeMetrics.popupHeight = layouts.getVar("Globals.PopUp.Height", themeMetrics.textHeight);
	themeMetrics.checkboxHeight = layouts.getVar("Globals.Checkbox.Height", themeMetrics.textHeight);
	themeMetrics.buttonHeight = layouts.getVar("Globals.Button.Height", themeMetrics.textHeight);
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
	availableWidth = MAX(1, availableWidth - horizontalPadding);
	const float scale = g_gui.getScaleFactor();
	const int pageInset = getGlkOptionPageInset(themeMetrics.textHeight, scale);
	const int effectivePageInset = (int)(pageInset * scale);
	const int contentWidth = MAX(1, availableWidth - effectivePageInset * 2);
	const int labelWidth = contentWidth / 3;
	const int rowSpacing = themeMetrics.spacing;

	GlkOptionsPageLayout reading(layouts, *this, "GlkOptionsReading",
		contentWidth, labelWidth, pageInset, effectivePageInset,
		themeMetrics);
	reading.addOptionRow("TFont", "TFont0", _tfontPopUp, true);
	reading.addExplanation("GridExplanation");
	reading.addOptionRow("propsizelbl", "propsize", _propsize);
	reading.addOptionRow("monosizelbl", "monosize", _monosize);
	reading.addOptionRow("justifylbl", "justify", _justify);
#ifdef USE_TTS
	reading.addOptionRow("speaklbl", "speak", _speak);
	reading.addOptionRow("speakinputlbl", "speak_input", _speakInput);
#endif
	reading.close();

	GlkOptionsPageLayout compatibility(layouts, *this,
		"GlkOptionsCompatibility", contentWidth, labelWidth,
		pageInset, effectivePageInset, themeMetrics);
	compatibility.addOptionRow("leadinglbl", "leading", _leading);
	compatibility.addOptionRow("baselinelbl", "baseline", _baseline);
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
	styles.addOptionRow("StyleWindowLabel", "StyleWindow", _styleWindow, true);
	styles.addOptionRow("StyleNameLabel", "StyleName", _styleName, true);
	styles.setStyleColumns(_styleFont, _styleForeground, _styleBackground);
	styles.addStyleRow("StyleFontLabel", "StyleFont", _styleFont, "ResetStyleFont", _resetStyleFont);
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
	layoutPage.addOptionRow("wmarginxlbl", "wmarginx", _wmarginx);
	layoutPage.addOptionRow("wmarginylbl", "wmarginy", _wmarginy);
	layoutPage.addOptionRow("tmarginxlbl", "tmarginx", _tmarginx);
	layoutPage.addOptionRow("tmarginylbl", "tmarginy", _tmarginy);
	layoutPage.addOptionRow("wpaddingxlbl", "wpaddingx", _wpaddingx);
	layoutPage.addOptionRow("wpaddingylbl", "wpaddingy", _wpaddingy);
	layoutPage.addOptionRow("wborderx", "wborderhorizontal", _wborderx);
	layoutPage.addOptionRow("wbordery", "wbordervertical", _wbordery);
	layoutPage.addOptionRow("lockcolslbl", "lockcols", _lockcols);
	layoutPage.addOptionRow("colslbl", "cols", _cols);
	layoutPage.addOptionRow("lockrowslbl", "lockrows", _lockrows);
	layoutPage.addOptionRow("rowslbl", "rows", _rows);
	layoutPage.close();

	GlkOptionsPageLayout interaction(layouts, *this,
		"GlkOptionsInteraction", contentWidth, labelWidth,
		pageInset, effectivePageInset, themeMetrics);
	const char *interactionCheckboxes[] = { "capslbl", "stylehintlabel", "safeclickslabel", "graphicslbl" };
	interaction.setCheckboxLabels(interactionCheckboxes, ARRAYSIZE(interactionCheckboxes));
	interaction.addOptionRow("caretshape", "caretShape", _caretShape, true);
	interaction.addOptionRow("linkstyle", "linkStyle", _linkStyle, true);
	interaction.addOptionRow("capslbl", "caps", _caps);
	interaction.addOptionRow("stylehintlabel", "stylehint", _styleHint);
	interaction.addOptionRow("safeclickslabel", "safeclicks", _safeClicks);
	interaction.addOptionRow("graphicslbl", "graphics", _graphics);
	interaction.close();

	GlkOptionsPageLayout paging(layouts, *this, "GlkOptionsPaging",
		contentWidth, labelWidth, pageInset, effectivePageInset,
		themeMetrics);
	paging.addOptionRow("moreprompt", "morePrompt", _morePrompt, true);
	paging.addOptionRow("morealignlbl", "morealign", _morealign, true);
	paging.addOptionRow("morefontlbl", "morefont", _morefont, true);
	paging.addOptionRow("quoteslbl", "quotes", _quotes, true);
	paging.addOptionRow("dasheslbl", "dashes", _dashes, true);
	paging.addOptionRow("spaceslbl", "spaces", _spaces, true);
	paging.close();

	const int pageHeights[] = { reading.getContentHeight(), compatibility.getContentHeight(),
		colors.getContentHeight(), styles.getContentHeight(), layoutPage.getContentHeight(),
		interaction.getContentHeight(), paging.getContentHeight() };
	const int pageHeight = pageHeights[_tabs->getActiveTab()];
	const int tabHeaderHeight = layouts.getVar("Globals.TabWidget.Tab.Height", lineHeight);
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
	const GlkOptionsLabelWidget *notice = findOptionLabel("GlkOptionsDialog.RestartNotice");
	const GlkOptionsLabelWidget *preferences = findOptionLabel("GlkOptionsDialog.PreferenceNotice");
	const GlkOptionsLabelWidget *status = findOptionLabel("GlkOptionsDialog.ActionStatus");
	const GlkOptionsLabelWidget *previewStatus = findOptionLabel("GlkOptionsDialog.PreviewStatus");
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

} // End of namespace Glk
