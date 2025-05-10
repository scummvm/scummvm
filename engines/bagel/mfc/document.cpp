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

#include "common/textconsole.h"
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {

const CString &CDocument::GetTitle() const {
	return _title;
}

void CDocument::SetTitle(LPCSTR lpszTitle) {
	_title = CString(lpszTitle);
}

const CString &CDocument::GetPathName() const {
	return _unusedPathName;
}
void CDocument::SetPathName(LPCSTR lpszPathName, BOOL bAddToMRU) {
}

void CDocument::ClearPathName() {
}

BOOL CDocument::IsModified() {
	return _isModified;
}

void CDocument::SetModifiedFlag(BOOL bModified) {
	_isModified = bModified;
}

void CDocument::DeleteContents() {
}

void CDocument::ReportSaveLoadException(LPCSTR lpszPathName,
		CException *e, BOOL bSaving, UINT nIDPDefault) {
	error("Save/load error");
}


} // namespace MFC
} // namespace Bagel
