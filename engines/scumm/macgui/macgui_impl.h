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

#ifndef SCUMM_MACGUI_MACGUI_IMPL_H
#define SCUMM_MACGUI_MACGUI_IMPL_H

#define TEXT_END_MARKER { 0, 0, kStyleRegular, Graphics::kTextAlignLeft, nullptr }

#include "common/events.h"
#include "common/hashmap.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/str-array.h"

#include "engines/engine.h"

#include "graphics/font.h"
#include "graphics/surface.h"

// Use negative values for these. Zero and upwards are reserved for widget IDs.

#define kDialogQuit           -1
#define kDialogWantsAttention -2

class OSystem;

namespace Graphics {
struct Surface;
class MacWindowManager;
}

namespace Scumm {

class ScummEngine;
class Actor;

class MacGuiImpl {
public:
	class MacDialogWindow;

	enum MacStringIds {
		kMSISkip = -1,
		kMSIAboutGameName = 1,
		kMSIRoughCommandMsg,
		kMSIAreYouSureYouWantToQuit,
		kMSIAreYouSureYouWantToRestart,
		kMSIGameName,
		kMSIOpenGameFile,
		kMSISaveGameFileAs,
		kMSIGameFile,
		kMSIAboutString1,
		kMSIAboutString2,
		kMSIAboutString3,
		kMSIAboutString4,
		kMSIAboutString5,
		kMSIAboutString6,
		kMSIAboutString7,
		kMSIAboutString8,
		kMSIAboutString9,
		kMSIAboutString10,
		kMSIAboutString11,
		kMSIAboutString12,
		kMSIAboutString13,
		kMSIAboutString14,
		kMSIAboutString15,
		kMSIAboutString16,
		kMSIAboutString17,
		kMSIAboutString18,
		kMSIAboutString19,
		kMSIAboutString20,
		kMSIAboutString21,
		kMSIAboutString22,
		kMSIAboutString23,
		kMSIAboutString24,
		kMSIAboutString25,
		kMSIAboutString26,
		kMSIAboutString27,
		kMSIAboutString28,
		kMSIAboutString29,
		kMSIAboutString30,
		kMSIAboutString31,
		kMSIAboutString32,
		kMSIAboutString33,
		kMSIAboutString34,
		kMSIAboutString35,
		kMSIAboutString36,
		kMSIAboutString37,
		kMSIAboutString38,
		kMSIAboutString39,
		kMSIAboutString40
	};

	enum ParsingMethod {
		kStrC,
		kStrPascal,
	};

	struct MacSTRSParsingEntry {
		MacStringIds strId;
		ParsingMethod parsingMethod;
		int numStrings;
	};

	enum MacWidgetType {
		kWidgetUnknown,
		kWidgetButton,
		kWidgetCheckbox,
		kWidgetStaticText,
		kWidgetEditText,
		kWidgetPicture,
		kWidgetSlider,
		kWidgetListBox,
		kWidgetPictureSlider
	};

protected:
	ScummEngine *_vm = nullptr;
	OSystem *_system = nullptr;

	Graphics::Surface *_surface = nullptr;
	MacGuiImpl::MacDialogWindow *_bannerWindow = nullptr;

	Common::Path _resourceFile;

	bool _paletteDirty = false;

	bool _menuIsActive = false;
	bool _cursorWasVisible = false;

	Common::HashMap<int, const Graphics::Font *> _fonts;
	Common::Array<Common::String> _strsStrings;

	int _gameFontId = -1;

	byte _unicodeToMacRoman[96];

	enum DelayStatus {
		kDelayDone = 0,
		kDelayInterrupted = 1,
		kDelayAborted
	};

	enum FontId {
		kSystemFont,

		kAboutFontRegular,
		kAboutFontBold,
		kAboutFontBold2,
		kAboutFontExtraBold,
		kAboutFontHeaderInside,
		kAboutFontHeaderOutside,
		kAboutFontHeader,
		kAboutFontHeaderSimple1,
		kAboutFontHeaderSimple2,

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
		kStyleHeader1,
		kStyleHeader2,
		kStyleHeaderSimple1,
		kStyleHeaderSimple2,
		kStyleBold,
		kStyleBold2,
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
		kWindowStyleNormal,
		kWindowStyleRounded
	};

	enum MacDialogMenuStyle {
		kMenuStyleNone,
		kMenuStyleDisabled,
		kMenuStyleApple
	};

	MacGuiImpl::DelayStatus delay(uint32 ms = 0);

	virtual bool getFontParams(FontId fontId, int &id, int &size, int &slant) const;

	Common::String getDialogString(Common::SeekableReadStream *res, int len);

	virtual bool handleMenu(int id, Common::String &name);

	virtual void runAboutDialog() = 0;
	virtual bool runOpenDialog(int &saveSlotToHandle);
	virtual bool runSaveDialog(int &saveSlotToHandle, Common::String &saveName);
	virtual bool runOptionsDialog() = 0;
	void prepareSaveLoad(Common::StringArray &savegameNames, bool *availSlots, int *slotIds, int size);

	bool runOkCancelDialog(Common::String text);

	bool readStrings();
	void parseSTRSBlock(uint8 *strsData, const MacSTRSParsingEntry *parsingTable, int parsingTableSize);

	// These are non interactable, no point in having them as widgets for now...
	void drawFakePathList(MacDialogWindow *window, Common::Rect r, const char *text);
	void drawFakeDriveLabel(MacDialogWindow *window, Common::Rect r, const char *text);

public:
	class MacGuiObject {
	protected:
		Common::Rect _bounds;
		bool _redraw = false;
		bool _enabled = false;
		bool _visible = true;

	public:
		MacGuiObject(Common::Rect bounds, bool enabled) : _bounds(bounds), _enabled(enabled) {}
		virtual ~MacGuiObject() {}

		Common::Rect getBounds() const { return _bounds; }
		bool getRedraw() const { return _redraw; }
		bool isEnabled() const { return _enabled; }
		bool isVisible() const { return _visible; }
	};

	class MacWidget : public MacGuiObject {
	protected:
		MacGuiImpl::MacDialogWindow *_window;
		uint32 _black;
		uint32 _white;

		int _id = -1;
		MacWidgetType _type = kWidgetUnknown;

		bool _fullRedraw = false;

		Common::String _text;
		int _value = 0;

		int drawText(Common::String text, int x, int y, int w, uint32 fg = 0, uint32 bg = 0, Graphics::TextAlign align = Graphics::kTextAlignLeft, bool wordWrap = false, int deltax = 0) const;
		void drawBitmap(Common::Rect r, const uint16 *bitmap, uint32 color) const;

	public:
		MacWidget(MacGuiImpl::MacDialogWindow *window, Common::Rect bounds, Common::String text, bool enabled);
		virtual ~MacWidget() {};

		void setId(int id) { _id = id; }
		int getId() const { return _id; }

		void setType(MacWidgetType type) { _type = type; }
		MacWidgetType getType() { return _type; }

		// Visibility never changes after initialization, so it does
		// not trigger a redraw.
		void setVisible(bool visible) { _visible = visible; }

		virtual void getFocus() { setRedraw(); }
		virtual void loseFocus() { setRedraw(); }

		virtual void setRedraw(bool fullRedraw = false);

		void setEnabled(bool enabled);

		virtual void setValue(int value);
		int getValue() const { return _value; }

		Common::String getText() const;

		virtual bool useBeamCursor() { return false; }
		virtual bool findWidget(int x, int y) const;
		virtual bool shouldDeferAction() { return false; }

		virtual void draw(bool drawFocused = false) = 0;

		virtual void handleMouseDown(Common::Event &event) {}
		virtual bool handleDoubleClick(Common::Event &event) { return false; }
		virtual bool handleMouseUp(Common::Event &event) { return false; }
		virtual void handleMouseMove(Common::Event &event) {}
		virtual void handleMouseHeld() {}
		virtual void handleWheelUp() {}
		virtual void handleWheelDown() {}
		virtual bool handleKeyDown(Common::Event &event) { return false; }
	};

	class MacButton : public MacWidget {
	private:
		struct CornerLine {
			int start;
			int length;
		};

		void hLine(int x0, int y0, int x1, bool enabled);
		void vLine(int x0, int y0, int y1, bool enabled);
		void drawCorners(Common::Rect r, CornerLine *corner, bool enabled);

	public:
		MacButton(MacGuiImpl::MacDialogWindow *window, Common::Rect bounds, Common::String text, bool enabled) : MacWidget(window, bounds, text, enabled) {}

		void draw(bool drawFocused = false);

		bool handleMouseUp(Common::Event &event) { return true; }
	};

	class MacCheckbox : public MacWidget {
	private:
		Common::Rect _hitBounds;

	public:
		MacCheckbox(MacGuiImpl::MacDialogWindow *window, Common::Rect bounds, Common::String text, bool enabled);

		bool findWidget(int x, int y) const;
		void draw(bool drawFocused = false);
		bool handleMouseUp(Common::Event &event);
	};

	// The dialogs add texts as disabled, but we don't want it to be drawn
	// as disabled so we enable it and make it "disabled" by giving it a
	// custom findWidget().

	class MacStaticText : public MacWidget {
	private:
		uint32 _fg;
		uint32 _bg;
		Graphics::TextAlign _alignment = Graphics::kTextAlignLeft;
		bool _wordWrap = false;

	public:
		MacStaticText(
			MacGuiImpl::MacDialogWindow *window,
			Common::Rect bounds, Common::String text,
			bool enabled, Graphics::TextAlign alignment = Graphics::kTextAlignLeft) : MacWidget(window, bounds, text, true) {
			_alignment = alignment;
			_fg = _black;
			_bg = _white;
		}

		void getFocus() {}
		void loseFocus() {}

		void setWordWrap(bool wordWrap) { _wordWrap = wordWrap; }

		void setText(Common::String text) {
			if (text != _text) {
				_text = text;
				setRedraw();
			}
		}

		void setColor(uint32 fg, uint32 bg) {
			if (fg != _fg || bg != _bg) {
				_fg = fg;
				_bg = bg;
				setRedraw();
			}
		}

		void draw(bool drawFocused = false);
	};

	class MacEditText : public MacWidget {
	private:
		// Max length of a SCUMM saved game name. We could make this
		// configurable later, if needed.
		uint _maxLength = 31;

		int _textPos = 1;
		int _selectLen = 0;
		int _caretPos = 0;
		int _caretX = -1;

		uint32 _nextCaretBlink = 0;
		bool _caretVisible = true;

		const Graphics::Font *_font;
		Graphics::Surface _textSurface;

		int getTextPosFromMouse(int x, int y);

		void updateSelection(int x, int y);
		void deleteSelection();

	public:
		MacEditText(MacGuiImpl::MacDialogWindow *window, Common::Rect bounds, Common::String text, bool enabled);

		void getFocus() override {}
		void loseFocus() override {}

		void selectAll();

		bool useBeamCursor() override { return true; }
		bool findWidget(int x, int y) const override;
		bool shouldDeferAction() override { return true; }

		void draw(bool drawFocused = false) override;

		void handleMouseDown(Common::Event &event) override;
		bool handleDoubleClick(Common::Event &event) override;
		bool handleKeyDown(Common::Event &event) override;
		void handleMouseHeld() override;
		void handleMouseMove(Common::Event &event) override;
	};

	class MacPicture : public MacWidget {
	private:
		Graphics::Surface *_picture = nullptr;

	public:
		MacPicture(MacGuiImpl::MacDialogWindow *window, Common::Rect bounds, int id, bool enabled);
		~MacPicture();

		Graphics::Surface *getPicture() const { return _picture; }

		void draw(bool drawFocused = false);
	};

	class MacSliderBase : public MacWidget {
	protected:
		int _minValue;
		int _maxValue;
		int _minPos;
		int _maxPos;
		int _handlePos = -1;
		int _grabOffset = -1;

		int calculateValueFromPos() const;
		int calculatePosFromValue() const;

	public:
		MacSliderBase(MacGuiImpl::MacDialogWindow *window, Common::Rect bounds, int minValue, int maxValue, int minPos, int maxPos, bool enabled)
			: MacWidget(window, bounds, "SliderBase", enabled),
			_minValue(minValue), _maxValue(maxValue),
			_minPos(minPos), _maxPos(maxPos) {}

		void getFocus() {}
		void loseFocus() {}

		void setValue(int value);
	};

	class MacSlider : public MacSliderBase {
	private:
		Common::Point _clickPos;
		uint32 _nextRepeat = 0;

		int _pageSize = 0;
		int _paging = 0;

		bool _upArrowPressed = false;
		bool _downArrowPressed = false;

		Common::Rect _boundsButtonUp;
		Common::Rect _boundsButtonDown;
		Common::Rect _boundsBody;

		Common::Rect getHandleRect(int value);

		void fill(Common::Rect r, bool inverted = false);

		void drawUpArrow(bool markAsDirty);
		void drawDownArrow(bool markAsDirty);
		void drawArrow(Common::Rect r, const uint16 *bitmap, bool markAsDirty);

		void eraseDragHandle();
		void drawHandle(Common::Rect r);

	public:
		MacSlider(MacGuiImpl::MacDialogWindow *window, Common::Rect bounds, int minValue, int maxValue, int pageSize, bool enabled);

		bool isScrollable() const { return (_maxValue - _minValue) > 0; }
		int getPageSize() const { return _pageSize; }

		bool findWidget(int x, int y) const;
		void draw(bool drawFocued = false);
		void redrawHandle(int oldValue, int newValue);

		void handleMouseDown(Common::Event &event);
		bool handleMouseUp(Common::Event &event);
		void handleMouseMove(Common::Event &event);
		void handleMouseHeld();
		void handleWheelUp();
		void handleWheelDown();
	};

	class MacPictureSlider : public MacSliderBase {
	private:
		MacPicture *_background;
		MacPicture *_handle;
		int _minX;
		int _maxX;

		void eraseHandle();
		void drawHandle();

	public:
		 MacPictureSlider(MacGuiImpl::MacDialogWindow *window, MacPicture *background, MacPicture *handle, bool enabled, int minX, int maxX, int minValue, int maxValue, int leftMargin, int rightMargin)
			: MacSliderBase(window, background->getBounds(), minValue, maxValue, minX + leftMargin, maxX - rightMargin, enabled),
			_background(background), _handle(handle), _minX(minX),
			_maxX(maxX) {}

		bool findWidget(int x, int y) const;
		void draw(bool drawFocused = false);

		void handleMouseDown(Common::Event &event);
		bool handleMouseUp(Common::Event &event);
		void handleMouseMove(Common::Event &event);
		void handleWheelUp();
		void handleWheelDown();
	};

	class MacListBox : public MacWidget {
	private:
		Common::StringArray _texts;
		Common::Array<MacStaticText *> _textWidgets;
		MacSlider *_slider;
		bool _sliderFocused = false;

		void updateTexts();
		void handleWheel(int distance);

	public:
		MacListBox(MacGuiImpl::MacDialogWindow *window, Common::Rect bounds, Common::StringArray texts, bool enabled, bool contentUntouchable = true);
		~MacListBox();

		void getFocus() {}
		void loseFocus() {}

		void setValue(int value) {
			if (value != _value) {
				_value = value;
				updateTexts();
			}
		}

		int getValue() {
			return _value;
		}

		bool findWidget(int x, int y) const;
		void setRedraw(bool fullRedraw = false);
		void draw(bool drawFocused = false);

		void handleMouseDown(Common::Event &event);
		bool handleDoubleClick(Common::Event &event);
		bool handleMouseUp(Common::Event &event);
		void handleMouseMove(Common::Event &event);
		void handleMouseHeld();
		void handleWheelUp();
		void handleWheelDown();
		bool handleKeyDown(Common::Event &event);
	};

	class MacDialogWindow {
	private:
		uint32 _black;
		uint32 _white;

		bool _shakeWasEnabled;

		Common::Rect _bounds;
		int _margin;

		bool _visible = false;

		uint32 _lastClickTime = 0;
		Common::Point _lastClickPos;

		Graphics::Surface *_beamCursor = nullptr;
		Common::Point _beamCursorPos;
		bool _cursorWasVisible = false;
		bool _beamCursorVisible = false;
		const int _beamCursorHotspotX = 3;
		const int _beamCursorHotspotY = 4;

		void drawBeamCursor();
		void undrawBeamCursor();

		PauseToken _pauseToken;

		Graphics::Surface *_from = nullptr;
		Graphics::Surface *_backup = nullptr;
		Graphics::Surface _surface;
		Graphics::Surface _innerSurface;

		Common::Array<MacWidget *> _widgets;

		MacWidget *_defaultWidget = nullptr;

		MacWidget *_focusedWidget = nullptr;
		Common::Point _focusClick;
		Common::Point _oldMousePos;
		Common::Point _mousePos;
		Common::Point _realMousePos;

		Common::StringArray _substitutions;
		Common::Array<Common::Rect> _dirtyRects;

		void copyToScreen(Graphics::Surface *s = nullptr) const;

		void addWidget(MacWidget *widget, MacWidgetType type);

	public:
		OSystem *_system;
		MacGuiImpl *_gui;

		MacDialogWindow(MacGuiImpl *gui, OSystem *system, Graphics::Surface *from, Common::Rect bounds, MacDialogWindowStyle windowStyle = kWindowStyleNormal, MacDialogMenuStyle menuStyle = kMenuStyleDisabled);
		~MacDialogWindow();

		Graphics::Surface *surface() { return &_surface; }
		Graphics::Surface *innerSurface() { return &_innerSurface; }

		bool isVisible() const { return _visible; }

		void show();
		int runDialog(Common::Array<int> &deferredActionIds);
		void updateCursor();

		MacWidget *getWidget(MacWidgetType type, int nr = 0) const;

		void setDefaultWidget(MacWidget *widget) { _defaultWidget = widget; }
		MacWidget *getDefaultWidget() const { return _defaultWidget; }

		void setFocusedWidget(int x, int y);
		void clearFocusedWidget();
		MacWidget *getFocusedWidget() const { return _focusedWidget; }
		Common::Point getFocusClick() const { return _focusClick; }
		Common::Point getMousePos() const { return _mousePos; }

		int findWidget(int x, int y) const;

		MacGuiImpl::MacButton *addButton(Common::Rect bounds, Common::String text, bool enabled);
		MacGuiImpl::MacCheckbox *addCheckbox(Common::Rect bounds, Common::String text, bool enabled);
		MacGuiImpl::MacStaticText *addStaticText(Common::Rect bounds, Common::String text, bool enabled, Graphics::TextAlign alignment = Graphics::kTextAlignLeft);
		MacGuiImpl::MacEditText *addEditText(Common::Rect bounds, Common::String text, bool enabled);
		MacGuiImpl::MacPicture *addPicture(Common::Rect bounds, int id, bool enabled);
		MacGuiImpl::MacSlider *addSlider(int x, int y, int h, int minValue, int maxValue, int pageSize, bool enabled);
		MacGuiImpl::MacPictureSlider *addPictureSlider(int backgroundId, int handleId, bool enabled, int minX, int maxX, int minValue, int maxValue, int leftMargin = 0, int rightMargin = 0);
		MacGuiImpl::MacListBox *addListBox(Common::Rect bounds, Common::StringArray texts, bool enabled, bool contentUntouchable = false);

		void addSubstitution(Common::String text) { _substitutions.push_back(text); }
		void replaceSubstitution(int nr, Common::String text) { _substitutions[nr] = text; }

		bool hasSubstitution(uint n) const { return n < _substitutions.size(); }
		Common::String &getSubstitution(uint n) { return _substitutions[n]; }

		void markRectAsDirty(Common::Rect r);
		void update(bool fullRedraw = false);

		static void plotPixel(int x, int y, int color, void *data);
		static void plotPattern(int x, int y, int pattern, void *data);
		static void plotPatternDarkenOnly(int x, int y, int pattern, void *data);

		void drawDottedHLine(int x0, int y, int x1);
		void fillPattern(Common::Rect r, uint16 pattern, bool fillBlack = true, bool fillWhite = true);
		void drawSprite(const Graphics::Surface *sprite, int x, int y);
		void drawSprite(const Graphics::Surface *sprite, int x, int y, Common::Rect clipRect);
		void drawTexts(Common::Rect r, const TextLine *lines, bool inverse = false);
		void drawTextBox(Common::Rect r, const TextLine *lines, bool inverse = false, int arc = 9);
	};

	MacGuiImpl(ScummEngine *vm, const Common::Path &resourceFile);
	virtual ~MacGuiImpl();

	Graphics::MacWindowManager *_windowManager = nullptr;
	bool _forceMenuClosed = false;

	virtual int getNumColors() const = 0;

	Graphics::Surface *surface() { return _surface; }
	uint32 getBlack() const;
	uint32 getWhite() const;

	virtual const Common::String name() const = 0;

	Common::String readCString(uint8 *&data);
	Common::String readPascalString(uint8 *&data);

	int toMacRoman(int unicode) const;

	void setPaletteDirty();
	void updatePalette();

	virtual bool handleEvent(Common::Event event);

	static void menuCallback(int id, Common::String &name, void *data);
	virtual bool initialize();
	void updateWindowManager();

	const Graphics::Font *getFont(FontId fontId);
	virtual const Graphics::Font *getFontByScummId(int32 id) = 0;

	Graphics::Surface *loadPict(int id);

	virtual bool isVerbGuiActive() const { return false; }
	virtual void reset() {}
	virtual void resetAfterLoad() = 0;
	virtual void update(int delta) = 0;

	bool runQuitDialog();
	bool runRestartDialog();

	virtual void setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate) = 0;

	virtual Graphics::Surface *textArea() { return nullptr; }
	virtual void clearTextArea() {}
	virtual void initTextAreaForActor(Actor *a, byte color) {}
	virtual void printCharToTextArea(int chr, int x, int y, int color) {}

	MacDialogWindow *createWindow(Common::Rect bounds, MacDialogWindowStyle style = kWindowStyleNormal, MacDialogMenuStyle menuStyle = kMenuStyleDisabled);
	MacDialogWindow *createDialog(int dialogId);
	void drawBanner(char *message);
	void undrawBanner();

	void drawBitmap(Graphics::Surface *s, Common::Rect r, const uint16 *bitmap, uint32 color) const;
};

} // End of namespace Scumm
#endif
