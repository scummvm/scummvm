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

#define NEW_EDIT(widget, name) \
    widget = new GUI::EditTextWidget(widgetsBoss(), name, Common::U32String())

#define NEW_CHECKBOX(widget, name) \
    widget = new GUI::CheckboxWidget(widgetsBoss(), name, Common::U32String())

#define NEW_POPUP(widget, name) \
    widget = new GUI::PopUpWidget(widgetsBoss(), name)

#define NEW_EDIT_CMD(widget, name, cmd) \
    widget = new GUI::EditTextWidget(widgetsBoss(), name, Common::U32String(), Common::U32String(), cmd, cmd)

#define NEW_POPUP_CMD(widget, name, cmd) \
    widget = new GUI::PopUpWidget(widgetsBoss(), name, Common::U32String(), cmd)

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

#define INIT_FLOAT(widget, configMember) \
    if (g_conf && widget) \
        widget->setEditString(Common::U32String(Common::String::format("%g", configMember)))

namespace Glk {

struct GlkColor {
    const char *name;
    unsigned int rgb;
};

static const GlkColor GLK_COLORS[] = {
    { "white", 0xFFFFFF },
    { "green", 0x00FF00 },
    { "red",   0xFF0000 },
    { "blue",  0x0000FF },
    { "black", 0x000000 },
    { "grey",  0x808080 }
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
    WindowStyle *styles;      // for t/g/w/b
    PropFontInfo *propInfo;   // for caret/link/more
    const char *confKey;
    bool isProp;              // true for caret/link/more
};

struct FontOption {
    GUI::PopUpWidget *popup;
    WindowStyle *styles;
    const char *confKeyPrefix;
};

static GUI::StaticTextWidget *createHeading(GUI::GuiObject *boss, const char *id, const char *text) {
    auto h = new GUI::StaticTextWidget(boss, id, _(text));
    h->setAlign(Graphics::kTextAlignCenter);
    return h;
}

GlkOptionsWidget::GlkOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain)
    : GUI::OptionsContainerWidget(boss, name, "GlkOptionsDialog", domain) {

    static const char *fontLabels[] = { "Monospace Regular", "Monospace Bold", "Monospace Italic", "Monospace Bold Italic", "Proportional Regular", "Proportional Bold", "Proportional Italic", "Proportional Bold Italic" };

    for (int f = MONOR; f <= PROPZ; ++f) {
        _tfontPopUps[f] = nullptr;
        _gfontPopUps[f] = nullptr;
    }

    _fontHeadinglbl = createHeading(widgetsBoss(), "GlkOptionsDialog.FontHeading", "Font");

    NEW_POPUP(_tfontPopUps[0], "GlkOptionsDialog.TFont0");
    NEW_LABEL("GlkOptionsDialog.TFont", "Text:", "");
    for (int f = MONOR; f <= PROPZ; ++f)
        _tfontPopUps[0]->appendEntry(fontLabels[f], f);

    NEW_POPUP(_gfontPopUps[0], "GlkOptionsDialog.GFont0");
    NEW_LABEL("GlkOptionsDialog.GFont", "Grid:", "Font for drawn graphics like maps, diagrams, puzzles, etc");
    for (int f = MONOR; f <= PROPZ; ++f)
        _gfontPopUps[0]->appendEntry(fontLabels[f], f);

    _colorHeadinglbl = createHeading(widgetsBoss(), "GlkOptionsDialog.ColorHeading", "Color");

    NEW_LABEL("GlkOptionsDialog.THexPrefix", "0x", "");
    NEW_EDIT_CMD(_manualTColorHexInput, "GlkOptionsDialog.THex", kTHexChangedCmd);
    NEW_POPUP_CMD(_tcolorPopUps[0], "GlkOptionsDialog.TColor0", kTColorChangedCmd);
    NEW_LABEL("GlkOptionsDialog.TColor", "Text:", "Color of the interactive text");
    for (int c = 0; c <= 5; ++c)
        _tcolorPopUps[0]->appendEntry(_(GLK_COLORS[c].name), c);
    _tcolorPopUps[0]->appendEntry("Custom", 6);

    NEW_LABEL("GlkOptionsDialog.GHexPrefix", "0x", "");
    NEW_EDIT_CMD(_manualGColorHexInput, "GlkOptionsDialog.GHex", kGHexChangedCmd);
    NEW_POPUP_CMD(_gcolorPopUps[0], "GlkOptionsDialog.GColor0", kGColorChangedCmd);
    NEW_LABEL("GlkOptionsDialog.GColor", "Grid:", "Color for drawn graphics such as maps, diagrams, puzzles, etc");
    for (int c = 0; c <= 5; ++c)
        _gcolorPopUps[0]->appendEntry(_(GLK_COLORS[c].name), c);
    _gcolorPopUps[0]->appendEntry("Custom", 6);

    NEW_LABEL("GlkOptionsDialog.WHexPrefix", "0x", "");
    NEW_EDIT_CMD(_manualWColorHexInput, "GlkOptionsDialog.WHex", kWHexChangedCmd);
    NEW_POPUP_CMD(_wcolorPopUps[0], "GlkOptionsDialog.WColor0", kWColorChangedCmd);
    NEW_LABEL("GlkOptionsDialog.WColor", "Window:", "Color of the window(background)");
    _wcolorPopUps[0]->appendEntry("Game default", 7);
    for (int c = 0; c <= 5; ++c)
        _wcolorPopUps[0]->appendEntry(_(GLK_COLORS[c].name), c);
    _wcolorPopUps[0]->appendEntry("Custom", 6);

    NEW_LABEL("GlkOptionsDialog.BHexPrefix", "0x", "");
    NEW_EDIT_CMD(_manualBColorHexInput, "GlkOptionsDialog.BHex", kBHexChangedCmd);
    NEW_POPUP_CMD(_bcolorPopUps[0], "GlkOptionsDialog.BColor0", kBColorChangedCmd);
    NEW_LABEL("GlkOptionsDialog.BColor", "Border:", "Color of the window border");
    _bcolorPopUps[0]->appendEntry("Game default", 7);
    for (int c = 0; c <= 5; ++c)
        _bcolorPopUps[0]->appendEntry(_(GLK_COLORS[c].name), c);
    _bcolorPopUps[0]->appendEntry("Custom", 6);

    _borderHeadinglbl = createHeading(widgetsBoss(), "GlkOptionsDialog.wborderlbl", "Border");

    NEW_LABEL("GlkOptionsDialog.wborderx", "Horizontal:", "");
    NEW_EDIT(_wborderx, "GlkOptionsDialog.wborderhorizontal");
    NEW_LABEL("GlkOptionsDialog.wbordery", "Vertical:", "");
    NEW_EDIT(_wbordery, "GlkOptionsDialog.wbordervertical");

    NEW_LABEL("GlkOptionsDialog.CHexPrefix", "0x", "");
    NEW_EDIT_CMD(_manualCColorHexInput, "GlkOptionsDialog.CHex", kCHexChangedCmd);
    NEW_POPUP_CMD(_ccolorPopUps[0], "GlkOptionsDialog.CColor0", kCColorChangedCmd);
    NEW_LABEL("GlkOptionsDialog.CColor", "Caret:", "Color of the cursor.");
    _ccolorPopUps[0]->appendEntry("Game default", 7);
    for (int c = 0; c <= 5; ++c)
        _ccolorPopUps[0]->appendEntry(_(GLK_COLORS[c].name), c);
    _ccolorPopUps[0]->appendEntry("Custom", 6);

    NEW_LABEL("GlkOptionsDialog.LHexPrefix", "0x", "");
    NEW_EDIT_CMD(_manualLColorHexInput, "GlkOptionsDialog.LHex", kLHexChangedCmd);
    NEW_POPUP_CMD(_lcolorPopUps[0], "GlkOptionsDialog.LColor0", kLColorChangedCmd);
    NEW_LABEL("GlkOptionsDialog.LColor", "Link:", "Color for URLs if they appear in-game");
    _lcolorPopUps[0]->appendEntry("Game default", 7);
    for (int c = 0; c <= 5; ++c)
        _lcolorPopUps[0]->appendEntry(_(GLK_COLORS[c].name), c);
    _lcolorPopUps[0]->appendEntry("Custom", 6);

    NEW_LABEL("GlkOptionsDialog.MHexPrefix", "0x", "");
    NEW_EDIT_CMD(_manualMColorHexInput, "GlkOptionsDialog.MHex", kMHexChangedCmd);
    NEW_POPUP_CMD(_mcolorPopUps[0], "GlkOptionsDialog.MColor0", kMColorChangedCmd);
    NEW_LABEL("GlkOptionsDialog.MColor", "More:", "Color for the \"More...\" markers in the text");
    _mcolorPopUps[0]->appendEntry("Game default", 7);
    for (int c = 0; c <= 5; ++c)
        _mcolorPopUps[0]->appendEntry(_(GLK_COLORS[c].name), c);
    _mcolorPopUps[0]->appendEntry("Custom", 6);

    _typographyHeadinglbl = createHeading(widgetsBoss(), "GlkOptionsDialog.topographyheadinglbl", "Typography");

    NEW_LABEL("GlkOptionsDialog.linkstyle", "Link style:", "Style for URLs if they appear in-game");
    NEW_EDIT(_linkStyle, "GlkOptionsDialog.linkStyle");
    NEW_LABEL("GlkOptionsDialog.caretshape", "Caret shape:", "Shape of the cursor");
    NEW_EDIT(_caretShape, "GlkOptionsDialog.caretShape");
    NEW_LABEL("GlkOptionsDialog.moreprompt", "More prompt:", "Custom marker in place of the \"More...\" marker");
    NEW_EDIT(_morePrompt, "GlkOptionsDialog.morePrompt");

    NEW_LABEL("GlkOptionsDialog.stylehintlabel", "Style hints:", "Let the game suggest text styling options");
    NEW_CHECKBOX(_styleHint, "GlkOptionsDialog.stylehint");
    NEW_LABEL("GlkOptionsDialog.safeclickslabel", "Safe clicks:", "Safely apply clicks while input is pending.");
    NEW_CHECKBOX(_safeClicks, "GlkOptionsDialog.safeclicks");

    NEW_LABEL("GlkOptionsDialog.colslbl", "Column count:", "Number of columns.");
    NEW_EDIT(_cols, "GlkOptionsDialog.cols");
    NEW_LABEL("GlkOptionsDialog.rowslbl", "Rows count:", "Number of rows.");
    NEW_EDIT(_rows, "GlkOptionsDialog.rows");

    NEW_LABEL("GlkOptionsDialog.lockcolslbl", "Lock columns:", "Set it to 1 to manually change the column count.");
    NEW_CHECKBOX(_lockcols, "GlkOptionsDialog.lockcols");
    NEW_LABEL("GlkOptionsDialog.lockrowslbl", "Lock rows:", "Set it to 1 to manually change the row count.");
    NEW_CHECKBOX(_lockrows, "GlkOptionsDialog.lockrows");

    NEW_LABEL("GlkOptionsDialog.justifylbl", "Justify:", "Enable text justification.");
    NEW_CHECKBOX(_justify, "GlkOptionsDialog.justify");

    NEW_LABEL("GlkOptionsDialog.quoteslbl", "Typographic quotes:", "Choose typographic quotes.");
    NEW_POPUP(_quotes, "GlkOptionsDialog.quotes");
    _quotes->appendEntry(_("Off"), 0);
    _quotes->appendEntry(_("Normal"), 1);
    _quotes->appendEntry(_("Rabid"), 2);

    NEW_LABEL("GlkOptionsDialog.capslbl", "Caps:", "Force uppercase input.");
    NEW_CHECKBOX(_caps, "GlkOptionsDialog.caps");

    NEW_LABEL("GlkOptionsDialog.dasheslbl", "Dashes:", "Types of dashes");
    NEW_POPUP(_dashes, "GlkOptionsDialog.dashes");
    _dashes->appendEntry(_("Off"), 0);
    _dashes->appendEntry(_("Em dashes"), 1);
    _dashes->appendEntry(_("En+Em dashes"), 2);

    _userExperiencelbl = createHeading(widgetsBoss(), "GlkOptionsDialog.userexplbl", "User experience");

    NEW_LABEL("GlkOptionsDialog.spaceslbl", "Spaces:", "Types of spaces");
    NEW_POPUP(_spaces, "GlkOptionsDialog.spaces");
    _spaces->appendEntry(_("Off"), 0);
    _spaces->appendEntry(_("Compress double spaces"), 1);
    _spaces->appendEntry(_("Expand single spaces"), 2);

    NEW_LABEL("GlkOptionsDialog.graphicslbl", "Graphics:", "Turn graphics on/off.");
    NEW_CHECKBOX(_graphics, "GlkOptionsDialog.graphics");

    _wmarginHeadinglbl = createHeading(widgetsBoss(), "GlkOptionsDialog.WindowMarginHeading", "Window margin");
    NEW_LABEL("GlkOptionsDialog.wmarginxlbl", "Horizontal:", "Horizontal margin");
    NEW_EDIT(_wmarginx, "GlkOptionsDialog.wmarginx");
    NEW_LABEL("GlkOptionsDialog.wmarginylbl", "Vertical:", "Vertical margin");
    NEW_EDIT(_wmarginy, "GlkOptionsDialog.wmarginy");

    _wpaddingHeadinglbl = createHeading(widgetsBoss(), "GlkOptionsDialog.wpaddinglbl", "Window padding");
    NEW_LABEL("GlkOptionsDialog.wpaddingxlbl", "Horizontal:", "Horizontal padding");
    NEW_EDIT(_wpaddingx, "GlkOptionsDialog.wpaddingx");
    NEW_LABEL("GlkOptionsDialog.wpaddingylbl", "Vertical:", "Vertical padding");
    NEW_EDIT(_wpaddingy, "GlkOptionsDialog.wpaddingy");

    _tmarginHeadinglbl = createHeading(widgetsBoss(), "GlkOptionsDialog.tmarginlbl", "Text margin");
    NEW_LABEL("GlkOptionsDialog.tmarginxlbl", "Horizontal:", "Horizontal margin for text");
    NEW_EDIT(_tmarginx, "GlkOptionsDialog.tmarginx");
    NEW_LABEL("GlkOptionsDialog.tmarginylbl", "Vertical:", "Vertical margin for text");
    NEW_EDIT(_tmarginy, "GlkOptionsDialog.tmarginy");

    NEW_LABEL("GlkOptionsDialog.leadinglbl", "Leading:", "Vertical distance between text rows");
    NEW_EDIT(_leading, "GlkOptionsDialog.leading");
    NEW_LABEL("GlkOptionsDialog.baselinelbl", "Baseline:", "Invisible horizontal line on which text sits");
    NEW_EDIT(_baseline, "GlkOptionsDialog.baseline");

    NEW_LABEL("GlkOptionsDialog.monosizelbl", "Monosize:", "Font size scaling for the monospace text font");
    NEW_EDIT(_monosize, "GlkOptionsDialog.monosize");
    NEW_LABEL("GlkOptionsDialog.propsizelbl", "Propsize:", "Font size scaling for the proportional text font");
    NEW_EDIT(_propsize, "GlkOptionsDialog.propsize");

    _moreHeadinglbl = createHeading(widgetsBoss(), "GlkOptionsDialog.morelbl", "More");
    NEW_LABEL("GlkOptionsDialog.morealignlbl", "Align:", "Alignment of the \"More...\" marker on the window");
    NEW_POPUP(_morealign, "GlkOptionsDialog.morealign");
    _morealign->appendEntry(_("Left"), 0);
    _morealign->appendEntry(_("Centre"), 1);
    _morealign->appendEntry(_("Right"), 2);

    NEW_POPUP(_morefont, "GlkOptionsDialog.morefont");
    NEW_LABEL("GlkOptionsDialog.morefontlbl", "Font:", "Font for the \"More...\" marker");
    for (int f = MONOR; f <= PROPZ; ++f)
        _morefont->appendEntry(fontLabels[f], f);

}

GlkOptionsWidget::~GlkOptionsWidget() = default;

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

    int16 headingOffset = g_gui.xmlEval()->getVar("GlkOptionsDialog.HeadingVerticalOffset", 50);
    int16 headingMinGap = g_gui.xmlEval()->getVar("GlkOptionsDialog.HeadingMinGap", 2);

    HeadingY headingYpos[] = {
        { "GlkOptionsDialog.TFont0", _fontHeadinglbl, &fontY, &fontRowY, headingOffset },
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

    if (hasConf) {
        if (!overrideEnabled) {
            popup->setSelectedTag(defaultValue);
            hexInput->setEditString(Common::U32String());
            return;
        }
        Common::String colorStr = ConfMan.get(confKey, domain);
        Common::String hexPart = colorStr.substr(0, 6);
        hexInput->setEditString(Common::U32String(hexPart));

        unsigned int rgb;
        sscanf(hexPart.c_str(), "%x", &rgb);
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
    } else {
        popup->setSelectedTag((uint32)GUI::ListWidget::getThemeColor(ConfMan.get(themeKey, domain)));
    }
}

void GlkOptionsWidget::load() {

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

    FloatOption floatOptions[] = {
        { _monosize, &g_conf->_monoInfo._size, "monosize", 0.0 },
        { _propsize, &g_conf->_propInfo._size, "propsize", 0.0 }
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

    for (auto &opt : floatOptions) {
        if (opt.widget && g_conf)
            opt.widget->setEditString(Common::U32String(Common::String::format("%g", *opt.target)));
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
    INIT_INT(_linkStyle, g_conf->_propInfo._linkStyle);
    INIT_INT(_caretShape, g_conf->_propInfo._caretShape);
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
    INIT_FLOAT(_monosize, g_conf->_monoInfo._size);    
    INIT_FLOAT(_propsize, g_conf->_propInfo._size);
    INIT_POPUP(_morealign, g_conf->_propInfo._moreAlign);
    INIT_POPUP(_morefont, g_conf->_propInfo._moreFont);
}

void GlkOptionsWidget::defineLayout(GUI::ThemeEval &layouts,
                                    const Common::String &layoutName,
                                    const Common::String &overlayedLayout) const {
    layouts.addDialog(layoutName, overlayedLayout)
        		.addLayout(GUI::ThemeLayout::kLayoutVertical)
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal, 0, GUI::ThemeLayout::kItemAlignStretch)
                        .addWidget("FontHeading", "OptionsLabel")
                    .closeLayout()
            		.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                   		.addWidget("TFont", "OptionsLabel")
                		.addWidget("TFont0", "PopUp", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("GFont", "OptionsLabel")
                        .addWidget("GFont0", "PopUp", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(8, 8, 8, 8)
                        .addWidget("ColorHeading", "OptionsLabel")
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("TColor", "OptionsLabel")
                        .addWidget("TColor0", "PopUp", 80)
                        .addWidget("THexPrefix", "OptionsLabel")
                        .addWidget("THex", "EditTextWidget")
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("GColor", "OptionsLabel")
                        .addWidget("GColor0", "PopUp", 80)
                        .addWidget("GHexPrefix", "OptionsLabel")
                        .addWidget("GHex", "EditTextWidget")
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("WColor", "OptionsLabel")
                        .addWidget("WColor0", "PopUp", 80)
                        .addWidget("WHexPrefix", "OptionsLabel")
                        .addWidget("WHex", "EditTextWidget")
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("BColor", "OptionsLabel")
                        .addWidget("BColor0", "PopUp", 80)
                        .addWidget("BHexPrefix", "OptionsLabel")
                        .addWidget("BHex", "EditTextWidget")
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("CColor", "OptionsLabel")
                        .addWidget("CColor0", "PopUp", 80)
                        .addWidget("CHexPrefix", "OptionsLabel")
                        .addWidget("CHex", "EditTextWidget")
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("LColor", "OptionsLabel")
                        .addWidget("LColor0", "PopUp", 80)
                        .addWidget("LHexPrefix", "OptionsLabel")
                        .addWidget("LHex", "EditTextWidget")
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("MColor", "OptionsLabel")
                        .addWidget("MColor0", "PopUp", 80)
                        .addWidget("MHexPrefix", "OptionsLabel")
                        .addWidget("MHex", "EditTextWidget")
                    .closeLayout()
                        .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(8, 8, 8, 8)
                        .addWidget("wborderlbl", "OptionsLabel")
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("wborderx", "OptionsLabel")
                        .addWidget("wborderhorizontal", "EditTextWidget", 80)
                        .addWidget("wbordery", "OptionsLabel")
                        .addWidget("wbordervertical", "EditTextWidget", 80)
                    .closeLayout()
                        .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(8, 8, 8, 8)
                        .addWidget("WindowMarginHeading", "OptionsLabel")
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("wmarginxlbl", "OptionsLabel")
                        .addWidget("wmarginx", "EditTextWidget", 80)
                        .addWidget("wmarginylbl", "OptionsLabel")
                        .addWidget("wmarginy", "EditTextWidget", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(8, 8, 8, 8)
                        .addWidget("tmarginlbl", "OptionsLabel")
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("tmarginxlbl", "OptionsLabel")
                        .addWidget("tmarginx", "EditTextWidget", 80)
                        .addWidget("tmarginylbl", "OptionsLabel")
                        .addWidget("tmarginy", "EditTextWidget", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(8, 8, 8, 8)
                        .addWidget("wpaddinglbl", "OptionsLabel")
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("wpaddingxlbl", "OptionsLabel")
                        .addWidget("wpaddingx", "EditTextWidget", 80)
                        .addWidget("wpaddingylbl", "OptionsLabel")
                        .addWidget("wpaddingy", "EditTextWidget", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(8, 8, 8, 8)
                        .addWidget("morelbl", "OptionsLabel")
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("morealignlbl", "OptionsLabel")
                        .addWidget("morealign", "PopUp", 80)
                        .addWidget("morefontlbl", "OptionsLabel")
                        .addWidget("morefont", "PopUp", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(8, 8, 8, 8)
                        .addWidget("topographyheadinglbl", "OptionsLabel")
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("linkstyle", "OptionsLabel")
                        .addWidget("linkStyle", "EditTextWidget", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("caretshape", "OptionsLabel")
                        .addWidget("caretShape", "EditTextWidget", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("moreprompt", "OptionsLabel")
                        .addWidget("morePrompt", "EditTextWidget", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("leadinglbl", "OptionsLabel")
                        .addWidget("leading", "EditTextWidget", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("baselinelbl", "OptionsLabel")
                        .addWidget("baseline", "EditTextWidget", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("monosizelbl", "OptionsLabel")
                        .addWidget("monosize", "EditTextWidget", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0 ,0)
                        .addWidget("propsizelbl", "OptionsLabel")
                        .addWidget("propsize", "EditTextWidget", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("capslbl", "OptionsLabel")
                        .addWidget("caps", "Checkbox", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("quoteslbl", "OptionsLabel")
                        .addWidget("quotes", "PopUp", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("dasheslbl", "OptionsLabel")
                        .addWidget("dashes", "PopUp", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("spaceslbl", "OptionsLabel")
                        .addWidget("spaces", "PopUp", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(8, 8, 8, 8)
                        .addWidget("userexplbl", "OptionsLabel")
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("stylehintlabel", "OptionsLabel")
                        .addWidget("stylehint", "Checkbox", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("safeclickslabel", "OptionsLabel")
                        .addWidget("safeclicks", "Checkbox", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("colslbl", "OptionsLabel")
                        .addWidget("cols", "EditTextWidget", 80)
                        .addWidget("lockcolslbl", "OptionsLabel")
                        .addWidget("lockcols", "Checkbox")
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("rowslbl", "OptionsLabel")
                        .addWidget("rows", "EditTextWidget", 80)
                        .addWidget("lockrowslbl", "OptionsLabel")
                        .addWidget("lockrows", "Checkbox")
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0, 0)
                        .addWidget("justifylbl", "OptionsLabel")
                        .addWidget("justify", "Checkbox", 80)
                    .closeLayout()
                    .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
                        .addPadding(0, 0, 0 ,0)
                        .addWidget("graphicslbl", "OptionsLabel")
                        .addWidget("graphics", "Checkbox", 80)
                    .closeLayout()
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

static void saveFloatField(GUI::EditTextWidget *widget, double &outValue, const char *confKey, double minValue, const Common::String &domain) {

    Common::String text = widget->getEditString().encode();
    double value = text.empty() ? minValue : atof(text.c_str());

    if (value < minValue)
        value = minValue;

    outValue = value;
    ConfMan.setFloat(confKey, value, domain);
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

bool GlkOptionsWidget::save() {

    if (!g_conf)
        return true;

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

    FloatOption floatOptions[] = {
        { _monosize, &g_conf->_monoInfo._size, "monosize", 0.0 },
        { _propsize, &g_conf->_propInfo._size, "propsize", 0.0 }
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

    for (auto &opt : floatOptions)
        if (opt.widget) saveFloatField(opt.widget, *opt.target, opt.confKey, opt.minValue, _domain);

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
    Common::String linkStr = _linkStyle->getEditString().encode();
    Common::String caretStr = _caretShape->getEditString().encode();
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

                for (int i = 0; i < style_NUMSTYLES; i++) {
                    g_conf->_windowColor = engineColor;
                    Common::String key = Common::String::format("windowcolor");
                    Common::String line = Common::String::format("%s,%s", rgbStr.c_str(), rgbStr.c_str());
                    ConfMan.set(key, line, _domain);
                }
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

                for (int i = 0; i < style_NUMSTYLES; i++) {
                    g_conf->_borderColor = engineColor;
                    Common::String key = Common::String::format("bordercolor");
                    Common::String line = Common::String::format("%s,%s", rgbStr.c_str(), rgbStr.c_str());
                    ConfMan.set(key, line, _domain);
                }
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


    int linkstyle = linkStr.empty() ? 0 : atoi(linkStr.c_str());
    if (linkstyle < 0)
        linkstyle = 0;

    g_conf->_propInfo._linkStyle = linkstyle;
    g_conf->_monoInfo._linkStyle = linkstyle;
    ConfMan.setInt("linkstyle", linkstyle, _domain);

    int caretshape = caretStr.empty() ? 0 : atoi(caretStr.c_str());
    if (caretshape < 0 || caretshape >4)
        caretshape = 0;

    g_conf->_propInfo._caretShape = caretshape;
    FontInfo::_caretShape = caretshape;
    ConfMan.setInt("caretshape", caretshape, _domain);

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


    g_vm->_screen->initialize();
    g_vm->_windows->rearrange();

    if (g_vm && g_vm->_windows) {
        for (Glk::Windows::iterator it = g_vm->_windows->begin(); it != g_vm->_windows->end(); ++it) {
            Glk::Window *w = *it;
            if (auto tb = dynamic_cast<Glk::TextBufferWindow *>(w))
                tb->refreshStyles();
            else if (auto tg = dynamic_cast<Glk::TextGridWindow *>(w))
                tg->refreshStyles();

        }
        g_vm->_windows->_forceRedraw = true;
        g_vm->_events->redraw();
    }

    return true;
}

static void setManualColorHex(GUI::PopUpWidget *popup, GUI::EditTextWidget *hexInput) {
    int idx = popup->getSelectedTag();

    byte r = (GLK_COLORS[idx].rgb >> 16) & 0xFF;
    byte g = (GLK_COLORS[idx].rgb >> 8) & 0xFF;
    byte b = GLK_COLORS[idx].rgb & 0xFF;
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
    }

    OptionsContainerWidget::handleCommand(sender, cmd, data);
}

} // End of namespace Glk
