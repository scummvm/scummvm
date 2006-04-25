/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#ifndef DISABLE_FANCY_THEMES

#include "gui/theme.h"
#include "gui/eval.h"

#include "graphics/imageman.h"
#include "graphics/imagedec.h"
#include "graphics/colormasks.h"

#include "common/config-manager.h"
#include "common/file.h"

#include "common/unzip.h"

#define kShadowTr1 16
#define kShadowTr2 32
#define kShadowTr3 64
#define kShadowTr4 128

#define THEME_VERSION 10

using Graphics::Surface;

/** Specifies the currently active 16bit pixel format, 555 or 565. */
extern int gBitFormat;

static void getColorFromConfig(const Common::ConfigFile &cfg, const Common::String &value, OverlayColor &color) {
	Common::String temp;
	if (!cfg.hasKey(value, "colors")) {
		color = g_system->RGBToColor(0, 0, 0);
		return;
	}
	cfg.getKey(value, "colors", temp);

	int rgb[3], pos = 0;
	const char *colors = temp.c_str();

	for (int cnt = 0; cnt < 3; cnt++) {
		rgb[cnt] = atoi(colors + pos);
		pos = strchr(colors + pos, ' ') - colors + 1;
	}
	color = g_system->RGBToColor(rgb[0], rgb[1], rgb[2]);
}

static void getValueFromConfig(const Common::ConfigFile &cfg, const Common::String &section, const Common::String &value, uint &val, uint defaultVal) {
	if (!cfg.hasKey(value, section)) {
		val = defaultVal;
		return;
	}
	Common::String temp;
	cfg.getKey(value, section, temp);
	val = atoi(temp.c_str());
}

static void getValueFromConfig(const Common::ConfigFile &cfg, const Common::String &section, const Common::String &value, int &val, int defaultVal) {
	if (!cfg.hasKey(value, section)) {
		val = defaultVal;
		return;
	}
	Common::String temp;
	cfg.getKey(value, section, temp);
	val = atoi(temp.c_str());
}

#define getFactorFromConfig(x, y, z) getValueFromConfig(x, "gradients", y, z, 1)
#define getExtraValueFromConfig(x, y, z, a) getValueFromConfig(x, "extra", y, z, a)

namespace GUI {

OverlayColor getColorAlpha(OverlayColor col1, OverlayColor col2, int alpha);
OverlayColor calcGradient(OverlayColor start, OverlayColor end, int pos, int max, uint factor);

#pragma mark -

ThemeNew::ThemeNew(OSystem *system, Common::String stylefile) : Theme(), _system(system), _screen(), _initOk(false),
_lastUsedBitMask(0), _forceRedraw(false), _fonts(), _imageHandles(0), _images(0), _colors(), _cursor(0), _gradientFactors() {
	_stylefile = stylefile;
	_initOk = false;
	_useCursor = false;
	memset(&_screen, 0, sizeof(_screen));
	memset(&_dialog, 0, sizeof(_dialog));
	memset(&_colors, 0, sizeof(_colors));
	memset(&_gradientFactors, 0, sizeof(_gradientFactors));

	_screen.create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(OverlayColor));
	if (_screen.pixels) {
		_initOk = true;
		clearAll();
	}

	if (ConfMan.hasKey("extrapath"))
		Common::File::addDefaultDirectoryRecursive(ConfMan.get("extrapath"));

	if (ConfMan.hasKey("themepath"))
		Common::File::addDefaultDirectory(ConfMan.get("themepath"));

	ImageMan.addArchive(stylefile + ".zip");

	if (!_configFile.loadFromFile(stylefile + ".ini")) {
#ifdef USE_ZLIB
		// Maybe find a nicer solution to this
		unzFile zipFile = unzOpen((stylefile + ".zip").c_str());
		if (zipFile && unzLocateFile(zipFile, (stylefile + ".ini").c_str(), 2) == UNZ_OK) {
			unz_file_info fileInfo;
			unzOpenCurrentFile(zipFile);
			unzGetCurrentFileInfo(zipFile, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
			uint8 *buffer = new uint8[fileInfo.uncompressed_size+1];
			assert(buffer);
			memset(buffer, 0, (fileInfo.uncompressed_size+1)*sizeof(uint8));
			unzReadCurrentFile(zipFile, buffer, fileInfo.uncompressed_size);
			unzCloseCurrentFile(zipFile);
			Common::MemoryReadStream stream(buffer, fileInfo.uncompressed_size+1);
			if (!_configFile.loadFromStream(stream)) {
				warning("Can not find theme config file '%s'", (stylefile + ".ini").c_str());
				unzClose(zipFile);
				return;
			}
			delete [] buffer;
			buffer = 0;
		} else {
			unzClose(zipFile);
			warning("Can not find theme config file '%s'", (stylefile + ".ini").c_str());
			return;
		}
		unzClose(zipFile);
#else
		warning("Can not find theme config file '%s'", (stylefile + ".ini").c_str());
		return;
#endif
	}

	Common::String temp = "";
	_configFile.getKey("version", "theme", temp);
	if (atoi(temp.c_str()) != THEME_VERSION) {
		// TODO: improve this detection and handle it nicer
		warning("Theme config uses a different version (you have: '%s', needed is: '%d')", temp.c_str(), THEME_VERSION);
		return;
	}
	
	static Common::String imageHandlesTable[kImageHandlesMax];

	// load the pixmap filenames from the config file
	_configFile.getKey("dialog_corner", "pixmaps", imageHandlesTable[kDialogBkgdCorner]);
	_configFile.getKey("dialog_top", "pixmaps", imageHandlesTable[kDialogBkgdTop]);
	_configFile.getKey("dialog_left", "pixmaps", imageHandlesTable[kDialogBkgdLeft]);
	_configFile.getKey("dialog_bkgd", "pixmaps", imageHandlesTable[kDialogBkgd]);
	
	_configFile.getKey("widget_corner", "pixmaps", imageHandlesTable[kWidgetBkgdCorner]);
	_configFile.getKey("widget_top", "pixmaps", imageHandlesTable[kWidgetBkgdTop]);
	_configFile.getKey("widget_left", "pixmaps", imageHandlesTable[kWidgetBkgdLeft]);
	_configFile.getKey("widget_bkgd", "pixmaps", imageHandlesTable[kWidgetBkgd]);
	
	_configFile.getKey("widget_small_corner", "pixmaps", imageHandlesTable[kWidgetSmallBkgdCorner]);
	_configFile.getKey("widget_small_top", "pixmaps", imageHandlesTable[kWidgetSmallBkgdTop]);
	_configFile.getKey("widget_small_left", "pixmaps", imageHandlesTable[kWidgetSmallBkgdLeft]);
	_configFile.getKey("widget_small_bkgd", "pixmaps", imageHandlesTable[kWidgetSmallBkgd]);
	
	_configFile.getKey("checkbox_empty", "pixmaps", imageHandlesTable[kCheckboxEmpty]);
	_configFile.getKey("checkbox_checked", "pixmaps", imageHandlesTable[kCheckboxChecked]);
	
	_configFile.getKey("widget_arrow", "pixmaps", imageHandlesTable[kWidgetArrow]);
	
	_configFile.getKey("tab_corner", "pixmaps", imageHandlesTable[kTabBkgdCorner]);
	_configFile.getKey("tab_top", "pixmaps", imageHandlesTable[kTabBkgdTop]);
	_configFile.getKey("tab_left", "pixmaps", imageHandlesTable[kTabBkgdLeft]);
	_configFile.getKey("tab_bkgd", "pixmaps", imageHandlesTable[kTabBkgd]);
	
	_configFile.getKey("slider_bkgd_corner", "pixmaps", imageHandlesTable[kSliderBkgdCorner]);
	_configFile.getKey("slider_bkgd_top", "pixmaps", imageHandlesTable[kSliderBkgdTop]);
	_configFile.getKey("slider_bkgd_left", "pixmaps", imageHandlesTable[kSliderBkgdLeft]);
	_configFile.getKey("slider_bkgd_bkgd", "pixmaps", imageHandlesTable[kSliderBkgd]);
	
	_configFile.getKey("slider_corner", "pixmaps", imageHandlesTable[kSliderCorner]);
	_configFile.getKey("slider_top", "pixmaps", imageHandlesTable[kSliderTop]);
	_configFile.getKey("slider_left", "pixmaps", imageHandlesTable[kSliderLeft]);
	_configFile.getKey("slider_bkgd", "pixmaps", imageHandlesTable[kSlider]);
	
	_configFile.getKey("scrollbar_bkgd_corner", "pixmaps", imageHandlesTable[kScrollbarBkgdCorner]);
	_configFile.getKey("scrollbar_bkgd_top", "pixmaps", imageHandlesTable[kScrollbarBkgdTop]);
	_configFile.getKey("scrollbar_bkgd_left", "pixmaps", imageHandlesTable[kScrollbarBkgdLeft]);
	_configFile.getKey("scrollbar_bkgd_bkgd", "pixmaps", imageHandlesTable[kScrollbarBkgd]);
	
	_configFile.getKey("scrollbar_corner", "pixmaps", imageHandlesTable[kScrollbarCorner]);
	_configFile.getKey("scrollbar_top", "pixmaps", imageHandlesTable[kScrollbarTop]);
	_configFile.getKey("scrollbar_left", "pixmaps", imageHandlesTable[kScrollbarLeft]);
	_configFile.getKey("scrollbar_bkgd", "pixmaps", imageHandlesTable[kScrollbar]);
	
	_configFile.getKey("button_corner", "pixmaps", imageHandlesTable[kButtonBkgdCorner]);
	_configFile.getKey("button_top", "pixmaps", imageHandlesTable[kButtonBkgdTop]);
	_configFile.getKey("button_left", "pixmaps", imageHandlesTable[kButtonBkgdLeft]);
	_configFile.getKey("button_bkgd", "pixmaps", imageHandlesTable[kButtonBkgd]);

	_configFile.getKey("theme_logo", "pixmaps", imageHandlesTable[kThemeLogo]);

	_configFile.getKey("popupwidget_corner", "pixmaps", imageHandlesTable[kPopUpWidgetBkgdCorner]);
	_configFile.getKey("popupwidget_top", "pixmaps", imageHandlesTable[kPopUpWidgetBkgdTop]);
	_configFile.getKey("popupwidget_left", "pixmaps", imageHandlesTable[kPopUpWidgetBkgdLeft]);
	_configFile.getKey("popupwidget_bkgd", "pixmaps", imageHandlesTable[kPopUpWidgetBkgd]);

	_configFile.getKey("cursor_image", "pixmaps", imageHandlesTable[kGUICursor]);
	
	// load the gradient factors from the config file
	getFactorFromConfig(_configFile, "main_dialog", _gradientFactors[kMainDialogFactor]);
	getFactorFromConfig(_configFile, "dialog", _gradientFactors[kDialogFactor]);
	getFactorFromConfig(_configFile, "dialog_special", _gradientFactors[kDialogSpecialFactor]);
	
	getFactorFromConfig(_configFile, "widget_small", _gradientFactors[kWidgetSmallFactor]);
	getFactorFromConfig(_configFile, "widget", _gradientFactors[kWidgetFactor]);
	
	getFactorFromConfig(_configFile, "button", _gradientFactors[kButtonFactor]);
	
	getFactorFromConfig(_configFile, "slider", _gradientFactors[kSliderFactor]);
	getFactorFromConfig(_configFile, "silder_bkgd", _gradientFactors[kSliderBackground]);
	
	getFactorFromConfig(_configFile, "tab", _gradientFactors[kTabFactor]);
	
	getFactorFromConfig(_configFile, "scrollbar", _gradientFactors[kScrollbarFactor]);
	getFactorFromConfig(_configFile, "scrollbar_background", _gradientFactors[kScrollbarBkgdFactor]);

	getFactorFromConfig(_configFile, "popupwidget", _gradientFactors[kPopUpWidgetFactor]);
	
	// load values with default values from the config file
	getExtraValueFromConfig(_configFile, "shadow_left_width", _shadowLeftWidth, 2);
	getExtraValueFromConfig(_configFile, "shadow_right_width", _shadowRightWidth, 4);
	getExtraValueFromConfig(_configFile, "shadow_top_height", _shadowTopHeight, 2);
	getExtraValueFromConfig(_configFile, "shadow_bottom_height", _shadowBottomHeight, 4);

	getExtraValueFromConfig(_configFile, "cursor_hotspot_x", _cursorHotspotX, 0);
	getExtraValueFromConfig(_configFile, "cursor_hotspot_y", _cursorHotspotY, 0);
	
	// inactive dialog shading stuff
	_dialogShadingCallback = 0;
	
	if (_configFile.hasKey("inactive_dialog_shading", "extra")) {
		_configFile.getKey("inactive_dialog_shading", "extra", temp);
		if (temp == "no_effect") {
			// nothing
		} else if (temp == "luminance") {
			_dialogShadingCallback = &ThemeNew::calcLuminance;
			// don't cache colors for the luminance effect
			//createCacheTable = true;
		} else if (temp == "dim") {
			if (!_configFile.hasKey("shading_dim_percent", "extra")) {
				warning("no 'shading_dim_percent' specified");
			} else {
				getValueFromConfig(_configFile, "extra", "shading_dim_percent", _dimPercentValue, -1);
				
				if (_dimPercentValue < 0) {
					_dimPercentValue = 0;
				} else if (_dimPercentValue > 100) {
					_dimPercentValue = 100;
				}
				
				if (_dimPercentValue != 0) {
					_dimPercentValue = 256 * (100 - _dimPercentValue) / 100;
					_dialogShadingCallback = &ThemeNew::calcDimColor;
				}
			}
		} else {
			warning("no valid 'inactive_dialog_shading' specified");
		}
	}

	// load up all fonts
	setupFonts();
	
	// load the colors from the config file
	setupColors();

	_imageHandles = imageHandlesTable;

	_images = new const Graphics::Surface*[ARRAYSIZE(imageHandlesTable)];
	assert(_images);

	for (int i = 0; i < kImageHandlesMax; ++i) {
		ImageMan.registerSurface(_imageHandles[i], 0);
		_images[i] = ImageMan.getSurface(_imageHandles[i]);
	}
	
	_lastUsedBitMask = gBitFormat;

	// creats the cursor image
	if (_system->hasFeature(OSystem::kFeatureCursorHasPalette)) {
		createCursor();
	}
}

ThemeNew::~ThemeNew() {
	deleteFonts();
	deinit();
	delete [] _images;
	delete [] _cursor;
	_images = 0;
	if (_imageHandles) {
		for (int i = 0; i < kImageHandlesMax; ++i) {
			ImageMan.unregisterSurface(_imageHandles[i]);
		}
	}
}

bool ThemeNew::init() {
	if (!_images)
		return false;
	for (int i = 0; i < kImageHandlesMax; ++i) {
		if (!_images[i]) {
			return false;
		}
	}

	deinit();
	_screen.create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(OverlayColor));
	if (_screen.pixels) {
		_initOk = true;
		clearAll();
		setupFonts();
		resetDrawArea();
	}

	if (isThemeLoadingRequired()) {
		loadTheme(_defaultConfig);
		loadTheme(_configFile, false); // Don't reset
	}
	
	return true;
}

void ThemeNew::deinit() {
	if (_initOk) {
		_system->hideOverlay();
		_screen.free();
		_initOk = false;
	}
}

void ThemeNew::refresh() {
	init();
	resetupGuiRenderer();
	if (_enabled)
		_system->showOverlay();
}

void ThemeNew::enable() {
	init();
	resetupGuiRenderer();
	resetDrawArea();
	setUpCursor();
	_system->showOverlay();
	clearAll();
	_enabled = true;
}

void ThemeNew::disable() {
	_system->disableCursorPalette(true);
	_system->hideOverlay();
	_enabled = false;
}

void ThemeNew::openDialog(bool topDialog) {
	if (!_dialog) {
		_dialog = new DialogState;
		assert(_dialog);
		// first dialog
		_dialog->screen.create(_screen.w, _screen.h, sizeof(OverlayColor));
	}
	
	if (_dialogShadingCallback && topDialog) {
		OverlayColor *col = (OverlayColor*)_screen.pixels;
		for (int y = 0; y < _screen.h; ++y) {
			for (int x = 0; x < _screen.w; ++x) {
				col[x] = (this->*(_dialogShadingCallback))(col[x]);
			}
			col += _screen.w;
		}
	}
	
	memcpy(_dialog->screen.pixels, _screen.pixels, _screen.pitch*_screen.h);
	
	if ((_dialogShadingCallback) && topDialog)
		addDirtyRect(Common::Rect(0, 0, _screen.w, _screen.h), false, false);
}

void ThemeNew::closeDialog() {
	if (_dialog) {
		_dialog->screen.free();
		delete _dialog;
		_dialog = 0;
	}
	_forceRedraw = true;
}

void ThemeNew::clearAll() {
	if (!_initOk)
		return;
	_system->clearOverlay();
	// FIXME: problem with the 'pitch'
	_system->grabOverlay((OverlayColor*)_screen.pixels, _screen.w);
}

void ThemeNew::drawAll() {
	// TODO: see ThemeNew::addDirtyRect
	_forceRedraw = false;
}

void ThemeNew::setDrawArea(const Common::Rect &r) {
	if (_initOk) {
		_drawArea = r;
		_shadowDrawArea = Common::Rect(r.left-_shadowLeftWidth, r.top-_shadowTopHeight, r.right+_shadowRightWidth, r.bottom+_shadowBottomHeight);
		_drawArea.clip(_screen.w, _screen.h);
		_shadowDrawArea.clip(_screen.w, _screen.h);
	}
}

void ThemeNew::resetDrawArea() {
	if (_initOk) {
		_drawArea = Common::Rect(0, 0, _screen.w, _screen.h);
		_shadowDrawArea = _drawArea;
	}
}

#define surface(x) (_images[x])
 
void ThemeNew::drawDialogBackground(const Common::Rect &r, uint16 hints, kState state) {
	if (!_initOk)
		return;

	Common::Rect r2 = shadowRect(r, kShadowFull);

	if ((hints & THEME_HINT_SAVE_BACKGROUND) && !(hints & THEME_HINT_FIRST_DRAW) && !_forceRedraw) {
		restoreBackground(r2, true);
		return;
	}

	if (hints & THEME_HINT_MAIN_DIALOG) {
		colorFade(r, _colors[kMainDialogStart], _colors[kMainDialogEnd], _gradientFactors[kMainDialogFactor]);
	} else if (hints & THEME_HINT_SPECIAL_COLOR) {
		drawShadow(r, surface(kDialogBkgdCorner), surface(kDialogBkgdTop), surface(kDialogBkgdLeft), surface(kDialogBkgd), kShadowFull);

		drawRectMasked(r, surface(kDialogBkgdCorner), surface(kDialogBkgdTop), surface(kDialogBkgdLeft), surface(kDialogBkgd),
			256, _colors[kMainDialogStart], _colors[kMainDialogEnd], _gradientFactors[kDialogSpecialFactor]);
	} else {
		drawShadow(r, surface(kDialogBkgdCorner), surface(kDialogBkgdTop), surface(kDialogBkgdLeft), surface(kDialogBkgd), kShadowFull);

		drawRectMasked(r, surface(kDialogBkgdCorner), surface(kDialogBkgdTop), surface(kDialogBkgdLeft), surface(kDialogBkgd),
				256, _colors[kDialogStart], _colors[kDialogEnd], _gradientFactors[kDialogFactor]);
	}

	addDirtyRect(r2, (hints & THEME_HINT_SAVE_BACKGROUND) != 0, true);
}

void ThemeNew::drawText(const Common::Rect &r, const Common::String &str, kState state, kTextAlign align, bool inverted, int deltax, bool useEllipsis, kFontStyle font) {
	if (!_initOk)
		return;
	Common::Rect r2(r.left, r.top, r.right, r.top+getFontHeight(font));
	uint32 color;

	restoreBackground(r2);
	r2.bottom += 4;

	if (inverted) {
		_screen.fillRect(r, _colors[kTextInvertedBackground]);
		color = _colors[kTextInvertedColor];
	} else {
		color = getColor(state);
	}

	getFont(font)->drawString(&_screen, str, r.left, r.top, r.width(), color, convertAligment(align), deltax, useEllipsis);
	addDirtyRect(r2);
}

void ThemeNew::drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, kState state) {
	if (!_initOk)
		return;
	restoreBackground(r);
	font->drawChar(&_screen, ch, r.left, r.top, getColor(state));
	addDirtyRect(r);
}

void ThemeNew::drawWidgetBackground(const Common::Rect &r, uint16 hints, kWidgetBackground background, kState state) {
	if (!_initOk)
		return;

	Common::Rect r2;
	
	if ((hints & THEME_HINT_SAVE_BACKGROUND) && !(hints & THEME_HINT_FIRST_DRAW) && !_forceRedraw) {
		restoreBackground((hints & THEME_HINT_USE_SHADOW) ? r2 : r);
		return;
	}
	
	if (background == kWidgetBackgroundBorderSmall) {
		if ((hints & THEME_HINT_USE_SHADOW)) {
			r2 = shadowRect(r, kShadowSmall);
			restoreBackground(r2);
			// shadow
			drawShadow(r, surface(kWidgetSmallBkgdCorner), surface(kWidgetSmallBkgdTop), surface(kWidgetSmallBkgdLeft),
						surface(kWidgetSmallBkgd), kShadowSmall);
		}

		drawRectMasked(r, surface(kWidgetSmallBkgdCorner), surface(kWidgetSmallBkgdTop), surface(kWidgetSmallBkgdLeft), surface(kWidgetSmallBkgd),
						(state == kStateDisabled) ? -30 : 256, _colors[kWidgetBackgroundSmallStart], _colors[kWidgetBackgroundSmallEnd],
						_gradientFactors[kWidgetSmallFactor]);
	} else {
		if ((hints & THEME_HINT_USE_SHADOW)) {
			r2 = shadowRect(r, kShadowFull);
			restoreBackground(r2);
			// shadow
			drawShadow(r, surface(kWidgetBkgdCorner), surface(kWidgetBkgdTop), surface(kWidgetBkgdLeft), surface(kWidgetBkgd),
						kShadowFull);
		}

		drawRectMasked(r, surface(kWidgetBkgdCorner), surface(kWidgetBkgdTop), surface(kWidgetBkgdLeft), surface(kWidgetBkgd),
						(state == kStateDisabled) ? -30 : 256, _colors[kWidgetBackgroundStart], _colors[kWidgetBackgroundEnd],
						_gradientFactors[kWidgetFactor]);
	}

	addDirtyRect((hints & THEME_HINT_USE_SHADOW) ? r2 : r, (hints & THEME_HINT_SAVE_BACKGROUND) != 0);
}

void ThemeNew::drawButton(const Common::Rect &r, const Common::String &str, kState state) {
	if (!_initOk)
		return;
	
	Common::Rect r2 = shadowRect(r, kShadowButton);
	restoreBackground(r2);

	// shadow
	drawShadow(r, surface(kButtonBkgdCorner), surface(kButtonBkgdTop), surface(kButtonBkgdLeft), surface(kButtonBkgd), kShadowButton);

	if (state == kStateHighlight) {
		drawRectMasked(r, surface(kButtonBkgdCorner), surface(kButtonBkgdTop), surface(kButtonBkgdLeft), surface(kButtonBkgd),
						256, _colors[kButtonBackgroundHighlightStart], _colors[kButtonBackgroundHighlightEnd],
						_gradientFactors[kButtonFactor]);
	} else {
		drawRectMasked(r, surface(kButtonBkgdCorner), surface(kButtonBkgdTop), surface(kButtonBkgdLeft), surface(kButtonBkgd),
						(state == kStateDisabled) ? -30 : 256, _colors[kButtonBackgroundStart], _colors[kButtonBackgroundEnd],
						_gradientFactors[kButtonFactor]);
	}

	const int off = (r.height() - getFontHeight()) / 2;

	OverlayColor col = 0;
	switch (state) {
	case kStateEnabled:
		col = _colors[kButtonTextEnabled];
		break;

	case kStateHighlight:
		col = _colors[kButtonTextHighlight];
		break;

	default:
		col = _colors[kButtonTextDisabled];
		break;
	};

	getFont()->drawString(&_screen, str, r.left, r.top + off, r.width(), col, Graphics::kTextAlignCenter, 0, true);

	addDirtyRect(r2);
}

void ThemeNew::drawSurface(const Common::Rect &r, const Graphics::Surface &surface, kState state, bool transparency) {
	if (!_initOk)
		return;

	Common::Rect rect(r.left, r.top, r.left + surface.w, r.top + surface.h);
	rect.clip(_screen.w, _screen.h);

	if (!rect.isValidRect())
		return;
	
	assert(surface.bytesPerPixel == sizeof(OverlayColor));

	if (transparency) {
		drawSurface(rect, &surface, false, false, 256);
		addDirtyRect(r);
		return;
	}

	OverlayColor *src = (OverlayColor *)surface.pixels;
	OverlayColor *dst = (OverlayColor *)_screen.getBasePtr(rect.left, rect.top);

	int w = rect.width();
	int h = rect.height();

	while (h--) {
		memcpy(dst, src, surface.pitch);
		src += w;
		// FIXME: this should be pitch
		dst += _screen.w;
	}
	addDirtyRect(r);
}

void ThemeNew::drawSlider(const Common::Rect &r, int width, kState state) {
	if (!_initOk)
		return;

	drawRectMasked(r, surface(kSliderBkgdCorner), surface(kSliderBkgdTop), surface(kSliderBkgdLeft), surface(kSliderBkgd), 256,
					_colors[kSliderBackgroundStart], _colors[kSliderBackgroundEnd], _gradientFactors[kSliderBackground]);

	Common::Rect r2 = r;
	r2.left = r.left + 2;
	r2.top = r.top + 2;
	r2.bottom = r.bottom - 2;
	r2.right = r2.left + width;
	if (r2.right > r.right - 2) {
		r2.right = r.right - 2;
	}
	
	// shadow
	drawShadow(r2, surface(kSliderCorner), surface(kSliderTop), surface(kSliderLeft), surface(kSliderBkgd), kShadowSlider);

	if (state == kStateHighlight) {
		drawRectMasked(r2, surface(kSliderCorner), surface(kSliderTop), surface(kSliderLeft), surface(kSliderBkgd),
					256, _colors[kSliderHighStart], _colors[kSliderHighEnd], _gradientFactors[kSliderFactor]);
	} else {
		drawRectMasked(r2, surface(kSliderCorner), surface(kSliderTop), surface(kSliderLeft), surface(kSliderBkgd),
					(state == kStateDisabled) ? -30 : 256, _colors[kSliderStart], _colors[kSliderEnd], _gradientFactors[kSliderFactor]);
	}

	addDirtyRect(r);
}

void ThemeNew::drawPopUpWidget(const Common::Rect &r, const Common::String &sel, int deltax, kState state, kTextAlign align) {
	if (!_initOk)
		return;

	Common::Rect r2 = shadowRect(r, kShadowSmall);
	restoreBackground(r2);

	OverlayColor start = _colors[kPopUpWidgetStart], end = _colors[kPopUpWidgetEnd];
	if (state == kStateHighlight) {
		start = _colors[kPopUpWidgetHighlightStart];
		end = _colors[kPopUpWidgetHighlightEnd];
	}

	drawShadow(r, surface(kDialogBkgdCorner), surface(kDialogBkgdTop), surface(kDialogBkgdLeft), surface(kDialogBkgd), kShadowSmall);

	drawRectMasked(r, surface(kPopUpWidgetBkgdCorner), surface(kPopUpWidgetBkgdTop), surface(kPopUpWidgetBkgdLeft), surface(kPopUpWidgetBkgd),
						(state == kStateDisabled) ? -30 : 256, start, end, _gradientFactors[kPopUpWidgetFactor]);

	const Graphics::Surface *arrow = surface(kWidgetArrow);

	int yOff = r.height() / 2 - arrow->h;
	if (yOff < 0)
		yOff = 0;

	Common::Rect arrowRect(r.right - 4 - arrow->w, r.top + yOff, r.right - 4, r.top + yOff + arrow->h);
	arrowRect.clip(r);

	drawSurface(arrowRect, arrow, false, false, (state == kStateDisabled) ? -30 : 256);

	arrowRect.top += arrow->h;
	arrowRect.bottom += arrow->h;
	arrowRect.clip(r);
	drawSurface(arrowRect, arrow, true, false, (state == kStateDisabled) ? -30 : 256);

	if (sel != "") {
		Common::Rect text(r.left + 2, r.top + 3, r.right - 4, r.top + 3 + getFont()->getFontHeight());
		getFont()->drawString(&_screen, sel, text.left, text.top, text.width(), getColor(state), convertAligment(align), deltax, false);
	}

	addDirtyRect(r2);
}

void ThemeNew::drawCheckbox(const Common::Rect &r, const Common::String &str, bool checked, kState state) {
	if (!_initOk)
		return;
	Common::Rect r2 = r;

	const Graphics::Surface *checkBox = surface(checked ? kCheckboxChecked : kCheckboxEmpty);
	int checkBoxSize = checkBox->w;
	
	restoreBackground(Common::Rect(r.left, r.top, r.left+checkBox->w, r.top+checkBox->h));

	drawSurface(Common::Rect(r.left, r.top, r.left+checkBox->w, r.top+checkBox->h), checkBox, false, false, (state == kStateDisabled) ? 128 : 256);

	r2.left += checkBoxSize + 5;
	getFont()->drawString(&_screen, str, r2.left, r2.top, r2.width(), getColor(state), Graphics::kTextAlignLeft, 0, false);

	addDirtyRect(r);
}

void ThemeNew::drawTab(const Common::Rect &r, int tabHeight, int tabWidth, const Common::Array<Common::String> &tabs, int active, uint16 hints, kState state) {
	if (!_initOk)
		return;

	restoreBackground(r);
	int tabXOffset = surface(kWidgetSmallBkgdCorner)->w;
	
	OverlayColor tabEnd = calcGradient(_colors[kTabBackgroundStart], _colors[kTabBackgroundEnd], tabHeight, r.height(), _gradientFactors[kTabFactor]);

	for (int i = 0; i < (int)tabs.size(); ++i) {
		if (i == active)
			continue;

		Common::Rect tabRect(r.left + tabXOffset + i * tabWidth, r.top, r.left + tabXOffset + i * tabWidth + tabWidth, r.top + tabHeight);
		drawRectMasked(tabRect, surface(kTabBkgdCorner), surface(kTabBkgdTop), surface(kTabBkgdLeft), surface(kTabBkgd),
					128, _colors[kTabBackgroundStart], tabEnd, _gradientFactors[kTabFactor], true);

		getFont()->drawString(&_screen, tabs[i], tabRect.left, tabRect.top+2, tabRect.width(), getColor(kStateEnabled), Graphics::kTextAlignCenter, 0, true);
	}
	
	Common::Rect widgetBackground = Common::Rect(r.left, r.top + tabHeight, r.right, r.bottom);
	drawRectMasked(widgetBackground, surface(kWidgetSmallBkgdCorner), surface(kWidgetSmallBkgdTop), surface(kWidgetSmallBkgdLeft), surface(kWidgetSmallBkgd),
						(state == kStateDisabled) ? -30 : 256, tabEnd, _colors[kTabBackgroundEnd],
						_gradientFactors[kTabFactor]);
	addDirtyRect(widgetBackground, true);
	
	Common::Rect tabRect(r.left + tabXOffset + active * tabWidth, r.top, r.left + tabXOffset + active * tabWidth + tabWidth, r.top + tabHeight + 1);
	drawRectMasked(tabRect, surface(kTabBkgdCorner), surface(kTabBkgdTop), surface(kTabBkgdLeft), surface(kTabBkgd),
				256, _colors[kTabBackgroundStart], tabEnd, _gradientFactors[kTabFactor], true);

	getFont()->drawString(&_screen, tabs[active], tabRect.left, tabRect.top+2, tabRect.width(), getColor(kStateHighlight), Graphics::kTextAlignCenter, 0, true);

	addDirtyRect(r);
}

void ThemeNew::drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, kScrollbarState scrollState, kState state) {
	if (!_initOk)
		return;
	const int UP_DOWN_BOX_HEIGHT = r.width() + 1;
	Common::Rect r2 = r;

	// draws the scrollbar background
	drawRectMasked(r2, surface(kScrollbarBkgdCorner), surface(kScrollbarBkgdTop), surface(kScrollbarBkgdLeft), surface(kScrollbarBkgd), 256,
					_colors[kScrollbarBackgroundStart], _colors[kScrollbarBackgroundEnd], _gradientFactors[kScrollbarBkgdFactor]);

	// draws the 'up' button
	OverlayColor buttonStart = 0;
	OverlayColor buttonEnd = 0;
	
	if (scrollState == kScrollbarStateUp) {
		buttonStart = _colors[kScrollbarButtonHighlightStart];
		buttonEnd = _colors[kScrollbarButtonHighlightEnd];
	} else {
		buttonStart = _colors[kScrollbarButtonStart];
		buttonEnd = _colors[kScrollbarButtonEnd];
	}
	
	r2.bottom = r2.top + UP_DOWN_BOX_HEIGHT;
	drawRectMasked(r2, surface(kScrollbarBkgdCorner), surface(kScrollbarBkgdTop), surface(kScrollbarBkgdLeft), surface(kScrollbarBkgd), 256,
					buttonStart, buttonEnd, _gradientFactors[kScrollbarBkgdFactor]);

	const Graphics::Surface *arrow = surface(kWidgetArrow);
	r2.left += 1 + (r2.width() - arrow->w) / 2;
	r2.right = r2.left + arrow->w;
	r2.top += (r2.height() - arrow->h) / 2;
	r2.bottom = r2.top + arrow->h;
	drawSurface(r2, arrow, false, false, 256);

	// draws the slider
	OverlayColor sliderStart = 0;
	OverlayColor sliderEnd = 0;
	
	if (scrollState == kScrollbarStateSlider) {
		sliderStart = _colors[kScrollbarSliderHighlightStart];
		sliderEnd = _colors[kScrollbarSliderHighlightEnd];
	} else {
		sliderStart = _colors[kScrollbarSliderStart];
		sliderEnd = _colors[kScrollbarSliderEnd];
	}
	
	r2 = r;
	r2.top += sliderY;
	r2.bottom = r2.top + sliderHeight;
	
	drawShadow(r2, surface(kSliderCorner), surface(kSliderTop), surface(kSliderLeft), surface(kSliderBkgd), kShadowSmall);

	r2.left += 2;
	r2.right -= 2;	

	r2.bottom = r2.top + sliderHeight / 2 + surface(kScrollbarCorner)->h + 4;
	drawRectMasked(r2, surface(kScrollbarCorner), surface(kScrollbarTop), surface(kScrollbarLeft), surface(kScrollbarBkgd), 256,
					sliderStart, sliderEnd, _gradientFactors[kScrollbarFactor]);
	r2.top += sliderHeight / 2;
	r2.bottom += sliderHeight / 2 - surface(kScrollbarCorner)->h - 4;
	drawRectMasked(r2, surface(kScrollbarCorner), surface(kScrollbarTop), surface(kScrollbarLeft), surface(kScrollbarBkgd), 256,
					sliderEnd, sliderStart, _gradientFactors[kScrollbarFactor]);

	// draws the 'down' button
	
	if (scrollState == kScrollbarStateDown) {
		buttonStart = _colors[kScrollbarButtonHighlightStart];
		buttonEnd = _colors[kScrollbarButtonHighlightEnd];
	} else {
		buttonStart = _colors[kScrollbarButtonStart];
		buttonEnd = _colors[kScrollbarButtonEnd];
	}
	
	r2 = r;
	r2.top = r2.bottom - UP_DOWN_BOX_HEIGHT;
	drawRectMasked(r2, surface(kScrollbarBkgdCorner), surface(kScrollbarBkgdTop), surface(kScrollbarBkgdLeft), surface(kScrollbarBkgd), 256,
					buttonStart, buttonEnd, _gradientFactors[kScrollbarBkgdFactor]);

	r2.left += 1 + (r2.width() - arrow->w) / 2;
	r2.right = r2.left + arrow->w;
	r2.top += (r2.height() - arrow->h) / 2;
	r2.bottom = r2.top + arrow->h;
	drawSurface(r2, arrow, true, false, 256);

	addDirtyRect(r);
}

void ThemeNew::drawCaret(const Common::Rect &r, bool erase, kState state) {
	if (!_initOk)
		return;

	restoreBackground(r);
	if (!erase) {
		_screen.vLine(r.left, r.top, r.bottom, _colors[kCaretColor]);
	} else if (r.top >= 0) {
		// FIXME: hack to restore the caret background correctly
		const OverlayColor search = _colors[kTextInvertedBackground];
		const OverlayColor *src = (const OverlayColor*)_screen.getBasePtr(r.left-1, r.top-1);
		int height = r.height() + 2;
		if (r.top + height > _screen.h) {
			height = _screen.h - r.top;
		}
		bool drawInvBackground = false;
		while (height--) {
			if (src[0] == search || src[1] == search || src[2] == search) {
				drawInvBackground = true;
			}
			src += _screen.w;
		}
		if (drawInvBackground) {
			_screen.vLine(r.left, r.top, r.bottom, search);
		}
	}
	addDirtyRect(r);
}

void ThemeNew::drawLineSeparator(const Common::Rect &r, kState state) {
	if (!_initOk)
		return;
	_screen.hLine(r.left - 1, r.top + r.height() / 2, r.right, _system->RGBToColor(0, 0, 0));
	addDirtyRect(r);
}

#pragma mark - intern drawing

void ThemeNew::restoreBackground(Common::Rect r, bool special) {
	r.clip(_screen.w, _screen.h);
	if (special) {
		r.clip(_shadowDrawArea);
	} else {
		r.clip(_drawArea);
	}
	if (_dialog) {
		if (!_dialog->screen.pixels) {
			return;
		}
		const OverlayColor *src = (const OverlayColor*)_dialog->screen.getBasePtr(r.left, r.top);
		OverlayColor *dst = (OverlayColor*)_screen.getBasePtr(r.left, r.top);

		int h = r.height();
		int w = r.width();
		while (h--) {
			memcpy(dst, src, w*sizeof(OverlayColor));
			src += _dialog->screen.w;
			dst += _screen.w;
		}
	}
}

bool ThemeNew::addDirtyRect(Common::Rect r, bool backup, bool special) {
	// TODO: implement proper dirty rect handling
	// FIXME: problem with the 'pitch'
	r.clip(_screen.w, _screen.h);
	if (special) {
		r.clip(_shadowDrawArea);
	} else {
		r.clip(_drawArea);
	}
	_system->copyRectToOverlay((OverlayColor*)_screen.getBasePtr(r.left, r.top), _screen.w, r.left, r.top, r.width(), r.height());
	if (_dialog && backup) {
		if (_dialog->screen.pixels) {
			OverlayColor *dst = (OverlayColor*)_dialog->screen.getBasePtr(r.left, r.top);
			const OverlayColor *src = (const OverlayColor*)_screen.getBasePtr(r.left, r.top);
			int h = r.height();
			while (h--) {
				memcpy(dst, src, r.width()*sizeof(OverlayColor));
				dst += _dialog->screen.w;
				src += _screen.w;
			}
		}
	}
	return true;
}

void ThemeNew::colorFade(const Common::Rect &r, OverlayColor start, OverlayColor end, uint factor) {
	OverlayColor *ptr = (OverlayColor*)_screen.getBasePtr(r.left, r.top);
	int h = r.height();
	while (h--) {
		OverlayColor col = calcGradient(start, end, r.height()-h, r.height(), factor);
		for (int i = 0; i < r.width(); ++i) {
			ptr[i] = col;
		}
		ptr += _screen.w;
	}
}

void ThemeNew::drawRect(const Common::Rect &r, const Surface *corner, const Surface *top, const Surface *left, const Surface *fill, int alpha, bool skipLastRow) {
	drawRectMasked(r, corner, top, left, fill, alpha, _system->RGBToColor(255, 255, 255), _system->RGBToColor(255, 255, 255), 1, skipLastRow);
}

void ThemeNew::drawRectMasked(const Common::Rect &r, const Graphics::Surface *corner, const Graphics::Surface *top,
							const Graphics::Surface *left, const Graphics::Surface *fill, int alpha,
							OverlayColor start, OverlayColor end, uint factor, bool skipLastRow) {
	int drawWidth = MIN(corner->w, MIN(top->w, MIN(left->w, fill->w)));
	int drawHeight = MIN(corner->h, MIN(top->h, MIN(left->h, fill->h)));
	int partsH = r.height() / drawHeight;
	int partsW = r.width() / drawWidth;
	int yPos = r.top;

	int specialHeight = 0;
	int specialWidth = 0;

	if (drawHeight*2 > r.height()) {
		drawHeight = r.height() / 2;
		partsH = 2;
	} else {
		specialHeight = r.height() % drawHeight;
		if (specialHeight != 0)
			++partsH;
	}

	if (drawWidth*2 > r.width()) {
		drawWidth = r.width() / 2;
		partsW = 2;
	} else {
		specialWidth = r.width() % drawWidth;
		if (specialWidth != 0)
			++partsW;
	}

	for (int y = 0; y < partsH; ++y) {
		int xPos = r.left;
		bool upDown = (y == partsH - 1);

		// calculate the correct drawing height
		int usedHeight = drawHeight;
		if (specialHeight && y == 1) {
			usedHeight = specialHeight;
		}

		OverlayColor startCol = calcGradient(start, end, yPos-r.top, r.height(), factor);
		OverlayColor endCol = calcGradient(start, end, yPos-r.top+usedHeight, r.height(), factor);

		for (int i = 0; i < partsW; ++i) {
			// calculate the correct drawing width
			int usedWidth = drawWidth;
			if (specialWidth && i == 1) {
				usedWidth = specialWidth;
			}

			// draw the right surface
			if (!i || i == partsW - 1) {
				if (!y || (y == partsH - 1 && !skipLastRow)) {
					drawSurfaceMasked(Common::Rect(xPos, yPos, xPos+usedWidth, yPos+usedHeight), corner, upDown, (i == partsW - 1), alpha, startCol, endCol);
				} else {
					drawSurfaceMasked(Common::Rect(xPos, yPos, xPos+usedWidth, yPos+usedHeight), left, upDown, (i == partsW - 1), alpha, startCol, endCol);
				}
			} else if (!y || (y == partsH - 1 && !skipLastRow)) {
				drawSurfaceMasked(Common::Rect(xPos, yPos, xPos+usedWidth, yPos+usedHeight), top, upDown, false, alpha, startCol, endCol);
			} else {
				drawSurfaceMasked(Common::Rect(xPos, yPos, xPos+usedWidth, yPos+usedHeight), fill, false, false, alpha, startCol, endCol);
			}
			xPos += usedWidth;
		}

		yPos += usedHeight;
	}
}

Common::Rect ThemeNew::shadowRect(const Common::Rect &r, uint32 shadowStyle) {
	switch (shadowStyle) {
	case kShadowButton:
		return Common::Rect(r.left - _shadowLeftWidth/2, r.top - _shadowTopHeight/2, r.right + _shadowRightWidth/2 - 1, r.bottom + _shadowBottomHeight/2 - 1);
		break;

	default:
		return Common::Rect(r.left - _shadowLeftWidth/2, r.top - _shadowTopHeight/2, r.right + _shadowRightWidth/2 + 1, r.bottom + _shadowBottomHeight/2 + 1);
		break;
	}

	return Common::Rect();
}

void ThemeNew::drawShadow(const Common::Rect &r, const Graphics::Surface *corner, const Graphics::Surface *top,
						const Graphics::Surface *left, const Graphics::Surface *fill, uint32 shadowStyle, bool skipLastRow) {
	switch (shadowStyle) {
	case kShadowFull: {
		Common::Rect r2(r.left - _shadowLeftWidth/2, r.top - _shadowTopHeight/2, r.right + _shadowRightWidth/2 + 1, r.bottom + _shadowBottomHeight/2 + 1);
		Common::Rect r3(r.left - _shadowLeftWidth/2, r.top - _shadowTopHeight/2, r.right + _shadowRightWidth/2, r.bottom + _shadowBottomHeight/2);
		Common::Rect r4(r.left - _shadowLeftWidth/2 + 1, r.top - _shadowTopHeight/2 + 1, r.right + _shadowRightWidth/2-1, r.bottom + _shadowBottomHeight/2-1);

		drawShadowRect(r2, r, corner, top, left, fill, kShadowTr1, skipLastRow);
		drawShadowRect(r3, r, corner, top, left, fill, kShadowTr2, skipLastRow);
		drawShadowRect(r4, r, corner, top, left, fill, kShadowTr3, skipLastRow);
		} break;

	case kShadowSmall: {
		Common::Rect r3(r.left - _shadowLeftWidth/2, r.top - _shadowTopHeight/2, r.right + _shadowRightWidth/2, r.bottom + _shadowBottomHeight/2);
		Common::Rect r4(r.left - _shadowLeftWidth/2 + 1, r.top - _shadowTopHeight/2 + 1, r.right + _shadowRightWidth/2-1, r.bottom + _shadowBottomHeight/2-1);

		drawShadowRect(r3, r, corner, top, left, fill, kShadowTr1, skipLastRow);
		drawShadowRect(r4, r, corner, top, left, fill, kShadowTr2, skipLastRow);
		} break;

	case kShadowButton: {
		Common::Rect r2(r.left - _shadowLeftWidth/2, r.top - _shadowTopHeight/2, r.right + _shadowRightWidth/2 - 1, r.bottom + _shadowBottomHeight/2 - 1);
		Common::Rect r3(r.left - _shadowLeftWidth/2 + 1, r.top - _shadowTopHeight/2 + 1, r.right + _shadowRightWidth/2 - 1, r.bottom + _shadowBottomHeight/2 - 1);

		drawShadowRect(r2, r, corner, top, left, fill, kShadowTr2, skipLastRow);
		drawShadowRect(r3, r, corner, top, left, fill, kShadowTr4, skipLastRow);
		} break;

	case kShadowSlider: {
		Common::Rect r3(r.left - _shadowLeftWidth/2, r.top - _shadowTopHeight/2, r.right + _shadowRightWidth/2, r.bottom + _shadowBottomHeight/2);
		Common::Rect r4(r.left - _shadowLeftWidth/2, r.top - _shadowTopHeight/2, r.right + _shadowRightWidth/2 - 1, r.bottom + _shadowBottomHeight/2 - 1);

		drawShadowRect(r3, r, corner, top, left, fill, kShadowTr2, skipLastRow);
		drawShadowRect(r4, r, corner, top, left, fill, kShadowTr3, skipLastRow);
		};

	default:
		break;
	}
}

void ThemeNew::drawShadowRect(const Common::Rect &r, const Common::Rect &area, const Graphics::Surface *corner,
							const Graphics::Surface *top, const Graphics::Surface *left, const Graphics::Surface *fill,
							int alpha, bool skipLastRow) {
	int drawWidth = MIN(corner->w, MIN(top->w, MIN(left->w, fill->w)));
	int drawHeight = MIN(corner->h, MIN(top->h, MIN(left->h, fill->h)));
	int partsH = r.height() / drawHeight;
	int partsW = r.width() / drawWidth;
	int yPos = r.top;

	int yDrawTilesTop = 1 + (ABS(area.top - r.top) % drawHeight);
	int xDrawTilesLeft = 1 + (ABS(area.left - r.left) % drawWidth);
	int yDrawTilesBottom = 1 + (ABS(area.bottom - r.bottom) % drawHeight);
	int xDrawTilesRight = 1 + (ABS(area.right - r.right) % drawWidth);

	int specialHeight = 0;
	int specialWidth = 0;

	if (drawHeight*2 > r.height()) {
		drawHeight = r.height() / 2;
		partsH = 2;
	} else {
		specialHeight = r.height() % drawHeight;
		if (specialHeight != 0)
			++partsH;
	}

	if (drawWidth*2 > r.width()) {
		drawWidth = r.width() / 2;
		partsW = 2;
	} else {
		specialWidth = r.width() % drawWidth;
		if (specialWidth != 0)
			++partsW;
	}

	OverlayColor startCol = g_system->RGBToColor(0, 0, 0);
	OverlayColor endCol = g_system->RGBToColor(0, 0, 0);

	for (int y = 0; y < partsH; ++y) {
		// calculate the correct drawing height
		int usedHeight = drawHeight;
		if (specialHeight && y == 1) {
			usedHeight = specialHeight;
		}

		int xPos = r.left;
		bool upDown = (y == partsH - 1);

		for (int i = 0; i < partsW; ++i) {
			// calculate the correct drawing width
			int usedWidth = drawWidth;
			if (specialWidth && i == 1) {
				usedWidth = specialWidth;
			}

			if (i >= xDrawTilesLeft && i <= partsW - xDrawTilesRight && y >= yDrawTilesTop && y <= partsH - yDrawTilesBottom) {
				xPos += usedWidth;
				continue;
			}

			// draw the right surface
			if (!i || i == partsW - 1) {
				if (!y || (y == partsH - 1 && !skipLastRow)) {
					drawSurfaceMasked(Common::Rect(xPos, yPos, xPos+usedWidth, yPos+usedHeight), corner, upDown, (i == partsW - 1), alpha, startCol, endCol);
				} else {
					drawSurfaceMasked(Common::Rect(xPos, yPos, xPos+usedWidth, yPos+usedHeight), left, upDown, (i == partsW - 1), alpha, startCol, endCol);
				}
			} else if (!y || (y == partsH - 1 && !skipLastRow)) {
				drawSurfaceMasked(Common::Rect(xPos, yPos, xPos+usedWidth, yPos+usedHeight), top, upDown, false, alpha, startCol, endCol);
			} else {
				drawSurfaceMasked(Common::Rect(xPos, yPos, xPos+usedWidth, yPos+usedHeight), fill, upDown, false, alpha, startCol, endCol);
			}
			xPos += usedWidth;
		}
		yPos += usedHeight;
	}
}

void ThemeNew::drawSurface(const Common::Rect &r, const Surface *surf, bool upDown, bool leftRight, int alpha) {
	drawSurfaceMasked(r, surf, upDown, leftRight, alpha, _system->RGBToColor(255, 255, 255), _system->RGBToColor(255, 255, 255));
}

void ThemeNew::drawSurfaceMasked(const Common::Rect &r, const Graphics::Surface *surf, bool upDown, bool leftRight,
								int alpha, OverlayColor start, OverlayColor end, uint factor) {
	OverlayColor *dst = (OverlayColor*)_screen.getBasePtr(r.left, r.top);
	const OverlayColor *src = 0;

	const OverlayColor transparency = _colors[kColorTransparency];
	int drawWidth = (r.width() < surf->w) ? r.width() : surf->w;
	if (r.left + drawWidth > _screen.w)
		drawWidth = _screen.w - r.left;

	int srcAdd = 0;
	if (upDown) {
		src = (const OverlayColor*)surf->pixels + (surf->h - 1) * surf->w;
		srcAdd = -surf->w;
	} else {
		src = (const OverlayColor*)surf->pixels;
		srcAdd = surf->w;
	}

	int h = r.height();
	if (r.top + h > _screen.h)
		h = _screen.h - r.top;

	while (dst < _screen.pixels) {
		dst += _screen.w;
		src += srcAdd;
		--h;
	}

	if (h <= 0)
		return;

#define NO_EFFECT(x, y) (src[x] & rowColor)
#define ALPHA_EFFECT(x, y) (getColorAlpha(src[x] & rowColor, dst[y], alpha))
#define DARKEN_EFFECT(x, y) (calcDimColor(src[x] & rowColor))

#define LEFT_RIGHT_OFFSET(x) (drawWidth-x-1)
#define NORMAL_OFFSET(x) (x)

#define blitSurface(a, b) \
		for (int i = 0; i < h; ++i) { \
			OverlayColor rowColor = calcGradient(start, end, i, r.height(), factor); \
			for (int x = 0; x < drawWidth; ++x) { \
				if (src[a(x)] != transparency) \
					dst[x] = b(a(x), x); \
			} \
			dst += _screen.w; \
			src += srcAdd; \
		}
	if (alpha >= 256) {
		if (leftRight) {
			blitSurface(LEFT_RIGHT_OFFSET, NO_EFFECT);
		} else {
			blitSurface(NORMAL_OFFSET, NO_EFFECT);
		}
	} else if (alpha < 0) {
		int backUp = _dimPercentValue;
		_dimPercentValue = 256 * (100 - (-alpha)) / 100;

		if (leftRight) {
			blitSurface(LEFT_RIGHT_OFFSET, DARKEN_EFFECT);
		} else {
			blitSurface(NORMAL_OFFSET, DARKEN_EFFECT);
		}

		_dimPercentValue = backUp;
	} else {
		if (leftRight) {
			blitSurface(LEFT_RIGHT_OFFSET, ALPHA_EFFECT);
		} else {
			blitSurface(NORMAL_OFFSET, ALPHA_EFFECT);
		}
	}
#undef blitSurface

#undef NORMAL_OFFSET
#undef LEFT_RIGHT_OFFSET

#undef DARKEN_EFFECT
#undef ALPHA_EFFECT
#undef NO_EFFECT
}

OverlayColor ThemeNew::getColor(kState state) {
	switch (state) {
	case kStateDisabled:
		return _colors[kColorStateDisabled];
		break;

	case kStateHighlight:
		return _colors[kColorStateHighlight];
		break;

	default:
		break;
	};
	return _colors[kColorStateEnabled];
}

void ThemeNew::resetupGuiRenderer() {
	if (_lastUsedBitMask == gBitFormat || !_initOk) {
		// ok same format no need to reload
		return;
	}
	
	_lastUsedBitMask = gBitFormat;
	
	int i;
	for (i = 0; i < kImageHandlesMax; ++i) {
		ImageMan.unregisterSurface(_imageHandles[i]);
	}
	
	for (i = 0; i < kImageHandlesMax; ++i) {
		ImageMan.registerSurface(_imageHandles[i], 0);
		_images[i] = ImageMan.getSurface(_imageHandles[i]);
	}
	
	setupColors();
}

void ThemeNew::setupColors() {
	// load the colors from the config file
	getColorFromConfig(_configFile, "main_dialog_start", _colors[kMainDialogStart]);
	getColorFromConfig(_configFile, "main_dialog_end", _colors[kMainDialogEnd]);
	
	getColorFromConfig(_configFile, "dialog_start", _colors[kDialogStart]);
	getColorFromConfig(_configFile, "dialog_end", _colors[kDialogEnd]);
	
	getColorFromConfig(_configFile, "color_state_disabled", _colors[kColorStateDisabled]);
	getColorFromConfig(_configFile, "color_state_highlight", _colors[kColorStateHighlight]);
	getColorFromConfig(_configFile, "color_state_enabled", _colors[kColorStateEnabled]);
	getColorFromConfig(_configFile, "color_transparency", _colors[kColorTransparency]);
	
	getColorFromConfig(_configFile, "text_inverted_background", _colors[kTextInvertedBackground]);
	getColorFromConfig(_configFile, "text_inverted_color", _colors[kTextInvertedColor]);
	
	getColorFromConfig(_configFile, "widget_bkgd_start", _colors[kWidgetBackgroundStart]);
	getColorFromConfig(_configFile, "widget_bkgd_end", _colors[kWidgetBackgroundEnd]);
	getColorFromConfig(_configFile, "widget_bkgd_small_start", _colors[kWidgetBackgroundSmallStart]);
	getColorFromConfig(_configFile, "widget_bkgd_small_end", _colors[kWidgetBackgroundSmallEnd]);
	
	getColorFromConfig(_configFile, "button_bkgd_start", _colors[kButtonBackgroundStart]);
	getColorFromConfig(_configFile, "button_bkgd_end", _colors[kButtonBackgroundEnd]);
	getColorFromConfig(_configFile, "button_bkgd_highlight_start", _colors[kButtonBackgroundHighlightStart]);
	getColorFromConfig(_configFile, "button_bkgd_highlight_end", _colors[kButtonBackgroundHighlightEnd]);
	getColorFromConfig(_configFile, "button_text_enabled", _colors[kButtonTextEnabled]);
	getColorFromConfig(_configFile, "button_text_disabled", _colors[kButtonTextDisabled]);
	getColorFromConfig(_configFile, "button_text_highlight", _colors[kButtonTextHighlight]);
	
	getColorFromConfig(_configFile, "slider_background_start", _colors[kSliderBackgroundStart]);
	getColorFromConfig(_configFile, "slider_background_end", _colors[kSliderBackgroundEnd]);
	getColorFromConfig(_configFile, "slider_start", _colors[kSliderStart]);
	getColorFromConfig(_configFile, "slider_end", _colors[kSliderEnd]);	
	getColorFromConfig(_configFile, "slider_highlight_start", _colors[kSliderHighStart]);
	getColorFromConfig(_configFile, "slider_highlight_end", _colors[kSliderHighEnd]);
	
	getColorFromConfig(_configFile, "tab_background_start", _colors[kTabBackgroundStart]);
	getColorFromConfig(_configFile, "tab_background_end", _colors[kTabBackgroundEnd]);
	
	getColorFromConfig(_configFile, "scrollbar_background_start", _colors[kScrollbarBackgroundStart]);
	getColorFromConfig(_configFile, "scrollbar_background_end", _colors[kScrollbarBackgroundEnd]);
	getColorFromConfig(_configFile, "scrollbar_button_start", _colors[kScrollbarButtonStart]);
	getColorFromConfig(_configFile, "scrollbar_button_end", _colors[kScrollbarButtonEnd]);
	getColorFromConfig(_configFile, "scrollbar_slider_start", _colors[kScrollbarSliderStart]);
	getColorFromConfig(_configFile, "scrollbar_slider_end", _colors[kScrollbarSliderEnd]);
	getColorFromConfig(_configFile, "scrollbar_button_highlight_start", _colors[kScrollbarButtonHighlightStart]);
	getColorFromConfig(_configFile, "scrollbar_button_highlight_end", _colors[kScrollbarButtonHighlightEnd]);
	getColorFromConfig(_configFile, "scrollbar_slider_highlight_start", _colors[kScrollbarSliderHighlightStart]);
	getColorFromConfig(_configFile, "scrollbar_slider_highlight_end", _colors[kScrollbarSliderHighlightEnd]);
	
	getColorFromConfig(_configFile, "caret_color", _colors[kCaretColor]);

	getColorFromConfig(_configFile, "popupwidget_start", _colors[kPopUpWidgetStart]);
	getColorFromConfig(_configFile, "pupupwidget_end", _colors[kPopUpWidgetEnd]);
	getColorFromConfig(_configFile, "popupwidget_highlight_start", _colors[kPopUpWidgetHighlightStart]);
	getColorFromConfig(_configFile, "popupwidget_highlight_end", _colors[kPopUpWidgetHighlightEnd]);
}

#define FONT_NAME_NORMAL "newgui_normal"
#define FONT_NAME_BOLD "newgui_bold"
#define FONT_NAME_ITALIC "newgui_italic"

void ThemeNew::setupFonts() {
	if (_screen.w >= 400 && _screen.h >= 300) {
		if (_configFile.hasKey("fontfile_bold", "extra")) {
			_fonts[kFontStyleBold] = FontMan.getFontByName(FONT_NAME_BOLD);

			if (!_fonts[kFontStyleBold]) {
				Common::String temp;
				_configFile.getKey("fontfile_bold", "extra", temp);

				_fonts[kFontStyleBold] = loadFont(temp.c_str());
				if (!_fonts[kFontStyleBold])
					error("Couldn't load bold font '%s'", temp.c_str());

				FontMan.assignFontToName(FONT_NAME_BOLD, _fonts[kFontStyleBold]);
			}
		} else {
			_fonts[kFontStyleBold] = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		}

		if (_configFile.hasKey("fontfile_normal", "extra")) {
			_fonts[kFontStyleNormal] = FontMan.getFontByName(FONT_NAME_NORMAL);

			if (!_fonts[kFontStyleNormal]) {
				Common::String temp;
				_configFile.getKey("fontfile_normal", "extra", temp);

				_fonts[kFontStyleNormal] = loadFont(temp.c_str());
				if (!_fonts[kFontStyleNormal])
					error("Couldn't load normal font '%s'", temp.c_str());

				FontMan.assignFontToName(FONT_NAME_NORMAL, _fonts[kFontStyleNormal]);
			}
		} else {
			_fonts[kFontStyleNormal] = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		}

		if (_configFile.hasKey("fontfile_italic", "extra")) {
			_fonts[kFontStyleItalic] = FontMan.getFontByName(FONT_NAME_ITALIC);

			if (!_fonts[kFontStyleItalic]) {
				Common::String temp;
				_configFile.getKey("fontfile_italic", "extra", temp);

				_fonts[kFontStyleNormal] = loadFont(temp.c_str());
				if (!_fonts[kFontStyleItalic])
					error("Couldn't load italic font '%s'", temp.c_str());

				FontMan.assignFontToName(FONT_NAME_ITALIC, _fonts[kFontStyleItalic]);
			}
		} else {
			_fonts[kFontStyleItalic] = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		}
	} else {
		_fonts[kFontStyleBold] = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
		_fonts[kFontStyleNormal] = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
		_fonts[kFontStyleItalic] = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	}
}

void ThemeNew::deleteFonts() {
	const Graphics::Font *normal = FontMan.getFontByName(FONT_NAME_NORMAL);
	const Graphics::Font *bold = FontMan.getFontByName(FONT_NAME_BOLD);
	const Graphics::Font *italic = FontMan.getFontByName(FONT_NAME_ITALIC);

	delete normal;
	delete bold;
	delete italic;

	FontMan.removeFontName(FONT_NAME_NORMAL);
	FontMan.removeFontName(FONT_NAME_BOLD);
	FontMan.removeFontName(FONT_NAME_ITALIC);
}

const Graphics::Font *ThemeNew::loadFont(const char *filename) {
	const Graphics::NewFont *font = 0;
	Common::String cacheFilename = genCacheFilename(filename);
	Common::File fontFile;

	if (cacheFilename != "") {
		if (fontFile.open(cacheFilename))
			font = Graphics::NewFont::loadFromCache(fontFile);
		if (font)
			return font;

#ifdef USE_ZLIB
		unzFile zipFile = unzOpen((_stylefile + ".zip").c_str());
		if (zipFile && unzLocateFile(zipFile, cacheFilename.c_str(), 2) == UNZ_OK) {
			unz_file_info fileInfo;
			unzOpenCurrentFile(zipFile);
			unzGetCurrentFileInfo(zipFile, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
			uint8 *buffer = new uint8[fileInfo.uncompressed_size+1];
			assert(buffer);
			memset(buffer, 0, (fileInfo.uncompressed_size+1)*sizeof(uint8));
			unzReadCurrentFile(zipFile, buffer, fileInfo.uncompressed_size);
			unzCloseCurrentFile(zipFile);
			Common::MemoryReadStream stream(buffer, fileInfo.uncompressed_size+1);
	
			font = Graphics::NewFont::loadFromCache(stream);
	
			delete [] buffer;
			buffer = 0;
		}
		unzClose(zipFile);
#endif
		if (font)
			return font;
	}

	printf("normal font open!\n");
	fflush(stdout);

	// normal open
	if (fontFile.open(filename)) {
		font = Graphics::NewFont::loadFont(fontFile);
	}

#ifdef USE_ZLIB
	if (!font) {
		unzFile zipFile = unzOpen((_stylefile + ".zip").c_str());
		if (zipFile && unzLocateFile(zipFile, filename, 2) == UNZ_OK) {
			unz_file_info fileInfo;
			unzOpenCurrentFile(zipFile);
			unzGetCurrentFileInfo(zipFile, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
			uint8 *buffer = new uint8[fileInfo.uncompressed_size+1];
			assert(buffer);
			memset(buffer, 0, (fileInfo.uncompressed_size+1)*sizeof(uint8));
			unzReadCurrentFile(zipFile, buffer, fileInfo.uncompressed_size);
			unzCloseCurrentFile(zipFile);
			Common::MemoryReadStream stream(buffer, fileInfo.uncompressed_size+1);
	
			font = Graphics::NewFont::loadFont(stream);
	
			delete [] buffer;
			buffer = 0;
		}
		unzClose(zipFile);
	}
#endif

	if (font) {
		if (cacheFilename != "") {
			if (!Graphics::NewFont::cacheFontData(*font, cacheFilename)) {
				warning("Couldn't create cache file for font '%s'", filename);
			}
		}
	}

	return font;
}

Common::String ThemeNew::genCacheFilename(const char *filename) {
	Common::String cacheName = filename;
	for (int i = cacheName.size() - 1; i >= 0; --i) {
		if (cacheName[i] == '.') {
			while ((uint)i < cacheName.size() - 1) {
				cacheName.deleteLastChar();
			}

			cacheName += "fcc";
			return cacheName;
		}
	}

	return "";
}

#pragma mark -

OverlayColor ThemeNew::calcLuminance(OverlayColor col) {
	uint8 r, g, b;
	_system->colorToRGB(col, r, g, b);

	uint lum = (r >> 2) + (g >> 1) + (b >> 3);
	
	return _system->RGBToColor(lum, lum, lum);
}

OverlayColor ThemeNew::calcDimColor(OverlayColor col) {
	uint8 r, g, b;
	_system->colorToRGB(col, r, g, b);
	
	r = r * _dimPercentValue >> 8;
	g = g * _dimPercentValue >> 8;
	b = b * _dimPercentValue >> 8;

	return _system->RGBToColor(r, g, b);
}

#pragma mark -

void ThemeNew::setUpCursor() {
	_system->setCursorPalette(_cursorPal, 0, MAX_CURS_COLORS);
	_system->setMouseCursor(_cursor, _cursorWidth, _cursorHeight, _cursorHotspotX, _cursorHotspotY);
	_system->disableCursorPalette(false);
}

void ThemeNew::createCursor() {
	const Surface *cursor = _images[kGUICursor];

	_cursorWidth = cursor->w;
	_cursorHeight = cursor->h;

	uint colorsFound = 0;
	const OverlayColor *src = (const OverlayColor*)cursor->pixels;

	byte *table = new byte[65536];
	assert(table);
	memset(table, 0, sizeof(byte)*65536);

	byte r, g, b;

	_system->colorToRGB(_colors[kColorTransparency], r, g, b);
	uint16 transparency = RGBToColor<ColorMasks<565> >(r, g, b);

	_cursor = new byte[_cursorWidth * _cursorHeight];
	assert(_cursor);
	memset(_cursor, 255, sizeof(byte)*_cursorWidth*_cursorHeight);

	for (uint y = 0; y < _cursorHeight; ++y) {
		for (uint x = 0; x < _cursorWidth; ++x) {
			_system->colorToRGB(src[x], r, g, b);
			uint16 col = RGBToColor<ColorMasks<565> >(r, g, b);
			if (!table[col] && col != transparency) {
				table[col] = colorsFound++;

				uint index = table[col];
				_cursorPal[index * 4 + 0] = r;
				_cursorPal[index * 4 + 1] = g;
				_cursorPal[index * 4 + 2] = b;
				_cursorPal[index * 4 + 3] = 0xFF;

				if (colorsFound > MAX_CURS_COLORS)
					error("Cursor contains too much colors (%d, but only %d are allowed)", colorsFound, MAX_CURS_COLORS);
			}

			if (col != transparency) {
				uint index = table[col];
				_cursor[y * _cursorWidth + x] = index;
			}
		}
		src += _cursorWidth;
	}
	
	_useCursor = true;
	delete [] table;
}

#pragma mark -

template<class T>
inline OverlayColor getColorAlphaImpl(OverlayColor col1, OverlayColor col2, int alpha) {
	OverlayColor output = 0;
	output |= ((alpha * ((col1 & T::kRedMask) - (col2 & T::kRedMask)) >> 8) + (col2 & T::kRedMask)) & T::kRedMask;
	output |= ((alpha * ((col1 & T::kGreenMask) - (col2 & T::kGreenMask)) >> 8) + (col2 & T::kGreenMask)) & T::kGreenMask;
	output |= ((alpha * ((col1 & T::kBlueMask) - (col2 & T::kBlueMask)) >> 8) + (col2 & T::kBlueMask)) & T::kBlueMask;
	return output;
}

OverlayColor getColorAlpha(OverlayColor col1, OverlayColor col2, int alpha) {
	if (gBitFormat == 565) {
		return getColorAlphaImpl<ColorMasks<565> >(col1, col2, alpha);
	} else {
		return getColorAlphaImpl<ColorMasks<555> >(col1, col2, alpha);
	}
}

template<class T>
inline OverlayColor calcGradient(OverlayColor start, OverlayColor end, int pos) {
	OverlayColor output = 0;
	output |= (start + ((((end & T::kRedMask) - (start & T::kRedMask))) * pos >> 12)) & T::kRedMask;
	output |= (start + ((((end & T::kGreenMask) - (start & T::kGreenMask))) * pos >> 12)) & T::kGreenMask;
	output |= (start + ((((end & T::kBlueMask) - (start & T::kBlueMask))) * pos >> 12)) & T::kBlueMask;
	return output;
}

OverlayColor calcGradient(OverlayColor start, OverlayColor end, int pos, int max, uint factor = 1) {
	max /= factor;
	pos *= factor;
	if (pos >= max)
		return end;

	pos = (0x1000 * pos) / max;

	if (gBitFormat == 565) {
		return calcGradient<ColorMasks<565> >(start, end, pos);
	} else {
		return calcGradient<ColorMasks<555> >(start, end, pos);
	}
}
} // end of namespace GUI

#endif
