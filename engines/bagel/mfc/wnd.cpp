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

IMPLEMENT_DYNAMIC(CWnd, CCmdTarget)
BEGIN_MESSAGE_MAP(CWnd, CCmdTarget)
END_MESSAGE_MAP()

BOOL CWnd::Create(LPCSTR lpszClassName, LPCSTR lpszWindowName,
		DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID,
		CCreateContext *pContext) {
	error("TODO: CWnd::Create");
}

const MSG *CWnd::GetCurrentMessage() {
	return nullptr;
}

void CWnd::DoDataExchange(CDataExchange *) {
	error("TODO: CWnd::DoDataExchange");
}

BOOL CWnd::OnCommand(WPARAM wParam, LPARAM lParam) {
	error("TODO: CWnd::OnCommand");
}

BOOL CWnd::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult) {
	error("TODO: CWnd::OnNotify");
}

void CWnd::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized) {}
void CWnd::OnActivateApp(BOOL bActive, DWORD dwThreadID) {}
LRESULT CWnd::OnActivateTopLevel(WPARAM, LPARAM) { return 0; }
void CWnd::OnCancelMode() {}
void CWnd::OnChildActivate() {}
void CWnd::OnClose() {}
void CWnd::OnContextMenu(CWnd *pWnd, CPoint pos) {}
int CWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) { return 0; }
HBRUSH CWnd::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor) { return 0; }
void CWnd::OnDestroy() {}
void CWnd::OnEnable(BOOL bEnable) {}
void CWnd::OnEndSession(BOOL bEnding) {}
void CWnd::OnEnterIdle(UINT nWhy, CWnd *pWho) {}
BOOL CWnd::OnEraseBkgnd(CDC *pDC) { return false; }
void CWnd::OnIconEraseBkgnd(CDC *pDC) {}
void CWnd::OnKillFocus(CWnd *pNewWnd) {}
void CWnd::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu) {}
void CWnd::OnMove(int x, int y) {}
void CWnd::OnPaint() {}
void CWnd::OnSyncPaint() {}
void CWnd::OnParentNotify(UINT message, LPARAM lParam) {}
UINT CWnd::OnNotifyFormat(CWnd *pWnd, UINT nCommand) { return 0; }
HCURSOR CWnd::OnQueryDragIcon() { return 0; }
BOOL CWnd::OnQueryEndSession() { return false; }
BOOL CWnd::OnQueryNewPalette() { return false; }
BOOL CWnd::OnQueryOpen() { return false; }
void CWnd::OnSetFocus(CWnd *pOldWnd) {}
void CWnd::OnShowWindow(BOOL bShow, UINT nStatus) {}
void CWnd::OnSize(UINT nType, int cx, int cy) {}
void CWnd::OnTCard(UINT idAction, DWORD dwActionData) {}
void CWnd::OnSessionChange(UINT nSessionState, UINT nId) {}
void CWnd::OnChangeUIState(UINT nAction, UINT nUIElement) {}
void CWnd::OnUpdateUIState(UINT nAction, UINT nUIElement) {}
UINT CWnd::OnQueryUIState() { return 0; }
BOOL CWnd::OnNcActivate(BOOL bActive) { return false; }
BOOL CWnd::OnNcCreate(LPCREATESTRUCT lpCreateStruct) { return false; }
void CWnd::OnNcDestroy() {}
LRESULT CWnd::OnNcHitTest(CPoint point) { return 0; }
void CWnd::OnNcLButtonDblClk(UINT nHitTest, CPoint point) {}
void CWnd::OnNcLButtonDown(UINT nHitTest, CPoint point) {}
void CWnd::OnNcLButtonUp(UINT nHitTest, CPoint point) {}
void CWnd::OnNcMButtonDblClk(UINT nHitTest, CPoint point) {}
void CWnd::OnNcMButtonDown(UINT nHitTest, CPoint point) {}
void CWnd::OnNcMButtonUp(UINT nHitTest, CPoint point) {}
void CWnd::OnNcMouseHover(UINT nHitTest, CPoint point) {}
void CWnd::OnNcMouseLeave() {}
void CWnd::OnNcMouseMove(UINT nHitTest, CPoint point) {}
void CWnd::OnNcPaint() {}
void CWnd::OnNcRButtonDblClk(UINT nHitTest, CPoint point) {}
void CWnd::OnNcRButtonDown(UINT nHitTest, CPoint point) {}
void CWnd::OnNcRButtonUp(UINT nHitTest, CPoint point) {}
void CWnd::OnNcXButtonDown(short zHitTest, UINT nButton, CPoint point) {}
void CWnd::OnNcXButtonUp(short zHitTest, UINT nButton, CPoint point) {}
void CWnd::OnNcXButtonDblClk(short zHitTest, UINT nButton, CPoint point) {}
void CWnd::OnDropFiles(HDROP hDropInfo) {}
void CWnd::OnPaletteIsChanging(CWnd *pRealizeWnd) {}
void CWnd::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags) {}
void CWnd::OnSysCommand(UINT nID, LPARAM lParam) {}
void CWnd::OnSysDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags) {}
void CWnd::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {}
void CWnd::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {}
BOOL CWnd::OnAppCommand(CWnd *pWnd, UINT nCmd, UINT nDevice, UINT nKey) { return false; }
void CWnd::OnRawInput(UINT nInputCode, HRAWINPUT hRawInput) {}
void CWnd::OnCompacting(UINT nCpuTime) {}
void CWnd::OnDevModeChange(_In_z_ LPTSTR lpDeviceName) {}
void CWnd::OnFontChange() {}
void CWnd::OnPaletteChanged(CWnd *pFocusWnd) {}
void CWnd::OnSpoolerStatus(UINT nStatus, UINT nJobs) {}
void CWnd::OnSysColorChange() {}
void CWnd::OnTimeChange() {}
void CWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) {}
void CWnd::OnWinIniChange(LPCTSTR lpszSection) {}
UINT CWnd::OnPowerBroadcast(UINT nPowerEvent, LPARAM lEventData) { return 0; }
void CWnd::OnUserChanged() {}
void CWnd::OnInputLangChange(UINT nCharSet, UINT nLocaleId) {}
void CWnd::OnInputLangChangeRequest(UINT nFlags, UINT nLocaleId) {}
void CWnd::OnInputDeviceChange(unsigned short nFlags, HANDLE hDevice) {}
void CWnd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {}
void CWnd::OnDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags) {}
void CWnd::OnUniChar(UINT nChar, UINT nRepCnt, UINT nFlags) {}
void CWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar) {}
void CWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar) {}
void CWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {}
void CWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {}
void CWnd::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2) {}
void CWnd::OnLButtonDblClk(UINT nFlags, CPoint point) {}
void CWnd::OnLButtonDown(UINT nFlags, CPoint point) {}
void CWnd::OnLButtonUp(UINT nFlags, CPoint point) {}
void CWnd::OnMButtonDblClk(UINT nFlags, CPoint point) {}
void CWnd::OnMButtonDown(UINT nFlags, CPoint point) {}
void CWnd::OnMButtonUp(UINT nFlags, CPoint point) {}
void CWnd::OnXButtonDblClk(UINT nFlags, UINT nButton, CPoint point) {}
void CWnd::OnXButtonDown(UINT nFlags, UINT nButton, CPoint point) {}
void CWnd::OnXButtonUp(UINT nFlags, UINT nButton, CPoint point) {}
int CWnd::OnMouseActivate(CWnd *pDesktopWnd, UINT nHitTest, UINT message) { return 0; }
void CWnd::OnMouseHover(UINT nFlags, CPoint point) {}
void CWnd::OnMouseLeave() {}
void CWnd::OnMouseMove(UINT nFlags, CPoint point) {}
void CWnd::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt) {}
BOOL CWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) { return false; }
void CWnd::OnRButtonDblClk(UINT nFlags, CPoint point) {}
void CWnd::OnRButtonDown(UINT nFlags, CPoint point) {}
void CWnd::OnRButtonUp(UINT nFlags, CPoint point) {}
BOOL CWnd::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message) { return false; }
void CWnd::OnTimer(UINT_PTR nIDEvent) {}
void CWnd::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {}
UINT CWnd::OnGetDlgCode() { return 0; }
void CWnd::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) {}
int CWnd::OnCharToItem(UINT nChar, CListBox *pListBox, UINT nIndex) { return 0; }
int CWnd::OnVKeyToItem(UINT nKey, CListBox *pListBox, UINT nIndex) { return 0; }

CWnd *CWnd::FromHandlePermanent(HWND hWnd) {
	error("TODO: CWnd::FromHandlePermanent");
}

CWnd *CWnd::GetParent() const {
	error("TODO: CWnd::GetParent");
}

void CWnd::ShowWindow(int nCmdShow) {
	error("TODO: CWnd::ShowWindow");
}

BOOL CWnd::EnableWindow(BOOL bEnable) {
	error("TODO: CWnd::EnableWindow");
}

void CWnd::UpdateWindow() {
	error("TODO: CWnd::UpdateWindow");
}

void CWnd::SetActiveWindow() {
	error("TODO: CWnd::SetActiveWindow");
}

void CWnd::SetFocus() {
	error("TODO: CWnd::SetFocus");
}

void CWnd::DestroyWindow() {
	error("TODO: CWnd::DestroyWindow");
}

BOOL CWnd::IsWindowVisible() const {
	error("TODO: CWnd::IsWindowVisible");
}

void CWnd::Invalidate(BOOL bErase) {
	error("TODO: CWnd::Invalidate");
}

int CWnd::GetWindowText(CString &rString) const {
	error("TODO: CWnd::GetWindowText");
}

int CWnd::GetWindowText(LPSTR lpszStringBuf, int nMaxCount) const {
	error("TODO: CWnd::GetWindowText");
}

BOOL CWnd::SetWindowText(LPCSTR lpszString) {
	error("TODO: CWnd::SetWindowText");
}

CDC *CWnd::GetDC() {
	error("TODO: CWnd::GetDC");
}

int CWnd::ReleaseDC(CDC *pDC) {
	error("TODO: CWnd::ReleaseDC");
}

BOOL CWnd::PostMessage(UINT message, WPARAM wParam, LPARAM lParam) {
	error("TODO: CWnd::PostMessage");
}

LRESULT CWnd::SendMessage(UINT message, WPARAM wParam, LPARAM lParam) {
	error("TODO: CWnd::SendMessage");
}

BOOL CWnd::ValidateRect(LPCRECT lpRect) {
	error("TODO: CWnd::ValidateRect");
}

BOOL CWnd::InvalidateRect(LPCRECT lpRect, BOOL bErase) {
	error("TODO: CWnd::InvalidateRect");
}

void CWnd::GetWindowRect(LPRECT lpRect) const {
	error("TODO: CWnd::GetWindowRect");
}

BOOL CWnd::GetUpdateRect(LPRECT lpRect, BOOL bErase) {
	error("TODO: CWnd::GetUpdateRect");
}

BOOL CWnd::GetClientRect(LPRECT lpRect) const {
	error("TODO: CWnd::GetClientRect");
}
void CWnd::MoveWindow(LPCRECT lpRect, BOOL bRepaint) {
	error("TODO: CWnd::MoveWindow");
}

void CWnd::MoveWindow(int x, int y, int nWidth, int nHeight,
		BOOL bRepaint) {
	error("TODO: CWnd::MoveWIndow");
}

HDC CWnd::BeginPaint(LPPAINTSTRUCT lpPaint) {
	error("TODO: CWnd::BeginPaint");
}

BOOL CWnd::EndPaint(const PAINTSTRUCT *lpPaint) {
	error("TODO: CWnd::EndPaint");
}

CWnd *CWnd::GetDlgItem(int nID) const {
	error("TODO: CWnd::GetDlgItem");
}

CWnd *CWnd::GetNextDlgGroupItem(CWnd *pWndCtl, BOOL bPrevious) const {
	error("TODO: CWnd::GetNextDlgGroupItem");
}

BOOL CWnd::GotoDlgCtrl(CWnd *pWndCtrl) {
	error("TODO: CWnd::GotoDlgCtrl");
}

BOOL CWnd::SubclassDlgItem(UINT nID, CWnd *pParent) {
	error("TODO: CWnd::SubclassDlgItem");
}

int CWnd::GetDlgCtrlID() const {
	error("TODO: CWnd::GetDlgCtrlID");
}

void CWnd::CheckDlgButton(int nIDButton, UINT nCheck) {
	error("TODO: CWnd::CheckDlgButton");
}

LRESULT CWnd::SendDlgItemMessage(int nID, UINT message,
		WPARAM wParam, LPARAM lParam) const {
	error("TODO: CWnd::SendDlgItemMessage");
}

} // namespace MFC
} // namespace Bagel
