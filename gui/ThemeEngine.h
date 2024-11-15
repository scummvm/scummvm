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

#ifndef GUI_THEME_ENGINE_H
#define GUI_THEME_ENGINE_H

#include "common/scummsys.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/language.h"
#include "common/list.h"
#include "common/str.h"
#include "common/rect.h"

#include "graphics/managed_surface.h"
#include "graphics/font.h"
#include "graphics/pixelformat.h"


#define SCUMMVM_THEME_VERSION_STR "SCUMMVM_STX0.9.20"

class OSystem;

namespace Graphics {
struct DrawStep;
class VectorRenderer;
}

namespace GUI {

struct WidgetDrawData;
struct TextDrawData;
class Dialog;
class GuiObject;
class ThemeEval;
class ThemeParser;

/**
 * DrawData sets enumeration.
 * Each DD set corresponds to the actual looks
 * of a widget in a given state.
 */
enum DrawData {
	kDDMainDialogBackground,
	kDDSpecialColorBackground,
	kDDPlainColorBackground,
	kDDTooltipBackground,
	kDDDefaultBackground,
	kDDTextSelectionBackground,
	kDDTextSelectionFocusBackground,
	kDDThumbnailBackground,
	kDDGridItemIdle,
	kDDGridItemHover,
	kDDWidgetBackgroundDefault,
	kDDWidgetBackgroundSmall,
	kDDWidgetBackgroundEditText,
	kDDWidgetBackgroundSlider,

	kDDButtonIdle,
	kDDButtonHover,
	kDDButtonDisabled,
	kDDButtonPressed,

	kDDDropDownButtonIdle,
	kDDDropDownButtonHoverLeft,
	kDDDropDownButtonHoverRight,
	kDDDropDownButtonDisabled,
	kDDDropDownButtonPressedLeft,
	kDDDropDownButtonPressedRight,

	kDDDropDownButtonIdleRTL,
	kDDDropDownButtonHoverLeftRTL,
	kDDDropDownButtonHoverRightRTL,
	kDDDropDownButtonDisabledRTL,
	kDDDropDownButtonPressedLeftRTL,
	kDDDropDownButtonPressedRightRTL,

	kDDSliderFull,
	kDDSliderHover,
	kDDSliderDisabled,

	kDDCheckboxDefault,
	kDDCheckboxDisabled,
	kDDCheckboxSelected,
	kDDCheckboxDisabledSelected,

	kDDRadiobuttonDefault,
	kDDRadiobuttonDisabled,
	kDDRadiobuttonSelected,

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
	kDDPopUpDisabled,

	kDDPopUpIdleRTL,
	kDDPopUpHoverRTL,
	kDDPopUpDisabledRTL,

	kDDCaret,
	kDDSeparator,
	kDrawDataMAX,
	kDDNone = -1
};

/**
 * Dialog layers.
 * The currently active dialog has two layers, background and foreground.
 * The background layer is drawn to the backbuffer. The foreground layer
 * is drawn to the screen. This allows draw calls to restore the background
 * layer before redrawing a widget.
 */
enum DrawLayer {
	kDrawLayerBackground,
	kDrawLayerForeground
};

// FIXME: TextData is really a bad name, not conveying what this enum is about.
enum TextData {
	kTextDataNone = -1,
	kTextDataDefault = 0,
	kTextDataButton,
	kTextDataNormalFont,
	kTextDataTooltip,
	kTextDataConsole,
	kTextDataExtraLang,
	kTextDataMAX
};

enum TextColor {
	kTextColorNormal = 0,
	kTextColorNormalInverted,
	kTextColorNormalHover,
	kTextColorNormalDisabled,
	kTextColorAlternative,
	kTextColorAlternativeInverted,
	kTextColorAlternativeHover,
	kTextColorAlternativeDisabled,
	kTextColorOverride,
	kTextColorOverrideInverted,
	kTextColorOverrideHover,
	kTextColorOverrideDisabled,
	kTextColorButton,
	kTextColorButtonHover,
	kTextColorButtonDisabled,
	kTextColorMAX
};

struct TextColorData {
	int r, g, b;
};

class LangExtraFont {
public:
	LangExtraFont(TextData textId, Common::Array<Common::Language> &lngs, const Common::String &filename, const Common::String &scalableFile, int ps) : _langs(lngs) {
		storeFileNames(textId, filename, scalableFile, ps);
	}

	void storeFileNames(TextData textId, const Common::String &filename, const Common::String &scalableFile, int ps) {
		assert(textId < kTextDataMAX);
		_fontFilesStd[textId] = filename;
		_fontFilesScalable[textId] = scalableFile;
		_fontSize[textId] = ps;
	}

	bool operator==(Common::Language l) const {
		return (Common::find(_langs.begin(), _langs.end(), l) != _langs.end());
	}

	Common::String file(TextData textId) const { return _fontFilesStd[textId]; }
	Common::String sclFile(TextData textId) const { return _fontFilesScalable[textId]; }
	int fntSize(TextData textId) const { return _fontSize[textId]; }

private:
	Common::Array<Common::Language> _langs;
	Common::String _fontFilesStd[kTextDataMAX];
	Common::String _fontFilesScalable[kTextDataMAX];
	int _fontSize[kTextDataMAX];
};

class ThemeEngine {
protected:
	typedef Common::HashMap<Common::String, Graphics::ManagedSurface *> ImagesMap;

	friend class GUI::Dialog;
	friend class GUI::GuiObject;

public:
	/// Vertical alignment of the text.
	enum TextAlignVertical {
		kTextAlignVInvalid,
		kTextAlignVBottom,
		kTextAlignVCenter,
		kTextAlignVTop
	};

	/// Widget background type
	enum WidgetBackground {
		kWidgetBackgroundNo,            ///< No background at all
		kWidgetBackgroundPlain,         ///< Simple background, this may not include borders
		kWidgetBackgroundBorder,        ///< Same as kWidgetBackgroundPlain just with a border
		kWidgetBackgroundBorderSmall,   ///< Same as kWidgetBackgroundPlain just with a small border
		kWidgetBackgroundEditText,      ///< Background used for edit text fields
		kWidgetBackgroundSlider,        ///< Background used for sliders
		kThumbnailBackground,			///< Background used for thumbnails
		kGridItemBackground,			///< Default Background used for grid items
		kGridItemHighlight				///< Highlight Background used for grid items
	};

	/// Dialog background type
	enum DialogBackground {
		kDialogBackgroundMain,
		kDialogBackgroundSpecial,
		kDialogBackgroundPlain,
		kDialogBackgroundTooltip,
		kDialogBackgroundDefault,
		kDialogBackgroundNone
	};

	/// State of the widget to be drawn
	enum State {
		kStateDisabled,     ///< Indicates that the widget is disabled, that does NOT include that it is invisible
		kStateEnabled,      ///< Indicates that the widget is enabled
		kStateHighlight,    ///< Indicates that the widget is highlighted by the user
		kStatePressed       ///< Indicates that the widget is pressed, currently works for buttons
	};

	typedef State WidgetStateInfo;

	/// Text inversion state of the text to be draw
	enum TextInversionState {
		kTextInversionNone, ///< Indicates that the text should not be drawn inverted
		kTextInversion,     ///< Indicates that the text should be drawn inverted, but not focused
		kTextInversionFocus ///< Indicates that the text should be drawn inverted, and focused
	};

	enum ScrollbarState {
		kScrollbarStateNo,
		kScrollbarStateUp,
		kScrollbarStateDown,
		kScrollbarStateSlider,
		kScrollbarStateSinglePage
	};

	/// Font style selector
	enum FontStyle {
		kFontStyleBold = 0,         ///< A bold font. This is also the default font.
		kFontStyleNormal = 1,       ///< A normal font.
		kFontStyleItalic = 2,       ///< Italic styled font.
		kFontStyleFixedNormal = 3,  ///< Fixed size font.
		kFontStyleFixedBold = 4,    ///< Fixed size bold font.
		kFontStyleFixedItalic = 5,  ///< Fixed size italic font.
		kFontStyleTooltip = 6,      ///< Tiny console font
		kFontStyleConsole = 7,      ///< Debug console font
		kFontStyleLangExtra = 8,	///< Language specific font for ingame dialogs (e. g. the SCUMM pause/restart dialogs)
		kFontStyleMax
	};

	/// Font color selector
	enum FontColor {
		kFontColorFormatting = -1,	///< Use color from formatting
		kFontColorNormal = 0,       ///< The default color of the theme
		kFontColorAlternate = 1,    ///< Alternative font color
		kFontColorOverride = 2,     ///< Color of overwritten text
		kFontColorMax
	};

	/// Function used to process areas other than the current dialog
	enum ShadingStyle {
		kShadingNone,       ///< No special post processing
		kShadingDim,        ///< Dimming unused areas
		kShadingLuminance   ///< Converting colors to luminance for unused areas
	};

	/// AlphaBitmap scale mode selector
	enum AutoScaleMode {
		kAutoScaleNone = 0,		///< Use image dimensions
		kAutoScaleStretch = 1,	///< Stretch image to full widget size
		kAutoScaleFit = 2,		///< Scale image to widget size but keep aspect ratio
		kAutoScaleNinePatch = 3 ///< 9-patch image
	};

	// Special image ids for images used in the GUI
	static const char *const kImageLogo;      ///< ScummVM logo used in the launcher
	static const char *const kImageLogoSmall; ///< ScummVM logo used in the GMM
	static const char *const kImageSearch;    ///< Search tool image used in the launcher
	static const char *const kImageGroup;     ///< Select Group image used in the launcher
	static const char *const kImageEraser;     ///< Clear input image used in the launcher
	static const char *const kImageDelButton; ///< Delete characters in the predictive dialog
	static const char *const kImageList;      ///< List image used in save/load chooser selection
	static const char *const kImageGrid;      ///< Grid image used in save/load chooser selection
	static const char *const kImageStopButton; ///< Stop recording button in recorder onscreen dialog
	static const char *const kImageEditButton; ///< Edit recording metadata in recorder onscreen dialog
	static const char *const kImageSwitchModeButton; ///< Switch mode button in recorder onscreen dialog
	static const char *const kImageFastReplayButton; ///< Fast playback mode button in recorder onscreen dialog
	static const char *const kImageStopSmallButton; ///< Stop recording button in recorder onscreen dialog (for 320xY)
	static const char *const kImageEditSmallButton; ///< Edit recording metadata in recorder onscreen dialog (for 320xY)
	static const char *const kImageSwitchModeSmallButton; ///< Switch mode button in recorder onscreen dialog (for 320xY)
	static const char *const kImageFastReplaySmallButton; ///< Fast playback mode button in recorder onscreen dialog (for 320xY)

	/**
	 * Graphics mode enumeration.
	 * Each item represents a set of BPP and Renderer modes for a given
	 * surface.
	 */
	enum GraphicsMode {
		kGfxDisabled = 0,   ///< No GFX
		kGfxStandard,  ///< Standard (aliased) renderer.
		kGfxAntialias  ///< Optimized AA renderer.
	};

	/** Constant value to expand dirty rectangles, to make sure they are fully copied */
	static const int kDirtyRectangleThreshold = 1;

	struct Renderer {
		const char *name;
		const char *shortname;
		const char *cfg;
		GraphicsMode mode;
	};

	static const Renderer _rendererModes[];
	static const uint _rendererModesSize;

	static const GraphicsMode _defaultRendererMode;

	static GraphicsMode findMode(const Common::String &cfg);
	static const char *findModeConfigName(GraphicsMode mode);

	/** Default constructor */
	ThemeEngine(Common::String id, GraphicsMode mode);

	/** Default destructor */
	~ThemeEngine();

	void setBaseResolution(int w, int h, float s);
	bool init();
	void clearAll();

	void refresh();
	void enable();

	void showCursor();
	void hideCursor();

	void disable();


	/**
	 * Query the set up pixel format.
	 */
	const Graphics::PixelFormat getPixelFormat() const { return _overlayFormat; }

	/**
	 * Draw full screen shading with the supplied style
	 *
	 * This is used to dim the inactive dialogs so the active one stands out.
	 */
	void applyScreenShading(ShadingStyle shading);

	/**
	 * Sets the active drawing surface to the back buffer.
	 *
	 * All drawing from this point on will be done on that surface.
	 * The back buffer surface needs to be copied to the screen surface
	 * in order to become visible.
	 */
	void drawToBackbuffer();

	/**
	 * Sets the active drawing surface to the screen.
	 *
	 * All drawing from this point on will be done on that surface.
	 */
	void drawToScreen();

	/**
	 * The updateScreen() method is called every frame.
	 * It copies dirty rectangles in the Screen surface to the overlay.
	 */
	void updateScreen();

	/**
	 * Copy the entire backbuffer surface to the screen surface
	 */
	void copyBackBufferToScreen();


	/** @name FONT MANAGEMENT METHODS */
	//@{

	TextData fontStyleToData(FontStyle font) const {
		if (font == kFontStyleNormal)
			return kTextDataNormalFont;
		if (font == kFontStyleTooltip)
			return kTextDataTooltip;
		if (font == kFontStyleConsole)
			return kTextDataConsole;
		if (font == kFontStyleLangExtra)
			return kTextDataExtraLang;
		return kTextDataDefault;
	}

	const Graphics::Font *getFont(FontStyle font = kFontStyleBold) const;

	int getFontHeight(FontStyle font = kFontStyleBold) const;

	int getStringWidth(const Common::U32String &str, FontStyle font = kFontStyleBold) const;

	int getCharWidth(uint32 c, FontStyle font = kFontStyleBold) const;

	int getKerningOffset(uint32 left, uint32 right, FontStyle font = kFontStyleBold) const;

	//@}

	/**
	 * Set the clipping rect to be used by the widget drawing methods defined below.
	 *
	 * Widgets are not drawn outside of the clipping rect. Widgets that overlap the
	 * clipping rect are drawn partially.
	 *
	 * @param newRect The new clipping rect
	 * @return The previous clipping rect
	 */
	Common::Rect swapClipRect(const Common::Rect &newRect);
	const Common::Rect getClipRect();

	/**
	 * Set the clipping rect to allow rendering on the whole surface.
	 */
	void disableClipRect();

	/** @name WIDGET DRAWING METHODS */
	//@{

	void drawWidgetBackground(const Common::Rect &r, WidgetBackground background);

	void drawButton(const Common::Rect &r, const Common::U32String &str, WidgetStateInfo state = kStateEnabled,
	                uint16 hints = 0);

	void drawDropDownButton(const Common::Rect &r, uint32 dropdownWidth, const Common::U32String &str,
	                        WidgetStateInfo buttonState, bool inButton, bool inDropdown, bool rtl = false);

	void drawManagedSurface(const Common::Point &p, const Graphics::ManagedSurface &surface, Graphics::AlphaType alphaType);

	void drawSlider(const Common::Rect &r, int width, WidgetStateInfo state = kStateEnabled, bool rtl = false);

	void drawCheckbox(const Common::Rect &r, int spacing, const Common::U32String &str, bool checked,
	                  WidgetStateInfo state = kStateEnabled, bool override = false, bool rtl = false);

	void drawRadiobutton(const Common::Rect &r, int spacing, const Common::U32String &str, bool checked,
	                     WidgetStateInfo state = kStateEnabled, bool rtl = false);

	void drawTab(const Common::Rect &r, int tabHeight, const Common::Array<int> &tabWidths,
	             const Common::Array<Common::U32String> &tabs, int active, bool rtl,
				 ThemeEngine::TextAlignVertical alignV);

	void drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState scrollState);

	void drawPopUpWidget(const Common::Rect &r, const Common::U32String &sel, int deltax,
	                     WidgetStateInfo state = kStateEnabled, bool rtl = false);

	void drawCaret(const Common::Rect &r, bool erase);

	void drawLineSeparator(const Common::Rect &r);

	void drawDialogBackground(const Common::Rect &r, DialogBackground type);

	void drawText(const Common::Rect &r, const Common::U32String &str, WidgetStateInfo state = kStateEnabled,
	              Graphics::TextAlign align = Graphics::kTextAlignCenter,
	              TextInversionState inverted = kTextInversionNone, int deltax = 0, bool useEllipsis = true,
	              FontStyle font = kFontStyleBold, FontColor color = kFontColorNormal, bool restore = true,
	              const Common::Rect &drawableTextArea = Common::Rect(0, 0, 0, 0));

	void drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, FontColor color = kFontColorNormal, TextInversionState inverted = ThemeEngine::kTextInversionNone);

	void drawFoldIndicator(const Common::Rect &r, bool expanded);

	//@}



	/**
	 * Actual implementation of a dirty rect handling.
	 * Dirty rectangles are queued on a list, merged and optimized
	 * when possible and are later used for the actual drawing.
	 *
	 * @param r Area of the dirty rect.
	 */
	void addDirtyRect(Common::Rect r);


	/**
	 * Returns the DrawData enumeration value that represents the given string
	 * in the DrawDataDefaults enumeration.
	 * It's slow, but called sparsely.
	 *
	 * @returns The drawdata enum value, or -1 if not found.
	 * @param name The representing name, as found on Theme Description XML files.
	 * @see kDrawDataDefaults[]
	 */
	DrawData parseDrawDataId(const Common::String &name) const;

	TextData getTextData(DrawData ddId) const;
	TextColor getTextColor(DrawData ddId) const;

	TextColorData *getTextColorData(TextColor color) const;

	/**
	 * Interface for ThemeParser class: Parsed DrawSteps are added via this function.
	 * There is no return type because DrawSteps can always be added, unless something
	 * goes horribly wrong.
	 * The specified step will be added to the Steps list of the given DrawData id.
	 *
	 * @param drawDataId The representing DrawData name, as found on Theme Description XML files.
	 * @param step The actual DrawStep struct to be added.
	 */
	void addDrawStep(const Common::String &drawDataId, const Graphics::DrawStep &step);

	/**
	 * Interface for the ThemeParser class: Parsed DrawData sets are added via this function.
	 * The goal of the function is to initialize each DrawData set before their DrawSteps can
	 * be added, hence this must be called for each DD set before addDrawStep() can be called
	 * for that given set.
	 *
	 * @param data The representing DrawData name, as found on Theme Description XML files.
	 * @param cached Whether this DD set will be cached beforehand.
	 */
	bool addDrawData(const Common::String &data, bool cached);


	/**
	 * Interface for the ThemeParser class: Loads a font to use on the GUI from the given
	 * filename.
	 *
	 * @param textId            Identifier name for the font.
	 * @param language          Wildcard for the language(s) to use.
	 * @param file              Filename of the non-scalable font version.
	 * @param scalableFile      Filename of the scalable version. (Optional)
	 * @param pointsize         Point size for the scalable font. (Optional)
	 */
	bool addFont(TextData textId, const Common::String &language, const Common::String &file, const Common::String &scalableFile, const int pointsize);

	/**
	 * Store language specific font names for ingame GUI dialogs which might require
	 * a different language than the current GUI setting
	 *
	 * @param textId, language, file, scalableFile, pointsize			All exactly the same as with addFont()
	*/
	void storeFontNames(TextData textId, const Common::String &language, const Common::String &file, const Common::String &scalableFile, const int pointsize);

	/**
	 * Load language specific font for ingame use
	 * @param style				font style associated with the font file
	 * @param lang				language associated with the font file
	 * @return
	*/
	bool loadExtraFont(FontStyle style, Common::Language lang);

	/**
	 * Interface for the ThemeParser class: adds a text color value.
	 *
	 * @param colorId Identifier for the color type.
	 * @param r Red color component
	 * @param g Green color component
	 * @param b Blue color component
	 */
	bool addTextColor(TextColor colorId, int r, int g, int b);


	/**
	 * Interface for the ThemeParser class: Loads a bitmap file to use on the GUI.
	 * The filename is also used as its identifier.
	 *
	 * @param filename Name of the bitmap file.
	 * @param filename Name of the scalable (SVG) file, could be empty
	 * @param width, height Default image dimensions
	 */
	bool addBitmap(const Common::String &filename, const Common::String &scalablefile, int widht, int height);

	/**
	 * Adds a new TextStep from the ThemeParser. This will be deprecated/removed once the
	 * new Font API is in place. FIXME: Is that so ???
	 */
	bool addTextData(const Common::String &drawDataId, TextData textId, TextColor id, Graphics::TextAlign alignH, TextAlignVertical alignV);

protected:
	/**
	 * Returns if the Theme is ready to draw stuff on screen.
	 * Must be called instead of just checking _initOk, because
	 * this checks if the renderer is initialized AND if the theme
	 * is loaded.
	 */
	bool ready() const {
		return _initOk && _themeOk;
	}

	/** Load the them from the file with the specified name. */
	void loadTheme(const Common::String &themeid);

	/**
	 * Changes the active graphics mode of the GUI; may be used to either
	 * initialize the GUI or to change the mode while the GUI is already running.
	 */
	void setGraphicsMode(GraphicsMode mode);

public:
	inline ThemeEval *getEvaluator() { return _themeEval; }
	inline Graphics::VectorRenderer *renderer() { return _vectorRenderer; }

	inline bool supportsImages() const { return true; }
	inline bool ownCursor() const { return _useCursor; }

	Graphics::ManagedSurface *getImageSurface(const Common::String &name) const {
		return _bitmaps.contains(name) ? _bitmaps[name] : 0;
	}

	/**
	 * Interface for the Theme Parser: Creates a new cursor by loading the given
	 * bitmap and sets it as the active cursor.
	 *
	 * @param filename File name of the bitmap to load.
	 * @param hotspotX X Coordinate of the bitmap which does the cursor click.
	 * @param hotspotY Y Coordinate of the bitmap which does the cursor click.
	 */
	bool createCursor(const Common::String &filename, int hotspotX, int hotspotY);

	/**
	 * Wrapper for restoring data from the Back Buffer to the screen.
	 * The actual processing is done in the VectorRenderer.
	 *
	 * @param r Area to restore.
	 */
	void restoreBackground(Common::Rect r);

	const Common::String &getThemeName() const { return _themeName; }
	const Common::String &getThemeId() const { return _themeId; }
	int getGraphicsMode() const { return _graphicsMode; }

protected:

	/**
	 * Loads the given theme into the ThemeEngine.
	 *
	 * @param themeId Theme identifier.
	 * @returns true if the theme was successfully loaded.
	 */
	bool loadThemeXML(const Common::String &themeId);

	/**
	 * Loads the default theme file (the embedded XML file found
	 * in ThemeDefaultXML.cpp).
	 * Called only when no other themes are available.
	 */
	bool loadDefaultXML();

	/**
	 * Unloads the currently loaded theme so another one can
	 * be loaded.
	 */
	void unloadTheme();

	/**
	 * Unload the language specific font loaded via loadExtraFont()
	*/
	void unloadExtraFont();

	const Graphics::Font *loadScalableFont(const Common::String &filename, const int pointsize, Common::String &name);
	const Graphics::Font *loadFont(const Common::String &filename, Common::String &name);
	Common::String genCacheFilename(const Common::String &filename) const;
	const Graphics::Font *loadFont(const Common::String &filename, const Common::String &scalableFilename, const int pointsize, const bool makeLocalizedFont);

	/**
	 * Dirty Screen handling function.
	 * Draws all the dirty rectangles in the list to the overlay.
	 */
	void updateDirtyScreen();

	/**
	 * Draws a GUI element according to a DrawData descriptor.
	 *
	 * Only calls with a DrawData layer attribute matching the active layer
	 * are actually drawn to the active surface.
	 *
	 * These functions are called from all the Widget drawing methods.
	 */
	void drawDD(DrawData type, const Common::Rect &r, uint32 dynamic = 0, bool forceRestore = false);
	void drawDDText(TextData type, TextColor color, const Common::Rect &r, const Common::U32String &text, bool restoreBg,
	                bool elipsis, Graphics::TextAlign alignH = Graphics::kTextAlignLeft,
	                TextAlignVertical alignV = kTextAlignVTop, int deltax = 0,
	                const Common::Rect &drawableTextArea = Common::Rect(0, 0, 0, 0));

	/**
	 * DEBUG: Draws a white square and writes some text next to it.
	 */
	void debugWidgetPosition(const char *name, const Common::Rect &r);

public:
	struct ThemeDescriptor {
		Common::String name;
		Common::String id;
		Common::Path filename;
	};

	/**
	 * Lists all theme files useable.
	 */
	static void listUsableThemes(Common::List<ThemeDescriptor> &list);
private:
	static bool themeConfigUsable(const Common::FSNode &node, Common::String &themeName);
	static bool themeConfigUsable(const Common::ArchiveMember &member, Common::String &themeName);
	static bool themeConfigParseHeader(Common::String header, Common::String &themeName);

	static Common::Path getThemeFile(const Common::String &id);
	static Common::String getThemeId(const Common::Path &filename);
	static void listUsableThemes(const Common::FSNode &node, Common::List<ThemeDescriptor> &list, int depth = -1);
	static void listUsableThemes(Common::Archive &archive, Common::List<ThemeDescriptor> &list);

protected:
	OSystem *_system; /** Global system object. */

	/** Vector Renderer object, does the actual drawing on screen */
	Graphics::VectorRenderer *_vectorRenderer;

	/** XML Parser, does the Theme parsing instead of the default parser */
	GUI::ThemeParser *_parser;

	/** Theme getEvaluator (changed from GUI::Eval to add functionality) */
	GUI::ThemeEval *_themeEval;

	/** Main screen surface. This is blitted straight into the overlay. */
	Graphics::ManagedSurface _screen;

	/** Backbuffer surface. Stores previous states of the screen to blit back */
	Graphics::ManagedSurface _backBuffer;

	/**
	 * Filter the submitted DrawData descriptors according to their layer attribute
	 *
	 * This is used to selectively draw the background or foreground layer
	 * of the dialogs.
	 */
	DrawLayer _layerToDraw;

	/** Bytes per pixel of the Active Drawing Surface (i.e. the screen) */
	int _bytesPerPixel;

	/** Current graphics mode */
	GraphicsMode _graphicsMode;

	int16 _baseWidth, _baseHeight;
	float _scaleFactor;
	bool _needScaleRefresh = false;

	/** Font info. */
	const Graphics::Font *_font;

	/**
	 * Array of all the DrawData elements than can be drawn to the screen.
	 * Must be full so the renderer can work.
	 */
	WidgetDrawData *_widgets[kDrawDataMAX];

	/** Array of all the text fonts that can be drawn. */
	TextDrawData *_texts[kTextDataMAX];

	/** Array of all font colors available. */
	TextColorData *_textColors[kTextColorMAX];

	/** Extra font file names for languages like Japanese, Korean or Chinese
	 *  for use in ingame dialogs (like the SCUMM pause/restart dialogs)
	 */
	Common::Array<LangExtraFont> _langExtraFonts;

	ImagesMap _bitmaps;
	Graphics::PixelFormat _overlayFormat;
	Graphics::PixelFormat _cursorFormat;

	/** List of all the dirty screens that must be blitted to the overlay. */
	Common::List<Common::Rect> _dirtyScreen;

	bool _initOk;  ///< Class and renderer properly initialized
	bool _themeOk; ///< Theme data successfully loaded.
	bool _enabled; ///< Whether the Theme is currently shown on the overlay

	Common::String _themeName; ///< Name of the currently loaded theme
	Common::String _themeId;
	Common::Path _themeFile;
	Common::Archive *_themeArchive;
	Common::SearchSet _themeFiles;

	bool _useCursor;
	int _cursorHotspotX, _cursorHotspotY;
	uint32 _cursorTransparent;
	byte *_cursor;
	uint _cursorWidth, _cursorHeight;

	enum {
		MAX_CURS_COLORS = 255
	};
	byte _cursorPal[3 * MAX_CURS_COLORS];
	byte _cursorPalSize;

	Common::Rect _clip;
};

} // End of namespace GUI.

#endif
