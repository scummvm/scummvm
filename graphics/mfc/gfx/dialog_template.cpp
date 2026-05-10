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
#include "graphics/mfc/gfx/dialog_template.h"
#include "graphics/mfc/afxwin.h"

namespace Graphics {
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

bool CDialogTemplate::HasFont() const {
	return false;
}

bool CDialogTemplate::SetFont(const char *lpFaceName, uint16 nFontSize) {
	return false;
}

bool CDialogTemplate::SetSystemFont(uint16 nFontSize) {
	return false;
}

bool CDialogTemplate::GetFont(CString &strFaceName, uint16 &nFontSize) const {
	return false;
}

void CDialogTemplate::GetSizeInDialogUnits(SIZE *pSize) const {
}

void CDialogTemplate::GetSizeInPixels(SIZE *pSize) const {
}

bool CDialogTemplate::GetFont(LPCDLGTEMPLATE pTemplate,
		CString &strFaceName, uint16 &nFontSize) {
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

LOGFONT CDialogTemplate::ParseFontFromTemplate() {
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));

	Common::strcpy_s(lf.lfFaceName,
		_header._fontInfo._fontName.c_str());
	lf.lfHeight = _header._fontInfo._pointSize;

	return lf;
}

void CDialogTemplate::getBaseUnits(int &x, int &y) {
	// Step 1: Get the font
	LOGFONT lf = ParseFontFromTemplate();
	HFONT hFont = CreateFontIndirect(&lf);

	// Step 2: Create a temporary DC and select the font
	HDC hdc = CreateCompatibleDC(nullptr);
	HGDIOBJ hOldFont = SelectObject(hdc, hFont);

	// Step 3: Get metrics
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);

	// Calculate DLU conversion
	x = tm.tmAveCharWidth;
	y = tm.tmHeight;

	// Clean up
	SelectObject(hdc, hOldFont);
	DeleteDC(hdc);
	DeleteObject(hFont);
}

void CDialogTemplate::loadTemplate(CDialog *dialog) {
	int base_unit_x, base_unit_y;
	getBaseUnits(base_unit_x, base_unit_y);

	// Set up the overall window
	RECT bounds = RectToRECT(
		SafeMulDiv(_header._x, base_unit_x, 4),
		SafeMulDiv(_header._y, base_unit_y, 8),
		SafeMulDiv(_header._x + _header._w, base_unit_x, 4),
		SafeMulDiv(_header._y + _header._h, base_unit_y, 8)
	);

	// WORKAROUND: For Hodj n Podj Boardgame dialog
	bounds.right = MIN<long>(bounds.right, 640);
	bounds.bottom = MIN<long>(bounds.bottom, 480);

	CWnd *wndDialog = static_cast<CWnd *>(dialog);
	wndDialog->Create(_header._className.c_str(),
		_header._caption.c_str(),
		_header._style | WS_POPUP | WS_VISIBLE,
		bounds,
		dialog->m_pParentWnd,
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

		// Convert dialog DLU to actual pixels
		int x1 = SafeMulDiv(item._x, base_unit_x, 4);
		int y1 = SafeMulDiv(item._y, base_unit_y, 8);
		int x2 = SafeMulDiv(item._x + item._w, base_unit_x, 4);
		int y2 = SafeMulDiv(item._y + item._h, base_unit_y, 8);

		// Set up control
		bounds = RectToRECT(x1, y1, x2, y2);
		ctl->Create(item._className.c_str(),
			item._title.c_str(),
			item._style | WS_VISIBLE | WS_TABSTOP,
			bounds,
			dialog,
			item._id
		);

		// Register the control as needing to be
		// freed when the dialog is closed
		dialog->_ownedControls.push_back(ctl);

		if (item._style & BS_DEFPUSHBUTTON)
			dialog->_defaultId = item._id;
	}

	// Apply the font to the window and all child controls
	LOGFONT lf = ParseFontFromTemplate();
	dialog->_dialogFont.CreateFontIndirect(&lf);
	dialog->SendMessageToDescendants(WM_SETFONT,
		(WPARAM)dialog->_dialogFont.m_hObject, 0);

	// Allow the dialog to set up replacement controls
	CDataExchange exchange = { false };
	dialog->DoDataExchange(&exchange);
}

} // namespace Gfx
} // namespace MFC
} // namespace Graphics
