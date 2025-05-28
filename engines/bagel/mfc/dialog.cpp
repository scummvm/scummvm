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

BOOL CDialog::Create(LPCSTR lpszTemplateName,
                     CWnd *pParentWnd) {
	error("TODO: CDialog::Create");
}
BOOL CDialog::Create(UINT nIDTemplate,
                     CWnd *pParentWnd) {
	error("TODO: CDialog::Create");
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

void CDialog::DDX_Control(CDataExchange *pDX, int nIDC, CWnd &rControl) {
	error("TODO: CDialog::DDX_Control");
}

void CDialog::DDX_Radio(CDataExchange *pDX,
                        int nIDCButton1, int &value) {
	error("TODO: CDialog::DDX_Radio");
}

void CDialog::DDX_Text(CDataExchange *pDX, int nIDC, int &value) {
	error("TODO: CDialog::DDX_Text");
}
void CDialog::DDX_Text(CDataExchange *pDX, int nIDC, CString &value) {
	error("TODO: CDialog::DDX_Text");
}
void CDialog::DDX_Text(CDataExchange *pDX, int nIDC, UINT &value) {
	error("TODO: CDialog::DDX_Text");
}
void CDialog::DDX_Text(CDataExchange *pDX, int nIDC, long &value) {
	error("TODO: CDialog::DDX_Text");
}
void CDialog::DDX_Text(CDataExchange *pDX, int nIDC, double &value) {
	error("TODO: CDialog::DDX_Text");
}

void CDialog::DDX_Check(CDataExchange *pDX,
                        int nIDC, int &value) {
	error("CDialog::DDX_Check");
}

void CDialog::DDV_MinMaxInt(CDataExchange *pDX,
                            int value, int nMin, int nMax) {
	error("TODO: CDialog::DDV_MinMaxInt");
}

void CDialog::EndDialog(int nResult) {
	error("TODO: CDialog::EndDialog");
}

BOOL CDialog::UpdateData(BOOL bSaveAndValidate) {
	error("TODO: CDialog::UpdateData");
}


} // namespace MFC
} // namespace Bagel
