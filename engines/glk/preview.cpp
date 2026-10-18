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

#include "glk/preview.h"
#include "glk/screen.h"
#include "common/fs.h"
#include "common/system.h"
#include "common/translation.h"
#include "gui/dialog.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/widgets/popup.h"
#include "gui/widgets/scrollcontainer.h"
#include "graphics/managed_surface.h"

namespace Glk {

struct PreviewSample {
	int fontIndex;
	bool grid;
	int style;
	const char *text;
	bool link;
};

static const PreviewSample PREVIEW_SAMPLES[] = {
	{ 0, false, style_Header, _s("A representative heading"), false },
	{ 1, false, style_Normal, _s("A longer prose paragraph wraps across lines so that font size, "
		"line spacing, margins, and reading width are visible together."), false },
	{ 2, false, style_Emphasized, _s("Emphasized text keeps a visibly distinct face."), false },
	{ 3, false, style_Input, _s("> TAKE LAMP"), false },
	{ 1, false, style_Normal, _s("Clickable example"), true },
	{ 4, true, style_Normal, _s("STATUS  SCORE  42\n+---+  FIXED-WIDTH MAP  +---+"), false }
};

static const struct {
	bool grid;
	int style;
} PREVIEW_FONTS[] = {
	{ false, style_Header }, { false, style_Normal },
	{ false, style_Emphasized }, { false, style_Input },
	{ true, style_Normal }
};

static void previewFontPaths(const GlkOptionsState &settings, Common::String (&paths)[3]) {
	paths[0] = settings.getString("path", "");
	paths[1] = settings.getString("extrapath", "");
	paths[2] = ConfMan.getDomain(Common::ConfigManager::kApplicationDomain)->getValOrDefault("extrapath");
}

bool glkPreviewFontInputsChanged(const GlkOptionsState &before, const Conf &oldSettings,
		const GlkOptionsState &after, const Conf &newSettings) {
	if (oldSettings._monoInfo._size != newSettings._monoInfo._size ||
			oldSettings._propInfo._size != newSettings._propInfo._size)
		return true;
	for (uint i = 0; i < ARRAYSIZE(PREVIEW_FONTS); ++i) {
		if (before.getStyleFont(PREVIEW_FONTS[i].grid, PREVIEW_FONTS[i].style) !=
				after.getStyleFont(PREVIEW_FONTS[i].grid, PREVIEW_FONTS[i].style))
			return true;
	}
	Common::String oldPaths[3], newPaths[3];
	previewFontPaths(before, oldPaths);
	previewFontPaths(after, newPaths);
	for (uint i = 0; i < ARRAYSIZE(oldPaths); ++i)
		if (oldPaths[i] != newPaths[i])
			return true;
	return false;
}

static Common::Archive *openPreviewFonts(const GlkOptionsState &settings) {
	Common::SearchSet resources;
	Common::String paths[3];
	previewFontPaths(settings, paths);
	for (uint i = 0; i < ARRAYSIZE(paths); ++i) {
		if (paths[i].empty())
			continue;
		const Common::FSNode directory(Common::Path::fromConfig(paths[i]));
		if (directory.exists() && directory.isDirectory() &&
				!resources.hasArchive(paths[i]))
			resources.addDirectory(paths[i], directory, 0, i == 0 ? 4 : 1);
	}
	// Borrow launcher/system resources without registering target paths globally.
	resources.add("launcher-resources", &SearchMan, -1, false);
	return Screen::openFontArchive(&resources);
}

GlkPreviewFonts::GlkPreviewFonts() {
	for (uint i = 0; i < ARRAYSIZE(_fonts); ++i)
		_fonts[i] = nullptr;
}

GlkPreviewFonts::~GlkPreviewFonts() {
	clear();
}

void GlkPreviewFonts::clear() {
	for (uint i = 0; i < ARRAYSIZE(_fonts); ++i) {
		delete _fonts[i];
		_fonts[i] = nullptr;
	}
}

bool GlkPreviewFonts::fitsSurface(int width, int height, const Graphics::PixelFormat &format) {
	return width > 0 && height > 0 && width <= 32767 && height <= 32767 &&
		format.bytesPerPixel >= 2 && format.bytesPerPixel <= 4 &&
		format.rBits() && format.gBits() && format.bBits() &&
		(uint32)width * height <= (2U * 1024 * 1024) / format.bytesPerPixel;
}

bool GlkPreviewFonts::load(const GlkOptionsState &settings, const Conf &resolved, int scale) {
	clear();
	if (scale != 100 && scale != 150 && scale != 200)
		return false;
	const double mono = resolved._monoInfo._size * scale / 100.0;
	const double prop = resolved._propInfo._size * scale / 100.0;
	if (!(mono >= 1 && mono <= 64 && prop >= 1 && prop <= 64))
		return false;
	Common::Archive *archive = openPreviewFonts(settings);
	if (!archive)
		return false;
	bool valid = true;
	for (uint i = 0; i < ARRAYSIZE(_fonts); ++i) {
		const FACES face = settings.getStyleFont(PREVIEW_FONTS[i].grid, PREVIEW_FONTS[i].style);
		_fonts[i] = Screen::loadFontFromArchive(face, archive, face >= PROPR ? prop : mono);
		if (!_fonts[i]) {
			valid = false;
			break;
		}
	}
	delete archive;
	if (!valid) {
		clear();
		return false;
	}
	for (uint i = 0; i < ARRAYSIZE(_fonts); ++i) {
		_metrics[i] = i == 4 ? static_cast<const FontInfo &>(resolved._monoInfo) : static_cast<const FontInfo &>(resolved._propInfo);
		_metrics[i]._leading = _metrics[i]._leading * scale / 100;
		_metrics[i]._baseLine = _metrics[i]._baseLine * scale / 100;
		Screen::measureFont(_metrics[i], *_fonts[i], *_fonts[4], i == 4 ? 0 : resolved._propInfo._lineSeparation * scale / 100);
	}
	return true;
}

bool GlkPreviewLayout::measure() {
	lineHeight = MAX(g_gui.getFontHeight(), g_gui.getFontHeight(GUI::ThemeEngine::kFontStyleBold));
	GUI::ThemeEval &theme = *g_gui.xmlEval();
	popupHeight = MAX(lineHeight + 2, theme.getVar("Globals.PopUp.Height", lineHeight + 2));
	buttonHeight = MAX(lineHeight + 2, theme.getVar("Globals.Button.Height", lineHeight + 2));
	spacing = MAX(2, lineHeight / 4);
	// Theme padding is scaled by the layout engine; all other dimensions here
	// are already measured display pixels.
	padding = MAX(1, (int)(4 * g_gui.getScaleFactor()));
	popupWidth = g_gui.getStringWidth("200%") + popupHeight +
		theme.getVar("Globals.PopUpWidget.Padding.Left", 0) +
		theme.getVar("Globals.PopUpWidget.Padding.Right", 0);
	closeWidth = MAX(g_gui.getStringWidth(_("Close")) + spacing * 4,
		theme.getVar("Globals.Button.Width", 0));
	const int minimumWidth = MAX(closeWidth, MAX(
		g_gui.getStringWidth(_("Preview scale:")) + popupWidth + spacing,
		g_gui.getStringWidth(_("Text preview"), GUI::ThemeEngine::kFontStyleBold)));
	const int chromeHeight = padding * 2 + lineHeight + popupHeight + buttonHeight + spacing * 3;
	const Common::Rect safe = g_system->getSafeOverlayArea();
	const int availableWidth = safe.width() - lineHeight * 2 - padding * 2;
	const int availableHeight = safe.height() - lineHeight * 2 - chromeHeight;
	const Graphics::PixelFormat &format = g_gui.theme()->getPixelFormat();
	const int minimumHeight = lineHeight * 4;
	const uint32 pixels = format.bytesPerPixel ? (2U * 1024 * 1024) / format.bytesPerPixel : 0;
	// Prefer reading width, reserving enough surface budget for four GUI lines.
	const int candidateWidth = MIN(MIN(availableWidth, 32767), (int)(pixels / minimumHeight));
	const int candidateHeight = candidateWidth > 0 ?
		MIN(MIN(availableHeight, 32767), (int)(pixels / candidateWidth)) : 0;
	const bool available = candidateWidth >= minimumWidth && candidateHeight >= minimumHeight &&
		GlkPreviewFonts::fitsSurface(candidateWidth, candidateHeight, format);
	// Keep Close in the ordinary dialog layout even when no sample can be drawn.
	contentWidth = available ? candidateWidth : MAX(1, availableWidth);
	contentHeight = available ? candidateHeight : MAX(1, availableHeight);
	width = contentWidth + padding * 2;
	height = contentHeight + chromeHeight;
	return available;
}

bool glkPreviewFontsAvailable(const GlkOptionsState &settings, const Conf &resolved) {
	GlkPreviewFonts fonts;
	return fonts.load(settings, resolved, 100);
}

class GlkPreviewWidget : public GUI::Widget {
private:
	const GlkOptionsState *_settings;
	const Conf &_resolved;
	Graphics::ManagedSurface _surface;
	GlkPreviewFonts _resources;
	GUI::ScrollContainerWidget *_viewport;
	int _scale;
	int _sampleHeight;
	int _requiredHeight;
	bool _layoutAvailable;

	uint getColor(const Common::String &value) const {
		const uint rgb = strtol(value.c_str(), nullptr, 16);
		return _surface.format.RGBToColor((rgb >> 16) & 0xff,
			(rgb >> 8) & 0xff, rgb & 0xff);
	}

	int drawWrapped(int fontIndex, bool grid, int style,
			const Common::U32String &text, int y, bool underline,
			uint32 linkColor, int linkStyle) {
		const Graphics::Font *font = _resources.font(fontIndex);

		Common::Array<Common::U32String> lines;
		const int padding = 8;
		font->wordWrapText(text, MAX(1, (int)_surface.w - padding * 2),
			lines);
		const uint foreground = underline ? linkColor :
			getColor(_settings->getStyleForeground(grid, style));
		const uint background = getColor(
			_settings->getStyleBackground(grid, style));
		const int lineHeight = _resources.lineHeight(fontIndex);
		for (uint line = 0; line < lines.size(); ++line) {
			if (y + lineHeight < 0 || y >= _surface.h) {
				y += lineHeight;
				continue;
			}
			_surface.fillRect(Common::Rect(padding, y,
				_surface.w - padding, y + lineHeight), background);
			font->drawString(&_surface, lines[line], padding, y,
				_surface.w - padding * 2, foreground);
			if (underline && linkStyle > 0) {
				const int width = MIN(font->getStringWidth(lines[line]),
					(int)_surface.w - padding * 2);
				_surface.hLine(padding, y + font->getFontHeight(),
					padding + width, foreground);
				if (linkStyle > 1)
					_surface.hLine(padding, y + font->getFontHeight() + 1,
						padding + width, foreground);
			}
			y += lineHeight;
		}
		return y + 3;
	}

	void drawGuiText(const Common::U32String &text, int y) {
		if (text.empty())
			return;
		const Graphics::Font &font = g_gui.getFont(GUI::ThemeEngine::kFontStyleNormal);
		Common::Array<Common::U32String> lines;
		font.wordWrapText(text, MAX(1, (int)_w - 32), lines);
		const int lineHeight = font.getFontHeight();
		const Graphics::TextAlign align = Graphics::convertTextAlignH(Graphics::kTextAlignStart, g_gui.useRTL());
		for (uint line = 0; line < lines.size(); ++line) {
			g_gui.theme()->drawText(Common::Rect(_x + 16, y, _x + _w - 16, y + lineHeight),
				lines[line], GUI::ThemeEngine::kStateEnabled, align,
				GUI::ThemeEngine::kTextInversionNone, 0, false, GUI::ThemeEngine::kFontStyleNormal);
			y += lineHeight;
		}
	}

	int warningSpacing() const {
		return MAX(2, g_gui.getFontHeight(GUI::ThemeEngine::kFontStyleNormal) / 2);
	}

	Common::U32String contrastWarning() const {
		if (_settings->hasLowContrast(false, style_Normal))
			return _("Low contrast: prose text and background");
		if (_settings->hasLowContrast(true, style_Normal))
			return _("Low contrast: fixed-width text and background");
		return Common::U32String();
	}

	int measureContentHeight(int width) {
		const int padding = 8;
		const int textWidth = MAX(1, width - padding * 2);
		int height = 6;
		for (uint i = 0; i < ARRAYSIZE(PREVIEW_SAMPLES); ++i) {
			const PreviewSample &sample = PREVIEW_SAMPLES[i];
			Common::Array<Common::U32String> lines;
			_resources.font(sample.fontIndex)->wordWrapText(_(sample.text), textWidth, lines);
			height += lines.size() * _resources.lineHeight(sample.fontIndex) + 3;
		}
		return height + 6;
	}

	void rebuild(int visibleHeight, int offset) {
		const int framePadding = 8;
		const Graphics::PixelFormat &format = g_gui.theme()->getPixelFormat();
		const int width = _w - framePadding * 2;
		if (!GlkPreviewFonts::fitsSurface(width, visibleHeight, format)) {
			_surface.free();
			return;
		}
		if (_surface.w != width || _surface.h != visibleHeight || _surface.format != format)
			_surface.create(width, visibleHeight, format);
		_surface.fillRect(Common::Rect(0, 0, _surface.w, _surface.h),
			getColor(_settings->getStyleBackground(false, style_Normal)));

		const uint32 linkColor = getColor(_settings->getColor("linkcolor",
			_resolved._propInfo._linkColor));
		const int linkStyle = _settings->getInt("linkstyle",
			_resolved._propInfo._linkStyle);
		int y = 6 - offset;
		for (uint i = 0; i < ARRAYSIZE(PREVIEW_SAMPLES); ++i) {
			const PreviewSample &sample = PREVIEW_SAMPLES[i];
			y = drawWrapped(sample.fontIndex, sample.grid, sample.style, _(sample.text), y,
				sample.link, linkColor, linkStyle);
		}
	}

protected:
	void reflowLayout() override {
		GUI::Widget::reflowLayout();
		measureRequiredHeight(_w);
	}

	void drawWidget() override {
		const int top = MAX((int)_y + 8, (int)_viewport->getAbsY());
		const int sampleBottom = _y + 8 + _sampleHeight;
		const int bottom = MIN(sampleBottom, (int)_viewport->getAbsY() + _viewport->getHeight());
		if (!_layoutAvailable || !_resources.font(0)) {
			_surface.free();
			drawGuiText(_layoutAvailable ? _("Preview is unavailable with these fonts or font sizes.") :
				_("Preview is unavailable with these display settings."), _y + 14);
		} else {
			if (bottom > top) {
				rebuild(bottom - top, top - _y - 8);
				if (!_surface.empty())
					g_gui.theme()->drawManagedSurface(Common::Point(_x + 8, top), _surface, Graphics::ALPHA_OPAQUE);
			}
			drawGuiText(contrastWarning(), sampleBottom + warningSpacing());
		}
	}

public:
	GlkPreviewWidget(GUI::ScrollContainerWidget *boss, const Common::String &name,
			const GlkOptionsState *settings, const Conf &resolved)
		: GUI::Widget(boss, name), _settings(settings), _resolved(resolved), _viewport(boss), _scale(100),
		  _sampleHeight(0), _requiredHeight(0), _layoutAvailable(false) {
		setFlags(GUI::WIDGET_ENABLED | GUI::WIDGET_CLEARBG);
	}

	void setLayoutAvailable(bool available) {
		_layoutAvailable = available;
		if (!available)
			_surface.free();
	}

	bool prepareFonts() {
		return _resources.load(*_settings, _resolved, _scale);
	}

	bool setScale(int scale) {
		if (_scale == scale)
			return false;
		_scale = scale;
		prepareFonts();
		markAsDirty();
		return true;
	}

	int measureRequiredHeight(int width) {
		const int framePadding = 8;
		const Graphics::Font &font = g_gui.getFont(GUI::ThemeEngine::kFontStyleNormal);
		Common::Array<Common::U32String> lines;
		if (!_layoutAvailable || !_resources.font(0)) {
			_sampleHeight = 0;
			font.wordWrapText(_layoutAvailable ? _("Preview is unavailable with these fonts or font sizes.") :
				_("Preview is unavailable with these display settings."), MAX(1, width - framePadding * 4), lines);
			_requiredHeight = framePadding * 2 + 12 + lines.size() * font.getFontHeight();
			return _requiredHeight;
		}
		_sampleHeight = measureContentHeight(width - framePadding * 2);
		_requiredHeight = _sampleHeight + framePadding * 2;
		const Common::U32String warning = contrastWarning();
		if (!warning.empty()) {
			font.wordWrapText(warning, MAX(1, width - framePadding * 4), lines);
			_requiredHeight += warningSpacing() + lines.size() * font.getFontHeight();
		}
		return _requiredHeight;
	}

	void getMinSize(int &minWidth, int &minHeight) override {
		minWidth = -1;
		minHeight = _requiredHeight;
	}
};

class GlkPreviewDialog : public GUI::Dialog {
private:
	enum {
		kScaleChangedCmd = 'GPSC'
	};
	GUI::PopUpWidget *_scale;
	GUI::ScrollContainerWidget *_content;
	GlkPreviewWidget *_sample;

	void defineContentLayout(int height) {
		g_gui.xmlEval()->addDialog("GlkPreviewContent",
			"GlkPreviewDialog.Content")
			.addPadding(0, 0, 0, 0)
			.addLayout(GUI::ThemeLayout::kLayoutVertical, 0)
				.addPadding(0, 0, 0, 0)
				.addWidget("Sample", "", -1, height)
			.closeLayout()
			.closeDialog();
	}

	void defineLayout(const GlkPreviewLayout &layout) {
		g_gui.xmlEval()->setVar("Dialog.GlkPreviewDialog.Shading", GUI::ThemeEngine::kShadingDim);
		g_gui.xmlEval()->addDialog("GlkPreviewDialog", "screen_center",
			layout.width, layout.height, 0)
			.addPadding(0, 0, 0, 0)
			.addLayout(GUI::ThemeLayout::kLayoutVertical, layout.spacing)
				.addPadding(4, 4, 4, 4)
				.addWidget("Heading", "", layout.contentWidth, layout.lineHeight)
				.addLayout(GUI::ThemeLayout::kLayoutHorizontal, layout.spacing, GUI::ThemeLayout::kItemAlignCenter)
					.addPadding(0, 0, 0, 0)
					.addWidget("ScaleLabel", "", layout.contentWidth - layout.popupWidth - layout.spacing, layout.lineHeight)
					.addWidget("Scale", "", layout.popupWidth, layout.popupHeight)
				.closeLayout()
				.addWidget("Content", "ScrollContainerWidget", layout.contentWidth, layout.contentHeight)
				.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 0)
					.addPadding(0, 0, 0, 0)
					.addSpace()
					.addWidget("Close", "", layout.closeWidth, layout.buttonHeight)
				.closeLayout()
			.closeLayout()
			.closeDialog();
	}

protected:
	void reflowLayout() override {
		GlkPreviewLayout layout;
		_sample->setLayoutAvailable(layout.measure());
		const int scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);
		const int width = MAX(1, layout.contentWidth - scrollbarWidth);
		const int height = _sample->measureRequiredHeight(width);
		// A theme refresh discards both dynamic layouts. Recreate them before
		// any widget lookup, even when the measured dimensions are unchanged.
		defineLayout(layout);
		defineContentLayout(height);
		GUI::Dialog::reflowLayout();
	}

public:
	void open() override {
		GUI::Dialog::open();
		// Use the same shaded stack on opening as after popup dismissal.
		// The usual nested-dialog fast path preserves the parent's shading.
		g_gui.redrawFull();
	}

	GlkPreviewDialog(const GlkOptionsState *settings, const Conf &resolved)
		: GUI::Dialog("GlkPreviewDialog"), _content(nullptr),
		  _sample(nullptr) {
		new GUI::StaticTextWidget(this, "GlkPreviewDialog.Heading",
			_("Text preview"), Common::U32String(),
			GUI::ThemeEngine::kFontStyleBold);
		new GUI::StaticTextWidget(this, "GlkPreviewDialog.ScaleLabel",
			_("Preview scale:"));
		_scale = new GUI::PopUpWidget(this, "GlkPreviewDialog.Scale",
			_("Scale used only inside this preview"), kScaleChangedCmd);
		_scale->appendEntry(_("100%"), 100);
		_scale->appendEntry(_("150%"), 150);
		_scale->appendEntry(_("200%"), 200);
		_scale->setSelectedTag(100);
		_content = new GUI::ScrollContainerWidget(
			this, "GlkPreviewDialog.Content", "GlkPreviewContent");
		_content->setBackgroundType(GUI::ThemeEngine::kWidgetBackgroundNo);
		_sample = new GlkPreviewWidget(_content, "GlkPreviewContent.Sample",
			settings, resolved);
		new GUI::ButtonWidget(this, "GlkPreviewDialog.Close", _("Close"),
			Common::U32String(), GUI::kCloseCmd);
	}

	bool prepareFonts() {
		return _sample->prepareFonts();
	}

	void handleCommand(GUI::CommandSender *sender, uint32 cmd,
			uint32 data) override {
		if (cmd == kScaleChangedCmd) {
			const bool scrolled = _content->getMaximumScrollPosition() > 0;
			if (!_sample->setScale(_scale->getSelectedTag()))
				return;
			reflowLayout();
			// Reset scrolling and redraw the dialog background, including the
			// area vacated by a smaller sample or an unavailable-state message.
			_content->setScrollPosition(0);
			// Scrollbar bases are drawn into the GUI backbuffer. A theme's
			// dialog background need not be opaque, so removing the scrollbar
			// also needs the existing full background reconstruction.
			if (scrolled && !_content->getMaximumScrollPosition())
				g_gui.scheduleFullRedraw();
			return;
		}
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
};

bool showGlkPreview(const GlkOptionsState &settings, const Conf &resolved) {
	// The draft and its resolved preferences remain unchanged during the modal.
	GlkPreviewDialog dialog(&settings, resolved);
	if (!dialog.prepareFonts())
		return false;
	dialog.runModal();
	return true;
}

} // End of namespace Glk
