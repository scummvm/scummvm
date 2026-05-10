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
#include "graphics/mfc/afxwin.h"

namespace Graphics {
namespace MFC {

CMenu::CMenu() {
}

bool CMenu::CreateMenu() {
	error("TODO: CMenu::CreateMenu");
}
bool CMenu::CreatePopupMenu() {
	error("TODO: CMenu::CreatePopupMenu");
}
bool CMenu::LoadMenu(const char *lpszResourceName) {
	error("TODO: CMenu::LoadMenu");
}
bool CMenu::LoadMenu(unsigned int nIDResource) {
	error("TODO: CMenu::LoadMenu");
}
bool CMenu::LoadMenuIndirect(const void *lpMenuTemplate) {
	error("TODO: CMenu::LoadMenuIndirect");
}
bool CMenu::DestroyMenu() {
	error("TODO: CMenu::DestroyMenu");
}

CMenu *CMenu::FromHandle(HMENU hMenu) {
	error("TODO: CMenu::FromHandle");
}

unsigned int CMenu::CheckMenuItem(unsigned int nIDCheckItem, unsigned int nCheck) {
	error("TODO: CMenu::CheckMenuItem");
}


} // namespace MFC
} // namespace Graphics
