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
 * $URL$
 * $Id$
 *
 */

#ifndef GUI_THEME_ENGINE_H
#define GUI_THEME_ENGINE_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/fs.h"
#include "graphics/surface.h"
#include "graphics/fontman.h"

#define SCUMMVM_THEME_VERSION_STR "SCUMMVM_STX0.3"

namespace Graphics {
struct DrawStep;
class VectorRenderer;
}

namespace GUI {

struct WidgetDrawData;
struct DrawDataInfo;
struct TextDrawData;
class Dialog;
class GuiObject;
class ThemeEval;
class ThemeItem;
class ThemeParser;

class ThemeEngine {
protected:
	typedef Common::HashMap<Common::String, Graphics::Surface*> ImagesMap;

	friend class GUI::Dialog;
	friend class GUI::GuiObject;

	/** Sets whether backcaching is enabled */
	static const bool kEnableBackCaching = true;

	/**
	 *	DrawData sets enumeration.
	 *	Each DD set corresponds to the actual looks
	 *	of a widget in a given state.
	*/
	enum DrawData {
		kDDMainDialogBackground,
		kDDSpecialColorBackground,
		kDDPlainColorBackground,
		kDDDefaultBackground,
		kDDTextSelectionBackground,

		kDDWidgetBackgroundDefault,
		kDDWidgetBackgroundSmall,
		kDDWidgetBackgroundEditText,
		kDDWidgetBackgroundSlider,

		kDDButtonIdle,
		kDDButtonHover,
		kDDButtonDisabled,

		kDDSliderFull,
		kDDSliderHover,
		kDDSliderDisabled,

		kDDCheckboxDefault,
		kDDCheckboxDisabled,
		kDDCheckboxSelected,

		kDDTabActive,
		kDDTabInactive,
		kDDTabBackground,

		kDDScrollbarBase,
		kDDScrollbarButtonIdle,
		kDDScrollbarButtonHover,
		kDDScrollbarHandleIdle,
		kDDScrollbarHandleHover,

		kDDPopUpIdle,
		kDDPopUpHover,

		kDDCaret,
		kDDSeparator,
		kDrawDataMAX,
		kDDNone = -1
	};

	/**
	 * Default values for each DrawData item.
	 * @see kDrawDataDefaults[] for implementation.
	 */
	static const struct DrawDataInfo {
		DrawData id; 		/** The actual ID of the DrawData item. */
		const char *name; 	/** The name of the DrawData item as it appears in the Theme Description files */
		bool buffer; 		/** Sets whether this item is buffered on the backbuffer or drawn directly to the screen. */
		DrawData parent; 	/** Parent DrawData item, for items that overlay. E.g. kButtonIdle -> kButtonHover */
	} kDrawDataDefaults[];


	enum TextData {
		kTextDataNone = -1,
		kTextDataDefault = 0,
		kTextDataHover,
		kTextDataDisabled,
		kTextDataInverted,
		kTextDataButton,
		kTextDataButtonHover,
		kTextDataNormalFont,
		kTextDataMAX
	};

	static const struct TextDataInfo {
		TextData id;
		const char *name;
	} kTextDataDefaults[];

public:
	//! Defined the align of the text
	enum TextAlign {
		kTextAlignLeft,		//!< Text should be aligned to the left
		kTextAlignCenter,	//!< Text should be centered
		kTextAlignRight		//!< Text should be aligned to the right
	};
	
	//! Vertical alignment of the text.
	enum TextAlignVertical {
		kTextAlignVBottom,
		kTextAlignVCenter,
		kTextAlignVTop
	};

	//! Widget background type
	enum WidgetBackground {
		kWidgetBackgroundNo,			//!< No background at all
		kWidgetBackgroundPlain,			//!< Simple background, this may not include borders
		kWidgetBackgroundBorder,		//!< Same as kWidgetBackgroundPlain just with a border
		kWidgetBackgroundBorderSmall,	//!< Same as kWidgetBackgroundPlain just with a small border
		kWidgetBackgroundEditText,		//!< Background used for edit text fields
		kWidgetBackgroundSlider			//!< Background used for sliders
	};
	
	//! Dialog background type
	enum DialogBackground {
		kDialogBackgroundMain,
		kDialogBackgroundSpecial,
		kDialogBackgroundPlain,
		kDialogBackgroundDefault
	};

	//! State of the widget to be drawn
	enum State {
		kStateDisabled,		//!< Indicates that the widget is disabled, that does NOT include that it is invisible
		kStateEnabled,		//!< Indicates that the widget is enabled
		kStateHighlight		//!< Indicates that the widget is highlighted by the user
	};

	typedef State WidgetStateInfo;

	enum ScrollbarState {
		kScrollbarStateNo,
		kScrollbarStateUp,
		kScrollbarStateDown,
		kScrollbarStateSlider,
		kScrollbarStateSinglePage
	};

	//! Font style selector
	enum FontStyle {
		kFontStyleBold = 0,			//!< A bold font. This is also the default font.
		kFontStyleNormal = 1,		//!< A normal font.
		kFontStyleItalic = 2,		//!< Italic styled font.
		kFontStyleFixedNormal = 3,	//!< Fixed size font.
		kFontStyleFixedBold = 4,	//!< Fixed size bold font.
		kFontStyleFixedItalic = 5,	//!< Fixed size italic font.
		kFontStyleMax
	};

	//! Function used to process areas other than the current dialog
	enum ShadingStyle {
		kShadingNone,		//!< No special post processing
		kShadingDim,		//!< Dimming unused areas
		kShadingLuminance	//!< Converting colors to luminance for unused areas
	};
	
	//! Special image ids for images used in the GUI
	enum kThemeImages {
		kImageLogo = 0,		//!< ScummVM Logo used in the launcher
		kImageLogoSmall		//!< ScummVM logo used in the GMM
	};
	
	/** Graphics mode enumeration.
	 *	Each item represents a set of BPP and Renderer modes for a given
	 * surface.
	 */
	enum GraphicsMode {
		kGfxDisabled = 0,	/** No GFX */
		kGfxStandard16bit,	/** 2BPP with the standard (aliased) renderer. */
		kGfxAntialias16bit	/** 2BPP with the optimized AA renderer. */
	};
	
	/** Constant value to expand dirty rectangles, to make sure they are fully copied */
	static const int kDirtyRectangleThreshold = 1;

	struct Renderer {
		const char *name;
		const char *cfg;
		GraphicsMode mode;
	};

	static const Renderer _rendererModes[];
	static const uint _rendererModesSize;

	static const GraphicsMode _defaultRendererMode;

	static GraphicsMode findMode(const Common::String &cfg);
	static const char *findModeConfigName(GraphicsMode mode);

	/** Default constructor */
	ThemeEngine(Common::String fileName, GraphicsMode mode);

	/** Default destructor */
	~ThemeEngine();

	/**
	 *	VIRTUAL METHODS
	 *	This is the implementation of the GUI::Theme API to allow
	 *	the ThemeEngine class to be plugged in as any other GUI
	 *	theme. In fact, the renderer works like any other GUI theme,
	 *	but supports extensive customization of the drawn widgets.
	 */
	bool init();
	void deinit();
	void clearAll();

	void refresh();
	void enable();
	void disable();

	/**
	 *	Implementation of the GUI::Theme API. Called when a
	 *	new dialog is opened. Note that the boolean parameter
	 *	meaning has been changed.
	 *
	 * @param enableBuffering If set to true, buffering is enabled for
	 *						  drawing this dialog, and will continue enabled
	 *						  until disabled.
	 */
	void openDialog(bool enableBuffering, ShadingStyle shading = kShadingNone);

	/**
	 *	The updateScreen() method is called every frame.
	 *	It processes all the drawing queues and then copies dirty rects
	 *	in the current Screen surface to the overlay.
	 */
	void updateScreen();

	/** Since the rendering pipeline changes, closing all dialogs causes no effect
		TODO: remove this from the original GUI::Theme API */
	void closeAllDialogs() {}

	/** Drawing area has been removed: it was too hackish. A workaround is on the works.
	 	TODO: finish the workaround for the credits dialog
		TODO: remove this from the original GUI::Theme API */
	void resetDrawArea() {}


	/**
	 *	FONT MANAGEMENT METHODS
	 */

	TextData fontStyleToData(FontStyle font) const {
		switch (font) {
			case kFontStyleNormal:
				return kTextDataNormalFont;

			default:
				return kTextDataDefault;
		}
	}

	const Graphics::Font *getFont(FontStyle font = kFontStyleBold) const;

	int getFontHeight(FontStyle font = kFontStyleBold) const;

	int getStringWidth(const Common::String &str, FontStyle font = kFontStyleBold) const;

	int getCharWidth(byte c, FontStyle font = kFontStyleBold) const;


	/**
	 *	WIDGET DRAWING METHODS
	 */
	void drawWidgetBackground(const Common::Rect &r, uint16 hints,
		WidgetBackground background = kWidgetBackgroundPlain, WidgetStateInfo state = kStateEnabled);

	void drawButton(const Common::Rect &r, const Common::String &str,
		WidgetStateInfo state = kStateEnabled, uint16 hints = 0);

	void drawSurface(const Common::Rect &r, const Graphics::Surface &surface,
		WidgetStateInfo state = kStateEnabled, int alpha = 256, bool themeTrans = false);

	void drawSlider(const Common::Rect &r, int width,
		WidgetStateInfo state = kStateEnabled);

	void drawCheckbox(const Common::Rect &r, const Common::String &str,
		bool checked, WidgetStateInfo state = kStateEnabled);

	void drawTab(const Common::Rect &r, int tabHeight, int tabWidth,
		const Common::Array<Common::String> &tabs, int active, uint16 hints,
		int titleVPad, WidgetStateInfo state = kStateEnabled);

	void drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight,
		ScrollbarState, WidgetStateInfo state = kStateEnabled);

	void drawPopUpWidget(const Common::Rect &r, const Common::String &sel,
		int deltax, WidgetStateInfo state = kStateEnabled, TextAlign align = kTextAlignLeft);

	void drawCaret(const Common::Rect &r, bool erase,
		WidgetStateInfo state = kStateEnabled);

	void drawLineSeparator(const Common::Rect &r, WidgetStateInfo state = kStateEnabled);

	void drawDialogBackground(const Common::Rect &r, DialogBackground type, WidgetStateInfo state = kStateEnabled);

	void drawText(const Common::Rect &r, const Common::String &str, WidgetStateInfo state = kStateEnabled, TextAlign align = kTextAlignCenter, bool inverted = false, int deltax = 0, bool useEllipsis = true, FontStyle font = kFontStyleBold);

	void drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, WidgetStateInfo state = kStateEnabled);

	/**
	 *	Actual implementation of a Dirty Rect drawing routine.
	 * 	Dirty rectangles are queued on a list and are later merged/calculated
	 *	before the actual drawing.
	 *
	 *	@param r Area of the dirty rect.
	 *	@param backup Deprecated.
	 *	@param special Deprecated.
	 */
	bool addDirtyRect(Common::Rect r, bool backup = false, bool special = false) {
		r.clip(_screen->w, _screen->h);
		_dirtyScreen.push_back(r);
		return true;
	}


	/**
	 *	Returns the DrawData enumeration value that represents the given string
	 *	in the DrawDataDefaults enumeration.
	 *	It's slow, but called sparsely.
	 *
	 *	@returns The drawdata enum value, or -1 if not found.
	 *	@param name The representing name, as found on Theme Description XML files.
	 *	@see kDrawDataDefaults[]
	 */
	DrawData getDrawDataId(const Common::String &name) {
		for (int i = 0; i < kDrawDataMAX; ++i)
			if (name.compareToIgnoreCase(kDrawDataDefaults[i].name) == 0)
				return kDrawDataDefaults[i].id;

		return kDDNone;
	}

	TextData getTextDataId(const Common::String &name) {
		for (int i = 0; i < kTextDataMAX; ++i)
			if (name.compareToIgnoreCase(kTextDataDefaults[i].name) == 0)
				return kTextDataDefaults[i].id;

		return kTextDataNone;
	}

	/**
	 *	Interface for ThemeParser class: Parsed DrawSteps are added via this function.
	 *	There is no return type because DrawSteps can always be added, unless something
	 *	goes horribly wrong.
	 *	The specified step will be added to the Steps list of the given DrawData id.
	 *
	 *	@param drawDataId The representing DrawData name, as found on Theme Description XML files.
	 *	@param step The actual DrawStep struct to be added.
	 */
	void addDrawStep(const Common::String &drawDataId, const Graphics::DrawStep &step);

	/**
	 *	Interfacefor the ThemeParser class: Parsed DrawData sets are added via this function.
	 *	The goal of the function is to initialize each DrawData set before their DrawSteps can
	 *	be added, hence this must be called for each DD set before addDrawStep() can be called
	 *	for that given set.
	 *
	 *	@param data The representing DrawData name, as found on Theme Description XML files.
	 *	@param cached Whether this DD set will be cached beforehand.
	 */
	bool addDrawData(const Common::String &data, bool cached);


	/**
	 *	Interface for the ThemeParser class: Loads a font to use on the GUI from the given
	 *	filename.
	 *
	 *	@param fontName Identifier name for the font.
	 * 	@param file Name of the font file.
	 *	@param r, g, b Color of the font.
	 */
	bool addFont(const Common::String &fontName, const Common::String &file, int r, int g, int b);


	/**
	 *	Interface for the ThemeParser class: Loads a bitmap file to use on the GUI.
	 * 	The filename is also used as its identifier.
	 *
	 *	@param filename Name of the bitmap file.
	 */
	bool addBitmap(const Common::String &filename);

	/**
	 *	Adds a new TextStep from the ThemeParser. This will be deprecated/removed once the
	 *	new Font API is in place.
	 */
	bool addTextData(const Common::String &drawDataId, const Common::String &textDataId, TextAlign alignH, TextAlignVertical alignV);

	/**
	 *	Returns if the Theme is ready to draw stuff on screen.
	 *	Must be called instead of just checking _initOk, because
	 *	this checks if the renderer is initialized AND if the theme
	 *	is loaded.
	 */
	bool ready() const {
		return _initOk && _themeOk;
	}

	/** Load the them from the file with the specified name. */
	bool loadTheme(const Common::String &fileName);

	/**
	 *	Changes the active graphics mode of the GUI; may be used to either
	 *	initialize the GUI or to change the mode while the GUI is already running.
	 */
	void setGraphicsMode(GraphicsMode mode);


	/**
	 *	Finishes buffering: widgets from then on will be drawn straight on the screen
	 *	without drawing queues.
	 */
	inline void finishBuffering() { _buffering = false; }
	inline void startBuffering() { _buffering = true; }

	inline ThemeEval *getEvaluator() { return _themeEval; }
	inline Graphics::VectorRenderer *renderer() { return _vectorRenderer; }

	inline bool supportsImages() const { return true; }
	inline bool ownCursor() const { return _useCursor; }

	Graphics::Surface *getBitmap(const Common::String &name) {
		return _bitmaps.contains(name) ? _bitmaps[name] : 0;
	}

	const Graphics::Surface *getImageSurface(const kThemeImages n) const {
		if (n == kImageLogo)
			return _bitmaps.contains("logo.bmp") ? _bitmaps["logo.bmp"] : 0;
		else if (n == kImageLogoSmall)
			return _bitmaps.contains("logo_small.bmp") ? _bitmaps["logo_small.bmp"] : 0;

		return 0;
	}

	/**
	 *	Interface for the Theme Parser: Creates a new cursor by loading the given
	 *	bitmap and sets it as the active cursor.
	 *
	 *	@param filename File name of the bitmap to load.
	 * 	@param hotspotX X Coordinate of the bitmap which does the cursor click.
	 *	@param hotspotY	Y Coordinate of the bitmap which does the cursor click.
	 *	@param scale	Scale at which the bitmap is supposed to be used.
	 */
	bool createCursor(const Common::String &filename, int hotspotX, int hotspotY, int scale);

	/**
	 *	Wrapper for restoring data from the Back Buffer to the screen.
	 *	The actual processing is done in the VectorRenderer.
	 *
	 *	@param r Area to restore.
	 *	@param special Deprecated.
	 */
	void restoreBackground(Common::Rect r, bool special = false);

	/**
	 *	Checks if a given DrawData set for a widget has been cached beforehand
	 *	and is ready to be blit into the screen.
	 *
	 *	@param type DrawData type of the widget.
	 *	@param r Size of the widget which is expected to be cached.
	 */
	bool isWidgetCached(DrawData type, const Common::Rect &r);

	const Common::String &getThemeName() const { return _themeName; }
	const Common::String &getThemeFileName() const { return _themeFileName; }
	int getGraphicsMode() const { return _graphicsMode; }

protected:
	/**
	 *	Initializes the drawing screen surfaces, _screen and _backBuffer.
	 *	If the surfaces already exist, they are cleared and re-initialized.
	 *
	 *	@param backBuffer Sets whether the _backBuffer surface should be initialized.
	 *	@template PixelType C type which specifies the size of each pixel.
	 *						Defaults to uint16 (2 BPP for the surfaces)
	 */
	template<typename PixelType> void screenInit(bool backBuffer = true);

	/**
	 *	Loads the given theme into the ThemeEngine.
	 *	Note that ThemeName is an identifier, not a filename.
	 *
	 *	@param ThemeName Theme identifier.
	 *	@returns true if the theme was successfully loaded.
	 */
	bool loadThemeXML(const Common::String &themeName);

	/**
	 *	Loads the default theme file (the embedded XML file found
	 *	in ThemeDefaultXML.cpp).
	 *	Called only when no other themes are available.
	 */
	bool loadDefaultXML();

	/**
	 *	Unloads the currently loaded theme so another one can
	 *	be loaded.
	 */
	void unloadTheme();

	/**
	 * Not implemented yet.
	 * TODO: reload themes, reload the renderer, recheck everything
	 */
	void screenChange() {
		error("Screen Changes are not supported yet. Fix this!");
	}

	/**
	 *	Actual Dirty Screen handling function.
	 *	Handles all the dirty squares in the list, merges and optimizes
	 *	them when possible and draws them to the screen.
	 *	Called from updateScreen()
	 */
	void renderDirtyScreen();

	/**
	 *	Frees the vector renderer.
	 */
	void freeRenderer();

	/**
	 * Frees the Back buffer surface, only if it's available.
	 */
	void freeBackbuffer();

	/**
	 * Frees the main screen drawing surface, only if it's available.
	 */
	void freeScreen();

	TextData getTextData(DrawData ddId);

	/**
	 * Draws a cached widget directly on the screen. Currently deprecated.
	 *
	 * @param type DrawData type of the widget.
	 * @param r Position on screen to draw the widget.
	 */
	void drawCached(DrawData type, const Common::Rect &r);

	/**
	 *	Calculates the background threshold offset of a given DrawData item.
	 *	After fully loading all DrawSteps of a DrawData item, this function must be
	 *	called in order to calculate if such draw steps would be drawn outside of
	 *	the actual widget drawing zone (e.g. shadows). If this is the case, a constant
	 *	value will be added when restoring the background of the widget.
	 *
	 *	@param type DrawData type of the widget.
	 */
	void calcBackgroundOffset(DrawData type);

	/**
	 *	Generates a DrawQueue item and enqueues it so it's drawn to the screen
	 *	when the drawing queue is processed.
	 *
	 *	If Buffering is enabled, the DrawQueue item will be automatically placed
	 *	on its corresponding queue.
	 *	If Buffering is disabled, the DrawQueue item will be processed immediately
	 *	and drawn to the screen.
	 *
	 *	This function is called from all the Widget Drawing methods.
	 */
	void queueDD(DrawData type,  const Common::Rect &r, uint32 dynamic = 0);
	void queueDDText(TextData type, const Common::Rect &r, const Common::String &text, bool restoreBg,
		bool elipsis, TextAlign alignH = kTextAlignLeft, TextAlignVertical alignV = kTextAlignVTop, int deltax = 0);
	void queueBitmap(const Graphics::Surface *bitmap, const Common::Rect &r, bool alpha);

	/**
	 *	DEBUG: Draws a white square and writes some text next to it.
	 */
	void debugWidgetPosition(const char *name, const Common::Rect &r);

public:
	
	/**
	 *	LEGACY: Old GUI::Theme API
	 */
	
	bool needThemeReload() { 
		return ((_loadedThemeX != g_system->getOverlayWidth()) ||
				(_loadedThemeY != g_system->getOverlayHeight())); 
	}

	const Graphics::Font *loadFont(const Common::String &filename);
	const Graphics::Font *loadFontFromArchive(const Common::String &filename);
	Common::String genCacheFilename(const char *filename);
	
	Graphics::TextAlignment convertAligment(TextAlign align) const {
		switch (align) {
		case kTextAlignLeft:
			return Graphics::kTextAlignLeft;
			break;

		case kTextAlignRight:
			return Graphics::kTextAlignRight;
			break;

		default:
			break;
		};
		return Graphics::kTextAlignCenter;
	}

	TextAlign convertAligment(Graphics::TextAlignment align) const {
		switch (align) {
		case Graphics::kTextAlignLeft:
			return kTextAlignLeft;
			break;

		case Graphics::kTextAlignRight:
			return kTextAlignRight;
			break;

		default:
			break;
		}
		return kTextAlignCenter;
	}


	bool isThemeLoadingRequired();
	
	static bool themeConfigUseable(const Common::FSNode &node, Common::String &themeName);
	static bool themeConfigParseHeader(Common::String header, Common::String &themeName);

	int getTabSpacing() const { return 0; }
	int getTabPadding() const { return 3; }

protected:
	OSystem *_system; /** Global system object. */

	/** Vector Renderer object, does the actual drawing on screen */
	Graphics::VectorRenderer *_vectorRenderer;

	/** XML Parser, does the Theme parsing instead of the default parser */
	GUI::ThemeParser *_parser;

	/** Theme getEvaluator (changed from GUI::Eval to add functionality) */
	GUI::ThemeEval *_themeEval;

	/** Main screen surface. This is blitted straight into the overlay. */
	Graphics::Surface *_screen;

	/** Backbuffer surface. Stores previous states of the screen to blit back */
	Graphics::Surface *_backBuffer;

	/** Sets whether the current drawing is being buffered (stored for later
		processing) or drawn directly to the screen. */
	bool _buffering;

	/** Bytes per pixel of the Active Drawing Surface (i.e. the screen) */
	int _bytesPerPixel;

	/** Current graphics mode */
	GraphicsMode _graphicsMode;

	/** Font info. */
	Common::String _fontName;
	const Graphics::Font *_font;

	/** Array of all the DrawData elements than can be drawn to the screen.
		Must be full so the renderer can work. */
	WidgetDrawData *_widgets[kDrawDataMAX];

	/** Array of all the text fonts that can be drawn. */
	TextDrawData *_texts[kTextDataMAX];

	ImagesMap _bitmaps;

	/** List of all the dirty screens that must be blitted to the overlay. */
	Common::List<Common::Rect> _dirtyScreen;

	/** Queue with all the drawing that must be done to the Back Buffer */
	Common::List<ThemeItem *> _bufferQueue;

	/** Queue with all the drawing that must be done to the screen */
	Common::List<ThemeItem *> _screenQueue;

	bool _initOk; /** Class and renderer properly initialized */
	bool _themeOk; /** Theme data successfully loaded. */
	bool _enabled; /** Whether the Theme is currently shown on the overlay */

	Common::String _themeName; /** Name of the currently loaded theme */
	Common::String _themeFileName;

	/** Custom Cursor Management */
	void setUpCursor();

	bool _useCursor;
	int _cursorHotspotX, _cursorHotspotY;
	int _cursorTargetScale;
	enum { MAX_CURS_COLORS = 255 };
	byte *_cursor;
	bool _needPaletteUpdates;
	uint _cursorWidth, _cursorHeight;
	byte _cursorPal[4*MAX_CURS_COLORS];
	int _loadedThemeX, _loadedThemeY;
};

} // end of namespace GUI.

#endif
