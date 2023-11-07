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
class Actor;

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

		// Reserved for custom colors, loaded from PICT resources.
		kCustomColor = 100,

		kBackground = 254,	// Gray or checkerboard
		kTransparency = 255
	};

	enum FontId {
		kSystemFont,

		kAboutFontRegular,
		kAboutFontBold,
		kAboutFontExtraBold,
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

	enum TextStyle {
		kStyleHeader,
		kStyleBold,
		kStyleExtraBold,
		kStyleRegular
	};

	struct TextLine {
		int x;
		int y;
		TextStyle style;
		Graphics::TextAlign align;
		const char *str;
	};

	enum MacDialogWindowStyle {
		kStyleNormal,
		kStyleRounded
	};

	int delay(uint32 ms = 0);

	virtual bool getFontParams(FontId fontId, int &id, int &size, int &slant) const;

	Common::String getDialogString(Common::SeekableReadStream *res, int len);

	virtual bool handleMenu(int id, Common::String &name);

	virtual void runAboutDialog() = 0;
	virtual bool runOpenDialog() = 0;
	virtual bool runSaveDialog() = 0;
	virtual bool runOptionsDialog() = 0;

	bool runOkCancelDialog(Common::String text);
	bool runQuitDialog();
	bool runRestartDialog();

public:
	class MacDialogWindow;

	class MacGuiObject {
	protected:
		bool _redraw = false;
		bool _enabled = false;
		Common::Rect _bounds;

	public:
		MacGuiObject(Common::Rect bounds, bool enabled) : _bounds(bounds), _enabled(enabled) {}
		virtual ~MacGuiObject() {}

		Common::Rect getBounds() const { return _bounds; }
	};

	class MacWidget : public MacGuiObject {
	protected:
		MacGui::MacDialogWindow *_window;
		int _id = -1;

		bool _visible = true;
		bool _fullRedraw = false;

		Common::String _text;
		int _value = 0;

		int drawText(Common::String text, int x, int y, int w, Color color, Graphics::TextAlign align = Graphics::kTextAlignLeft, int deltax = 0);

	public:
		MacWidget(MacGui::MacDialogWindow *window, Common::Rect bounds, Common::String text, bool enabled);
		virtual ~MacWidget() {};

		void setId(int id) { _id = id; }
		int getId() const { return _id; }

		// Visibility never changes after initialization, so it does
		// not trigger a redraw.
		void setVisible(bool visible) { _visible = visible; }
		bool isVisible() const { return _visible; }

		virtual void getFocus() { setRedraw(); }
		virtual void loseFocus() { setRedraw(); }

		void setRedraw(bool fullRedraw = false);

		bool isEnabled() const { return _enabled; }
		void setEnabled(bool enabled);

		virtual void setValue(int value);
		int getValue() const { return _value; }

		virtual bool useBeamCursor() { return false; }
		virtual bool findWidget(int x, int y) const;

		virtual void draw(bool drawFocused = false) = 0;

		virtual void handleMouseDown(Common::Event &event) {}
		virtual void handleMouseUp(Common::Event &event) {}
		virtual void handleMouseMove(Common::Event &event) {}
		virtual void handleMouseHeld() {}
		virtual bool handleKeyDown(Common::Event &event) { return false; }
	};

	class MacButton : public MacWidget {
	private:
		struct CornerLine {
			int start;
			int length;
		};

	public:
		MacButton(MacGui::MacDialogWindow *window, Common::Rect bounds, Common::String text, bool enabled) : MacWidget(window, bounds, text, enabled) {}

		void draw(bool drawFocused = false);

		void drawCorners(Common::Rect r, CornerLine *corner);
	};

	class MacCheckbox : public MacWidget {
	private:
		Common::Rect _hitBounds;
		bool _isChecked = false;

	public:
		MacCheckbox(MacGui::MacDialogWindow *window, Common::Rect bounds, Common::String text, bool enabled);

		bool findWidget(int x, int y) const;
		void draw(bool drawFocused = false);
		void handleMouseUp(Common::Event &event);
	};

	// The dialogs add texts as disabled, but we don't want it to be drawn
	// as disabled so we enable it and make it "disabled" by giving it a
	// custom findWidget().

	class MacStaticText : public MacWidget {
	public:
		MacStaticText(MacGui::MacDialogWindow *window, Common::Rect bounds, Common::String text, bool enabled) : MacWidget(window, bounds, text, true) {}

		bool findWidget(int x, int y) const { return false; }

		void draw(bool drawFocused = false);
	};

	class MacEditText : public MacWidget {
	private:
		int _textPos = 1;
		int _selectLen = 0;
		int _caretPos = 0;
		int _caretX = -1;

		uint32 _lastClickTime = 0;
		uint32 _lastScrollTime = 0;

		int _lastClickX = 0;

		uint32 _nextCaretBlink = 0;
		bool _caretVisible = true;

		const Graphics::Font *_font;
		Graphics::Surface _textSurface;

		int getTextPosFromMouse(int x, int y);

		void updateSelection(int x, int y);
		void deleteSelection();

	public:
		MacEditText(MacGui::MacDialogWindow *window, Common::Rect bounds, Common::String text, bool enabled);

		void getFocus() {}
		void loseFocus() {}

		void selectAll();

		bool useBeamCursor() { return true; }
		bool findWidget(int x, int y) const;

		void draw(bool drawFocused = false);

		void handleMouseDown(Common::Event &event);
		bool handleKeyDown(Common::Event &event);
		void handleMouseMove(Common::Event &event);
		void handleMouseHeld();
	};

	class MacPicture : public MacWidget {
	private:
		Graphics::Surface *_picture = nullptr;

	public:
		MacPicture(MacGui::MacDialogWindow *window, Common::Rect bounds, int id, bool enabled);
		~MacPicture();

		Graphics::Surface *getPicture() const { return _picture; }

		void draw(bool drawFocused = false);
	};

	class MacSlider : public MacWidget {
	private:
		int _minValue;
		int _maxValue;
		int _pageSize;

	public:
		MacSlider(MacGui::MacDialogWindow *window, Common::Rect bounds, int minValue, int maxValue, int pageSize, bool enabled)
			: MacWidget(window, bounds, "Slider", enabled),
			_minValue(minValue), _maxValue(maxValue), _pageSize(pageSize) {}

		void draw(bool drawFocued = false);

		void handleMouseDown(Common::Event &event);
		void handleMouseMove(Common::Event &event);
	};

	class MacPictureSlider : public MacWidget {
	private:
		MacPicture *_background;
		MacPicture *_handle;
		int _minX;
		int _maxX;
		int _handleX;
		int _grabOffset;
		int _lastHandleX = -1;
		int _minValue;
		int _maxValue;
		int _leftMargin;
		int _rightMargin;

	public:
		 MacPictureSlider(MacGui::MacDialogWindow *window, MacPicture *background, MacPicture *handle, bool enabled, int minX, int maxX, int minValue, int maxValue, int leftMargin, int rightMargin)
			: MacWidget(window, background->getBounds(), "Slider", enabled),
			_background(background), _handle(handle), _minX(minX),
			_maxX(maxX), _minValue(minValue), _maxValue(maxValue),
			_leftMargin(leftMargin), _rightMargin(rightMargin) {}

		bool findWidget(int x, int y) const;
		void setValue(int value);
		void draw(bool drawFocused = false);

		void handleMouseDown(Common::Event &event);
		void handleMouseUp(Common::Event &event);
		void handleMouseMove(Common::Event &event);
	};

	class MacDialogWindow {
	private:
		Common::Rect _bounds;
		int _margin;

		bool _visible = false;

		Graphics::Surface *_beamCursor = nullptr;
		Common::Point _beamCursorPos;
		bool _beamCursorVisible = false;
		int _beamCursorHotspotX = 3;
		int _beamCursorHotspotY = 4;

		void drawBeamCursor();
		void undrawBeamCursor();

		PauseToken _pauseToken;

		Graphics::Surface *_from = nullptr;
		Graphics::Surface *_backup = nullptr;
		Graphics::Surface _surface;
		Graphics::Surface _innerSurface;

		Common::Array<MacWidget *> _widgets;

		MacWidget *_defaultWidget = nullptr;

		int _mouseOverWidget = -1;
		MacWidget *_focusedWidget = nullptr;
		Common::Point _focusClick;
		Common::Point _oldMousePos;
		Common::Point _mousePos;
		Common::Point _realMousePos;

		Common::StringArray _substitutions;
		Common::Array<Common::Rect> _dirtyRects;

		void copyToScreen(Graphics::Surface *s = nullptr) const;

	public:
		OSystem *_system;
		MacGui *_gui;

		MacDialogWindow(MacGui *gui, OSystem *system, Graphics::Surface *from, Common::Rect bounds, MacDialogWindowStyle style = kStyleNormal);
		~MacDialogWindow();

		Graphics::Surface *surface() { return &_surface; }
		Graphics::Surface *innerSurface() { return &_innerSurface; }

		bool isVisible() const { return _visible; }

		void show();
		int runDialog();
		void updateCursor();

		void setDefaultWidget(int nr) { _defaultWidget = _widgets[nr]; }
		MacWidget *getDefaultWidget() const { return _defaultWidget; }

		void setFocusedWidget(int x, int y);
		void clearFocusedWidget();
		MacWidget *getFocusedWidget() const { return _focusedWidget; }
		Common::Point getFocusClick() const { return _focusClick; }
		Common::Point getMousePos() const { return _mousePos; }

		void setWidgetEnabled(int nr, bool enabled) { _widgets[nr]->setEnabled(enabled); }
		bool isWidgetEnabled(int nr) const { return _widgets[nr]->isEnabled(); }
		void setWidgetVisible(int nr, bool visible) { _widgets[nr]->setVisible(visible); }
		int getWidgetValue(int nr) const { return _widgets[nr]->getValue(); }
		void setWidgetValue(int nr, int value) { _widgets[nr]->setValue(value); }
		int findWidget(int x, int y) const;
		void redrawWidget(int nr) { _widgets[nr]->setRedraw(true); }

		MacGui::MacButton *addButton(Common::Rect bounds, Common::String text, bool enabled);
		MacGui::MacCheckbox *addCheckbox(Common::Rect bounds, Common::String text, bool enabled);
		MacGui::MacStaticText *addStaticText(Common::Rect bounds, Common::String text, bool enabled);
		MacGui::MacEditText *addEditText(Common::Rect bounds, Common::String text, bool enabled);
		MacGui::MacPicture *addPicture(Common::Rect bounds, int id, bool enabled);
		MacGui::MacSlider *addSlider(int x, int y, int h, int minValue, int maxValue, int pageSize, bool enabled);
		MacGui::MacPictureSlider *addPictureSlider(int backgroundId, int handleId, bool enabled, int minX, int maxX, int minValue, int maxValue, int leftMargin = 0, int rightMargin = 0);

		void addSubstitution(Common::String text) { _substitutions.push_back(text); }
		void replaceSubstitution(int nr, Common::String text) { _substitutions[nr] = text; }

		bool hasSubstitution(uint n) const { return n < _substitutions.size(); }
		Common::String &getSubstitution(uint n) { return _substitutions[n]; }

		void markRectAsDirty(Common::Rect r);
		void update(bool fullRedraw = false);

		static void plotPixel(int x, int y, int color, void *data);
		static void plotPattern(int x, int y, int pattern, void *data);
		static void plotPatternDarkenOnly(int x, int y, int pattern, void *data);

		void fillPattern(Common::Rect r, uint16 pattern);
		void drawSprite(const Graphics::Surface *sprite, int x, int y);
		void drawSprite(const Graphics::Surface *sprite, int x, int y, Common::Rect clipRect);
		void drawTexts(Common::Rect r, const TextLine *lines);
		void drawTextBox(Common::Rect r, const TextLine *lines, int arc = 9);
	};

	MacGui(ScummEngine *vm, Common::String resourceFile);
	virtual ~MacGui();

	Graphics::Surface *surface() { return _surface; }

	virtual const Common::String name() const = 0;

	void setPalette(const byte *palette, uint size);
	virtual bool handleEvent(Common::Event &event);
  
	static void menuCallback(int id, Common::String &name, void *data);
	virtual void initialize();
	void updateWindowManager();

	const Graphics::Font *getFont(FontId fontId);
	virtual const Graphics::Font *getFontByScummId(int32 id) = 0;

	Graphics::Surface *loadPict(int id);
	Graphics::Surface *decodePictV1(Common::SeekableReadStream *res);

	virtual bool isVerbGuiActive() const { return false; }
	virtual void reset() {}
	virtual void resetAfterLoad() = 0;
	virtual void update(int delta) = 0;

	virtual void setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate) = 0;

	virtual Graphics::Surface *textArea() { return nullptr; }
	virtual void clearTextArea() {}
	virtual void initTextAreaForActor(Actor *a, byte color) {}
	virtual void printCharToTextArea(int chr, int x, int y, int color) {}

	MacDialogWindow *createWindow(Common::Rect bounds, MacDialogWindowStyle style = kStyleNormal);
	MacDialogWindow *createDialog(int dialogId);
	MacDialogWindow *drawBanner(char *message);
};

class MacLoomGui : public MacGui {
public:
	MacLoomGui(ScummEngine *vm, Common::String resourceFile);
	~MacLoomGui();

	const Common::String name() const { return "Loom"; }

	bool handleEvent(Common::Event &event);

	const Graphics::Font *getFontByScummId(int32 id);

	void setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate);

	void resetAfterLoad();
	void update(int delta);

protected:
	bool getFontParams(FontId fontId, int &id, int &size, int &slant) const;

	bool handleMenu(int id, Common::String &name);

	void runAboutDialog();
	bool runOpenDialog();
	bool runSaveDialog();
	bool runOptionsDialog();

private:
	Graphics::Surface *_practiceBox = nullptr;
	Common::Point _practiceBoxPos;
	int _practiceBoxNotes;
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

	Graphics::Surface _textArea;

	const Graphics::Font *getFontByScummId(int32 id);

	void setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate);

	Graphics::Surface *textArea() { return &_textArea; }
	void clearTextArea() { _textArea.fillRect(Common::Rect(_textArea.w, _textArea.h), kBlack); }
	void initTextAreaForActor(Actor *a, byte color);
	void printCharToTextArea(int chr, int x, int y, int color);

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

protected:
	bool getFontParams(FontId fontId, int &id, int &size, int &slant) const;

	bool handleMenu(int id, Common::String &name);

	void runAboutDialog();
	bool runOpenDialog();
	bool runSaveDialog();
	bool runOptionsDialog();
	bool runIqPointsDialog();

private:
	bool _visible = false;

	bool _leftButtonIsPressed = false;
	Common::Point _leftButtonPressed;
	Common::Point _leftButtonHeld;

	int _timer = 0;

	bool updateVerbs(int delta);
	void updateMouseHeldTimer(int delta);
	void drawVerbs();

	void clearAboutDialog(MacDialogWindow *window);

	int getInventoryScrollOffset() const;
	void setInventoryScrollOffset(int n) const;

	class Widget : public MacGuiObject {
	private:
		int _timer = 0;

	public:
		static ScummEngine *_vm;
		static MacIndy3Gui *_gui;
		static Graphics::Surface *_surface;

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
