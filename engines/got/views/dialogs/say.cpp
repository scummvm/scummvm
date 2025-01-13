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

#include "got/views/dialogs/say.h"
#include "got/vars.h"

namespace Got {
namespace Views {
namespace Dialogs {

Say::Say() : Dialog("Say") {
	setBounds(Common::Rect(32, 48, 288, 160));
}

void Say::show(int item, const Gfx::Pics &speakerIcon, int type) {
	Say *view = (Say *)g_events->findView("Say");
	view->_item = item;
	view->_speakerIcon = speakerIcon;
	view->_type = type;
	view->addView();
}

bool Say::msgFocus(const FocusMessage &msg) {
	if (!_type)
		play_sound(WOOP, 1);

	_content = (const char *)_G(tmp_buff);
	_contentLength = 0;
	_waitForResponse = WAIT_NONE;
	_woopCtr = 0;
	_picIndex = 0;

	return true;
}

bool Say::msgUnfocus(const UnfocusMessage &msg) {
	// Since the Ask dialog is shown by scripts, closing the view
	// starts the calling script running again
	_G(scripts).resume();
	return true;
}

void Say::draw() {
	Dialog::draw();
	GfxSurface s = getSurface();

	if (_item)
		s.simpleBlitFrom(_G(objects)[_item], Common::Point(160, 17));

	const char *p = _content;
	const char *endP = _content + _contentLength;
	int color = 14;
	int x = 20, lc = 0;

	while (p < endP) {
		if (*p == '~' && Common::isXDigit(*(p + 1))) {
			p++;
			int ch = *p;
			p++;
			if (Common::isDigit(ch))
				ch -= '0';
			else
				ch = toupper(ch) - 'A' + 10;

			color = Gfx::DIALOG_COLOR[ch];
			continue;
		}

		if (*p == '\n') {
			x = 20;
			lc++;
			if (lc > 4) {
				// Got a full text to display
				if (_waitForResponse == WAIT_NONE)
					_waitForResponse = WAIT_MORE;
				_picIndex = 1;
				break;
			}

			p++;
			continue;
		}

		s.printChar(*p, x, 35 + (lc * 10), color);

		p++;
		x += 8;
	}

	s.simpleBlitFrom(_speakerIcon[_picIndex], Common::Point(120, 17));

	if (_waitForResponse == WAIT_MORE)
		s.print(Common::Point(184, 86), "More...", 15);
}

bool Say::msgKeypress(const KeypressMessage &msg) {
	if (_waitForResponse == WAIT_DONE) {
		close();
	} else if (_waitForResponse == WAIT_MORE) {
		_waitForResponse = WAIT_NONE;
		_content = _content + _contentLength;
		_contentLength = 0;
		redraw();
	} else {
		showEntirePage();
		redraw();
	}

	return false;
}

bool Say::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE || _waitForResponse == WAIT_DONE) {
		close();
	} else if (_waitForResponse == WAIT_MORE) {
		_waitForResponse = WAIT_NONE;
		_content = _content + _contentLength;
		_contentLength = 0;
		redraw();
	} else {
		showEntirePage();
		redraw();
	}

	return true;
}

bool Say::tick() {
	if (_waitForResponse == WAIT_NONE && ++_contentCtr > 1) {
		_contentCtr = 0;
		const char *contentEnd = _content + ++_contentLength;

		if (*(contentEnd - 1) == '~' && Common::isXDigit(*contentEnd))
			++_contentLength;

		if (!*contentEnd) {
			// Reached end of text to display
			_waitForResponse = WAIT_DONE;
			_picIndex = 1;
		} else if (++_picIndex > 3) {
			_picIndex = 0;
		}

		if (_type) {
			play_sound(WOOP, 1);
			_woopCtr = 0;
		}

		redraw();
	}

	return true;
}

void Say::showEntirePage() {
	int lc = 0;
	char c;

	for (_contentLength = 0; (c = *(_content + _contentLength)) != 0; ++_contentLength) {
		if (c == '~' && Common::isXDigit(*(_content + _contentLength + 1)))
			++_contentLength;
		else if (c == '\n') {
			if (++lc > 4)
				break;
		}
	}
}

} // namespace Dialogs
} // namespace Views
} // namespace Got
