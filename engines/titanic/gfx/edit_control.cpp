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

#include "titanic/gfx/edit_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CEditControl, CGameObject)
	ON_MESSAGE(EditControlMsg)
END_MESSAGE_MAP()

CEditControl::CEditControl() : CGameObject(), _fieldBC(false),  _fontNumber(0), _fieldD4(2),
		_textR(0), _textG(0), _textB(0), _fieldF0(0), _isPassword(false) {
}

void CEditControl::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeNumberLine(_editLeft, indent);
	file->writeNumberLine(_editBottom, indent);
	file->writeNumberLine(_editHeight, indent);
	file->writeNumberLine(_maxTextChars, indent);
	file->writeNumberLine(_fontNumber, indent);
	file->writeNumberLine(_fieldD4, indent);
	file->writeNumberLine(_textR, indent);
	file->writeNumberLine(_textG, indent);
	file->writeNumberLine(_textB, indent);
	file->writeQuotedLine(_text, indent);
	file->writeNumberLine(_fieldF0, indent);
	file->writeNumberLine(_isPassword, indent);

	CGameObject::save(file, indent);
}

void CEditControl::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_editLeft = file->readNumber();
	_editBottom = file->readNumber();
	_editHeight = file->readNumber();
	_maxTextChars = file->readNumber();
	_fontNumber = file->readNumber();
	_fieldD4 = file->readNumber();
	_textR = file->readNumber();
	_textG = file->readNumber();
	_textB = file->readNumber();
	_text = file->readString();
	_fieldF0 = file->readNumber();
	_isPassword = file->readNumber();

	CGameObject::load(file);
}

bool CEditControl::EditControlMsg(CEditControlMsg *msg) {
	switch (msg->_mode) {
	case 0: {
		// WORKAROUND: Fix original bug where MissiveOMat username & password
		// text weren't initialised after the first time you use the MissiveOMat
		_editHeight = _bounds.height();
		_editBottom = _bounds.bottom;
		_editLeft = _bounds.left + _bounds.width() / 2;
		_maxTextChars = msg->_param;
		setTextFontNumber(_fontNumber);

		CEditControlMsg ctlMsg;
		ctlMsg._mode = 10;
		ctlMsg._param = _fieldD4;
		ctlMsg.execute(this);

		ctlMsg._mode = 11;
		ctlMsg._textR = _textR;
		ctlMsg._textG = _textG;
		ctlMsg._textB = _textB;
		ctlMsg.execute(this);
		break;
	}

	case 1: {
		_text = "";
		CEditControlMsg ctlMsg;
		ctlMsg._mode = 14;
		ctlMsg.execute(this);
		break;
	}

	case 2: {
		_text = msg->_text;
		CEditControlMsg ctlMsg;
		ctlMsg._mode = 14;
		ctlMsg.execute(this);
		break;
	}

	case 3:
		msg->_text = _text;
		break;

	case 4:
		msg->_param = _text.size();
		break;

	case 5:
		_maxTextChars = msg->_param;
		break;

	case 6:
		if (msg->_param == 8 && !_text.empty()) {
			_text = _text.left(_text.size() - 1);
			CEditControlMsg ctlMsg;
			ctlMsg._mode = 14;
			ctlMsg.execute(this);
		} else if (msg->_param == 13) {
			msg->_param = 1000;
		} else if (msg->_param >= 32 && msg->_param < 127
				&& _text.size() < _maxTextChars) {
			char c = (char)msg->_param;
			_text += c;

			CEditControlMsg ctlMsg;
			ctlMsg._mode = 14;
			ctlMsg.execute(this);
		}
		break;

	case 7:
		setTextFontNumber(msg->_param);
		break;

	case 8:
		if (!_fieldBC) {
			_fieldBC = true;
			CEditControlMsg ctlMsg;
			ctlMsg._mode = 14;
			ctlMsg.execute(this);
		}
		break;

	case 9:
		if (_fieldBC) {
			_fieldBC = false;
			getTextCursor()->hide();
		}
		break;

	case 10: {
		setTextHasBorders((msg->_param & 1) != 0);
		if (msg->_param & 4)
			_fieldF0 = 1;
		else if (msg->_param & 8)
			_fieldF0 = 2;
		else
			_fieldF0 = 0;

		_isPassword = (msg->_param & 0x10) != 0;
		CEditControlMsg ctlMsg;
		ctlMsg._mode = 14;
		ctlMsg.execute(this);
		break;
	}

	case 11:
		setTextColor(msg->_textR, msg->_textG, msg->_textB);
		break;

	case 12:
		setVisible(true);
		break;

	case 13:
		setVisible(false);
		break;

	case 14: {
		makeDirty();
		CString str = _isPassword ? CString('*', _text.size()) : _text;
		setText(str);

		int textWidth = getTextWidth();
		if (_fieldF0 == 2) {
			_bounds.left = _editLeft - textWidth / 2;
			_bounds.setWidth(textWidth + 16);
			setTextBounds();
			makeDirty();
		}

		if (_fieldBC) {
			CTextCursor *textCursor = getTextCursor();
			textCursor->show();
			textCursor->setPos(Point(_bounds.left + textWidth + 1, _bounds.top + 3));
			textCursor->setSize(Point(2, _editHeight - 6));
			textCursor->setColor(0xff, 0xff, 0xff);
			textCursor->clearBounds();
		}
		break;
	}

	default:
		break;
	}

	return true;
}

} // End of namespace Titanic
