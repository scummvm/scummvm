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

#ifndef GARGOYLE_WINDOW_TEXT_BUFFER_H
#define GARGOYLE_WINDOW_TEXT_BUFFER_H

#include "gargoyle/windows.h"
#include "gargoyle/picture.h"

namespace Gargoyle {

/**
 * Text Buffer window
 */
class TextBufferWindow : public Window {
	/**
	 * Structure for a row within the window
	 */
	struct TextBufferRow {
		Common::Array<uint32> chars;
		Common::Array<Attributes> attr;
		int len, newline;
		bool dirty, repaint;
		Picture *lpic, *rpic;
		glui32 lhyper, rhyper;
		int lm, rm;

		/**
		 * Constructor
		 */
		TextBufferRow();

		/**
		 * Resize the row
		 */
		void resize(size_t newSize);
	};
	typedef Common::Array<TextBufferRow> TextBufferRows;
private:
	void reflow();
	void touchScroll();
	bool putPicture(Picture *pic, glui32 align, glui32 linkval);
	void putTextUni(const glui32 *buf, int len, int pos, int oldlen);
	void flowBreak();

	/**
	 * Mark a given text row as modified
	 */
	void touch(int line);
public:
	int _width, _height;
	int _spaced;
	int _dashed;

	TextBufferRows _lines;
	int _scrollBack;

	int _numChars;        ///< number of chars in last line: lines[0]
	glui32 *_chars;       ///< alias to lines[0].chars
	Attributes *_attrs;  ///< alias to lines[0].attrs

    ///< adjust margins temporarily for images
	int _ladjw;
	int _ladjn;
	int _radjw;
	int _radjn;

	/* Command history. */
	glui32 *_history[HISTORYLEN];
	int _historyPos;
	int _historyFirst, _historyPresent;

	/* for paging */
	int _lastSeen;
	int _scrollPos;
	int _scrollMax;

	/* for line input */
	void *_inBuf;        ///< unsigned char* for latin1, glui32* for unicode
	int _inMax;
	long _inFence;
	long _inCurs;
	Attributes _origAttr;
	gidispatch_rock_t _inArrayRock;

	glui32 _echoLineInput;
	glui32 *_lineTerminators;

	/* style hints and settings */
	WindowStyle styles[style_NUMSTYLES];

	/* for copy selection */
	glui32 *_copyBuf;
	int _copyPos;
public:
	/**
	 * Constructor
	 */
	TextBufferWindow(Windows *windows, uint32 rock);

	/**
	 * Destructor
	 */
	virtual ~TextBufferWindow();

	/**
	 * Rearranges the window
	 */
	virtual void rearrange(const Common::Rect &box) override;

	/**
	 * Get window split size within parent pair window
	 */
	virtual glui32 getSplit(glui32 size, bool vertical) const override;

	/**
	 * Write a unicode character
	 */
	virtual void putCharUni(uint32 ch) override;

	/**
	 * Unput a unicode character
	 */
	virtual bool unputCharUni(uint32 ch) override;

	/**
	 * Move the cursor
	 */
	virtual void moveCursor(const Common::Point &newPos) override;

	/**
	 * Clear the window
	 */
	virtual void clear() override;

	/**
	 * Prepare for inputing a line
	 */
	virtual void requestLineEvent(char *buf, glui32 maxlen, glui32 initlen) override;

	/**
	 * Prepare for inputing a line
	 */
	virtual void requestLineEventUni(glui32 *buf, glui32 maxlen, glui32 initlen) override;

	/**
	 * Cancel an input line event
	 */
	virtual void cancelLineEvent(Event *ev) override;

	/**
	 * Cancel a hyperlink event
	 */
	virtual void cancelHyperlinkEvent() override { _hyperRequest = false; }

	/**
	 * Redraw the window
	 */
	virtual void redraw() override;
};

} // End of namespace Gargoyle

#endif
