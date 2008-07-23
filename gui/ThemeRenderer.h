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

#ifndef INTERFACE_MANAGER_H
#define INTERFACE_MANAGER_H

#include "common/scummsys.h"
#include "graphics/surface.h"
#include "common/system.h"

#include "graphics/surface.h"
#include "graphics/fontman.h"

#include "gui/dialog.h"
#include "gui/ThemeParser.h"
#include "graphics/VectorRenderer.h"

namespace GUI {

struct WidgetDrawData;
struct DrawDataInfo;

struct TextDrawData {
	const Graphics::Font *_fontPtr;
	
	struct { 
		uint8 r, g, b;
	} _color;
};

struct WidgetDrawData {
	/** List of all the steps needed to draw this widget */
	Common::List<Graphics::DrawStep> _steps;
	
	int _textDataId;
	GUI::Theme::TextAlign _textAlignH;
	GUI::Theme::TextAlignVertical _textAlignV;

	/** Extra space that the widget occupies when it's drawn.
	    E.g. when taking into account rounded corners, drop shadows, etc 
		Used when restoring the widget background */
	uint16 _backgroundOffset;

	/** Sets whether the widget is cached beforehand. */
	bool _cached;
	bool _buffer;

	/** Texture where the cached widget is stored. */
	Graphics::Surface *_surfaceCache;

	~WidgetDrawData() {
		_steps.clear();

		if (_surfaceCache) {
			_surfaceCache->free();
			delete _surfaceCache;
		}
	}
};

class ThemeRenderer : public Theme {
	
protected:
	typedef Common::String String;
	typedef GUI::Dialog Dialog;

	friend class GUI::Dialog;
	friend class GUI::GuiObject;

	/** Constant value to expand dirty rectangles, to make sure they are fully copied */
	static const int kDirtyRectangleThreshold = 2;
	
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

		kDDCheckboxEnabled,
		kDDCheckboxDisabled,

		kDDTabActive,
		kDDTabInactive,

		kDDScrollbarBase,
		kDDScrollbarHandle,

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
	
	/**
	 *	Queue Node for the drawing queue.
	 *	Specifies the exact drawing to be done when processing
	 *	the drawing queues.
	 */
	struct DrawQueue {
		DrawData type;		/** DrawData item to draw. */
		Common::Rect area;	/** Place on screen to draw it. */
		uint32 dynData;		/** Dynamic data which modifies the DrawData item (optional)*/
	};
	
	enum TextData {
		kTextDataNone = -1,
		kTextDataDefault = 0,
		kTextDataHover,
		kTextDataDisabled,
		kTextDataInverted,
		kTextDataMAX
	};
	
	static const struct TextDataInfo {
		TextData id;
		const char *name;
	} kTextDataDefaults[];
	
	struct DrawQueueText {
		TextData type;
		Common::Rect area;
		Common::String text;
		
		GUI::Theme::TextAlign alignH;
		GUI::Theme::TextAlignVertical alignV;
		bool elipsis;
		bool restoreBg;
		int deltax;
	};
	
public:
	/** Graphics mode enumeration.
	 *	Each item represents a set of BPP and Renderer modes for a given
	 * surface.
	 */
	enum GraphicsMode {
		kGfxDisabled = 0,	/** No GFX */
		kGfxStandard16bit,	/** 2BPP with the standard (aliased) renderer. */
		kGfxAntialias16bit	/** 2BPP with the optimized AA renderer. */
	};
	
	/** Default constructor */
	ThemeRenderer(Common::String themeName, GraphicsMode mode);

	/** Default destructor */
	~ThemeRenderer() {
		freeRenderer();
		freeScreen();
		unloadTheme();
		delete _parser;
	}

	/**
	 *	VIRTUAL METHODS
	 *	This is the implementation of the GUI::Theme API to allow
	 *	the ThemeRenderer class to be plugged in as any other GUI
	 *	theme. In fact, the renderer works like any other GUI theme,
	 *	but supports extensive customization of the drawn widgets.
	 */
	bool init();
	void deinit();
	void clearAll();

	void refresh() {}
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
	void openDialog(bool enableBuffering);
	
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
	const Graphics::Font *getFont(FontStyle font) const { return _font; }
	int getFontHeight(FontStyle font = kFontStyleBold) const { if (_initOk) return _font->getFontHeight(); return 0; }
	int getStringWidth(const Common::String &str, FontStyle font) const { if (_initOk) return _font->getStringWidth(str); return 0; }
	int getCharWidth(byte c, FontStyle font) const { if (_initOk) return _font->getCharWidth(c); return 0; }


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

	void drawDialogBackground(const Common::Rect &r, uint16 hints, WidgetStateInfo state);
	
	void drawText(const Common::Rect &r, const Common::String &str, 
		WidgetStateInfo state, TextAlign align, bool inverted, int deltax, bool useEllipsis, FontStyle font);
	
	void drawChar(const Common::Rect &r, byte ch, 
		const Graphics::Font *font, WidgetStateInfo state) {}
	
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
	void addDrawStep(const Common::String &drawDataId, Graphics::DrawStep step);
	
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
	bool addFont(const Common::String &fontName, int r, int g, int b);
	
	/**
	 *	Adds a new TextStep from the ThemeParser. This will be deprecated/removed once the 
	 *	new Font API is in place.
	 */
	bool addTextData(const Common::String &drawDataId, const Common::String &textDataId, TextAlign alignH, TextAlignVertical alignV);

	/** Interface to the new Theme XML parser */
	ThemeParser *parser() {
		return _parser;
	}

	/**
	 *	Returns if the Theme is ready to draw stuff on screen.
	 *	Must be called instead of just checking _initOk, because
	 *	this checks if the renderer is initialized AND if the theme
	 *	is loaded.
	 */
	bool ready() {
		return _initOk && _themeOk;
	}

	/** Custom implementation of the GUI::Theme API, changed to use the XML parser. */
	bool loadTheme(Common::String themeName);
	
	/**
	 *	Changes the active graphics mode of the GUI; may be used to either
	 *	initialize the GUI or to change the mode while the GUI is already running.
	 */
	void setGraphicsMode(GraphicsMode mode);


	/**
	 *	Finishes buffering: widgets from there one will be drawn straight on the screen
	 *	without drawing queues.
	 */
	void finishBuffering() {
		_buffering = false;
	}

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
	 *	Loads the given theme into the ThemeRenderer.
	 *	Note that ThemeName is an identifier, not a filename.
	 *
	 *	@param ThemeName Theme identifier.
	 *	@returns True if the theme was succesfully loaded.
	 */
	bool loadThemeXML(Common::String themeName);
	
	/**
	 *	Loads the default theme file (the embeded XML file found
	 *	in ThemeDefaultXML.cpp).
	 *	Called only when no other themes are available.
	 */
	bool loadDefaultXML();

	/**
	 *	Unloads the currently loaded theme so another one can
	 *	be loaded.
	 */
	void unloadTheme() {
		if (!_themeOk)
			return;

		for (int i = 0; i < kDrawDataMAX; ++i) {
			delete _widgets[i];
			_widgets[i] = 0;
		}
		
		for (int i = 0; i < kTextDataMAX; ++i) {
			delete _texts[i];
			_texts[i] = 0;
		}

		_themeOk = false;
	}

	/**
	 * Not implemented yet.
	 * TODO: reload themes, reload the renderer, recheck everything
	 */
	void screenChange() {}
	
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
	void freeRenderer() {
		delete _vectorRenderer;
		_vectorRenderer = 0;
	}
	
	/**
	 * Frees the Back buffer surface, only if it's available.
	 */
	void freeBackbuffer() {
		if (_backBuffer != 0) {
			_backBuffer->free();
			delete _backBuffer;
			_backBuffer = 0;
		}
	}

	/**
	 * Frees the main screen drawing surface, only if it's available.
	 */
	void freeScreen() {
		if (_screen != 0) {
			_screen->free();
			delete _screen;
			_screen = 0;
		}
	}
	
	TextData getTextData(DrawData ddId) {
		return _widgets[ddId] ? (TextData)_widgets[ddId]->_textDataId : kTextDataNone;
	}
	
	/**
	 *	Checks if a given DrawData set for a widget has been cached beforehand
	 *	and is ready to be blit into the screen.
	 *
	 *	@param type DrawData type of the widget.
	 *	@param r Size of the widget which is expected to be cached.
	 */
	bool isWidgetCached(DrawData type, const Common::Rect &r);
	
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
	 *	Draws a DrawQueue item (which contains DrawData information and a screen
	 *	position) into the screen.
	 *
	 *	This is the only way the Theme has to drawn on the screen.
	 *
	 *	@param draw Sets if the DrawData info will be actually drawn.
	 *	@param restore	Sets if the background behind the widget will be restored before drawing.
	 */
	inline void drawDD(const DrawQueue &q, bool draw = true, bool restore = false);
	inline void drawDDText(const DrawQueueText &q);
	
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
	inline void queueDD(DrawData type,  const Common::Rect &r, uint32 dynamic = 0);
	inline void queueDDText(TextData type, const Common::Rect &r, const Common::String &text, bool restoreBg,
		bool elipsis, TextAlign alignH = kTextAlignLeft, TextAlignVertical alignV = kTextAlignVTop, int deltax = 0);
	
	/**
	 *	DEBUG: Draws a white square around the given position and writes the given next to it.
	 */
	inline void debugWidgetPosition(const char *name, const Common::Rect &r);


	/**
	 *	Wrapper for restoring data from the Back Buffer to the screen.
	 *	The actual processing is done in the VectorRenderer.
	 *
	 *	@param r Area to restore.
	 *	@param special Deprecated.
	 */
	void restoreBackground(Common::Rect r, bool special = false);

	
	/**
	 *	Default values from GUI::Theme
	 */
	int getTabSpacing() const { return 0; }
	int getTabPadding() const { return 3; }

	OSystem *_system; /** Global system object. */
	
	/** Vector Renderer object, does the actual drawing on screen */
	Graphics::VectorRenderer *_vectorRenderer;
	
	/** XML Parser, does the Theme parsing instead of the default parser */
	GUI::ThemeParser *_parser;

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
	
	/** List of all the dirty screens that must be blitted to the overlay. */
	Common::List<Common::Rect> _dirtyScreen;
	
	/** Queue with all the drawing that must be done to the Back Buffer */
	Common::List<DrawQueue> _bufferQueue;
	
	/** Queue with all the drawing that must be done to the screen */
	Common::List<DrawQueue> _screenQueue;
	
	/** Queue with all the text drawing that must be done to the screen */
	Common::List<DrawQueueText> _textQueue;

	bool _initOk; /** Class and renderer properly initialized */
	bool _themeOk; /** Theme data successfully loaded. */
	bool _enabled; /** Whether the Theme is currently shown on the overlay */

	Common::String _themeName; /** Name of the currently loaded theme */
};

} // end of namespace GUI.

#endif
