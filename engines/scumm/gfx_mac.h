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

#ifndef SCUMM_GFX_MAC_H
#define SCUMM_GFX_MAC_H

#include "graphics/font.h"

class OSystem;

namespace Graphics {
struct Surface;
class MacWindowManager;
}

namespace Scumm {

class ScummEngine;

class MacGui {
protected:
	ScummEngine *_vm = nullptr;
	OSystem *_system = nullptr;

	Graphics::MacWindowManager *_windowManager = nullptr;
	Graphics::Surface *_surface = nullptr;
	Common::String _resourceFile;

	bool _menuIsActive = false;
	bool _cursorWasVisible = false;

	Common::HashMap<int, const Graphics::Font *> _fonts;
	int _gameFontId = -1;

	enum Color {
		kBlack = 0,
		kBlue = 1,
		kGreen = 2,
		kCyan = 3,
		kRed = 4,
		kMagenta = 5,
		kBrown = 6,
		kLightGray = 7,
		kDarkGray = 8,
		kBrightBlue = 9,
		kBrightGreen = 10,
		kBrightCyan = 11,
		kBrightRed = 12,
		kBrightMagenta = 13,
		kBrightYellow = 14,
		kWhite = 15,
		kBackground = 254,	// Gray or checkerboard
		kTransparency = 255
	};

public:
	enum TextStyle {
		kStyleHeader,
		kStyleBold,
		kStyleRegular
	};

	struct TextLine {
		int x;
		int y;
		TextStyle style;
		Graphics::TextAlign align;
		const char *str;
	};

	enum SimpleWindowStyle {
		kStyleNormal,
		kStyleRounded
	};

	class SimpleWindow {
	private:
		MacGui *_gui;
		OSystem *_system;
		Common::Rect _bounds;
		int _margin;

		PauseToken _pauseToken;

		Graphics::Surface *_from = nullptr;
		Graphics::Surface *_backup = nullptr;
		Graphics::Surface _surface;
		Graphics::Surface _innerSurface;

		Common::Array<Common::Rect> _dirtyRects;

		void copyToScreen(Graphics::Surface *s = nullptr) const;
	public:
		SimpleWindow(MacGui *gui, OSystem *system, Graphics::Surface *from, Common::Rect bounds, SimpleWindowStyle style = kStyleNormal);
		~SimpleWindow();

		Graphics::Surface *surface() { return &_surface; }
		Graphics::Surface *innerSurface() { return &_innerSurface; }

		void show();

		void markRectAsDirty(Common::Rect r);
		void update(bool fullRedraw = false);

		static void plotPixel(int x, int y, int color, void *data);
		static void plotPattern(int x, int y, int pattern, void *data);

		void fillPattern(Common::Rect r, uint16 pattern);
		void drawSprite(const Graphics::Surface *sprite, int x, int y);
		void drawSprite(const Graphics::Surface *sprite, int x, int y, Common::Rect clipRect);
		void drawTextBox(Common::Rect r, const TextLine *lines, int arc = 9);
	};

	enum FontId {
		kSystemFont,

		kAboutFontRegular,
		kAboutFontBold,
		kAboutFontHeaderInside,
		kAboutFontHeaderOutside,

		kIndy3FontSmall,
		kIndy3FontMedium,
		kIndy3VerbFontRegular,
		kIndy3VerbFontBold,
		kIndy3VerbFontOutline,

		kLoomFontSmall,
		kLoomFontMedium,
		kLoomFontLarge
	};

	MacGui(ScummEngine *vm, Common::String resourceFile);
	virtual ~MacGui();

	virtual const Common::String name() const = 0;

	static void menuCallback(int id, Common::String &name, void *data);

	virtual void initialize();

	const Graphics::Font *getFont(FontId fontId);
	virtual const Graphics::Font *getFontByScummId(int32 id) = 0;
	virtual bool getFontParams(FontId fontId, int &id, int &size, int &slant);

	Graphics::Surface *loadPict(int id);

	virtual bool handleMenu(int id, Common::String &name);

	virtual void showAboutDialog() = 0;

	virtual bool isVerbGuiActive() const { return false; }
	virtual void reset() {}
	virtual void resetAfterLoad() {}
	virtual void update(int delta) {}

	void updateWindowManager();

	virtual void setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate) = 0;
	virtual bool handleEvent(Common::Event &event);

	void setPalette(const byte *palette, uint size);

	SimpleWindow *drawBanner(char *message);

	int delay(uint32 ms);
	SimpleWindow *openWindow(Common::Rect bounds, SimpleWindowStyle style = kStyleNormal);
};

class MacLoomGui : public MacGui {
public:
	MacLoomGui(ScummEngine *vm, Common::String resourceFile) : MacGui(vm, resourceFile) {}
	~MacLoomGui() {}

	const Common::String name() const { return "Loom"; }

	const Graphics::Font *getFontByScummId(int32 id);
	bool getFontParams(FontId fontId, int &id, int &size, int &slant);

	void setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate);

	bool handleMenu(int id, Common::String &name);

	void showAboutDialog();
};

class MacIndy3Gui : public MacGui {
public:
	enum ScrollDirection {
		kScrollUp,
		kScrollDown
	};

	MacIndy3Gui(ScummEngine *vm, Common::String resourceFile);
	~MacIndy3Gui();

	const Common::String name() const { return "Indy"; }

	const Graphics::Font *getFontByScummId(int32 id);
	bool getFontParams(FontId fontId, int &id, int &size, int &slant);

	void setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate);

	bool handleMenu(int id, Common::String &name);

	void showAboutDialog();
	void clearAboutDialog(SimpleWindow *window);

	// There is a distinction between the GUI being allowed and being
	// active. Allowed means that it's allowed to draw verbs, but not that
	// it necessarily is. Active means that there are verbs on screen. From
	// the outside, only the latter is relevant.
	//
	// One case where this makes a difference is when boxing with the
	// coach. During the "10 minutes later" sign, the GUI is active but
	// it's not drawing verbs, so the SCUMM engine is allowed to draw in
	// the verb area to clear the power meters and text.

	bool isVerbGuiActive() const;

	void reset();
	void resetAfterLoad();
	void update(int delta);
	bool handleEvent(Common::Event &event);

	int getInventoryScrollOffset() const;
	void setInventoryScrollOffset(int n) const;

private:
	bool _visible = false;

	bool _leftButtonIsPressed = false;
	Common::Point _leftButtonPressed;
	Common::Point _leftButtonHeld;

	int _timer = 0;

	bool updateVerbs(int delta);
	void updateMouseHeldTimer(int delta);
	void drawVerbs();

	class Widget {
	private:
		int _timer = 0;

	protected:
		bool _redraw = false;
		bool _enabled = false;

	public:
		static ScummEngine *_vm;
		static MacIndy3Gui *_gui;
		static Graphics::Surface *_surface;

		Common::Rect _bounds;

		Widget(int x, int y, int width, int height);
		virtual ~Widget() {}

		void setEnabled(bool enabled) {
			if (enabled != _enabled)
				setRedraw(true);
			if (!_enabled)
				_timer = 0;
			_enabled = enabled;
		}

		void setTimer(int t) { _timer = t; }
		void clearTimer() { _timer = 0; }
		bool hasTimer() const { return _timer > 0; }

		bool getRedraw() const { return _redraw; }
		virtual void setRedraw(bool redraw) { _redraw = redraw; }

		virtual void reset();

		virtual bool handleEvent(Common::Event &event) = 0;
		virtual bool handleMouseHeld(Common::Point &pressed, Common::Point &held) { return false; }
		virtual void updateTimer(int delta);
		virtual void timeOut() {}

		virtual void draw();
		virtual void undraw();

		byte translateChar(byte c) const;

		// Primitives
		void fill(Common::Rect r) const;
		void drawBitmap(Common::Rect r, const uint16 *bitmap, Color color) const;
		void drawShadowBox(Common::Rect r) const;
		void drawShadowFrame(Common::Rect r, Color shadowColor, Color fillColor) const;

		void markScreenAsDirty(Common::Rect r) const;
	};

	class VerbWidget : public Widget {
	protected:
		bool _visible = false;
		int _verbid = 0;
		int _verbslot = -1;
		bool _kill = false;

	public:
		VerbWidget(int x, int y, int width, int height) : Widget(x, y, width, height) {}

		void setVerbid(int n) { _verbid = n; }
		bool hasVerb() const { return _verbslot != -1; }
		bool isVisible() const { return _visible; }
		void threaten() { _kill = true; }
		bool isDying() const { return _kill; }

		void reset();

		virtual void updateVerb(int verbslot);

		void draw();
		void undraw();
	};

	class Button : public VerbWidget {
	private:
		Common::String _text;

	public:

		Button(int x, int y, int width, int height) : VerbWidget(x, y, width, height) {}

		bool handleEvent(Common::Event &event);

		void reset();
		void timeOut();
		void updateVerb(int verbslot);

		void draw();
	};

	class Inventory : public VerbWidget {
	private:
		class ScrollBar : public Widget {
		private:
			int _invCount = 0;
			int _invOffset = 0;

		public:
			ScrollBar(int x, int y, int width, int height);

			void setInventoryParameters(int invCount, int invOffset);
			void scroll(ScrollDirection dir);
			int getHandlePosition();

			void reset();

			bool handleEvent(Common::Event &event);

			void draw();
		};

		class ScrollButton : public Widget {
		public:
			ScrollDirection _direction;

			ScrollButton(int x, int y, int width, int height, ScrollDirection direction);

			bool handleEvent(Common::Event &event);
			bool handleMouseHeld(Common::Point &pressed, Common::Point &held);
			void timeOut();

			void draw();
		};

		class Slot : public Widget {
		private:
			Common::String _name;
			int _slot = -1;
			int _obj = -1;

		public:
			Slot(int slot, int x, int y, int width, int height);

			void clearName() { _name.clear(); }
			bool hasName() const { return !_name.empty(); }

			void clearObject();
			void setObject(int n);
			int getObject() const { return _obj; }

			void reset();

			bool handleEvent(Common::Event &event);
			void timeOut();

			void draw();
		};

		Slot *_slots[6];
		ScrollBar *_scrollBar;
		ScrollButton *_scrollButtons[2];

		static const uint16 _upArrow[16];
		static const uint16 _downArrow[16];

	public:
		Inventory(int x, int y, int width, int height);
		~Inventory();

		void setRedraw(bool redraw);

		void reset();

		bool handleEvent(Common::Event &event);
		bool handleMouseHeld(Common::Point &pressed, Common::Point &held);
		void updateTimer(int delta);
		void updateVerb(int verbslot);

		void draw();
	};

	Common::HashMap<int, VerbWidget *> _widgets;
	Common::Array<Common::Rect> _dirtyRects;

	bool isVerbGuiAllowed() const;

	void show();
	void hide();

	void fill(Common::Rect r) const;
	void drawBitmap(Common::Rect r, const uint16 *bitmap, Color color) const;

	void markScreenAsDirty(Common::Rect r);
	void copyDirtyRectsToScreen();
};

} // End of namespace Scumm

#endif
