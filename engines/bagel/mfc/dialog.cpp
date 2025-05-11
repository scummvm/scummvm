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

IMPLEMENT_DYNAMIC(CDialog, CWnd)
BEGIN_MESSAGE_MAP(CDialog, CWnd)
END_MESSAGE_MAP()

void CDialog::OnOK() {
	error("TODO: CDialog::OnOK");
}

void CDialog::OnCancel() {
	error("TODO: CDialog::OnCancel");
}

CDialog::CDialog(LPCSTR lpszTemplateName,
		CWnd *pParentWnd) {
}

CDialog::CDialog(UINT nIDTemplate, CWnd *pParentWnd) {
}

int CDialog::DoModal() {
	error("TODO: CDialog::DoModal");
}

BOOL CDialog::OnInitDialog() {
	error("TODO: CDialog::OnInitDialog");
}

DWORD CDialog::GetDefID() {
	error("TODO: CDialog::GetDefID");
}

void CDialog::SetDefID(UINT nID) {
	error("TODO: CDialog::SetDefID");
}

void CDialog::EndDialog(int nResult) {
	error("TODO: CDialog::EndDialog");
}

} // namespace MFC
} // namespace Bagel
