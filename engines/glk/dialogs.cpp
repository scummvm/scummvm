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

#include "glk/conf.h"
#include "glk/fonts.h"
#include "glk/screen.h"
#include "glk/streams.h"
#include "glk/windows.h"
#include "glk/window_text_buffer.h"
#include "glk/window_text_grid.h"

#include "common/system.h"
#include "common/translation.h"

#define NEW_LABEL(name, text, tooltip) \
	do { \
		auto label = new GUI::StaticTextWidget(widgetsBoss(), name, _(text), _(tooltip)); \
		label->setAlign(Graphics::kTextAlignRight); \
	} while (0)

#define NEW_EDIT(widget, name, tooltip) \
	widget = new GUI::EditTextWidget(widgetsBoss(), name, Common::U32String(), _(tooltip))

#define NEW_CHECKBOX(widget, name, tooltip) \
	widget = new GUI::CheckboxWidget(widgetsBoss(), name, Common::U32String(), _(tooltip))

#define NEW_POPUP(widget, name, tooltip) \
	widget = new GUI::PopUpWidget(widgetsBoss(), name, _(tooltip))

#define NEW_SLIDER(widget, name, tooltip) \
	widget = new GUI::SliderWidget(widgetsBoss(), name, _(tooltip))

#define NEW_EDIT_CMD(widget, name, tooltip, cmd) \
	widget = new GUI::EditTextWidget(widgetsBoss(), name, Common::U32String(), _(tooltip), cmd, cmd)

#define NEW_POPUP_CMD(widget, name, tooltip, cmd) \
	widget = new GUI::PopUpWidget(widgetsBoss(), name, _(tooltip), cmd)

#define NEW_SLIDER_CMD(widget, name, tooltip, cmd) \
	widget = new GUI::SliderWidget(widgetsBoss(), name, _(tooltip), cmd)

#define INIT_NUM(widget, configMember) \
	if (g_conf && widget) \
		widget->setEditString(Common::U32String(Common::String::format("%d", g_conf->configMember)))

#define INIT_INT(widget, configMember) \
	if (g_conf && widget) \
		widget->setEditString(Common::U32String(Common::String::format("%d", configMember)))

#define INIT_STRING(widget, configExpression) \
	if (g_conf && widget) \
		widget->setEditString(Common::U32String(configExpression))

#define INIT_BOOL(widget, configMember) \
	if (g_conf && widget) \
		widget->setState(configMember)

#define INIT_POPUP(widget, configValue) \
	if (g_conf && widget) \
		widget->setSelectedTag(configValue)

#define INIT_SLIDER_FLOAT(widget, configMember, minV, maxV) \
	if (g_conf && widget) { \
		widget->setMinValue(minV); \
		widget->setMaxValue(maxV); \
		widget->setValue((int)(configMember)); \
	}

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

#define NEW_LABEL_SLIDER_CMD(lbl_name, text, sld_widget, sld_name, val_widget, val_name, tooltip, cmd) \
	do { \
		NEW_LABEL(lbl_name, text, tooltip); \
		NEW_SLIDER_CMD(sld_widget, sld_name, tooltip, cmd); \
		val_widget = new GUI::StaticTextWidget(widgetsBoss(), val_name, Common::U32String("10"), _(tooltip)); \
		val_widget->setFlags(GUI::WIDGET_CLEARBG); \
	} while (0)

#define COLOR_EDIT(prefix, varhex, var, labelText, tooltip, hexcmd, colorcmd) \
	do { \
		NEW_LABEL(prefix "HexPrefix", "0x", tooltip); \
		NEW_EDIT_CMD(varhex, prefix "Hex", tooltip, hexcmd); \
		NEW_POPUP_CMD(var, prefix "Color0", tooltip, colorcmd); \
		NEW_LABEL(prefix "Color", labelText, tooltip); \
	} while (0)

namespace Glk {

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

struct BoolOption {
	GUI::CheckboxWidget *widget;
	int *targetInt;
	bool *targetBool;
	const char *confKey;
};

struct IntOption {
	GUI::EditTextWidget *widget;
	int *target;
	const char *confKey;
	int minValue;
};

struct FloatOption {
	GUI::EditTextWidget *widget;
	double *target;
	const char *confKey;
	double minValue;
};

struct ColorOption {
	GUI::PopUpWidget *popup;
	GUI::EditTextWidget *hexInput;
	WindowStyle *styles;	  // for t/g/w/b
	PropFontInfo *propInfo;   // for caret/link/more
	const char *confKey;
	bool isProp;			  // true for caret/link/more
};

struct FontOption {
	GUI::PopUpWidget *popup;
	WindowStyle *styles;
	const char *confKeyPrefix;
};

static GUI::StaticTextWidget *createHeading(GUI::GuiObject *boss, const char *id, Common::U32String text) {
	auto h = new GUI::StaticTextWidget(boss, id, text);
	h->setAlign(Graphics::kTextAlignCenter);
	return h;
}

GlkOptionsWidget::GlkOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain)
	: GUI::OptionsContainerWidget(boss, name, "GlkOptionsDialog", domain) {

	static const char *fontLabels[] = {
		_s("Monospace Regular"),
		_s("Monospace Bold"),
		_s("Monospace Italic"),
		_s("Monospace Bold Italic"),
		_s("Proportional Regular"),
		_s("Proportional Bold"),
		_s("Proportional Italic"),
		_s("Proportional Bold Italic"),
	};

	for (int f = MONOR; f <= PROPZ; ++f) {
		_tfontPopUps[f] = nullptr;
		_gfontPopUps[f] = nullptr;
	}

	_fontHeadinglbl = createHeading(widgetsBoss(), "GlkOptionsDialog.FontHeading", _("Font"));

	NEW_LABEL_POPUP("GlkOptionsDialog.TFont", _("Text:"), _tfontPopUps[0], "GlkOptionsDialog.TFont0", _("Font for the text"));
	for (int f = MONOR; f <= PROPZ; ++f)
		_tfontPopUps[0]->appendEntry(_(fontLabels[f]), f);

	NEW_LABEL_POPUP("GlkOptionsDialog.GFont", _("Grid:"), _gfontPopUps[0], "GlkOptionsDialog.GFont0", _("Font for drawn graphics like maps, diagrams, puzzles, etc"));
	for (int f = MONOR; f <= PROPZ; ++f)
		_gfontPopUps[0]->appendEntry(_(fontLabels[f]), f);

	_colorHeadinglbl = createHeading(widgetsBoss(), "GlkOptionsDialog.ColorHeading", _("Color"));

	COLOR_EDIT("GlkOptionsDialog.T", _manualTColorHexInput, _tcolorPopUps[0], _("Text:"), _("Color of the interactive text"), kTHexChangedCmd, kTColorChangedCmd);
	for (int c = 0; c <= 5; ++c)
		_tcolorPopUps[0]->appendEntry(_(GLK_COLORS[c].name), c);
	_tcolorPopUps[0]->appendEntry(_("<custom>"), 6);

	COLOR_EDIT("GlkOptionsDialog.G", _manualGColorHexInput, _gcolorPopUps[0], _("Grid:"), _("Color for drawn graphics such as maps, diagrams, puzzles, etc"), kGHexChangedCmd, kGColorChangedCmd);
	for (int c = 0; c <= 5; ++c)
		_gcolorPopUps[0]->appendEntry(_(GLK_COLORS[c].name), c);
	_gcolorPopUps[0]->appendEntry(_("<custom>"), 6);

	COLOR_EDIT("GlkOptionsDialog.W", _manualWColorHexInput, _wcolorPopUps[0], _("Window:"), _("Color of the window (background)"), kWHexChangedCmd, kWColorChangedCmd);
	_wcolorPopUps[0]->appendEntry(_("<default>"), 7);
	for (int c = 0; c <= 5; ++c)
		_wcolorPopUps[0]->appendEntry(_(GLK_COLORS[c].name), c);
	_wcolorPopUps[0]->appendEntry(_("<custom>"), 6);

	COLOR_EDIT("GlkOptionsDialog.B", _manualBColorHexInput, _bcolorPopUps[0], _("Border:"), _("Color of the window border"), kBHexChangedCmd, kBColorChangedCmd);
	_bcolorPopUps[0]->appendEntry(_("<default>"), 7);
	for (int c = 0; c <= 5; ++c)
		_bcolorPopUps[0]->appendEntry(_(GLK_COLORS[c].name), c);
	_bcolorPopUps[0]->appendEntry(_("<custom>"), 6);

    // I18N: This is a section for text windows border settings
	_borderHeadinglbl = createHeading(widgetsBoss(), "GlkOptionsDialog.wborderlbl", _("Border"));

	NEW_LABEL_EDIT("GlkOptionsDialog.wborderx", _("Horizontal:"), _wborderx, "GlkOptionsDialog.wborderhorizontal", _("Horizontal border size"));
	NEW_LABEL_EDIT("GlkOptionsDialog.wbordery", _("Vertical:"), _wbordery, "GlkOptionsDialog.wbordervertical", _("Vertical border size"));

	COLOR_EDIT("GlkOptionsDialog.C", _manualCColorHexInput, _ccolorPopUps[0], _("Caret:"), _("Color of the cursor"), kCHexChangedCmd, kCColorChangedCmd);
	_ccolorPopUps[0]->appendEntry(_("<default>"), 7);
	for (int c = 0; c <= 5; ++c)
		_ccolorPopUps[0]->appendEntry(_(GLK_COLORS[c].name), c);
	_ccolorPopUps[0]->appendEntry(_("<custom>"), 6);

	COLOR_EDIT("GlkOptionsDialog.L", _manualLColorHexInput, _lcolorPopUps[0], _("Link:"), _("Color for URLs if they appear in-game"), kLHexChangedCmd, kLColorChangedCmd);
	_lcolorPopUps[0]->appendEntry(_("<default>"), 7);
	for (int c = 0; c <= 5; ++c)
		_lcolorPopUps[0]->appendEntry(_(GLK_COLORS[c].name), c);
	_lcolorPopUps[0]->appendEntry(_("<custom>"), 6);

	COLOR_EDIT("GlkOptionsDialog.M", _manualMColorHexInput, _mcolorPopUps[0],
		// I18N: "More..." is a prompt in text windows, which appears when the text exceeds the window size
		_("More:"),
		_("Color for the \"More...\" markers in the text"), kMHexChangedCmd, kMColorChangedCmd);
	_mcolorPopUps[0]->appendEntry(_("<default>"), 7);
	for (int c = 0; c <= 5; ++c)
		_mcolorPopUps[0]->appendEntry(_(GLK_COLORS[c].name), c);
	_mcolorPopUps[0]->appendEntry(_("<custom>"), 6);

	// I18n: This is a section for typography settings, such as font size, spacing, etc.
	_typographyHeadinglbl = createHeading(widgetsBoss(), "GlkOptionsDialog.topographyheadinglbl", _("Typography"));

	NEW_LABEL_POPUP("GlkOptionsDialog.linkstyle", _("Link style:"), _linkStyle, "GlkOptionsDialog.linkStyle", _("Style for URLs if they appear in-game"));
	_linkStyle->appendEntry("1", 0);
	_linkStyle->appendEntry("2", 1);
	_linkStyle->appendEntry("3", 2);

	NEW_LABEL_POPUP("GlkOptionsDialog.caretshape", _("Caret shape:"), _caretShape, "GlkOptionsDialog.caretShape", _("Shape of the cursor"));
	_caretShape->appendEntry("0", 0);
	_caretShape->appendEntry("1", 1);
	_caretShape->appendEntry("2", 2);
	_caretShape->appendEntry("3", 3);
	_caretShape->appendEntry("4", 4);

	NEW_LABEL_EDIT("GlkOptionsDialog.moreprompt", _("More prompt:"), _morePrompt, "GlkOptionsDialog.morePrompt", _("Custom marker in place of the \"More...\" marker for eg. \"continue\". Leave it blank to use the game's default"));

	NEW_LABEL_CHECKBOX("GlkOptionsDialog.stylehintlabel", _("Style hints:"), _styleHint, "GlkOptionsDialog.stylehint", _("Let the game suggest text styling options"));
	NEW_LABEL_CHECKBOX("GlkOptionsDialog.safeclickslabel", _("Safe clicks:"), _safeClicks, "GlkOptionsDialog.safeclicks", _("Safely apply clicks while input is pending"));

	// I18N: This is setting for number of text columns
	NEW_LABEL_EDIT("GlkOptionsDialog.colslbl", _("Column count:"), _cols, "GlkOptionsDialog.cols", _("Number of columns"));
	// I18N: This is setting for number of text rows
	NEW_LABEL_EDIT("GlkOptionsDialog.rowslbl", _("Rows count:"), _rows, "GlkOptionsDialog.rows", _("Number of rows"));

	NEW_LABEL_CHECKBOX("GlkOptionsDialog.lockcolslbl", _("Lock columns:"), _lockcols, "GlkOptionsDialog.lockcols", _("Check it to manually change the column count"));
	NEW_LABEL_CHECKBOX("GlkOptionsDialog.lockrowslbl", _("Lock rows:"), _lockrows, "GlkOptionsDialog.lockrows", _("Check it to manually change the row count"));

	NEW_LABEL_CHECKBOX("GlkOptionsDialog.justifylbl", _("Justify:"), _justify, "GlkOptionsDialog.justify", _("Enable text justification"));

	NEW_LABEL_POPUP("GlkOptionsDialog.quoteslbl", _("Typographic quotes:"), _quotes, "GlkOptionsDialog.quotes", _("Choose typographic quotes"));
	_quotes->appendEntry(_("Off"), 0);
	// I18N: This is a setting for using normal typographic quotes, which are like in most books, with the opening quote higher than the closing one
	_quotes->appendEntry(_("Normal"), 1);
	// I18N: This is a setting for using "rabid" quotes, which are like normal typographic quotes but with the opening quote lower than the closing one, like in some comic books
	_quotes->appendEntry(_("Rabid"), 2);

	NEW_LABEL_CHECKBOX("GlkOptionsDialog.capslbl", _("Caps:"), _caps, "GlkOptionsDialog.caps", _("Force uppercase input"));

	NEW_LABEL_POPUP("GlkOptionsDialog.dasheslbl", _("Dashes:"), _dashes, "GlkOptionsDialog.dashes", _("Types of dashes"));
	_dashes->appendEntry(_("Off"), 0);
	// I18N: This is a setting for using normal typographic dashes, which are like in most books, with the em dash being the longest and the en dash being half of it
	_dashes->appendEntry(_("Em dashes"), 1);
	// I18N: This is a setting for using "en+em" dashes, which are like normal typographic dashes but with the en dash being the same length as the em dash, like in some comic books
	_dashes->appendEntry(_("En+Em dashes"), 2);

	_userExperiencelbl = createHeading(widgetsBoss(), "GlkOptionsDialog.userexplbl", _("User experience"));

	NEW_LABEL_POPUP("GlkOptionsDialog.spaceslbl", _("Spaces:"), _spaces, "GlkOptionsDialog.spaces", _("Types of spaces"));
	_spaces->appendEntry(_("Off"), 0);
	// I18N: This is a setting for compressing double spaces into single ones in text
	_spaces->appendEntry(_("Compress double spaces"), 1);
	// I18N: This is a setting for expanding single spaces into double ones in text
	_spaces->appendEntry(_("Expand single spaces"), 2);

	NEW_LABEL_CHECKBOX("GlkOptionsDialog.graphicslbl", _("Graphics:"), _graphics, "GlkOptionsDialog.graphics", _("Turn graphics on/off"));

	_wmarginHeadinglbl = createHeading(widgetsBoss(), "GlkOptionsDialog.WindowMarginHeading", _("Window margin"));
	NEW_LABEL_EDIT("GlkOptionsDialog.wmarginxlbl", _("Horizontal:"), _wmarginx, "GlkOptionsDialog.wmarginx", _("Horizontal margin for window"));
	NEW_LABEL_EDIT("GlkOptionsDialog.wmarginylbl", _("Vertical:"), _wmarginy, "GlkOptionsDialog.wmarginy", _("Vertical margin for window"));

	_wpaddingHeadinglbl = createHeading(widgetsBoss(), "GlkOptionsDialog.wpaddinglbl", _("Window padding"));
	NEW_LABEL_EDIT("GlkOptionsDialog.wpaddingxlbl", _("Horizontal:"), _wpaddingx, "GlkOptionsDialog.wpaddingx", _("Horizontal padding for window"));
	NEW_LABEL_EDIT("GlkOptionsDialog.wpaddingylbl", _("Vertical:"), _wpaddingy, "GlkOptionsDialog.wpaddingy", _("Vertical padding for window"));

	_tmarginHeadinglbl = createHeading(widgetsBoss(), "GlkOptionsDialog.tmarginlbl", _("Text margin"));
	NEW_LABEL_EDIT("GlkOptionsDialog.tmarginxlbl", _("Horizontal:"), _tmarginx, "GlkOptionsDialog.tmarginx", _("Horizontal margin for text"));
	NEW_LABEL_EDIT("GlkOptionsDialog.tmarginylbl", _("Vertical:"), _tmarginy, "GlkOptionsDialog.tmarginy", _("Vertical margin for text"));

	NEW_LABEL_EDIT("GlkOptionsDialog.leadinglbl", _("Leading:"), _leading, "GlkOptionsDialog.leading", _("Vertical distance between text rows. Valid inputs - 12, 16, 22 etc."));
	NEW_LABEL_EDIT("GlkOptionsDialog.baselinelbl", _("Baseline:"), _baseline, "GlkOptionsDialog.baseline", _("Invisible horizontal line on which text sits. Valid inputs - 8, 16 etc. as per your font size"));

	NEW_LABEL_SLIDER_CMD("GlkOptionsDialog.monosizelbl", _("Monosize:"), _monosize, "GlkOptionsDialog.monosize", _monosizeVal, "GlkOptionsDialog.monosizeval", _("Font size scaling for the monospace text font"), kMonoSizeCmd);
	NEW_LABEL_SLIDER_CMD("GlkOptionsDialog.propsizelbl", _("Propsize:"), _propsize, "GlkOptionsDialog.propsize", _propsizeVal, "GlkOptionsDialog.propsizeval", _("Font size scaling for the proportional text font"), kPropSizeCmd);

	// I18N: This is a section for settings related to the "More..." prompt in text windows
	_moreHeadinglbl = createHeading(widgetsBoss(), "GlkOptionsDialog.morelbl", _("More"));
	NEW_LABEL_POPUP("GlkOptionsDialog.morealignlbl", _("Align:"), _morealign, "GlkOptionsDialog.morealign", _("Alignment of the \"More...\" marker on the window"));
	_morealign->appendEntry(_("Left"), 0);
	_morealign->appendEntry(_("Centre"), 1);
	_morealign->appendEntry(_("Right"), 2);

	NEW_LABEL_POPUP("GlkOptionsDialog.morefontlbl", _("Font:"), _morefont, "GlkOptionsDialog.morefont", _("Font for the \"More...\" marker"));
	for (int f = MONOR; f <= PROPZ; ++f)
		_morefont->appendEntry(_(fontLabels[f]), f);

}

GlkOptionsWidget::~GlkOptionsWidget() {}

void GlkOptionsWidget::reflowLayout() {
	GUI::OptionsContainerWidget::reflowLayout();
	layoutHeadings();
}

void GlkOptionsWidget::layoutHeadings() {

	int16 containerX = 0, containerY = 0, containerW = 0, containerH = 0;
	bool useRTL = false;
	if (!g_gui.xmlEval()->getWidgetData("GlkOptionsDialog", containerX, containerY, containerW, containerH, useRTL))
		return;

	if (containerW <= 0)
		return;

	struct HeadingX {
		GUI::StaticTextWidget *widget;
		int16 *xOut;
		int16 *wOut;
	};

	int16 fontX, colorX, borderX, wmarginX, tmarginX, wpaddingX, moreX, typographyX, userexpX;
	int16 fontW, colorW, borderW, wmarginW, tmarginW, wpaddingW, moreW, typographyW, userexpW;

	HeadingX headingXPos[] = {
		{ _fontHeadinglbl, &fontX, &fontW },
		{ _colorHeadinglbl, &colorX, &colorW },
		{ _borderHeadinglbl, &borderX, &borderW },
		{ _wmarginHeadinglbl, &wmarginX, &wmarginW },
		{ _tmarginHeadinglbl, &tmarginX, &tmarginW },
		{ _wpaddingHeadinglbl, &wpaddingX, &wpaddingW },
		{ _moreHeadinglbl, &moreX, &moreW },
		{ _typographyHeadinglbl, &typographyX, &typographyW },
		{ _userExperiencelbl, &userexpX, &userexpW }
	};

	for (auto &h : headingXPos) {
		int w = MIN<int>(h.widget->getWidth(), containerW);
		int x = containerX + (containerW - w) / 2;
		*(h.wOut) = w;
		*(h.xOut) = x;
	}

	struct HeadingY {
		const char *rowWidgetName;
		GUI::StaticTextWidget *heading;
		int16 *yOut;
		int16 *rowYOut;
		int16 extraOffset;
	};

	int16 fontY, colorY, borderY, wmarginY, tmarginY, wpaddingY, moreY, typographyY, userexpY;
	int16 fontRowY, colorRowY, borderRowY, wmarginRowY, tmarginRowY, wpaddingRowY, moreRowY, typographyRowY, userexpRowY;

	int16 headingMinGap = g_gui.xmlEval()->getVar("GlkOptionsDialog.HeadingMinGap", 2);

	HeadingY headingYpos[] = {
		{ "GlkOptionsDialog.TFont0", _fontHeadinglbl, &fontY, &fontRowY, 0 },
		{ "GlkOptionsDialog.TColor0", _colorHeadinglbl, &colorY, &colorRowY, 0 },
		{ "GlkOptionsDialog.linkStyle", _typographyHeadinglbl, &typographyY, &typographyRowY, 0 },
		{ "GlkOptionsDialog.wborderx", _borderHeadinglbl, &borderY, &borderRowY, 0 },
		{ "GlkOptionsDialog.wmarginy", _wmarginHeadinglbl, &wmarginY, &wmarginRowY, 0 },
		{ "GlkOptionsDialog.tmarginx", _tmarginHeadinglbl, &tmarginY, &tmarginRowY, 0 },
		{ "GlkOptionsDialog.wpaddingx", _wpaddingHeadinglbl, &wpaddingY, &wpaddingRowY, 0 },
		{ "GlkOptionsDialog.morealign", _moreHeadinglbl, &moreY, &moreRowY, 0 },
		{ "GlkOptionsDialog.stylehintlabel", _userExperiencelbl, &userexpY, &userexpRowY, 0 }
	};

	int spacing = g_gui.xmlEval()->getVar("Globals.Line.Height", 16) / 2;

	for (auto &h : headingYpos) {
		int16 tmpX = 0, tmpW = 0, tmpH = 0;
		if (g_gui.xmlEval()->getWidgetData(h.rowWidgetName, tmpX, *h.rowYOut, tmpW, tmpH, useRTL))
			*h.yOut = *h.rowYOut - h.heading->getHeight() - spacing + h.extraOffset;
	}

	if (fontY + _fontHeadinglbl->getHeight() + headingMinGap >= colorY)
		fontY = colorY - _fontHeadinglbl->getHeight() - headingMinGap;

	struct HeadingResize {
		GUI::StaticTextWidget *widget;
		int16 *x;
		int16 *y;
		int16 *w;
	};

	HeadingResize headingResize[] = {
		{ _fontHeadinglbl, &fontX, &fontY, &fontW },
		{ _colorHeadinglbl, &colorX, &colorY, &colorW },
		{ _typographyHeadinglbl, &typographyX, &typographyY, &typographyW },
		{ _borderHeadinglbl, &borderX, &borderY, &borderW },
		{ _wmarginHeadinglbl, &wmarginX, &wmarginY, &wmarginW },
		{ _tmarginHeadinglbl, &tmarginX, &tmarginY, &tmarginW },
		{ _moreHeadinglbl, &moreX, &moreY, &moreW },
		{ _wpaddingHeadinglbl, &wpaddingX, &wpaddingY, &wpaddingW },
		{ _userExperiencelbl, &userexpX, &userexpY, &userexpW },
	};

	for (auto &h : headingResize)
		h.widget->resize(*h.x, *h.y, *h.w, h.widget->getHeight(), false);
}

static void setFontPopUp(GUI::PopUpWidget *popup, bool hasConf, FACES font, const char *settingName, const Common::String &domain) {

	if (hasConf)
		popup->setSelectedTag(font);
	else
		popup->setSelectedTag(Screen::getFontId(ConfMan.get(settingName, domain)));
}

static void setSimpleColorPopUp(GUI::PopUpWidget *popup, GUI::EditTextWidget *hexInput, bool hasConf, const char *confKey, const char *themeKey, const Common::String &domain, bool overrideEnabled, int defaultValue = 7) {

	Common::String targetKey = hasConf ? confKey : themeKey;

	if (hasConf && !overrideEnabled) {
		popup->setSelectedTag(defaultValue);
		hexInput->setEditString(Common::U32String());
		return;
	}

	// Grab the color from ConfMan (returns "RRGGBB" or "RRGGBB,RRGGBB")
	Common::String colorStr = ConfMan.get(targetKey, domain);
	Common::String hexPart = colorStr.substr(0, 6);

	if (hexInput) {
		hexInput->setEditString(Common::U32String(hexPart));
	}

	unsigned int rgb = 0;
	sscanf(hexPart.c_str(), "%x", &rgb);
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

void GlkOptionsWidget::load() {
	bool tempConfCreated = false;
	if (!g_conf) {
		tempConfCreated = true;
		// instantiate a global g_conf so the dialog can read values from it
		new Conf(INTERPRETER_GLULX);
		g_conf->load();
	}

	BoolOption boolOptions[] = {
		{ _safeClicks, nullptr, &g_conf->_safeClicks, "safeclicks" },
		{ _styleHint, &g_conf->_styleHint, nullptr, "stylehint" },
		{ _lockcols, &g_conf->_lockCols, nullptr, "lockcols" },
		{ _lockrows, &g_conf->_lockRows, nullptr, "lockrows" },
		{ _justify, &g_conf->_propInfo._justify, nullptr, "justify" },
		{ _caps, &g_conf->_propInfo._caps, nullptr, "caps" },
		{ _graphics, nullptr, &g_conf->_graphics, "graphics" }
	};

	IntOption intOptions[] = {
		{ _wmarginx, &g_conf->_wMarginX, "wmarginx", 0 },
		{ _wmarginy, &g_conf->_wMarginY, "wmarginy", 0 },
		{ _wpaddingx, &g_conf->_wPaddingX, "wpaddingx", 0 },
		{ _wpaddingy, &g_conf->_wPaddingY, "wpaddingy", 0 },
		{ _tmarginx, &g_conf->_tMarginX, "tmarginx", 0 },
		{ _tmarginy, &g_conf->_tMarginY, "tmarginy", 0 },
		{ _leading, &g_conf->_monoInfo._leading, "leading", 0 },
		{ _baseline, &g_conf->_propInfo._baseLine, "baseline", 0 }
	};

	ColorOption colorOptions[] = {
		{ _tcolorPopUps[0], _manualTColorHexInput, g_conf->_tStyles, nullptr, "tcolor_0", false },
		{ _gcolorPopUps[0], _manualGColorHexInput, g_conf->_gStyles, nullptr, "gcolor_0", false },
		{ _wcolorPopUps[0], _manualWColorHexInput, nullptr, nullptr, "windowcolor", false },
		{ _bcolorPopUps[0], _manualBColorHexInput, nullptr, nullptr, "bordercolor", false },
		{ _ccolorPopUps[0], _manualCColorHexInput, nullptr, &g_conf->_propInfo, "caretcolor", true },
		{ _lcolorPopUps[0], _manualLColorHexInput, nullptr, &g_conf->_propInfo, "linkcolor", true },
		{ _mcolorPopUps[0], _manualMColorHexInput, nullptr, &g_conf->_propInfo, "morecolor", true }
	};

	FontOption fontOptions[] = {
		{ _tfontPopUps[0], g_conf->_tStyles, "tfont" },
		{ _gfontPopUps[0], g_conf->_gStyles, "gfont" }
	};

	for (auto &opt : fontOptions) {
		if (g_conf)
			setFontPopUp(opt.popup, true, opt.styles[style_Normal].font, nullptr, _domain);
		else
			setFontPopUp(opt.popup, false, (FACES)0, opt.confKeyPrefix, _domain);
	}

	for (auto &opt : colorOptions) {
		if (g_conf) {
			bool overrideEnabled = true;
			if (!opt.isProp && strcmp(opt.confKey, "windowcolor") == 0)
				overrideEnabled = g_conf->_windowColorOverride;
			if (!opt.isProp && strcmp(opt.confKey, "bordercolor") == 0)
				overrideEnabled = g_conf->_borderColorOverride;
			setSimpleColorPopUp(opt.popup, opt.hexInput, true, opt.confKey, opt.confKey, _domain, overrideEnabled);
		} else {
			setSimpleColorPopUp(opt.popup, opt.hexInput, false, opt.confKey, opt.confKey, _domain, true);
		}
	}

	// number fields
	for (auto &opt : intOptions) {
		if (opt.widget && g_conf)
			opt.widget->setEditString(Common::U32String(Common::String::format("%d", *opt.target)));
	}

	for (auto &opt : boolOptions) {
		if (opt.widget && g_conf) {
			if (opt.targetBool)
				opt.widget->setState(*opt.targetBool);
			else
				opt.widget->setState(*opt.targetInt != 0);
		}
	}

	INIT_NUM(_wborderx ,_wBorderX);
	INIT_NUM(_wbordery ,_wBorderY);
	INIT_POPUP(_linkStyle, g_conf->_propInfo._linkStyle);
	INIT_POPUP(_caretShape, g_conf->_propInfo._caretShape);
	INIT_STRING(_morePrompt, g_conf->_propInfo._morePrompt);
	INIT_BOOL(_styleHint, g_conf->_styleHint);
	INIT_BOOL(_safeClicks, g_conf->_safeClicks);
	INIT_NUM(_cols, _cols);
	INIT_NUM(_rows, _rows);
	INIT_BOOL(_lockcols, g_conf->_lockCols);
	INIT_BOOL(_lockrows, g_conf->_lockRows);
	INIT_BOOL(_justify, g_conf->_propInfo._justify);
	INIT_BOOL(_caps, g_conf->_propInfo._caps);
	INIT_POPUP(_quotes, g_conf->_propInfo._quotes);
	INIT_POPUP(_dashes, g_conf->_propInfo._dashes);
	INIT_POPUP(_spaces, g_conf->_propInfo._spaces);
	INIT_BOOL(_graphics, g_conf->_graphics);
	INIT_NUM(_wmarginx, _wMarginX);
	INIT_NUM(_wmarginy, _wMarginY);
	INIT_NUM(_wpaddingx, _wPaddingX);
	INIT_NUM(_wpaddingy, _wPaddingY);
	INIT_NUM(_tmarginx, _tMarginX);
	INIT_NUM(_tmarginy, _tMarginY);
	INIT_INT(_leading, g_conf->_propInfo._leading);
	INIT_INT(_baseline, g_conf->_propInfo._baseLine);
	INIT_SLIDER_FLOAT(_monosize, g_conf->_monoInfo._size, 10, 25);
	INIT_SLIDER_FLOAT(_propsize, g_conf->_propInfo._size, 10, 25);
	_monosizeVal->setValue(_monosize->getValue());
	_propsizeVal->setValue(_propsize->getValue());
	INIT_POPUP(_morealign, g_conf->_propInfo._moreAlign);
	INIT_POPUP(_morefont, g_conf->_propInfo._moreFont);

	if (tempConfCreated)
		delete g_conf;
}

void GlkOptionsWidget::defineLayout(GUI::ThemeEval &layouts,
									const Common::String &layoutName,
									const Common::String &overlayedLayout) const {
	int16 baseScale = g_gui.xmlEval()->getVar("Globals.Line.Height", 16);
	int16 labelWidth = baseScale * 4;
	int16 popUpWidth = baseScale * 4.5;
	int16 prefixWidth = baseScale * 1.5;
	int16 hexWidth = baseScale * 3.5;

	layouts.addDialog(layoutName, overlayedLayout)
			.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
				.addPadding(0, 0, 0, 0)
				.addSpace()
				.addLayout(GUI::ThemeLayout::kLayoutVertical)
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(8, 8, 8, 8)
						.addWidget("FontHeading", "OptionsLabel")
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, prefixWidth + hexWidth, 0, 0)
				   		.addWidget("TFont", "OptionsLabel", labelWidth, baseScale)
						.addWidget("TFont0", "PopUp", baseScale * 10)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, prefixWidth + hexWidth, 0, 0)
						.addWidget("GFont", "OptionsLabel", labelWidth, baseScale)
						.addWidget("GFont0", "PopUp", baseScale * 10)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(8, 8, 8, 8)
						.addWidget("ColorHeading", "OptionsLabel")
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("TColor", "OptionsLabel", labelWidth)
						.addWidget("TColor0", "PopUp", popUpWidth)
						.addWidget("THexPrefix", "", prefixWidth)
						.addWidget("THex", "EditTextWidget", hexWidth)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("GColor", "OptionsLabel")
						.addWidget("GColor0", "PopUp", popUpWidth)
						.addWidget("GHexPrefix", "", prefixWidth)
						.addWidget("GHex", "EditTextWidget", hexWidth)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("WColor", "OptionsLabel")
						.addWidget("WColor0", "PopUp", popUpWidth)
						.addWidget("WHexPrefix", "", prefixWidth)
						.addWidget("WHex", "EditTextWidget", hexWidth)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("BColor", "OptionsLabel")
						.addWidget("BColor0", "PopUp", popUpWidth)
						.addWidget("BHexPrefix", "", prefixWidth)
						.addWidget("BHex", "EditTextWidget", hexWidth)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("CColor", "OptionsLabel")
						.addWidget("CColor0", "PopUp", popUpWidth)
						.addWidget("CHexPrefix", "", prefixWidth)
						.addWidget("CHex", "EditTextWidget", hexWidth)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("LColor", "OptionsLabel")
						.addWidget("LColor0", "PopUp", popUpWidth)
						.addWidget("LHexPrefix", "", prefixWidth)
						.addWidget("LHex", "EditTextWidget", hexWidth)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("MColor", "OptionsLabel")
						.addWidget("MColor0", "PopUp", popUpWidth)
						.addWidget("MHexPrefix", "", prefixWidth)
						.addWidget("MHex", "EditTextWidget", hexWidth)
					.closeLayout()
						.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(8, 8, 8, 8)
						.addWidget("wborderlbl", "OptionsLabel")
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("wborderx", "OptionsLabel")
						.addWidget("wborderhorizontal", "EditTextWidget", baseScale * 2)
						.addWidget("wbordery", "", baseScale * 3.75)
						.addWidget("wbordervertical", "EditTextWidget", baseScale * 2)
					.closeLayout()
						.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(8, 8, 8, 8)
						.addWidget("WindowMarginHeading", "OptionsLabel")
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("wmarginxlbl", "OptionsLabel")
						.addWidget("wmarginx", "EditTextWidget", baseScale * 2)
						.addWidget("wmarginylbl", "", baseScale * 3.75)
						.addWidget("wmarginy", "EditTextWidget", baseScale * 2)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(8, 8, 8, 8)
						.addWidget("tmarginlbl", "OptionsLabel")
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("tmarginxlbl", "OptionsLabel")
						.addWidget("tmarginx", "EditTextWidget", baseScale * 2)
						.addWidget("tmarginylbl", "", baseScale * 3.75)
						.addWidget("tmarginy", "EditTextWidget", baseScale * 2)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(8, 8, 8, 8)
						.addWidget("wpaddinglbl", "OptionsLabel")
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("wpaddingxlbl", "OptionsLabel")
						.addWidget("wpaddingx", "EditTextWidget", baseScale * 1.875)
						.addWidget("wpaddingylbl", "", baseScale * 3.75)
						.addWidget("wpaddingy", "EditTextWidget", baseScale * 1.875)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(8, 8, 8, 8)
						.addWidget("morelbl", "OptionsLabel")
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("morealignlbl", "OptionsLabel")
						.addWidget("morealign", "PopUp", baseScale * 10)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("morefontlbl", "OptionsLabel")
						.addWidget("morefont", "PopUp", baseScale * 10)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(8, 8, 8, 8)
						.addWidget("topographyheadinglbl", "OptionsLabel")
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("linkstyle", "OptionsLabel")
						.addWidget("linkStyle", "EditTextWidget", baseScale * 10)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("caretshape", "OptionsLabel")
						.addWidget("caretShape", "EditTextWidget", baseScale * 10)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("moreprompt", "OptionsLabel")
						.addWidget("morePrompt", "EditTextWidget", baseScale * 10)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("leadinglbl", "OptionsLabel")
						.addWidget("leading", "EditTextWidget", baseScale * 10)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("baselinelbl", "OptionsLabel")
						.addWidget("baseline", "EditTextWidget", baseScale * 10)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("monosizelbl", "OptionsLabel")
						.addWidget("monosize", "SliderWidget", baseScale * 10)
						.addWidget("monosizeval", "StaticTextWidget", baseScale * 2)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0 ,0)
						.addWidget("propsizelbl", "OptionsLabel")
						.addWidget("propsize", "SliderWidget", baseScale * 10)
						.addWidget("propsizeval", "StaticTextWidget", baseScale * 2)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("capslbl", "OptionsLabel")
						.addWidget("caps", "Checkbox", baseScale * 10)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("quoteslbl", "OptionsLabel")
						.addWidget("quotes", "PopUp", baseScale * 10)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("dasheslbl", "OptionsLabel")
						.addWidget("dashes", "PopUp", baseScale * 10)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("spaceslbl", "OptionsLabel")
						.addWidget("spaces", "PopUp", baseScale * 10)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(8, 8, 8, 8)
						.addWidget("userexplbl", "OptionsLabel")
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("stylehintlabel", "OptionsLabel")
						.addWidget("stylehint", "Checkbox", baseScale * 5)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("safeclickslabel", "OptionsLabel")
						.addWidget("safeclicks", "Checkbox", baseScale * 5)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("colslbl", "OptionsLabel")
						.addWidget("cols", "EditTextWidget", baseScale * 1.875)
						.addWidget("lockcolslbl", "OptionsLabel")
						.addWidget("lockcols", "Checkbox", baseScale * 2)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("rowslbl", "OptionsLabel")
						.addWidget("rows", "EditTextWidget", baseScale * 1.875)
						.addWidget("lockrowslbl", "OptionsLabel")
						.addWidget("lockrows", "Checkbox", baseScale * 2)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0, 0)
						.addWidget("justifylbl", "OptionsLabel")
						.addWidget("justify", "Checkbox", baseScale * 5)
					.closeLayout()
					.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
						.addPadding(0, 0, 0 ,0)
						.addWidget("graphicslbl", "OptionsLabel")
						.addWidget("graphics", "Checkbox", baseScale * 5)
					.closeLayout()
				.closeLayout()
				.addSpace()
			.closeLayout()
		.closeDialog();
}

static void saveFontPopUp(GUI::PopUpWidget *popup, WindowStyle *styles, const char *confKeyPrefix, const Common::String &domain) {

	FACES selectedFont = (FACES)popup->getSelectedTag();
	for (int i = 0; i < style_NUMSTYLES; ++i) {
		styles[i].font = selectedFont;
		Common::String key = Common::String::format("%s%d", confKeyPrefix, i);
		ConfMan.set(key, Screen::getFontName(selectedFont), domain);
	}
}

static void saveColorPopUp(GUI::PopUpWidget *popup, GUI::EditTextWidget *hexInput, WindowStyle *styles, const char *prefix, const Common::String &domain) {

	int idx = popup->getSelectedTag();
	unsigned int rgb = 0;
	bool valid = false;

	if (idx >= 0 && idx <= 5) {
		rgb = GLK_COLORS[idx].rgb;
		valid = true;
	} else if (idx == 6) { // custom
		Common::String hexStr = hexInput ? hexInput->getEditString().encode() : Common::String();
		if (hexStr.size() == 6) {
			unsigned int parsed;
			if (sscanf(hexStr.c_str(), "%x", &parsed) == 1) {
				rgb = parsed;
				valid = true;
			}
		}
	}

	if (!valid)
		return;

	byte r = (rgb >> 16) & 0xFF;
	byte g = (rgb >> 8) & 0xFF;
	byte b = rgb & 0xFF;
	Common::String rgbStr = Common::String::format("%02x%02x%02x", r, g, b);

	ConfMan.set(prefix, rgbStr, domain);

	Common::String stylePrefix(prefix);
	if (!stylePrefix.empty() && stylePrefix[stylePrefix.size()-1] == '0')
		stylePrefix = stylePrefix.substr(0, stylePrefix.size()-1);

	unsigned int engineColor = g_conf->parseColor(rgb);

		if (styles) {
			for (int i = 0; i < style_NUMSTYLES; ++i)
				styles[i].fg = engineColor;
		}

	for (int i = 0; i < style_NUMSTYLES; ++i) {
		Common::String key = Common::String::format("%s%d", stylePrefix.c_str(), i);
		Common::String line = Common::String::format("%s,%s", rgbStr.c_str(), rgbStr.c_str());
		ConfMan.set(key, line, domain);
	}
}

static void savePropColorPopUp(GUI::PopUpWidget *popup, GUI::EditTextWidget *hexInput, PropFontInfo &propInfo, const char *confKey, const Common::String &domain) {
	int idx = popup->getSelectedTag();
	unsigned int rgb = 0;
	bool valid = false;

	if (idx >= 0 && idx <= 5) {
		rgb = GLK_COLORS[idx].rgb;
		valid = true;
	} else if (idx == 6) {
		Common::String hex = hexInput ? hexInput->getEditString().encode() : Common::String();
		hex.toLowercase();
		if (hex.size() == 6) {
			bool allHex = true;
			for (char c : hex) {
				if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
					allHex = false;
					break;
				}
			}
			if (allHex) {
				sscanf(hex.c_str(), "%x", &rgb);
				valid = true;
			}
		}
	} else if (idx == 7) {
		ConfMan.set(confKey, Common::String(), domain);
		return;
	}

	if (!valid)
		return;

	unsigned int engineColor = g_conf->parseColor(rgb);
	if (strcmp(confKey, "caretcolor") == 0)
		propInfo._caretColor = engineColor;
	else if (strcmp(confKey, "linkcolor") == 0)
		propInfo._linkColor = engineColor;
	else if (strcmp(confKey, "morecolor") == 0)
		propInfo._moreColor = engineColor;

	byte r = (rgb >> 16) & 0xFF;
	byte g = (rgb >> 8) & 0xFF;
	byte b = rgb & 0xFF;
	Common::String rgbStr = Common::String::format("%02x%02x%02x", r, g, b);
	Common::String value = Common::String::format("%s,%s", rgbStr.c_str(), rgbStr.c_str());
	ConfMan.set(confKey, value, domain);
}

static void saveIntField(GUI::EditTextWidget *widget, int &outValue, const char *confKey, int minValue, const Common::String &domain) {
	Common::String txt = widget->getEditString().encode();
	int value = txt.empty() ? 0 : atoi(txt.c_str());
	if (value < minValue) value = minValue;
		outValue = value;
	ConfMan.setInt(confKey, value, domain);
}

static void saveBoolField(GUI::CheckboxWidget *widget, bool &outValue, const char *confKey, const Common::String &domain) {
	bool value = widget->getState();
	outValue = value;
	ConfMan.setBool(confKey, value, domain);
}

static void saveBoolField(GUI::CheckboxWidget *widget, int &outValue, const char *confKey, const Common::String &domain) {
	bool value = widget->getState();
	outValue = value ? 1 : 0;
	ConfMan.setInt(confKey, outValue, domain);
}

static void savePopupField(GUI::PopUpWidget *popup, int &outValue, const char *confKey, const Common::String &domain) {
	int value = popup->getSelectedTag();
	outValue = value;
	ConfMan.setInt(confKey, value, domain);
}

static void saveSliderFloatField(GUI::SliderWidget *widget, double &outValue, const char *confKey, const Common::String &domain) {
	if (!widget) return;

	double value = widget->getValue();
	outValue = value;
	ConfMan.setFloat(confKey, value, domain);
}

bool GlkOptionsWidget::save() {

	bool tempConfCreated = false;
	if (!g_conf) {
		tempConfCreated = true;
		// instantiate a global g_conf so the dialog can save values into it
		new Conf(INTERPRETER_GLULX);
		g_conf->load();
	}

	BoolOption boolOptions[] = {
		{ _safeClicks, nullptr, &g_conf->_safeClicks, "safeclicks" },
		{ _styleHint, &g_conf->_styleHint, nullptr, "stylehint" },
		{ _lockcols, &g_conf->_lockCols, nullptr, "lockcols" },
		{ _lockrows, &g_conf->_lockRows, nullptr, "lockrows" },
		{ _justify, &g_conf->_propInfo._justify, nullptr, "justify" },
		{ _caps, &g_conf->_propInfo._caps, nullptr, "caps" },
		{ _graphics, nullptr, &g_conf->_graphics, "graphics" }
	};

	IntOption intOptions[] = {
		{ _wmarginx, &g_conf->_wMarginX, "wmarginx", 0 },
		{ _wmarginy, &g_conf->_wMarginY, "wmarginy", 0 },
		{ _wpaddingx, &g_conf->_wPaddingX, "wpaddingx", 0 },
		{ _wpaddingy, &g_conf->_wPaddingY, "wpaddingy", 0 },
		{ _tmarginx, &g_conf->_tMarginX, "tmarginx", 0 },
		{ _tmarginy, &g_conf->_tMarginY, "tmarginy", 0 },
		{ _leading, &g_conf->_monoInfo._leading, "leading", 0 },
		{ _baseline, &g_conf->_propInfo._baseLine, "baseline", 0 }
	};

	ColorOption colorOptions[] = {
		{ _tcolorPopUps[0], _manualTColorHexInput, g_conf->_tStyles, nullptr, "tcolor_0", false },
		{ _gcolorPopUps[0], _manualGColorHexInput, g_conf->_gStyles, nullptr, "gcolor_0", false },
		{ _wcolorPopUps[0], _manualWColorHexInput, nullptr, nullptr, "windowcolor", false },
		{ _bcolorPopUps[0], _manualBColorHexInput, nullptr, nullptr, "bordercolor", false },
		{ _ccolorPopUps[0], _manualCColorHexInput, nullptr, &g_conf->_propInfo, "caretcolor", true },
		{ _lcolorPopUps[0], _manualLColorHexInput, nullptr, &g_conf->_propInfo, "linkcolor", true },
		{ _mcolorPopUps[0], _manualMColorHexInput, nullptr, &g_conf->_propInfo, "morecolor", true }
	};

	FontOption fontOptions[] = {
		{ _tfontPopUps[0], g_conf->_tStyles, "tfont" },
		{ _gfontPopUps[0], g_conf->_gStyles, "gfont" }
	};


	for (auto &opt : fontOptions)
		saveFontPopUp(opt.popup, opt.styles, opt.confKeyPrefix, _domain);

	for (auto &opt : colorOptions) {
		if (opt.isProp)
			savePropColorPopUp(opt.popup, opt.hexInput, *opt.propInfo, opt.confKey, _domain);
		else
			saveColorPopUp(opt.popup, opt.hexInput, opt.styles, opt.confKey, _domain);
	}

	for (auto &opt : intOptions)
		if (opt.widget) saveIntField(opt.widget, *opt.target, opt.confKey, opt.minValue, _domain);

	saveSliderFloatField(_monosize, g_conf->_monoInfo._size, "monosize", _domain);
	saveSliderFloatField(_propsize, g_conf->_propInfo._size, "propsize", _domain);

	for (auto &opt : boolOptions) {
		if (opt.widget) {
			if (opt.targetBool)
				saveBoolField(opt.widget, *opt.targetBool, opt.confKey, _domain);
			else if (opt.targetInt)
				saveBoolField(opt.widget, *opt.targetInt, opt.confKey, _domain);
		}
	}

	g_conf->_propInfo._leading = g_conf->_monoInfo._leading;
	g_conf->_monoInfo._baseLine = g_conf->_propInfo._baseLine;

	Common::String WHexStr = _manualWColorHexInput->getEditString().encode();
	Common::String BHexStr = _manualBColorHexInput->getEditString().encode();
	Common::String bordStrx = _wborderx->getEditString().encode();
	Common::String bordStry = _wbordery->getEditString().encode();
	Common::String morepromptStr = _morePrompt->getEditString().encode();
	Common::String colsStr = _cols->getEditString().encode();
	Common::String rowsStr = _rows->getEditString().encode();


	int wIdx = _wcolorPopUps[0]->getSelectedTag();

	if (wIdx == 7) {
		// user chose to return control to the game
		g_conf->_windowColorOverride = false;
		ConfMan.setBool("windowcolor_override", false, _domain);
		// revert the stored colour to the original saved value. the game may immediately overwrite it
		// when it sends its next colour command.
		g_conf->_windowColor = g_conf->_windowSave;
		g_conf->_borderColor = g_conf->_borderSave;
		// clear global override so that next colour command from the game (or the existing _windowColor)
		// determines what is shown
		Windows::_overrideBgSet = false;
	} else {
		// any other selection does a manual override
		g_conf->_windowColorOverride = true;
		ConfMan.setBool("windowcolor_override", true, _domain);
		Windows::_overrideBgSet = true;

		if (wIdx == 6) {
			if (WHexStr.size() == 6) {
				unsigned int rgb;
				sscanf(WHexStr.c_str(), "%x", &rgb);
				unsigned int engineColor = g_conf->parseColor(rgb);
				byte r = (rgb >> 16) & 0xFF;
				byte g = (rgb >> 8) & 0xFF;
				byte b = rgb & 0xFF;
				Common::String rgbStr = Common::String::format("%02x%02x%02x", r, g, b);

				g_conf->_windowColor = engineColor;
				ConfMan.set("windowcolor", rgbStr, _domain);
			}
		} else if (_wcolorPopUps[0]) {
			int idx = _wcolorPopUps[0]->getSelectedTag();
			if (idx >= 0 && idx <= 5) {
				unsigned int engineColor = g_conf->parseColor(GLK_COLORS[idx].rgb);
				g_conf->_windowColor = engineColor;
				byte r = (GLK_COLORS[idx].rgb >> 16) & 0xFF;
				byte g = (GLK_COLORS[idx].rgb >> 8) & 0xFF;
				byte b = GLK_COLORS[idx].rgb & 0xFF;
				Common::String rgbStr = Common::String::format("%02x%02x%02x", r, g, b);
				ConfMan.set("windowcolor", rgbStr, _domain);
			}
		}
	}

	int bIdx = _bcolorPopUps[0]->getSelectedTag();

	if (bIdx == 7) {
		g_conf->_borderColorOverride = false;
		ConfMan.setBool("bordercolor_override", false, _domain);

		g_conf->_windowColor = g_conf->_windowSave;
		g_conf->_borderColor = g_conf->_borderSave;

		Windows::_overrideBgSet = false;
	} else {
		g_conf->_borderColorOverride = true;
		ConfMan.setBool("bordercolor_override", true, _domain);
		Windows::_overrideBgSet = true;

		if (bIdx == 6) {
			if (BHexStr.size() == 6) {
				unsigned int rgb;
				sscanf(BHexStr.c_str(), "%x", &rgb);
				unsigned int engineColor = g_conf->parseColor(rgb);
				byte r = (rgb >> 16) & 0xFF;
				byte g = (rgb >> 8) & 0xFF;
				byte b = rgb & 0xFF;
				Common::String rgbStr = Common::String::format("%02x%02x%02x", r, g, b);

				g_conf->_borderColor = engineColor;
				ConfMan.set("bordercolor", rgbStr, _domain);
			}
		} else if (_bcolorPopUps[0]) {
			int idx = _bcolorPopUps[0]->getSelectedTag();
			if (idx >= 0 && idx <= 5) {
				unsigned int engineColor = g_conf->parseColor(GLK_COLORS[idx].rgb);
				g_conf->_borderColor = engineColor;
				byte r = (GLK_COLORS[idx].rgb >> 16) & 0xFF;
				byte g = (GLK_COLORS[idx].rgb >> 8) & 0xFF;
				byte b = GLK_COLORS[idx].rgb & 0xFF;
				Common::String rgbStr = Common::String::format("%02x%02x%02x", r, g, b);
				ConfMan.set("bordercolor", rgbStr, _domain);
			}
		}
	}

	int bordx = bordStrx.empty() ? 0 : atoi(bordStrx.c_str());
	if (bordx < 0)
		bordx = 0;

	g_conf->_wBorderX = bordx;
	ConfMan.setInt("wborderx", bordx, _domain);

	int bordy = bordStry.empty() ? 0 : atoi(bordStry.c_str());
	if (bordy < 0)
		bordy = 0;
	g_conf->_wBorderY = bordy;
	ConfMan.setInt("wbordery", bordy, _domain);


	if (_linkStyle) {
		savePopupField(_linkStyle, g_conf->_propInfo._linkStyle, "linkstyle", _domain);
		g_conf->_monoInfo._linkStyle = g_conf->_propInfo._linkStyle;
	}

	if (_caretShape) {
		savePopupField(_caretShape, g_conf->_propInfo._caretShape, "caretShape", _domain);
		g_conf->_monoInfo._caretShape = g_conf->_propInfo._caretShape;
	}

	g_conf->_propInfo._morePrompt = morepromptStr;
	ConfMan.set("moreprompt", morepromptStr, _domain);

	if (_cols) {
		int cols = colsStr.empty() ? 0 : atoi(colsStr.c_str());
		g_conf->_cols = cols;
		ConfMan.setInt("cols", cols, _domain);
	}

	if (_rows) {
		int rows = rowsStr.empty() ? 0 : atoi(rowsStr.c_str());
		g_conf->_rows = rows;
		ConfMan.setInt("rows", rows, _domain);
	}

	if (_quotes)
		savePopupField(_quotes, g_conf->_propInfo._quotes, "quotes", _domain);

	if (_dashes)
		savePopupField(_dashes, g_conf->_propInfo._dashes, "dashes", _domain);

	if (_spaces)
		savePopupField(_spaces, g_conf->_propInfo._spaces, "spaces", _domain);


	if (_morealign) {
		int x = _morealign->getSelectedTag();
		g_conf->_propInfo._moreAlign = x;
		ConfMan.setInt("morealign", x, _domain);
	}

	if (_morefont) {
		FACES tf = (FACES)_morefont->getSelectedTag();
		g_conf->_propInfo._moreFont = tf;
		ConfMan.set("morefont", Screen::getFontName(tf), _domain);
	}


	if (g_vm) {
		if (g_vm->_screen)
			g_vm->_screen->initialize();

		if (g_vm->_windows) {
			g_vm->_windows->rearrange();

			for (Glk::Windows::iterator it = g_vm->_windows->begin(); it != g_vm->_windows->end(); ++it) {
				Glk::Window *w = *it;
				if (auto tb = dynamic_cast<Glk::TextBufferWindow *>(w))
					tb->refreshStyles();
				else if (auto tg = dynamic_cast<Glk::TextGridWindow *>(w))
					tg->refreshStyles();
			}
			g_vm->_windows->_forceRedraw = true;
		}

		if (g_vm->_events)
			g_vm->_events->redraw();
	}

	if (tempConfCreated)
		delete g_conf;

	return true;
}

static void setManualColorHex(GUI::PopUpWidget *popup, GUI::EditTextWidget *hexInput) {
	int idx = popup->getSelectedTag();

	if (idx < 0 || idx > 5)
		return;

	byte r = (GLK_COLORS[idx].rgb >> 16) & 0xFF;
	byte g = (GLK_COLORS[idx].rgb >> 8)  & 0xFF;
	byte b = (GLK_COLORS[idx].rgb		& 0xFF);
	hexInput->setEditString(Common::U32String(Common::String::format("%02X%02X%02X", r, g, b)));
}

static void setColorPopupFromHex(GUI::PopUpWidget *popup, GUI::EditTextWidget *hexInput) {
	Common::String hex = hexInput->getEditString().encode();
	if (hex.size() != 6)
		return;

	unsigned int rgb;
	if (sscanf(hex.c_str(), "%x", &rgb) != 1)
		return;

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
	// Update the hex field to match the color chosen from the color popup
	if (cmd == kTColorChangedCmd) {
		setManualColorHex(_tcolorPopUps[0], _manualTColorHexInput);
		save();
		return;
	} else if (cmd == kGColorChangedCmd) {
		setManualColorHex(_gcolorPopUps[0], _manualGColorHexInput);
		save();
		return;
	} else if (cmd == kWColorChangedCmd) {
		setManualColorHex(_wcolorPopUps[0], _manualWColorHexInput);
		save();
		return;
	} else if (cmd == kBColorChangedCmd) {
		setManualColorHex(_bcolorPopUps[0], _manualBColorHexInput);
		save();
		return;
	} else if (cmd == kCColorChangedCmd) {
		setManualColorHex(_ccolorPopUps[0], _manualCColorHexInput);
		save();
		return;
	} else if (cmd == kLColorChangedCmd) {
		setManualColorHex(_lcolorPopUps[0], _manualLColorHexInput);
		save();
		return;
	} else if (cmd == kMColorChangedCmd) {
		setManualColorHex(_mcolorPopUps[0], _manualMColorHexInput);
		save();
		return;
	} else if (cmd == kTHexChangedCmd) { // hex input → popup sync
		setColorPopupFromHex(_tcolorPopUps[0], _manualTColorHexInput);
		save();
		return;
	} else if (cmd == kGHexChangedCmd) {
		setColorPopupFromHex(_gcolorPopUps[0], _manualGColorHexInput);
		save();
		return;
	} else if (cmd == kWHexChangedCmd) {
		setColorPopupFromHex(_wcolorPopUps[0], _manualWColorHexInput);
		save();
		return;
	} else if (cmd == kBHexChangedCmd) {
		setColorPopupFromHex(_bcolorPopUps[0], _manualBColorHexInput);
		save();
		return;
	} else if (cmd == kCHexChangedCmd) {
		setColorPopupFromHex(_ccolorPopUps[0], _manualCColorHexInput);
		save();
		return;
	} else if (cmd == kLHexChangedCmd) {
		setColorPopupFromHex(_lcolorPopUps[0], _manualLColorHexInput);
		save();
		return;
	} else if (cmd == kMHexChangedCmd) {
		setColorPopupFromHex(_mcolorPopUps[0], _manualMColorHexInput);
		save();
		return;
	} else if (cmd == kMonoSizeCmd) {
		_monosizeVal->setValue(_monosize->getValue());
		_monosizeVal->markAsDirty();
		return;
	} else if (cmd == kPropSizeCmd) {
		_propsizeVal->setValue(_propsize->getValue());
		_propsizeVal->markAsDirty();
		return;
	}

	OptionsContainerWidget::handleCommand(sender, cmd, data);
}

} // End of namespace Glk
