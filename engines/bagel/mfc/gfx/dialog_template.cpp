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

#include "common/memstream.h"
#include "bagel/mfc/gfx/dialog_template.h"
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {
namespace Gfx {

CDialogTemplate::CDialogTemplate(LPCDLGTEMPLATE pTemplate) {
	setTemplate(pTemplate);
}

CDialogTemplate::CDialogTemplate(HGLOBAL hGlobal) {
	LPDLGTEMPLATE dlgTemplate = (LPDLGTEMPLATE)GlobalLock(hGlobal);
	setTemplate(dlgTemplate);
	GlobalUnlock(hGlobal);
}

bool CDialogTemplate::setTemplate(LPCDLGTEMPLATE pTemplate) {
	Common::MemoryReadStream src((const byte *)pTemplate, 99999);
	_header.load(src);

	_items.clear();
	_items.resize(_header._itemCount);

	for (auto &item : _items)
		item.load(src);

	return true;
}

BOOL CDialogTemplate::HasFont() const {
	return false;
}

BOOL CDialogTemplate::SetFont(LPCSTR lpFaceName, WORD nFontSize) {
	return false;
}

BOOL CDialogTemplate::SetSystemFont(WORD nFontSize) {
	return false;
}

BOOL CDialogTemplate::GetFont(CString &strFaceName, WORD &nFontSize) const {
	return false;
}

void CDialogTemplate::GetSizeInDialogUnits(SIZE *pSize) const {
}

void CDialogTemplate::GetSizeInPixels(SIZE *pSize) const {
}

BOOL CDialogTemplate::GetFont(LPCDLGTEMPLATE pTemplate,
		CString &strFaceName, WORD &nFontSize) {
	return false;
}

/*--------------------------------------------*/

void CDialogTemplate::Header::load(Common::SeekableReadStream &src) {
	byte bTerm;

	_style = src.readUint32LE();
	_itemCount = src.readByte();
	_x = src.readSint16LE();
	_y = src.readSint16LE();
	_w = src.readSint16LE();
	_h = src.readSint16LE();

	bTerm = src.readByte();
	switch (bTerm) {
	case 0:
		_menuName.clear();
		break;
	case 0xff:
		// Int resource Id
		src.skip(2);
		break;
	default:
		src.seek(-1, SEEK_CUR);
		_menuName = src.readString();
		break;
	}

	_className = src.readString();
	_caption = src.readString();

	if (_style & DS_SETFONT) {
		_fontInfo._pointSize = src.readUint16LE();
		_fontInfo._fontName = src.readString();
	} else {
		_fontInfo._pointSize = 0;
		_fontInfo._fontName.clear();
	}
}

/*--------------------------------------------*/

void CDialogTemplate::Item::load(Common::SeekableReadStream &src) {
	byte bTerm;

	_x = src.readSint16LE();
	_y = src.readSint16LE();
	_w = src.readSint16LE();
	_h = src.readSint16LE();
	_id = src.readUint16LE();
	_style = src.readUint32LE();

	bTerm = src.readByte();
	if (bTerm & 0x80) {
		switch (bTerm) {
		case 0x80: _className = "BUTTON"; break;
		case 0x81: _className = "EDIT"; break;
		case 0x82: _className = "STATIC"; break;
		case 0x83: _className = "LISTBOX"; break;
		case 0x84: _className = "SCROLLBAR"; break;
		case 0x85: _className = "COMBOBOX"; break;
		default:
			_className.clear();
			break;
		}
	} else {
		src.seek(-1, SEEK_CUR);
		_className = src.readString();
	}

	bTerm = src.readByte();
	if (bTerm == 0xff) {
		// Integer id, not documented
		src.skip(2);
	} else {
		src.seek(-1, SEEK_CUR);
		_title = src.readString();
	}

	_data.resize(src.readByte());
	if (!_data.empty())
		src.read(&_data[0], _data.size());
}

void CDialogTemplate::loadTemplate(CWnd *parent) {
	// Set up the overall window
	RECT bounds(_header._x, _header._style,
		_header._x + _header._w,
		_header._y + _header._h);
	parent->Create(_header._className.c_str(),
		_header._caption.c_str(),
		_header._style,
		bounds,
		parent->m_pParentWnd,
		0
	);

	// Iterate through the controls
	for (const auto &item : _items) {
		CWnd *ctl;
		if (item._className == "BUTTON")
			ctl = new CButton();
		else if (item._className == "EDIT")
			ctl = new CEdit();
		else if (item._className == "STATIC")
			ctl = new CStatic();
		else if (item._className == "LISTBOX")
			ctl = new CListBox();
		else if (item._className == "SCROLLBAR")
			ctl = new CScrollBar();
		else
			error("Unhandle dialog item - %s",
				item._className.c_str());

		// Set up control
		bounds = RECT(item._x, item._style,
			item._x + item._w,
			item._y + item._h);
		ctl->Create(item._className.c_str(),
			item._title.c_str(),
			item._style,
			bounds,
			parent,
			item._id
		);

		// Register the control as needing to be
		// freed when the dialog is closed
		parent->_ownedControls.push_back(ctl);
	}
}

} // namespace Gfx
} // namespace MFC
} // namespace Bagel
