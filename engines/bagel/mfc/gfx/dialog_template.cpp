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

	_style = src.readUint16LE();
	_itemCount = src.readByte();
	_x = src.readSint16LE();
	_y = src.readSint16LE();
	_w = src.readSint16LE();
	_h = src.readSint16LE();

	bTerm = src.readByte();
	if (!bTerm) {
		src.skip(1);
		_menuName.clear();
	} else {
		src.seek(-1, SEEK_CUR);
		_menuName = src.readString();
	}

	bTerm = src.readByte();
	if (!bTerm) {
		src.skip(1);
		_className.clear();
	} else {
		src.seek(-1, SEEK_CUR);
		_className = src.readString();
	}

	_caption = src.readString();

	if (_style & DS_SETFONT) {
		_fontInfo._pointSize = src.readByte();
		_fontInfo._fontName = src.readString();
	} else {
		_fontInfo._pointSize = 0;
		_fontInfo._fontName.clear();
	}
}

/*--------------------------------------------*/

void CDialogTemplate::Item::load(Common::SeekableReadStream &src) {
	byte bTerm;

	_style = src.readUint16LE();
	_x = src.readSint16LE();
	_y = src.readSint16LE();
	_w = src.readSint16LE();
	_h = src.readSint16LE();
	_id = src.readUint16LE();

	bTerm = src.readByte();
	if (bTerm == 0x80) {
		_classNameId = ((int)bTerm << 8) | src.readByte();
	} else {
		src.seek(-1, SEEK_CUR);
		_classNameStr = src.readString();
	}

	bTerm = src.readByte();
	if (bTerm == 0x80) {
		_titleId = ((int)bTerm << 8) | src.readByte();
	} else {
		src.seek(-1, SEEK_CUR);
		_titleStr = src.readString();
	}

	_custom.resize(src.readUint16LE());
	if (!_custom.empty())
		src.read(&_custom[0], _custom.size());
}

} // namespace Gfx
} // namespace MFC
} // namespace Bagel
