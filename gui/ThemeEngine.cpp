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

#include "common/system.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/unzip.h"
#include "common/tokenizer.h"
#include "common/translation.h"
#include "common/unicode-bidi.h"

#include "graphics/conversion.h"
#include "graphics/cursorman.h"
#include "graphics/fontman.h"
#include "graphics/surface.h"
#include "graphics/transparent_surface.h"
#include "graphics/VectorRenderer.h"
#include "graphics/fonts/bdf.h"
#include "graphics/fonts/ttf.h"

#include "image/bmp.h"
#include "image/png.h"

#include "gui/widget.h"
#include "gui/ThemeEngine.h"
#include "gui/ThemeEval.h"
#include "gui/ThemeParser.h"

namespace GUI {

const char *const ThemeEngine::kImageLogo = "logo.bmp";
const char *const ThemeEngine::kImageLogoSmall = "logo_small.bmp";
const char *const ThemeEngine::kImageSearch = "search.bmp";
const char *const ThemeEngine::kImageEraser = "eraser.bmp";
const char *const ThemeEngine::kImageDelButton = "delbtn.bmp";
const char *const ThemeEngine::kImageList = "list.bmp";
const char *const ThemeEngine::kImageGrid = "grid.bmp";
const char *const ThemeEngine::kImageStopButton = "stopbtn.bmp";
const char *const ThemeEngine::kImageEditButton = "editbtn.bmp";
const char *const ThemeEngine::kImageSwitchModeButton = "switchbtn.bmp";
const char *const ThemeEngine::kImageFastReplayButton = "fastreplay.bmp";
const char *const ThemeEngine::kImageStopSmallButton = "stopbtn_small.bmp";
const char *const ThemeEngine::kImageEditSmallButton = "editbtn_small.bmp";
const char *const ThemeEngine::kImageSwitchModeSmallButton = "switchbtn_small.bmp";
const char *const ThemeEngine::kImageFastReplaySmallButton = "fastreplay_small.bmp";

struct TextDrawData {
	const Graphics::Font *_fontPtr;
};

struct TextColorData {
	int r, g, b;
};

struct WidgetDrawData {
	/** List of all the steps needed to draw this widget */
	Common::List<Graphics::DrawStep> _steps;

	TextData _textDataId;
	TextColor _textColorId;
	Graphics::TextAlign _textAlignH;
	GUI::ThemeEngine::TextAlignVertical _textAlignV;

	/** Extra space that the widget occupies when it's drawn.
	    E.g. when taking into account rounded corners, drop shadows, etc
	    Used when restoring the widget background */
	uint16 _backgroundOffset;
	uint16 _shadowOffset;

	DrawLayer _layer;


	/**
	 * Calculates the background threshold offset of a given DrawData item.
	 * After fully loading all DrawSteps of a DrawData item, this function must be
	 * called in order to calculate if such draw steps would be drawn outside of
	 * the actual widget drawing zone (e.g. shadows). If this is the case, a constant
	 * value will be added when restoring the background of the widget.
	 */
	void calcBackgroundOffset();
};

/**********************************************************
 *  Data definitions for theme engine elements
 *********************************************************/
struct DrawDataInfo {
	DrawData id;        ///< The actual ID of the DrawData item.
	const char *name;   ///< The name of the DrawData item as it appears in the Theme Description files
	DrawLayer layer;    ///< Sets whether this item is part of the foreground or background layer of its dialog
	DrawData parent;    ///< Parent DrawData item, for items that overlay. E.g. kDDButtonIdle -> kDDButtonHover
};

/**
 * Default values for each DrawData item.
 */
static const DrawDataInfo kDrawDataDefaults[] = {
	{kDDMainDialogBackground,         "mainmenu_bg",          kDrawLayerBackground,   kDDNone},
	{kDDSpecialColorBackground,       "special_bg",           kDrawLayerBackground,   kDDNone},
	{kDDPlainColorBackground,         "plain_bg",             kDrawLayerBackground,   kDDNone},
	{kDDTooltipBackground,            "tooltip_bg",           kDrawLayerBackground,   kDDNone},
	{kDDDefaultBackground,            "default_bg",           kDrawLayerBackground,   kDDNone},
	{kDDTextSelectionBackground,      "text_selection",       kDrawLayerForeground,  kDDNone},
	{kDDTextSelectionFocusBackground, "text_selection_focus", kDrawLayerForeground,  kDDNone},

	{kDDWidgetBackgroundDefault,    "widget_default",   kDrawLayerBackground,   kDDNone},
	{kDDWidgetBackgroundSmall,      "widget_small",     kDrawLayerBackground,   kDDNone},
	{kDDWidgetBackgroundEditText,   "widget_textedit",  kDrawLayerBackground,   kDDNone},
	{kDDWidgetBackgroundSlider,     "widget_slider",    kDrawLayerBackground,   kDDNone},

	{kDDButtonIdle,                 "button_idle",      kDrawLayerBackground,   kDDNone},
	{kDDButtonHover,                "button_hover",     kDrawLayerForeground,   kDDButtonIdle},
	{kDDButtonDisabled,             "button_disabled",  kDrawLayerBackground,   kDDNone},
	{kDDButtonPressed,              "button_pressed",   kDrawLayerForeground,   kDDButtonIdle},

	{kDDDropDownButtonIdle,         "dropdown_button_idle",          kDrawLayerBackground, kDDNone},
	{kDDDropDownButtonHoverLeft,    "dropdown_button_hover_left",    kDrawLayerForeground, kDDDropDownButtonIdle},
	{kDDDropDownButtonHoverRight,   "dropdown_button_hover_right",   kDrawLayerForeground, kDDDropDownButtonIdle},
	{kDDDropDownButtonDisabled,     "dropdown_button_disabled",      kDrawLayerForeground, kDDNone},
	{kDDDropDownButtonPressedLeft,  "dropdown_button_pressed_left",  kDrawLayerForeground, kDDDropDownButtonIdle},
	{kDDDropDownButtonPressedRight, "dropdown_button_pressed_right", kDrawLayerForeground, kDDDropDownButtonIdle},

	{kDDDropDownButtonIdleRTL,			"dropdown_button_idle_rtl",				kDrawLayerBackground, kDDNone},
	{kDDDropDownButtonHoverLeftRTL,		"dropdown_button_hover_left_rtl",		kDrawLayerForeground, kDDDropDownButtonIdleRTL},
	{kDDDropDownButtonHoverRightRTL,	"dropdown_button_hover_right_rtl",		kDrawLayerForeground, kDDDropDownButtonIdleRTL},
	{kDDDropDownButtonDisabledRTL,		"dropdown_button_disabled_rtl",			kDrawLayerForeground, kDDNone},
	{kDDDropDownButtonPressedLeftRTL,	"dropdown_button_pressed_left_rtl",		kDrawLayerForeground, kDDDropDownButtonIdleRTL},
	{kDDDropDownButtonPressedRightRTL,	"dropdown_button_pressed_right_rtl",	kDrawLayerForeground, kDDDropDownButtonIdleRTL},

	{kDDSliderFull,                 "slider_full",      kDrawLayerForeground,  kDDNone},
	{kDDSliderHover,                "slider_hover",     kDrawLayerForeground,  kDDNone},
	{kDDSliderDisabled,             "slider_disabled",  kDrawLayerForeground,  kDDNone},

	{kDDCheckboxDefault,            "checkbox_default",           kDrawLayerBackground,   kDDNone},
	{kDDCheckboxDisabled,           "checkbox_disabled",          kDrawLayerBackground,   kDDNone},
	{kDDCheckboxSelected,           "checkbox_selected",          kDrawLayerForeground,  kDDCheckboxDefault},
	{kDDCheckboxDisabledSelected,   "checkbox_disabled_selected", kDrawLayerForeground,  kDDCheckboxDisabled},

	{kDDRadiobuttonDefault,         "radiobutton_default",      kDrawLayerBackground,   kDDNone},
	{kDDRadiobuttonDisabled,        "radiobutton_disabled",     kDrawLayerBackground,   kDDNone},
	{kDDRadiobuttonSelected,        "radiobutton_selected",     kDrawLayerForeground,  kDDRadiobuttonDefault},

	{kDDTabActive,                  "tab_active",               kDrawLayerForeground,  kDDTabInactive},
	{kDDTabInactive,                "tab_inactive",             kDrawLayerBackground,   kDDNone},
	{kDDTabBackground,              "tab_background",           kDrawLayerBackground,   kDDNone},

	{kDDScrollbarBase,              "scrollbar_base",           kDrawLayerBackground,   kDDNone},

	{kDDScrollbarButtonIdle,        "scrollbar_button_idle",    kDrawLayerBackground,   kDDNone},
	{kDDScrollbarButtonHover,       "scrollbar_button_hover",   kDrawLayerForeground,  kDDScrollbarButtonIdle},

	{kDDScrollbarHandleIdle,        "scrollbar_handle_idle",    kDrawLayerForeground,  kDDNone},
	{kDDScrollbarHandleHover,       "scrollbar_handle_hover",   kDrawLayerForeground,  kDDScrollbarBase},

	{kDDPopUpIdle,                  "popup_idle",       kDrawLayerBackground,   kDDNone},
	{kDDPopUpHover,                 "popup_hover",      kDrawLayerForeground,  kDDPopUpIdle},
	{kDDPopUpDisabled,              "popup_disabled",   kDrawLayerBackground,   kDDNone},

	{kDDPopUpIdleRTL,				"popup_idle_rtl",		kDrawLayerBackground,   kDDNone},
	{kDDPopUpHoverRTL,              "popup_hover_rtl",		kDrawLayerForeground,	kDDPopUpIdleRTL},
	{kDDPopUpDisabledRTL,           "popup_disabled_rtl",   kDrawLayerBackground,   kDDNone},

	{kDDCaret,                      "caret",        kDrawLayerForeground,  kDDNone},
	{kDDSeparator,                  "separator",    kDrawLayerBackground,   kDDNone},
};

/**********************************************************
 * ThemeEngine class
 *********************************************************/
ThemeEngine::ThemeEngine(Common::String id, GraphicsMode mode) :
	_system(nullptr), _vectorRenderer(nullptr),
	_layerToDraw(kDrawLayerBackground), _bytesPerPixel(0),  _graphicsMode(kGfxDisabled),
	_font(nullptr), _initOk(false), _themeOk(false), _enabled(false), _themeFiles(),
	_cursor(nullptr) {

	_system = g_system;
	_parser = new ThemeParser(this);
	_themeEval = new GUI::ThemeEval();

	_useCursor = false;

	for (int i = 0; i < kDrawDataMAX; ++i) {
		_widgets[i] = nullptr;
	}

	for (int i = 0; i < kTextDataMAX; ++i) {
		_texts[i] = nullptr;
	}

	for (int i = 0; i < kTextColorMAX; ++i) {
		_textColors[i] = nullptr;
	}

	// We currently allow two different ways of theme selection in our config file:
	// 1) Via full path
	// 2) Via a basename, which will need to be translated into a full path
	// This function assures we have a correct path to pass to the ThemeEngine
	// constructor.
	_themeFile = getThemeFile(id);
	// We will use getThemeId to retrive the theme id from the given filename
	// here, since the user could have passed a fixed filename as 'id'.
	_themeId = getThemeId(_themeFile);

	_graphicsMode = mode;
	_themeArchive = nullptr;
	_initOk = false;

	_cursorHotspotX = _cursorHotspotY = 0;
	_cursorWidth = _cursorHeight = 0;
#ifndef USE_RGB_COLOR
	_cursorFormat = Graphics::PixelFormat::createFormatCLUT8();
	_cursorTransparent = 255;
	_cursorPalSize = 0;
#endif

	// We prefer files in archive bundles over the common search paths.
	_themeFiles.add("default", &SearchMan, 0, false);
}

ThemeEngine::~ThemeEngine() {
	delete _vectorRenderer;
	_vectorRenderer = nullptr;
	_screen.free();
	_backBuffer.free();

	unloadTheme();
	unloadExtraFont();

	// Release all graphics surfaces
	for (ImagesMap::iterator i = _bitmaps.begin(); i != _bitmaps.end(); ++i) {
		Graphics::Surface *surf = i->_value;
		if (surf) {
			surf->free();
			delete surf;
		}
	}
	_bitmaps.clear();

	for (AImagesMap::iterator i = _abitmaps.begin(); i != _abitmaps.end(); ++i) {
		Graphics::TransparentSurface *surf = i->_value;
		if (surf) {
			surf->free();
			delete surf;
		}
	}
	_abitmaps.clear();

	delete _parser;
	delete _themeEval;
	delete[] _cursor;
}



/**********************************************************
 * Rendering mode management
 *********************************************************/
const ThemeEngine::Renderer ThemeEngine::_rendererModes[] = {
	{ _s("Disabled GFX"), _sc("Disabled GFX", "lowres"), "none", kGfxDisabled },
	{ _s("Standard renderer"), _s("Standard"), "normal", kGfxStandard },
#ifndef DISABLE_FANCY_THEMES
	{ _s("Antialiased renderer"), _s("Antialiased"), "antialias", kGfxAntialias }
#endif
};

const uint ThemeEngine::_rendererModesSize = ARRAYSIZE(ThemeEngine::_rendererModes);

const ThemeEngine::GraphicsMode ThemeEngine::_defaultRendererMode =
#ifndef DISABLE_FANCY_THEMES
	ThemeEngine::kGfxAntialias;
#else
	ThemeEngine::kGfxStandard;
#endif

ThemeEngine::GraphicsMode ThemeEngine::findMode(const Common::String &cfg) {
	for (uint i = 0; i < _rendererModesSize; ++i) {
		if (cfg.equalsIgnoreCase(_rendererModes[i].cfg))
			return _rendererModes[i].mode;
	}

	return kGfxDisabled;
}

const char *ThemeEngine::findModeConfigName(GraphicsMode mode) {
	for (uint i = 0; i < _rendererModesSize; ++i) {
		if (mode == _rendererModes[i].mode)
			return _rendererModes[i].cfg;
	}

	return findModeConfigName(kGfxDisabled);
}





/**********************************************************
 * Theme setup/initialization
 *********************************************************/
bool ThemeEngine::init() {
	// reset everything and reload the graphics
	_initOk = false;
	_overlayFormat = _system->getOverlayFormat();
	setGraphicsMode(_graphicsMode);

	if (_screen.getPixels() && _backBuffer.getPixels()) {
		_initOk = true;
	}

	// TODO: Instead of hard coding the font here, it should be possible
	// to specify the fonts to be used for each resolution in the theme XML.
	if (_screen.w >= 400 && _screen.h >= 300) {
		_font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	} else {
		_font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	}

	// Try to create a Common::Archive with the files of the theme.
	if (!_themeArchive && !_themeFile.empty()) {
		Common::FSNode node(_themeFile);
		if (node.isDirectory()) {
			_themeArchive = new Common::FSDirectory(node);
		} else if (_themeFile.matchString("*.zip", true)) {
			// TODO: Also use "node" directly?
			// Look for the zip file via SearchMan
			Common::ArchiveMemberPtr member = SearchMan.getMember(_themeFile);
			if (member) {
				_themeArchive = Common::makeZipArchive(member->createReadStream());
				if (!_themeArchive) {
					warning("Failed to open Zip archive '%s'.", member->getDisplayName().c_str());
				}
			} else {
				_themeArchive = Common::makeZipArchive(node);
				if (!_themeArchive) {
					warning("Failed to open Zip archive '%s'.", node.getPath().c_str());
				}
			}
		}

		if (_themeArchive)
			_themeFiles.add("theme_archive", _themeArchive, 1, true);
	}

	// Load the theme
	// We pass the theme file here by default, so the user will
	// have a descriptive error message. The only exception will
	// be the builtin theme which has no filename.
	loadTheme(_themeFile.empty() ? _themeId : _themeFile);

	return ready();
}

void ThemeEngine::clearAll() {
	if (_initOk) {
		_system->clearOverlay();
		_system->grabOverlay(_backBuffer.getPixels(), _backBuffer.pitch);
	}
}

void ThemeEngine::refresh() {

	// Flush all bitmaps if the overlay pixel format changed.
	if (_overlayFormat != _system->getOverlayFormat()) {
		for (ImagesMap::iterator i = _bitmaps.begin(); i != _bitmaps.end(); ++i) {
			Graphics::Surface *surf = i->_value;
			if (surf) {
				surf->free();
				delete surf;
			}
		}
		_bitmaps.clear();

		for (AImagesMap::iterator i = _abitmaps.begin(); i != _abitmaps.end(); ++i) {
			Graphics::TransparentSurface *surf = i->_value;
			if (surf) {
				surf->free();
				delete surf;
			}
		}
		_abitmaps.clear();
	}

	init();

	if (_enabled) {
		_system->showOverlay();

		if (_useCursor) {
#ifndef USE_RGB_COLOR
			CursorMan.replaceCursorPalette(_cursorPal, 0, _cursorPalSize);
#endif
			CursorMan.replaceCursor(_cursor, _cursorWidth, _cursorHeight, _cursorHotspotX, _cursorHotspotY, _cursorTransparent, true, &_cursorFormat);
		}
	}
}

void ThemeEngine::enable() {
	if (_enabled)
		return;

	showCursor();

	_system->showOverlay();
	clearAll();
	_enabled = true;
}

void ThemeEngine::disable() {
	if (!_enabled)
		return;

	_system->hideOverlay();

	hideCursor();


	_enabled = false;
}

void ThemeEngine::setGraphicsMode(GraphicsMode mode) {
	switch (mode) {
	case kGfxStandard:
#ifndef DISABLE_FANCY_THEMES
	case kGfxAntialias:
#endif
		if (g_system->getOverlayFormat().bytesPerPixel == 4) {
			_bytesPerPixel = sizeof(uint32);
			break;
		} else if (g_system->getOverlayFormat().bytesPerPixel == 2) {
			_bytesPerPixel = sizeof(uint16);
			break;
		}
		// fall through
	default:
		error("Invalid graphics mode");
	}

	uint32 width = _system->getOverlayWidth();
	uint32 height = _system->getOverlayHeight();

	_backBuffer.free();
	_backBuffer.create(width, height, _overlayFormat);

	_screen.free();
	_screen.create(width, height, _overlayFormat);

	delete _vectorRenderer;
	_vectorRenderer = Graphics::createRenderer(mode);
	_vectorRenderer->setSurface(&_screen);

	// Since we reinitialized our screen surfaces we know nothing has been
	// drawn so far. Sometimes we still end up with dirty screen bits in the
	// list. Clearing it avoids invalid overlay writes when the backend
	// resizes the overlay.
	_dirtyScreen.clear();
}

void WidgetDrawData::calcBackgroundOffset() {
	uint maxShadow = 0, maxBevel = 0;
	for (Common::List<Graphics::DrawStep>::const_iterator step = _steps.begin();
	        step != _steps.end(); ++step) {
		if ((step->autoWidth || step->autoHeight) && step->shadow > maxShadow)
			maxShadow = step->shadow;

		if (step->drawingCall == &Graphics::VectorRenderer::drawCallback_BEVELSQ && step->bevel > maxBevel)
			maxBevel = step->bevel;
	}

	_backgroundOffset = maxBevel;
	_shadowOffset = maxShadow;
}

void ThemeEngine::restoreBackground(Common::Rect r) {
	if (_vectorRenderer->getActiveSurface() == &_backBuffer) {
		// Only restore the background when drawing to the screen surface
		return;
	}

	r.clip(_screen.w, _screen.h);
	_vectorRenderer->blitSurface(&_backBuffer, r);

	addDirtyRect(r);
}



/**********************************************************
 * Theme elements management
 *********************************************************/
void ThemeEngine::addDrawStep(const Common::String &drawDataId, const Graphics::DrawStep &step) {
	DrawData id = parseDrawDataId(drawDataId);

	assert(id != kDDNone && _widgets[id] != nullptr);
	_widgets[id]->_steps.push_back(step);
}

bool ThemeEngine::addTextData(const Common::String &drawDataId, TextData textId, TextColor colorId, Graphics::TextAlign alignH, TextAlignVertical alignV) {
	DrawData id = parseDrawDataId(drawDataId);

	if (id == -1 || textId == -1 || colorId == kTextColorMAX || !_widgets[id])
		return false;

	_widgets[id]->_textDataId = textId;
	_widgets[id]->_textColorId = colorId;
	_widgets[id]->_textAlignH = alignH;
	_widgets[id]->_textAlignV = alignV;

	return true;
}

bool ThemeEngine::addFont(TextData textId, const Common::String &language, const Common::String &file, const Common::String &scalableFile, const int pointsize) {
	if (textId == -1)
		return false;

	if (!language.empty()) {
#ifdef USE_TRANSLATION
		Common::String cl = TransMan.getCurrentLanguage();
		if (!cl.matchString(language, true))
			return true;	// Skip

		if (_texts[textId] != nullptr)	// We already loaded something
			return true;
#else
		return true;	// Safely ignore
#endif
	}

	if (_texts[textId] != nullptr)
		delete _texts[textId];

	_texts[textId] = new TextDrawData;

	if (file == "default") {
		_texts[textId]->_fontPtr = _font;
	} else {
		Common::String localized = FontMan.genLocalizedFontFilename(file);
		const Common::String charset
#ifdef USE_TRANSLATION
		                            (textId == kTextDataExtraLang ? "" : TransMan.getCurrentCharset())
#endif
		                            ;

		// Try localized fonts
		_texts[textId]->_fontPtr = loadFont(localized, scalableFile, charset, pointsize, textId == kTextDataDefault);

		if (!_texts[textId]->_fontPtr) {
			warning("Failed to load localized font '%s'", localized.c_str());
			// Try standard fonts
			_texts[textId]->_fontPtr = loadFont(file, scalableFile, Common::String(), pointsize, textId == kTextDataDefault);

			if (!_texts[textId]->_fontPtr) {
				error("Couldn't load font '%s'/'%s'", file.c_str(), scalableFile.c_str());
#ifdef USE_TRANSLATION
				TransMan.setLanguage("C");
#ifdef USE_TTS
				Common::TextToSpeechManager *ttsMan;
				if ((ttsMan = g_system->getTextToSpeechManager()) != nullptr)
					ttsMan->setLanguage("en");
#endif // USE_TTS
#endif // USE_TRANSLATION
				return false; // fall-back attempt failed
			}
			// Success in fall-back attempt to standard (non-localized) font.
			// However, still returns false here, probably to avoid ugly / garbage glyphs side-effects
			// FIXME If we return false anyway why would we attempt the fall-back in the first place?
#ifdef USE_TRANSLATION
			TransMan.setLanguage("C");
#ifdef USE_TTS
				Common::TextToSpeechManager *ttsMan;
				if ((ttsMan = g_system->getTextToSpeechManager()) != nullptr)
					ttsMan->setLanguage("en");
#endif // USE_TTS
#endif // USE_TRANSLATION
			// Returning true here, would allow falling back to standard fonts for the missing ones,
			// but that leads to "garbage" glyphs being displayed on screen for non-Latin languages
			return false;
		}
	}

	return true;

}

// This should work independently of the translation manager, since it is is just about displaying
// correct ingame messages in Chinese, Korean and Japanese games, even if the launcher is set to
// English and/or the translations are disabled.
Common::Array<Common::Language> getLangIdentifiers(const Common::String &language) {
	struct IdStr2Lang {
		const char strId[6];
		Common::Language lang;
	};

	// I have added only the languages that currently make sense (the only other extra font that we
	// currently have is for Hindi, but that language is only supported in the launcher, there are
	// no games in Hindi).
	IdStr2Lang matchPairs[] = {
	//  { "hi", Common::UNK_LANG },
		{ "ja", Common::JA_JPN },
		{ "ko", Common::KO_KOR },
		{ "zh", Common::ZH_ANY },
		{ "zh", Common::ZH_CNA },
		{ "zh", Common::ZH_TWN }
	};

	Common::Array<Common::Language> result;

	for (int i = 0; i < ARRAYSIZE(matchPairs); ++i) {
		if (language.contains(matchPairs[i].strId))
			result.push_back(matchPairs[i].lang);
	}

	return result;
}

void ThemeEngine::storeFontNames(TextData textId, const Common::String &language, const Common::String &file, const Common::String &scalableFile, const int pointsize) {
	if (language.empty())
		return;

	Common::Array<Common::Language> langs = getLangIdentifiers(language);
	if (langs.empty())
		return;

	Common::Array<LangExtraFont>::iterator entry = Common::find(_langExtraFonts.begin(), _langExtraFonts.end(), langs[0]);
	if (entry == _langExtraFonts.end())
		_langExtraFonts.push_back(LangExtraFont(textId, langs, file, scalableFile, pointsize));
	else
		entry->storeFileNames(textId, file, scalableFile, pointsize);
}

bool ThemeEngine::loadExtraFont(FontStyle style, Common::Language lang) {
	if (style >= kFontStyleMax)
		return false;

	Common::Array<LangExtraFont>::iterator entry = Common::find(_langExtraFonts.begin(), _langExtraFonts.end(), lang);
	if (entry == _langExtraFonts.end())
		return false;
	TextData td = fontStyleToData(style);
	return addFont(kTextDataExtraLang, Common::String(), entry->file(td), entry->sclFile(td), entry->fntSize(td));
}

bool ThemeEngine::addTextColor(TextColor colorId, int r, int g, int b) {
	if (colorId >= kTextColorMAX)
		return false;

	if (_textColors[colorId] != nullptr)
		delete _textColors[colorId];

	_textColors[colorId] = new TextColorData;

	_textColors[colorId]->r = r;
	_textColors[colorId]->g = g;
	_textColors[colorId]->b = b;

	return true;
}

bool ThemeEngine::addBitmap(const Common::String &filename) {
	// Nothing has to be done if the bitmap already has been loaded.
	Graphics::Surface *surf = _bitmaps[filename];
	if (surf)
		return true;

	const Graphics::Surface *srcSurface = nullptr;

	if (filename.hasSuffix(".png")) {
		// Maybe it is PNG?
#ifdef USE_PNG
		Image::PNGDecoder decoder;
		Common::ArchiveMemberList members;
		_themeFiles.listMatchingMembers(members, filename);
		for (Common::ArchiveMemberList::const_iterator i = members.begin(), end = members.end(); i != end; ++i) {
			Common::SeekableReadStream *stream = (*i)->createReadStream();
			if (stream) {
				if (!decoder.loadStream(*stream))
					error("Error decoding PNG");

				srcSurface = decoder.getSurface();
				delete stream;
				if (srcSurface)
					break;
			}
		}

		if (srcSurface && srcSurface->format.bytesPerPixel != 1)
			surf = srcSurface->convertTo(_overlayFormat);
#else
		error("No PNG support compiled in");
#endif
	} else {
		// If not, try to load the bitmap via the BitmapDecoder class.
		Image::BitmapDecoder bitmapDecoder;
		Common::ArchiveMemberList members;
		_themeFiles.listMatchingMembers(members, filename);
		for (Common::ArchiveMemberList::const_iterator i = members.begin(), end = members.end(); i != end; ++i) {
			Common::SeekableReadStream *stream = (*i)->createReadStream();
			if (stream) {
				bitmapDecoder.loadStream(*stream);
				srcSurface = bitmapDecoder.getSurface();
				delete stream;
				if (srcSurface)
					break;
			}
		}

		if (srcSurface && srcSurface->format.bytesPerPixel != 1)
			surf = srcSurface->convertTo(_overlayFormat);
	}

	// Store the surface into our hashmap (attention, may store NULL entries!)
	_bitmaps[filename] = surf;

	return surf != nullptr;
}

bool ThemeEngine::addAlphaBitmap(const Common::String &filename) {
	// Nothing has to be done if the bitmap already has been loaded.
	Graphics::TransparentSurface *surf = _abitmaps[filename];
	if (surf)
		return true;

#ifdef USE_PNG
	const Graphics::TransparentSurface *srcSurface = nullptr;
#endif

	if (filename.hasSuffix(".png")) {
		// Maybe it is PNG?
#ifdef USE_PNG
		Image::PNGDecoder decoder;
		Common::ArchiveMemberList members;
		_themeFiles.listMatchingMembers(members, filename);
		for (Common::ArchiveMemberList::const_iterator i = members.begin(), end = members.end(); i != end; ++i) {
			Common::SeekableReadStream *stream = (*i)->createReadStream();
			if (stream) {
				if (!decoder.loadStream(*stream))
					error("Error decoding PNG");

				srcSurface = new Graphics::TransparentSurface(*decoder.getSurface(), true);
				delete stream;
				if (srcSurface)
					break;
			}
		}

		if (srcSurface && srcSurface->format.bytesPerPixel != 1)
			surf = srcSurface->convertTo(_overlayFormat);
#else
		error("No PNG support compiled in");
#endif
	} else {
		error("Only PNG is supported as alphabitmap");
	}

	// Store the surface into our hashmap (attention, may store NULL entries!)
	_abitmaps[filename] = surf;

	return surf != nullptr;
}

bool ThemeEngine::addDrawData(const Common::String &data, bool cached) {
	DrawData id = parseDrawDataId(data);

	if (id == -1)
		return false;

	if (_widgets[id] != nullptr)
		delete _widgets[id];

	_widgets[id] = new WidgetDrawData;
	_widgets[id]->_layer = kDrawDataDefaults[id].layer;
	_widgets[id]->_textDataId = kTextDataNone;

	return true;
}


/**********************************************************
 * Theme XML loading
 *********************************************************/
void ThemeEngine::loadTheme(const Common::String &themeId) {
	unloadTheme();

	debug(6, "Loading theme %s", themeId.c_str());

	if (themeId == "builtin") {
		_themeOk = loadDefaultXML();
	} else {
		// Load the archive containing image and XML data
		_themeOk = loadThemeXML(themeId);
	}

	if (!_themeOk) {
		warning("Failed to load theme '%s'", themeId.c_str());
		return;
	}

	for (int i = 0; i < kDrawDataMAX; ++i) {
		if (_widgets[i] == nullptr) {
			warning("Missing data asset: '%s'", kDrawDataDefaults[i].name);
		} else {
			_widgets[i]->calcBackgroundOffset();
		}
	}
}

void ThemeEngine::unloadTheme() {
	if (!_themeOk)
		return;

	for (int i = 0; i < kDrawDataMAX; ++i) {
		delete _widgets[i];
		_widgets[i] = nullptr;
	}

	for (int i = 0; i < kTextDataMAX; ++i) {
		// Don't unload the language specific extra font here or it will be lost after a refresh() call.
		if (i == kTextDataExtraLang)
			continue;
		delete _texts[i];
		_texts[i] = nullptr;
	}

	for (int i = 0; i < kTextColorMAX; ++i) {
		delete _textColors[i];
		_textColors[i] = nullptr;
	}

	_themeEval->reset();
	_themeOk = false;
}

void ThemeEngine::unloadExtraFont() {
	delete _texts[kTextDataExtraLang];
	_texts[kTextDataExtraLang] = nullptr;
}

bool ThemeEngine::loadDefaultXML() {

	// The default XML theme is included on runtime from a pregenerated
	// file inside the themes directory.
	// Use the Python script "makedeftheme.py" to convert a normal XML theme
	// into the "default.inc" file, which is ready to be included in the code.
#ifndef DISABLE_GUI_BUILTIN_THEME
#include "themes/default.inc"
	int xmllen = 0;

	for (int i = 0; i < ARRAYSIZE(defaultXML); i++)
		xmllen += strlen(defaultXML[i]);

	byte *tmpXML = (byte *)malloc(xmllen + 1);
	tmpXML[0] = '\0';

	for (int i = 0; i < ARRAYSIZE(defaultXML); i++)
		strncat((char *)tmpXML, defaultXML[i], xmllen);

	if (!_parser->loadBuffer(tmpXML, xmllen)) {
		free(tmpXML);

		return false;
	}

	_themeName = "ScummVM Classic Theme (Builtin Version)";
	_themeId = "builtin";
	_themeFile.clear();

	bool result = _parser->parse();
	_parser->close();

	free(tmpXML);

	return result;
#else
	warning("The built-in theme is not enabled in the current build. Please load an external theme");
	return false;
#endif
}

bool ThemeEngine::loadThemeXML(const Common::String &themeId) {
	assert(_parser);
	assert(_themeArchive);

	_themeName.clear();


	//
	// Now that we have a Common::Archive, verify that it contains a valid THEMERC File
	//
	Common::File themercFile;
	themercFile.open("THEMERC", *_themeArchive);
	if (!themercFile.isOpen()) {
		warning("Theme '%s' contains no 'THEMERC' file.", themeId.c_str());
		return false;
	}

	Common::String stxHeader = themercFile.readLine();
	if (!themeConfigParseHeader(stxHeader, _themeName) || _themeName.empty()) {
		warning("Corrupted 'THEMERC' file in theme '%s'", themeId.c_str());
		return false;
	}

	Common::ArchiveMemberList members;
	if (0 == _themeArchive->listMatchingMembers(members, "*.stx")) {
		warning("Found no STX files for theme '%s'.", themeId.c_str());
		return false;
	}

	//
	// Loop over all STX files, load and parse them
	//
	for (Common::ArchiveMemberList::iterator i = members.begin(); i != members.end(); ++i) {
		assert((*i)->getName().hasSuffix(".stx"));

		if (_parser->loadStream((*i)->createReadStream()) == false) {
			warning("Failed to load STX file '%s'", (*i)->getDisplayName().c_str());
			_parser->close();
			return false;
		}

		if (_parser->parse() == false) {
			warning("Failed to parse STX file '%s'", (*i)->getDisplayName().c_str());
			_parser->close();
			return false;
		}

		_parser->close();
	}

	assert(!_themeName.empty());
	return true;
}



/**********************************************************
 * Draw Date descriptors drawing functions
 *********************************************************/
void ThemeEngine::drawDD(DrawData type, const Common::Rect &r, uint32 dynamic, bool forceRestore) {
	WidgetDrawData *drawData = _widgets[type];

	if (!drawData)
		return;

	if (kDrawDataDefaults[type].parent != kDDNone && kDrawDataDefaults[type].parent != type)
		drawDD(kDrawDataDefaults[type].parent, r, dynamic);

	Common::Rect area = r;
	area.clip(_screen.w, _screen.h);

	Common::Rect extendedRect = area;
	extendedRect.grow(kDirtyRectangleThreshold + drawData->_backgroundOffset);
	if (drawData->_shadowOffset > drawData->_backgroundOffset) {
		extendedRect.right += drawData->_shadowOffset - drawData->_backgroundOffset;
		extendedRect.bottom += drawData->_shadowOffset - drawData->_backgroundOffset;
	}

	if (!_clip.isEmpty()) {
		extendedRect.clip(_clip);
	}

	// Cull the elements not in the clip rect
	if (extendedRect.isEmpty()) {
		return;
	}

	if (forceRestore || drawData->_layer == kDrawLayerBackground)
		restoreBackground(extendedRect);

	if (drawData->_layer == _layerToDraw) {
		Common::List<Graphics::DrawStep>::const_iterator step;
		for (step = drawData->_steps.begin(); step != drawData->_steps.end(); ++step) {
			_vectorRenderer->drawStep(area, _clip, *step, dynamic);
		}

		addDirtyRect(extendedRect);
	}
}

void ThemeEngine::drawDDText(TextData type, TextColor color, const Common::Rect &r, const Common::U32String &text,
	bool restoreBg, bool ellipsis, Graphics::TextAlign alignH, TextAlignVertical alignV,
	int deltax, const Common::Rect &drawableTextArea) {

	if (type == kTextDataNone || !_texts[type] || _layerToDraw == kDrawLayerBackground)
		return;

	Common::Rect area = r;
	area.clip(_screen.w, _screen.h);

	Common::Rect dirty = drawableTextArea;
	if (dirty.isEmpty()) dirty = area;
	else dirty.clip(area);

	if (!_clip.isEmpty()) {
		dirty.clip(_clip);
	}

	// HACK: One small pixel should be invisible enough
	if (dirty.isEmpty()) dirty = Common::Rect(0, 0, 1, 1);

	if (restoreBg)
		restoreBackground(dirty);

	_vectorRenderer->setFgColor(_textColors[color]->r, _textColors[color]->g, _textColors[color]->b);
#ifdef USE_FRIBIDI
	_vectorRenderer->drawString(_texts[type]->_fontPtr, Common::convertBiDiU32String(text), area, alignH, alignV, deltax, ellipsis, dirty);
#else
	_vectorRenderer->drawString(_texts[type]->_fontPtr, text, area, alignH, alignV, deltax, ellipsis, dirty);
#endif

	addDirtyRect(dirty);
}

/**********************************************************
 * Widget drawing functions
 *********************************************************/
void ThemeEngine::drawButton(const Common::Rect &r, const Common::U32String &str, WidgetStateInfo state, uint16 hints) {
	if (!ready())
		return;

	DrawData dd = kDDButtonIdle;

	if (state == kStateEnabled)
		dd = kDDButtonIdle;
	else if (state == kStateHighlight)
		dd = kDDButtonHover;
	else if (state == kStateDisabled)
		dd = kDDButtonDisabled;
	else if (state == kStatePressed)
		dd = kDDButtonPressed;

	drawDD(dd, r, 0, hints & WIDGET_CLEARBG);
	drawDDText(getTextData(dd), getTextColor(dd), r, str, false, true, convertTextAlignH(_widgets[dd]->_textAlignH, false),
	           _widgets[dd]->_textAlignV);
}

void ThemeEngine::drawDropDownButton(const Common::Rect &r, uint32 dropdownWidth, const Common::U32String &str,
                                     ThemeEngine::WidgetStateInfo buttonState, bool inButton, bool inDropdown, bool rtl) {
	if (!ready())
		return;

	DrawData dd;

	if (buttonState == kStateHighlight && inButton)
		dd = rtl ? kDDDropDownButtonHoverLeftRTL : kDDDropDownButtonHoverLeft;
	else if (buttonState == kStateHighlight && inDropdown)
		dd = rtl ? kDDDropDownButtonHoverRightRTL : kDDDropDownButtonHoverRight;
	else if (buttonState == kStateDisabled)
		dd = rtl ? kDDDropDownButtonDisabledRTL : kDDDropDownButtonDisabled;
	else if (buttonState == kStatePressed && inButton)
		dd = rtl ? kDDDropDownButtonPressedLeftRTL : kDDDropDownButtonPressedLeft;
	else if (buttonState == kStatePressed && inDropdown)
		dd = rtl ? kDDDropDownButtonPressedRightRTL : kDDDropDownButtonPressedRight;
	else
		dd = rtl ? kDDDropDownButtonIdleRTL : kDDDropDownButtonIdle;

	drawDD(dd, r);

	// Center the text in the button without using the area of the drop down button
	Common::Rect textRect = r;
	textRect.left  = r.left  + dropdownWidth;
	textRect.right = r.right - dropdownWidth;
	drawDDText(getTextData(dd), getTextColor(dd), textRect, str, false, true, convertTextAlignH(_widgets[dd]->_textAlignH, rtl),
	           _widgets[dd]->_textAlignV);
}

void ThemeEngine::drawLineSeparator(const Common::Rect &r) {
	if (!ready())
		return;

	drawDD(kDDSeparator, r);
}

void ThemeEngine::drawCheckbox(const Common::Rect &r, const Common::U32String &str, bool checked, WidgetStateInfo state, bool rtl) {
	if (!ready())
		return;

	Common::Rect r2 = r;
	DrawData dd = kDDCheckboxDefault;

	if (checked)
		dd = kDDCheckboxSelected;

	if (state == kStateDisabled)
		dd = checked ? kDDCheckboxDisabledSelected : kDDCheckboxDisabled;

	const int checkBoxSize = MIN((int)r.height(), getFontHeight());

	r2.left = rtl ? r.right - checkBoxSize : r2.left;
	r2.bottom = r2.top + checkBoxSize;
	r2.right = r2.left + checkBoxSize;

	drawDD(dd, r2);

	if (rtl) {
		r2.right = r2.left - checkBoxSize;
		r2.left = r.left;
	} else {
		r2.left = r2.right + checkBoxSize;
		r2.right = r.right;
	}

	if (r2.right > r2.left) {
		drawDDText(getTextData(dd), getTextColor(dd), r2, str, true, false, convertTextAlignH(_widgets[dd]->_textAlignH, rtl),
		           _widgets[dd]->_textAlignV);
	}
}

void ThemeEngine::drawRadiobutton(const Common::Rect &r, const Common::U32String &str, bool checked, WidgetStateInfo state, bool rtl) {
	if (!ready())
		return;

	Common::Rect r2 = r;
	DrawData dd = kDDRadiobuttonDefault;

	if (checked)
		dd = kDDRadiobuttonSelected;

	if (state == kStateDisabled)
		dd = kDDRadiobuttonDisabled;

	const int checkBoxSize = MIN((int)r.height(), getFontHeight());

	r2.left = rtl ? r2.right - checkBoxSize : r2.left;
	r2.bottom = r2.top + checkBoxSize;
	r2.right = r2.left + checkBoxSize;

	drawDD(dd, r2);

	if (rtl) {
		r2.right = r2.left - checkBoxSize;
		r2.left = r.left;
	} else {
		r2.left = r2.right + checkBoxSize;
		r2.right = MAX(r2.left, r.right);
	}

	drawDDText(getTextData(dd), getTextColor(dd), r2, str, true, false, convertTextAlignH(_widgets[dd]->_textAlignH, rtl),
		_widgets[dd]->_textAlignV);
}

void ThemeEngine::drawSlider(const Common::Rect &r, int width, WidgetStateInfo state, bool rtl) {
	if (!ready())
		return;

	DrawData dd = kDDSliderFull;

	if (state == kStateHighlight)
		dd = kDDSliderHover;
	else if (state == kStateDisabled)
		dd = kDDSliderDisabled;

	Common::Rect r2 = r;
	r2.setWidth(MIN((int16)width, r.width()));
	//	r2.top++; r2.bottom--; r2.left++; r2.right--;

	if (rtl) {
		r2.left = r.right - r2.width();
		r2.right = r.right;
	}

	drawWidgetBackground(r, kWidgetBackgroundSlider);

	drawDD(dd, r2);
}

void ThemeEngine::drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState scrollState) {
	if (!ready())
		return;

	drawDD(kDDScrollbarBase, r);

	Common::Rect r2 = r;
	const int buttonExtra = r.width() + 1; // scrollbar.cpp's UP_DOWN_BOX_HEIGHT

	r2.bottom = r2.top + buttonExtra;
	drawDD(scrollState == kScrollbarStateUp ? kDDScrollbarButtonHover : kDDScrollbarButtonIdle, r2,
	       Graphics::VectorRenderer::kTriangleUp);

	r2.translate(0, r.height() - r2.height());
	drawDD(scrollState == kScrollbarStateDown ? kDDScrollbarButtonHover : kDDScrollbarButtonIdle, r2,
	       Graphics::VectorRenderer::kTriangleDown);

	r2 = r;
	r2.left += 1;
	r2.right -= 1;
	r2.top += sliderY;
	r2.bottom = r2.top + sliderHeight;
	drawDD(scrollState == kScrollbarStateSlider ? kDDScrollbarHandleHover : kDDScrollbarHandleIdle, r2);
}

void ThemeEngine::drawDialogBackground(const Common::Rect &r, DialogBackground bgtype) {
	if (!ready())
		return;

	switch (bgtype) {
	case kDialogBackgroundMain:
		drawDD(kDDMainDialogBackground, r);
		break;

	case kDialogBackgroundSpecial:
		drawDD(kDDSpecialColorBackground, r);
		break;

	case kDialogBackgroundPlain:
		drawDD(kDDPlainColorBackground, r);
		break;

	case kDialogBackgroundTooltip:
		drawDD(kDDTooltipBackground, r);
		break;

	case kDialogBackgroundDefault:
		drawDD(kDDDefaultBackground, r);
		break;

	default:
		// fallthrough intended
	case kDialogBackgroundNone:
		// no op
		break;
	}
}

void ThemeEngine::drawCaret(const Common::Rect &r, bool erase) {
	if (!ready())
		return;

	if (erase) {
		restoreBackground(r);
	} else
		drawDD(kDDCaret, r);
}

void ThemeEngine::drawPopUpWidget(const Common::Rect &r, const Common::U32String &sel, int deltax, WidgetStateInfo state, bool rtl) {
	if (!ready())
		return;

	DrawData dd = rtl ? kDDPopUpIdleRTL : kDDPopUpIdle;

	if (state == kStateEnabled)
		dd = rtl ? kDDPopUpIdleRTL : kDDPopUpIdle;
	else if (state == kStateHighlight)
		dd = rtl ? kDDPopUpHoverRTL : kDDPopUpHover;
	else if (state == kStateDisabled)
		dd = rtl ? kDDPopUpDisabledRTL : kDDPopUpDisabled;

	drawDD(dd, r);

	if (!sel.empty() && r.width() >= 13 && r.height() >= 1) {
		Common::Rect text(r.left + 3, r.top + 1, r.right - 10, r.bottom);
		drawDDText(getTextData(dd), getTextColor(dd), text, sel, true, false, convertTextAlignH(_widgets[dd]->_textAlignH, rtl),
		           _widgets[dd]->_textAlignV, deltax);
	}
}

void ThemeEngine::drawSurface(const Common::Point &p, const Graphics::Surface &surface, bool themeTrans) {
	if (!ready())
		return;

	if (_layerToDraw == kDrawLayerBackground)
		return;

	_vectorRenderer->setClippingRect(_clip);
	if (themeTrans)
		_vectorRenderer->blitKeyBitmap(&surface, p);
	else
		_vectorRenderer->blitSubSurface(&surface, p);

	Common::Rect dirtyRect = Common::Rect(p.x, p.y, p.x + surface.w, p.y + surface.h);
	dirtyRect.clip(_clip);
	addDirtyRect(dirtyRect);
}

void ThemeEngine::drawWidgetBackground(const Common::Rect &r, WidgetBackground background) {
	if (!ready())
		return;

	switch (background) {
	case kWidgetBackgroundNo:
		break;

	case kWidgetBackgroundBorderSmall:
		drawDD(kDDWidgetBackgroundSmall, r);
		break;

	case kWidgetBackgroundEditText:
		drawDD(kDDWidgetBackgroundEditText, r);
		break;

	case kWidgetBackgroundSlider:
		drawDD(kDDWidgetBackgroundSlider, r);
		break;

	default:
		drawDD(kDDWidgetBackgroundDefault, r);
		break;
	}
}

void ThemeEngine::drawTab(const Common::Rect &r, int tabHeight, const Common::Array<int> &tabWidths,
                          const Common::Array<Common::U32String> &tabs, int active, bool rtl) {
	if (!ready())
		return;

	assert(tabs.size() == tabWidths.size());

	drawDD(kDDTabBackground, Common::Rect(r.left, r.top, r.right, r.top + tabHeight));

	const int numTabs = (int)tabs.size();
	int width = 0;

	if (rtl) {
		for (int i = 0; i < numTabs; i++) {
			width += tabWidths[i];
		}
		width = r.width() - width;
	}

	int activePos = -1;
	for (int i = 0; i < numTabs; i++) {
		int current = rtl ? (numTabs - i - 1) : i;

		if (r.left + width > r.right || r.left + width + tabWidths[current] > r.right) {
			width += tabWidths[current];
			continue;
		}

		if (current == active) {
			activePos = width;
			width += tabWidths[current];
			continue;
		}


		Common::Rect tabRect(r.left + width, r.top, r.left + width + tabWidths[current], r.top + tabHeight);
		drawDD(kDDTabInactive, tabRect);
		drawDDText(getTextData(kDDTabInactive), getTextColor(kDDTabInactive), tabRect, tabs[current], false, false,
		           convertTextAlignH(_widgets[kDDTabInactive]->_textAlignH, rtl), _widgets[kDDTabInactive]->_textAlignV);
		width += tabWidths[current];
	}

	if (activePos >= 0) {
		Common::Rect tabRect(r.left + activePos, r.top, r.left + activePos + tabWidths[active], r.top + tabHeight);
		const uint16 tabLeft = activePos;
		const uint16 tabRight = MAX(r.right - tabRect.right, 0);
		drawDD(kDDTabActive, tabRect, (tabLeft << 16) | (tabRight & 0xFFFF));
		drawDDText(getTextData(kDDTabActive), getTextColor(kDDTabActive), tabRect, tabs[active], false, false,
		           convertTextAlignH(_widgets[kDDTabActive]->_textAlignH, rtl), _widgets[kDDTabActive]->_textAlignV);
	}
}

void ThemeEngine::drawText(const Common::Rect &r, const Common::U32String &str, WidgetStateInfo state,
                           Graphics::TextAlign align, TextInversionState inverted, int deltax, bool useEllipsis,
                           FontStyle font, FontColor color, bool restore, const Common::Rect &drawableTextArea) {
	if (!ready())
		return;

	TextColor colorId = kTextColorMAX;

	switch (color) {
	case kFontColorNormal:
		if (inverted) {
			colorId = kTextColorNormalInverted;
		} else {
			switch (state) {
			case kStateDisabled:
				colorId = kTextColorNormalDisabled;
				break;

			case kStateHighlight:
				colorId = kTextColorNormalHover;
				break;

			default:
				// fallthrough intended
			case kStateEnabled:
			case kStatePressed:
				colorId = kTextColorNormal;
				break;
			}
		}
		break;

	case kFontColorAlternate:
		if (inverted) {
			colorId = kTextColorAlternativeInverted;
		} else {
			switch (state) {
			case kStateDisabled:
				colorId = kTextColorAlternativeDisabled;
				break;

			case kStateHighlight:
				colorId = kTextColorAlternativeHover;
				break;

			default:
				// fallthrough intended
			case kStateEnabled:
			case kStatePressed:
				colorId = kTextColorAlternative;
				break;
			}
		}
		break;

	default:
		return;
	}

	TextData textId = fontStyleToData(font);

	switch (inverted) {
	case kTextInversion:
		drawDD(kDDTextSelectionBackground, r);
		restore = false;
		break;

	case kTextInversionFocus:
		drawDD(kDDTextSelectionFocusBackground, r);
		restore = false;
		break;

	default:
		break;
	}

	drawDDText(textId, colorId, r, str, restore, useEllipsis, align, kTextAlignVCenter, deltax, drawableTextArea);
}

void ThemeEngine::drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, FontColor color) {
	if (!ready())
		return;

	Common::Rect charArea = r;
	charArea.clip(_screen.w, _screen.h);

	uint32 rgbColor = _overlayFormat.RGBToColor(_textColors[color]->r, _textColors[color]->g, _textColors[color]->b);

	// TODO: Handle clipping when drawing chars

	restoreBackground(charArea);
	font->drawChar(&_screen, ch, charArea.left, charArea.top, rgbColor);
	addDirtyRect(charArea);
}

void ThemeEngine::debugWidgetPosition(const char *name, const Common::Rect &r) {
	_font->drawString(&_screen, name, r.left, r.top, r.width(), 0xFFFF, Graphics::kTextAlignRight, 0, true);
	_screen.hLine(r.left, r.top, r.right, 0xFFFF);
	_screen.hLine(r.left, r.bottom, r.right, 0xFFFF);
	_screen.vLine(r.left, r.top, r.bottom, 0xFFFF);
	_screen.vLine(r.right, r.top, r.bottom, 0xFFFF);
}

/**********************************************************
 * Screen/overlay management
 *********************************************************/
void ThemeEngine::copyBackBufferToScreen() {
	memcpy(_screen.getPixels(), _backBuffer.getPixels(), (size_t)(_screen.pitch * _screen.h));
}

void ThemeEngine::updateScreen() {
#ifdef LAYOUT_DEBUG_DIALOG
	_vectorRenderer->fillSurface();
	_themeEval->debugDraw(&_screen, _font);
	_vectorRenderer->copyWholeFrame(_system);
#else
	updateDirtyScreen();
#endif
}

void ThemeEngine::addDirtyRect(Common::Rect r) {
	// Clip the rect to screen coords
	r.clip(_screen.w, _screen.h);

	// If it is empty after clipping, we are done
	if (r.isEmpty())
		return;

	// Check if the new rectangle is contained within another in the list
	Common::List<Common::Rect>::iterator it;
	for (it = _dirtyScreen.begin(); it != _dirtyScreen.end();) {
		// If we find a rectangle which fully contains the new one,
		// we can abort the search.
		if (it->contains(r))
			return;

		// Conversely, if we find rectangles which are contained in
		// the new one, we can remove them
		if (r.contains(*it))
			it = _dirtyScreen.erase(it);
		else
			++it;
	}

	// If we got here, we can safely add r to the list of dirty rects.
	_dirtyScreen.push_back(r);
}

void ThemeEngine::updateDirtyScreen() {
	if (_dirtyScreen.empty())
		return;

	Common::List<Common::Rect>::iterator i;
	for (i = _dirtyScreen.begin(); i != _dirtyScreen.end(); ++i) {
		_vectorRenderer->copyFrame(_system, *i);
	}

	_dirtyScreen.clear();
}

void ThemeEngine::applyScreenShading(ShadingStyle style) {
	if (style != kShadingNone) {
		_vectorRenderer->applyScreenShading(style);
		addDirtyRect(Common::Rect(0, 0, _screen.w, _screen.h));
	}
}

bool ThemeEngine::createCursor(const Common::String &filename, int hotspotX, int hotspotY) {
	// Try to locate the specified file among all loaded bitmaps
	const Graphics::Surface *cursor = _bitmaps[filename];
	if (!cursor)
		return false;

	// Set up the cursor parameters
	_cursorHotspotX = hotspotX;
	_cursorHotspotY = hotspotY;

	_cursorWidth = cursor->w;
	_cursorHeight = cursor->h;

#ifdef USE_RGB_COLOR
	_cursorFormat = cursor->format;
	_cursorTransparent = _cursorFormat.RGBToColor(0xFF, 0, 0xFF);

	// Allocate a new buffer for the cursor
	delete[] _cursor;
	_cursor = new byte[_cursorWidth * _cursorHeight * _cursorFormat.bytesPerPixel];
	assert(_cursor);
	Graphics::copyBlit(_cursor, (const byte *)cursor->getPixels(),
	                   _cursorWidth * _cursorFormat.bytesPerPixel, cursor->pitch,
	                   _cursorWidth, _cursorHeight, _cursorFormat.bytesPerPixel);

	_useCursor = true;
#else
	if (!_system->hasFeature(OSystem::kFeatureCursorPalette))
		return true;

	// Allocate a new buffer for the cursor
	delete[] _cursor;
	_cursor = new byte[_cursorWidth * _cursorHeight];
	assert(_cursor);
	memset(_cursor, 0xFF, sizeof(byte) * _cursorWidth * _cursorHeight);

	// the transparent color is 0xFF00FF
	const uint32 colTransparent = _overlayFormat.RGBToColor(0xFF, 0, 0xFF);

	// Now, scan the bitmap. We have to convert it from 16 bit color mode
	// to 8 bit mode, and have to create a suitable palette on the fly.
	uint colorsFound = 0;
	Common::HashMap<int, int> colorToIndex;
	const byte *src = (const byte *)cursor->getPixels();
	for (uint y = 0; y < _cursorHeight; ++y) {
		for (uint x = 0; x < _cursorWidth; ++x) {
			uint32 color = colTransparent;
			byte r, g, b;

			if (cursor->format.bytesPerPixel == 2) {
				color = READ_UINT16(src);
			} else if (cursor->format.bytesPerPixel == 4) {
				color = READ_UINT32(src);
			}

			src += cursor->format.bytesPerPixel;

			// Skip transparency
			if (color == colTransparent)
				continue;

			cursor->format.colorToRGB(color, r, g, b);
			const int col = (r << 16) | (g << 8) | b;

			// If there is no entry yet for this color in the palette: Add one
			if (!colorToIndex.contains(col)) {
				if (colorsFound >= MAX_CURS_COLORS) {
					warning("Cursor contains too many colors (%d, but only %d are allowed)", colorsFound, MAX_CURS_COLORS);
					return false;
				}

				const int index = colorsFound++;
				colorToIndex[col] = index;

				_cursorPal[index * 3 + 0] = r;
				_cursorPal[index * 3 + 1] = g;
				_cursorPal[index * 3 + 2] = b;
			}

			// Copy pixel from the 16 bit source surface to the 8bit target surface
			const int index = colorToIndex[col];
			_cursor[y * _cursorWidth + x] = index;
		}
	}

	_useCursor = true;
	_cursorPalSize = colorsFound;
#endif

	return true;
}


/**********************************************************
 * Legacy GUI::Theme support functions
 *********************************************************/

const Graphics::Font *ThemeEngine::getFont(FontStyle font) const {
	return _texts[fontStyleToData(font)]->_fontPtr;
}

int ThemeEngine::getFontHeight(FontStyle font) const {
	return ready() ? _texts[fontStyleToData(font)]->_fontPtr->getFontHeight() : 0;
}

int ThemeEngine::getStringWidth(const Common::U32String &str, FontStyle font) const {
	return ready() ? _texts[fontStyleToData(font)]->_fontPtr->getStringWidth(str) : 0;
}

int ThemeEngine::getCharWidth(byte c, FontStyle font) const {
	return ready() ? _texts[fontStyleToData(font)]->_fontPtr->getCharWidth(c) : 0;
}

int ThemeEngine::getKerningOffset(byte left, byte right, FontStyle font) const {
	return ready() ? _texts[fontStyleToData(font)]->_fontPtr->getKerningOffset(left, right) : 0;
}

TextData ThemeEngine::getTextData(DrawData ddId) const {
	return _widgets[ddId] ? (TextData)_widgets[ddId]->_textDataId : kTextDataNone;
}

TextColor ThemeEngine::getTextColor(DrawData ddId) const {
	return _widgets[ddId] ? _widgets[ddId]->_textColorId : kTextColorMAX;
}

DrawData ThemeEngine::parseDrawDataId(const Common::String &name) const {
	for (int i = 0; i < kDrawDataMAX; ++i)
		if (name.compareToIgnoreCase(kDrawDataDefaults[i].name) == 0)
			return kDrawDataDefaults[i].id;

	return kDDNone;
}

/**********************************************************
 * External data loading
 *********************************************************/

const Graphics::Font *ThemeEngine::loadScalableFont(const Common::String &filename, const Common::String &charset, const int pointsize, Common::String &name) {
#ifdef USE_FREETYPE2
	name = Common::String::format("%s-%s@%d", filename.c_str(), charset.c_str(), pointsize);

	// Try already loaded fonts.
	const Graphics::Font *font = FontMan.getFontByName(name);
	if (font)
		return font;

	Common::ArchiveMemberList members;
	_themeFiles.listMatchingMembers(members, filename);

	for (Common::ArchiveMemberList::const_iterator i = members.begin(), end = members.end(); i != end; ++i) {
		Common::SeekableReadStream *stream = (*i)->createReadStream();
		if (stream) {
			font = Graphics::loadTTFFont(*stream, pointsize, Graphics::kTTFSizeModeCharacter, 0, Graphics::kTTFRenderModeLight);
			delete stream;

			if (font)
				return font;
		}
	}

	// Try loading the font from the common fonts archive.
	font = Graphics::loadTTFFontFromArchive(filename, pointsize, Graphics::kTTFSizeModeCharacter, 0, Graphics::kTTFRenderModeLight);
	if (font)
		return font;
#endif
	return nullptr;
}

const Graphics::Font *ThemeEngine::loadFont(const Common::String &filename, Common::String &name) {
	name = filename;

	// Try already loaded fonts.
	const Graphics::Font *font = FontMan.getFontByName(name);
	if (font)
		return font;

	Common::ArchiveMemberList members;
	const Common::String cacheFilename(genCacheFilename(filename));
	_themeFiles.listMatchingMembers(members, cacheFilename);
	_themeFiles.listMatchingMembers(members, filename);

	for (Common::ArchiveMemberList::const_iterator i = members.begin(), end = members.end(); i != end; ++i) {
		Common::SeekableReadStream *stream = (*i)->createReadStream();
		if (stream) {
			if ((*i)->getName().equalsIgnoreCase(cacheFilename)) {
				font = Graphics::BdfFont::loadFromCache(*stream);
			} else {
				font = Graphics::BdfFont::loadFont(*stream);
				if (font && !cacheFilename.empty()) {
					if (!Graphics::BdfFont::cacheFontData(*(const Graphics::BdfFont *)font, cacheFilename))
						warning("Couldn't create cache file for font '%s'", filename.c_str());
				}
			}
			delete stream;

			if (font)
				return font;
		}
	}

	return nullptr;
}

const Graphics::Font *ThemeEngine::loadFont(const Common::String &filename, const Common::String &scalableFilename, const Common::String &charset, const int pointsize, const bool makeLocalizedFont) {
	Common::String fontName;

	const Graphics::Font *font = nullptr;

	// Prefer scalable fonts over non-scalable fonts
	if (!scalableFilename.empty())
		font = loadScalableFont(scalableFilename, charset, pointsize, fontName);

	if (!font)
		font = loadFont(filename, fontName);

	// If the font is successfully loaded store it in the font manager.
	if (font) {
		FontMan.assignFontToName(fontName, font);
		// If this font should be the new default localized font, we set it up
		// for that.
		if (makeLocalizedFont)
			FontMan.setLocalizedFont(fontName);
	}

	return font;
}

Common::String ThemeEngine::genCacheFilename(const Common::String &filename) const {
	Common::String cacheName(filename);
	for (int i = cacheName.size() - 1; i >= 0; --i) {
		if (cacheName[i] == '.') {
			while ((uint)i < cacheName.size() - 1) {
				cacheName.deleteLastChar();
			}

			cacheName += "fcc";
			return cacheName;
		}
	}

	return Common::String();
}


/**********************************************************
 * Static Theme XML functions
 *********************************************************/

bool ThemeEngine::themeConfigParseHeader(Common::String header, Common::String &themeName) {
	// Check that header is not corrupted
	if ((byte)header[0] > 127) {
		warning("Corrupted theme header found");
		return false;
	}

	header.trim();

	if (header.empty())
		return false;

	if (header[0] != '[' || header.lastChar() != ']')
		return false;

	header.deleteChar(0);
	header.deleteLastChar();

	Common::StringTokenizer tok(header, ":");

	if (tok.nextToken() != SCUMMVM_THEME_VERSION_STR)
		return false;

	themeName = tok.nextToken();
	Common::String author = tok.nextToken();

	return tok.empty();
}

bool ThemeEngine::themeConfigUsable(const Common::ArchiveMember &member, Common::String &themeName) {
	Common::File stream;
	bool foundHeader = false;

	if (member.getName().matchString("*.zip", true)) {
		Common::Archive *zipArchive = Common::makeZipArchive(member.createReadStream());

		if (zipArchive && zipArchive->hasFile("THEMERC")) {
			stream.open("THEMERC", *zipArchive);
		}

		delete zipArchive;
	}

	if (stream.isOpen()) {
		Common::String stxHeader = stream.readLine();
		foundHeader = themeConfigParseHeader(stxHeader, themeName);
	}

	return foundHeader;
}

bool ThemeEngine::themeConfigUsable(const Common::FSNode &node, Common::String &themeName) {
	Common::File stream;
	bool foundHeader = false;

	if (node.getName().matchString("*.zip", true) && !node.isDirectory()) {
		Common::Archive *zipArchive = Common::makeZipArchive(node);
		if (zipArchive && zipArchive->hasFile("THEMERC")) {
			// Open THEMERC from the ZIP file.
			stream.open("THEMERC", *zipArchive);
		}
		// Delete the ZIP archive again. Note: This only works because
		// stream.open() only uses ZipArchive::createReadStreamForMember,
		// and that in turn happens to read all the data for a given
		// archive member into a memory block. So there will be no dangling
		// reference to zipArchive anywhere. This could change if we
		// ever modify ZipArchive::createReadStreamForMember.
		delete zipArchive;
	} else if (node.isDirectory()) {
		Common::FSNode headerfile = node.getChild("THEMERC");
		if (!headerfile.exists() || !headerfile.isReadable() || headerfile.isDirectory())
			return false;

		if (!stream.open(headerfile))
			return false;
	}

	if (stream.isOpen()) {
		Common::String stxHeader = stream.readLine();
		foundHeader = themeConfigParseHeader(stxHeader, themeName);
	}

	return foundHeader;
}

namespace {

struct TDComparator {
	const Common::String _id;
	TDComparator(const Common::String &id) : _id(id) {}

	bool operator()(const ThemeEngine::ThemeDescriptor &r) {
		return _id == r.id;
	}
};

} // end of anonymous namespace

void ThemeEngine::listUsableThemes(Common::List<ThemeDescriptor> &list) {
#ifndef DISABLE_GUI_BUILTIN_THEME
	ThemeDescriptor th;
	th.name = "ScummVM Classic Theme (Builtin Version)";
	th.id = "builtin";
	th.filename.clear();
	list.push_back(th);
#endif

	if (ConfMan.hasKey("themepath"))
		listUsableThemes(Common::FSNode(ConfMan.get("themepath")), list);

	listUsableThemes(SearchMan, list);

	// Now we need to strip all duplicates
	// TODO: It might not be the best idea to strip duplicates. The user might
	// have different versions of a specific theme in his paths, thus this code
	// might show him the wrong version. The problem is we have no ways of checking
	// a theme version currently. Also since we want to avoid saving the full path
	// in the config file we can not do any better currently.
	Common::List<ThemeDescriptor> output;

	for (Common::List<ThemeDescriptor>::const_iterator i = list.begin(); i != list.end(); ++i) {
		if (Common::find_if(output.begin(), output.end(), TDComparator(i->id)) == output.end())
			output.push_back(*i);
	}

	list = output;
	output.clear();
}

void ThemeEngine::listUsableThemes(Common::Archive &archive, Common::List<ThemeDescriptor> &list) {
	ThemeDescriptor td;

	Common::ArchiveMemberList fileList;
	archive.listMatchingMembers(fileList, "*.zip");
	for (Common::ArchiveMemberList::iterator i = fileList.begin();
	        i != fileList.end(); ++i) {
		td.name.clear();
		if (themeConfigUsable(**i, td.name)) {
			td.filename = (*i)->getName();
			td.id = (*i)->getDisplayName();

			// If the name of the node object also contains
			// the ".zip" suffix, we will strip it.
			if (td.id.matchString("*.zip", true)) {
				for (int j = 0; j < 4; ++j)
					td.id.deleteLastChar();
			}

			list.push_back(td);
		}
	}

	fileList.clear();
}

void ThemeEngine::listUsableThemes(const Common::FSNode &node, Common::List<ThemeDescriptor> &list, int depth) {
	if (!node.exists() || !node.isReadable() || !node.isDirectory())
		return;

	ThemeDescriptor td;

	// Check whether we point to a valid theme directory.
	if (themeConfigUsable(node, td.name)) {
		td.filename = node.getPath();
		td.id = node.getName();

		list.push_back(td);

		// A theme directory should never contain any other themes
		// thus we just return to the caller here.
		return;
	}

	Common::FSList fileList;
	// Check all files. We need this to find all themes inside ZIP archives.
	if (!node.getChildren(fileList, Common::FSNode::kListFilesOnly))
		return;

	for (Common::FSList::iterator i = fileList.begin(); i != fileList.end(); ++i) {
		// We will only process zip files for now
		if (!i->getPath().matchString("*.zip", true))
			continue;

		td.name.clear();
		if (themeConfigUsable(*i, td.name)) {
			td.filename = i->getPath();
			td.id = i->getName();

			// If the name of the node object also contains
			// the ".zip" suffix, we will strip it.
			if (td.id.matchString("*.zip", true)) {
				for (int j = 0; j < 4; ++j)
					td.id.deleteLastChar();
			}

			list.push_back(td);
		}
	}

	fileList.clear();

	// Check if we exceeded the given recursion depth
	if (depth - 1 == -1)
		return;

	// As next step we will search all subdirectories
	if (!node.getChildren(fileList, Common::FSNode::kListDirectoriesOnly))
		return;

	for (Common::FSList::iterator i = fileList.begin(); i != fileList.end(); ++i)
		listUsableThemes(*i, list, depth == -1 ? - 1 : depth - 1);
}

Common::String ThemeEngine::getThemeFile(const Common::String &id) {
	// FIXME: Actually "default" rather sounds like it should use
	// our default theme which would mean "scummremastered" instead
	// of the builtin one.
	if (id.equalsIgnoreCase("default"))
		return Common::String();

	// For our builtin theme we don't have to do anything for now too
	if (id.equalsIgnoreCase("builtin"))
		return Common::String();

	Common::FSNode node(id);

	// If the given id is a full path we'll just use it
	if (node.exists() && (node.isDirectory() || node.getName().matchString("*.zip", true)))
		return id;

	// FIXME:
	// A very ugly hack to map a id to a filename, this will generate
	// a complete theme list, thus it is slower than it could be.
	// But it is the easiest solution for now.
	Common::List<ThemeDescriptor> list;
	listUsableThemes(list);

	for (Common::List<ThemeDescriptor>::const_iterator i = list.begin(); i != list.end(); ++i) {
		if (id.equalsIgnoreCase(i->id))
			return i->filename;
	}

	warning("Could not find theme '%s' falling back to builtin", id.c_str());

	// If no matching id has been found we will
	// just fall back to the builtin theme
	return Common::String();
}

Common::String ThemeEngine::getThemeId(const Common::String &filename) {
	// If no filename has been given we will initialize the builtin theme
	if (filename.empty())
		return "builtin";

	Common::FSNode node(filename);
	if (node.exists()) {
		if (node.getName().matchString("*.zip", true)) {
			Common::String id = node.getName();

			for (int i = 0; i < 4; ++i)
				id.deleteLastChar();

			return id;
		} else {
			return node.getName();
		}
	}

	// FIXME:
	// A very ugly hack to map a id to a filename, this will generate
	// a complete theme list, thus it is slower than it could be.
	// But it is the easiest solution for now.
	Common::List<ThemeDescriptor> list;
	listUsableThemes(list);

	for (Common::List<ThemeDescriptor>::const_iterator i = list.begin(); i != list.end(); ++i) {
		if (filename.equalsIgnoreCase(i->filename))
			return i->id;
	}

	return "builtin";
}

void ThemeEngine::showCursor() {
	if (_useCursor) {
#ifndef USE_RGB_COLOR
		CursorMan.pushCursorPalette(_cursorPal, 0, _cursorPalSize);
#endif
		CursorMan.pushCursor(_cursor, _cursorWidth, _cursorHeight, _cursorHotspotX, _cursorHotspotY, _cursorTransparent, true, &_cursorFormat);
		CursorMan.showMouse(true);
	}
}

void ThemeEngine::hideCursor() {
	if (_useCursor) {
#ifndef USE_RGB_COLOR
		CursorMan.popCursorPalette();
#endif
		CursorMan.popCursor();
	}
}

void ThemeEngine::drawToBackbuffer() {
	_vectorRenderer->setSurface(&_backBuffer);
}

void ThemeEngine::drawToScreen() {
	_vectorRenderer->setSurface(&_screen);
}

Common::Rect ThemeEngine::swapClipRect(const Common::Rect &newRect) {
	Common::Rect oldRect = _clip;
	_clip = newRect;
	return oldRect;
}

void ThemeEngine::disableClipRect() {
	_clip = Common::Rect(_screen.w, _screen.h);
}

} // End of namespace GUI.
