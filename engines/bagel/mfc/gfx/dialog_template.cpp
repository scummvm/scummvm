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

#include "bagel/mfc/gfx/dialog_template.h"
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {
namespace Gfx {

CDialogTemplate::CDialogTemplate(LPCDLGTEMPLATE pTemplate) {
}

CDialogTemplate::CDialogTemplate(HGLOBAL hGlobal) {
}

CDialogTemplate::~CDialogTemplate() {
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

BOOL CDialogTemplate::Load(LPCSTR lpDialogTemplateID) {
	return false;
}

HGLOBAL CDialogTemplate::Detach() {
	return 0;
}

} // namespace Gfx
} // namespace MFC
} // namespace Bagel
