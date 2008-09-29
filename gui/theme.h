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
 */

#ifndef GUI_THEME_H
#define GUI_THEME_H

#include "common/system.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/fs.h"
#include "common/config-file.h"

#include "graphics/surface.h"
#include "graphics/fontman.h"

#define THEME_VERSION 24
#define SCUMMVM_THEME_VERSION_STR "SCUMMVM_THEME_V23"

namespace GUI {

class ThemeEval;

/**
 * Our theme renderer class.
 *
 * It is used to draw the different widgets and
 * getting the layout of the widgets for different
 * resolutions.
 */
class Theme {
public:
	Theme();

	virtual ~Theme();

	//! Defined the align of the text
	enum TextAlign {
		kTextAlignLeft,		//! Text should be aligned to the left
		kTextAlignCenter,	//! Text should be centered
		kTextAlignRight		//! Text should be aligned to the right
	};
	
	//! Vertical alignment of the text.
	enum TextAlignVertical {
		kTextAlignVBottom,
		kTextAlignVCenter,
		kTextAlignVTop
	};

	//! Widget background type
	enum WidgetBackground {
		kWidgetBackgroundNo,			//! No background at all
		kWidgetBackgroundPlain,			//! Simple background, this may not include borders
		kWidgetBackgroundBorder,		//! Same as kWidgetBackgroundPlain just with a border
		kWidgetBackgroundBorderSmall,	//! Same as kWidgetBackgroundPlain just with a small border
		kWidgetBackgroundEditText,		//! Background used for edit text fields
		kWidgetBackgroundSlider			//! Background used for sliders
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
		kStateDisabled,		//! Indicates that the widget is disabled, that does NOT include that it is invisible
		kStateEnabled,		//! Indicates that the widget is enabled
		kStateHighlight		//! Indicates that the widget is highlighted by the user
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
		kFontStyleBold = 0,			//! A bold font. This is also the default font.
		kFontStyleNormal = 1,		//! A normal font.
		kFontStyleItalic = 2,		//! Italic styled font.
		kFontStyleFixedNormal = 3,	//! Fixed size font.
		kFontStyleFixedBold = 4,	//! Fixed size bold font.
		kFontStyleFixedItalic = 5,	//! Fixed size italic font.
		kFontStyleMax
	};

	//! Function used to process areas other than the current dialog
	enum ShadingStyle {
		kShadingNone,		//! No special post processing
		kShadingDim,		//! Dimming unused areas
		kShadingLuminance	//! Converting colors to luminance for unused areas
	};

	/**
	 * This initializes all the data needed by the theme renderer.
	 * It should just be called *once*, when first using the renderer.
	 *
	 * Other functions of the renderer should just be used after
	 * calling this function, else the result is undefined.
	 *
	 * If used again it should just be used after deinit,
	 * if there is need to use the renderer again.
	 *
	 * @see deinit
	 */
	virtual bool init() = 0;

	/**
	 * Unloads all data used by the theme renderer.
	 */
	virtual void deinit() = 0;

	/**
	 * Updates the renderer to changes to resolution,
	 * bit depth and other video related configuration.
	 */
	virtual void refresh() = 0;

	/**
	 * Checks if the theme supplies its own cursor.
	 *
	 * @return true if using an own cursor
	 */
	virtual bool ownCursor() const { return false; }

	/**
	 * Enables the theme renderer for use.
	 *
	 * This for examples displays the overlay, clears the
	 * renderer's temporary screen buffers and does other
	 * things to make the renderer for use.
	 *
	 * This will NOT back up the data on the overlay.
	 * So if you've got data in the overlay save it before
	 * calling this.
	 *
	 * Unlike init, this makes the renderer ready to draw
	 * something to the screen. And of course it relies on the data
	 * loaded by init.
	 *
	 * @see disable
	 * @see init
	 */
	virtual void enable() = 0;

	/**
	 * Disables the theme renderer.
	 *
	 * This for example hides the overlay and undoes
	 * other things done by enable.
	 *
	 * Unlike uninit, this just makes the renderer unable
	 * to do any screen drawing, but still keeps all data
	 * loaded into memory.
	 *
	 * @see enable
	 * @see uninit
	 */
	virtual void disable() = 0;

	/**
	 * Tells the theme renderer that a new dialog is opened.
	 *
	 * This can be used for internal caching and marking
	 * area of all but the not top dialog in a special way.
	 *
	 * TODO: This needs serious reworking, since at least for
	 * normal usage, a dialog opened with openDialog should always
	 * be the top dialog. Currently our themes have no good enough
	 * implementation to handle a single open dialog though, so we
	 * have to stay this way until we implement proper dialog
	 * 'caching'/handling.
	 *
	 * @param topDialog	if true it indicates that this is the top dialog
	 *
	 * @see closeAllDialogs
	 */
	virtual void openDialog(bool topDialog, ShadingStyle shading = kShadingNone) = 0;

	/**
	 * This indicates that all dialogs have been closed.
	 *
	 * @see openDialog
	 */
	virtual void closeAllDialogs() = 0;

	/**
	 * Closes the topmost dialog, and redraws the screen
	 * accordingly.
	 *
	 * TODO: Make this purely virtual by making ThemeClassic
	 * and ThemeModern implement it too.
	 *
	 * @returns True if the dialog was sucessfully closed.
	 *          If we weren't able to restore the screen after closing
	 *          the dialog, we return false, which means we need to redraw
	 *          the dialog stack from scratch.
	 */
	virtual void startBuffering() = 0;
	virtual void finishBuffering() = 0;

	/**
	 * Clear the complete GUI screen.
	 */
	virtual void clearAll() = 0;

	/**
	 * Update the GUI screen aka overlay.
	 *
	 * This does NOT call OSystem::updateScreen,
	 * it just copies all (changed) data to the overlay.
	 */
	virtual void updateScreen() = 0;

	virtual const Graphics::Font *getFont(FontStyle font = kFontStyleBold) const = 0;
	virtual int getFontHeight(FontStyle font = kFontStyleBold) const = 0;
	virtual int getStringWidth(const Common::String &str, FontStyle font = kFontStyleBold) const = 0;
	virtual int getCharWidth(byte c, FontStyle font = kFontStyleBold) const = 0;

	virtual void drawDialogBackground(const Common::Rect &r, DialogBackground type, WidgetStateInfo state = kStateEnabled) = 0;
	virtual void drawText(const Common::Rect &r, const Common::String &str, WidgetStateInfo state = kStateEnabled, TextAlign align = kTextAlignCenter, bool inverted = false, int deltax = 0, bool useEllipsis = true, FontStyle font = kFontStyleBold) = 0;
	// this should ONLY be used by the debugger until we get a nicer solution
	virtual void drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, WidgetStateInfo state = kStateEnabled) = 0;

	virtual void drawWidgetBackground(const Common::Rect &r, uint16 hints, WidgetBackground background = kWidgetBackgroundPlain, WidgetStateInfo state = kStateEnabled) = 0;
	virtual void drawButton(const Common::Rect &r, const Common::String &str, WidgetStateInfo state = kStateEnabled, uint16 hints = 0) = 0;
	virtual void drawSurface(const Common::Rect &r, const Graphics::Surface &surface, WidgetStateInfo state = kStateEnabled, int alpha = 256, bool themeTrans = false) = 0;
	virtual void drawSlider(const Common::Rect &r, int width, WidgetStateInfo state = kStateEnabled) = 0;
	virtual void drawCheckbox(const Common::Rect &r, const Common::String &str, bool checked, WidgetStateInfo state = kStateEnabled) = 0;
	virtual void drawTab(const Common::Rect &r, int tabHeight, int tabWidth, const Common::Array<Common::String> &tabs, int active, uint16 hints, int titleVPad, WidgetStateInfo state = kStateEnabled) = 0;
	virtual void drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState, WidgetStateInfo state = kStateEnabled) = 0;
	virtual void drawPopUpWidget(const Common::Rect &r, const Common::String &sel, int deltax, WidgetStateInfo state = kStateEnabled, TextAlign align = kTextAlignLeft) = 0;
	virtual void drawCaret(const Common::Rect &r, bool erase, WidgetStateInfo state = kStateEnabled) = 0;
	virtual void drawLineSeparator(const Common::Rect &r, WidgetStateInfo state = kStateEnabled) = 0;

	virtual void restoreBackground(Common::Rect r, bool special = false) = 0;
	virtual bool addDirtyRect(Common::Rect r, bool save = false, bool special = false) = 0;

	virtual int getTabSpacing() const = 0;
	virtual int getTabPadding() const = 0;

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
	virtual ThemeEval *evaluator() = 0;

	static bool themeConfigUseable(const Common::FilesystemNode &node, Common::String &themeName);
	static bool themeConfigParseHeader(Common::String header, Common::String &themeName);

	virtual const Common::String &getThemeFileName() const = 0;
	virtual const Common::String &getThemeName() const = 0;
	virtual int getGraphicsMode() const = 0;

	/**
	 * Checks if the theme renderer supports drawing of images.
	 *
	 * @return true on support, else false
	 */
	virtual bool supportsImages() const { return false; }

	//! Special image ids for images used in the GUI
	enum kThemeImages {
		kImageLogo = 0,		//! ScummVM Logo used in the launcher
		kImageLogoSmall		//! ScummVM logo used in the GMM
	};

	/**
	 * Returns the given image.
	 *
	 * @param n	id of the image, see kThemeImages
	 * @return 0 if no such image exists for the theme, else pointer to the image
	 *
	 * @see kThemeImages
	 */
	virtual const Graphics::Surface *getImageSurface(const kThemeImages n) const { return 0; }
protected:

	const Graphics::Font *loadFont(const char *filename);
	Common::String genCacheFilename(const char *filename);

public:
	bool needThemeReload() { return ((_loadedThemeX != g_system->getOverlayWidth()) ||
									 (_loadedThemeY != g_system->getOverlayHeight())); }

private:
	static const char *_defaultConfigINI;
	int _loadedThemeX, _loadedThemeY;
};
} // end of namespace GUI

#endif // GUI_THEME_H
