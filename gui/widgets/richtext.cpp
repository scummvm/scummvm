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

#include "common/system.h"
#include "common/unicode-bidi.h"
#include "common/translation.h"

#include "graphics/macgui/mactext.h"
#include "graphics/macgui/macfontmanager.h"

#include "gui/gui-manager.h"

#include "gui/ThemeEngine.h"
#include "gui/ThemeEval.h"

#include "gui/widgets/richtext.h"
#include "gui/widgets/scrollbar.h"
#include "gui/animation/FluidScroll.h"

namespace GUI {

const Graphics::TTFMap ttfFamily[] = {
	{"NotoSans-Regular.ttf", Graphics::kMacFontRegular},
	{"NotoSans-Bold.ttf", Graphics::kMacFontBold},
	{"NotoSerif-Italic.ttf", Graphics::kMacFontItalic},
	{"NotoSerif-Bold-Italic.ttf", Graphics::kMacFontBold | Graphics::kMacFontItalic},
	{nullptr, 0}
};

const Graphics::TTFMap ttfFamilyJapanese[] = {
	{"NotoSansJP-Regular.otf", Graphics::kMacFontRegular},
	{"NotoSansJP-Bold.otf", Graphics::kMacFontBold},
	{nullptr, 0}
};

const Graphics::TTFMap ttfFamilyKorean[] = {
	{"NotoSansKR-Regular.otf", Graphics::kMacFontRegular},
	{"NotoSansKR-Bold.otf", Graphics::kMacFontBold},
	{nullptr, 0}
};

const Graphics::TTFMap ttfFamilySimplifiedChinese[] = {
	{"NotoSansSC-Regular.otf", Graphics::kMacFontRegular},
	{"NotoSansSC-Bold.otf", Graphics::kMacFontBold},
	{nullptr, 0}
};

const Graphics::TTFMap ttfFamilyTraditionalChinese[] = {
	{"NotoSansTC-Regular.otf", Graphics::kMacFontRegular},
	{"NotoSansTC-Bold.otf", Graphics::kMacFontBold},
	{nullptr, 0}
};

static const Graphics::TTFMap *getTTFFamily() {
#ifdef USE_TRANSLATION
	const Common::String language = TransMan.getCurrentLanguage();
	if (language == "ja")
		return ttfFamilyJapanese;
	if (language == "ko")
		return ttfFamilyKorean;
	if (language == "zh_Hans")
		return ttfFamilySimplifiedChinese;
	if (language == "zh_Hant" || language == "zh")
		return ttfFamilyTraditionalChinese;
#endif

	return ttfFamily;
}

RichTextWidget::RichTextWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &text, const Common::U32String &tooltip)
	: Widget(boss, x, y, w, h, scale, tooltip), CommandSender(nullptr)  {

	_text = text;

	init();
}

RichTextWidget::RichTextWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &text, const Common::U32String &tooltip)
	: RichTextWidget(boss, x, y, w, h, false, text, tooltip) {
}

RichTextWidget::RichTextWidget(GuiObject *boss, const Common::String &name, const Common::U32String &text, const Common::U32String &tooltip)
	: Widget(boss, name, tooltip), CommandSender(nullptr)  {

	_text = text;

	init();
}

void RichTextWidget::init() {
	// WIDGET_HOOK_DRAG: prevent hooking by a container as we need drag for ourselves
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_TRACK_MOUSE | WIDGET_DYN_TOOLTIP | WIDGET_HOOK_DRAG);

	_type = kRichTextWidget;

	_verticalScroll = new ScrollBarWidget(this, _w - 16, 0, 16, _h);
	_verticalScroll->setTarget(this);
	_scrolledX = 0;
	_scrolledY = 0;

	_innerMargin = g_gui.xmlEval()->getVar("Globals.RichTextWidget.InnerMargin", 0);
	_scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);

	_textWidth = MAX(1, _w - _scrollbarWidth - 2 * _innerMargin);
	_textHeight = MAX(1, _h - 2 * _innerMargin);

	_limitH = 140;

	_scrollPos = 0.0f;
	_fluidScroller = new FluidScroller();
	_isDragging = false;
}


RichTextWidget::~RichTextWidget() {
	delete _txtWnd;

	if (_surface)
		_surface->free();
	delete _surface;

	if (_cachedTextSurface)
		_cachedTextSurface->free();
	delete _cachedTextSurface;

	unregisterTickleWidget(this);
	delete _fluidScroller;
}

void RichTextWidget::handleMouseWheel(int x, int y, int direction) {
	if (!_verticalScroll->isVisible())
		return;
	_fluidScroller->handleMouseWheel(direction);
	registerTickleWidget(this);
}

void RichTextWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	_mouseDownY = _mouseDownStartY = y;
	_fluidScroller->stopAnimation();
}

void RichTextWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	_mouseDownY = _mouseDownStartY = 0;
	if (_isDragging) {
		_fluidScroller->startFling();
		registerTickleWidget(this);
		_isDragging = false;
		return;
	}
	_isDragging = false;

	if (!_txtWnd)
		return;

	Common::String link = _txtWnd->getMouseLink(x - _innerMargin + _scrolledX, y - _innerMargin + _scrolledY).encode();

	if (link.hasPrefixIgnoreCase("http"))
		g_system->openUrl(link);
}

void RichTextWidget::cancelDrag() {
	if (_isDragging) {
		_fluidScroller->stopAnimation();
	}
	_mouseDownY = _mouseDownStartY = 0;
	_isDragging = false;
}

void RichTextWidget::handleMouseMoved(int x, int y, int button) {
	if (_txtWnd) {
		Common::String link = _txtWnd->getMouseLink(x - _innerMargin + _scrolledX, y - _innerMargin + _scrolledY).encode();

		if (!link.empty() && link.hasPrefixIgnoreCase("http"))
			g_gui.theme()->setActiveCursor(GUI::ThemeEngine::kCursorIndex);
		else
			g_gui.theme()->setActiveCursor(GUI::ThemeEngine::kCursorNormal);

		// Read a link once when the mouse moves onto it
		const Common::Point pos(x - _innerMargin + _scrolledX, y - _innerMargin + _scrolledY);
		int hovered = -1;
		for (uint i = 0; i < _links.size(); ++i) {
			if (_links[i].bounds.contains(pos)) {
				hovered = i;
				break;
			}
		}
		if (hovered != _hoveredLink) {
			_hoveredLink = hovered;
			if (hovered >= 0)
				readLink(hovered);
		}
	}

	if (_mouseDownStartY == 0 || _mouseDownY == y || !_txtWnd || !_verticalScroll->isVisible())
		return;

	int deltaY = _mouseDownY - y;

	if (!_isDragging && ABS(deltaY) > 5)
		_isDragging = true;

	if (_isDragging) {
		_mouseDownY = y;
		if (deltaY != 0) {
			_fluidScroller->feedDrag(g_system->getMillis(), deltaY);
			applyScrollPos();
		}
	}
}

Common::U32String RichTextWidget::getSpokenDescription() const {
	// Arriving on a link with Shift+Tab describes the link
	if (_focusedLink >= 0 && _focusedLink < (int)_links.size()) {
		// I18N: Spoken by text-to-speech for the kind of control a link is
		return _links[_focusedLink].text + Common::U32String(", ") + _c("link", "text-to-speech");
	}

	Common::U32String text;
	if (_txtWnd)
		text = _txtWnd->getPlainText();
	// I18N: Spoken by text-to-speech for the kind of control a block of text is
	return composeSpokenDescription(findSpokenLabel(), _c("text", "text-to-speech"), text);
}

bool RichTextWidget::wantsKeyboardFocus() {
	return _verticalScroll->isVisible() || !_links.empty();
}

void RichTextWidget::enterFromKeyboard(int direction) {
	// The stops are the text, then each link. Shift+Tab starts from the end.
	if (direction < 0 && !_links.empty())
		focusLink(_links.size() - 1);
	else
		focusLink(-1);
}

void RichTextWidget::lostFocusWidget() {
	_focusedLink = -1;
	markAsDirty();
}

bool RichTextWidget::handleKeyDown(Common::KeyState state) {
	if (!isEnabled() || !_txtWnd)
		return false;

	const bool modified = state.flags & (Common::KBD_CTRL | Common::KBD_ALT | Common::KBD_META);

	// Past either end, Tab is left to the dialog to move to the next widget
	if (state.keycode == Common::KEYCODE_TAB && !modified) {
		const int next = _focusedLink + (state.hasFlags(Common::KBD_SHIFT) ? -1 : 1);
		if (next < -1 || next >= (int)_links.size())
			return false;
		focusLink(next);
		return true;
	}

	if (_focusedLink >= 0 && !modified &&
			(state.keycode == Common::KEYCODE_RETURN || state.keycode == Common::KEYCODE_KP_ENTER ||
			 state.keycode == Common::KEYCODE_SPACE)) {
		const Common::String link = _links[_focusedLink].link.encode();
		if (link.hasPrefixIgnoreCase("http"))
			g_system->openUrl(link);
		return true;
	}

	if (!_verticalScroll->isVisible())
		return false;

	int position;
	if (!_verticalScroll->scrollPositionForKey(state, position))
		return false;

	scrollTo(position);
	return true;
}

void RichTextWidget::handleMouseLeft(int button) {
	_hoveredLink = -1;
}

void RichTextWidget::updateLinks() {
	_links.clear();
	_focusedLink = -1;
	_hoveredLink = -1;
	if (!_txtWnd)
		return;

	const Common::Array<Graphics::MacTextLink> links = _txtWnd->getLinks();
	for (uint i = 0; i < links.size(); ++i) {
		Link link;
		link.link = links[i].link;
		link.text = links[i].text;
		link.bounds = links[i].bounds;
		_links.push_back(link);
	}
}

void RichTextWidget::focusLink(int index) {
	_focusedLink = index;
	if (index >= 0) {
		scrollToLink(index);
		// Arriving on the widget is announced with the focus itself
		if (_hasFocus)
			readLink(index);
	}
	markAsDirty();
}

void RichTextWidget::readLink(int index) {
	// I18N: Spoken by text-to-speech for the kind of control a link is
	read(_links[index].text + Common::U32String(", ") + _c("link", "text-to-speech"));
}

void RichTextWidget::scrollToLink(int index) {
	if (!_verticalScroll->isVisible())
		return;

	const Common::Rect &bounds = _links[index].bounds;
	int position = _scrolledY;
	if (bounds.top < position)
		position = bounds.top;
	else if (bounds.bottom > position + _textHeight)
		position = bounds.bottom - _textHeight;

	position = CLIP(position, 0, MAX(0, _verticalScroll->_numEntries - _verticalScroll->_entriesPerPage));
	if (position != _scrolledY)
		scrollTo(position);
}

void RichTextWidget::scrollTo(int position) {
	_fluidScroller->stopAnimation();
	_fluidScroller->setPosition((float)position, false);
	applyScrollPos();
}

void RichTextWidget::handleTickle() {
	if (_fluidScroller->update(g_system->getMillis(), _scrollPos)) {
		applyScrollPos();
	} else {
		unregisterTickleWidget(this);
	}
}

void RichTextWidget::cancelTickle() {
	_fluidScroller->stopAnimation();
}

void RichTextWidget::applyScrollPos() {
	_scrollPos = _fluidScroller->getVisualPosition();
	_scrolledY = (int)_scrollPos;
	int maxScroll = MAX(0, _txtWnd->getTextHeight() - _limitH);
	_verticalScroll->_currentPos = CLIP((int)_scrolledY, 0, (int)maxScroll);
	_verticalScroll->recalc();
	markAsDirty();
}

void RichTextWidget::handleTooltipUpdate(int x, int y) {
	if (!_txtWnd)
		return;

	_tooltip = _txtWnd->getMouseLink(x - _innerMargin + _scrolledX, y - _innerMargin + _scrolledY);
}

void RichTextWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	Widget::handleCommand(sender, cmd, data);
	switch (cmd) {
	case kSetPositionCmd:
		_scrolledY = _verticalScroll->_currentPos;
		_scrollPos = _scrolledY;
		_fluidScroller->stopAnimation();
		_scrollPos = _fluidScroller->setPosition(_scrollPos, false);
		reflowLayout();
		g_gui.scheduleTopDialogRedraw();
		break;
	default:
		break;
	}
}

void RichTextWidget::recalc() {
	_innerMargin = g_gui.xmlEval()->getVar("Globals.RichTextWidget.InnerMargin", 0);
	_scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);
	_textWidth = MAX(1, _w - _scrollbarWidth - 2 * _innerMargin);
	_textHeight = MAX(1, _h - 2 * _innerMargin);
	_limitH = _textHeight;

	// Workaround: Currently Graphics::MacText::setMaxWidth does not work well.
	// There is a known limitation that the size is skipped when the text contains table,
	// and there is also an issue with the font.
	// So for now we recreate the widget.
//	if (_txtWnd) {
//		_txtWnd->setMaxWidth(_textWidth);
//		if (_surface->w != _textWidth || _surface->h != _textHeight)
//			_surface->create(_textWidth, _textHeight, g_gui.getWM()->_pixelformat);
//	} else {
//		createWidget();
//	}

	if (!_surface || _surface->w != _textWidth) {
		if (_surface) {
			_surface->free();
			delete _surface;
			_surface = nullptr;
		}
		delete _txtWnd;
		_txtWnd = nullptr;
		if (_cachedTextSurface) {
			_cachedTextSurface->free();
			delete _cachedTextSurface;
			_cachedTextSurface = nullptr;
		}
	} else if (_surface->h != _textHeight) {
		_surface->create(_textWidth, _textHeight, g_gui.getWM()->_pixelformat);

		int h = _txtWnd->getTextHeight();
		int maxScroll = MAX(0, h - _limitH);
		_verticalScroll->_numEntries = h;
		_verticalScroll->_currentPos = CLIP((int)_scrolledY, 0, (int)maxScroll);
		_verticalScroll->_entriesPerPage = _limitH;
		_verticalScroll->_singleStep = _h / 4;
		_verticalScroll->setPos(_w - _scrollbarWidth, 0);
		_verticalScroll->setSize(_scrollbarWidth, _h - 1);
		_verticalScroll->setVisible(_verticalScroll->_numEntries > _limitH); //show when there is something to scroll
		_verticalScroll->recalc();
		_fluidScroller->setBounds((float)maxScroll, (float)_limitH, (float)_verticalScroll->_singleStep);
	}
}

void RichTextWidget::createWidget() {
	Graphics::MacWindowManager *wm = g_gui.getWM();

	uint8 bgR, bgG, bgB;
	uint32 bg;
	if (g_gui.theme()->getDrawDataColor(kDDWidgetBackgroundDefault, bgR, bgG, bgB))
		bg = wm->_pixelformat.ARGBToColor(255, bgR, bgG, bgB);
	else
		bg = wm->_pixelformat.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF);

	TextColorData *normal = g_gui.theme()->getTextColorData(kTextColorNormal);
	uint32 fg = wm->_pixelformat.RGBToColor(normal->r, normal->g, normal->b);

	const int fontHeight = g_gui.xmlEval()->getVar("Globals.Font.Height", 25);

	int newId;
	bool useTTF = false; // For English we use MacFONTs

#ifdef USE_TRANSLATION
	// MacFONTs do not contain diacritic marks or non-English characters, so we have to use TTF instead
	if (TransMan.getCurrentLanguage() != "en")
		useTTF = true;
#endif

	if (useTTF)
		newId = wm->_fontMan->registerTTFFont(getTTFFamily());
	else
		newId = Graphics::kMacFontNewYork;

#ifdef USE_FREETYPE2
		// Use anti-aliased (light) rendering for the GUI help text.
		Graphics::TTFRenderMode prevRenderMode = wm->_fontMan->getTTFRenderMode();
		wm->_fontMan->setTTFRenderMode(Graphics::kTTFRenderModeLight);
#endif

	Graphics::MacFont macFont(newId, fontHeight, Graphics::kMacFontRegular);
	_txtWnd = new Graphics::MacText(Common::U32String(), wm, &macFont, fg, bg, _textWidth, Graphics::kTextAlignLeft);

	if (!_imageArchive.empty())
		_txtWnd->setImageArchive(_imageArchive);

	_txtWnd->setMarkdownText(_text);
	updateLinks();

#ifdef USE_FREETYPE2
	// Restore previous render mode now that all fonts have been loaded
	wm->_fontMan->setTTFRenderMode(prevRenderMode);
#endif

	int textHeight = _txtWnd->getTextHeight();

	if (textHeight > 0) {
		if (!_cachedTextSurface || _cachedTextSurface->w != _textWidth || _cachedTextSurface->h != textHeight) {
			if (_cachedTextSurface) {
				_cachedTextSurface->free();
				delete _cachedTextSurface;
			}
			_cachedTextSurface = new Graphics::ManagedSurface(_textWidth, textHeight, wm->_pixelformat);
		}
		_cachedTextSurface->clear(bg);
		_txtWnd->draw(_cachedTextSurface, 0, 0, _textWidth, textHeight, 0, 0);
	}

	if (!_surface || _surface->w != _textWidth || _surface->h != _textHeight) {
    	if (_surface)
        	_surface->create(_textWidth, _textHeight, wm->_pixelformat);
    	else
        	_surface = new Graphics::ManagedSurface(_textWidth, _textHeight, wm->_pixelformat);
	}

	int h = _txtWnd->getTextHeight();
	int maxScroll = MAX(0, h - _limitH);
	_verticalScroll->_numEntries = h;
	_verticalScroll->_currentPos = CLIP((int)_scrolledY, 0, (int)maxScroll);
	_verticalScroll->_entriesPerPage = _limitH;
	_verticalScroll->_singleStep = _h / 4;
	_verticalScroll->setPos(_w - _scrollbarWidth, 0);
	_verticalScroll->setSize(_scrollbarWidth, _h - 1);
	_verticalScroll->setVisible(_verticalScroll->_numEntries > _limitH); //show when there is something to scroll
	_verticalScroll->recalc();
	_fluidScroller->setBounds((float)maxScroll, (float)_limitH, (float)_verticalScroll->_singleStep);
}

void RichTextWidget::reflowLayout() {
	Widget::reflowLayout();

	recalc();
}

void RichTextWidget::ensureWidget() {
	if (_txtWnd)
		return;
	createWidget();
}

void RichTextWidget::drawWidget() {
	ensureWidget();

	if (!_txtWnd)
		recalc();

	// Show the keyboard focus with a frame, as around a list
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + _w, _y + _h),
	                                    _hasFocus ? ThemeEngine::kWidgetBackgroundBorder : ThemeEngine::kWidgetBackgroundPlain);

	uint8 bgR, bgG, bgB;
	uint32 bg;
	if (g_gui.theme()->getDrawDataColor(kDDWidgetBackgroundDefault, bgR, bgG, bgB))
		bg = g_gui.getWM()->_pixelformat.ARGBToColor(255, bgR, bgG, bgB);
	else
		bg = g_gui.getWM()->_pixelformat.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF);

	_surface->clear(bg);

	if (_cachedTextSurface) {
		int cachedHeight = _cachedTextSurface->h;
		int srcY = _scrolledY < 0 ? 0 : (int)_scrolledY;
		int destY = _scrolledY < 0 ? -(int)_scrolledY : 0;

		if (srcY < cachedHeight)
			_surface->simpleBlitFrom(*_cachedTextSurface, Common::Rect(0, srcY, _textWidth,
			MIN(srcY + _textHeight - destY, cachedHeight)), Common::Point(0, destY));
	} else
		_txtWnd->draw(_surface, 0, _scrolledY, _textWidth, _textHeight, 0, 0);

	// Frame the focused link in the colour of the focused selection
	if (_hasFocus && _focusedLink >= 0 && _focusedLink < (int)_links.size()) {
		Common::Rect frame = _links[_focusedLink].bounds;
		frame.translate(0, -_scrolledY);
		frame.grow(1);
		frame.clip(Common::Rect(_surface->w, _surface->h));

		if (!frame.isEmpty()) {
			uint8 r, g, b;
			if (!g_gui.theme()->getDrawDataColor(kDDTextSelectionFocusBackground, r, g, b)) {
				TextColorData *normal = g_gui.theme()->getTextColorData(kTextColorNormal);
				r = normal->r;
				g = normal->g;
				b = normal->b;
			}
			_surface->frameRect(frame, g_gui.getWM()->_pixelformat.RGBToColor(r, g, b));
		}
	}

	g_gui.theme()->drawManagedSurface(Common::Point(_x + _innerMargin, _y + _innerMargin), *_surface, Graphics::ALPHA_OPAQUE);
}

void RichTextWidget::draw() {
	Widget::draw();

	if (_verticalScroll->isVisible()) {
		_verticalScroll->draw();
	}
}

void RichTextWidget::markAsDirty() {
	Widget::markAsDirty();

	if (_verticalScroll->isVisible()) {
		_verticalScroll->markAsDirty();
	}
}

bool RichTextWidget::containsWidget(Widget *w) const {
	if (w == _verticalScroll || _verticalScroll->containsWidget(w))
		return true;
	return false;
}

Widget *RichTextWidget::findWidget(int x, int y) {
	if (_verticalScroll->isVisible() && x >= _w - _scrollbarWidth)
		return _verticalScroll;

	return this;
}

} // End of namespace GUI
