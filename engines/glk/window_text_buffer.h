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

#ifndef GLK_WINDOW_TEXT_BUFFER_H
#define GLK_WINDOW_TEXT_BUFFER_H

#include "glk/windows.h"
#include "glk/picture.h"
#include "glk/speech.h"
#include "glk/conf.h"
#include "common/array.h"
#include "common/ustr.h"

namespace Glk {

/**
 * Text Buffer window
 */
class TextBufferWindow : public TextWindow, Speech {
	/**
	 * Structure for a row within the window
	 */
	struct TextBufferRow {
		uint32 _chars[TBLINELEN];
		Attributes _attrs[TBLINELEN];
		int _len, _newLine;
		bool _dirty, _repaint;
		Picture *_lPic, *_rPic;
		uint _lHyper, _rHyper;
		int _lm, _rm;

		/**
		 * Constructor
		 */
		TextBufferRow();
	};
	typedef Common::Array<TextBufferRow> TextBufferRows;
private:
	PropFontInfo &_font;
private:
	void reflow();
	void touchScroll();
	bool putPicture(Picture *pic, uint align, uint linkval);

	/**
	 * @remarks Only for input text
	 */
	void putText(const char *buf, int len, int pos, int oldlen);

	/**
	 * @remarks Only for input text
	 */
	void putTextUni(const uint32 *buf, int len, int pos, int oldlen);

	/**
	 * Return or enter, during line input. Ends line input.
	 */
	void acceptLine(uint32 keycode);

	/**
	 * Return true if a following quotation mark should be an opening mark,
	 * false if it should be a closing mark. Opening quotation marks will
	 * appear following an open parenthesis, open square bracket, or
	 * whitespace.
	 */
	bool leftquote(uint32 c);

	/**
	 * Mark a given text row as modified
	 */
	void touch(int line);

	void scrollOneLine(bool forced);
	void scrollResize();
	int calcWidth(const uint32 *chars, const Attributes *attrs, int startchar, int numchars, int spw);
public:
	int _width, _height;
	int _spaced;
	int _dashed;

	TextBufferRows _lines;
	int _scrollBack;

	int _numChars;        ///< number of chars in last line: lines[0]
	uint32 *_chars;       ///< alias to lines[0].chars
	Attributes *_attrs;   ///< alias to lines[0].attrs

	///< adjust margins temporarily for images
	int _ladjw;
	int _ladjn;
	int _radjw;
	int _radjn;

	// Command history.
	Common::Array<Common::U32String> _history;
	int _historyPos;
	int _historyFirst, _historyPresent;

	// for paging
	int _lastSeen;
	int _scrollPos;
	int _scrollMax;

	// for line input
	void *_inBuf;        ///< unsigned char* for latin1, uint* for unicode
	int _inMax;
	long _inFence;
	long _inCurs;
	Attributes _origAttr;
	gidispatch_rock_t _inArrayRock;

	uint _echoLineInput;
	uint *_lineTerminators;

	// style hints and settings
	WindowStyle _styles[style_NUMSTYLES];

	// for copy selection
	uint32 *_copyBuf;
	int _copyPos;
public:
	/**
	 * Constructor
	 */
	TextBufferWindow(Windows *windows, uint rock);

	/**
	 * Destructor
	 */
	~TextBufferWindow() override;

	int acceptScroll(uint arg);

	uint drawPicture(const Common::String &image, uint align, uint scaled, uint width, uint height);

	/**
	 * Get the font info structure associated with the window
	 */
	FontInfo *getFontInfo() override { return &_font; }

	/**
	 * Rearranges the window
	 */
	void rearrange(const Rect &box) override;

	/**
	 * Get window split size within parent pair window
	 */
	uint getSplit(uint size, bool vertical) const override;

	/**
	 * Write a unicode character
	 */
	void putCharUni(uint32 ch) override;

	/**
	 * Unput a unicode character
	 */
	bool unputCharUni(uint32 ch) override;

	/**
	 * Clear the window
	 */
	void clear() override;

	/**
	 * Click the window
	 */
	void click(const Point &newPos) override;

	/**
	 * Prepare for inputing a line
	 */
	void requestLineEvent(char *buf, uint maxlen, uint initlen) override;

	/**
	 * Prepare for inputing a line
	 */
	void requestLineEventUni(uint32 *buf, uint maxlen, uint initlen) override;

	/**
	 * Cancel an input line event
	 */
	void cancelLineEvent(Event *ev) override;

	/**
	 * Cancel a hyperlink event
	 */
	void cancelHyperlinkEvent() override {
		_hyperRequest = false;
	}

	/**
	 * Redraw the window
	 */
	void redraw() override;

	void acceptReadLine(uint32 arg) override;

	void acceptReadChar(uint arg) override;

	void getSize(uint *width, uint *height) const override;

	void requestCharEvent() override;

	void requestCharEventUni() override;

	void setEchoLineEvent(uint val) override {
		_echoLineInput = val != 0;
	}

	void requestHyperlinkEvent() override {
		_hyperRequest = true;
	}

	void cancelCharEvent() override {
		_charRequest = _charRequestUni = false;
	}

	void flowBreak() override;

	/**
	 * Returns a pointer to the styles for the window
	 */
	const WindowStyle *getStyles() const override {
		return _styles;
	}
};

} // End of namespace Glk

#endif
